/*
 * thermal_amg88.c
 *
 * Panasonic AMG88xx 8x8 infrared thermopile array driver.
 */

#include "thermal_amg88.h"
#include <string.h>
#include "logging.h"
#include "mydefs.h"

/* The HAL API expects the 8-bit address (7-bit address shifted left). */
#ifndef THERMAL_AMG88_I2C_ADDRESS
#define THERMAL_AMG88_I2C_ADDRESS (0x68U << 1)
#endif

#define THERMAL_AMG88_REG_POWER       0x00U
#define THERMAL_AMG88_REG_RESET       0x01U
#define THERMAL_AMG88_REG_FRAME_RATE  0x02U
#define THERMAL_AMG88_REG_PIXEL_BASE  0x80U

#define THERMAL_AMG88_POWER_NORMAL    0x00U
#define THERMAL_AMG88_POWER_SLEEP     0x10U
#define THERMAL_AMG88_RESET_FLAG      0x3FU
#define THERMAL_AMG88_FRAME_RATE_10HZ 0x00U
#define THERMAL_AMG88_FRAME_RATE_1HZ  0x01U
#define THERMAL_AMG88_PIXEL_COUNT     64U
#define THERMAL_AMG88_PIXEL_BYTES     (THERMAL_AMG88_PIXEL_COUNT * 2U)

thermal_amg88_t _thermal_amg88Data = { };

/**
 * @brief Writes one byte to an AMG88xx register.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral.
 * @param reg AMG88xx register address.
 * @param value Value to write.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
static HAL_StatusTypeDef thermal_amg88_WriteRegister(I2C_HandleTypeDef *hi2c,uint8_t reg, uint8_t value)
{
    uint8_t data[2] = { reg, value };

    return (hi2c == NULL) ? HAL_ERROR : HAL_I2C_Master_Transmit(hi2c, THERMAL_AMG88_I2C_ADDRESS, data, sizeof(data), 1000);
}

/**
 * @brief Converts an AMG88xx two-byte pixel value to degrees Celsius.
 *
 * @param low Least significant pixel byte.
 * @param high Most significant pixel byte.
 * @return Signed pixel temperature in degrees Celsius.
 */
static float thermal_amg88_DecodePixel(uint8_t low, uint8_t high)
{
    int16_t raw = (int16_t) (((uint16_t) high << 8) | low);

    /* Pixel data is a signed 12-bit two's-complement value in bits 11:0. */
    raw &= 0x0FFF;
    if ((raw & 0x0800) != 0)
    {
        raw |= (int16_t) 0xF000;
    }

    return (float) raw * 0.25f;
}

/**
 * @brief Detects and initializes an AMG88xx device.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return 1 when initialization succeeds, otherwise 0.
 */
int8_t thermal_amg88_Is(I2C_HandleTypeDef *hi2c)
{
    return (thermal_amg88_Init(hi2c) == HAL_OK) ? 1 : 0;
}

/**
 * @brief Resets and configures the AMG88xx sensor, then enters sleep mode.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_amg88_Init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    memset(&_thermal_amg88Data, 0, sizeof(_thermal_amg88Data));
    do
    {
        if (hi2c == NULL)
            break;
        if ((status = HAL_I2C_IsDeviceReady(hi2c, THERMAL_AMG88_I2C_ADDRESS, 2, 1000)) != HAL_OK)
            break;
        if ((status = thermal_amg88_WriteRegister(hi2c, THERMAL_AMG88_REG_RESET,THERMAL_AMG88_RESET_FLAG)) != HAL_OK)
            break;
        HAL_Delay(2);
        if ((status = thermal_amg88_WriteRegister(hi2c, THERMAL_AMG88_REG_FRAME_RATE, THERMAL_AMG88_FRAME_RATE_10HZ)) != HAL_OK)
            break;
        if ((status = thermal_amg88_Off(hi2c)) != HAL_OK)
            break;
        status = HAL_OK;
        _thermal_amg88Data.IsInit = 1;

    } while (0);

    return status;
}

/**
 * @brief Starts AMG88xx temperature conversion.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_amg88_On(I2C_HandleTypeDef *hi2c)
{
    return thermal_amg88_WriteRegister(hi2c, THERMAL_AMG88_REG_POWER, THERMAL_AMG88_POWER_NORMAL);
}

/**
 * @brief Stops conversion and places the AMG88xx in sleep mode.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_amg88_Off(I2C_HandleTypeDef *hi2c)
{
    return thermal_amg88_WriteRegister(hi2c, THERMAL_AMG88_REG_POWER,THERMAL_AMG88_POWER_SLEEP);
}

/**
 * @brief Reads all 64 AMG88xx pixels and updates the thermal data structure.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_amg88_Read(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint8_t raw[THERMAL_AMG88_PIXEL_BYTES];
    uint8_t row;
    uint8_t column;

    _thermal_amg88Data.IsDataValid = 0;
    do
    {
        if (hi2c == NULL)
            break;
        if (!_thermal_amg88Data.IsInit)
            break;
        if ((status = HAL_I2C_Mem_Read(hi2c, THERMAL_AMG88_I2C_ADDRESS, THERMAL_AMG88_REG_PIXEL_BASE, I2C_MEMADD_SIZE_8BIT, raw, sizeof(raw), 1000)) != HAL_OK)
            break;
        for (row = 0; row < THERMAL_AMG88_ROWS; ++row)
        {
            for (column = 0; column < THERMAL_AMG88_COLS; ++column)
            {
                uint16_t pixel = (uint16_t) row * 8U + column;
                _thermal_amg88Data.DataGrid[row][column] = thermal_amg88_DecodePixel(raw[pixel * 2U], raw[pixel * 2U + 1U]);
            }
        }
        _thermal_amg88Data.IsDataValid = 1;
        status = HAL_OK;

    }while(0);
    return status;
}

void thermal_logData()
{
    if (_thermal_amg88Data.IsDataValid)
    {
        int row, col;
        log_write("---------------------------");
        for (row = 0; row < THERMAL_AMG88_ROWS; ++row)
        {
            for (col = 0; col < THERMAL_AMG88_COLS; ++col)
            {
                log_writeRaw(PRIf_0X(5) " ", PRIf_02D(_thermal_amg88Data.DataGrid[row][col]));
            }
            log_writeRaw("\r\n");  // \n
        }
        log_write("---------------------------");
    }
    else
        log_write("_thermal_amg88Data no data");

}

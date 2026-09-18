/*
 * thermal_adi.c
 *
 * Panasonic AMG88xx 8x8 infrared thermopile array driver.
 */

#include "thermal_adi.h"
#include <string.h>

/* The HAL API expects the 8-bit address (7-bit address shifted left). */
#ifndef THERMAL_ADI_I2C_ADDRESS
#define THERMAL_ADI_I2C_ADDRESS (0x69U << 1)
#endif

#define THERMAL_ADI_REG_POWER       0x00U
#define THERMAL_ADI_REG_RESET       0x01U
#define THERMAL_ADI_REG_FRAME_RATE  0x02U
#define THERMAL_ADI_REG_PIXEL_BASE  0x80U

#define THERMAL_ADI_POWER_NORMAL    0x00U
#define THERMAL_ADI_POWER_SLEEP     0x10U
#define THERMAL_ADI_RESET_FLAG      0x3FU
#define THERMAL_ADI_FRAME_RATE_10HZ 0x00U
#define THERMAL_ADI_PIXEL_COUNT     64U
#define THERMAL_ADI_PIXEL_BYTES     (THERMAL_ADI_PIXEL_COUNT * 2U)

thermal_adi_t _thermal_adiData = { 0 };

/**
 * @brief Writes one byte to an AMG88xx register.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral.
 * @param reg AMG88xx register address.
 * @param value Value to write.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
static HAL_StatusTypeDef thermal_adi_WriteRegister(I2C_HandleTypeDef *hi2c,
                                                    uint8_t reg,
                                                    uint8_t value)
{
    uint8_t data[2] = { reg, value };
    return HAL_I2C_Master_Transmit(hi2c, THERMAL_ADI_I2C_ADDRESS,
                                   data, sizeof(data), HAL_MAX_DELAY);
}

/**
 * @brief Converts an AMG88xx two-byte pixel value to degrees Celsius.
 *
 * @param low Least significant pixel byte.
 * @param high Most significant pixel byte.
 * @return Signed pixel temperature in degrees Celsius.
 */
static float thermal_adi_DecodePixel(uint8_t low, uint8_t high)
{
    int16_t raw = (int16_t)(((uint16_t)high << 8) | low);

    /* Pixel data is a signed 12-bit two's-complement value in bits 11:0. */
    raw &= 0x0FFF;
    if ((raw & 0x0800) != 0)
    {
        raw |= (int16_t)0xF000;
    }

    return (float)raw * 0.25f;
}

/**
 * @brief Detects and initializes an AMG88xx device.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return 1 when initialization succeeds, otherwise 0.
 */
int8_t thermal_adi_Is(I2C_HandleTypeDef *hi2c)
{
    return (thermal_adi_Init(hi2c) == HAL_OK) ? 1 : 0;
}

/**
 * @brief Resets and configures the AMG88xx sensor, then enters sleep mode.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_adi_Init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    status = HAL_I2C_IsDeviceReady(hi2c, THERMAL_ADI_I2C_ADDRESS, 2,
                                   HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        _thermal_adiData.IsDataValid = 0;
        return status;
    }

    status = thermal_adi_WriteRegister(hi2c, THERMAL_ADI_REG_RESET,
                                       THERMAL_ADI_RESET_FLAG);
    if (status == HAL_OK)
    {
        HAL_Delay(2);
        status = thermal_adi_WriteRegister(hi2c, THERMAL_ADI_REG_FRAME_RATE,
                                           THERMAL_ADI_FRAME_RATE_10HZ);
    }
    if (status == HAL_OK)
    {
        status = thermal_adi_WriteRegister(hi2c, THERMAL_ADI_REG_POWER,
                                            THERMAL_ADI_POWER_SLEEP);
    }

    _thermal_adiData.IsDataValid = 0;
    if (status != HAL_OK)
    {
        memset(_thermal_adiData.DataGrid, 0, sizeof(_thermal_adiData.DataGrid));
    }
    return status;
}

/**
 * @brief Starts AMG88xx temperature conversion.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_adi_On(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    return thermal_adi_WriteRegister(hi2c, THERMAL_ADI_REG_POWER,
                                     THERMAL_ADI_POWER_NORMAL);
}

/**
 * @brief Stops conversion and places the AMG88xx in sleep mode.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_adi_Off(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    status = thermal_adi_WriteRegister(hi2c, THERMAL_ADI_REG_POWER,
                                       THERMAL_ADI_POWER_SLEEP);
    if (status == HAL_OK)
    {
        _thermal_adiData.IsDataValid = 0;
    }
    return status;
}

/**
 * @brief Reads all 64 AMG88xx pixels and updates the thermal data structure.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef thermal_adi_Read(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t raw[THERMAL_ADI_PIXEL_BYTES];
    float grid[8][8];
    uint8_t row;
    uint8_t column;

    if (hi2c == NULL)
    {
        _thermal_adiData.IsDataValid = 0;
        return HAL_ERROR;
    }

    status = HAL_I2C_Mem_Read(hi2c, THERMAL_ADI_I2C_ADDRESS,
                              THERMAL_ADI_REG_PIXEL_BASE, I2C_MEMADD_SIZE_8BIT,
                              raw, sizeof(raw), HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        _thermal_adiData.IsDataValid = 0;
        return status;
    }

    for (row = 0; row < 8; ++row)
    {
        for (column = 0; column < 8; ++column)
        {
            uint16_t pixel = (uint16_t)row * 8U + column;
            grid[row][column] = thermal_adi_DecodePixel(raw[pixel * 2U],
                                                        raw[pixel * 2U + 1U]);
        }
    }

    memcpy(_thermal_adiData.DataGrid, grid, sizeof(grid));
    _thermal_adiData.IsDataValid = 1;
    return HAL_OK;
}

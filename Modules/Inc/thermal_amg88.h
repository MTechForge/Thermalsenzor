/*
 * thermal_amg88.h
 *
 * AMG88xx Panasonic 8x8 infrared thermopile array driver.
 */

#ifndef INC_THERMAL_AMG88_H_
#define INC_THERMAL_AMG88_H_

#include "main.h"

/**
 * @brief Stores the latest 8x8 AMG88xx thermal image.
 */
typedef struct
{
    int8_t IsDataValid; /**< Non-zero when DataGrid contains a successful reading. */
    float DataGrid[8][8]; /**< Pixel temperatures in degrees Celsius. */
} thermal_amg88_t;

extern thermal_amg88_t _thermal_amg88Data;

/**
 * @brief Detects and initializes an AMG88xx device.
 *
 * The device is reset, configured for 10 frames per second, and left in
 * sleep mode after successful initialization.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return 1 when the sensor is detected and initialized, otherwise 0.
 */
int8_t hermal_amg88_Is(I2C_HandleTypeDef *hi2c);

/**
 * @brief Resets and configures the AMG88xx sensor.
 *
 * The sensor is deliberately placed in sleep mode at the end of this call.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef hermal_amg88_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Starts AMG88xx temperature conversion.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef hermal_amg88_On(I2C_HandleTypeDef *hi2c);

/**
 * @brief Stops conversion and places the AMG88xx in sleep mode.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef hermal_amg88_Off(I2C_HandleTypeDef *hi2c);

/**
 * @brief Reads all 64 AMG88xx pixels into the global thermal data structure.
 *
 * Each pixel is decoded as a signed 12-bit value with a resolution of
 * 0.25 degrees Celsius. DataGrid is updated only after the complete I2C
 * transaction and all decoded values have been received successfully.
 *
 * @param hi2c Pointer to the STM32 HAL I2C peripheral used by the sensor.
 * @return HAL_OK on success or the HAL error status otherwise.
 */
HAL_StatusTypeDef hermal_amg88_Read(I2C_HandleTypeDef *hi2c);

#endif /* INC_THERMAL_AMG88_H_ */

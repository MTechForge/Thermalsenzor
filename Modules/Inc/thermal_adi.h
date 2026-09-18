/*
 * thermal_adi.h
 *
 *  Created on: 18. 9. 2026
 *      Author: Milan
 */

#ifndef INC_THERMAL_ADI_H_
#define INC_THERMAL_ADI_H_

#include "main.h"
typedef struct
{
    int8_t IsDataValid; /**< 1  values are valid; 0 last read failed or sensor not ready */
    float DataGrid[8][8];	// data - 2 dim arrays of result
} thermal_adi_t;

extern thermal_adi_t _thermal_adiData;

int8_t hermal_adi_Is(I2C_HandleTypeDef *hi2c, int8_t tryInit);

HAL_StatusTypeDef hermal_adi_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef hermal_adi_On(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef hermal_adi_Off(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef hermal_adi_Read(I2C_HandleTypeDef *hi2c);


#endif /* INC_THERMAL_ADI_H_ */

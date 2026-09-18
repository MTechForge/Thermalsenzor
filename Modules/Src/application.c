/*
 * application.c
 *
 *  Created on: 18. 9. 2026
 *      Author: Milan
 */

#include "application.h"
#include "logging.h"

void application_init()
{

}

void application_loop()
{

}



void i2c_scan(I2C_HandleTypeDef *hi2c) //
{
	HAL_StatusTypeDef res;
	int8_t found = 0;

	HAL_Delay(1500);
	log_write("Scanning I2C bus...");
	for (uint8_t addr = 1; addr < 127; addr++) //
	{
		res = HAL_I2C_IsDeviceReady(hi2c, addr << 1, 2, 2);
		if (res == HAL_OK) //
		{
			log_write("Found I2C device at 0x%02X", addr);
			found++;
			HAL_Delay(100);
		}
	}
	if (!found)
		log_write("no I2C devices");
	else
		HAL_Delay(1500);
}

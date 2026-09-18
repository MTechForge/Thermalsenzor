/*
 * application.c
 *
 *  Created on: 18. 9. 2026
 *      Author: Milan
 */

#include "application.h"
#include "logging.h"
#include "thermal_amg88.h"
#include "utils.h"

static I2C_HandleTypeDef *_hi2c;
static sleeper_t _thermReader;

static void i2c_scan() //
{
    HAL_StatusTypeDef res;
    int8_t found = 0;

    HAL_Delay(1500);
    log_write("Scanning I2C bus...");
    for (uint8_t addr = 1; addr < 127; addr++) //
    {
        res = HAL_I2C_IsDeviceReady(_hi2c, addr << 1, 2, 2);
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

void application_init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    _hi2c = hi2c;
    i2c_scan();
    status = thermal_amg88_Init(_hi2c);
    log_write("thermal_amg88 Init:%s", ((status == HAL_OK) ? "OK" : "FAILED"));
    if (status == HAL_OK)
    {
        status = thermal_amg88_On(_hi2c);
        log_write("thermal_amg88 ON:%s", ((status == HAL_OK) ? "OK" : "FAILED"));

    }
    sleeper_Init(&_thermReader, 1000);
}

void application_loop()
{
    // reading data
    HAL_StatusTypeDef status;
    static int n = 0;

    if (sleeper_IsElapsedNext(&_thermReader))
    {
        status = thermal_amg88_Read(_hi2c);
        log_write("thermal_amg88 read:%05d %s dataReady:%d", n++, ((status == HAL_OK) ? "OK" : "FAILED"), (int)_thermal_amg88Data.IsDataValid);
        thermal_logData();


    }


}





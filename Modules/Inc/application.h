/*
 * application.h
 *
 *  Created on: 18. 9. 2026
 *      Author: Milan
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief One-time application startup: initializes and wires together all modules.
 * Must be called once, after HAL/peripheral init, before application_loop().
 */
void application_init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Application main-loop tick: drives the scheduler and the communication protocol.
 * Must be called repeatedly (e.g. from Core/Src/main.c's superloop).
 */
void application_loop();


#ifdef __cplusplus
}
#endif

#endif /* INC_APPLICATION_H_ */

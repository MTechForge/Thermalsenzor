/*
 * logging.h
 *
 *  Created on: 18. 9. 2026
 *      Author: Milan
 *
 *  The logging module on UART
 */

#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_

// maximim log data to send via UART - !!! do no exceed, can cause system crash !!!
#define MAX_LOG_DATA 200

#include "main.h"

/**
 * @brief Initialization of logging module
 * @param logUart
 */
void log_init(UART_HandleTypeDef* logUart);


/**
 * @brief variadic parameter logging function
 * !!! do not exceed MAX_LOG_DATA buffer !!!
 */
void log_write(const char *format, ...);


#endif /* INC_LOGGING_H_ */

/*
 * logging.c
 *
 *  Created on: 18. 9. 2026
 *      Author: Milan
 */


#include "logging.h"
#include "usart.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef* _logUart = NULL;
char _buf[MAX_LOG_DATA];


static void log_writeVA(const char *format, uint8_t addNL, va_list argList)
{
	// misra complies
	if (_logUart != NULL)
		if (_logUart->Instance != NULL)
		{
			vsprintf(_buf, format, argList);
			if (addNL)
			{
	            int len = strlen(_buf);
                if (len > 0 && _buf[len - 1] != '\n' && _buf[len - 2] != '\r')
                    strcat(_buf, "\r\n");
			}
			HAL_UART_Transmit(_logUart, (const uint8_t*)_buf, strlen(_buf), 100);
		}
}


void log_init(UART_HandleTypeDef* logUart)
{
	_logUart = logUart;
}


void log_writeRaw(const char *format, ...) //
{
    va_list argList;
    va_start(argList, format);
    log_writeVA(format, 0, argList);
    va_end(argList);
}


void log_write(const char *format, ...) //
{
	va_list argList;
	va_start(argList, format);
	log_writeVA(format, 1, argList);
	va_end(argList);
}

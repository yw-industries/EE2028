/*
 * uart.h
 *
 *  Created on: Nov 1, 2026
 *      Author: yw
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l4xx_hal.h" // Replace with stm32f4xx_hal.h or matching target variant if needed
#include <string.h>

extern UART_HandleTypeDef huart1;

void UART1_Init(void);
void UART_Print(char *message);

#endif /* INC_UART_H_ */

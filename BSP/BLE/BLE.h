/*
 * BLE.h
 *
 *  Created on: Apr 27, 2023
 *      Author: SOFIEN ZEKRI
 */
#include "stdbool.h"



#ifndef SRC_BLE_H_
#define SRC_BLE_H_

void HM10_AT_command(char* command); //function to config the module with AT commands

void send_data(uint8_t * send_data_buffer , int *flag,int* end_history); //function to send data when connected

void BLE_Init(void);
//void BLE_UART_RxCpltCallback(UART_HandleTypeDef * huart);
void History_Range(uint32_t * start_epoch, uint32_t  * end_epoch);

#endif /* SRC_BLE_H_ */

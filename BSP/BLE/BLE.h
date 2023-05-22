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
bool connected(); //function to check if the module is connected or not
void send_data(uint8_t * send_data_buffer); //function to send data when connected
void BLE_Init(void);

#endif /* SRC_BLE_H_ */

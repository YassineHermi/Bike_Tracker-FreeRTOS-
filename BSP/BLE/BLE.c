/*
 * BLE.c
 *
 *  Created on: Apr 27, 2023
 *      Author: SOFIEN ZEKRI
 */
#define BLE_STATE_GPIO_PIN GPIO_PIN_0

#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "main.h"
#include "BLE.h"

UART_HandleTypeDef huart1;
//UART_HandleTypeDef huart2;
GPIO_InitTypeDef GPIO_InitStruct = {0};
uint8_t buff[25];  // buff de la fct HM10_AT_command
char response[30]; // response de la fct HM10_AT_command
uint8_t send_data_test_buffer[13]=""; //to test if the new data != old data

// Private function --------------------------------------------------------------/
static void MX_USART1_UART_Init(void);
static void BLE_State_Pin_Init(void);

//function to config the module with AT commands
void HM10_AT_command(char* command)
{

	// send command to HM10 module
    HAL_UART_Transmit(&huart1, (uint8_t*)command, strlen((char*)command), 100);
    // receive response from HM10 module
    HAL_UART_Receive(&huart1, buff, sizeof(buff), 100);
    // show response on terminal
    sprintf(response, "%s\n\r", buff);
    //HAL_UART_Transmit(&huart2, (uint8_t*)response, strlen(response), 100);
    // clear command buffer
    if (command != NULL) memset(command, 0, strlen (command));
}


bool connected()
{
	return HAL_GPIO_ReadPin(GPIOD,BLE_STATE_GPIO_PIN);
}

//function to send data when connected
void send_data(uint8_t * send_data_buffer)
{
	 if(strcmp((char *)send_data_test_buffer,(char *)send_data_buffer)!=0)
	    {
	        HAL_UART_Transmit(&huart1, send_data_buffer, 13, 100);
	        strcpy((char *)send_data_test_buffer,(char *)send_data_buffer);
	    }
}

//function to config usart1
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);
}

//function to config usart2 for the terminal
/*static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);
}*/


static void BLE_State_Pin_Init(void)
{
  /*Configure BLE STATE GPIO PIN (PD0) */
  GPIO_InitStruct.Pin = BLE_STATE_GPIO_PIN ;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void BLE_Init(void)
{
	MX_USART1_UART_Init();
	//MX_USART2_UART_Init();
	BLE_State_Pin_Init();
}

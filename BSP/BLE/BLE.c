/*
 * BLE.c
 *
 *  Created on: Apr 27, 2023
 *      Author: SOFIEN ZEKRI
 */
#define BLE_STATE_GPIO_PIN GPIO_PIN_0

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "main.h"
#include "BLE.h"

UART_HandleTypeDef huart1;

int History_Flag=0; // history request flag
uint8_t buffer[20];
//UART_HandleTypeDef huart2;
GPIO_InitTypeDef GPIO_InitStruct = {0};
uint8_t buff[25];  // buff de la fct HM10_AT_command
char response[30]; // response de la fct HM10_AT_command
uint8_t send_data_test_buffer[14]=""; //to test if the new data != old data
//uint8_t buffer2[20]="";
//uint8_t buffer3[14];
uint8_t epoch1[10];
uint8_t epoch2[10];
char int_epoch1[10];
char hexa_epoch1[8];
char int_epoch2[10];
char hexa_epoch2[8];
int x1,x2;
uint32_t epoch11_value;
uint32_t epoch22_value;
int cmp=0;


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




//function to send data when connected
void send_data(uint8_t * send_data_buffer, int *flag,int*end_history)
{
	*flag = History_Flag;
	// Niveau de charge de la batterie :

	 /* led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
	  led2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8);
	  led3 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
	  led4 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);

	  if (led4==1)
		  niv=100;
	  else if ((led4==0)&&(led3==1))
		  niv=75;
	  else if ((led3==0)&&(led2==1))
		  niv=50;
	  else if ((led2==0)&&(led1==1))
		  niv=25;
	  else
		  niv=0;*/

	if(strcmp((char *)send_data_test_buffer,(char *)send_data_buffer)!=0)
	    {
			/*strcat((char*)buffer3,(char*)send_data_buffer);
			strcat((char*)buffer3,(char*)niv);*/
		    HAL_UART_Transmit(&huart1, send_data_buffer, 14, 100);
		    strcpy((char *)send_data_test_buffer,(char *)send_data_buffer);
	    }


	HAL_Delay(8);
	huart1.pRxBuffPtr = (uint8_t *)buffer;
	huart1.RxXferCount = sizeof(buffer);

	if (*end_history==1)
	{
		History_Flag = 0;

	}
	/*if (History_Flag==0)
		*end_history=0;*/


}

void History_Range(uint32_t * start_epoch, uint32_t  * end_epoch)
{

if (buffer[0] == '\001')
	        	 {
	        		  memcpy((char*)epoch1, buffer, 10);
	        		  sprintf(int_epoch1,"%d%d%d%d%d%d%d%d%d%d",epoch1[0],epoch1[1],epoch1[2],epoch1[3],epoch1[4],epoch1[5],epoch1[6],epoch1[7],epoch1[8],epoch1[9]);
	        		 x1=strtol(int_epoch1, NULL, 10);
	        		  sprintf(hexa_epoch1,"%x",x1);
	                          epoch11_value = strtoul(hexa_epoch1,NULL,16);

	        		      memcpy((char*)epoch2, (buffer) + 10, 10);
	        		      sprintf(int_epoch2,"%d%d%d%d%d%d%d%d%d%d",epoch2[0],epoch2[1],epoch2[2],epoch2[3],epoch2[4],epoch2[5],epoch2[6],epoch2[7],epoch2[8],epoch2[9]);
	        		      x2 = strtol(int_epoch2, NULL, 10);
	        		      sprintf(hexa_epoch2,"%x",x2);
	                              epoch22_value = strtoul(hexa_epoch2,NULL,16);
	              		   //flag=0;
	                              *start_epoch = epoch11_value;
	                              *end_epoch = epoch22_value;


	        	  }
	    	/*  else
	        	  {
	                  HAL_Delay(50);
	        		  huart1.pRxBuffPtr = (uint8_t *)buffer;
	        		  huart1.RxXferCount = sizeof(buffer);
	        	  }*/

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
	//MX_DMA_Init();
	//MX_USART2_UART_Init();
	BLE_State_Pin_Init();
	HAL_UART_Receive_DMA(&huart1,(uint8_t *) buffer, 20);
}

/*void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{

	HAL_UART_Receive_IT(&huart1, (uint8_t*)buffer,20);
   // cmp++;
  if ((buffer[0]=='\001')&&(strcmp((char*)buffer2,(char*)buffer) != 0))
  {

      History_Flag=1;
      memcpy((char*)buffer2, buffer, 20);
  }

  if ((buffer[0]=='\005')&&(buffer[19]=='\005'))
		History_Flag=0;



}*/
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{

		HAL_UART_Receive_DMA(&huart1, (uint8_t*)buffer,20);
	    cmp++;
	  if ((buffer[0]=='\001'))
	  {

	      History_Flag=1;
	  }
	  if ((buffer[0]=='\005')&&(buffer[19]=='\005'))
			History_Flag=0;

}



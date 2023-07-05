/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "BLE.h"
#include "GPS.h"
#include "Memory.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* Definitions for GPS_Get_Data */
osThreadId_t GPS_Get_DataHandle;
const osThreadAttr_t GPS_Get_Data_attributes = {
  .name = "GPS_Get_Data",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for Mem_Access_Data */
osThreadId_t Mem_Access_DataHandle;
const osThreadAttr_t Mem_Access_Data_attributes = {
  .name = "Mem_Access_Data",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for BLE_Send_Data */
osThreadId_t BLE_Send_DataHandle;
const osThreadAttr_t BLE_Send_Data_attributes = {
  .name = "BLE_Send_Data",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal7,
};
/* Definitions for Bike_state */
osThreadId_t Bike_stateHandle;
const osThreadAttr_t Bike_state_attributes = {
  .name = "Bike_state",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};
/* Definitions for myQueue02 */
osMessageQueueId_t myQueue02Handle;
const osMessageQueueAttr_t myQueue02_attributes = {
  .name = "myQueue02"
};
/* Definitions for myQueue03 */
osMessageQueueId_t myQueue03Handle;
const osMessageQueueAttr_t myQueue03_attributes = {
  .name = "myQueue03"
};
/* Definitions for Semaphore1 */
osSemaphoreId_t Semaphore1Handle;
const osSemaphoreAttr_t Semaphore1_attributes = {
  .name = "Semaphore1"
};
/* Definitions for Semaphore2 */
osSemaphoreId_t Semaphore2Handle;
const osSemaphoreAttr_t Semaphore2_attributes = {
  .name = "Semaphore2"
};
/* Definitions for Semaphore3 */
osSemaphoreId_t Semaphore3Handle;
const osSemaphoreAttr_t Semaphore3_attributes = {
  .name = "Semaphore3"
};
/* Definitions for Semaphore0 */
osSemaphoreId_t Semaphore0Handle;
const osSemaphoreAttr_t Semaphore0_attributes = {
  .name = "Semaphore0"
};
/* USER CODE BEGIN PV */

DMA_HandleTypeDef hdma_usart1_rx;
uint8_t GPS_Data[14];
uint8_t BLE_Data[14];
uint8_t memory_data[14];
int Data_size = 16;
uint8_t buffer_queue1[14];
uint8_t buffer_queue2[14];
uint8_t buffer_queue3[14];
uint8_t size;
int bouton=0;
int Temps_vit_null=0;
uint8_t compt=0;
bool flg_hist=false;
int history_flag=0;
int End_History = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
void StartGPS_Get_Data(void *argument);
void StartMem_Access_Data(void *argument);
void StartBLE_Send_Data(void *argument);
void StartBike_state(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
  {
  	if (GPIO_Pin == GPIO_PIN_2)
  		{
  		 if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET)
  		   bouton = 1;
  		 else
  		   bouton = 0;

  		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  		}

  		//compt++;
  }

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  /* USER CODE BEGIN 2 */
  BLE_Init();
  GPS_Init();
  Memory_Init();


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of Semaphore1 */
  Semaphore1Handle = osSemaphoreNew(1, 0, &Semaphore1_attributes);

  /* creation of Semaphore2 */
  Semaphore2Handle = osSemaphoreNew(1, 0, &Semaphore2_attributes);

  /* creation of Semaphore3 */
  Semaphore3Handle = osSemaphoreNew(1, 0, &Semaphore3_attributes);

  /* creation of Semaphore0 */
  Semaphore0Handle = osSemaphoreNew(1, 1, &Semaphore0_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (14, sizeof(uint8_t), &myQueue01_attributes);

  /* creation of myQueue02 */
  myQueue02Handle = osMessageQueueNew (14, sizeof(uint8_t), &myQueue02_attributes);

  /* creation of myQueue03 */
  myQueue03Handle = osMessageQueueNew (14, sizeof(uint8_t), &myQueue03_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of GPS_Get_Data */
  GPS_Get_DataHandle = osThreadNew(StartGPS_Get_Data, NULL, &GPS_Get_Data_attributes);

  /* creation of Mem_Access_Data */
  Mem_Access_DataHandle = osThreadNew(StartMem_Access_Data, NULL, &Mem_Access_Data_attributes);

  /* creation of BLE_Send_Data */
  BLE_Send_DataHandle = osThreadNew(StartBLE_Send_Data, NULL, &BLE_Send_Data_attributes);

  /* creation of Bike_state */
  Bike_stateHandle = osThreadNew(StartBike_state, NULL, &Bike_state_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA3 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PE8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartGPS_Get_Data */
/**
  * @brief  Function implementing the GPS_Get_Data thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartGPS_Get_Data */
void StartGPS_Get_Data(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {

	  osSemaphoreAcquire(Semaphore1Handle, osWaitForever);

	  if (history_flag==0)
	  {

		  Get_Data(GPS_Data, BLE_Data,&Temps_vit_null);

		  for (int j=0;j<14;j++)

			  {
				osMessageQueuePut(myQueue01Handle, &(GPS_Data[j]), sizeof(GPS_Data[j]), 100);
				osMessageQueuePut(myQueue02Handle, &(BLE_Data[j]), sizeof(BLE_Data[j]), 100);
			  }

	  }
	  osSemaphoreRelease(Semaphore2Handle);

  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartMem_Access_Data */
/**
* @brief Function implementing the Mem_Access_Data thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMem_Access_Data */
void StartMem_Access_Data(void *argument)
{
  /* USER CODE BEGIN StartMem_Access_Data */
  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(Semaphore2Handle, osWaitForever);


	      if (history_flag==0)
	      {

			  for (int k=0;k<14;k++)

					  {
						osMessageQueueGet(myQueue01Handle, &(buffer_queue1[k]), &size, 100);
					  }

			  Store_Data(buffer_queue1,Data_size);

	      }
	      else
	      {

	    	  if(End_History==0)
	    	     get_history(memory_data,Data_size,&flg_hist);

	    	  for (int j=0;j<14;j++)

	    	  		  {
	    	  		    osMessageQueuePut(myQueue03Handle, &(memory_data[j]), sizeof(memory_data[j]), 100);
	    	  		  }

	      }

	      End_History=0;
	 	  osSemaphoreRelease(Semaphore3Handle);
  }
  /* USER CODE END StartMem_Access_Data */
}

/* USER CODE BEGIN Header_StartBLE_Send_Data */
/**
* @brief Function implementing the BLE_Send_Data thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBLE_Send_Data */
void StartBLE_Send_Data(void *argument)
{
  /* USER CODE BEGIN StartBLE_Send_Data */
  /* Infinite loop */
  for(;;)
  {
	      osSemaphoreAcquire(Semaphore3Handle, osWaitForever);

	      if (flg_hist==true)
	      		{

	      		    End_History=1;
	      		    history_flag=0;
	      		    flg_hist = false;

	      		 }

	      if (history_flag==0)
	      {
			  for (int l=0;l<14;l++)

					  {
						osMessageQueueGet(myQueue02Handle, &(buffer_queue2[l]), &size, 100);
					  }

			  //send_data(buffer2);
			  send_data(buffer_queue2,&history_flag,&End_History);
	      }
	      else
	      {
	    	  for (int l=0;l<14;l++)

	    	  	  {
	    	  	  	osMessageQueueGet(myQueue03Handle, &(buffer_queue3[l]), &size, 100);
	    	  	  }

	    	  //send_data(buffer2);
	    	  send_data(buffer_queue3,&history_flag,&End_History);
	      }


	  	  osSemaphoreRelease(Semaphore0Handle);
  }
  /* USER CODE END StartBLE_Send_Data */
}

/* USER CODE BEGIN Header_StartBike_state */
/**
* @brief Function implementing the Bike_state thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartBike_state */
void StartBike_state(void *argument)
{
  /* USER CODE BEGIN StartBike_state */
  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(Semaphore0Handle, osWaitForever);

	  if ((bouton==0) && (Temps_vit_null>=27))
		  {
		      HAL_SuspendTick();
		      HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4);
		      HAL_PWR_EnterSTANDBYMode();

		  }
	  else
		  {
              HAL_ResumeTick();
              if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET)
               		   bouton = 1;
		  }

	  osSemaphoreRelease(Semaphore1Handle);

  }
  /* USER CODE END StartBike_state */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

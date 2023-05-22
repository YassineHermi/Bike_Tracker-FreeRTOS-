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
/* Definitions for Mem_Store_Data */
osThreadId_t Mem_Store_DataHandle;
const osThreadAttr_t Mem_Store_Data_attributes = {
  .name = "Mem_Store_Data",
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
/* Definitions for Semaphore1 */
osSemaphoreId_t Semaphore1Handle;
const osSemaphoreAttr_t Semaphore1_attributes = {
  .name = "Semaphore1"
};
/* Definitions for semaphore2 */
osSemaphoreId_t Semaphore2Handle;
const osSemaphoreAttr_t Semaphore2_attributes = {
  .name = "semaphore2"
};
/* Definitions for Semaphore3 */
osSemaphoreId_t Semaphore3Handle;
const osSemaphoreAttr_t Semaphore3_attributes = {
  .name = "Semaphore3"
};

/* USER CODE BEGIN PV */

uint8_t GPS_Data[13];
uint8_t BLE_Data[13];
int Data_size = 16;
uint8_t buffer[13];
uint8_t buffer2[13];
uint8_t size;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void GPIO_Init(void);
void StartGPS_Get_Data(void *argument);
void StartMem_Store_Data(void *argument);
void StartBLE_Send_Data(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


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
  GPIO_Init();
  BLE_Init();
  /* USER CODE BEGIN 2 */
  GPS_Init();
  Memory_Init();

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Create the semaphores(s) */
  /* creation of Semaphore1 */
  Semaphore1Handle = osSemaphoreNew(1, 0, &Semaphore1_attributes);

  /* creation of semaphore2 */
  Semaphore2Handle = osSemaphoreNew(1, 0, &Semaphore2_attributes);

  /* creation of Semaphore3 */
  Semaphore3Handle = osSemaphoreNew(1, 1, &Semaphore3_attributes);



  /* Create the queue(s) */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (13, sizeof(uint8_t), &myQueue01_attributes);

  /* creation of myQueue02 */
  myQueue02Handle = osMessageQueueNew (13, sizeof(uint8_t), &myQueue02_attributes);


  /* Create the thread(s) */
  /* creation of GPS_Get_Data */
  GPS_Get_DataHandle = osThreadNew(StartGPS_Get_Data, NULL, &GPS_Get_Data_attributes);

  /* creation of Mem_Store_Data */
  Mem_Store_DataHandle = osThreadNew(StartMem_Store_Data, NULL, &Mem_Store_Data_attributes);

  /* creation of BLE_Send_Data */
  BLE_Send_DataHandle = osThreadNew(StartBLE_Send_Data, NULL, &BLE_Send_Data_attributes);

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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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

	  osSemaphoreAcquire(Semaphore3Handle, osWaitForever);

	  Get_Data(GPS_Data, BLE_Data);
	  for (int j=0;j<13;j++)

		  {
		    osMessageQueuePut(myQueue01Handle, &(GPS_Data[j]), sizeof(GPS_Data[j]), 100);
		    osMessageQueuePut(myQueue02Handle, &(BLE_Data[j]), sizeof(BLE_Data[j]), 100);
		  }

	  osSemaphoreRelease(Semaphore1Handle);

  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartMem_Store_Data */
/**
* @brief Function implementing the Mem_Store_Data thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMem_Store_Data */
void StartMem_Store_Data(void *argument)
{
  /* USER CODE BEGIN StartMem_Store_Data */
  /* Infinite loop */
  for(;;)
  {
	  osSemaphoreAcquire(Semaphore1Handle, osWaitForever);

	  for (int k=0;k<13;k++)

	  		  {
	  		    osMessageQueueGet(myQueue01Handle, &(buffer[k]), &size, 100);
	  		  }

	  Store_Data(buffer,Data_size);

	  osSemaphoreRelease(Semaphore2Handle);
  }
  /* USER CODE END StartMem_Store_Data */
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
	      osSemaphoreAcquire(Semaphore2Handle, osWaitForever);

	      for (int l=0;l<13;l++)

	  	  		  {
	  	  		    osMessageQueueGet(myQueue02Handle, &(buffer2[l]), &size, 100);
	  	  		  }

	  	  send_data(buffer2);

	  	  osSemaphoreRelease(Semaphore3Handle);
  }
  /* USER CODE END StartBLE_Send_Data */
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

/* Includes ------------------------------------------------------------------*/

#include "Memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "stm32l4xx_hal.h"
#include "GPS.h"

/* Private variables ---------------------------------------------------------*/

QSPI_HandleTypeDef QSPIHandle;
int compteur_final = 20223;
bool a=false;
uint8_t store=0;
int comp=-1;
char hex[9];
char hexinv[9];
char hex2[9];
char hex2inv[9];
uint32_t min;
uint32_t value;
uint32_t value2;
uint32_t hexa_value;

/* Private functions ---------------------------------------------------------*/

static void    QSPI_MspInit(void);
static void    QSPI_MspDeInit(void);
static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout);
static uint32_t Hexadecimal(int nombre);
static bool Est_Vide(uint32_t ADDR, uint32_t Size);
static bool Mem_pleine(int Size);
static void inversion_hex(char hex[8], char hexinv[8]);
static void Write_Data(uint8_t *pData, int Size);
static void Erase_Subsector(int Size);

/* Exported functions --------------------------------------------------------*/

/*
 * @brief  Initializes the QSPI interface.
 * @retval QSPI memory status
*/

void Memory_Init(void)
{
  QSPIHandle.Instance = QUADSPI;

  /* Call the DeInit function to reset the driver */
  HAL_QSPI_DeInit(&QSPIHandle);

  /* System level initialization */
  QSPI_MspInit();

  /* QSPI initialization */
  QSPIHandle.Init.ClockPrescaler     = 1; /* QSPI clock = 80MHz / (ClockPrescaler+1) = 40MHz */
  QSPIHandle.Init.FifoThreshold      = 4;
  QSPIHandle.Init.FlashSize = 23;
  QSPIHandle.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  QSPIHandle.Init.FlashSize          = POSITION_VAL(N25Q128A_FLASH_SIZE) - 1;
  QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  QSPIHandle.Init.ClockMode          = QSPI_CLOCK_MODE_0;

  HAL_QSPI_Init(&QSPIHandle);

  /* QSPI memory reset */
  QSPI_ResetMemory(&QSPIHandle);

  /* Configuration of the dummy cucles on QSPI memory side */
  QSPI_DummyCyclesCfg(&QSPIHandle);

}

/**
  * @brief  De-Initializes the QSPI interface.
  * @retval QSPI memory status
  */
void BSP_QSPI_DeInit(void)
{
  QSPIHandle.Instance = QUADSPI;

  /* Call the DeInit function to reset the driver */
  HAL_QSPI_DeInit(&QSPIHandle);

  /* System level De-initialization */
  QSPI_MspDeInit();

}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData: Pointer to data to be read
  * @param  ReadAddr: Read start address
  * @param  Size: Size of data to read
  * @retval QSPI memory status
  */
void BSP_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef sCommand;

  /* Initialize the read command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = QUAD_INOUT_FAST_READ_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.Address           = ReadAddr;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_4_LINES;
  sCommand.DummyCycles       = N25Q128A_DUMMY_CYCLES_READ_QUAD;
  sCommand.NbData            = Size;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

  /* Reception of the data */
  HAL_QSPI_Receive(&QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

  /* Data is read ==> Flag=0 */
  //Reset_Flag(ReadAddr);

}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData: Pointer to data to be written
  * @param  WriteAddr: Write start address
  * @param  Size: Size of data to write
  * @retval QSPI memory status
  */
void BSP_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
  QSPI_CommandTypeDef sCommand;
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = N25Q128A_PAGE_SIZE - (WriteAddr % N25Q128A_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the address variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;

  /* Initialize the program command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = EXT_QUAD_IN_FAST_PROG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_4_LINES;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Perform the write page by page */
  do
  {
    sCommand.Address = current_addr;
    sCommand.NbData  = current_size;

    /* Enable write operations */
    QSPI_WriteEnable(&QSPIHandle);

    /* Configure the command */
    HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

    /* Transmission of the data */
    HAL_QSPI_Transmit(&QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

    /* Configure automatic polling mode to wait for end of program */
    QSPI_AutoPollingMemReady(&QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + N25Q128A_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : N25Q128A_PAGE_SIZE;
  }
  while (current_addr < end_addr);

}
/**
  * @brief  Stores an amount of data to the QSPI memory.
  * @param  pData: Pointer to data to be stored
  * @param  Size: Size of data to store
 */
void Store_Data(uint8_t *pData, int Size)
{
	if (store==1)
	{

		if (Mem_pleine(Size))
				{
				  Erase_Subsector(Size);
				}
		else
				{
				  Write_Data(pData,Size);
				}

		// Reset du compteur :
		if ((comp==compteur_final)&&(Est_Vide(Hexadecimal(comp*Size),Size)==false))
				{
				   comp=0;
				}
	}
}


/**
  * @brief  Erases the specified block of the QSPI memory.
  * @param  BlockAddress: Block address to erase
  * @retval QSPI memory status
  */
void BSP_QSPI_Erase_Block(uint32_t BlockAddress)
{
  QSPI_CommandTypeDef sCommand;

  /* Initialize the erase command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = SUBSECTOR_ERASE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.Address           = BlockAddress;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  QSPI_WriteEnable(&QSPIHandle);

  /* Send the command */
  HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

  /* Configure automatic polling mode to wait for end of erase */
  QSPI_AutoPollingMemReady(&QSPIHandle, N25Q128A_SUBSECTOR_ERASE_MAX_TIME);

}

/**
  * @brief  Erases the specified sector of the QSPI memory.
  * @param  Sector: Sector address to erase (0 to 255)
  * @retval QSPI memory status
  * @note This function is non blocking meaning that sector erase
  *       operation is started but not completed when the function
  *       returns. Application has to call BSP_QSPI_GetStatus()
  *       to know when the device is available again (i.e. erase operation
  *       completed).
  */
void BSP_QSPI_Erase_Sector(uint32_t Sector)
{
  QSPI_CommandTypeDef sCommand;

  /* Initialize the erase command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = SECTOR_ERASE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
  sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
  sCommand.Address           = (Sector * N25Q128A_SECTOR_SIZE);
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  QSPI_WriteEnable(&QSPIHandle);

  /* Send the command */
  HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

}

/**
  * @brief  Erases the entire QSPI memory.
  * @retval QSPI memory status
  */
void BSP_QSPI_Erase_Chip(void)
{
  QSPI_CommandTypeDef sCommand;

  /* Initialize the erase command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = BULK_ERASE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  QSPI_WriteEnable(&QSPIHandle);

  /* Send the command */
  HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);

  /* Configure automatic polling mode to wait for end of erase */
  QSPI_AutoPollingMemReady(&QSPIHandle, N25Q128A_BULK_ERASE_MAX_TIME);

}

/* Private functions ---------------------------------------------------------------------*/

/**
  * @brief  Initializes the QSPI MSP.
  * @retval None
  */
static void QSPI_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the QuadSPI memory interface clock */
  __HAL_RCC_QSPI_CLK_ENABLE();

  /* Reset the QuadSPI memory interface */
  __HAL_RCC_QSPI_FORCE_RESET();
  __HAL_RCC_QSPI_RELEASE_RESET();

  /* Enable GPIO clocks */
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* QSPI CS GPIO pin configuration  */
  GPIO_InitStruct.Pin       = GPIO_PIN_11;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* QSPI CLK, D0, D1, D2 and D3 GPIO pins configuration  */
  GPIO_InitStruct.Pin       = (GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
  * @brief  De-Initializes the QSPI MSP.
  * @retval None
  */
static void QSPI_MspDeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* QSPI CLK, CS, PE10 - PE15 GPIO pins de-configuration  */

  __HAL_RCC_GPIOE_CLK_ENABLE();
  HAL_GPIO_DeInit(GPIOE, (GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15));
  /* Set GPIOE pin 11 in pull up mode (optimum default setting) */
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pin       = GPIO_PIN_11;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* Set GPIOE pin 10 in no pull, low state (optimum default setting) */
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP  ;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Pin       = (GPIO_PIN_10);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);

  /* Reset the QuadSPI memory interface */
  __HAL_RCC_QSPI_FORCE_RESET();
  __HAL_RCC_QSPI_RELEASE_RESET();

  /* Disable the QuadSPI memory interface clock */
  __HAL_RCC_QSPI_CLK_DISABLE();
}

/**
  * @brief  This function reset the QSPI memory.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef sCommand;

  /* Initialize the reset enable command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = RESET_ENABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the reset memory command */
  sCommand.Instruction = RESET_MEMORY_CMD;
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(&QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef sCommand;
  uint8_t reg;

  /* Initialize the read volatile configuration register command */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.NbData            = 1;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&QSPIHandle, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Update volatile configuration register (with new dummy cycles) */
  sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
  MODIFY_REG(reg, N25Q128A_VCR_NB_DUMMY, (N25Q128A_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(N25Q128A_VCR_NB_DUMMY)));

  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(&QSPIHandle, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Enable write operations */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = WRITE_ENABLE_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_NONE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  sConfig.Match           = N25Q128A_SR_WREN;
  sConfig.Mask            = N25Q128A_SR_WREN;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  sCommand.Instruction    = READ_STATUS_REG_CMD;
  sCommand.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(&QSPIHandle, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi: QSPI handle
  * @param  Timeout: Timeout for auto-polling
  * @retval None
  */
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout)
{
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Configure automatic polling mode to wait for memory ready */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_STATUS_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  sConfig.Match           = 0;
  sConfig.Mask            = N25Q128A_SR_WIP;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(&QSPIHandle, &sCommand, &sConfig, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
 * convert decimal to hexadecimal
 */
 static uint32_t Hexadecimal(int nombre)
{
	uint32_t Adresse;
	char Hexa[10];
	sprintf(Hexa,"%x",nombre);
    Adresse = strtoul(Hexa,NULL,16);
    return Adresse;
}


/**
 * tester si le block d'adresse ADDR est vide ou nn
 */
static bool Est_Vide(uint32_t ADDR, uint32_t Size)
{
	uint8_t buffer[Size];
	int i=0;
	int nb=0;
	BSP_QSPI_Read(buffer, ADDR, Size);
	while (((int)(buffer[i])== 255)&&(i<Size))
		{
			 nb++;
		     i++;
		}
	if (nb==Size)
		return true;
	else
		return false;

}
/**
 * Tester si la mémoire est pleine ou non
 */

 static bool Mem_pleine(int Size)
{
	int nb_data_per_subsector=(int)(4096/Size);
	int i=nb_data_per_subsector-1;
	while ((i<=compteur_final)&&(Est_Vide(Hexadecimal(i*Size),Size)==false))
		{
		  i = i+ nb_data_per_subsector;
		}
	if (i==(compteur_final+nb_data_per_subsector))
		return true;
	else
		return false;

}

 /**
  * la fonction ci-dessous permet d'inverser tableau de caractères hexadécimale de la façon suivante : "acbe12h5" ==> "h512beac"
  */
static void inversion_hex(char hex[8], char hexinv[8])
 {
     int k=0;
     for(int i=6;i>=0;i=i-2)
     {
         for(int j=i;j<i+2;j++)
         {
             hexinv[k]=hex[j];
             k++;
         }
     }
     hexinv[k]='\0';
 }

/**
 * la fonction ci-dessous permet de chercher une case vide dans la mémoire puis écrire les données
 */
static void Write_Data(uint8_t *pData, int Size)
{
	int nb_data_per_subsector = (int)(4096/Size);
	uint32_t ADDR = Hexadecimal(comp*Size);
	if (Est_Vide(ADDR,Size)==true)
		{
			a=true;
			//BSP_QSPI_Write(pData, ADDR, Size);
		}
		else
		{

			if ((comp%nb_data_per_subsector == 0)&&(a==false))
			{
				if ((Est_Vide(Hexadecimal((comp+(nb_data_per_subsector-1))*Size),Size)==false)&&((comp+(nb_data_per_subsector-1)) < compteur_final))
				{
					comp = comp + nb_data_per_subsector;
				}
				else
				{
					int j;
				    for(j=comp+1;j<comp+nb_data_per_subsector;j++)
					   {
						  if (Est_Vide(Hexadecimal(j*Size),Size))
						  break;
						}
					comp = j;
				}

			}
		}
}
/**
 * la fonction ci dessous permet d'effacer un subsector de la mémoire de façon circulaire
 */
static void Erase_Subsector(int Size)
{

	    int nb_data_per_subsector = (int)(4096/Size); // = 256
	    uint8_t buff1[4], buff2[4];
	    BSP_QSPI_Read(buff1,0,4);
	    sprintf(hex, "%02x%02x%02x%02x", buff1[0], buff1[1], buff1[2], buff1[3]);
	    inversion_hex(hex,hexinv);
	    value = strtoul(hexinv,NULL,16);
	    min = value;
	    int j;
	      	     for(int i =0;i<=(compteur_final-255);i=i+nb_data_per_subsector)
	      	     {
	      	    	 BSP_QSPI_Read(buff2,Hexadecimal(i*16),4);
	      	         sprintf(hex2, "%02x%02x%02x%02x", buff2[0], buff2[1], buff2[2], buff2[3]);
	      	         inversion_hex(hex2,hex2inv);
	      	         value2 = strtoul(hex2inv,NULL,16);
	      	         if (value2 <= min)
	      	         {
	      	             min = value2;
	      	             j=i;
	      	         }
	      	     }
	    BSP_QSPI_Erase_Block(Hexadecimal(j*Size));
	    comp= j;

}
/**
 * la fonction ci-dessous est appellé lorsqu'une donnée est prète à stocker dans la mémoire
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	store=1;
	comp++;
}

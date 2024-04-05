/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Global buffer to store the comming uart frames from host */
static uint8_t BL_HostBuffer[3];
static uint32_t FLAG_APP_OFF = 0xEEEEEEEE ;
static uint8_t Booting = 0xAA ;

/* That is the commends that you use to communicate with bootloader */
typedef enum {
	CBL_GET_VER_CMD                    = 0x10 ,/* Read bootloader version (1 byte)*/
	CBL_GET_RDP_STATUS_CMD             = 0x11 ,/* Read the flash protection level (1 byte) */
	CBL_GO_TO_ADDER_CMD                = 0x12 ,/* Sucess or Error code --> This commend is used to jump on bootloader to specified address (1 byte)*/
	CBL_FLASH_ERASE_CMD                = 0x13 ,/* Sucess or Error code --> This commend is used to miss erase or sector erase of the user flash (1 byte) */
	CBL_MEM_WRITE_CMD                  = 0x14 ,/* Sucess or Error code --> This commend is used to write data in different memories of the MCU (1 byte) */
	CBL_CHANGE_ROP_Level_CMD           = 0x15 ,/* Change Read protection level of flash memory */
	CBL_CHANGE_WOP_Level_CMD           = 0x16  /* Change Write Protection Level */
}COMMENDS;

typedef enum {
	FIRST_COMMEND                      = 0x10 ,
	LAST_COMMEND                       = 0x16 ,
	NumberOfCommends                   = 0x07 ,

	/* the address of application region */
	FLASH_PAGE_BASE_ADDRESS_APP1       = 0x08005000U ,
	FLASH_PAGE_BASE_ADDRESS_APP2       = 0x0800A000U ,
	FLAG_APP1_ADDRESS                  = 0x0800F000U ,
	FLAG_APP2_ADDRESS                  = 0x0800F004U ,

	/* To check that the given address is within range */
	ADDRESS_IS_VALID                   = 1 ,
	ADDRESS_IS_INVALID                 = 0 ,

	APPLICATION_SIZE                   =  19 ,

	/* Related to pages in flash memory */
	CBL_FLASH_MAX_PAGES_NUMBER         =  128 ,
	CBL_PAGE_END                       =  19 ,

	/* The status of Erasing flash memory */
	SUCESSFUL_ERASE                    =       0x03 ,
	UNSUCESSFUL_ERASE                  =       0x02 ,
	PAGE_VALID_NUMBER                  =       0x01 ,
	PAGE_INVALID_NUMBER                =       0x00 ,
	HAL_SUCESSFUL_ERASE                =       0xFFFFFFFF ,

	/* Erase all sectors in application region */
	 CBL_FLASH_MASS_ERASE              =       0xFF ,

	 /* Defines writing in memory status */
	 FLASH_PAYLOAD_WRITING_FAILED      =  0x00 ,
	 FLASH_PAYLOAD_WRITING_PASSED      =  0x01 ,


	 /* definations related to read protection level */
	 RDP_LEVEL_READ_INVALID            =     0x00       ,
	 RDP_LEVEL_READ_VALID              =     0x01       ,

	 /* Change read protection level */
	 ROP_LEVEL_CHANGE_INVALID          =     0x00       ,
	 ROP_LEVEL_CHANGE_VALID            =     0x01       ,

	 /* Change read protection level */
	 WOP_LEVEL_CHANGE_INVALID          =     0x00       ,
	 WOP_LEVEL_CHANGE_VALID            =     0x01       ,

	 CBL_ROP_LEVEL_0                   =     0x00       ,
	 CBL_ROP_LEVEL_1                   =     0x01       ,

	 READ_RP                           =     0x00       ,
	 READ_WP                           =     0x01       ,

	 TURN_ON                           =     0x01       ,
	 TURN_OFF                          =     0x00       ,

	 BL_BEGIN_MATCH                    =     0xAA       ,
	 BL_BEGIN_NOT_MATCH                =     0xEE       ,
	 ONE_BYTE                          =     0x08       ,

	 FLAG_RISED                        =     0xAAAAAAAA
}DEFINATIONS;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void APP_Fetch_Commend(void);
static void ClearApplicationFlag(void);
static uint8_t Perform_Flash_Erase (uint32_t PageAddress, uint8_t Number_Of_Pages);
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_PayLoad,
	uint32_t Payload_Start_Address, uint8_t Payload_Len) ;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void APP_Fetch_Commend(void){
	/* To clear buffer of RX and prevent carbadge messages of buffer */
	memset(BL_HostBuffer, RESET , 3);

	HAL_UART_Receive(&huart1, BL_HostBuffer, 1,
			20);

	if ( BL_HostBuffer[0] == 0xAA ){
		HAL_UART_Transmit(&huart1,(uint8_t *) &Booting,1, HAL_MAX_DELAY);
		ClearApplicationFlag();
		HAL_NVIC_SystemReset();
	}
}

static void ClearApplicationFlag(void)
{
	Perform_Flash_Erase(0x800F000, 0x01);
	Flash_Memory_Write_Payload((uint8_t *) &FLAG_APP_OFF, 0x800F000, 4);
}


/* Get erasing flash actually */
static uint8_t Perform_Flash_Erase (uint32_t PageAddress, uint8_t Number_Of_Pages){
	/* To check that the sectors in not overflow the size of flash */
	uint8_t Page_validity_Status  = PAGE_INVALID_NUMBER ;
	/* Status of erasing flash */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;
	/* Error sector status */
	uint32_t PageError = RESET ;
	/* Define struct to configure parameters[in] */
	FLASH_EraseInitTypeDef pEraseInit ;
	/* Define the used bank in flash memory */
	pEraseInit.Banks = FLASH_BANK_1 ;

	/* another pages is agreed but check that is acess the number of pages in flash */
	if (Number_Of_Pages >= CBL_FLASH_MAX_PAGES_NUMBER && CBL_FLASH_MASS_ERASE != PageAddress){
		Page_validity_Status = PAGE_INVALID_NUMBER ;

	}
	/* erase all memory or specific page */
	else {

		Page_validity_Status = PAGE_VALID_NUMBER ;
		pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

		/* Check if he want to erase all memory flash */
		if ( CBL_FLASH_MASS_ERASE == PageAddress  ){
			pEraseInit.PageAddress = FLASH_PAGE_BASE_ADDRESS_APP1;
			pEraseInit.NbPages = APPLICATION_SIZE;
		}
		/* erase specific page */
		else {
			pEraseInit.PageAddress = PageAddress;
			pEraseInit.NbPages = Number_Of_Pages;
		}

		/* To unlock flash memory */
		HAL_Status = HAL_FLASH_Unlock();

		/* if it's opened correctly */
		if (HAL_Status == HAL_OK){
			/* Perform a mass erase or erase the specified FLASH memory sectors */
			HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

			/* To check that the flash memory is erased sucessfully */
			if (HAL_SUCESSFUL_ERASE == PageError){
				Page_validity_Status = SUCESSFUL_ERASE ;
			}
			/* Didn't erase*/
			else {
				Page_validity_Status = UNSUCESSFUL_ERASE ;
			}

			HAL_Status = HAL_FLASH_Lock();
		}
		/* Not opened */
		else {
			Page_validity_Status = UNSUCESSFUL_ERASE ;
		}
	}
	return Page_validity_Status ;
}

/* Write actually in flash memory */
static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_PayLoad,
	uint32_t Payload_Start_Address, uint8_t Payload_Len) {
	/* The status in dealing HAL functions */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	/* Status writing in flash memory */
	uint8_t Status = FLASH_PAYLOAD_WRITING_FAILED;
	/* The number of words in data appliction sections */
	uint8_t PayLoad_Counter = RESET ;
	/* buffering half word */
	uint16_t Payload_Buffer = RESET ;
	/* address of current writing half word */
	uint32_t Address = RESET ;

	/* Writing steps */
	/* Open flash memory */
	HAL_Status = HAL_FLASH_Unlock();

	/* If it opened */
	if (HAL_Status == HAL_OK) {
		/* Transfer the data sections half word by half word */
		while (Payload_Len !=0 && Status == HAL_OK ){
			Payload_Buffer = (uint16_t) Host_PayLoad[PayLoad_Counter]
													 |( (uint16_t)Host_PayLoad[PayLoad_Counter+1] << ONE_BYTE );

			/*update the flash address each itration */
			Address = Payload_Start_Address + PayLoad_Counter ;

			/*Writing the Date in the flash Halfword by Halfword */
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
					Address, (uint64_t)Payload_Buffer);

			/*To increment PayLoad_Counter and stopped on new half word in host packet frame
			  To decrement Payload_Len to be sure that all of host packet frame is written
			 */
			Payload_Len-=2;
			PayLoad_Counter+=2;
		}

		/* if function can't write on memory Status be failed */
		if (HAL_Status != HAL_OK) {
			Status = FLASH_PAYLOAD_WRITING_FAILED;
		} else {
			/* All iterations, It can write on memory make status passed */
			Status = FLASH_PAYLOAD_WRITING_PASSED;
		}
	}
	else {
		/* If it can't open memory make status failed */
		Status = FLASH_PAYLOAD_WRITING_FAILED;
	}

	/* If all status is OK so It will lock memory */
	if (Status == FLASH_PAYLOAD_WRITING_PASSED && HAL_Status == HAL_OK) {
		HAL_Status = HAL_FLASH_Lock();

		/* Check if it locked it true or not */
		if (HAL_Status != HAL_OK) {
			Status = FLASH_PAYLOAD_WRITING_FAILED;
		} else {
			Status = FLASH_PAYLOAD_WRITING_PASSED;
		}
	} else {
		/* If one of status is not OK so It will make returned status with failed */
		Status = FLASH_PAYLOAD_WRITING_FAILED;
	}
	return Status;
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  APP_Fetch_Commend();
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	  HAL_Delay(3000);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	  HAL_Delay(3000);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

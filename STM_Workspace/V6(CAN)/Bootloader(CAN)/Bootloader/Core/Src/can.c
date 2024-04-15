/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

	/* USER CODE BEGIN CAN_Init 0 */

	/* USER CODE END CAN_Init 0 */

	/* USER CODE BEGIN CAN_Init 1 */

	/* USER CODE END CAN_Init 1 */
	hcan.Instance = CAN1;
	hcan.Init.Prescaler = 32;
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan.Init.TimeSeg1 = CAN_BS1_16TQ;
	hcan.Init.TimeSeg2 = CAN_BS2_8TQ;
	hcan.Init.TimeTriggeredMode = DISABLE;
	hcan.Init.AutoBusOff = DISABLE;
	hcan.Init.AutoWakeUp = DISABLE;
	hcan.Init.AutoRetransmission = ENABLE;
	hcan.Init.ReceiveFifoLocked = ENABLE;
	hcan.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN CAN_Init 2 */
	CAN_FilterTypeDef Config_CAN_Filter;

	/*Slave Receive Data filter configuration:
	 *Data message ID must be (0bXXX 1111 XXXX) (ID is 11 bit length as we use StandardID)
	 *Data message ID forbidden to be any value that have this structure:(0bXXX 1111 1111) to avoid conflicting with Remote message filter(This condition is applied to allow using of Data match index)
	 *Data message Match index is 2
	 *FIFO0 used for Data messages*/
	Config_CAN_Filter.FilterActivation = CAN_FILTER_ENABLE;
	Config_CAN_Filter.FilterBank = 2;
	Config_CAN_Filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	Config_CAN_Filter.FilterIdHigh = 0x0FF<<5; //shift to write to standard ID(look at datasheet for more illustration)
	Config_CAN_Filter.FilterIdLow = 0x0000;
	Config_CAN_Filter.FilterMaskIdHigh = 0xF0<<5;//shift to write to standard ID(look at datasheet for more illustration)
	Config_CAN_Filter.FilterMaskIdLow = 0x0000;
	Config_CAN_Filter.FilterMode = CAN_FILTERMODE_IDMASK;
	Config_CAN_Filter.FilterScale = CAN_FILTERSCALE_32BIT;
	Config_CAN_Filter.SlaveStartFilterBank = 13; //for dual CAN from 13 to 27 are slaves

	HAL_CAN_ConfigFilter(&hcan, &Config_CAN_Filter);


    /*Slave Receive Remote filter configuration:
	 *Remote message ID must be (0bXXX XXXX 1111) (ID is 11 bit length as we use StandardID)
	 *Remote message ID forbidden to be any value that have this structure:(0bXXX 1111 1111) to avoid conflict with Data message filter(This condition is applied to allow using of Remote match index)
	 *Remote message Match index is 0
	 *FIFO1 used for Remote messages*/
	Config_CAN_Filter.FilterBank = 1;
	Config_CAN_Filter.FilterFIFOAssignment = CAN_FILTER_FIFO1;
	Config_CAN_Filter.FilterIdHigh = 0x00F<<5; //shift to write to standard ID(look at datasheet for more illustration)
	Config_CAN_Filter.FilterMaskIdHigh = 0x00F<<5;//shift to write to standard ID(look at datasheet for more illustration)

	HAL_CAN_ConfigFilter(&hcan, &Config_CAN_Filter);



	/* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(canHandle->Instance==CAN1)
	{
		/* USER CODE BEGIN CAN1_MspInit 0 */

		/* USER CODE END CAN1_MspInit 0 */
		/* CAN1 clock enable */
		__HAL_RCC_CAN1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* CAN1 interrupt Init */
		HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
		HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
		HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
		HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
		/* USER CODE BEGIN CAN1_MspInit 1 */

		/* USER CODE END CAN1_MspInit 1 */
	}
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

	if(canHandle->Instance==CAN1)
	{
		/* USER CODE BEGIN CAN1_MspDeInit 0 */

		/* USER CODE END CAN1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_CAN1_CLK_DISABLE();

		/**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

		/* CAN1 interrupt Deinit */
		HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
		HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
		HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
		HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
		/* USER CODE BEGIN CAN1_MspDeInit 1 */

		/* USER CODE END CAN1_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

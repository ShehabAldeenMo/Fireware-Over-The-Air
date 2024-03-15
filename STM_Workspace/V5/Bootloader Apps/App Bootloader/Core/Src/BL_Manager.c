/**
 ******************************************************************************
 * @file name      : BL_Manager.c
 * @Author         : Shehab aldeen mohammed, Ali Mamdouh
 *
 ******************************************************************************
 * @Notes:
 *
 *
 ******************************************************************************
 ******************************************************************************
*/




/****************************************************************************
 ******************************  Including  **********************************
 *****************************************************************************/
#include "BL_Manager.h"
#include "BL_Manager_Config.h"







/*****************************************************************************
 *************************  Global Variables Definations  ********************
 *****************************************************************************/
uint8_t BL_Jump_Status = 0; //Should used with UART
uint8_t BL_Jump_Flag  = 0;   //Used with BL response










/*****************************************************************************
 ***********************  Software Interface Definations  ********************
 *****************************************************************************/
void Write_Program_Flag(uint32_t Address, uint32_t Value)
{
	uint32_t status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, (uint64_t)Value);
}








uint32_t Get_Program_Flag_Status(uint32_t Address)
{
	uint32_t Status = (*(uint32_t*)Address);
	return Status;
}







void BL_Response(void)
{
	if(BL_Jump_Flag == BL_BEGIN_MATCH)
	{
		/*Write Flag of BL to 1(To jump to BL)*/
		HAL_FLASH_Unlock();
		Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_ON);
		HAL_FLASH_Lock();

		/*Reset App*/
		Jump_To_Address(RESET_ADDRESS); //Must make Reset by your own as Soft reset return you to Flash base address.
	}
	else if(BL_Jump_Flag == BL_BEGIN_NOT_MATCH)
	{
		uint8_t Ack = BL_BEGIN_NOT_MATCH;

		/*Send Ack to Host*/
#if(ECU_CONFIG == MASTERID)
		HAL_UART_Transmit(MASTER_TO_HOST_CONNECTION, &Ack, 1, 5);
#elif(ECU_CONFIG == SLAVE1)
		HAL_UART_Transmit(SLAVE_TO_MASTER_CONNECTION, &Ack, 1, 5);
#endif

		/*Reset App*/
		Jump_To_Address(RESET_ADDRESS); //Must make Reset by your own as Soft reset return you to Flash base address.
	}
	else
	{
		/*Do nothing*/
	}
}








void Jump_To_Address (uint32_t Address)
{
	/* Value of the main stack pointer of our main application find at address 0 in IVT */
	uint32_t MSP_Value = *((volatile uint32_t*)Address);

	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(Address+4));

	/* Declare pointer to function contain the beginning address of reset function in user application */
	pFunc ResetHandler_Address = (pFunc)MainAppAddr;

	/* Deinit of modules that used in bootloader and work the configurations of new application */
	HAL_RCC_DeInit(); /* Resets the RCC clock configuration to the default reset state. */

	/* Reset main stack pointer */
	__set_MSP(MSP_Value);

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}














void BL_Manager(void)
{
	if(Get_Program_Flag_Status(FLAG_BL_ADDRESS) == FLAG_APP_ON)
	{
	    uint8_t Ack = BL_BEGIN_MATCH;
#if(ECU_CONFIG == MASTERID)
	    /*Receive and send Ack to Host*/
		HAL_UART_Transmit(MASTER_TO_SLAVE_CONNECTION, &Ack, 1, 3);
		HAL_UART_Receive(MASTER_TO_SLAVE_CONNECTION, &Ack, 1, HAL_MAX_DELAY);
		HAL_UART_Transmit(MASTER_TO_HOST_CONNECTION, &Ack, 1, 3);

		if(Ack != BL_BEGIN_MATCH)
		{
			/*Write Flag of BL to 1(To jump to BL)*/
			HAL_FLASH_Unlock();
			Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
			HAL_FLASH_Lock();

			/*Deinitialize*/
			BL_Jump_Status = 0; //Should used with UART
			BL_Jump_Flag   = 0;   //Used with BL response
			HAL_UART_Receive_IT(MASTER_TO_HOST_CONNECTION, &BL_Jump_Status, 1);//Enable to Receive message again
			return;
		}

		/*Delay without using Systick for stability*/
        int i;
		for(i =0; i<100; i++);

#elif(ECU_CONFIG == SLAVE1)
		/*Sending Ack then jumping to bootloader*/
		Ack = BL_BEGIN_MATCH;
		HAL_UART_Transmit(SLAVE_TO_MASTER_CONNECTION, &Ack, 1, 3);

		/*Jump to bootloader*/
		Jump_To_Address(FLASH_PAGE_BASE_ADDRESS_APP2);
#endif
		Jump_To_Address(FLASH_PAGE_BASE_ADDRESS_APP2);


	}
	else
	{

#if(ECU_CONFIG == MASTERID)
		HAL_UART_Receive_IT(MASTER_TO_HOST_CONNECTION, &BL_Jump_Status, 1);
#elif(ECU_CONFIG == SLAVE1)
		HAL_UART_Receive_IT(SLAVE_TO_MASTER_CONNECTION, &BL_Jump_Status, 1);
#endif
	}
}




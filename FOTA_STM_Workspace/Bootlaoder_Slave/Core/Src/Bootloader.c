/*============================================================================
 * @file name      : Bootloader.c
 * @Author         : Shehab aldeen mohammed, Ali Mamdouh
 *
 =============================================================================
 * @Notes:
 *
 *
 *
 ******************************************************************************
 ==============================================================================
*/


/*============================================================================
 ******************************  Including  **********************************
 ============================================================================*/
#include "Bootloader.h"
#include "Bootloader_Config.h"


/*============================================================================
 *********************  Static Functions Declerations  ***********************
 ============================================================================*/
/**
 * \section Service_Name
 * Bootloader_Get_Version
 *
 * \section Description
 * To get version of bootloader
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * BL_Status
 */
static void Bootloader_Get_Version (uint8_t *Host_Buffer);

/**
 * \section Service_Name
 * Bootloader_Send_Data_To_Host
 *
 * \section Description
 * To communicate with host like esp or TTL through uart
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t) --> buffer of uart frame
 * \param[in]    Data_Len (uint32_t)              --> Number of data bytes that you want to transmit
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Send_Data_To_Host(uint8_t* Host_Buffer , uint32_t Data_Len);

/**
 * \section Service_Name
 * Bootloader_Send_Data_To_Tree
 *
 * \section Description
 * To communicate with nodes on tree
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[in]    Data_Len (uint32_t)                --> Number of data bytes that you want to transmit
 * \param[in]    Time_To_Transmit (uint32_t)        --> The upper time limited to transmit this bytes
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Send_Data_To_Tree(uint8_t* Host_Buffer , uint32_t Data_Len, uint32_t Time_To_Transmit);

/**
 * \section Service_Name
 * Bootloader_Read_Protection_Level
 *
 * \section Description
 * Get protection level of flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer);

/**
 * \section Service_Name
 * Bootloader_Enable_R_Protection
 *
 * \section Description
 * Change Read protection level of flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Enable_R_Protection(uint8_t *Host_Buffer);

/**
 * \section Service_Name
 * Bootloader_Enable_R_Protection
 *
 * \section Description
 * Change Write protection level of flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Enable_W_Protection(uint8_t *Host_Buffer);

/**
 * \section Service_Name
 * Bootloader_Memory_Write
 *
 * \section Description
 * Write yart frames into flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Memory_Write (uint8_t *Host_Buffer);


/**
 * \section Service_Name
 * Bootloader_Erase_Flash
 *
 * \section Description
 * Erase according to uart frames
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Erase_Flash (uint8_t *Host_Buffer);

/**
 * \section Service_Name
 * Bootloader_Jump_To_Address
 *
 * \section Description
 * Jump to specific address
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)   --> buffer of uart frame
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer);

/**
 * \section Service_Name
 * BL_SetApplication_Flag
 *
 * \section Description
 *
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Jump_Address (uint32_t)
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void BL_SetApplication_Flag(uint32_t Host_Jump_Address);

/**
 * \section Service_Name
 * CBL_STM32F103_GET_RDP_Level
 *
 * \section Description
 * Get level of read protection level in flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    None
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * level of read protection level (uint8_t)
 */
static uint8_t CBL_STM32F103_GET_RDP_Level ();

/**
 * \section Service_Name
 * CBL_STM32F103_GET_WDP_Level
 *
 * \section Description
 * Get level of write protection level in flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    None
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * level of Write protection level (uint8_t)
 */
static uint8_t CBL_STM32F103_GET_WDP_Level ();

/**
 * \section Service_Name
 * CBL_STM32F103_GET_WDP_Level
 *
 * \section Description
 * Get level of write protection level in flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    Host_Buffer (pointer to uint8_t)  --> buffer of uart frame
 * \param[in]    Payload_Start_Address (uint32_t)  -->address of data that you want to write into it
 * \param[in]    Payload_Len (uint8_t)             -->address of data that you want to write into it
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * Status of writing data(uint8_t)
 */
static uint8_t Flash_Memory_Write_Payload (uint8_t *Host_PayLoad , uint32_t Payload_Start_Address,uint8_t Payload_Len);

/**
 * \section Service_Name
 * Perform_Flash_Erase
 *
 * \section Description
 * Erase pages into flash memory
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    PageAddress (uint32_t)       -->address of data that you want to erase it
 * \param[in]    Number_Of_Pages (uint8_t)    -->number of pages
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * Status of erasing data(uint8_t)
 */
static uint8_t Perform_Flash_Erase (uint32_t PageAddress, uint8_t Number_Of_Pages);

/**
 * \section Service_Name
 * Host_Jump_Address_Verfication
 *
 * \section Description
 * Verify that the address is valid
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    PageAddress (uint32_t)       -->address of data that you want to erase it
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * Status of erasing data(uint8_t)
 */
static uint8_t Host_Jump_Address_Verfication (uint32_t Jump_Address);

/**
 * \section Service_Name
 * Bootloader_Jump_To_User_App1
 *
 * \section Description
 *
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    None
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void    Bootloader_Jump_To_User_App1 ();

/**
 * \section Service_Name
 * Bootloader_Jump_To_User_App2
 *
 * \section Description
 *
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    None
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void    Bootloader_Jump_To_User_App2 ();

/**
 * \section Service_Name
 * BL_Manager
 *
 * \section Description
 *
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    None
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
static void    BL_Manager(void);

/*============================================================================
 *********************  Static global Variables Definations  *****************
 ============================================================================*/
/* Global buffer to store the comming uart frames from host */
static uint8_t BL_HostBuffer[BL_HOST_BUFFER_RX_LENGTH];

/* */
static uint8_t ErrorFrame = 0xAA ;

/* Object to structure to store the inner items */
FLASH_OBProgramInitTypeDef pOBInit;

/* store names for flags that assigned in flash memory in
 * specific sections to define the statue of application */
uint32_t APP1_Flag __attribute__((section(".APPLICATION1_SECTION"))); //To read counter of Full TI in Flash
uint32_t APP2_Flag __attribute__((section(".APPLICATION2_SECTION"))); //To read counter of Full TI in Flash

/* Pointer to bootloader functions */
static BL_pFunc Bootloader_Functions [NumberOfCommends] = {&Bootloader_Get_Version,
		&Bootloader_Read_Protection_Level,&Bootloader_Jump_To_Address,
		&Bootloader_Erase_Flash,&Bootloader_Memory_Write,&Bootloader_Enable_R_Protection,
		&Bootloader_Enable_W_Protection} ;




static uint32_t FLAG_APP_ON  = 0xAAAAAAAA ;
static uint32_t FLAG_APP_OFF = 0xEEEEEEEE ;

/*============================================================================
 ***********************  Software Interface Definations  ********************
 ============================================================================*/
BL_Status BL_Fetch_Commend(void) {
	/* To detect the status of function */
	BL_Status Status = BL_NACK;

	/* To detect the status of uart in transmitting and receiving data */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;

	/* The data length that the host should be transmit at first */
	uint8_t Data_Length = RESET;

	/*Check whether to jump to app or continue in bootloader*/
    BL_Manager();

	/* To clear buffer of RX and prevent carbadge messages of buffer */
	memset(BL_HostBuffer, RESET , BL_HOST_BUFFER_RX_LENGTH);

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage("Bootloader started..\r\n");
#endif
	/* Host commend format :
	   => Commend Length  (1 byte = Data_Length )
	 */
	HAL_Status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, BL_HostBuffer, 1,
			HAL_MAX_DELAY);

	if (HAL_Status != HAL_OK){
		Status = BL_NACK ;
	}
	else {
		if ( BL_HostBuffer[0] == 0xCC){
			HAL_NVIC_SystemReset();
		}
		else {
			/*
			 Depending on Data_Length we will recieve the number of bytes of the sending code
			 Commend Code (1 byte) + Delails (Data_Length)
			 Where :
			 => Commend Code is the order that Host want to do in code
			 => Delails explain what you transmit
			 */
			Data_Length = BL_HostBuffer[0];
		}

		/* we determine the number of recieving bytes next from the first number transmit in first
		   time (using buffer Data_Length and store them in BL_HostBuffer) */
		HAL_Status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, &BL_HostBuffer[1],
				Data_Length, HAL_MAX_DELAY);

		/* if it don't receive correctly */
		if (HAL_Status != HAL_OK){
			Status = BL_NACK ;
		}
		else {
			/* To jump on the target function from the previous commend */
			if (BL_HostBuffer[1]>=FIRST_COMMEND &&BL_HostBuffer[1]<=LAST_COMMEND ){
				Bootloader_Functions[BL_HostBuffer[1]-FIRST_COMMEND](BL_HostBuffer);
			}
			else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage ("%d\n",BL_HostBuffer[1]);
				BL_PrintMassage ("Invalid commend code recieved from host !! \r\n");
#endif
				Status = BL_NACK ;
			}
		}
	}
	return Status;
}

/* Function to communicate with Host */
static void Bootloader_Send_Data_To_Host(uint8_t* Host_Buffer , uint32_t Data_Len){
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,Host_Buffer,(uint16_t) Data_Len, HAL_MAX_DELAY);
}

/* Function to communicate with tree */
static void Bootloader_Send_Data_To_Tree(uint8_t* Host_Buffer , uint32_t Data_Len , uint32_t Time_To_Transmit){
	HAL_UART_Transmit(BL_TREE_COMMUNICATION_UART,Host_Buffer,(uint16_t) Data_Len, Time_To_Transmit);
}

/*
 your packet is
   1- 1 byte for data length = 0x02
   2- 1 byte for commend number = 0x10
   3- 1 byte to define that is for master ECU or slave ECU "Maser = 0x00 , slave = 0x01:0x05"
 */
static void Bootloader_Get_Version (uint8_t *Host_Buffer){
	/* Sending the version and vendor id's to meet the target from command */
	uint8_t BL_Version[4] = { CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION,
			CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read Bootloader Version \r\n");
#endif

	/* To check that the coming id for master_id or slave_id */
	if (MASTERID == Host_Buffer[2]) {

		/* If you are master and id is master */
#if(ECU_CONFIG == MASTERID)

		Bootloader_Send_Data_To_Host(BL_Version,4);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Master Bootloader Version %d.%d.%d.%d\r\n", BL_Version[0],
				BL_Version[1], BL_Version[2], BL_Version[3]);
#endif

		/* If you are slave and id is master */
#elif(ECU_CONFIG == SLAVE1)
		/* Report Error frame */
		Bootloader_Send_Data_To_Host((uint8_t*) (&ErrorFrame),1);

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("You are configured this ECU with wrong status \r\n");
#endif

#endif

	}
	else if(SLAVE1 == Host_Buffer[2]){
		/* If you are master and id is slave */
#if(ECU_CONFIG == MASTERID)
			uint8_t Slave_Version[4];
			Bootloader_Send_Data_To_Tree(&Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
			Bootloader_Send_Data_To_Tree(&Host_Buffer[1], Host_Buffer[0], 10); //sending remaining frame, The time must be 10 to avoid errors
			HAL_UART_Receive(BL_TREE_COMMUNICATION_UART, Slave_Version, 4, HAL_MAX_DELAY); //Receive version
			Bootloader_Send_Data_To_Host(Slave_Version,4);

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Slave %d Bootloader Version %d.%d.%d.%d\r\n",SLAVE1 ,Slave_Version[0],
				Slave_Version[1], Slave_Version[2], Slave_Version[3]);
#endif
		/* If you are slave and id is slave */
#elif(ECU_CONFIG == SLAVE1)

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Slave %d Bootloader Version %d.%d.%d.%d\r\n", SLAVE1 ,BL_Version[0],
				BL_Version[1], BL_Version[2], BL_Version[3]);
#endif
			Bootloader_Send_Data_To_Host(BL_Version, 4);
#endif
	}
	/* neither nor */
	else
	{
		/* Report Error frame */
		Bootloader_Send_Data_To_Host((uint8_t*) (&ErrorFrame),1);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("You sent wrong id that is not exist in ECU tree\r\n");
#endif
		}
}

/* Get level of protection to flash memory */
static uint8_t CBL_STM32F103_GET_RDP_Level (){
	/* paramter input for function that get level of memory */
	FLASH_OBProgramInitTypeDef FLASH_OBProgram ;
	/* Get level of memory */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);
	/* Assign protection level in parameter [in\out] */
	return (uint8_t)FLASH_OBProgram.RDPLevel ;
}


/* Get level of protection to flash memory */
static uint8_t CBL_STM32F103_GET_WDP_Level (){
	/* paramter input for function that get level of memory */
	FLASH_OBProgramInitTypeDef FLASH_OBProgram ;
	/* Get level of memory */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);
	/* Assign protection level in parameter [in\out] */
	return (uint8_t)FLASH_OBProgram.WRPPage ; //Casting to uint8_t as I care only with first 32 pages as STM is low density(casting would change for higher densities)
}


/*
 Your packet is :
   1- 1 byte data length = 0x03
   2- 1 byte commend number = 0x11
   3- 1 byte for ECU id
   4- 1 byte for determine write or read protection level
 */
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer){

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read the flash protection out level \r\n");
#endif

	/* If It's master and id is slave */
	if (SLAVE1 == Host_Buffer[2]){

#if (ECU_CONFIG == MASTERID)
		/* To report statue of protection level */
		uint8_t Protection_Level = 0;

		Bootloader_Send_Data_To_Tree(&Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
		Bootloader_Send_Data_To_Tree(&Host_Buffer[1], Host_Buffer[0], 10); //sending remaining frame, The time must be 10 to avoid errors
		HAL_UART_Receive(BL_TREE_COMMUNICATION_UART, &Protection_Level, 1 , HAL_MAX_DELAY); //Receive version
		Bootloader_Send_Data_To_Host((uint8_t*) (&Protection_Level), 1);
		return ;
#endif
	}

	/* If it's slave and id is slave
	 * If it's master and id is master
	 *  */
	if (READ_RP == Host_Buffer[3]) {
		uint8_t RDP_Level = CBL_STM32F103_GET_RDP_Level();

		/* Report level */
		Bootloader_Send_Data_To_Host((uint8_t*) (&RDP_Level), 1);

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Read Protection level = %x\r\n", RDP_Level);
#endif
	}

	else if (READ_WP == Host_Buffer[3]) {
		uint8_t WDP_Level = CBL_STM32F103_GET_WDP_Level();

		/* Report level */
		Bootloader_Send_Data_To_Host((uint8_t*) (&WDP_Level), 1);

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Write Protection level = %x\r\n", WDP_Level);
#endif
	}
	else {
		/* Report Error frame */
		Bootloader_Send_Data_To_Host((uint8_t*) (&ErrorFrame),1);
	}
}


/*
 Your packet is :
   1- 1 byte data length = 0x03
   2- 1 byte commend number = 0x15
   3- 1 byte for ECU id
   4- 1 byte for determine write or read protection level
 */
static void Bootloader_Enable_R_Protection(uint8_t *Host_Buffer){
	/*
	 * Protect from any kind of flasher tools (JTAG Comm., ..etc)
	 * Device works using only supply not flasher.
	 *
	 * To Make your flash memory protected from reading
	 * and if anyone changed the OB bit the memory would got erased
	 * OB bit is the only unlocked reg while memory protection mode
	 * The conf for HAL_FLASHEx_OBProgram is
	 *
	 *   pOBInit.OptionType = OPTIONBYTE_RDP;
	 *   pOBInit.RDPLevel = OB_RDP_LEVEL_1;
	 *
	 * 1. Unlock the flash.  		HAL_FLASH_Unlock
	 * 2. Unlock option byte. 		HAL_FLASH_OB_Unlock
	 * 3. Program option bytes		HAL_FLASHEx_OBProgram
	 * 4. Lock the flash			HAL_FLASH_Lock
	 * 5. Lock option byte. 		HAL_FLASH_OB_Lock
	 * 6. Reset.					HAL_FLASH_OB_Launch
	 *
	 *
	 * */
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
	uint8_t Host_ROP_Level   = Host_Buffer[3] ;

	/* If It's master and id is slave */
	if (SLAVE1 == Host_Buffer[2]){
#if (ECU_CONFIG == MASTERID)
		Bootloader_Send_Data_To_Tree(&Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
		Bootloader_Send_Data_To_Tree(&Host_Buffer[1], Host_Buffer[0], 10); //sending remaining frame, The time must be 10 to avoid errors
		HAL_UART_Receive(BL_TREE_COMMUNICATION_UART, &ROP_Level_Status, 1 , HAL_MAX_DELAY); //Receive version
		Bootloader_Send_Data_To_Host((uint8_t*) (&ROP_Level_Status), 1);
		return ;
#endif
	}

	if (CBL_ROP_LEVEL_0 == Host_ROP_Level){
		pOBInit.OptionType = OPTIONBYTE_RDP;
		pOBInit.RDPLevel = OB_RDP_LEVEL_0;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();

		ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
		Bootloader_Send_Data_To_Host((uint8_t *)&ROP_Level_Status, 1);

		HAL_FLASH_OB_Launch();
	}
	else if (CBL_ROP_LEVEL_1 == Host_ROP_Level) {
		pOBInit.OptionType = OPTIONBYTE_RDP;
		pOBInit.RDPLevel = OB_RDP_LEVEL_1;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();

		ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
		Bootloader_Send_Data_To_Host((uint8_t *)&ROP_Level_Status, 1);

		HAL_FLASH_OB_Launch();
	}
	else {
		/* Report Error frame */
		Bootloader_Send_Data_To_Host((uint8_t*) (&ErrorFrame),1);
	}
}


/*
 Your packet is :
   1- 1 byte data length = 0x03
   2- 1 byte commend number = 0x16
   3- 1 byte for ECU id
   4- 1 byte for determine write or read protection level
 */
static void Bootloader_Enable_W_Protection(uint8_t *Host_Buffer){
	/*
	 * Protect from any kind of flasher tools (JTAG Comm., ..etc)
	 * Device works using only supply not flasher.
	 *
	 * To Make your flash memory protected from reading
	 * and if anyone changed the OB bit the memory would got erased
	 * OB bit is the only unlocked reg while memory protection mode
	 * The conf for HAL_FLASHEx_OBProgram is
	 *
	 *   pOBInit.OptionType = OPTIONBYTE_WDP;
	 *   pOBInit.WDPLevel = OB_RDP_LEVEL_1;
	 *
	 * 1. Unlock the flash.  		HAL_FLASH_Unlock
	 * 2. Unlock option byte. 		HAL_FLASH_OB_Unlock
	 * 3. Program option bytes		HAL_FLASHEx_OBProgram
	 * 4. Lock the flash			HAL_FLASH_Lock
	 * 5. Lock option byte. 		HAL_FLASH_OB_Lock
	 * 6. Reset.					HAL_FLASH_OB_Launch
	 *
	 *
	 * */

	uint8_t WOP_Level_Status = WOP_LEVEL_CHANGE_INVALID;
	uint8_t Host_WP_Level = Host_Buffer[3];

	/* If It's master and id is slave */
	if (SLAVE1 == Host_Buffer[2]){
#if (ECU_CONFIG == MASTERID)
		Bootloader_Send_Data_To_Tree(&Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
		Bootloader_Send_Data_To_Tree(&Host_Buffer[1], Host_Buffer[0], 10); //sending remaining frame, The time must be 10 to avoid errors
		HAL_UART_Receive(BL_TREE_COMMUNICATION_UART, &WOP_Level_Status, 1 , HAL_MAX_DELAY); //Receive version
		Bootloader_Send_Data_To_Host((uint8_t*) (&WOP_Level_Status), 1);
		return ;
#endif
	}

	if (OB_WRPSTATE_ENABLE == Host_WP_Level) {
		pOBInit.OptionType = OPTIONBYTE_WRP;
		pOBInit.WRPState = OB_WRPSTATE_ENABLE;
		pOBInit.WRPPage = OB_WRP_PAGES0TO3 | OB_WRP_PAGES4TO7 | OB_WRP_PAGES8TO11 | OB_WRP_PAGES12TO15 | OB_WRP_PAGES16TO19 | OB_WRP_PAGES20TO23 | OB_WRP_PAGES24TO27 | OB_WRP_PAGES28TO31;//We make ORing operations to enable write protect to all pages

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();

		WOP_Level_Status = WOP_LEVEL_CHANGE_VALID;
		Bootloader_Send_Data_To_Host((uint8_t*) (&WOP_Level_Status), 1);

		HAL_FLASH_OB_Launch();


	} else if (OB_WRPSTATE_DISABLE == Host_WP_Level) {
		pOBInit.OptionType = OPTIONBYTE_WRP;
		pOBInit.WRPState = OB_WRPSTATE_DISABLE;
		pOBInit.WRPPage = OB_WRP_PAGES0TO3 | OB_WRP_PAGES4TO7 | OB_WRP_PAGES8TO11 | OB_WRP_PAGES12TO15 | OB_WRP_PAGES16TO19 | OB_WRP_PAGES20TO23 | OB_WRP_PAGES24TO27 | OB_WRP_PAGES28TO31;//We make ORing operation to disable write protect to all pages(you should increase OR operation for higher densities STM)

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();

		WOP_Level_Status = WOP_LEVEL_CHANGE_VALID;
		Bootloader_Send_Data_To_Host((uint8_t*) (&WOP_Level_Status), 1);

		HAL_FLASH_OB_Launch();

	} else {
		/* Report Error frame */
		Bootloader_Send_Data_To_Host((uint8_t*) (&ErrorFrame),1);
	}
}





/* Verify that the address given from host is valid */
static uint8_t Host_Jump_Address_Verfication (uint32_t Jump_Address){
	/* TO check on state of given address is in region or not */
	uint8_t Address_Verification_State = ADDRESS_IS_INVALID ;

	/* Check that it is at RAM memory */
	if (Jump_Address>= SRAM_BASE && Jump_Address <=STM32F103_SRAM_END){
		Address_Verification_State = ADDRESS_IS_VALID ;
	}
	/* Check that it is at Flash memory */
	else if(Jump_Address>= FLASH_BASE && Jump_Address <=STM32F103_FLASH_END){
		Address_Verification_State = ADDRESS_IS_VALID ;
	}
	/* Invalid region */
	else {
		Address_Verification_State = ADDRESS_IS_INVALID ;
	}

	return Address_Verification_State ;
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
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage ("It is over the flash size\r\n");
#endif
	}
	/* erase all memory or specific page */
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage ("It is in range of flash memory \r\n");
#endif
		Page_validity_Status = PAGE_VALID_NUMBER ;
		pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;

		/* Check if he want to erase all memory flash */
		if ( CBL_FLASH_MASS_ERASE == PageAddress  ){
			pEraseInit.PageAddress = FLASH_PAGE_BASE_ADDRESS_APP1;
			pEraseInit.NbPages = APPLICATION_SIZE;
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage ("Mase erase \r\n");
#endif
		}
		/* erase specific page */
		else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage ("Page erase \r\n");
#endif
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






/*
 Your packet is :
   1- 1 byte data length = 0x0A
   2- 1 byte commend number = 0x15
   3- 4 bytes for page address
   4- 1 byte for number of pages
 */
static void Bootloader_Erase_Flash (uint8_t *Host_Buffer){
	/* To check on Erase state */
	uint8_t Erase_Status = UNSUCESSFUL_ERASE ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Mase erase or page erase of the user flash \r\n");
#endif

		/* Perform Mass erase or sector erase of the user flash */
		if (Host_Buffer[6] == MASTERID )
		{
#if(ECU_CONFIG == MASTERID)
			Erase_Status = Perform_Flash_Erase ( *( (uint32_t*)&Host_Buffer[2] ),Host_Buffer[7]);
			/* Report the erase state */
			Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);

#elif(ECU_CONFIG == SLAVE1)
			Erase_Status = UNSUCESSFUL_ERASE ;
			Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);
#endif
		}
		else if(Host_Buffer[6] == SLAVE1 )
		{
			/* Send it to Slave node */
#if(ECU_CONFIG == MASTERID)
			/*Sending frame to ECU2*/
			HAL_UART_Transmit(&huart2, &Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
			HAL_UART_Transmit(&huart2, &Host_Buffer[1], Host_Buffer[0], 5*Host_Buffer[0]); //sending remaining frame, The time must be of about 5 times the length to avoid errors


			/*Receiving Ack from ECU2*/
			HAL_UART_Receive(&huart2, (uint8_t *)&Erase_Status, 1, HAL_MAX_DELAY); //Receive Error status

			/*Sending Ack to ESP*/
			Bootloader_Send_Data_To_Host((uint8_t *)&Erase_Status, 1);
#elif(ECU_CONFIG == SLAVE1)
			Erase_Status = Perform_Flash_Erase ( *( (uint32_t*)&Host_Buffer[2] ),Host_Buffer[7]);
			/* Report the erase state */
			Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);

#endif

		}
		else
		{
			Erase_Status = UNSUCESSFUL_ERASE ;
			Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);
		}


		if ( SUCESSFUL_ERASE == Erase_Status){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage("Sucessful erased\r\n");
#endif
		}
		else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage("Unsucessful erased\r\n");
#endif
		}
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


/*
 Your packet is :
   1- 1 byte data length = 0x0B+0x0N
   2- 1 byte commend number = 0x16
   3- 4 bytes for address
   4- 1 byte for size of writing data
   5- N bytes of data info
 */
static void Bootloader_Memory_Write (uint8_t *Host_Buffer){
	/* Base address that you will write on */
	uint32_t HOST_Address = 0;
	/* Number of bytes that will be sent */
	uint8_t Payload_Len = 0;
	/* The status of input address from the host */
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	/* Status writing in flash memory */
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITING_FAILED;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Write data into memory \r\n");
#endif

		/* Extract the start address from the Host packet */
		HOST_Address = *((uint32_t *)(&Host_Buffer[2]));
		/* Extract the payload length from the Host packet */
		Payload_Len = Host_Buffer[8];

		/* Verify the Extracted address to be valid address */
		Address_Verification = Host_Jump_Address_Verfication(HOST_Address);

		if(ADDRESS_IS_VALID == Address_Verification)
		{
			if (Host_Buffer[6] == MASTERID){

#if(ECU_CONFIG == MASTERID)
				/* Write the payload to the Flash memory */
				Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[9], HOST_Address, Payload_Len);

				/* Report payload writing state */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);

#elif(ECU_CONFIG == SLAVE1)
				Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITING_FAILED;
				Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1); //send error frame

#endif

			}
			else if(Host_Buffer[6] == SLAVE1) {
#if(ECU_CONFIG == MASTERID)
				HAL_UART_Transmit(&huart2, &Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
				HAL_UART_Transmit(&huart2, &Host_Buffer[1], Host_Buffer[0], 5*Host_Buffer[0]); //sending remaining frame, The time must be of about 5 times the length to avoid errors
				HAL_UART_Receive(&huart2, (uint8_t *)&Flash_Payload_Write_Status, 1, HAL_MAX_DELAY); //Receive Error status
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);

#elif(ECU_CONFIG == SLAVE1)
				/* Write the payload to the Flash memory */
				Flash_Payload_Write_Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[9], HOST_Address, Payload_Len);

				/* Report payload writing state */
				Bootloader_Send_Data_To_Host((uint8_t *)&Flash_Payload_Write_Status, 1);
#endif
			}
			else
			{
				/* Report error frame */
				Address_Verification = ADDRESS_IS_INVALID;
				Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
			}

		}
		else
		{
			/* Report address verification failed */
			Address_Verification = ADDRESS_IS_INVALID;
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification, 1);
		}
}



/*
 Your packet is :
   1- 1 byte data length    = 0x06
   2- 1 byte commend number = 0x12
   3- 1 byte for ECU
   4,5,6,7- 4 bytes for address
 */
static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer){
	/* Buffering address */
	uint32_t HOST_Address = RESET ;
	/* TO check on state of given address is in region or not */
	uint8_t Address_Verification_State = ADDRESS_IS_INVALID ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Bootloader jump to specified address \r\n");
#endif


	/* To get the content of Host_Buffer and variable"Host_Jump_Address" realizes that it is address
	 - &Host_Buffer[2] --> express the address of array of host
	 - (uint32_t *)    --> casting it to pointer of uint32
	 - *               --> De-reference it and get the content of buffer at this element
	 */
	HOST_Address = *((uint32_t *)(&Host_Buffer[2]));

	/* If It's master and id is slave */
	if (SLAVE1 == Host_Buffer[6]){

#if (ECU_CONFIG == MASTERID)
		Bootloader_Send_Data_To_Tree(&Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
		Bootloader_Send_Data_To_Tree(&Host_Buffer[1], Host_Buffer[0], 20); //sending remaining frame, The time must be 10 to avoid errors
		return ;
#endif
	}

		/* To verify that the address in the region of memory */
		Address_Verification_State = Host_Jump_Address_Verfication(HOST_Address);

		if (Address_Verification_State == ADDRESS_IS_VALID ){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage("Address verification sucessed\r\n");
#endif

			if (HOST_Address == FLASH_PAGE_BASE_ADDRESS_APP1){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage("Jump To Application\r\n");
#endif

				BL_SetApplication_Flag(HOST_Address);
				Bootloader_Jump_To_User_App1();
			}
			else if (HOST_Address == FLASH_PAGE_BASE_ADDRESS_APP2){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage("Jump To Application\r\n");
#endif

				BL_SetApplication_Flag(HOST_Address);
				Bootloader_Jump_To_User_App2();
			}
			else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage("Jumped to : 0x%X \r\n",Host_Jump_Address);
#endif
				/* - Prepare the address to jump
				    - Increment 1 to be in thumb instruction */
				Jump_Ptr Jump_Address = (Jump_Ptr) (HOST_Address + 1) ;
				Jump_Address();
			}
		}
		else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage("Address verification unsucessed\r\n");
#endif
		}
}



/*
 Be sure that
   1- base address in application is updated in (Bootloader_Jump_To_User_App)
   2- update size of bootloader code with suitable size as 17k or 15k
   3- update origin address of application code in flash memory in linker script and size also
 */
static void Bootloader_Jump_To_User_App1 (){
	/* Value of the main stack pointer of our main application find at address 0 in IVT */
	uint32_t MSP_Value = *((volatile uint32_t*)FLASH_PAGE_BASE_ADDRESS_APP1);
	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(FLASH_PAGE_BASE_ADDRESS_APP1+4));

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage("Jump to application\r\n");
#endif

	/* Declare pointer to function contain the beginning address of reset function in user application */
	pFunc ResetHandler_Address = (pFunc)MainAppAddr;

	/* Deinitionalization of modules that used in bootloader and work
	   the configurations of new application */
	HAL_RCC_DeInit(); /* Resets the RCC clock configuration to the default reset state. */

	/* Reset main stack pointer */
	__set_MSP(MSP_Value);

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}

/*
 Be sure that
   1- base address in application is updated in (Bootloader_Jump_To_User_App)
   2- update size of bootloader code with suitable size as 17k or 15k
   3- update origin address of application code in flash memory in linker script and size also
 */
static void Bootloader_Jump_To_User_App2 (){
	/* Value of the main stack pointer of our main application find at address 0 in IVT */
	uint32_t MSP_Value = *((volatile uint32_t*)FLASH_PAGE_BASE_ADDRESS_APP2);
	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(FLASH_PAGE_BASE_ADDRESS_APP2+4));

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage("Jump to application\r\n");
#endif

	/* Declare pointer to function contain the beginning address of reset function in user application */
	pFunc ResetHandler_Address = (pFunc)MainAppAddr;

	/* Deinitionalization of modules that used in bootloader and work
	   the configurations of new application */
	HAL_RCC_DeInit(); /* Resets the RCC clock configuration to the default reset state. */

	/* Reset main stack pointer */
	__set_MSP(MSP_Value);

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}

static void BL_SetApplication_Flag(uint32_t Host_Jump_Address)
{
	Perform_Flash_Erase(FLAG_APP1_ADDRESS, 0x01);

	if (Host_Jump_Address == FLASH_PAGE_BASE_ADDRESS_APP1) {
		Flash_Memory_Write_Payload((uint8_t *) &FLAG_APP_ON, FLAG_APP1_ADDRESS, 4);
		Flash_Memory_Write_Payload((uint8_t *) &FLAG_APP_OFF, FLAG_APP2_ADDRESS, 4);
	} else if (Host_Jump_Address == FLASH_PAGE_BASE_ADDRESS_APP2) {
		Flash_Memory_Write_Payload((uint8_t *) &FLAG_APP_OFF, FLAG_APP1_ADDRESS, 4);
		Flash_Memory_Write_Payload((uint8_t *) &FLAG_APP_ON, FLAG_APP2_ADDRESS, 4);
	}
}


static void BL_Manager(void)
{
    if(Get_Program_Flag_Status(FLAG_APP1_ADDRESS) == FLAG_RISED)
	{
		/*Jump to App1*/
		Bootloader_Jump_To_User_App1();
	}
	else if(Get_Program_Flag_Status(FLAG_APP2_ADDRESS) == FLAG_RISED)
	{
		/*Jump to App2*/
		Bootloader_Jump_To_User_App2();
	}
}


uint32_t Get_Program_Flag_Status(uint32_t Address)
{
	uint32_t Status = (*(uint32_t*)Address);
	return Status;
}


void BL_PrintMassage(char *format, ...) {
	char Message[100] = { RESET };
	va_list args;
	/* Enable acess to the variable arguments */
	va_start(args, format);
	/* Write the formatted data from variable argument list to string */
	vsprintf(Message, format, args);
#if  BL_DEBUG_METHOD == BL_ENABLE_UART_DEBUG_MESSAGE
	/* Transmit the formatted data through the defined UART */
	HAL_UART_Transmit(BL_DEBUG_UART, (uint8_t*) Message, (uint16_t)sizeof(Message),
			HAL_MAX_DELAY);

#elif  BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE
	/* Transmit the formatted data through the defined CAN */
#elif  BL_DEBUG_METHOD == BL_ENABLE_ETHERNET_DEBUG_MESSAGE
	/* Transmit the formatted data through the defined ETHERNET */
#endif
	/* Perform cleanup for an ap object initialized by a call to va_start */
	va_end(args);
}

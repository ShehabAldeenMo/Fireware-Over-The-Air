/*
 * Name       : Bootloader.c
 * Created on : Oct 18, 2023
 * Author     : Shehab aldeen
 */

/*============================ Includes  =============================*/
#include "Bootloader.h"

/*=============== Static Functions Declerations  =====================*/
static void Bootloader_Get_Version (uint8_t *Host_Buffer);
static void Bootloader_Get_Help (uint8_t *Host_Buffer);
static void Bootloader_Get_chip_Identification_Number (uint8_t *Host_Buffer);
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer);
static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash (uint8_t *Host_Buffer);
static void Bootloader_Memory_Write (uint8_t *Host_Buffer);
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer);

static CRC_Status Bootloader_CRC_Verify(uint8_t *pData , uint8_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_ACK (uint8_t Reply_Length);
static void Bootloader_Send_NACK();
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer , uint32_t Data_Len);
static void Bootloader_Jump_To_User_App ();
static void CBL_STM32F103_GET_RDP_Level (uint8_t *RDP_Level);

static uint8_t Host_Jump_Address_Verfication (uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase (uint8_t Page_Number, uint8_t Number_Of_Pages);
static uint8_t Flash_Memory_Write_Payload (uint8_t *Host_PayLoad , uint32_t Payload_Start_Address,uint16_t Payload_Len);
static uint8_t Change_ROP_Level(uint8_t ROP_Level);

/*===================Static global Variables Definations  ==================*/
static uint8_t BL_HostBuffer[BL_HOST_BUFFER_RX_LENGTH];

/* All supported commends by bootloaders */
static uint8_t Bootloader_Supported_CMDs[13] = {
	CBL_GET_VER_CMD ,
	CBL_GET_HELP_CMD ,
	CBL_GET_CID_CMD ,
	CBL_GET_RDP_STATUS_CMD,
	CBL_GO_TO_ADDER_CMD,
	CBL_FLASH_ERASE_CMD,
	CBL_MEM_WRITE_CMD,
	CBL_EN_R_W_PROTECT_CMD,
	CBL_MEM_READ_CMD,
	CBL_READ_SECTOR_STATUS_CMD,
	CBL_OTP_READ_CMD,
	CBL_CHANGE_ROP_Level_CMD,
	CBL_JUMP_TO_APP
};

/*======================== Software Interface Definations  ====================*/
BL_Status BL_UART_Fetch_Host_Commend(void) {
	/* To detect the status of function */
	BL_Status Status = BL_NACK;
	/* To dtect the status of uart in transmitting and receiving data */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	/* The data length that the host should be transmit at first */
	uint8_t Data_Length = 0;

	/* To clear buffer of RX and prevent carbadge messages of buffer */
	memset(BL_HostBuffer, 0, BL_HOST_BUFFER_RX_LENGTH);

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
		/*
		 Depending on Data_Length we will recieve the number of bytes of the sending code
		 Commend Code (1 byte) + Delails (Data_Length) + CRC (4 bytes)
		 Where :
		 => Commend Code is the order that Host want to do in code
		 => Delails explain what you transmit
		 => CRC is safety algorthim on code
		 */
		Data_Length = BL_HostBuffer[0];

		/* we determine the number of recieving bytes next from the first number transmit in first
		   time (using buffer Data_Length and store them in BL_HostBuffer) */
		HAL_Status = HAL_UART_Receive(BL_HOST_COMMUNICATION_UART, &BL_HostBuffer[1],
				Data_Length, HAL_MAX_DELAY);

		if (HAL_Status != HAL_OK){
			Status = BL_NACK ;
		}
		else {
			/* To jump on the target function from the previous commend */
			switch (BL_HostBuffer[1]) {
				case CBL_GET_VER_CMD :
					Bootloader_Get_Version(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_GET_HELP_CMD :
					Bootloader_Get_Help(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_GET_CID_CMD :
					Bootloader_Get_chip_Identification_Number(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_GET_RDP_STATUS_CMD :
					Bootloader_Read_Protection_Level(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_GO_TO_ADDER_CMD :
					Bootloader_Jump_To_Address(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_FLASH_ERASE_CMD :
					Bootloader_Erase_Flash(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_MEM_WRITE_CMD :
					Bootloader_Memory_Write(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_CHANGE_ROP_Level_CMD :
					Bootloader_Change_Read_Protection_Level(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_JUMP_TO_APP :
					Bootloader_Jump_To_User_App();
					Status = BL_OK ;
					break;
				default :
					BL_PrintMassage ("Invalid commend code recieved from host !! \r\n ");
					Status = BL_NACK ;
					break;
			}
		}
	}
	return Status;
}

static CRC_Status Bootloader_CRC_Verify(uint8_t *pData , uint8_t Data_Len, uint32_t Host_CRC){
	/* Detect CRC status */
	CRC_Status Status = CRC_NACK ;
	/* Buffering the calculations on host bytes that it transmitted */
	uint32_t MCU_CRC_Calculated = 0 ;
	/* Used as counter on the number of bytes that host transmit without CRC bytes */
	uint8_t Data_Counter = 0 ;
	/* To avoid warning in sending address of uint8_t variable in address of
	   uint32_t variable in HAL_CRC_Accumulate */
	uint32_t Data_Buffer = 0 ;

	/* Calculate CRC32 */
	for (Data_Counter = 0 ; Data_Counter < Data_Len ; Data_Counter++){
		Data_Buffer = (uint32_t)pData[Data_Counter];
		MCU_CRC_Calculated = HAL_CRC_Accumulate(CRC_ENGINE_OBJ, &Data_Buffer, 1);
	}

	/* Reset the CRC Calculations unit */
	__HAL_CRC_DR_RESET(CRC_ENGINE_OBJ);

	if (MCU_CRC_Calculated == Host_CRC ){
		Status = CRC_OK ;
	}
	else {
		Status = CRC_NACK ;
	}
	return Status ;
}

/* Send Acknowledge message of bootloader then the number of the bytes that you will transmit
   next to host */
static void Bootloader_Send_ACK (uint8_t Reply_Length){
	uint8_t ACK_Value[2] = {0};
	ACK_Value[0] = CBL_SEND_ACK;
	ACK_Value[1] = Reply_Length;
	Bootloader_Send_Data_To_Host(ACK_Value, 2);
}

/* Send Noacknowledge message */
static void Bootloader_Send_NACK(){
	uint8_t ACK_Value = CBL_SEND_NACK;
	Bootloader_Send_Data_To_Host(&ACK_Value, 1);
}

/* Function to communicate with host */
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer , uint32_t Data_Len){
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,(uint8_t*) Host_Buffer,(uint16_t) Data_Len, HAL_MAX_DELAY);
}

/* To get version of bootloader to check on the leatest version
   To delay with this commend you should send :
   1- Data length = 0x01
   2- Commend number = 0x10 */
static void Bootloader_Get_Version (uint8_t *Host_Buffer){
	/* Buffering the version and vendor id's in BL_Version */
	uint8_t BL_Version[4] = { CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION,
			CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION };
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read bootloader version \r\n");
#endif
	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	//if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
/*#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Sending Acknowledge message and number of bytes which will be sent */
		//Bootloader_Send_ACK(4);
		/* Sending the version and vendor id's to meet the target from commend */
		//Bootloader_Send_Data_To_Host(BL_Version,4);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Bootloader Version %d.%d.%d\r\n",BL_Version[1],BL_Version[2],BL_Version[3]);
#endif
	/*}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

/* To get the help commends
   To delay with this commend you should send :
	1- Data length = 0x01
	2- Commend number = 0x11 */
static void Bootloader_Get_Help (uint8_t *Host_Buffer){
	//uint16_t Host_CMD_Packet_Len = 0 ;  /* used to define the beginning of CRC address in buffer */
	//uint32_t Host_CRC32 = 0 ;           /* Used to get CRC data */

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("All supported commends code \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Sending Acknowledge message and number of bytes which will be sent */
		//Bootloader_Send_ACK(12);
		/* Sending the list of commends to meet the target from commend */
		//Bootloader_Send_Data_To_Host(Bootloader_Supported_CMDs,12);
	BL_PrintMassage(
			"CBL_GET_VER_CMD %x\r\nCBL_GET_HELP_CMD %x\r\nCBL_GET_CID_CMD %x\r\n",
			Bootloader_Supported_CMDs[0], Bootloader_Supported_CMDs[1],
			Bootloader_Supported_CMDs[2]);
	BL_PrintMassage(
			"CBL_GET_RDP_STATUS_CMD %x\r\nCBL_GO_TO_ADDER_CMD %x\r\nCBL_FLASH_ERASE_CMD %x\r\n",
			Bootloader_Supported_CMDs[3], Bootloader_Supported_CMDs[4],
			Bootloader_Supported_CMDs[5]);
	BL_PrintMassage(
			"CBL_MEM_WRITE_CMD %x\r\nCBL_EN_R_W_PROTECT_CMD %x\r\nCBL_MEM_READ_CMD %x\r\n",
			Bootloader_Supported_CMDs[6], Bootloader_Supported_CMDs[7],
			Bootloader_Supported_CMDs[8]);
	BL_PrintMassage(
			"CBL_READ_SECTOR_STATUS_CMD %x\r\nCBL_OTP_READ_CMD %x\r\nCBL_CHANGE_ROP_Level_CMD %x\r\n",
			Bootloader_Supported_CMDs[9], Bootloader_Supported_CMDs[10],
			Bootloader_Supported_CMDs[11]);
	BL_PrintMassage("CBL_JUMP_TO_APP %x\r\n",Bootloader_Supported_CMDs[12]);
	/*}
	else
		{
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

/* To get Micro-controller number
   To delay with this commend you should send :
	1- Data length = 0x01
	2- Commend number = 0x12 */
static void Bootloader_Get_chip_Identification_Number (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
	/* Identify the id of used MCU */
	uint16_t MCU_IdentificationNumber = 0 ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read MCU chip identification number \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Get MCU chip identification number */
		MCU_IdentificationNumber = (uint16_t)((DBGMCU->IDCODE)&0x00000FFF);

		/* Report MCU chip identification number */
		//Bootloader_Send_ACK(2);
		//Bootloader_Send_Data_To_Host((uint8_t *)(&MCU_IdentificationNumber),2);
		BL_PrintMassage("IdentificationNumber = %x\r\n",MCU_IdentificationNumber);

	/*}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

static void CBL_STM32F103_GET_RDP_Level (uint8_t *RDP_Level){
	/* paramter input for function that get level of memory */
	FLASH_OBProgramInitTypeDef FLASH_OBProgram ;
	/* Get level of memory */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);
	/* Assign protection level in parameter [in\out] */
	*RDP_Level = (uint8_t)FLASH_OBProgram.RDPLevel ;
}

/* To get the protection level of flash memory in Micro-controller number
   To delay with this commend you should send :
	1- Data length = 0x01
	2- Commend number = 0x13 */
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
	/* Level of protection */
	uint8_t RDP_Level = 0 ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read the flash protection out level \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Report acknowledge message*/
		//Bootloader_Send_ACK(1);
		/* Read protection level */
		CBL_STM32F103_GET_RDP_Level(&RDP_Level);
		/* Report level */
		//Bootloader_Send_Data_To_Host((uint8_t *)(&RDP_Level),1);
		BL_PrintMassage("Protection level = %x\r\n",RDP_Level);
	/*}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}


static uint8_t Host_Jump_Address_Verfication (uint32_t Jump_Address){
	/* TO check on state of given address is in region or not */
	uint8_t Address_Verification_State = ADDRESS_IS_INVALID ;
	if (Jump_Address>= SRAM_BASE && Jump_Address <=STM32F103_SRAM_END){
		Address_Verification_State = ADDRESS_IS_VALID ;
	}
	else if(Jump_Address>= FLASH_BASE && Jump_Address <=STM32F103_FLASH_END){
		Address_Verification_State = ADDRESS_IS_VALID ;
	}
	else {
		Address_Verification_State = ADDRESS_IS_INVALID ;
	}
	return Address_Verification_State ;
}

/* To jump to specific address
   To delay with this commend you should send :
	1- Data length = 0x05
	2- Commend number = 0x14
	3- Wanted address = (4bytes) --> you should enter address in Realterm as this example
	0x0800014c --enter--> 0x4c then 0x01 then 0x00 then 0x08
	*/
static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
	/* Buffering address */
	uint32_t Host_Jump_Address = 0 ;
	/* TO check on state of given address is in region or not */
	uint8_t Address_Verification_State = ADDRESS_IS_INVALID ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Bootloader jump to specified address \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Report MCU chip identification number */
		//Bootloader_Send_ACK(1);

		/* To get the content of this address */
		Host_Jump_Address = *((uint32_t *) &(Host_Buffer[2])) ;

		/* To verify that the address in the region of memory */
		Address_Verification_State = Host_Jump_Address_Verfication(Host_Jump_Address);

		if (Address_Verification_State == ADDRESS_IS_VALID ){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Address verification sucessed\r\n");
#endif
			/* - Prepare the address to jump
			   - Increment 1 to be in thumb instruction */
			Jump_Ptr Jump_Address = (Jump_Ptr) (Host_Jump_Address + 1) ;
			Jump_Address();
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Jumped to : 0x%X \r\n",Jump_Address);
#endif
		}
		else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Address verification unsucessed\r\n");
#endif
		}
	//}
	/*else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

/* to work with this function you should make this variations :
	1- Data length = 0x01
	2- Jump to application commend = 0x22
   And be sure that
    1- base address in application is updated in (Bootloader_Jump_To_User_App)
    2- update size of bootloader code with suitable size as 17k or 15k
    3- update origin address of application code in flash memory in linker script and size also
    4- update the interrupt vector table to be allocate at start address of code in file (system_stm32f1xx.c)
       SCB->VTOR = FLASH_BASE | 0x8000;
  */
static void Bootloader_Jump_To_User_App (){
	/* Value of the main stack pointer of our main application */
	uint32_t MSP_Value = *((volatile uint32_t*)FLASH_PAGE_BASE_ADDRESS);
	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t*)(FLASH_PAGE_BASE_ADDRESS+4));

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Jump to application\r\n");
#endif

	/* Declare pointer to function contain the beginning address of application */
	pFunc ResetHandler_Address = (pFunc)MainAppAddr;

	/* Deinitionalization of modules that used in bootloader and work
	   the configurations of new application */
	HAL_RCC_DeInit(); /* Resets the RCC clock configuration to the default reset state. */

	/* Reset main stack pointer */
	__set_MSP(MSP_Value);

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}

static uint8_t Perform_Flash_Erase (uint8_t Page_Number, uint8_t Number_Of_Pages){
	/* To check that the sectors in not overflow the size of flash */
	uint8_t Page_validity_Status  = PAGE_INVALID_NUMBER ;
	/* Status of erasing flash */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;
	/* Error sector status */
	uint32_t PageError = 0 ;
	/* Define struct to configure parameters[in] */
	FLASH_EraseInitTypeDef pEraseInit ;
	/* Define the used bank in flash memory */
	pEraseInit.Banks = FLASH_BANK_1 ;

	/* trying to erase bootloader --> PAGE_INVALID_NUMBER */
	if (Page_Number < CBL_PAGE_END){
		Page_validity_Status = PAGE_INVALID_NUMBER ;
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("It will erase in bootloader code \r\n");
#endif
	}
	/* another pages is agreed but check that is acess the number of pages in flash */
	else if (((Page_Number + Number_Of_Pages) >= CBL_FLASH_MAX_PAGES_NUMBER)
			&& CBL_FLASH_MASS_ERASE != Page_Number) {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("It is over the flash size\r\n");
#endif
	Page_validity_Status = PAGE_INVALID_NUMBER ;
	}
	/* erase all memory or specific page */
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("It is in range of flash memory \r\n");
#endif
		Page_validity_Status = PAGE_VALID_NUMBER ;
		/* Check if he want to erase all memory flash */
		if ( CBL_FLASH_MASS_ERASE == Page_Number  ){
			pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE ;
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Mase erase \r\n");
#endif
		}
		/* erase specific page */
		else {
			pEraseInit.TypeErase   = FLASH_TYPEERASE_PAGES ;
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Page erase \r\n");
#endif
			pEraseInit.PageAddress = ((uint32_t) Page_Number
					* (STM32F103_FLASH_PAGE_SIZE) + FLASH_BASE);
			pEraseInit.NbPages     = Number_Of_Pages ;
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
			/* didn't erase*/
			else {
				Page_validity_Status = UNSUCESSFUL_ERASE ;
			}
		}
		/* Not opened */
		else {
			Page_validity_Status = UNSUCESSFUL_ERASE ;
		}
		/* if it erased correctly and opened correctly */
		if (Page_validity_Status == SUCESSFUL_ERASE && HAL_Status == HAL_OK ){
			/* To lock flash memory */
			HAL_Status = HAL_FLASH_Lock();
		}
		/* not erased or opened */
		else {
			Page_validity_Status  = PAGE_INVALID_NUMBER ;
		}
	}

	return Page_validity_Status ;
}

/* To erase specific page
   To delay with this commend you should send :
	1- Data length = 0x03
	2- First byte is commend number = 0x15
	3- Second byte is the number of page
	4- Third Byte equals the number of pages
	*/
static void Bootloader_Erase_Flash (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
	/* To check on Erase state */
	uint8_t Erase_Status = UNSUCESSFUL_ERASE ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Mase erase or page erase of the user flash \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Send acknowledge to host */
		//Bootloader_Send_ACK(1);
		/* Perform Mass erase or sector erase of the yser flash */
		Erase_Status = Perform_Flash_Erase (Host_Buffer[2],Host_Buffer[3]);
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
		/* Report the erase state */
		//Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);
	/*}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_PayLoad,
		uint32_t Payload_Start_Address, uint16_t Payload_Len) {
	/* The status in dealing HAL functions */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR;
	/* Status writing in flash memory */
	uint8_t Status = FLASH_PAYLOAD_WRITING_FAILED;
	/* The number of words in data appliction sections */
	uint16_t PayLoad_Counter = 0;
	/* Writing steps */
	/* Open flash memory */
	HAL_Status = HAL_FLASH_Unlock();
	/* If it opened */
	if (HAL_Status == HAL_OK) {
		/* Transfer the data sections word by word */
		for (PayLoad_Counter = 0; PayLoad_Counter < Payload_Len;
				PayLoad_Counter += 4) {
			HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
					Payload_Start_Address + PayLoad_Counter,
					Host_PayLoad[PayLoad_Counter]);
			/* if function can't write on memory Status be failed */
			if (HAL_Status != HAL_OK) {
				Status = FLASH_PAYLOAD_WRITING_FAILED;
				break;
			} else {
				/* All iterations, It can write on memory make status passed */
				Status = FLASH_PAYLOAD_WRITING_PASSED;
			}
		}
	} else {
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

/* To write on specific byte
   To delay with this commend you should send :
	1- Data length = 0x07
	2- First byte is commend number = 0x16
	3- Wanted address = (4bytes) --> you should enter address in Realterm as this example
	   0x0800014c --enter--> 0x4c then 0x01 then 0x00 then 0x08
	4- Third Byte equals the number of bytes that you want to write in
	5- Enter in fourth byte the data that you want to write
	*/
static void Bootloader_Memory_Write (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
	/* Base address that you will write on */
	uint32_t Host_Address = 0 ;
	/* Number of bytes that will be sent */
	uint8_t PayLoad_Len = 0 ;
	/* The status of input address from the host */
	uint8_t Address_Verfication = ADDRESS_IS_INVALID ;
	/* Status writing in flash memory */
	uint8_t Status = FLASH_PAYLOAD_WRITING_FAILED ;
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Write data into different memory \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if (CRC_OK == Bootloader_CRC_Verify(Host_Buffer,(Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)) {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Send acknowledge to host */
		//Bootloader_Send_ACK(1);
		/* Read Base address */
		Host_Address = *((uint32_t *)(&Host_Buffer[2])) ;
		/* Extract the size of new data in memory */
		PayLoad_Len = Host_Buffer[6] ;

		/* Verify the given address */
		Address_Verfication = Host_Jump_Address_Verfication(Host_Address);

		if (Address_Verfication == ADDRESS_IS_VALID){
			/* Write on memory */
			Status = Flash_Memory_Write_Payload((uint8_t *)&Host_Buffer[7], Host_Address, PayLoad_Len);
			/* Report the writing state */
			//Bootloader_Send_Data_To_Host((uint8_t *)(&Status),1);
			BL_PrintMassage ("Correct writing data into memory at %x \r\n",Host_Address);
		}
		else {
			BL_PrintMassage ("Incorrect writing data into memory at %x \r\n",Host_Address);
		}
		/* Report the address selection state */
		//Bootloader_Send_Data_To_Host((uint8_t *)(&Address_Verfication),1);
	/*}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

static uint8_t Change_ROP_Level(uint8_t ROP_Level){
	/* Status of function calling */
	HAL_StatusTypeDef HAL_Status = HAL_ERROR ;
	/* Parameter[in] of function that configure level of protection */
	FLASH_OBProgramInitTypeDef FLASH_OBProgmInit;
	/* Status of this function */
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;

	/* open flash option control registers acess */
	HAL_Status = HAL_FLASH_OB_Unlock();
	/* check if it is not opened correctly */
	if (HAL_Status != HAL_OK){
		ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
	}
	else {
		/* Initialize configurations of parameter[in] for configure level of protection */
		FLASH_OBProgmInit.OptionType = OPTIONBYTE_RDP ; /*!<RDP option byte configuration*/
		FLASH_OBProgmInit.Banks      = FLASH_BANK_1 ;
		FLASH_OBProgmInit.RDPLevel   = (uint32_t)ROP_Level ;
		/* Program option bytes */
		HAL_Status = HAL_FLASHEx_OBProgram(&FLASH_OBProgmInit);
		/* Check that it was failed */
		if (HAL_Status != HAL_OK){
			/* Locking the flash option control registers access */
			HAL_Status = HAL_FLASH_OB_Lock();
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
		}
		else {
			/* Launch the option byte loading */
			HAL_FLASH_OB_Launch();
			/* Locking the flash option control registers access */
			HAL_Status = HAL_FLASH_OB_Lock();
			/* Check that it was failed */
			if (HAL_Status != HAL_OK){
				ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
			}
			else {
				/* Locking the flash option control registers access */
				ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
			}
		}
	}
	return ROP_Level_Status ;
}

/* To write on specific byte
   To delay with this commend you should send :
	1- Data length = 0x02
	2- First byte is commend number = 0x17
	3- Second byte to enter your level 0,1
	*/
static void Bootloader_Change_Read_Protection_Level(uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	//uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	//uint32_t Host_CRC32 = 0 ;
	/* Status of this function */
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
	/* read protection level */
	 uint8_t ROP_Level = 0 ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Change the flash protection out level \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	//Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	//Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	/*if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif*/
		/* Report acknowledge message*/
		//Bootloader_Send_ACK(1);
		/* Assign read protection level from buffer */
		ROP_Level = Host_Buffer[2] ;
		/* reject operation if it was level 2 protection */
		if (ROP_Level == 2){
			ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
			BL_PrintMassage ("Rejected to be in level 2\r\n");
		}
		else {
			if ( ROP_Level == 0 ){
				/* 0xAA --> RDP_Level_0 */
				ROP_Level = 0xAA ;
				ROP_Level_Status = ROP_LEVEL_CHANGE_VALID ;
			}
			else if ( ROP_Level == 1 ) {
				/* 0x55 --> RDP_Level_1 */
				ROP_Level = 0x55 ;
				ROP_Level_Status = ROP_LEVEL_CHANGE_VALID ;
			}
			else {
				ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID ;
				BL_PrintMassage ("Invalid input level\r\n");
			}
			/* Request change the read protection level */
			ROP_Level_Status = Change_ROP_Level( (uint8_t) ROP_Level);

			if (ROP_Level_Status == ROP_LEVEL_CHANGE_VALID){
				BL_PrintMassage ("Sucessful changed level\r\n");
			}
			else {
				BL_PrintMassage ("Unsucessful changed level\r\n");
			}
		}
		//Bootloader_Send_Data_To_Host((uint8_t *)(&ROP_Level_Status),1);
	/*}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}*/
}

void BL_PrintMassage(char *format, ...) {
	char Message[100] = { 0 };
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


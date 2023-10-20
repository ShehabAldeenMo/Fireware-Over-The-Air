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
static void Bootloader_Enable_RW_Protection (uint8_t *Host_Buffer);
static void Bootloader_Memory_Read (uint8_t *Host_Buffer);
static void Bootloader_Get_Sector_Protection_Status (uint8_t *Host_Buffer);
static void Bootloader_Read_OTP (uint8_t *Host_Buffer);
static void Bootloader_Disable_RW_Protection (uint8_t *Host_Buffer);

static CRC_Status Bootloader_CRC_Verify(uint8_t *pData , uint8_t Data_Len, uint32_t Host_CRC);
static void Bootloader_Send_ACK (uint8_t Reply_Length);
static void Bootloader_Send_NACK();
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer , uint32_t Data_Len);
static void Bootloader_Jump_To_User_App (uint8_t *Host_Buffer);
static uint8_t Host_Jump_Address_Verfication (uint32_t Jump_Address);

/*===================Static global Variables Definations  ==================*/
static uint8_t BL_HostBuffer[BL_HOST_BUFFER_RX_LENGTH];

/* All supported commends by bootloaders */
static uint8_t Bootloader_Supported_CMDs[12] = {
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
	CBL_DIS_R_W_PROTECT_CMD,
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
					BL_PrintMassage ("CBL_GET_RDP_STATUS_CMD \r\n ");
					Bootloader_Read_Protection_Level(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_GO_TO_ADDER_CMD :
					Bootloader_Jump_To_Address(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_FLASH_ERASE_CMD :
					BL_PrintMassage ("CBL_FLASH_ERASE_CMD \r\n ");
					Bootloader_Erase_Flash(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_MEM_WRITE_CMD :
					BL_PrintMassage ("CBL_MEM_WRITE_CMD \r\n ");
					Bootloader_Memory_Write(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_EN_R_W_PROTECT_CMD :
					BL_PrintMassage ("CBL_EN_R_W_PROTECT_CMD \r\n ");
					Bootloader_Enable_RW_Protection(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_MEM_READ_CMD :
					BL_PrintMassage ("CBL_MEM_READ_CMD \r\n ");
					Bootloader_Memory_Read(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_READ_SECTOR_STATUS_CMD :
					BL_PrintMassage ("CBL_READ_SECTOR_STATUS_CMD \r\n ");
					Bootloader_Get_Sector_Protection_Status(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_OTP_READ_CMD :
					BL_PrintMassage ("CBL_OTP_READ_CMD \r\n ");
					Bootloader_Read_OTP(BL_HostBuffer);
					Status = BL_OK ;
					break;
				case CBL_DIS_R_W_PROTECT_CMD :
					BL_PrintMassage ("CBL_DIS_R_W_PROTECT_CMD \r\n ");
					Bootloader_Disable_RW_Protection(BL_HostBuffer);
					Status = BL_OK ;
					break;
				default :
					BL_PrintMassage ("Invalid commend code recieved from host !! \r\n ");
					Bootloader_Get_Help(BL_HostBuffer);
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
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART, Host_Buffer, Data_Len, HAL_MAX_DELAY);
}

/* To get version of bootloader to check on the leatest version */
static void Bootloader_Get_Version (uint8_t *Host_Buffer){
	/* Buffering the version and vendor id's in BL_Version */
	uint8_t BL_Version[4] = { CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION,
			CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION };
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read bootloader version \r\n ");
#endif
	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Sending Acknowledge message and number of bytes which will be sent */
		Bootloader_Send_ACK(4);
		/* Sending the version and vendor id's to meet the target from commend */
		Bootloader_Send_Data_To_Host(BL_Version,4);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Bootloader Ver. %d.%d.%d\r\n",BL_Version[1],BL_Version[2],BL_Version[3]);
#endif
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
}
static void Bootloader_Get_Help (uint8_t *Host_Buffer){
	uint16_t Host_CMD_Packet_Len = 0 ;  /* used to define the beginning of CRC address in buffer */
	uint32_t Host_CRC32 = 0 ;           /* Used to get CRC data */

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("What are the commends supported by the bootloader --> All supported commends code \r\n ");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Sending Acknowledge message and number of bytes which will be sent */
		Bootloader_Send_ACK(12);
		/* Sending the list of commends to meet the target from commend */
		Bootloader_Send_Data_To_Host(Bootloader_Supported_CMDs,12);

		Bootloader_Jump_To_User_App(Host_Buffer);
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
}
static void Bootloader_Get_chip_Identification_Number (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
	/* Identify the id of used MCU */
	uint16_t MCU_IdentificationNumber = 0 ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read MCU chip identification number \r\n ");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Get MCU chip identification number */
		MCU_IdentificationNumber = (uint16_t)((DBGMCU->IDCODE)&0x00000FFF);

		/* Report MCU chip identification number */
		Bootloader_Send_ACK(2);
		Bootloader_Send_Data_To_Host((uint8_t *)(&MCU_IdentificationNumber),2);
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
}
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer){

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

static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer){
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
	/* Buffering address */
	uint32_t Host_Jump_Address = 0 ;
	/* TO check on state of given address is in region or not */
	uint8_t Address_Verification_State = ADDRESS_IS_INVALID ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("CBL_GO_TO_ADDER_CMD \r\n ");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Report MCU chip identification number */
		Bootloader_Send_ACK(1);

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
		/* Report the verification state */
		Bootloader_Send_Data_To_Host((uint8_t *)(&Address_Verification_State),1);
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
}

static void Bootloader_Jump_To_User_App (uint8_t *Host_Buffer){
	/* Value of the main stack pointer of our main application */
	uint32_t MSP_Value = *((volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS) ;
	/* Reset Handler defination function of our main application */
	uint32_t MainAppAddr = *((volatile uint32_t *)FLASH_SECTOR2_BASE_ADDRESS+4) ;

	/* Declare pointer to function contain the beginning address of application */
	pFunc ResetHandler_Address = (pFunc)MainAppAddr;

	/* Reset main stack pointer */
	__set_MSP(MSP_Value);

	/* Deinitionalization of modules that used in bootloader and work
	   the configurations of new application */
	HAL_RCC_DeInit(); /* Resets the RCC clock configuration to the default reset state. */

	/* Jump to Apllication Reset Handler */
	ResetHandler_Address();
}

static void Bootloader_Erase_Flash (uint8_t *Host_Buffer){

}
static void Bootloader_Memory_Write (uint8_t *Host_Buffer){

}
static void Bootloader_Enable_RW_Protection (uint8_t *Host_Buffer){

}
static void Bootloader_Memory_Read (uint8_t *Host_Buffer){

}
static void Bootloader_Get_Sector_Protection_Status (uint8_t *Host_Buffer){

}
static void Bootloader_Read_OTP (uint8_t *Host_Buffer){

}
static void Bootloader_Disable_RW_Protection (uint8_t *Host_Buffer){

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
	HAL_UART_Transmit(BL_DEBUG_UART, (uint8_t*) Message, sizeof(Message),
	HAL_MAX_DELAY);
#elif  BL_DEBUG_METHOD == BL_ENABLE_CAN_DEBUG_MESSAGE
	/* Transmit the formatted data through the defined CAN */
#elif  BL_DEBUG_METHOD == BL_ENABLE_ETHERNET_DEBUG_MESSAGE
	/* Transmit the formatted data through the defined ETHERNET */
#endif
	/* Perform cleanup for an ap object initialized by a call to va_start */
	va_end(args);
}


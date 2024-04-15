/*
 * Name       : Bootloader.c
 * Created on : Oct 18, 2023
 * Author     : Shehab aldeen mohammed
 * Github     : https://github.com/ShehabAldeenMo
 * linkdin    : https://www.linkedin.com/in/shehab-aldeen-mohammed-0b3207228/
 */

/*============================ Includes  =============================*/
#include "Bootloader.h"
#include "Bootloader_Config.h"


/*===============  Functions Declerations  =====================*/
static void Bootloader_Get_Version (uint8_t *Host_Buffer);
static void Bootloader_Get_Help (uint8_t *Host_Buffer);
static void Bootloader_Get_chip_Identification_Number (uint8_t *Host_Buffer);
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer);
static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash (uint8_t *Host_Buffer);
static void Bootloader_Memory_Write (uint8_t *Host_Buffer);
#if PYTHON == 1
static CRC_Status Bootloader_CRC_Verify(uint8_t *pData , uint8_t Data_Len, uint32_t Host_CRC);

static void Bootloader_Send_ACK (uint8_t Reply_Length);
static void Bootloader_Send_NACK();
#endif
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer , uint32_t Data_Len);
static void Bootloader_Enable_R_Protection(uint8_t *Host_Buffer);
static void Bootloader_Enable_W_Protection(uint8_t *Host_Buffer);
static void Bootloader_SetApplication_Flag(uint8_t *Host_Buffer);

static uint8_t CBL_STM32F103_GET_RDP_Level ();
static uint8_t CBL_STM32F103_GET_WDP_Level ();
static uint8_t Host_Jump_Address_Verfication (uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase (uint32_t PageAddress, uint8_t Number_Of_Pages);
static uint8_t Flash_Memory_Write_Payload (uint8_t *Host_PayLoad , uint32_t Payload_Start_Address,uint8_t Payload_Len);
static void Bootloader_Jump_To_User_App1 ();
static void Bootloader_Jump_To_User_App2 ();
static void BL_Manager(void);

void CAN_Select_Func(void);
void CAN_Read_WP_Level(void);
void CAN_Read_RP_Level(void);
void CAN_ReceiveData(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *Header, uint8_t RxData[], uint16_t Length); //We make this function to handle sending large files that are bigger than MAX_DATA_CAN_LENGTH
void CAN_TransmitData(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *Header, uint8_t TxData[], uint16_t Length); //We make this function to handle Receiving large files that are bigger than MAX_DATA_CAN_LENGTH
void CAN_Receive_AllPendingMessages_FIFO0(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *Header, CAN_Receive_Buffer* RxData);
void CAN_Receive_AllPendingMessages_FIFO1(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *Header, CAN_Receive_Buffer* RxData);
void MemCopy(uint8_t* Dest_Ptr, uint8_t* Source_Ptr, uint32_t Length);





/*===================Static global Variables Definations  ==================*/
static uint8_t BL_HostBuffer[BL_HOST_BUFFER_RX_LENGTH];
static uint8_t Global_Calling = 0 ;
FLASH_OBProgramInitTypeDef pOBInit;
uint32_t APP1_Flag __attribute__((section(".APPLICATION1_SECTION"))); //To read counter of Full TI in Flash
uint32_t APP2_Flag __attribute__((section(".APPLICATION2_SECTION"))); //To read counter of Full TI in Flash


extern uint8_t BL_FiFO0_Flag;
#if(BL_CAN == ENABLE)
CAN_TxHeaderTypeDef BL_TxHeader;
CAN_RxHeaderTypeDef BL_RxHeader;
CAN_RxHeaderTypeDef BL_RxHeaderReceive;

uint32_t BL_TxMailbox[4];

uint8_t BL_TxData[8];
uint8_t BL_RxData[8];

uint8_t BL_CAN_ReceiveCounter0 = 0;
uint8_t BL_CAN_ReceiveFlag0 = 0;
uint8_t BL_CAN_ByteCounter0 = 0;

#endif

/* All supported commend-pos by bootloaders */
static uint8_t Bootloader_Supported_CMDs[NumberOfCommends] = {
		CBL_GET_VER_CMD ,
		CBL_GET_HELP_CMD ,
		CBL_GET_CID_CMD ,
		CBL_GET_RDP_STATUS_CMD,
		CBL_GO_TO_ADDER_CMD,
		CBL_FLASH_ERASE_CMD,
		CBL_MEM_WRITE_CMD,
		CBL_CHANGE_ROP_Level_CMD,
		JUMP_TO_APPLICATION_CMD,
		CBL_CHANGE_WOP_Level_CMD
};

static BL_pFunc Bootloader_Functions [NumberOfCommends] = {&Bootloader_Get_Version,
		&Bootloader_Get_Help,&Bootloader_Get_chip_Identification_Number,&Bootloader_Read_Protection_Level,
		&Bootloader_Jump_To_Address,&Bootloader_Erase_Flash,&Bootloader_Memory_Write,&Bootloader_Enable_R_Protection,
		&Bootloader_SetApplication_Flag,&Bootloader_Enable_W_Protection} ;


/*======================== Software Interface Definations  ====================*/
BL_Status BL_Fetch_Commend(void) {

#if((BL_CAN == ENABLE) && (ECU_CONFIG == SLAVE1))
	/*Status of BL process*/
	BL_Status Status = BL_OK;

	/*BL_Manger to jump to dedicated app if needed*/
	//BL_Manager(); //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Don't forget to uncomment this line after testing to enable CAN bootloader manager

	/*Allow Callback FIFO0 MSG pending to execute, to Handle both receiving with FIFO1(Polling) and FIFO0(interrupt) */
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING |CAN_IT_ERROR |CAN_IT_LAST_ERROR_CODE);

	/*Polling on CAN frames that come from FIFO1(FIFO1 is designed for remote frames)*/
	while(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO1) == 0) //Polling until there is any message received
	{
		if(BL_FiFO0_Flag != 0) //Break from infinite loop if message stored in FIFO0(BL_FiFO0_Flag is set if FIFO0_Msg_pending call back is called)
		{
			break;
		}
	}

	/*Receive the message that stored in either FIFO1 or FIFO0*/
	if(BL_FiFO0_Flag != 0) //Means message stored in FIFO0
	{
		BL_FiFO0_Flag = 0;
		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &BL_RxHeader, BL_RxData); //Receive from FIFO0
	}
	else //Means message stored in FIFO1
	{
		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO1, &BL_RxHeader, BL_RxData); //Receive from FIFO1
	}

	/*Select which BL_Function to call based on message_ID and Filter_match_index*/
	CAN_Select_Func();

	return Status;
#else
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

		/* if it don't recieve correctly */
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
#endif
}


void Write_Program_Flag(uint32_t Address, uint32_t Value)
{
	uint32_t status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, (uint64_t)Value);
}



uint32_t Get_Program_Flag_Status(uint32_t Address)
{
	uint32_t Status = (*(uint32_t*)Address);
	return Status;
}



#if PYTHON == 1
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

	/* Check that CRC is OK */
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
	uint8_t NACK_Value = CBL_SEND_NACK;
	Bootloader_Send_Data_To_Host(&NACK_Value, 1);
}
#endif

/* Function to communicate with host */
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer , uint32_t Data_Len){
	HAL_UART_Transmit(BL_HOST_COMMUNICATION_UART,(uint8_t*) Host_Buffer,(uint16_t)Data_Len, HAL_MAX_DELAY);
}

/*
 your packet is
   1- 1 byte for data length = 0x05
   2- 1 byte for commend number = 0x10
   3- 4 bytes for CRC verifications if we used python code as host
 */
static void Bootloader_Get_Version (uint8_t *Host_Buffer){
#if PYTHON == 1
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
#endif

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read bootloader version \r\n");
#endif

#if PYTHON == 1
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
#endif
#if((BL_CAN == ENABLE) && (ECU_CONFIG == SLAVE1))
		/*Create array to send Version through it*/
		const uint8_t VER[4] = { CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION,
				CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};

		/*Create and initialise TxHeader*/
		CAN_TxHeaderTypeDef BL_VerTxHeader;
		BL_VerTxHeader.DLC = 4;
		BL_VerTxHeader.ExtId = 0;
		BL_VerTxHeader.IDE = CAN_ID_STD;
		BL_VerTxHeader.RTR = CAN_RTR_DATA;
		BL_VerTxHeader.StdId = CAN_VER_RESP_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
		BL_VerTxHeader.TransmitGlobalTime = DISABLE;

		/*Send Version CAN frame*/
		HAL_CAN_AddTxMessage(&hcan, &BL_VerTxHeader, VER, BL_TxMailbox);
		while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.

#else
		if(MASTERID == Host_Buffer[2])
		{

#if(ECU_CONFIG == MASTERID)
			/* Sending the version and vendor id's to meet the target from command */
			uint8_t BL_Version[4] = { CBL_VENDOR_ID, CBL_SW_MAJOR_VERSION,
					CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};

			Bootloader_Send_Data_To_Host(BL_Version,4);
#elif(ECU_CONFIG == SLAVE1)
			/*Send error frame*/
#endif

		}
		else if(SLAVE1 == Host_Buffer[2])
		{
#if((BL_CAN == ENABLE) && (ECU_CONFIG == MASTERID))
			/*Transmit CAN VER(MASTER)*/
			uint8_t BL_Version[8];//Length must be 8 or more or undefined behaviour may occur
			CAN_TxHeaderTypeDef BL_VerTxHeader;

			BL_VerTxHeader.DLC = 4;
			BL_VerTxHeader.ExtId = 0;
			BL_VerTxHeader.IDE = CAN_ID_STD;
			BL_VerTxHeader.RTR = CAN_RTR_REMOTE;
			BL_VerTxHeader.StdId = CAN_VER_REQ_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
			BL_VerTxHeader.TransmitGlobalTime = DISABLE;

			HAL_CAN_AddTxMessage(&hcan, &BL_VerTxHeader, BL_Version, BL_TxMailbox);
			while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.

			/*Receive CAN VER(MASTER)*/
			CAN_RxHeaderTypeDef BL_VerRxHeader;

			do
			{
				while(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0); //Polling until there is any message received
				HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &BL_VerRxHeader, BL_Version);

			}while(BL_VerRxHeader.StdId != CAN_VER_RESP_ID);


			/*Send Version to ESP*/
			Bootloader_Send_Data_To_Host(BL_Version, 4);


#elif(ECU_CONFIG == MASTERID)

			uint8_t Version[4];

			HAL_UART_Transmit(&huart2, &Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
			HAL_UART_Transmit(&huart2, &Host_Buffer[1], 2, 10); //sending remaining frame, The time must be 10 to avoid errors

			HAL_UART_Receive(&huart2, Version, 4, HAL_MAX_DELAY); //Receive version

			Bootloader_Send_Data_To_Host(Version, 4);
#elif(ECU_CONFIG == SLAVE1)
			uint8_t BL_Version[4] = { CBL_VENDOR_SLAVE_ID, CBL_SW_MAJOR_VERSION,
					CBL_SW_MINOR_VERSION, CBL_SW_PATCH_VERSION};

			Bootloader_Send_Data_To_Host(BL_Version, 4);

#endif

		}
		else
		{
			/*Send Frame error*/
		}

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Bootloader Version %d.%d.%d\r\n",BL_Version[1],BL_Version[2],BL_Version[3]);
#endif

#if PYTHON == 1
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
#endif

#endif
}

/*
 Your packet is :
   1- 1 byte data length = 0x05
   2- 1 byte commend number = 0x11
   3- 4 bytes for CRC verifications if we used python code as host
 */
static void Bootloader_Get_Help (uint8_t *Host_Buffer){

#if PYTHON == 1
	uint16_t Host_CMD_Packet_Len = 0 ;  /* used to define the beginning of CRC address in buffer */
	uint32_t Host_CRC32 = 0 ;           /* Used to get CRC data */
#endif

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("All supported commends code \r\n");
#endif

	/* Extract the CRC32 and Packet length sent by the HOST */
#if PYTHON == 1
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Sending Acknowledge message and number of bytes which will be sent */
		Bootloader_Send_ACK(NumberOfCommends);
#endif
		/* Sending the list of commends to meet the target from commend */
		Bootloader_Send_Data_To_Host(Bootloader_Supported_CMDs,NumberOfCommends);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage(
				"CBL_GET_VER_CMD 0x%x\r\nCBL_GET_HELP_CMD 0x%x\r\nCBL_GET_CID_CMD 0x%x\r\n",
				Bootloader_Supported_CMDs[0], Bootloader_Supported_CMDs[1],
				Bootloader_Supported_CMDs[2]);
		BL_PrintMassage(
				"CBL_GET_RDP_STATUS_CMD 0x%x\r\nCBL_GO_TO_ADDER_CMD 0x%x\r\nCBL_FLASH_ERASE_CMD 0x%x\r\n",
				Bootloader_Supported_CMDs[3], Bootloader_Supported_CMDs[4],
				Bootloader_Supported_CMDs[5]);
		BL_PrintMassage(
				"CBL_MEM_WRITE_CMD 0x%x\r\nCBL_EN_R_W_PROTECT_CMD 0x%x\r\nCBL_MEM_READ_CMD 0x%x\r\n",
				Bootloader_Supported_CMDs[6], Bootloader_Supported_CMDs[7],
				Bootloader_Supported_CMDs[8]);
		BL_PrintMassage(
				"CBL_READ_SECTOR_STATUS_CMD 0x%x\r\nCBL_OTP_READ_CMD 0x%x\r\nCBL_CHANGE_ROP_Level_CMD 0x%x\r\n",
				Bootloader_Supported_CMDs[9], Bootloader_Supported_CMDs[10],
				Bootloader_Supported_CMDs[11]);
#endif

#if PYTHON == 1
	}
	else
	{
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
#endif
}

/*
 Your packet is :
   1- 1 byte for data length = 0x05
   2- 1 byte for commend number = 0x12
   3- 4 bytes for CRC verifications if we used python code as host
 */
static void Bootloader_Get_chip_Identification_Number (uint8_t *Host_Buffer){
#if PYTHON == 1
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
#endif
	/* Identify the id of used MCU */
	uint16_t MCU_IdentificationNumber = RESET ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read MCU chip identification number \r\n");
#endif

#if PYTHON == 1
	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Report MCU chip identification number */
		Bootloader_Send_ACK(2);
#endif

		/* Get MCU chip identification number */
		MCU_IdentificationNumber = (uint16_t)((DBGMCU->IDCODE)&0x00000FFF);

		Bootloader_Send_Data_To_Host((uint8_t *)(&MCU_IdentificationNumber),2);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("IdentificationNumber = %x\r\n",MCU_IdentificationNumber);
#endif

#if PYTHON == 1
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
#endif
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
   1- 1 byte data length = 0x05
   2- 1 byte commend number = 0x13
   3- 4 bytes for CRC verifications if we used python code as host
 */
static void Bootloader_Read_Protection_Level (uint8_t *Host_Buffer){
#if PYTHON == 1
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
#endif
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Read the flash protection out level \r\n");
#endif

#if PYTHON == 1
	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Report acknowledge message*/
		Bootloader_Send_ACK(1);
#endif



		if(READ_RP == Host_Buffer[2])
		{
#if((BL_CAN == ENABLE) && (ECU_CONFIG == MASTERID))
			/*Transmit CAN Read RP level Request*/
			uint8_t BL_Read_Level[8]; //Length must be 8 or more or undefined behaviour may occur

			CAN_TxHeaderTypeDef BL_RP_TxHeader;

			BL_RP_TxHeader.DLC = 1;
			BL_RP_TxHeader.ExtId = 0;
			BL_RP_TxHeader.IDE = CAN_ID_STD;
			BL_RP_TxHeader.RTR = CAN_RTR_REMOTE;
			BL_RP_TxHeader.StdId = CAN_READ_RP_REQ_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
			BL_RP_TxHeader.TransmitGlobalTime = DISABLE;

			HAL_CAN_AddTxMessage(&hcan, &BL_RP_TxHeader, BL_Read_Level, BL_TxMailbox);
			while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.


			/*Transmit CAN Read RP level Response*/
			CAN_RxHeaderTypeDef BL_Read_RP_RxHeader;

			do{
				/*Read response VER message*/
				while(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0); //Polling until there is any message received
				HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &BL_Read_RP_RxHeader, BL_Read_Level);

			}while(BL_RP_TxHeader.StdId != CAN_READ_RP_RESP_ID);


			/*Sending RP to ESP*/
			Bootloader_Send_Data_To_Host((uint8_t *)(&BL_Read_Level),1);

#else
			uint8_t RDP_Level = CBL_STM32F103_GET_RDP_Level();

			/* Report level */
			Bootloader_Send_Data_To_Host((uint8_t *)(&RDP_Level),1);
#endif
		}
		else if(READ_WP == Host_Buffer[2])
		{
#if((BL_CAN == ENABLE) && (ECU_CONFIG == MASTERID))
			/*Transmit CAN Read RP level Request*/
			uint8_t BL_Read_Level[8]; //Length must be 8 or more or undefined behaviour may occur

			CAN_TxHeaderTypeDef BL_WP_TxHeader;

			BL_WP_TxHeader.DLC = 1;
			BL_WP_TxHeader.ExtId = 0;
			BL_WP_TxHeader.IDE = CAN_ID_STD;
			BL_WP_TxHeader.RTR = CAN_RTR_REMOTE;
			BL_WP_TxHeader.StdId = CAN_READ_WP_REQ_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
			BL_WP_TxHeader.TransmitGlobalTime = DISABLE;

			HAL_CAN_AddTxMessage(&hcan, &BL_WP_TxHeader, BL_Read_Level, BL_TxMailbox);
			while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.


			/*Transmit CAN Read RP level Response*/
			CAN_RxHeaderTypeDef BL_Read_WP_RxHeader;

			do
			{
				/*Read response VER message*/
				while(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0); //Polling until there is any message received
				HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &BL_Read_WP_RxHeader, BL_Read_Level);

			}while(BL_Read_WP_RxHeader.StdId != CAN_READ_WP_RESP_ID);

			/*Sending RP to ESP*/
			Bootloader_Send_Data_To_Host((uint8_t *)(&BL_Read_Level),1);
#else

			uint8_t	WDP_Level = CBL_STM32F103_GET_WDP_Level();

			/* Report level */
			Bootloader_Send_Data_To_Host((uint8_t *)(&WDP_Level),1);
#endif
		}
		else
		{
			/*Send Frame Error*/
			Bootloader_Send_Data_To_Host(0xEE, 1); //Sending any value other than OB_RDP_LEVEL_0, OB_RDP_LEVEL_1, 0x00, 0xFF is considered error frame in this case
		}
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("Protection level = %x\r\n",RDP_Level);
#endif

#if PYTHON == 1
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
#endif
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

/*
 Your packet is :
   1- 1 byte data length = 0x09
   2- 1 byte commend number = 0x14
   3- 4 bytes for address
   4- 4 bytes for CRC verifications if we used python code as host
 */
static void Bootloader_Jump_To_Address (uint8_t *Host_Buffer){
#if PYTHON == 1
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
#endif

	/* Buffering address */
	uint32_t Host_Jump_Address = RESET ;
	/* TO check on state of given address is in region or not */
	uint8_t Address_Verification_State = ADDRESS_IS_INVALID ;

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Bootloader jump to specified address \r\n");
#endif

#if PYTHON == 1
	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Report acknowledge message */
		Bootloader_Send_ACK(1);
#endif

		/* To get the content of Host_Buffer and variable"Host_Jump_Address" realizes that it is address
		 - &Host_Buffer[2] --> express the address of array of host
		 - (uint32_t *)    --> casting it to pointer of uint32
		 - *               --> De-reference it and get the content of buffer at this element
		 */
		Host_Jump_Address = *((uint32_t *) &(Host_Buffer[2])) ;

		/* To verify that the address in the region of memory */
		Address_Verification_State = Host_Jump_Address_Verfication(Host_Jump_Address);

		if (Address_Verification_State == ADDRESS_IS_VALID ){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage("Address verification sucessed\r\n");
#endif
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification_State, 1);

			if (Host_Jump_Address == FLASH_PAGE_BASE_ADDRESS_APP1){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage("Jump To Application\r\n");
#endif
				Bootloader_Jump_To_User_App1();
			}
			else if (Host_Jump_Address == FLASH_PAGE_BASE_ADDRESS_APP2){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage("Jump To Application\r\n");
#endif
				Bootloader_Jump_To_User_App2();
			}
			else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
				BL_PrintMassage("Jumped to : 0x%X \r\n",Host_Jump_Address);
#endif
				/* - Prepare the address to jump
				    - Increment 1 to be in thumb instruction */
				Jump_Ptr Jump_Address = (Jump_Ptr) (Host_Jump_Address + 1) ;
				Jump_Address();
			}
		}
		else {
			Bootloader_Send_Data_To_Host((uint8_t *)&Address_Verification_State, 1);
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
			BL_PrintMassage("Address verification unsucessed\r\n");
#endif
		}
#if PYTHON == 1
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
#endif
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
   5- 4 bytes for CRC verifications if we used python code as host
 */
static void Bootloader_Erase_Flash (uint8_t *Host_Buffer){
#if PYTHON == 1
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0 ;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0 ;
#endif
#if((BL_CAN == ENABLE) && (ECU_CONFIG == SLAVE1))
	/* To check on Erase state */
	uint8_t Erase_Status = UNSUCESSFUL_ERASE ;

	/*Erase memory from begin address in &Host_Buffer[1] and with amount of pages in Host_Buffer[0]*/
	Erase_Status = Perform_Flash_Erase (*((uint32_t*)&Host_Buffer[1]), Host_Buffer[0]);

	/*Send Ack Erase status to Master*/
	CAN_TxHeaderTypeDef BL_Erase_TxHeader;

	BL_Erase_TxHeader.DLC = 1;
	BL_Erase_TxHeader.ExtId = 0;
	BL_Erase_TxHeader.IDE = CAN_ID_STD;
	BL_Erase_TxHeader.RTR = CAN_RTR_DATA;
	BL_Erase_TxHeader.StdId = CAN_ERASE_MEMORY_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
	BL_Erase_TxHeader.TransmitGlobalTime = DISABLE;

	HAL_CAN_AddTxMessage(&hcan, &BL_Erase_TxHeader, &Erase_Status, BL_TxMailbox);
	while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.
#else
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
	BL_PrintMassage ("Mase erase or page erase of the user flash \r\n");
#endif

#if PYTHON == 1
	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verfications */
	if ( CRC_OK == Bootloader_CRC_Verify(Host_Buffer, (Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32)){
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
#endif
		/* Send acknowledge to host */
		Bootloader_Send_ACK(1);
#endif

		/* To check on Erase state */
		uint8_t Erase_Status = UNSUCESSFUL_ERASE ;

		/* Perform Mass erase or sector erase of the user flash */
		if (Host_Buffer[6] == MASTERID )
		{
#if(ECU_CONFIG == MASTERID)
			Erase_Status = Perform_Flash_Erase (*((uint32_t*)(&Host_Buffer[2])), Host_Buffer[7]);

			/* Report the erase state */
			Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);

#elif(ECU_CONFIG == SLAVE1)
			Erase_Status = UNSUCESSFUL_ERASE ;
			Bootloader_Send_Data_To_Host((uint8_t *)(&Erase_Status),1);
#endif
		}
		else if(Host_Buffer[6] == SLAVE1 )
		{
#if((BL_CAN == ENABLE) && (ECU_CONFIG == MASTERID))
			/*Create buffer contains Amount of data and start address to be erased
			 *Length of buffer(when it act as buffer receiver) must be 8 or more, otherwise undefined behaviour may occur as CAN_Receive() will write data in un-allocated stack memory region
			 * */
			uint8_t BL_Erase_Memory[8];
			BL_Erase_Memory[0] = Host_Buffer[7]; //storing page amount to erase
			*((uint32_t *)(&BL_Erase_Memory[1])) = *((uint32_t*)&Host_Buffer[2]); //storing start address

			/*Transmit Amount of data and Start address needed to be erased through one CAN frame*/
			CAN_TxHeaderTypeDef BL_EraseMemory_TxHeader; //Create Erase_memory struct Txheader for CAN transmit function
			BL_EraseMemory_TxHeader.DLC = 5;
			BL_EraseMemory_TxHeader.ExtId = 0;
			BL_EraseMemory_TxHeader.IDE = CAN_ID_STD;
			BL_EraseMemory_TxHeader.RTR = CAN_RTR_DATA;
			BL_EraseMemory_TxHeader.StdId = CAN_ERASE_MEMORY_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
			BL_EraseMemory_TxHeader.TransmitGlobalTime = DISABLE;

			HAL_CAN_AddTxMessage(&hcan, &BL_EraseMemory_TxHeader, BL_Erase_Memory, BL_TxMailbox);
			while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.

			/*Receiving Ack status from Slave*/
			CAN_RxHeaderTypeDef BL_EraseMemory_RxHeader; //Create Erase_memory struct Rxheader for receive function

			do
			{
				while(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0); //Polling until there is any message received in FIFO0
				HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &BL_EraseMemory_RxHeader, BL_Erase_Memory);
			}while(BL_EraseMemory_RxHeader.StdId != CAN_ERASE_MEMORY_ID); //Neglect other IDs Re-receive if other ID frame is received in FIFO0, until receiving CAN_ERASE_MEMORY_ID frame

			/*Sending Ack status to ESP*/
			Bootloader_Send_Data_To_Host(BL_Erase_Memory, 1);
#elif(ECU_CONFIG == MASTERID)
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
#if PYTHON == 1
	}
	else {
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		Bootloader_Send_NACK();
	}
#endif
#endif
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
													 |( (uint16_t)Host_PayLoad[PayLoad_Counter+1] << TWO_BYTES );

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
   6- 4 bytes for CRC verifications if we used python code as host
 */

static void Bootloader_Memory_Write (uint8_t *Host_Buffer){
#if PYTHON == 1
	/* used to define the beginning of CRC address in buffer */
	uint16_t Host_CMD_Packet_Len = 0;
	/* Used to get CRC data */
	uint32_t Host_CRC32 = 0;
#endif

#if((BL_CAN == ENABLE) && (ECU_CONFIG == SLAVE1))

	/* Base address that you will write on */
	uint32_t HOST_Address = *((uint32_t *)(&Host_Buffer[1]));;
	/* Number of bytes that will be sent */
	uint16_t Payload_Len = Host_Buffer[0];

	/*Receive stream of file data*/
	CAN_RxHeaderTypeDef BL_WritePrograme_RxHeader;
	uint8_t Flash_Program_File_Buffer[100];
	CAN_ReceiveData(&hcan, CAN_RX_FIFO0, &BL_WritePrograme_RxHeader, Flash_Program_File_Buffer, Payload_Len); //Receive from FIFO0 as CAN_FLASH_PROGRAM_ID is mapped to be stored in FIFO0

	/* The status of input address from the host */
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	/* Status writing in flash memory */
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITING_FAILED;

	/*Create and initialise TxHeader*/
	CAN_TxHeaderTypeDef BL_WritePrograme_TxHeader;
	BL_WritePrograme_TxHeader.DLC = 1;
	BL_WritePrograme_TxHeader.ExtId = 0;
	BL_WritePrograme_TxHeader.IDE = CAN_ID_STD;
	BL_WritePrograme_TxHeader.RTR = CAN_RTR_DATA;
	BL_WritePrograme_TxHeader.StdId = CAN_FLASH_PROGRAM_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
	BL_WritePrograme_TxHeader.TransmitGlobalTime = DISABLE;


	Address_Verification = Host_Jump_Address_Verfication(HOST_Address);

	if(Address_Verification == ADDRESS_IS_VALID)
	{
		Flash_Payload_Write_Status = Flash_Memory_Write_Payload(Flash_Program_File_Buffer, HOST_Address, Payload_Len);

		/*Send Ack Status to Master*/
		HAL_CAN_AddTxMessage(&hcan, &BL_WritePrograme_TxHeader, &Flash_Payload_Write_Status, BL_TxMailbox);
		while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.
	}
	else
	{
		/*Send Not Ack to Master due to Address is invalid*/
		HAL_CAN_AddTxMessage(&hcan, &BL_WritePrograme_TxHeader, &Address_Verification, BL_TxMailbox);
		while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.
	}


#else
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

#if PYTHON == 1
	/* Extract the CRC32 and Packet length sent by the HOST */
	Host_CMD_Packet_Len = Host_Buffer[0]+1 ;
	Host_CRC32 = *(uint32_t *)(Host_Buffer + Host_CMD_Packet_Len - CRC_TYPE_SIZE) ;

	/* CRC Verification */
	if(CRC_OK == Bootloader_CRC_Verify(Host_Buffer,(Host_CMD_Packet_Len - CRC_TYPE_SIZE), Host_CRC32))
	{
		/* Send acknowledgement to the HOST */
		Bootloader_Send_ACK(1);
#endif

#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is passed\r\n");
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
#if((BL_CAN == ENABLE) && (ECU_CONFIG == MASTERID))
				/*Create buffer contains Amount of data and start address
				 *Length of buffer(when it act as buffer receiver) must be 8 or more, otherwise undefined behaviour may occur as CAN_Receive() will write data in un-allocated stack memory region
				 * */
				uint8_t BL_Write_Programe[8];
				BL_Write_Programe[0] = Payload_Len;
				*((uint32_t *)(&BL_Write_Programe[1])) = HOST_Address;

				/*Transmit Amount of data and Start address through one CAN frame*/
				CAN_TxHeaderTypeDef BL_WritePrograme_TxHeader; //Create Write_memory struct Txheader for CAN transmit function
				BL_WritePrograme_TxHeader.DLC = 5;
				BL_WritePrograme_TxHeader.ExtId = 0;
				BL_WritePrograme_TxHeader.IDE = CAN_ID_STD;
				BL_WritePrograme_TxHeader.RTR = CAN_RTR_DATA;
				BL_WritePrograme_TxHeader.StdId = CAN_FLASH_PROGRAM_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
				BL_WritePrograme_TxHeader.TransmitGlobalTime = DISABLE;

				HAL_CAN_AddTxMessage(&hcan, &BL_WritePrograme_TxHeader, BL_Write_Programe, BL_TxMailbox);
				while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.

				/*Send Stream of data file(We make this function to handle sending large files that are bigger than MAX_DATA_CAN_LENGTH)*/
				CAN_TransmitData(&hcan, &BL_WritePrograme_TxHeader, (uint8_t *)&Host_Buffer[9], Payload_Len);

				/*Receiving Ack status from Slave*/
				CAN_RxHeaderTypeDef BL_WritePrograme_RxHeader; //Create Write_memory struct Rxheader for receive function

				do
				{
					while(HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0); //Polling until there is any message received
					HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &BL_WritePrograme_RxHeader, BL_Write_Programe);

				}while(BL_WritePrograme_RxHeader.StdId != CAN_FLASH_PROGRAM_ID);

				/*Sending Ack status to ESP*/
				Bootloader_Send_Data_To_Host(BL_Write_Programe, 1);
			}
#elif(ECU_CONFIG == MASTERID)
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
#if PYTHON == 1
	}
	else
	{
#if BL_DEBUG_ENABLE == DEBUG_INFO_ENABLE
		BL_PrintMassage("CRC is failed\r\n");
#endif
		/* Send Not acknowledge to the HOST */
		Bootloader_Send_NACK();
	}
#endif
#endif
}

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
	uint8_t Host_ROP_Level = Host_Buffer[2] ;
	uint8_t ROP_Level_Status = ROP_LEVEL_CHANGE_INVALID;

	if (CBL_ROP_LEVEL_0 == Host_ROP_Level){

		Bootloader_Send_Data_To_Host("Level 0", 8);

		pOBInit.OptionType = OPTIONBYTE_RDP;
		pOBInit.RDPLevel = OB_RDP_LEVEL_0;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();
		HAL_FLASH_OB_Launch();

		ROP_Level_Status = ROP_LEVEL_CHANGE_VALID;
		Bootloader_Send_Data_To_Host((uint8_t *)&ROP_Level_Status, 1);
	}
	else if (CBL_ROP_LEVEL_1 == Host_ROP_Level) {

		Bootloader_Send_Data_To_Host("Level 1", 8);

		pOBInit.OptionType = OPTIONBYTE_RDP;
		pOBInit.RDPLevel = OB_RDP_LEVEL_1;

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();
		HAL_FLASH_OB_Launch();
	}
}

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
	uint8_t WP_Level_Status = ROP_LEVEL_CHANGE_INVALID;
	uint8_t Host_WP_Level = Host_Buffer[2];


	if (OB_WRPSTATE_ENABLE == Host_WP_Level) {

		Bootloader_Send_Data_To_Host("Level 1", 8);

		pOBInit.OptionType = OPTIONBYTE_WRP;
		pOBInit.WRPState = OB_WRPSTATE_ENABLE;
		pOBInit.WRPPage = OB_WRP_PAGES0TO3 | OB_WRP_PAGES4TO7 | OB_WRP_PAGES8TO11 | OB_WRP_PAGES12TO15 | OB_WRP_PAGES16TO19 | OB_WRP_PAGES20TO23 | OB_WRP_PAGES24TO27 | OB_WRP_PAGES28TO31;//We make ORing operations to enable write protect to all pages

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();
		HAL_FLASH_OB_Launch();


	} else if (OB_WRPSTATE_DISABLE == Host_WP_Level) {

		Bootloader_Send_Data_To_Host("Level 0", 8);

		pOBInit.OptionType = OPTIONBYTE_WRP;
		pOBInit.WRPState = OB_WRPSTATE_DISABLE;
		pOBInit.WRPPage = OB_WRP_PAGES0TO3 | OB_WRP_PAGES4TO7 | OB_WRP_PAGES8TO11 | OB_WRP_PAGES12TO15 | OB_WRP_PAGES16TO19 | OB_WRP_PAGES20TO23 | OB_WRP_PAGES24TO27 | OB_WRP_PAGES28TO31;//We make ORing operation to disable write protect to all pages(you should increase OR operation for higher densities STM)

		HAL_FLASH_Unlock();
		HAL_FLASH_OB_Unlock();
		HAL_FLASHEx_OBProgram(&pOBInit);
		HAL_FLASH_Lock();
		HAL_FLASH_OB_Lock();
		HAL_FLASH_OB_Launch();

	} else {
		//DO NOTHING
	}
}

/*
 Host_Buffer[0]=
 Host_Buffer[1]=0x18
 Host_Buffer[2]=Node Number
 Host_Buffer[3]=Application Number
 */
static void Bootloader_SetApplication_Flag(uint8_t *Host_Buffer)
{
#if((BL_CAN == ENABLE) && (ECU_CONFIG == SLAVE1))
	/*Get which app flag needed to set*/
	uint8_t APP = Host_Buffer[0] ;

	/*Unlock Flash memory To enable writing to flash*/
	HAL_FLASH_Unlock();

	/* To edit the application flag */
	if (APP == APPLICATION1)
	{
		Write_Program_Flag(FLAG_APP1_ADDRESS, FLAG_APP_ON);
		Write_Program_Flag(FLAG_APP2_ADDRESS, FLAG_APP_OFF);
		Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
		HAL_NVIC_SystemReset();
	}
	else if (APP == APPLICATION2)
	{
		Write_Program_Flag(FLAG_APP1_ADDRESS, FLAG_APP_OFF);
		Write_Program_Flag(FLAG_APP2_ADDRESS, FLAG_APP_ON);
		Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
		HAL_NVIC_SystemReset();
	}
	else
	{
		/* Warning to ESP */
	}

	/*Lock Flash memory disable writing to it after finishing the function*/
	HAL_FLASH_Lock();
#else
	/* To set application number */
	uint8_t ECU = Host_Buffer[2];
	uint8_t APP = Host_Buffer[3] ;
	HAL_FLASH_Unlock(); //To enable writing to flash

	if (ECU == MASTERID)
	{
#if(ECU_CONFIG == MASTERID)
		/* To edit the application flag */
		if (APP == APPLICATION1)
		{
			Write_Program_Flag(FLAG_APP1_ADDRESS, FLAG_APP_ON);
			Write_Program_Flag(FLAG_APP2_ADDRESS, FLAG_APP_OFF);
			Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
			HAL_NVIC_SystemReset();
		}
		else if (APP == APPLICATION2)
		{
			Write_Program_Flag(FLAG_APP1_ADDRESS, FLAG_APP_OFF);
			Write_Program_Flag(FLAG_APP2_ADDRESS, FLAG_APP_ON);
			Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
			HAL_NVIC_SystemReset();
		}
		else
		{
			/* Warning to ESP */
		}
#endif

	}
	else if(ECU == SLAVE1)
	{
#if((BL_CAN == ENABLE) && (ECU_CONFIG == MASTERID))
		/*Create and initiate buffer to send*/
		uint8_t BL_JUMP_TO_APP[8]; //Length must be 8 or more or undefined behaviour may occur
		BL_JUMP_TO_APP[0] = APP; //Store which App needed to jump to

		/*Transmit which app to jump to through one CAN frame*/
		CAN_TxHeaderTypeDef BL_Jump_To_App_TxHeader; //Create Jump_To_App struct Txheader for CAN transmit function

		BL_Jump_To_App_TxHeader.DLC = 1;
		BL_Jump_To_App_TxHeader.ExtId = 0;
		BL_Jump_To_App_TxHeader.IDE = CAN_ID_STD;
		BL_Jump_To_App_TxHeader.RTR = CAN_RTR_DATA;
		BL_Jump_To_App_TxHeader.StdId = CAN_JUMP_TO_APP_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
		BL_Jump_To_App_TxHeader.TransmitGlobalTime = DISABLE;

		HAL_CAN_AddTxMessage(&hcan, &BL_Jump_To_App_TxHeader, BL_JUMP_TO_APP, BL_TxMailbox);
		while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.
#elif(ECU_CONFIG == MASTERID)
		HAL_UART_Transmit(&huart2, &Host_Buffer[0], 1, 5); //sending length first, The time must be 5 to avoid errors
		HAL_UART_Transmit(&huart2, &Host_Buffer[1], Host_Buffer[0], 5*Host_Buffer[0]); //sending remaining frame, The time must be of about 5 times the length to avoid errors
#elif(ECU_CONFIG == SLAVE1)
		if (APP == APPLICATION1)
		{
			Write_Program_Flag(FLAG_APP1_ADDRESS, FLAG_APP_ON);
			Write_Program_Flag(FLAG_APP2_ADDRESS, FLAG_APP_OFF);
			Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
			HAL_NVIC_SystemReset();
		}
		else if (APP == APPLICATION2)
		{
			Write_Program_Flag(FLAG_APP1_ADDRESS, FLAG_APP_OFF);
			Write_Program_Flag(FLAG_APP2_ADDRESS, FLAG_APP_ON);
			Write_Program_Flag(FLAG_BL_ADDRESS, FLAG_APP_OFF);
			HAL_NVIC_SystemReset();
		}
		else
		{
			/* Warning to ESP */
		}
#endif

	}


	HAL_FLASH_Lock();//To disable writing to flash after finishing the function

#endif
}

static void BL_Manager(void)
{
	if(Get_Program_Flag_Status(FLAG_BL_ADDRESS) == FLAG_APP_ON)
	{
		/*Do nothing*/
	}
	else if(Get_Program_Flag_Status(FLAG_APP1_ADDRESS) == FLAG_APP_ON)
	{
		/*Jump to App1*/
		Bootloader_Jump_To_User_App1();
	}
	else if(Get_Program_Flag_Status(FLAG_APP2_ADDRESS) == FLAG_APP_ON)
	{
		/*Jump to App2*/
		Bootloader_Jump_To_User_App2();
	}
	else
	{
		/*Do nothing*/
	}
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




void MemCopy(uint8_t* Dest_Ptr, uint8_t* Source_Ptr, uint32_t Length)
{
	uint32_t i;
	for(i = 0; i < Length; i++)
	{
		Dest_Ptr[i] = Source_Ptr[i];
	}
}





/*We make this function to handle sending large files that are bigger than MAX_DATA_CAN_LENGTH
 *You shouldn't send remote frames by this function
 * */
void CAN_TransmitData(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *Header, uint8_t TxData[], uint16_t Length)
{
	uint16_t i =0;
	uint32_t Mailbox = 0;

	while(i < Length)
	{
		/*Wait until all messages in mailbox are send*/
		while(HAL_CAN_IsTxMessagePending(hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.

		/*Send data as 8 byte packets until remaining data is less than 8*/
		if(Length - i < MAX_DATA_CAN_LENGTH)
		{
			Header->DLC = Length - i;
			HAL_CAN_AddTxMessage(hcan, Header, &TxData[i], &Mailbox);
			return;
		}
		else
		{
			Header->DLC = MAX_DATA_CAN_LENGTH;
			HAL_CAN_AddTxMessage(hcan, Header, &TxData[i], &Mailbox);
			i += MAX_DATA_CAN_LENGTH;
		}
	}

}





/*We make this function to handle Receiving large files that are bigger than MAX_DATA_CAN_LENGTH
 * To avoid unexpected errors it is a MUST to Pass Buffer[8] or bigger to this function
 *This function will not receive any remote frame*/
void CAN_ReceiveData(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *Header, uint8_t RxData[], uint16_t Length)
{
	uint16_t i =0;
	uint8_t RxMessage[8];
	while(i < Length)
	{
		/*Receive single CAN Message*/
		while(HAL_CAN_GetRxFifoFillLevel(hcan, RxFifo) == 0); //Polling until there is any message received

		/*Get the received message information and data
		 *You may use HAL_CAN_GetRxMessage(hcan, RxFifo, Header, &RxData[i]) for better performance as there will no need to MemCopy(),
		 *but this will lead to undefined behaviour as we write to non allocated stack memory i.e: uint8 Rx[8]; Rx[9] = 10;
		 * */
		HAL_CAN_GetRxMessage(hcan, RxFifo, Header, RxMessage);

		/*Remote Message should be discarded*/
		if(Header->RTR == CAN_RTR_REMOTE)
		{
			continue;
		}
		else
		{
			/*Do nothing*/
		}

		/*Store Data in dedicated buffer*/
		if((i + Header->DLC) > Length) //This condition to handle when number of data in "RxData[]" is bigger than needed number of data in "Length"
		{
			MemCopy(&RxData[i], RxMessage, (Length - i)); //(Length - i) is the remaining part of the File after sending Frames of data size MAX_DATA_CAN_LENGTH from it
		}
		else
		{
			MemCopy(&RxData[i], RxMessage, Header->DLC);
		}
		i += Header->DLC;
	}
}






void CAN_Read_RP_Level(void)
{
	/*Create Variable to send RP level through it*/
	uint8_t RDP_Level = CBL_STM32F103_GET_RDP_Level();

	/*Create and initialise TxHeader*/
	CAN_TxHeaderTypeDef BL_RPTxHeader;
	BL_RPTxHeader.DLC = 1;
	BL_RPTxHeader.ExtId = 0;
	BL_RPTxHeader.IDE = CAN_ID_STD;
	BL_RPTxHeader.RTR = CAN_RTR_DATA;
	BL_RPTxHeader.StdId = CAN_READ_RP_RESP_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
	BL_RPTxHeader.TransmitGlobalTime = DISABLE;

	/*Send RP level CAN frame*/
	HAL_CAN_AddTxMessage(&hcan, &BL_RPTxHeader, &RDP_Level, BL_TxMailbox);
	while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.
}



void CAN_Read_WP_Level(void)
{
	/*Create Variable to send RP level through it*/
	uint8_t	WDP_Level = CBL_STM32F103_GET_WDP_Level();

	/*Create and initialise TxHeader*/
	CAN_TxHeaderTypeDef BL_WPTxHeader;
	BL_WPTxHeader.DLC = 1;
	BL_WPTxHeader.ExtId = 0;
	BL_WPTxHeader.IDE = CAN_ID_STD;
	BL_WPTxHeader.RTR = CAN_RTR_DATA;
	BL_WPTxHeader.StdId = CAN_READ_WP_RESP_ID; //Max value of StdId is 0x7FF(0b11111111111) i.e: 11 bit length.
	BL_WPTxHeader.TransmitGlobalTime = DISABLE;

	/*Send RP level CAN frame*/
	HAL_CAN_AddTxMessage(&hcan, &BL_WPTxHeader, &WDP_Level, BL_TxMailbox);
	while(HAL_CAN_IsTxMessagePending(&hcan, PEND_ON_ALL_TRANSMIT_MAILBOXES)); //In Retransmit mode make this with timer to detect error and avoid infinite loop.
}



void CAN_Receive_AllPendingMessages_FIFO0(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *Header, CAN_Receive_Buffer* RxData)
{
	/*Read all the three Read_Mail_boxes in FIFO0*/
	uint8_t counter = 0;
	while(hcan->Instance->RF0R & FIFO_RECEIVE_PENDING_MESSAGES_MASK)
	{
		if(counter == 0)
		{
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &BL_RxHeaderReceive, RxData->Rx1_Data);
		}
		else if(counter == 1)
		{
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &BL_RxHeaderReceive, RxData->Rx2_Data);
		}
		else if(counter ==2)
		{
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &BL_RxHeaderReceive, RxData->Rx3_Data);
		}
		else
		{
			/*Do nothing*/
		}
		counter++;
	}
}



void CAN_Receive_AllPendingMessages_FIFO1(CAN_HandleTypeDef *hcan, CAN_RxHeaderTypeDef *Header, CAN_Receive_Buffer* RxData)
{
	/*Read all the three Read_Mail_boxes in FIFO0*/
	uint8_t counter = 0;
	while(hcan->Instance->RF1R & FIFO_RECEIVE_PENDING_MESSAGES_MASK)
	{
		if(counter == 0)
		{
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &BL_RxHeaderReceive, RxData->Rx1_Data);
		}
		else if(counter == 1)
		{
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &BL_RxHeaderReceive, RxData->Rx2_Data);
		}
		else if(counter ==2)
		{
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &BL_RxHeaderReceive, RxData->Rx3_Data);
		}
		else
		{
			/*Do nothing*/
		}
		counter++;
	}
}






void CAN_Select_Func(void)
{
	/*Select which Bootloader Function to enter*/
	if(BL_RxHeader.FilterMatchIndex == CAN_MATCH_FILTER_INDEX0) //Entering this condition means received frame is Remote frame
	{
		/*Select needed Bootloader Function based on received StandardID of CAN frame*/
		switch(BL_RxHeader.StdId)
		{
		case CAN_VER_REQ_ID:
			Bootloader_Get_Version(BL_RxData);
			break;

		case CAN_READ_RP_REQ_ID:
			CAN_Read_RP_Level();
			break;

		case CAN_READ_WP_REQ_ID:
			CAN_Read_WP_Level();
			break;

		default:
			break;
		}
	}
	else if(BL_RxHeader.FilterMatchIndex == CAN_MATCH_FILTER_INDEX2) //Entering this condition means received frame is Data frame
	{
		/*Select needed Bootloader Function based on received StandardID of CAN frame*/
		switch(BL_RxHeader.StdId)
		{
		case CAN_FLASH_PROGRAM_ID:
			Bootloader_Memory_Write(BL_RxData);
			break;

		case CAN_ERASE_MEMORY_ID:
			Bootloader_Erase_Flash(BL_RxData);
			break;

		case CAN_JUMP_TO_APP_ID:
			Bootloader_SetApplication_Flag(BL_RxData);

		default:
			break;
		}

	}
	else
	{
		/*Do Nothing*/
	}

}

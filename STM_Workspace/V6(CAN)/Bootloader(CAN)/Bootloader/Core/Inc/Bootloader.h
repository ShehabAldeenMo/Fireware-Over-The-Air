/*
 * Name       : Bootloader.c
 * Created on : Oct 18, 2023
 * Author     : Shehab aldeen mohammed
 * Github     : https://github.com/ShehabAldeenMo
 * linkdin    : https://www.linkedin.com/in/shehab-aldeen-mohammed-0b3207228/
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

/*============================ Includes  =============================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "usart.h"
#include "can.h"
#include "crc.h"
#include "gpio.h"

/*==================== Macro Declerations  ===========================*/
#define FIRST_ITERATION                          0x01

/*User configurations */
#if PYTHON == 1
#define BL_HOST_COMMUNICATION_UART              &huart2 /* defination of communcation protocal with host */
#else
#define BL_HOST_COMMUNICATION_UART              &huart1 /* defination of communcation protocal with host */
#endif

#define CRC_ENGINE_OBJ                          &hcrc   /* defination of CRC address */

/* Length of rx buffer that you use for buffering data on MCU from
   hardware screen interface */
#define BL_HOST_BUFFER_RX_LENGTH                200

/* That is the commends that you use to communicate with bootloader */
#define CBL_GET_VER_CMD                         0x10 /* Read bootloader version (1 byte)*/
#define CBL_GET_HELP_CMD                        0x11 /* To know what are the commends supported by the bootloader --> All supported commends code (10 bytes)*/
#define CBL_GET_CID_CMD                         0x12 /* Read MCU chip identification number (2 bytes)*/
#define CBL_GET_RDP_STATUS_CMD                  0x13 /* Read the flash protection level (1 byte) */
#define CBL_GO_TO_ADDER_CMD                     0x14 /* Sucess or Error code --> This commend is used to jump on bootloader to specified address (1 byte)*/
#define CBL_FLASH_ERASE_CMD                     0x15 /* Sucess or Error code --> This commend is used to miss erase or sector erase of the user flash (1 byte) */
#define CBL_MEM_WRITE_CMD                       0x16 /* Sucess or Error code --> This commend is used to write data in different memories of the MCU (1 byte) */
#define CBL_CHANGE_ROP_Level_CMD                0x17
#define JUMP_TO_APPLICATION_CMD                 0x18
#define CBL_CHANGE_WOP_Level_CMD                0x19

#define FIRST_COMMEND                           0x10
#define LAST_COMMEND                            0x19

#define NumberOfCommends                        0x0A

/* That is the versions of your code and vendor id's */
#define CBL_VENDOR_ID                           100
#define CBL_SW_MAJOR_VERSION                    4
#define CBL_SW_MINOR_VERSION                    1
#define CBL_SW_PATCH_VERSION                    1

/* That is the versions of your code and vendor id's */
#define CBL_VENDOR_SLAVE_ID                     111


/* The number of bytes which transmits to detect the error noise effect on data */
#define CRC_TYPE_SIZE                           4  /* 4 bytes */


/*CAN Definitions*/
#define PEND_ON_ALL_TRANSMIT_MAILBOXES          CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2
#define MAX_DATA_CAN_LENGTH                     8
#define FIFO_RECEIVE_PENDING_MESSAGES_MASK      0x3 //used in CAN_Receive_AllPendingMessages function

/*CAN Match filter index*/
#define CAN_MATCH_FILTER_INDEX0                   0
#define CAN_MATCH_FILTER_INDEX1                   1
#define CAN_MATCH_FILTER_INDEX2                   2
#define CAN_MATCH_FILTER_INDEX3                   3
#define CAN_MATCH_FILTER_INDEX4                   4
#define CAN_MATCH_FILTER_INDEX5                   5
#define CAN_MATCH_FILTER_INDEX6                   6
#define CAN_MATCH_FILTER_INDEX7                   7
#define CAN_MATCH_FILTER_INDEX8                   8
#define CAN_MATCH_FILTER_INDEX9                   9
#define CAN_MATCH_FILTER_INDEX10                  10
#define CAN_MATCH_FILTER_INDEX11                  11
#define CAN_MATCH_FILTER_INDEX12                  12
#define CAN_MATCH_FILTER_INDEX13                  13

/*Version CAN ID Matrix*/
#define CAN_VER_REQ_ID                          0x00F //CAN Version ID Request(Sent by Master, Stored in FIFO1, Match index 0)
#define CAN_VER_RESP_ID                         0x0F0 //CAN ID Response(Sent by Slave, Stored in FIFO0, Match index 2)

#define CAN_READ_RP_REQ_ID                      0x01F //CAN Read RP level ID Request(Sent by Master, Stored in FIFO1, Match index 0)
#define CAN_READ_RP_RESP_ID                     0x0F1 //CAN Read RP level ID Response(Sent by Slave, Stored in FIFO0, Match index 2)

#define CAN_READ_WP_REQ_ID                      0x02F //CAN Read WP level ID Request(Sent by Master, Stored in FIFO1, Match index 0)
#define CAN_READ_WP_RESP_ID                     0x0F2 //CAN Read WP level ID Response(Sent by Slave, Stored in FIFO0, Match index 2)

#define CAN_FLASH_PROGRAM_ID                    0x0F3 //CAN Flash program ID (Sent by Master or Slave, Stored in FIFO0, Match index 2)

#define CAN_ERASE_MEMORY_ID                     0x0F4 //CAN Erase Memory ID (Sent by Master or Slave, Stored in FIFO0, Match index 2)

#define CAN_JUMP_TO_APP_ID                      0x0F5 //CAN Jump To App ID (Sent by Master, Stored in FIFO0, Match index 2)


/* Acknowledge and Noacknowledge messages through screen hardware interface */
#define CBL_SEND_ACK                            0xCD
#define CBL_SEND_NACK                           0xAB

/* the address of application region */
#define FLASH_PAGE_BASE_ADDRESS_APP1             0x08005000U
#define FLASH_PAGE_BASE_ADDRESS_APP2             0x0800A000U
#define FLAG_APP1_ADDRESS                        0x0801FC00U
#define FLAG_APP2_ADDRESS                        0x0801FC04U
#define FLAG_BL_ADDRESS                          0x0801FC08U


/* To check that the given address is within range */
#define ADDRESS_IS_VALID                        1
#define ADDRESS_IS_INVALID                      0

/* The sizes of memory */
#define STM32F103_SRAM_SIZE                     (20*1024)
#define STM32F103_FLASH_SIZE                    (64*1024) /* 128 pages */
#define STM32F103_SRAM_END                      (SRAM_BASE+STM32F103_SRAM_SIZE)
#define STM32F103_FLASH_END                     (FLASH_BASE+STM32F103_FLASH_SIZE)
#define STM32F103_FLASH_PAGE_SIZE               0x400 /* 1K */

#define APPLICATION_SIZE                        19

/* Related to pages in flash memory */
#define CBL_FLASH_MAX_PAGES_NUMBER              128
#define CBL_PAGE_END                            19

/* The status of Erasing flash memory */
#define SUCESSFUL_ERASE                         0x03
#define UNSUCESSFUL_ERASE                       0x02
#define PAGE_VALID_NUMBER                       0x01
#define PAGE_INVALID_NUMBER                     0x00
#define HAL_SUCESSFUL_ERASE                     0xFFFFFFFF

/* Erase all sectors in application region */
#define CBL_FLASH_MASS_ERASE                    0xFF

/* Defines writing in memory status */
#define FLASH_PAYLOAD_WRITING_FAILED            0x00
#define FLASH_PAYLOAD_WRITING_PASSED            0x01

/* definations related to read protection level */
#define RDP_LEVEL_READ_INVALID                  0x00
#define RDP_LEVEL_READ_VALID                    0x01

/* Change read protection level */
#define ROP_LEVEL_CHANGE_INVALID                0x00
#define ROP_LEVEL_CHANGE_VALID                  0x01

#define CBL_ROP_LEVEL_0                         0x00
#define CBL_ROP_LEVEL_1                         0x01

#define READ_RP                                 0x00
#define READ_WP                                 0x01

#define TURN_ON                                 0x01
#define TURN_OFF                                0x00

#define RESET                                   0

#define APPLICATION1                            1
#define APPLICATION2                            2

#define FLAG_APP_ON                        0xAAAAAAAAU
#define FLAG_APP_OFF                       0xEEEEEEEEU

#define BL_BEGIN_MATCH                     0xAA
#define BL_BEGIN_NOT_MATCH                 0xEE


#define TWO_BYTES                               0x08
/*==================== Data Types Declerations  ===========================*/
typedef enum {
	BL_NACK = 0 ,
	BL_OK
}BL_Status;

typedef enum {
	CRC_NACK = 0 ,
	CRC_OK
}CRC_Status;

/*Used in CAN_Receive_AllPendingMessages function*/
typedef struct
{
	uint8_t Rx1_Data[8];
	uint8_t Rx2_Data[8];
	uint8_t Rx3_Data[8];
}CAN_Receive_Buffer;


typedef void (*pFunc)(void) ;
typedef void (*Jump_Ptr)(void) ;
typedef void (*BL_pFunc)(uint8_t *);

/*==================== Software Interface Declerations  ===============*/
void      BL_PrintMassage(char* format, ...);
BL_Status BL_Fetch_Commend(void);
void Write_Program_Flag(uint32_t Address, uint32_t Value);
uint32_t Get_Program_Flag_Status(uint32_t Address);


#endif /* INC_BOOTLOADER_H_ */

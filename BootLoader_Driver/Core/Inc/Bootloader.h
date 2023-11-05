/*
 * Name       : Bootloader.h
 * Created on : Oct 18, 2023
 * Author     : Shehab aldeen
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

/*============================ Includes  =============================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "usart.h"
#include "crc.h"

/*==================== Macro Declerations  ===========================*/
/* The configurations of BL using */
#define BL_DEBUG_UART                           &huart2
#define BL_HOST_COMMUNICATION_UART              &huart2
#define CRC_ENGINE_OBJ                          &hcrc

/* Enable debugging messages to be sent to check the status of BL */
#define DEBUG_INFO_ENABLE                       0x01
#define DEBUG_INFO_DISABLE                      0x00
#define BL_DEBUG_ENABLE                         DEBUG_INFO_ENABLE

/* Choose the communication protocals that you use with bootloader */
#define BL_ENABLE_UART_DEBUG_MESSAGE            0x00
#define BL_ENABLE_CAN_DEBUG_MESSAGE             0x01
#define BL_ENABLE_ETHERNET_DEBUG_MESSAGE        0x02
#define BL_DEBUG_METHOD                         BL_ENABLE_UART_DEBUG_MESSAGE

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
#define CBL_EN_R_W_PROTECT_CMD                  0x17 /* Sucess or Error code --> This commend is used to write\read protect on different sectors of the user flash (1 byte) */
#define CBL_MEM_READ_CMD                        0x18 /* AMemory contents of length asked by the host (N bytes) This commend is used to read data from different memories of the MCU */
#define CBL_READ_SECTOR_STATUS_CMD              0x19 /* All sectors status, This commend is used to read all the sector protection status (2 bytes) */
#define CBL_OTP_READ_CMD                        0x20
#define CBL_CHANGE_ROP_Level_CMD                0x21
#define CBL_JUMP_TO_APP                         0x22

/* That is the versions of your code and vendor id's */
#define CBL_VENDOR_ID                           100
#define CBL_SW_MAJOR_VERSION                    1
#define CBL_SW_MINOR_VERSION                    1
#define CBL_SW_PATCH_VERSION                    0

/* The number of bytes which transmits to detect the error noise effect on data */
#define CRC_TYPE_SIZE                           4  /* 4 bytes */

/* Acknowledge and Noacknowledge messages through screen hardware interface */
#define CBL_SEND_ACK                            0xAB
#define CBL_SEND_NACK                           0xCD

/* the address of application region */
#define FLASH_SECTOR2_BASE_ADDRESS              0x8008000U

/* To check that the given address is within range */
#define ADDRESS_IS_VALID                        1
#define ADDRESS_IS_INVALID                      0

/* The sizes of memory */
#define STM32F103_SRAM_SIZE                     (20*1024)
#define STM32F103_FLASH_SIZE                    (128*1024) /* 128 pages */
#define STM32F103_SRAM_END                      (SRAM_BASE+STM32F103_SRAM_SIZE)
#define STM32F103_FLASH_END                     (FLASH_BASE+STM32F103_FLASH_SIZE)
#define STM32F103_FLASH_PAGE_SIZE               0x400 /* 1K */

/* Related to pages in flash memory */
#define CBL_FLASH_MAX_PAGES_NUMBER              128
#define CBL_PAGE_END                            14

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

/*==================== Data Types Declerations  ===========================*/
typedef enum {
	BL_NACK = 0 ,
	BL_OK
}BL_Status;

typedef enum {
	CRC_NACK = 0 ,
	CRC_OK
}CRC_Status;

typedef void (*pFunc)(void) ;
typedef void (*Jump_Ptr)(void) ;

/*==================== Software Interface Declerations  ===============*/
void      BL_PrintMassage(char* format, ...);
BL_Status BL_UART_Fetch_Host_Commend(void);

#endif /* INC_BOOTLOADER_H_ */

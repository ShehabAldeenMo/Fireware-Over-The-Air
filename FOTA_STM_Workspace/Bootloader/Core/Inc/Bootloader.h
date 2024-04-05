/**
 ******************************************************************************
 * @file name      : Bootloader.h
 * @Author         : Shehab aldeen mohammed, Ali Mamdouh
 *
 ******************************************************************************
 * @Notes:
 *
 *
 ******************************************************************************
 ******************************************************************************
*/

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

/*============================================================================
 ******************************  Including  *********************************
 ============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "usart.h"



/*============================================================================
 **************************  Macro Declerations  *****************************
 ============================================================================*/
/* Length of rx buffer that you use for buffering data on MCU from
   hardware screen interface */
#define BL_HOST_BUFFER_RX_LENGTH                200

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

/* The sizes of memory */
#define STM32F103_SRAM_SIZE        (20*1024)
#define STM32F103_FLASH_SIZE       (64*1024)  /* 128 pages */
#define STM32F103_SRAM_END         (SRAM_BASE+STM32F103_SRAM_SIZE)
#define STM32F103_FLASH_END        (FLASH_BASE+STM32F103_FLASH_SIZE)
#define STM32F103_FLASH_PAGE_SIZE   0x400     /* 1K */

/*============================================================================
 ************************* Data Types Declerations  **************************
 ============================================================================*/
typedef enum {
	BL_NACK = 0 ,
	BL_OK
}BL_Status;


typedef void (*pFunc)(void) ;
typedef void (*Jump_Ptr)(void) ;
typedef void (*BL_pFunc)(uint8_t *);



/*============================================================================
 ********************** Software Interface Declerations  *********************
 ============================================================================*/
/**
 * \section Service_Name
 * BL_PrintMassage
 *
 * \section Description
 * To send info using uart from stm using TTL to realterm
 *  and print debugging info about the statue of functions
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Renterancy
 * Reentrant
 *
 * \section Parameters
 * \param[in]    format (pointer to char)  , .......
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 * None
 */
void      BL_PrintMassage(char* format, ...);

/**
 * \section Service_Name
 * BL_Fetch_Commend
 *
 * \section Description
 * To fetch the commend using uart and interact according to it
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
 * BL_Status
 */
BL_Status BL_Fetch_Commend(void);

/**
 * \section Service_Name
 * Get_Program_Flag_Status
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
 * \param[in]    Address    (uint32_t)  --> Address of flag
 * \param[inout] None
 * \param[out]   None
 *
 * \section Return_value
 *  status of flag (uint32_t)
 */
uint32_t  Get_Program_Flag_Status(uint32_t Address);


#endif /* INC_BOOTLOADER_H_ */

/**
 ******************************************************************************
 * @file name      : BL_Manager_Config.h
 * @Author         : Shehab aldeen mohammed, Ali Mamdouh
 *
 ******************************************************************************
 * @Notes:
 *
 *
 ******************************************************************************
 ******************************************************************************
*/






#ifndef INC_BL_MANAGER_CONFIG_H_
#define INC_BL_MANAGER_CONFIG_H_

/****************************************************************************
 ******************************  Including  *********************************
 ****************************************************************************/
#include "stm32f1xx_hal.h"
#include "usart.h"








/*****************************************************************************
 **************************  Macro Declerations  *****************************
 *****************************************************************************/
#define MASTERID                                 0x00
#define SLAVE1                                   0x01






/*****************************************************************************
 *************************  User Configurations  *****************************
 *****************************************************************************/
#define FLASH_PAGE_BASE_ADDRESS_APP1             0x08005000U
#define FLASH_PAGE_BASE_ADDRESS_APP2             0x0800A000U
#define FLASH_PAGE_BASE_ADDRESS_BL               0x08000000U


#define RESET_ADDRESS                            FLASH_PAGE_BASE_ADDRESS_BL //@Note: Used to reset running app by Passing its address to Jump_To_Address() function

#define ECU_CONFIG                               SLAVE1

#define MASTER_TO_HOST_CONNECTION                &huart1
#define MASTER_TO_SLAVE_CONNECTION               &huart2
#define SLAVE_TO_MASTER_CONNECTION               &huart1




#endif /* INC_BL_MANAGER_CONFIG_H_ */

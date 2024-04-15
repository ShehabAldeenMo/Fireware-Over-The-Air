/*
 * Bootloader_Config.h
 *
 *  Created on: Feb 14, 2024
 *      Author: HP
 */

#ifndef INC_BOOTLOADER_CONFIG_H_
#define INC_BOOTLOADER_CONFIG_H_

/* Enable debugging messages to be sent to check the status of BL */
#define DEBUG_INFO_ENABLE                       0x01
#define DEBUG_INFO_DISABLE                      0x00

/* Choose the communication protocals that you use with bootloader */
#define BL_ENABLE_UART_DEBUG_MESSAGE            0x00
#define BL_ENABLE_CAN_DEBUG_MESSAGE             0x01
#define BL_ENABLE_ETHERNET_DEBUG_MESSAGE        0x02

#define MASTERID                                0x00
#define SLAVE1                                  0x01
#define SLAVE2                                  0x02
#define SLAVE3                                  0x03
#define SLAVE4                                  0x04
#define SLAVE5                                  0x05

#define ECU_CONFIG                              MASTERID
#define BL_CAN                                  DISABLE


/*================================   User Configurations   ===========================================*/

/*User configurations */
#define ESP        ENABLE
#define PYTHON     DISABLE

/* The configurations of BL using */
#define BL_DEBUG_UART                           &huart2 /* defination of communcation protocal in debuging without hosy port */
#define BL_DEBUG_ENABLE                         DEBUG_INFO_DISABLE /* Enable when you use BL_DEBUG_UART different from BL_HOST_COMMUNICATION_UART */


#define BL_DEBUG_METHOD                         BL_ENABLE_UART_DEBUG_MESSAGE

#define MASTER    ENABLE
#define SLAVE     DISABLE

#if SLAVE == ENABLE
#define SLAVE_NUMBER  SLAVE1
#endif

#endif /* INC_BOOTLOADER_CONFIG_H_ */

/**
*@file       Port_Cfg.h
*@version    2.1.0
*@details    It contains all prototypes of used functions and states in port driver
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/

#ifndef PORT_CFG_H_
#define PORT_CFG_H_

#include "Port_Interface.h"

#define NUM_OF_PORT     3

/* Design choose :
 * "PORT_NOOS"               --> No operating system
 * "PORT_FREERTOS"           --> Free RTOS
 *  */
#define PORT_DESIGN    PORT_NOOS

#endif /* PORT_CFG_H_ */

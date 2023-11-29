/*
*@file       Dio_Cfg.h
*@version    2.1.0
*@details    It has the cofigurations if driver
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Project          : AUTOSAR  4.3.1 MCAL
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/


#ifndef DIO_CFG_H_
#define DIO_CFG_H_

#include "DIO_Interface.h"

#define NUM_OF_PORT     3


/* Design choose :
 * "DIO_NOOS"               --> No operating system
 * "DIO_FREERTOS"           --> Free RTOS
 *  */
#define DIO_DESIGN    DIO_NOOS

#endif /* DIO_CFG_H_ */

/*
*@file       MCU_Cfg.h
*@version    2.1.0
*@details    It has the specific configurations of Mcu driver.
*@author     Shehab aldeen mohammed abdalah.
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/

#ifndef MCU_CFG_H_
#define MCU_CFG_H_

#include "Std_Types.h"
#include "MCU_Interface.h"

#define McuInitClock             FALSE
#define McuNoPll                 FALSE

/* if you used HSE define it otherwise neglect it*/
#define HSE             HSE_CRSTYAL
#define CSS_CFG         CSS_OFF

/* Design choose :
 * "MCU_NOOS"               --> No operating system
 * "MCU_FREERTOS"           --> Free RTOS
 *  */
#define MCU_DESIGN    MCU_NOOS

#endif /* MCU_CFG_H_ */

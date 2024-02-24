/*
*@file       MCU_Cfg.c
*@version    2.1.0
*@details    It contains all prototypes of used functions and states
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/
#include <Mcu_Interface.h>


Mcu_ConfigType Config_Clock = { HSE_CLOCK, PLL_NONE_0, PLL_NONE_ENTRY,
		ADC_PRE_DIVIDED_2, APB2__PRE_NOT_DIVIDED, APB1_PRE_NOT_DIVIDED,
		AHB_PRE_NOT_DIVIDED, HSE_SW, 0 };
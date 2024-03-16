/**
*@file       Port_Cfg.c
*@version    2.1.0
*@details    It contains all prototypes of used functions and states in port driver
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/

#include "Port_Interface.h"
#include "Port_Cfg.h"

Port_Cfg Port_ArrOfPorts[NUM_OF_PORT]= {
		{
				.Port_Num       = Port_GPIO_A                   ,
                .Mode           = OUTPUT_2|G_PUSH_PULL          ,
		},
		{
				.Port_Num       = Port_GPIO_B                    ,
                .Mode           = OUTPUT_2|G_PUSH_PULL           ,
		},
		{
				.Port_Num       = Port_GPIO_C                    ,
                .Mode           = OUTPUT_2|G_PUSH_PULL           ,
		}
};

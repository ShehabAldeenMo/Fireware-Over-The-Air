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

#include "DIO_Interface.h"
#include "Dio_Cfg.h"

Dio_Cfg ArrOfPorts[NUM_OF_PORT] = {
		{
				.Port_Num       = Dio_GPIO_A      ,
                .Level          = STD_OFF         ,
		},
		{
				.Port_Num       = Dio_GPIO_B      ,
                .Level          = STD_OFF         ,
		},
		{
				.Port_Num       = Dio_GPIO_C      ,
                .Level          = STD_OFF         ,
		}
};

/**
*@file       DET.h
*@version    1.0.0
*@brief      AUTOSAR Base - Default Error Tracer
*@details    It contains all prototypes of used functions and states
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Project          : AUTOSAR  4.0.0
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   AUTOSAR Version  : 4.0.0
*   AUTOSAR Revision : ASR_REL_4_1_REV_0000
*   SW Version       : 1.0.0
============================================================================*/

#ifndef DET_H_
#define DET_H_

#include"Std_Types.h"


Std_ReturnType Det_ReportError(uint16 ModuleId,uint8 ApiId , uint8 ErrorId);

#endif /* DET_H_ */

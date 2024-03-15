/**
*@file       Std_Types.h
*@version    1.0.0
*@brief      AUTOSAR Base - Standard types definations.
*@details    AUTOSAR standard types header file.
*            It contains all types that are used across several modules of basic software 
*            and that are platform and compilar independant
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Project          : AUTOSAR  4.3.1 MCAL
*   Platform         : ARM 
*   Peripherial      : STM32F103C8T6
*   AUTOSAR Version  : 4.3.1
*   AUTOSAR Revision : ASR_REL_4_1_REV_0001
*   SW Version       : 1.0.0
============================================================================*/

#ifndef STD_TYPES_H_
#define STD_TYPES_H_

#include "Platform_Types.h"

/****************************************************************************
************************  Version info  *************************************
*****************************************************************************/

#define STD_MAJOR_VERSION         1
#define STD_MINOR_VERSION         0
#define STD_SW_VERSION            0


/****************************************************************************
**************************  typedef   ***************************************
*****************************************************************************/

/*
 * Cover_req_[SWS_Std_00005]
 */
typedef uint8 Std_ReturnType;                     /* Range : E_OK 0 , E_NOT_OK 1 ,
                                                     " Available to user specific errors "  0x02-0x3F */

/****************************************************************************
**************************  defines   ***************************************
*****************************************************************************/

/*
 * Cover_req_SWS_Std_00007
 */
#define STD_LOW                   0x00
#define STD_HIGH                  0x01  

/*
 * Cover_req_SWS_Std_000013
 */
#define STD_IDLE                  0x00 
#define STD_ACTIVE                0x01


/* 
 *Cover_req_SWS_Std_00010
 */
#define STD_OFF                   0x00
#define STD_ON                    0x01

/*
 * Cover_req_SWS_Std_00031
 * Cover_req_SWS_Std_00006
 */
#define NULL_PTR       ((void *)0)

typedef enum {
 E_OK                   =   0x03,
 E_NOT_OK               =   0x01,
 E_INVALID_PARAMETER    =   0x02
}Error_State;

typedef enum {
	Uninitialized,
	Initialized,
	Idle         ,
	Busy
}State_Machine;

/****************************************************************************
**************************  Structure   ************************************
*****************************************************************************/
typedef struct {
	uint16 vendorID        ;
	uint16 moduleID        ;
	uint8 sw_major_version ;
	uint8 sw_minor_version ;
	uint8 sw_patch_version ;
}Std_VersionInfoType;
 
#endif /*Std_Types.h*/

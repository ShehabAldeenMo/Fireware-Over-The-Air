/**
*@file       Platform_Types.h
*@version    1.0.0
*@brief      AUTOSAR Base - Platform dependend data type defination.
*@details    AUTOSAR Platform types header file.
*            It contains all platform dependent types and symbols.
*            Those types must be abstracted in order to become platform and complair independent.
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

#ifndef PLATFORM_H_
#define PLATFORM_H_

/****************************************************************************
**************************  typedef   ***************************************
*****************************************************************************/

/*
 * Cover_req_[SWS_Platform_00013]
 * Cover_req_[SWS_Platform_00014]
 * Cover_req_[SWS_Platform_00015]
 * Cover_req_[SWS_Platform_00066]
 * Cover_req_[SWS_Platform_00016]
 * Cover_req_[SWS_Platform_00017]
 * Cover_req_[SWS_Platform_00018]
 * Cover_req_[SWS_Platform_00067]
 * Cover_req_[SWS_Platform_00041]
 * Cover_req_[SWS_Platform_00042]
 *
 * */
typedef unsigned char boolean         ;
typedef signed char sint8             ;
typedef unsigned char uint8           ;
typedef signed short int sint16       ;
typedef unsigned short int uint16     ;
typedef signed int sint32             ;
typedef unsigned int uint32           ;
typedef signed long long int sint64   ;
typedef unsigned long long int uint64 ;
typedef float float32                 ;
typedef double float64                ;


/****************************************************************************
**************************  define   ***************************************
*****************************************************************************/

/**
 * Cover_req_[SWS_Platform_00056]
 */
#ifndef TRUE
#define TRUE                     1
#endif

#ifndef FALSE
#define FALSE                    0
#endif

/**
 * Cover_req_[SWS_Platform_91001]
 * Cover_req_[SWS_Platform_91002]
 */
#define VoidPtr                  ((void *)0)
#define ConstVoidPtr             ((const void *)0)

#endif /*Platform_Types.h*/

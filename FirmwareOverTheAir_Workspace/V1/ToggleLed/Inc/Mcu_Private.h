/*
*@file       Mcu_Private.h
*@version    2.1.0
*@details    It has the private data like address and bits
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/

#ifndef MCU_PRIVATE_H_
#define MCU_PRIVATE_H_

#include "Std_Types.h"

/****************************************************************************
************************  Version info  *************************************
*****************************************************************************/
#define VENDOR_ID                 0x50
#define MODULE_ID                 0x65
#define MCU_MAJOR_VERSION         0x01
#define MCU_MINOR_VERSION         0x01
#define MCU_PATCH_VERSION         0x00



/****************************************************************************
************************  Development Errors Id   ***************************
*****************************************************************************/
#define MCU_E_PARAM_CONFIG        0x0A      /* API service called with wrong parameter */
#define MCU_E_PARAM_CLOCK         0x0B      /* API service called with wrong parameter */
#define MCU_E_PARAM_MODE          0x0c      /* API service called with wrong parameter */
#define MCU_E_PARAM_RAMSECTION    0x0D      /* API service called with wrong parameter */
#define MCU_E_PLL_NOT_LOCKED      0x0E      /* API service called with wrong parameter */
#define MCU_E_UNINIT              0x0F      /* API service called with wrong parameter */
#define MCU_E_PARAM_POINTER       0x10      /* API service called with wrong parameter */
#define MCU_E_INIT_FAILED         0x11      /* API service called with wrong parameter */
#define MCU_E_TIMEOUT             0x12      /* API service called with wrong parameter */


/****************************************************************************
***************************  define API IDs  ********************************
*****************************************************************************/
#define Mcu_Init_ID                 0x01
#define Mcu_InitClock_ID            0x02
#define Mcu_DistributePllClock_ID   0x03
#define Mcu_GetPllStatus_ID         0x04
#define Mcu_SetMode_ID              0x08
#define Mcu_GetVersionInfo_ID       0x09


/****************************************************************************
***************************  Structure address  *****************************
*****************************************************************************/
typedef struct {
	uint32 CR;
	uint32 CFGR;
	uint32 CIR;
	uint32 APB2RSTR;
	uint32 APB1RSTR;
	uint32 AHBENR;
	uint32 APB2ENR;
	uint32 APB1ENR;
	uint32 BDCR;
	uint32 CSR;
	uint32 AHBRSTR;
	uint32 CFGR2;
} RCC_REG;

#define RCC          ((volatile RCC_REG *)  (0x40021000))

#define NULL           0

#define HSE_CRSTYAL    0
#define HSE_RC         1
#define CSS_ON         1
#define CSS_OFF        0

#define CSSON          19
#define HSE_BYP        18

#define TIMEOUT_ROOF   1000

/* Input       : McuMode
 * Return      : None
 * Description : This service activates the MCU power modes.
 * */
void              Mcu_SetMode            (Mcu_ModeType McuMode)                       ;

#endif /* MCU_PRIVATE_H_ */
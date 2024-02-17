/**
*@file       Port_Private.h
*@version    2.1.0
*@details    It contains all prototypes of used functions and states in port driver
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/
#ifndef PORT_PRIVATE_H_
#define PORT_PRIVATE_H_

/****************************************************************************
************************  Version info  *************************************
*****************************************************************************/
#define VENDOR_ID                  150
#define MODULE_ID                  124
#define PORT_MAJOR_VERSION         1
#define PORT_MINOR_VERSION         0
#define PORT_PATCH_VERSION         0

/****************************************************************************
****************************  Errors   **************************************
*****************************************************************************/
#define PORT_E_PARAM_PIN                      0x0A       /* Invalid Port Pin ID requested                               */
#define PORT_E_DIRECTION_UNCHANGEABLE         0x0B       /* Port Pin not configured as changeable                       */
#define PORT_E_PARAM_INVALID_MODE             0x0D       /* API Port_SetPinMode service called when mode is unchangeable*/
#define PORT_E_MODE_UNCHANGEABLE              0x0E       /* API Port_SetPinMode service called when mode is unchangeable*/
#define PORT_E_UNINIT                         0x0F       /* API service called without module initialization            */
#define PORT_E_PARAM_POINTER                  0x10       /* APIs called with a Null Pointer                             */


/****************************************************************************
****************************  define   **************************************
*****************************************************************************/
#define Port_SetPinDirection_ID     0x01
#define Port_GetVersionInfo_ID      0x03
#define Port_SetPinMode_ID          0x04

#define BASE_CRL_CRH          4
#define BASE_BSRR             16
#define AMOUNT_PINS           48

#define LOW_PIN                7

/*******************************************************************
************************  Structure of Adress  *********************
********************************************************************/
typedef struct {

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 MODE0 :2;
			volatile uint32 CNF0 :2;
			volatile uint32 MODE1 :2;
			volatile uint32 CNF1 :2;
			volatile uint32 MODE2 :2;
			volatile uint32 CNF2 :2;
			volatile uint32 MODE3 :2;
			volatile uint32 CNF3 :2;
			volatile uint32 MODE4 :2;
			volatile uint32 CNF4 :2;
			volatile uint32 MODE5 :2;
			volatile uint32 CNF5 :2;
			volatile uint32 MODE6 :2;
			volatile uint32 CNF6 :2;
			volatile uint32 MODE7 :2;
			volatile uint32 CNF7 :2;
		};
	} CRL;/* configuration register low */

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 MODE8 :2;
			volatile uint32 CNF8 :2;
			volatile uint32 MODE9 :2;
			volatile uint32 CNF9 :2;
			volatile uint32 MODE10 :2;
			volatile uint32 CNF10 :2;
			volatile uint32 MODE11 :2;
			volatile uint32 CNF11 :2;
			volatile uint32 MODE12 :2;
			volatile uint32 CNF12 :2;
			volatile uint32 MODE13 :2;
			volatile uint32 CNF13 :2;
			volatile uint32 MODE14 :2;
			volatile uint32 CNF14 :2;
			volatile uint32 MODE15 :2;
			volatile uint32 CNF15 :2;
		};
	} CRH; /* configuration register High */

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 IDR  :1;
			volatile uint32 IDR1 :1;
			volatile uint32 IDR2 :1;
			volatile uint32 IDR3 :1;
			volatile uint32 IDR4 :1;
			volatile uint32 IDR5 :1;
			volatile uint32 IDR6 :1;
			volatile uint32 IDR7 :1;
			volatile uint32 IDR8 :1;
			volatile uint32 IDR9 :1;
			volatile uint32 IDR10 :1;
			volatile uint32 IDR11 :1;
			volatile uint32 IDR12 :1;
			volatile uint32 IDR13 :1;
			volatile uint32 IDR14 :1;
			volatile uint32 IDR15 :1;
			volatile uint32 RESERVED :16;

		};
	} IDR; /* input data register */

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 ODR0 :1;
			volatile uint32 ODR1 :1;
			volatile uint32 ODR2 :1;
			volatile uint32 ODR3 :1;
			volatile uint32 ODR4 :1;
			volatile uint32 ODR5 :1;
			volatile uint32 ODR6 :1;
			volatile uint32 ODR7 :1;
			volatile uint32 ODR8 :1;
			volatile uint32 ODR9 :1;
			volatile uint32 ODR10 :1;
			volatile uint32 ODR11 :1;
			volatile uint32 ODR12 :1;
			volatile uint32 ODR13 :1;
			volatile uint32 ODR14 :1;
			volatile uint32 ODR15 :1;

		};
	} ODR; /* output data register */

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 BS0 :1;
			volatile uint32 BS1 :1;
			volatile uint32 BS2 :1;
			volatile uint32 BS3 :1;
			volatile uint32 BS4 :1;
			volatile uint32 BS5 :1;
			volatile uint32 BS6 :1;
			volatile uint32 BS7 :1;
			volatile uint32 BS8 :1;
			volatile uint32 BS9 :1;
			volatile uint32 BS10 :1;
			volatile uint32 BS11 :1;
			volatile uint32 BS12 :1;
			volatile uint32 BS13 :1;
			volatile uint32 BS14 :1;
			volatile uint32 BS15 :1;
			volatile uint32 BR0 :1;
			volatile uint32 BR1 :1;
			volatile uint32 BR2 :1;
			volatile uint32 BR3 :1;
			volatile uint32 BR4 :1;
			volatile uint32 BR5 :1;
			volatile uint32 BR6 :1;
			volatile uint32 BR7 :1;
			volatile uint32 BR8 :1;
			volatile uint32 BR9 :1;
			volatile uint32 BR10 :1;
			volatile uint32 BR11 :1;
			volatile uint32 BR12 :1;
			volatile uint32 BR13 :1;
			volatile uint32 BR14 :1;
			volatile uint32 BR15 :1;
		};
	} BSRR; /* bit set/reset register*/

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 BR0;
			volatile uint32 BR1;
			volatile uint32 BR2;
			volatile uint32 BR3;
			volatile uint32 BR4;
			volatile uint32 BR5;
			volatile uint32 BR6;
			volatile uint32 BR7;
			volatile uint32 BR8;
			volatile uint32 BR9;
			volatile uint32 BR10;
			volatile uint32 BR11;
			volatile uint32 BR12;
			volatile uint32 BR13;
			volatile uint32 BR14;
			volatile uint32 BR15;
			volatile uint32 RESERVED :16;

		};
	} BRR; /* bit reset register */

	union {
		volatile uint32 Reg;
		struct {
			volatile uint32 LOK0 :1;
			volatile uint32 LOK1 :1;
			volatile uint32 LOK2 :1;
			volatile uint32 LOK3 :1;
			volatile uint32 LOK4 :1;
			volatile uint32 LOK5 :1;
			volatile uint32 LOK6 :1;
			volatile uint32 LOK7 :1;
			volatile uint32 LOK8 :1;
			volatile uint32 LOK9 :1;
			volatile uint32 LOK10 :1;
			volatile uint32 LOK11 :1;
			volatile uint32 LOK12 :1;
			volatile uint32 LOK13 :1;
			volatile uint32 LOK14 :1;
			volatile uint32 LOK15 :1;
			volatile uint32 LCKK :1;
			volatile uint32 RESERVED :15;
		};
	} LCKR;/* longer possible to modify the value of the port bit until the next reset. */

} GPIOX_REG;

/* base address */
#define GPIOA        ((GPIOX_REG *)(0x40010800))
#define GPIOB        ((GPIOX_REG *)(0x40010C00))
#define GPIOC        ((GPIOX_REG *)(0x40011000))
#define GPIOD        ((GPIOX_REG *)(0x40011400))

void    Port_SetPinMode      (GPIOX_REG *GPIOX,Port_PinType Pin,Port_PinModeType Mode)            ;

#endif /* PORT_PRIVATE_H_ */

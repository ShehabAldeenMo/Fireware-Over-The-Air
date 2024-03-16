/*
*@file       MGPIO_Private.h
*@version    2.1.0
*@details    It has the private data like address and bits
*@author     Shehab aldeen mohammed abdalah
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/
#ifndef DIO_PRIVATE_H_
#define DIO_PRIVATE_H_

#include "Std_Types.h"

/****************************************************************************
************************  Version info  *************************************
*****************************************************************************/
#define VENDOR_ID                  150
#define MODULE_ID                  120
#define DIO_MAJOR_VERSION         1
#define DIO_MINOR_VERSION         0
#define DIO_PATCH_VERSION         0


/****************************************************************************
************************  Development Errors Id   ***************************
*****************************************************************************/
#define DIO_E_PARAM_INVALID_CHANNEL_ID        0x0A      /* Invalid channel requested         */
#define DIO_E_PARAM_INVALID_PORT_ID           0x14      /* Invalid port requested            */
#define DIO_E_PARAM_INVALID_GROUP             0x1F      /* Invalid channel group requested   */
#define DIO_E_PARAM_POINTER                   0x20      /* API service called with a NULL pointer */


/****************************************************************************
***************************  define API IDs  ********************************
*****************************************************************************/
#define Dio_ReadChannel_ID          0x00
#define Dio_WriteChannel_ID         0x01
#define Dio_ReadPort_ID             0x02
#define Dio_WritePort_ID            0x03
#define Dio_GetVersionInfo_ID       0x12
#define Dio_FlipChannel_ID          0x11
#define Dio_EnumSetterPin_ID        0x14

/**************************************************************
************************  Defines   ***************************
***************************************************************/
#define BASE_CRL_CRH          4
#define BASE_BSRR             16
#define NUM_OF_PINS           48
#define DIO_IN                0x00
#define DIO_OUT               0x02

/*******************************************************************
************************  Structure of Adress  *********************
********************************************************************/
typedef struct {

	union {
		uint32 Reg;
		struct {
			uint32 MODE0 :2;
			uint32 CNF0 :2;
			uint32 MODE1 :2;
			uint32 CNF1 :2;
			uint32 MODE2 :2;
			uint32 CNF2 :2;
			uint32 MODE3 :2;
			uint32 CNF3 :2;
			uint32 MODE4 :2;
			uint32 CNF4 :2;
			uint32 MODE5 :2;
			uint32 CNF5 :2;
			uint32 MODE6 :2;
			uint32 CNF6 :2;
			uint32 MODE7 :2;
			uint32 CNF7 :2;
		};
	} CRL;/* configuration register low */

	union {
		uint32 Reg;
		struct {
			uint32 MODE8 :2;
			uint32 CNF8 :2;
			uint32 MODE9 :2;
			uint32 CNF9 :2;
			uint32 MODE10 :2;
			uint32 CNF10 :2;
			uint32 MODE11 :2;
			uint32 CNF11 :2;
			uint32 MODE12 :2;
			uint32 CNF12 :2;
			uint32 MODE13 :2;
			uint32 CNF13 :2;
			uint32 MODE14 :2;
			uint32 CNF14 :2;
			uint32 MODE15 :2;
			uint32 CNF15 :2;
		};
	} CRH; /* configuration register High */

	union {
		uint32 Reg;
		struct {
			uint32 IDR  :1;
			uint32 IDR1 :1;
			uint32 IDR2 :1;
			uint32 IDR3 :1;
			uint32 IDR4 :1;
			uint32 IDR5 :1;
			uint32 IDR6 :1;
			uint32 IDR7 :1;
			uint32 IDR8 :1;
			uint32 IDR9 :1;
			uint32 IDR10 :1;
			uint32 IDR11 :1;
			uint32 IDR12 :1;
			uint32 IDR13 :1;
			uint32 IDR14 :1;
			uint32 IDR15 :1;
			uint32 RESERVED :16;

		};
	} IDR; /* input data register */

	union {
		uint32 Reg;
		struct {
			uint32 ODR0 :1;
			uint32 ODR1 :1;
			uint32 ODR2 :1;
			uint32 ODR3 :1;
			uint32 ODR4 :1;
			uint32 ODR5 :1;
			uint32 ODR6 :1;
			uint32 ODR7 :1;
			uint32 ODR8 :1;
			uint32 ODR9 :1;
			uint32 ODR10 :1;
			uint32 ODR11 :1;
			uint32 ODR12 :1;
			uint32 ODR13 :1;
			uint32 ODR14 :1;
			uint32 ODR15 :1;

		};
	} ODR; /* output data register */

	union {
		uint32 Reg;
		struct {
			uint32 BS0 :1;
			uint32 BS1 :1;
			uint32 BS2 :1;
			uint32 BS3 :1;
			uint32 BS4 :1;
			uint32 BS5 :1;
			uint32 BS6 :1;
			uint32 BS7 :1;
			uint32 BS8 :1;
			uint32 BS9 :1;
			uint32 BS10 :1;
			uint32 BS11 :1;
			uint32 BS12 :1;
			uint32 BS13 :1;
			uint32 BS14 :1;
			uint32 BS15 :1;
			uint32 BR0 :1;
			uint32 BR1 :1;
			uint32 BR2 :1;
			uint32 BR3 :1;
			uint32 BR4 :1;
			uint32 BR5 :1;
			uint32 BR6 :1;
			uint32 BR7 :1;
			uint32 BR8 :1;
			uint32 BR9 :1;
			uint32 BR10 :1;
			uint32 BR11 :1;
			uint32 BR12 :1;
			uint32 BR13 :1;
			uint32 BR14 :1;
			uint32 BR15 :1;
		};
	} BSRR; /* bit set/reset register*/

	union {
		uint32 Reg;
		struct {
			uint32 BR0;
			uint32 BR1;
			uint32 BR2;
			uint32 BR3;
			uint32 BR4;
			uint32 BR5;
			uint32 BR6;
			uint32 BR7;
			uint32 BR8;
			uint32 BR9;
			uint32 BR10;
			uint32 BR11;
			uint32 BR12;
			uint32 BR13;
			uint32 BR14;
			uint32 BR15;
			uint32 RESERVED :16;

		};
	} BRR; /* bit reset register */

	union {
		uint32 Reg;
		struct {
			uint32 LOK0 :1;
			uint32 LOK1 :1;
			uint32 LOK2 :1;
			uint32 LOK3 :1;
			uint32 LOK4 :1;
			uint32 LOK5 :1;
			uint32 LOK6 :1;
			uint32 LOK7 :1;
			uint32 LOK8 :1;
			uint32 LOK9 :1;
			uint32 LOK10 :1;
			uint32 LOK11 :1;
			uint32 LOK12 :1;
			uint32 LOK13 :1;
			uint32 LOK14 :1;
			uint32 LOK15 :1;
			uint32 LCKK :1;
			uint32 RESERVED :15;
		};
	} LCKR;/* longer possible to modify the value of the port bit until the next reset. */

} GPIOX_REG;

#define GPIOA        ((volatile GPIOX_REG *)(0x40010800))
#define GPIOB        ((volatile GPIOX_REG *)(0x40010C00))
#define GPIOC        ((volatile GPIOX_REG *)(0x40011000))
#define GPIOD        ((volatile GPIOX_REG *)(0x40011400))

/*    Supporting functions     */
Dio_LevelType     Dio_ReadChannel (volatile GPIOX_REG *GPIOX,Dio_ChannelType ChannelId)                         ;
void              Dio_WriteChannel(volatile GPIOX_REG *GPIOX,Dio_ChannelType ChannelId,Dio_LevelType Level)     ;


#endif /* DIO_PRIVATE_H_ */

/**
*@file       Std_Types.h
*@version    1.0.0
*@details    To make process on specific bits in registers or variables
*@author     Shehab aldeen mohammed abdalah
*/

#ifndef BIT_MATH_H_
#define BIT_MATH_H_


/****************************************************************************
**************************  macros   ***************************************
*****************************************************************************/

#define SET_BIT(var,bit)  ( var = var | 1<<(bit) )        // OR with 1
#define CLR_BIT(var,bit)  ( var = var & ~(1<<(bit)) )     // AND with 0
#define GET_BIT(var,bit)  ( (var>>bit) & 1 )              // AND with 1 
#define TOGGLE_BIT(var,bit) ( var = var ^(1<<bit) )       // XOR with 1 

#endif /* Bit_Math.h */


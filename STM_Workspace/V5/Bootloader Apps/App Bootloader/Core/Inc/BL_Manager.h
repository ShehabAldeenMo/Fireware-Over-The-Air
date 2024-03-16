/**
 ******************************************************************************
 * @file name      : BL_Manager.h
 * @Author         : Shehab aldeen mohammed, Ali Mamdouh
 *
 ******************************************************************************
 * @Notes:
 *
 *
 ******************************************************************************
 ******************************************************************************
*/




#ifndef INC_BL_MANAGER_H_
#define INC_BL_MANAGER_H_
/****************************************************************************
 ******************************  Including  *********************************
 ****************************************************************************/
#include "stm32f1xx_hal.h"






/*****************************************************************************
 **************************  Macro Declerations  *****************************
 *****************************************************************************/
#define FLAG_APP_ON                              0xAAAAAAAAU
#define FLAG_APP_OFF                             0xEEEEEEEEU
#define FLAG_APP1_ADDRESS                        0x0801FC00U
#define FLAG_APP2_ADDRESS                        0x0801FC04U
#define FLAG_BL_ADDRESS                          0x0801FC08U
#define BL_BEGIN_MATCH                           0xAAU
#define BL_BEGIN_NOT_MATCH                       0xEEU





/*****************************************************************************
 ************************* Data Types Declerations  **************************
 *****************************************************************************/
typedef void (*pFunc)(void) ;






/*****************************************************************************
 *********************** Global Variables Declerations  **********************
 *****************************************************************************/
extern uint8_t BL_Jump_Status; //Should used with UART
extern uint8_t BL_Jump_Flag;   //Used with BL response








/*****************************************************************************
 ********************** Software Interface Declerations  *********************
 *****************************************************************************/
void Write_Program_Flag(uint32_t Address, uint32_t Value);
uint32_t Get_Program_Flag_Status(uint32_t Address);
void BL_Response(void);
void Jump_To_Address (uint32_t Address);
void BL_Manager(void);



#endif /* INC_BL_MANAGER_H_ */

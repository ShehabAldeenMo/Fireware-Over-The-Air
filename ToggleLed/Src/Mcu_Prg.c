/*
*@file       MCU_Prg.c
*@version    2.1.0
*@details    It has the core code of driver
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/

/****************************************************************************
*****************************  Includes   ***********************************
*****************************************************************************/
#include <Mcu_Interface.h>
#include <Mcu_Private.h>
#include "Mcu_Cfg.h"

/* To used in Mcu_Init */
extern Mcu_ConfigType Config_Clock;
const Mcu_ConfigType* ConfigPtr = & Config_Clock ;

/* TO access on APB1 APB2 AHB registers */
static uint32 PerpherialsAPB1;
static uint32 PerpherialsAPB2;
static uint32 PerpherialsAHB;

/* Create semphores to protect shared resources */
#if MCU_DESIGN == MCU_FREERTOS
SemaphoreHandle_t MCU_SemConfig_Clock    = NULL ;
SemaphoreHandle_t MCU_SemPerpherialsAPB1 = NULL ;
SemaphoreHandle_t MCU_SemPerpherialsAPB2 = NULL ;
SemaphoreHandle_t MCU_SemPerpherialsAHB  = NULL ;
#endif

/**
 * \section Service_Name
 * Mcu_Init
 *
 * \section Description
 * To initialize all clocks.
 *
 * \section Service_ID
 * 0x00
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * 
 * \section Parameters
 * \param[in] None 
 * \param[inout] None
 * \param[out] None
 * \return None
 * 
 *
 * \section Rational
 * To initialize all clocks.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * :check on coming pointer NULL_PTR == ConfigPtr or not(go to det report if yes);
 * #yellow:Create four binary semaphores and take from them only MCU_SemConfig_Clock **__(FreeRTOS Cfg on)__**; 
 * :select HSE[16]; 
 * :wait until the clock is stabled;
 * :if clock is not stable go to infinite loop; 
 * :Turn off or on external clock and clock security based **on configration**; 
 * :Select clock prescalers for peripherals and buses based on values put in Cfg.c;  
 * #yellow:Give binary semaphores MCU_SemConfig_Clock **__(FreeRTOS Cfg on)__**; 
 *
 * end
 *
 * @enduml
 *
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == ConfigPtr= NULL_PTR ==
 *
 * "Mcu_Init\nFunction" -[#red]> "Det_ReportError\nFunction": **MODULE_ID, Mcu_Init_ID, MCU_E_PARAM_POINTER**
 * note right #red: Stuck in infinite loop.  
 *
 * == FreeRTOS Cfg on ==
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemPerpherialsAPB1**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemPerpherialsAPB2**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Mcu_Init\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemPerpherialsAHB**
 * note right #green: Gives dedicated semaphore. 
 * 
 * == LOCAL_Time_limit = 0 ==
 *
 * "Mcu_Init\nFunction" -[#red]> "Det_ReportError\nFunction": **MODULE_ID, Mcu_Init_ID, MCU_E_PARAM_POINTER**
 * note right #red: Stuck in infinite loop.  
 *
 * == FreeRTOS Cfg on ==
 * "Mcu_Init\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemConfig_Clock**
 * note right #green: Gives dedicated semaphore. 
 * @enduml
 *
 *
 *
 *
 */
void   Mcu_Init () {
	/* check on coming pointer */
	if (NULL_PTR == ConfigPtr){
		Det_ReportError(MODULE_ID, Mcu_Init_ID, MCU_E_PARAM_POINTER);
	}

/* To protect shared resources by semphore
 * It's created taken
 **/
#if MCU_DESIGN == MCU_FREERTOS
	MCU_SemConfig_Clock    = xSemaphoreCreateBinary();
	MCU_SemPerpherialsAPB1 = xSemaphoreCreateBinary();
	xSemaphoreGive(MCU_SemPerpherialsAPB1);
	MCU_SemPerpherialsAPB2 = xSemaphoreCreateBinary();
	xSemaphoreGive(MCU_SemPerpherialsAPB2);
	MCU_SemPerpherialsAHB  = xSemaphoreCreateBinary();
	xSemaphoreGive(MCU_SemPerpherialsAHB);
#endif

	/* to select HSE[16] */
	uint32 Local_RCC_Value = ConfigPtr->CLOCK_SYS | ( ConfigPtr->HSITRIM << 3 ) ;
	RCC->CR      = Local_RCC_Value ;

	/* to check that the clock is stabled  :
	 PLLRDY    or   HSERDY   or   HSIRDY
	 */
	uint32 LOCAL_Clock_Flag = ( ConfigPtr->CLOCK_SYS << 1 ) ;  /*  to check on one in the bit of flag  */
	uint16 LOCAL_Time_limit = TIMEOUT_ROOF ;

	while ( (LOCAL_Clock_Flag & RCC->CR) && (0 == LOCAL_Time_limit))
	{
		LOCAL_Time_limit--;
	}

	if (0 == LOCAL_Time_limit){
		Det_ReportError(MODULE_ID, Mcu_Init_ID, MCU_E_TIMEOUT);
	}

	/* to turn off VCC in PIN out that used in HSE to get clock */
#if   HSE == HSE_CRSTYAL
	RCC->CR = RCC->CR & ~(1<<(HSE_BYP));
#elif HSE == HSE_RC
	RCC->CR = RCC->CR | 1<<(HSE_BYP);
#endif

#if CSS_CFG == CSS_OFF
	RCC->CR = RCC->CR & ~(1<<(CSSON));
#elif CSS_CFG == CSS_ON
	RCC->CR = RCC->CR | 1<<(CSSON);
#endif

	/* to select SW[1:0]
	 00: HSI selected as system clock
	 01: HSE selected as system clock
     10: PLL selected as system clock
	  */
	uint32 Local_RCC_CFGR = ConfigPtr->PLL_MUL_NUMBER
			| ConfigPtr->PLL_ENTRY_CLOCK | ConfigPtr->ADC_PRESCALAR_
			| ConfigPtr->APB2_PRESCALAR_ | ConfigPtr->APB1_PRESCALAR_
			| ConfigPtr->AHB_PRESCALAR_ | ConfigPtr->SW_ENABLE_CLOCK;
	RCC->CFGR    = Local_RCC_CFGR ;

#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreGive(MCU_SemConfig_Clock);
#endif
}






/**
 * \section Service_Name
 * Mcu_EnumSetPerAPB1
 *
 * \section Description
 * Set dedicated peripheral in APB1 bus.
 *
 * \section Service_ID
 * 0x01
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * 
 * \section Parameters
 * \param[in] BUS_PERPHERIALS Mcu_Per 
 * \param[inout] None
 * \param[out] None
 * \return Error_State
 * 
 *
 * \section Rational
 * Set dedicated peripheral in APB1 bus.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * #yellow:Take semaphore MCU_SemPerpherialsAPB1 **__(FreeRTOS Cfg on)__**; 
 * if (Mcu_Per is in range of peripherals of APB1) then (**yes**)
 * :enable dedicated peripheral; 
 * #blue:Return Ok; 
 * endif
 * #yellow:Give binary semaphore MCU_SemPerpherialsAPB1 **__(FreeRTOS Cfg on)__**;
 * #blue:Return Not_Ok;  
 *
 * end
 *
 * @enduml
 *
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == FreeRTOS Cfg on ==
 *
 * "Mcu_EnumSetPerAPB1\nFunction" -> "xSemaphoreTake\nFunction": **MCU_SemPerpherialsAPB1, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 *
 * "Mcu_EnumSetPerAPB1\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemPerpherialsAPB1**
 * note right #green: Gives dedicated semaphore. 
 *
 *
 * @enduml
 *
 *
 *
 *
 */
Error_State Mcu_EnumSetPerAPB1(BUS_PERPHERIALS Mcu_Per ) {
#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreTake(MCU_SemPerpherialsAPB1,portMAX_DELAY);
#endif
	if ( Mcu_Per >= APB1_PER_TIM2  && Mcu_Per <= APB1_PER_DAC ){
		PerpherialsAPB1 |= Mcu_Per ;
	return E_OK;
	}
#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreGive(MCU_SemPerpherialsAPB1);
#endif	
	return E_NOT_OK ;
}





/**
 * \section Service_Name
 * Mcu_EnumSetPerAPB2
 *
 * \section Description
 * Set dedicated peripheral in APB2 bus.
 *
 * \section Service_ID
 * 0x02
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * 
 * \section Parameters
 * \param[in] BUS_PERPHERIALS Mcu_Per 
 * \param[inout] None
 * \param[out] None
 * \return Error_State
 * 
 *
 * \section Rational
 * Set dedicated peripheral in APB2 bus.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * #yellow:Take semaphore MCU_SemPerpherialsAPB2 **__(FreeRTOS Cfg on)__**; 
 * if (Mcu_Per is in range of peripherals of APB2) then (**yes**)
 * :enable dedicated peripheral; 
 * #blue:Return Ok; 
 * endif
 * #yellow:Give binary semaphore MCU_SemPerpherialsAPB2 **__(FreeRTOS Cfg on)__**;
 * #blue:Return Not_Ok;  
 *
 * end
 *
 * @enduml
 *
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == FreeRTOS Cfg on ==
 *
 * "Mcu_EnumSetPerAPB2\nFunction" -> "xSemaphoreTake\nFunction": **MCU_SemPerpherialsAPB2, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 *
 * "Mcu_EnumSetPerAPB2\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemPerpherialsAPB2**
 * note right #green: Gives dedicated semaphore. 
 *
 *
 * @enduml
 *
 *
 *
 *
 */

Error_State       Mcu_EnumSetPerAPB2(BUS_PERPHERIALS Mcu_Per ) {
#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreTake(MCU_SemPerpherialsAPB2,portMAX_DELAY);
#endif
	if (Mcu_Per >= APB2_PER_AFIO  && Mcu_Per <= APB2_PER_TIM11){
		PerpherialsAPB2 |= Mcu_Per ;
	return E_OK;
	}
#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreGive(MCU_SemPerpherialsAPB2);
#endif
	
	return E_NOT_OK ;
}


/**
 * \section Service_Name
 * Mcu_EnumSetPerAHB
 *
 * \section Description
 * Set dedicated peripheral in AHB bus.
 *
 * \section Service_ID
 * 0x03
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * 
 * \section Parameters
 * \param[in] BUS_PERPHERIALS Mcu_Per 
 * \param[inout] None
 * \param[out] None
 * \return Error_State
 * 
 *
 * \section Rational
 * Set dedicated peripheral in AHB bus.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * #yellow:Take semaphore MCU_SemPerpherialsAHB **__(FreeRTOS Cfg on)__**; 
 * if (Mcu_Per is in range of peripherals of AHB) then (**yes**)
 * :enable dedicated peripheral; 
 * #blue:Return Ok; 
 * endif
 * #yellow:Give binary semaphore MCU_SemPerpherialsAHB **__(FreeRTOS Cfg on)__**;
 * #blue:Return Not_Ok;  
 *
 * end
 *
 * @enduml
 *
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == FreeRTOS Cfg on ==
 *
 * "Mcu_EnumSetPerAHB\nFunction" -> "xSemaphoreTake\nFunction": **MCU_SemPerpherialsAPB2, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 *
 * "Mcu_EnumSetPerAHB\nFunction" -> "xSemaphoreGive\nFunction": **MCU_SemPerpherialsAPB2**
 * note right #green: Gives dedicated semaphore. 
 *
 *
 * @enduml
 *
 *
 *
 *
 */
Error_State       Mcu_EnumSetPerAHB (BUS_PERPHERIALS Mcu_Per ){
#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreTake(MCU_SemPerpherialsAHB,portMAX_DELAY);
#endif
	if (Mcu_Per >= AHB_PER_DMA_1  && Mcu_Per <= AHB_PER_SDIO){
		PerpherialsAHB  |= Mcu_Per ;
	return E_OK;
	}
#if MCU_DESIGN == MCU_FREERTOS
	xSemaphoreGive(MCU_SemPerpherialsAHB);
#endif	
	return E_NOT_OK ;
}


/**
 * \section Service_Name
 * Mcu_VidRunnable
 *
 * \section Description
 * Set all peripherals seleted by setter.
 *
 * \section Service_ID
 * 0x04
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * 
 * \section Parameters
 * \param[in] None
 * \param[inout] None
 * \param[out] None
 * \return None
 * 
 *
 * \section Rational
 * Set all peripherals seleted by setter.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * if (PerpherialsAPB1 != NULL) then (**yes**)
 * #green:enable Perpherials of APB1 **(selected from setter function)**; 
 * endif
 *
 * if (PerpherialsAPB2 != NULL) then (**yes**)
 * #green:enable Perpherials of APB2 **(selected from setter function)**; 
 * endif
 * if (PerpherialsAHB != NULL) then (**yes**)
 * #green:enable Perpherials of AHB **(selected from setter function)**; 
 * endif 
 * end
 *
 * @enduml
 *
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == Dedicated peripheral buffer != NULL ==
 *
 * "Mcu_VidRunnable\nFunction" -> "Mcu_SetMode\nFunction": **PerpherialsAPB1**
 * note right #green: Enable selected peripherals. 
 *
 * "Mcu_VidRunnable\nFunction" -> "Mcu_SetMode\nFunction": **PerpherialsAPB2**
 * note right #green: Enable selected peripherals. 
 *
 * "Mcu_VidRunnable\nFunction" -> "Mcu_SetMode\nFunction": **PerpherialsAHB**
 * note right #green: Enable selected peripherals. 
 *
 *
 * @enduml
 *
 *
 *
 *
 */
void  Mcu_VidRunnable (void){
	if (PerpherialsAPB1 != NULL){
		Mcu_SetMode(PerpherialsAPB1);
	}

	if (PerpherialsAPB2 != NULL){
		Mcu_SetMode(PerpherialsAPB2);
	}

	if (PerpherialsAHB  != NULL){
		Mcu_SetMode(PerpherialsAHB) ;
	}
}



/**
 * \section Service_Name
 * Mcu_SetMode
 *
 * \section Description
 * Set all peripherals seleted by in peripherals buffer.
 *
 * \section Service_ID
 * 0x05
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 *
 * \section Sync_Async
 * Synchronous
 *
 * 
 * \section Parameters
 * \param[in] None
 * \param[inout] None
 * \param[out] None
 * \return None
 * 
 *
 * \section Rational
 * Set all peripherals seleted by setter.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * if (**McuMode in range of peripherals of APB1?**) then (**yes**)
 * #green:Enable all selected peripherals selected in the buffer**;
 * elseif (**McuMode in range of peripherals of APB2?**) then (**yes**) 
 * #green:Enable all selected peripherals selected in the buffer**;
 * elseif (**McuMode in range of peripherals of AHB?**) then (**yes**) 
 * #green:Enable all selected peripherals selected in the buffer**; 
 * else
 * #red:Det_ReportError(infinite loop);
 * endif
 * end
 *
 * @enduml
 *
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == Out of range condition ==
 *
 * "Mcu_SetMode\nFunction" -[#red]> "Det_ReportError\nFunction": **MODULE_ID, Mcu_SetMode_ID, MCU_E_PARAM_MODE**
 * note right #red: Stuck in infinite loop. 
 *
 *
 * @enduml
 *
 *
 *
 *
 */
void Mcu_SetMode (Mcu_ModeType McuMode){
	if ( McuMode >= APB1_PER_TIM2  && McuMode <= APB1_PER_DAC ){
		RCC->APB1ENR  |= McuMode ;
	}
	else if (McuMode >= APB2_PER_AFIO  && McuMode <= APB2_PER_TIM11){
		McuMode = McuMode & ~(1<<(31));
		RCC->APB2ENR  |= McuMode ;
	}
	else if (McuMode >= AHB_PER_DMA_1  && McuMode <= AHB_PER_SDIO){
		McuMode = McuMode & ~(1<<(30));
		RCC->AHBENR  |= McuMode ;
	}
	else {
		Det_ReportError(MODULE_ID, Mcu_SetMode_ID, MCU_E_PARAM_MODE);
	}
}

/**
 * \section Service_Name
 * Dio_GetVersionInfo
 *
 * \section Description
 * Function to Get the sw_major_version, sw_minor_version & sw_patch_version
 *
 * \section Req_ID
 * 0x06
 *
 * \section Scope
 * Public
 *
 * \section Re-entrancy
 * Reentrant
 *
 * \section Sync_Async
 * Synchronous
 *
 * \section Parameters
 * \param[in] None
 * \param[inout] None
 * \param[out] VersionInfo 
 * \return None
 * 
 *
 * \section Rational
 *  Service to get the version information of this module.
 *
 *
 *
 *
 *
 *
 *
 * \section Activity_Diagram
 *
 * @startuml 
 * start
 * #green:Put ModuleID in elements of versioninfo pointer to structure;
 * #green:Put Dio Major, Minor and Patch versions in elements of versioninfo pointer to structure;
 * #green:Put VENDOR_ID in elements of versioninfo pointer to structure; 
 * end
 *
 * @enduml
 *
 *
 *
 *
 *
 */
void Mcu_GetVersionInfo(Std_VersionInfoType* versioninfo) {
	versioninfo->moduleID         = MODULE_ID         ;
	versioninfo->sw_major_version = MCU_MAJOR_VERSION ;
	versioninfo->sw_minor_version = MCU_MINOR_VERSION ;
	versioninfo->sw_patch_version = MCU_PATCH_VERSION ;
	versioninfo->vendorID         = VENDOR_ID         ;
}
/**
*@file       Port_Interface.h
*@version    2.1.0
*@details    implementation of driver
*@author     Shehab aldeen mohammed abdalah
*            Ali  Mamdouh
*/

/*===========================================================================
*   Platform         : ARM
*   Peripherial      : STM32F103C8T6
*   SW Version       : 2.1.0
============================================================================*/

/****************************************************************************
*****************************  Includes   ***********************************
*****************************************************************************/
#include "Port_Interface.h"
#include "Port_Private.h"
#include "Port_Cfg.h"


/* Port_ArrOfPorts store the default values of port that you declare it in cfg.c  */
extern Port_Cfg Port_ArrOfPorts[NUM_OF_PORT] ;


/* Port_ArryOfPins declare in prg.c to access by setter and getter functions only to edit on it storage within
   runtime to use in runnable functions as buffer and implement the runnable using supporting functions  */
static Port_PinModeType  Port_ArryOfPins      [AMOUNT_PINS] ;
static Port_PinModeType  Port_ArryOfActivition[AMOUNT_PINS] ;

/* Create semphores to protect shared resources */
#if PORT_DESIGN == PORT_FREERTOS
SemaphoreHandle_t PORT_SemArrOfPorts       = NULL ;
SemaphoreHandle_t PORT_SemArryOfPins       = NULL ;
SemaphoreHandle_t PORT_SemArryOfActivition = NULL ;
#endif


/**
 * \section Service_Name
 * Port_GetVersionInfo
 *
 * \section Description
 * Function to Get the sw_major_version, sw_minor_version & sw_patch_version
 *
 * \section Req_ID
 * 0x00
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
 * #green:Put Port Major, Minor and Patch versions in elements of versioninfo pointer to structure;
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
void    Port_GetVersionInfo  (Std_VersionInfoType* versioninfo){
	versioninfo->moduleID         = MODULE_ID         ;
	versioninfo->sw_major_version = PORT_MAJOR_VERSION ;
	versioninfo->sw_minor_version = PORT_MINOR_VERSION ;
	versioninfo->sw_patch_version = PORT_PATCH_VERSION ;
	versioninfo->vendorID         = VENDOR_ID         ;
}


/**
 * \section Service_Name
 * Port_VidInit
 *
 * \section Description
 * To initialize all pins with the configured modes that used in Cfg.c.
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
 *  To initialize all pins with the configured modes.
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
 * #yellow:Create three binary semaphores then store them in Port_SemArrOfPorts, Port_SemArryOfPins, Port_SemArryOfActivition handlers **__(FreeRTOS Cfg on)__**; 
 * #yellow:Give two binary semaphores Port_SemArrOfPins, Port_SemArryOfActivition **__(FreeRTOS Cfg on)__**; 
 *
 * while (**i < Port_Num?**)
 *
 * switch (**ArrOfPorts[i]?**)
 *
 * case ( **condition PortA** )
 * :Reset all pins of the port;
 * #green:Set dedicated modes selected in Cfg.c in all pins of the port;
 * case ( **condition PortB** )
 * :Reset all pins of the port;
 * :Set dedicated modes selected in Cfg.c in all pins of the port;
 *
 * case ( **condition PortC** )
 * :Reset all pins of the port;
 * :Set dedicated modes selected in Cfg.c in all pins of the port;
 *
 * endswitch
 *
 * :i++; 
 * endwhile
 *
 * #yellow:Give binary semaphores Port_SemArrOfPorts **__(FreeRTOS Cfg on)__**; 
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
 * "Port_VidInit\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Port_VidInit\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Port_VidInit\nFunction" -> "xSemaphoreGive\nFunction": **Port_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Port_VidInit\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Port_VidInit\nFunction" -> "xSemaphoreGive\nFunction": **Port_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Port_VidInit\nFunction" -> "xSemaphoreGive\nFunction": **Port_SemArrOfPorts**
 * note right #green: Gives dedicated semaphore. 
 * 
 *
 * @enduml
 *
 *
 *
 *
 */
void Port_VidInit (void){
	/* To protect shared resources by semphore
	 * It's created taken
	 **/
#if PORT_DESIGN == PORT_FREERTOS
	PORT_SemArrOfPorts = xSemaphoreCreateBinary();
	PORT_SemArryOfPins = xSemaphoreCreateBinary();
	xSemaphoreGive(PORT_SemArryOfPins);
	PORT_SemArryOfActivition  = xSemaphoreCreateBinary();
	xSemaphoreGive(PORT_SemArryOfActivition);
#endif
	for (uint8 i = 0 ; i < NUM_OF_PORT ; i++){
		switch (Port_ArrOfPorts[i].Port_Num){
		case Port_GPIO_A :
			/* to Reset register */
			GPIOA->CRL.Reg = 0x00000000 ;
			GPIOA->CRH.Reg = 0x00000000 ;

			/* to set mode of pins */
			for (uint8 i = 0 ; i < 8 ; i++ ){
				GPIOA->CRL.Reg |= ( Port_ArrOfPorts[Port_GPIO_A].Mode <<(i  * BASE_CRL_CRH) ) ;
			}
			for (uint8 i = 0 ; i < 8 ; i++ ){
				GPIOA->CRH.Reg |= ( Port_ArrOfPorts[Port_GPIO_A].Mode <<(i  * BASE_CRL_CRH) ) ;
			}
			break ;
		case Port_GPIO_B :
			/* to Reset register */
			GPIOB->CRL.Reg = (0x00000000);
			GPIOB->CRH.Reg = (0x00000000);
			/* to set mode of pins */
			for (uint8 i = 0 ; i < 8 ; i++ ){
				GPIOB->CRL.Reg |= ( Port_ArrOfPorts[Port_GPIO_B].Mode <<(i  * BASE_CRL_CRH) ) ;
			}
			for (uint8 i = 0 ; i < 8 ; i++ ){
				GPIOB->CRH.Reg |= ( Port_ArrOfPorts[Port_GPIO_B].Mode <<(i  * BASE_CRL_CRH) ) ;
			}
			break ;
		case Port_GPIO_C :
			/* to Reset register */
			GPIOC->CRL.Reg = (0x00000000);
			GPIOC->CRH.Reg = (0x00000000);

			/* to set mode of pins of port C */
			GPIOC->CRH.Reg |= ( Port_ArrOfPorts[Port_GPIO_C].Mode <<(5  * BASE_CRL_CRH) ) ;
			GPIOC->CRH.Reg |= ( Port_ArrOfPorts[Port_GPIO_C].Mode <<(6  * BASE_CRL_CRH) ) ;
			break ;
		}
	}
#if PORT_DESIGN == PORT_FREERTOS
	xSemaphoreGive(PORT_SemArrOfPorts);
#endif
}



/**
 * \section Service_Name
 *  Port_EnumSetterPin
 *
 * \section Description
 * To set some thing in buffer call  Port_EnumSetterPin.
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
 * \param[in] Port_PinType Copy_ChannelId ,Port_PinModeType Copy_Mode
 * \param[inout] None
 * \param[out] None
 * \return Error_State
 * 
 *
 * \section Rational
 *  if you want to set some thing in buffer call  Port_EnumSetterPin then Port_VidRunnable will execute the configured data.
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
 * if (**Copy_ChannelId(input) in range of Micro pins?**) then (**yes**)
 * #yellow:Take binary semaphores Port_SemArryOfPins and Port_SemArryOfActivition with time portMAX_DELAY **__(FreeRTOS Cfg on)__**; 	 
 * #green:put in Array[ChannelId] of **__Pin Buffer__** its modes;
 * #green:put in Array[ChannelId] of **__Activation Buffer__** that it is ON;
 * #yellow:Give binary semaphores Port_SemArryOfPins and Port_SemArryOfActivition **__(FreeRTOS Cfg on)__**;  
 * #blue:return E_OK;  
 * endif 
 *
 * #red:Return E_INVALID_PARAMETER;  
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
 * "Port_EnumSetterPin\nFunction" -> "xSemaphoreTake\nFunction": **PORT_SemArryOfPins,portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 * "Port_EnumSetterPin\nFunction" -> "xSemaphoreTake\nFunction": **PORT_SemArryOfActivition,portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 * "Port_EnumSetterPin\nFunction" -> "xSemaphoreGive\nFunction": **PORT_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Port_EnumSetterPin\nFunction" -> "xSemaphoreGive\nFunction": **PORT_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * @enduml
 *
 *
 *
 *
 */

Error_State       Port_EnumSetterPin(Port_PinType Copy_ChannelId ,Port_PinModeType Copy_Mode){
	if ( Copy_ChannelId >=Port_A0 && Copy_ChannelId <=Port_C14){

#if PORT_DESIGN == PORT_FREERTOS
	xSemaphoreTake(PORT_SemArryOfPins,portMAX_DELAY);
	xSemaphoreTake(PORT_SemArryOfActivition,portMAX_DELAY);
#endif
		Port_ArryOfPins[Copy_ChannelId]       = Copy_Mode      ;
		Port_ArryOfActivition[Copy_ChannelId] = STD_ON         ;
#if PORT_DESIGN == PORT_FREERTOS
	xSemaphoreGive(PORT_SemArryOfPins);
	xSemaphoreGive(PORT_SemArryOfActivition);
#endif
		return E_OK ;
	}
	return E_INVALID_PARAMETER ;
}



/**
 * \section Service_Name
 * Port_VidRunnable
 *
 * \section Description
 * To execute the desired configuration within runtime "refresh pins".
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
 * \param[in] None
 * \param[inout] None
 * \param[out] None
 * \return None
 * 
 *
 * \section Rational
 * To execute the desired configuration.
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
 * #yellow:Take binary semaphores Port_SemArryOfPins and Port_SemArryOfActivition with time portMAX_DELAY **__(FreeRTOS Cfg on)__**; 
 * :Create variable i and initialize it with 0;  
 * while(**i < = A15**)
 * 
 * if (pin[i] is **__Active__**?) then (**yes**)	 
 * #green:Set this pin to **__Mode of its buffer__**;
 * endif 
 *
 * :i++;
 * endwhile	 
 *
 * while(**i < = B15**)
 * 
 * if (pin[i] is **__Active__**?) then (**yes**)	 
 * #green:Set this pin to **__Mode of its buffer__**;
 * endif 
 *
 * :i++;
 * endwhile	 
 *
 * while(**i < = C15**)
 * 
 * if (pin[i] is **__Active__**?) then (**yes**)	 
 * #green:Set this pin to **__Mode of its buffer__**;
 * endif 
 *
 * :i++;
 * endwhile	 
 *
 * #yellow:Give binary semaphores Port_SemArryOfPins and Port_SemArryOfActivition **__(FreeRTOS Cfg on)__**;  
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
 * "Port_VidRunnable\nFunction" -> "xSemaphoreTake\nFunction": **Port_SemArryOfPins, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 * "Port_VidRunnable\nFunction" -> "xSemaphoreTake\nFunction": **Port_SemArryOfActivition, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 * == Pins of A is Activated ==
 *
 * "Port_VidRunnable\nFunction" -> "Port_SetPinMode\nFunction": **GPIOA,i, Port_ArryOfPins[i]**
 * note right #blue: Set Mode in the buffer to dedicated pin. 
 *
 * == Pins of B is Activated ==
 *
 * "Port_VidRunnable\nFunction" -> "Port_SetPinMode\nFunction": **GPIOB,(i & ~(0x10)), Port_ArryOfPins[i]**
 * note right #blue: Set Mode in the buffer to dedicated pin. 
 *
 * == Pins of C is Activated and Out ==
 *
 * "Port_VidRunnable\nFunction" -> "Port_SetPinMode\nFunction": **GPIOC,(Port_C13 & ~(0x10)), Port_ArryOfPins[i]**
 * note right #blue: Set Mode in the buffer to dedicated pin. 
 *
 * == FreeRTOS Cfg on ==
 *
 * "Port_VidRunnable\nFunction" -> "xSemaphoreGive\nFunction": **Port_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Port_VidRunnable\nFunction" -> "xSemaphoreGive\nFunction": **Port_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * @enduml
 *
 *
 *
 *
 */
void  Port_VidRunnable (void) {
	uint8 i = 0 ;

#if PORT_DESIGN == PORT_FREERTOS
	xSemaphoreTake(PORT_SemArryOfPins,portMAX_DELAY);
	xSemaphoreTake(PORT_SemArryOfActivition,portMAX_DELAY);
#endif
	while(  i <= Port_A15 ){
		if (Port_ArryOfActivition[i] != STD_OFF){
			Port_SetPinMode(GPIOA,i, Port_ArryOfPins[i]);
		}
		i++;
	}

	while(  i <= Port_B15 ){
		if (Port_ArryOfActivition[i] != STD_OFF){
			Port_SetPinMode(GPIOB,(i & ~(0x10)), Port_ArryOfPins[i]);
		}
		i++;
	}

	if (Port_ArryOfActivition[Port_C13] != STD_OFF){
		Port_SetPinMode(GPIOC,(Port_C13 & ~(0x10)), Port_ArryOfPins[Port_C13]);
	}

	if (Port_ArryOfActivition[Port_C14] != STD_OFF){
		Port_SetPinMode(GPIOC,(Port_C14 & ~(0x10)), Port_ArryOfPins[Port_C13]);
	}

#if PORT_DESIGN == PORT_FREERTOS
	xSemaphoreGive(PORT_SemArryOfPins);
	xSemaphoreGive(PORT_SemArryOfActivition);
#endif

}


/**
 * \section Service_Name
 * Port_SetPinMode
 *
 * \section Description
 * To select the type of pin input or output.
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
 * \param[in] Port_PinType , Port_PinModeType
 * \param[inout] None
 * \param[out] None
 * \return None
 * 
 *
 * \section Rational
 *  To select the type of pin input or output.
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
 *
 * if (**Pin <= LOW_PIN?**) then (**yes**)	 
 * #green:Set Mode of this pin through CRL register;
 * else (**no**)
 * #green:Set Mode of this pin through CRL register; 
 * endif 
 * end
 *
 * @enduml
 *
 *
 *
 *
 *
 */
/*
  Input  : Port_PinType , Port_PinModeType
  output : void
  description :To select the type of pin input or output
  =================================================
	In input mode (MODE[1:0]=00):
	00: Analog mode
	01: Floating input (reset state)
	10: Input with pull-up / pull-down
	11: Reserved

	In output mode (MODE[1:0] > 00):
	00: General purpose output push-pull
	01: General purpose output Open-drain
	10: Alternate function output Push-pull
	11: Alternate function output Open-drain

	MODEy[1:0]
	00: Input mode (reset state)
	01: Output mode, max speed 10 MHz.
	10: Output mode, max speed 2 MHz.
	11: Output mode, max speed 50 MHz
=============================================== */
void    Port_SetPinMode      (GPIOX_REG *GPIOX,Port_PinType Pin,Port_PinModeType Mode){
	if (Pin <= LOW_PIN) {
		GPIOX->CRL.Reg  &= ~( 0xf << (Pin * BASE_CRL_CRH)) ;
		GPIOX->CRL.Reg |= (Mode << (Pin * BASE_CRL_CRH));
	} else {
		GPIOX->CRH.Reg  &= ~( 0xf << ( (Pin-8) * BASE_CRL_CRH)) ;
		GPIOX->CRH.Reg |= (Mode << ((Pin - 8) * BASE_CRL_CRH));
	}
}
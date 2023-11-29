/*
*@file       Dio_Prg.c
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
#include "DIO_Interface.h"
#include "DIO_Private.h"
#include "Dio_Cfg.h"

/* ArrOfPorts store the default values of port that you declare it in cfg.c  */
extern Dio_Cfg ArrOfPorts[NUM_OF_PORT];

/* ArryOfPins declare in prg.c to access by setter and getter functions only to edit on it storage within
 runtime to use in runnable functions as buffer and implement the runnable using supporting functions  */
static Dio_PortLevelType ArryOfPins[NUM_OF_PINS];
static Dio_PortLevelType ArryOfActivition[NUM_OF_PINS][2];

/* Create semphores to protect shared resources */
#if DIO_DESIGN == DIO_FREERTOS
SemaphoreHandle_t DIO_SemArrOfPorts       = NULL ;
SemaphoreHandle_t DIO_SemArryOfPins       = NULL ;
SemaphoreHandle_t DIO_SemArryOfActivition = NULL ;
#endif


/**
 * \section Service_Name
 * Dio_VidInit
 *
 * \section Description
 * To initialize all pins with the configured types that used in Cfg.c.
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
 *  To initialize all pins with the configured types.
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
 * #yellow:Create three binary semaphores then store them in DIO_SemArrOfPorts, DIO_SemArryOfPins, DIO_SemArryOfActivition handlers **__(FreeRTOS Cfg on)__**; 
 * #yellow:Give two binary semaphores DIO_SemArrOfPorts, DIO_SemArryOfActivition **__(FreeRTOS Cfg on)__**; 
 *
 * while (**i < Port_Num?**)
 *
 * switch (**ArrOfPorts[i]?**)
 *
 * case ( **condition PortA** )
 * if (Initialization of port **__buffer__** is high?) then (**yes**)
 * #green:Set dedicated port to **__High__**;
 * elseif (Initialization of port **__buffer__** is Low?) then (**yes**) 
 * #green:Set dedicated port to **__Low__**;
 * endif
 *
 * case ( **condition PortB** )
 * if (Initialization of port **__buffer__** is high?) then (**yes**)
 * #green:Set dedicated port to **__High__**;
 * elseif (Initialization of port **__buffer__** is Low?) then (**yes**) 
 * #green:Set dedicated port to **__Low__**;
 * endif
 *
 * case ( **condition PortC** )
 * if (Initialization of port **__buffer__** is high?) then (**yes**)
 * #green:Set dedicated port to **__High__**;
 * elseif (Initialization of port **__buffer__** is Low?) then (**yes**) 
 * #green:Set dedicated port to **__Low__**;
 * endif 
 *
 * endswitch
 *
 * :i++; 
 * endwhile
 *
 * #yellow:Give binary semaphores DIO_SemArrOfPorts **__(FreeRTOS Cfg on)__**; 
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
 * "Dio_VidInit\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Dio_VidInit\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Dio_VidInit\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Dio_VidInit\nFunction" -> "xSemaphoreCreateBinary\nFunction":
 * note right #green: Create semaphore then return handler to it. 
 *
 * "Dio_VidInit\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Dio_VidInit\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArrOfPorts**
 * note right #green: Gives dedicated semaphore. 
 * 
 * == PortId < NUMBER_OF_Dio_PORTS ==
 *
 * "Dio_ReadPort\nFunction" -[#red]> "Det_ReportError\nFunction": **DIO_MODULE_ID, DIO_INSTANCE_ID, Dio_READ_PORT_SID, DIO_E_PARAM_INVALID_CHANNEL_ID**
 * note right #red: Stuck in infinite loop.  
 *
 * @enduml
 *
 *
 *
 *
 */

/* to initialize all pins with the configured types that used in Cfg.c */
void Dio_VidInit(void) {
	/* To protect shared resources by semphore
	 * It's created taken
	 **/
#if DIO_DESIGN == DIO_FREERTOS
	DIO_SemArrOfPorts = xSemaphoreCreateBinary();
	DIO_SemArryOfPins = xSemaphoreCreateBinary();
	xSemaphoreGive(DIO_SemArryOfPins);
	DIO_SemArryOfActivition  = xSemaphoreCreateBinary();
	xSemaphoreGive(DIO_SemArryOfActivition);
#endif
	for (uint8 i = 0; i < NUM_OF_PORT; i++) {
		switch (ArrOfPorts[i].Port_Num) {
		case Dio_GPIO_A:
			if (ArrOfPorts[i].Level == STD_ON) {
				GPIOA->ODR.Reg = 0x0000FFFF;
			} else if (ArrOfPorts[i].Level == STD_OFF) {
				GPIOA->ODR.Reg = 0x00000000;
			} else {
				/* Invalid input */
			}
			break;
		case Dio_GPIO_B:
			if (ArrOfPorts[i].Level == STD_ON) {
				GPIOB->ODR.Reg = 0x0000FFFF;
			} else if (ArrOfPorts[i].Level == STD_OFF) {
				GPIOB->ODR.Reg = 0x00000000;
			} else {
				/* Invalid input */
			}
			break;
		case Dio_GPIO_C:
			if (ArrOfPorts[i].Level == STD_ON) {
				GPIOC->ODR.Reg = 0x0000FFFF;
			} else if (ArrOfPorts[i].Level == STD_OFF) {
				GPIOC->ODR.Reg = 0x00000000;
			} else {
				/* Invalid input */
			}
			break;
		}
	}
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreGive(DIO_SemArrOfPorts);
#endif
}




/**
 * \section Service_Name
 * Dio_EnumSetterPin
 *
 * \section Description
 * To set some thing in buffer call Dio_EnuSetterPin.
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
 * \param[in] Dio_ChannelType Copy_ChannelId, Dio_LevelType Copy_Level
 * \param[inout] None
 * \param[out] None
 * \return Error_State
 * 
 *
 * \section Rational
 *  if you want to set some thing in buffer call Dio_EnuSetterPin then Dio_VidRunnable will execute the configured data.
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
 * #yellow:Take binary semaphores DIO_SemArryOfPins and DIO_SemArryOfActivition with time portMAX_DELAY **__(FreeRTOS Cfg on)__**; 
 * if (**Copy_ChannelId(input) in range of Micro pins?**) then (**yes**)
 * #green:put in Array[ChannelId] of **__Pin Buffer__** input level;
 * #green:put in Array[ChannelId] of **__Activation Buffer__** that it is ON and act as Output;
 * #yellow:Give binary semaphores DIO_SemArryOfPins and DIO_SemArryOfActivition **__(FreeRTOS Cfg on)__**;  
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
 * "Dio_EnumSetterPin\nFunction" -> "xSemaphoreTake\nFunction": **DIO_SemArryOfPins, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 * "Dio_EnumSetterPin\nFunction" -> "xSemaphoreTake\nFunction": **DIO_SemArryOfActivition, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 * "Dio_EnumSetterPin\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Dio_EnumSetterPin\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * @enduml
 *
 *
 *
 *
 */

/* if you want to set some thing in buffer call Dio_EnuSetterPin then Dio_VidRunnable will
 execute the configured data
 */
Error_State Dio_EnumSetterPin(Dio_ChannelType Copy_ChannelId,
		Dio_LevelType Copy_Level) {
	if (Copy_ChannelId >= Dio_A0 && Copy_ChannelId <= Dio_C14) {
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreTake(DIO_SemArryOfPins,portMAX_DELAY);
	xSemaphoreTake(DIO_SemArryOfActivition,portMAX_DELAY);
#endif		
		ArryOfPins[Copy_ChannelId] = Copy_Level;
		ArryOfActivition[Copy_ChannelId][0] = STD_ON;
		ArryOfActivition[Copy_ChannelId][1] = DIO_OUT;
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreGive(DIO_SemArryOfPins);
	xSemaphoreGive(DIO_SemArryOfActivition);
#endif
	return E_OK;
	}
	return E_INVALID_PARAMETER;
}


/**
 * \section Service_Name
 * Dio_EnumGetterPin
 *
 * \section Description
 * To get some thing from buffer.
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
 * \param[in] Dio_ChannelType Copy_ChannelId, 
 * \param[inout] None
 * \param[out] Dio_LevelType *Copy_Level
 * \return Error_State
 * 
 *
 * \section Rational
 * if you want to get some thing from buffer call Dio_VidRunnable then Dio_EnuSetterPin will return the updated data.
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
 * #yellow:Take binary semaphores DIO_SemArryOfPins and DIO_SemArryOfActivition with time portMAX_DELAY **__(FreeRTOS Cfg on)__**; 
 *
 * if (**Input Copy_Level variable to put value of buffer in points to NULL?**) then (**yes**)
 * #red:Return E_NOT_OK;    
 * endif  
 *
 * if (**Copy_ChannelId(input) in range of Micro pins?**) then (**yes**)
 * #green:Put in input variable the value of Array[ChannelId] **__(Pin Buffer)__**;
 * #green:Put in Array[ChannelId] of **__Activation Buffer__** that it is ON and act as Input;
 * #yellow:Give binary semaphores DIO_SemArryOfPins and DIO_SemArryOfActivition **__(FreeRTOS Cfg on)__**;  
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
 * "Dio_EnumGetterPin\nFunction" -> "xSemaphoreTake\nFunction": **DIO_SemArryOfPins, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 * "Dio_EnumGetterPin\nFunction" -> "xSemaphoreTake\nFunction": **DIO_SemArryOfActivition, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 * "Dio_EnumGetterPin\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Dio_EnumGetterPin\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * @enduml
 *
 *
 *
 *
 */
/* if you want to get some thing from buffer call Dio_VidRunnable then Dio_EnuSetterPin will
 return the updated data
 */
Error_State Dio_EnumGetterPin(Dio_ChannelType Copy_ChannelId,
		Dio_LevelType *Copy_Level) {
	if (Copy_Level == NULL_PTR) {
		return E_NOT_OK;
	}

	if (Copy_ChannelId >= Dio_A0 && Copy_ChannelId <= Dio_C14) {
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreTake(DIO_SemArryOfPins,portMAX_DELAY);
	xSemaphoreTake(DIO_SemArryOfActivition,portMAX_DELAY);
#endif		
		*Copy_Level = ArryOfPins[Copy_ChannelId];
		ArryOfActivition[Copy_ChannelId][0] = STD_ON;
		ArryOfActivition[Copy_ChannelId][1] = DIO_IN;
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreGive(DIO_SemArryOfPins);
	xSemaphoreGive(DIO_SemArryOfActivition);
#endif
		return E_OK;
	}
	return E_INVALID_PARAMETER;
}





/**
 * \section Service_Name
 * Dio_VidRunnable
 *
 * \section Description
 * To execute the desired configuration within runtime "refresh pins".
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
 * #yellow:Take binary semaphores DIO_SemArryOfPins and DIO_SemArryOfActivition with time portMAX_DELAY **__(FreeRTOS Cfg on)__**; 
 * :Create variable i and initialize it with Dio_A0;  
 * while(**i < = Dio_A12**)
 * 
 * if (pin[i] is **__Active__**?) then (**yes**)
 * if (pin[i] is **__Out__**?) then (**yes**)	 
 * #green:Write this pin to **__Level of its buffer__**;
 * else (**no**)
 * #green:Read this pin in **__Its buffer__**; 
 * endif 
 * endif 
 *
 * :i++;
 * endwhile	 
 *
 * if (pin[A_15] is **__Active__**?) then (**yes**)
 * if (pin[A_15] is **__Out__**?) then (**yes**)	 
 * #green:Write this pin to **__Level of its buffer__**;
 * else (**no**)
 * #green:Read this pin in **__Its buffer__**; 
 * endif 
 * endif 
 *
 * :Put in variable i value Dio_B0;  
 * while(**i < = Dio_B15**)
 * 
 * if (pin[i] is **__Active__**?) then (**yes**)
 * if (pin[i] is **__Out__**?) then (**yes**)	 
 * #green:Write this pin to **__Level of its buffer__**;
 * else (**no**)
 * #green:Read this pin in **__Its buffer__**; 
 * endif 
 * endif 
 *
 * :i++;
 * endwhile	 
 *
 * :Put in variable i value Dio_C13;  
 * while(**i < = Dio_C14**)
 * 
 * if (pin[i] is **__Active__**?) then (**yes**)
 * if (pin[i] is **__Out__**?) then (**yes**)	 
 * #green:Write this pin to **__Level of its buffer__**;
 * else (**no**)
 * #green:Read this pin in **__Its buffer__**; 
 * endif 
 * endif 
 *
 * :i++;
 * endwhile	 
 *
 * #yellow:Give binary semaphores DIO_SemArryOfPins and DIO_SemArryOfActivition **__(FreeRTOS Cfg on)__**;  
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
 * "Dio_VidRunnable\nFunction" -> "xSemaphoreTake\nFunction": **DIO_SemArryOfPins, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 * "Dio_VidRunnable\nFunction" -> "xSemaphoreTake\nFunction": **DIO_SemArryOfActivition, portMAX_DELAY**
 * note right #green: Take dedicated semaphore. 
 *
 *
 * == Pin of A is Activated and Out ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_WriteChannel\nFunction": **GPIOA, i, ArryOfPins[i]**
 * note right #blue: Write level in the buffer to dedicated pin. 
 *
 * == Pin of A is Activated and Input ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_ReadChannel\nFunction": **GPIOA,i**
 * note right #blue: Read level from the buffer of dedicated pin. 
 *
 *
 * == Pin of A15 is Activated and Out ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_WriteChannel\nFunction": **GPIOA, Dio_A15, ArryOfPins[Dio_A15]**
 * note right #blue: Write level in the buffer to dedicated pin. 
 *
 * == Pin of A15 is Activated and Input ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_ReadChannel\nFunction": **GPIOA, Dio_A15**
 * note right #blue: Read level from the buffer of dedicated pin. 
 *
 * == Pin of B is Activated and Out ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_WriteChannel\nFunction": **GPIOB, (i & ~(0x10)) , ArryOfPins[i]**
 * note right #blue: Write level in the buffer to dedicated pin. 
 *
 * == Pin of B is Activated and Input ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_ReadChannel\nFunction": **GPIOB, (i & ~(0x10))**
 * note right #blue: Read level from the buffer of dedicated pin. 
 *
 *
 * == Pin of C is Activated and Out ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_WriteChannel\nFunction": **GPIOC, (i & ~(0x20)), ArryOfPins[i]**
 * note right #blue: Write level in the buffer to dedicated pin. 
 *
 * == Pin of C is Activated and Input ==
 *
 * "Dio_VidRunnable\nFunction" -> "Dio_ReadChannel\nFunction": **GPIOC, (i & ~(0x20))**
 * note right #blue: Read level from the buffer of dedicated pin. 
 *
 * == FreeRTOS Cfg on ==
 *
 * "Dio_VidRunnable\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfPins**
 * note right #green: Gives dedicated semaphore. 
 *
 * "Dio_VidRunnable\nFunction" -> "xSemaphoreGive\nFunction": **DIO_SemArryOfActivition**
 * note right #green: Gives dedicated semaphore. 
 *
 * @enduml
 *
 *
 *
 *
 */
/* to execute the desired configuration within runtime "refresh pins" */
void Dio_VidRunnable(void) {
	/* GPIOA */
	uint8 i = Dio_A0;
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreTake(DIO_SemArryOfPins,portMAX_DELAY);
	xSemaphoreTake(DIO_SemArryOfActivition,portMAX_DELAY);
#endif
	while (i <= Dio_A12) {
		if (ArryOfActivition[i][0] != STD_OFF) {
			if (ArryOfActivition[i][1] == DIO_OUT) {
				Dio_WriteChannel(GPIOA, i, ArryOfPins[i]);
			} else {
				ArryOfPins[i] = Dio_ReadChannel(GPIOA,i);
			}
		}
		i++;
	}

	if (ArryOfActivition[Dio_A15][0] != STD_OFF) {
		if (ArryOfActivition[Dio_A15][1] == DIO_OUT) {
			Dio_WriteChannel(GPIOA, Dio_A15, ArryOfPins[Dio_A15]);
		} else {
			ArryOfPins[Dio_A15] = Dio_ReadChannel(GPIOA, Dio_A15);
		}
	}

	/* GPIOB */
	i = Dio_B0;
	while (i <= Dio_B15) {
		if (ArryOfActivition[i][0] != STD_OFF) {
			if (ArryOfActivition[i][1] == DIO_OUT) {
				Dio_WriteChannel(GPIOB, (i & ~(0x10)) , ArryOfPins[i]);
			} else {
				ArryOfPins[i] = Dio_ReadChannel(GPIOB, (i & ~(0x10)));
			}
		}
		i++;
	}

	i = Dio_C13;
	while (i <= Dio_C14) {
		if (ArryOfActivition[i][0] != STD_OFF) {
			if (ArryOfActivition[i][1] == DIO_OUT) {
				Dio_WriteChannel(GPIOC, (i & ~(0x20)), ArryOfPins[i]);
			} else {
				ArryOfPins[i] = Dio_ReadChannel(GPIOC, (i & ~(0x20)));
			}
		}
		i++;
	}
#if DIO_DESIGN == DIO_FREERTOS
	xSemaphoreGive(DIO_SemArryOfPins);
	xSemaphoreGive(DIO_SemArryOfActivition);
#endif
}


/**
 * \section Service_Name
 * Dio_ReadChannel
 *
 * \section Description
 * To Read dedicated channel.
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
 * \param[in] volatile GPIOX_REG *GPIOX, Dio_ChannelType ChannelId
 * \param[inout] None
 * \param[out] None
 * \return Dio_LevelType
 * 
 *
 * \section Rational
 * To Read dedicated channel.
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
 * :Read dedicated port and store it in Local_IDR variable;
 * :Read dedicated channel from Local_IDR variable and store it in Local_IDR variable;
 * #green:Return Local_IDR variable; 
 * end
 *
 * @enduml
 *
 *
 *
 *
 *
 *
 */
/*
 Input  : Dio_ChannelType
 output : Dio_LevelType
 description :
 */
Dio_LevelType Dio_ReadChannel(volatile GPIOX_REG *GPIOX, Dio_ChannelType ChannelId) {
	uint32 Local_IDR = GPIOX->IDR.Reg;
	Local_IDR = ((Local_IDR >> (ChannelId)) & STD_HIGH);
	return Local_IDR;
}



/**
 * \section Service_Name
 * Dio_WriteChannel
 *
 * \section Description
 * To Write dedicated channel.
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
 * \param[in] volatile GPIOX_REG *GPIOX, Dio_ChannelType ChannelId, Dio_LevelType Level
 * \param[inout] None
 * \param[out] None
 * \return None
 * 
 *
 * \section Rational
 * To Read dedicated channel.
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
 * :Set positon of  **__ChannelId bit__** to **__1__** in **__uint32__** Local_Pin variable ; 
 * if (**Level == STD_HIGH?**) then (**yes**)
 * #green:Put Local_Pin variable in dedicated **__Set__** Register **(Set dedicated channel)**;
 * elseif (**Level == STD_LOW?**) then (**yes**) 
 * #green:Put Local_Pin variable in dedicated **__Reset__** Register **(Reset dedicated channel)**;
 * else
 * #red:Det_ReportError(infinite loop);
 * endif
 * end
 *
 * @enduml
 *
 *
 * \section Sequence_Diagram
 *
 * @startuml
 *
 * == Out of range condition ==
 *
 * "Dio_WriteChannel\nFunction" -[#red]> "Det_ReportError\nFunction": **MODULE_ID, Dio_WriteChannel_ID, DIO_E_PARAM_INVALID_CHANNEL_ID**
 * note right #red: Stuck in infinite loop.   
 * @enduml
 *
 *
 *
 *
 */
/*
 Input  : Dio_ChannelType
 output : Dio_LevelType
 description :
 */
void Dio_WriteChannel(volatile GPIOX_REG *GPIOX, Dio_ChannelType ChannelId,
		Dio_LevelType Level) {
	uint32 Local_Pin = (1 << ChannelId);

	if (Level == STD_HIGH) {
		GPIOX->BSRR.Reg = Local_Pin;
	} else if (Level == STD_LOW) {
		GPIOX->BRR.Reg = Local_Pin;
	}
	else {
		Det_ReportError(MODULE_ID, Dio_WriteChannel_ID, DIO_E_PARAM_INVALID_CHANNEL_ID);
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
/*
 Input  : Std_VersionInfoType
 output : None
 description :
 */
void Dio_GetVersionInfo(Std_VersionInfoType *versioninfo) {
	versioninfo->moduleID = MODULE_ID;
	versioninfo->sw_major_version = DIO_MAJOR_VERSION;
	versioninfo->sw_minor_version = DIO_MINOR_VERSION;
	versioninfo->sw_patch_version = DIO_PATCH_VERSION;
	versioninfo->vendorID = VENDOR_ID;
}
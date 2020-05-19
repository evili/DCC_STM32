/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "dcc.h"
#include "string.h"
#include "printf-stdarg.h"
#include "FreeRTOS_CLI.h"
#include "cli.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#if MOTOR_SHIELD_TYPE == MOTOR_SHIELD_TYPE_IHM041
#define SETUP_TIMER(track) \
		HAL_TIM_PWM_Start(&DCC_TIMER_##track , DCC_TIMER_ ##track## _CHANNEL_L);  /* PWM on Channel L */ \
		HAL_TIM_PWM_Start(&DCC_TIMER_ ##track , DCC_TIMER_ ##track## _CHANNEL_K); /* PWM on Channel K */ \
	    DCC_TIMER_ ##track## _INSTANCE->CR1 &= ~TIM_CR1_CEN; /* Stop Counter */ \
	    DCC_TIMER_ ##track## _INSTANCE->ARR = DCC_ZERO_ARR; /* Preload values */ \
	    DCC_TIMER_ ##track## _CCR_K = DCC_ZERO_CCR; \
	    DCC_TIMER_ ##track## _CCR_L = DCC_ZERO_CCR; \
	    DCC_TIMER_ ##track## _INSTANCE->EGR &= TIM_EGR_UG; /* Trigger update (preload loaded) */ \
	    DCC_TIMER_ ##track## _INSTANCE->EGR &= TIM_EGR_UG; \
	    DCC_TIMER_ ##track## _INSTANCE->SR = 0ul; /* Clear all Interrupts */ \
	    DCC_TIMER_ ##track## _INSTANCE->DIER = TIM_DIER_CC1IE; /* Enable conmutation Interrupt ONLY */ \
	    DCC_TIMER_ ##track## _INSTANCE->CR1 |= TIM_CR1_CEN; /* Enable timer */
#elif MOTOR_SHIELD_TYPE == MOTOR_SHIELD_TYPE_ARDUINO_V3
#define SETUP_TIMER(track) \
		HAL_TIM_PWM_Start(&DCC_TIMER_ ##track , DCC_TIMER_ ##track## _CHANNEL_K); /* PWM on Channel K */ \
	    DCC_TIMER_ ##track## _INSTANCE->CR1 &= ~TIM_CR1_CEN; /* Stop Counter */ \
	    DCC_TIMER_ ##track## _INSTANCE->ARR = DCC_ZERO_ARR; /* Preload values */ \
	    DCC_TIMER_ ##track## _CCR_K = DCC_ZERO_CCR; \
	    DCC_TIMER_ ##track## _INSTANCE->EGR &= TIM_EGR_UG; /* Trigger update (preload loaded) */ \
	    DCC_TIMER_ ##track## _INSTANCE->EGR &= TIM_EGR_UG; \
	    DCC_TIMER_ ##track## _INSTANCE->SR = 0ul; /* Clear all Interrupts */ \
	    DCC_TIMER_ ##track## _INSTANCE->DIER = TIM_DIER_CC1IE; /* Enable conmutation Interrupt ONLY */ \
	    DCC_TIMER_ ##track## _INSTANCE->CR1 |= TIM_CR1_CEN; /* Enable timer */
#endif
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
volatile uint32_t nfirst = 1050;
volatile uint8_t button_debounce = 1;
volatile uint8_t dcc_task_started = 0;
DCC_Packet_Pump main_pump;
DCC_Packet_Pump prog_pump;

//volatile uint32_t tim1_last_cnt;
//volatile uint32_t tim1_last_arr;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for dccTask */
osThreadId_t dccTaskHandle;
const osThreadAttr_t dccTask_attributes = {
  .name = "dccTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for commandTask */
osThreadId_t commandTaskHandle;
const osThreadAttr_t commandTask_attributes = {
  .name = "commandTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
/* Definitions for dccMainPacketQueue */
osMessageQueueId_t dccMainPacketQueueHandle;
const osMessageQueueAttr_t dccMainPacketQueue_attributes = {
  .name = "dccMainPacketQueue"
};
/* Definitions for commandQueue */
osMessageQueueId_t commandQueueHandle;
const osMessageQueueAttr_t commandQueue_attributes = {
  .name = "commandQueue"
};
/* Definitions for dccProgPacketQueue */
osMessageQueueId_t dccProgPacketQueueHandle;
const osMessageQueueAttr_t dccProgPacketQueue_attributes = {
  .name = "dccProgPacketQueue"
};
/* Definitions for dccDiscardQueue */
osMessageQueueId_t dccDiscardQueueHandle;
const osMessageQueueAttr_t dccDiscardQueue_attributes = {
  .name = "dccDiscardQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void vEnableUART(UART_HandleTypeDef *huart);
void vUpdateADCValue();
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartDccTask(void *argument);
void StartCommandTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {

}

__weak unsigned long getRunTimeCounterValue(void) {
	return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of dccMainPacketQueue */
  dccMainPacketQueueHandle = osMessageQueueNew (25, sizeof(DCC_Packet *), &dccMainPacketQueue_attributes);

  /* creation of commandQueue */
  commandQueueHandle = osMessageQueueNew (72, sizeof(char *), &commandQueue_attributes);

  /* creation of dccProgPacketQueue */
  dccProgPacketQueueHandle = osMessageQueueNew (3, sizeof(DCC_Packet *), &dccProgPacketQueue_attributes);

  /* creation of dccDiscardQueue */
  dccDiscardQueueHandle = osMessageQueueNew (3, sizeof(DCC_Packet *), &dccDiscardQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of dccTask */
  dccTaskHandle = osThreadNew(StartDccTask, NULL, &dccTask_attributes);

  /* creation of commandTask */
  commandTaskHandle = osThreadNew(StartCommandTask, NULL, &commandTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	// char  pcWriteBuffer[128];
	// DCC_Packet *current;
	// DCC_Packet Idle = DCC_PACKET_IDLE;
	// DCC_Packet Loco_3 = {1, -1, 0x03, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
	// DCC_Packet_set_speed(&Loco_3, 55, 1);
	// osMessageQueuePut(dccPAcketQueueHandle, &Idle, 0U, osWaitForever);
	// osMessageQueuePut(dccPAcketQueueHandle, &Loco_3, 0U, osWaitForever);
	//printf("Starting Default Task.\n");

	/* Infinite loop */
	for (;;) {
		osDelay(1000);
		HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
		//		vTaskList(pcWriteBuffer);
		//		printf("\n=============== TASK LIST ==============="
		//				"\nName ========== Stat == Prio == Stk = N ="
		//				"\n%s"
		//				"\n=========================================\n",
		//				pcWriteBuffer);
		// Get the last timer counter and print.
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartDccTask */
/**
 * @brief Function implementing the dccTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDccTask */
void StartDccTask(void *argument)
{
  /* USER CODE BEGIN StartDccTask */
	osStatus status;
	uint32_t flags;
	DCC_Packet *discardPacket;
	DCC_Packet_Pump_init(&main_pump, dccMainPacketQueueHandle, dccDiscardQueueHandle);
	DCC_Packet_Pump_init(&prog_pump, dccProgPacketQueueHandle, dccDiscardQueueHandle);

	// Disable timer on debug
	__HAL_DBGMCU_FREEZE_TIM2();
	__HAL_DBGMCU_FREEZE_TIM3();
	__HAL_DBGMCU_FREEZE_TIM6();
	__HAL_DBGMCU_FREEZE_TIM7();

	// Setup Timers
	SETUP_TIMER(MAIN);
	SETUP_TIMER(PROG);
	HAL_StatusTypeDef hal_stat = HAL_TIMEOUT;
	while(hal_stat != HAL_OK){
		osDelay(250);
		hal_stat = HAL_ADC_Start_DMA(&hadc1,(uint32_t *) ADC_Data, ADC_DATA_LEN*ADC_DATA_ROWS);
	}
	osDelay(200);
	/* Infinite loop */
	for (;;) {
		status = osMessageQueueGet(dccDiscardQueueHandle, &discardPacket, 0L, 0L);
		if(status == osOK)
			vPortFree(discardPacket);
		flags = osThreadFlagsWait(DCC_FLAGS, osFlagsWaitAny, 250L);
		switch(flags) {
		case DCC_FLAG_ADC_DATA:
			vUpdateADCValue();
			break;
		case DCC_FLAG_ADC_ERROR:
			HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
			do {
				osDelay(250);
				hal_stat = HAL_ADC_Stop_DMA(&hadc1);
			} while(hal_stat != HAL_OK);
			memset((void *) ADC_Data, 0, sizeof(ADC_Record) * ADC_DATA_ROWS);
			do {
				osDelay(250);
				hal_stat = HAL_ADC_Start_DMA(&hadc1,(uint32_t *) ADC_Data, ADC_DATA_LEN*ADC_DATA_ROWS);
			} while(hal_stat != HAL_OK);
			HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_RESET);
			break;
		default:
			break;
		}
		HAL_GPIO_TogglePin(LED_Green_GPIO_Port, LED_Green_Pin);
	}
  /* USER CODE END StartDccTask */
}

/* USER CODE BEGIN Header_StartCommandTask */
/**
 * @brief Function implementing the commandTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommandTask */
void StartCommandTask(void *argument)
{
  /* USER CODE BEGIN StartCommandTask */
	static unsigned char cInputString[ configCOMMAND_INT_MAX_OUTPUT_SIZE ], cOutputString[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
	BaseType_t xMoreDataToFollow;
	uint32_t flags;
	vRegisterCLICommands();
	vEnableUART(&huart3);
	// Save DCC++ Station Name, Motor and Version for Later (see command <s>)
	snprintf(DCCPP_STATION, DCCPP_STATION_MAX_LEN, "<iDCC++ BASE STATION FOR STM32F7 %s %s / %s>",
			MOTOR_SHIELD_NAME, __TIME__, __DATE__);
	snprintf((char *) cOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE,
			"%s\r\n", DCCPP_STATION);
	HAL_UART_Transmit_DMA(&huart3, cOutputString,
			strnlen((char *)cOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE));

	dcc_task_started = 1u;
	osDelay(200);
	for(;;) {
		/* Pass the string to FreeRTOS+CLI. */
		flags = osThreadFlagsWait(COMMAND_FLAGS, osFlagsWaitAny, 1000);
		if(flags & osFlagsError) {
			if(flags == osFlagsErrorTimeout) {
				continue;
			}
			else {
				HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
				//TODO: Error from CLI
				osDelay(100);
				continue;
			}
		}
		switch(flags) {
		case COMMAND_FLAG_TRANSMIT_OK:
			HAL_GPIO_TogglePin(LED_Blue_GPIO_Port, LED_Blue_Pin);
			memset(cInputString, 0, configCOMMAND_INT_MAX_OUTPUT_SIZE);
			HAL_UART_Receive_DMA(&huart3, cInputString, configCOMMAND_INT_MAX_OUTPUT_SIZE);
			break;
		case COMMAND_FLAG_RECEIVE_OK:
			//HAL_GPIO_TogglePin(LED_Red_GPIO_Port, LED_Red_Pin);
			xMoreDataToFollow = pdTRUE;
			while(xMoreDataToFollow) {
				// Replace the final '>' by an ' ' as we do not care...
				//for(int i=0; (i<configCOMMAND_INT_MAX_OUTPUT_SIZE) && (cInputString[i] != '\0'); i++){
				//	cInputString[i] = (cInputString[i] == 0x3E) ? 0x0D :  cInputString[i];
				//}
				xMoreDataToFollow = FreeRTOS_CLIProcessCommand((char *) cInputString, (char *) cOutputString,
						configCOMMAND_INT_MAX_OUTPUT_SIZE );
				size_t out_len = strnlen((char *) cOutputString,
						configCOMMAND_INT_MAX_OUTPUT_SIZE);
				if(out_len > 0)
				{
					HAL_UART_Transmit_DMA(&huart3, cOutputString, out_len);
					// Wait for transmission complete
					while(!(huart3.Instance->ISR &= USART_ISR_TC)){
						osDelay(10);
					}
				}
				else {
					// Tell MySelf transmit is OK
					osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_TRANSMIT_OK);
				}
			}
			break;
		case COMMAND_FLAG_ERROR:
			osThreadExit();
			break;
		}
	}
  /* USER CODE END StartCommandTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void vUpdateADCValue()
{
	ADC_Value = (ADC_Record) {0u, 0u, 0u};
	for(int i=0; i<ADC_DATA_ROWS; i++)
	{
		ADC_Value.temp += ADC_Data[i].temp;
		ADC_Value.senseA += ADC_Data[i].senseB;
		ADC_Value.senseB += ADC_Data[i].senseB;
	}
	ADC_Value.temp /= ADC_DATA_ROWS;
	ADC_Value.senseA /= ADC_DATA_ROWS;
	ADC_Value.senseB /= ADC_DATA_ROWS;
}

void vEnableUART(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3) {
		  int32_t state = osKernelLock();
		  // Disable USART.
		  huart->Instance->CR1 &= ~USART_CR1_UE;
		  // huart->Instance->CR1 &= ~USART_CR1_TE;
		  // huart->Instance->CR1 &= ~USART_CR1_RE;
		  // Set end-of-line detection
		  huart->Instance->CR2 |= (USART_CR2_ADD & (COMMAND_END_OF_LINE << USART_CR2_ADD_Pos)) | USART_CR2_ADDM7;
		  // Clear All IF Flags
		  huart->Instance->ICR |= USART_ICR_CLEAR_ALL;
		  // Enable CMF Interrupt
		  huart->Instance->CR1 |= USART_CR1_CMIE;
		  // Reenable USART
		  huart->Instance->CR1 |= USART_CR1_UE;
		  osKernelRestoreLock(state);
	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {

	if (htim->Instance == DCC_TIMER_MAIN_INSTANCE) {
		if (htim->Channel == DCC_TIMER_MAIN_ACTIVE_CHANNEL_K) {
			if (DCC_Packet_Pump_next(&main_pump) == DCC_ZERO) {
				DCC_TIMER_MAIN_INSTANCE->ARR  = DCC_ZERO_ARR;
				DCC_TIMER_MAIN_CCR_K = DCC_ZERO_CCR;
#ifdef DCC_TIMER_MAIN_CCR_L
				DCC_TIMER_MAIN_CCR_L = DCC_ZERO_CCR;
#endif
			} else {
				DCC_TIMER_MAIN_INSTANCE->ARR  = DCC_ONE_ARR;
				DCC_TIMER_MAIN_CCR_K = DCC_ONE_CCR;

#ifdef DCC_TIMER_MAIN_CCR_L
				DCC_TIMER_MAIN_CCR_L = DCC_ONE_CCR;
#endif
			}
		}
	}
	else if (htim->Instance == DCC_TIMER_PROG_INSTANCE) {
		if (htim->Channel == DCC_TIMER_PROG_ACTIVE_CHANNEL_K) {
			if (DCC_Packet_Pump_next(&prog_pump) == DCC_ZERO) {
				DCC_TIMER_PROG_INSTANCE->ARR  = DCC_ZERO_ARR;
				DCC_TIMER_PROG_CCR_K = DCC_ZERO_CCR;
#ifdef DCC_TIMER_PROG_CCR_L
				DCC_TIMER_PROG_CCR_L = DCC_ZERO_CCR;
#endif
			} else {
				DCC_TIMER_PROG_INSTANCE->ARR  = DCC_ONE_ARR;
				DCC_TIMER_PROG_CCR_K = DCC_ONE_CCR;
#ifdef DCC_TIMER_MAIN_CCR_L
				DCC_TIMER_PROG_CCR_L = DCC_ONE_CCR;
#endif
			}
		}
	}
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART3) {
		osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_TRANSMIT_OK);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART3) {
		osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_RECEIVE_OK);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART3) {
		osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_ERROR);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == USER_Btn_Pin)
	{
		if(button_debounce == 1) {
			HAL_TIM_Base_Start_IT(&htim6);
			HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, GPIO_PIN_SET);
			button_debounce = 0;
		}
		else {
			__NOP();
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance == ADC1)
	{
		osThreadFlagsSet(dccTaskHandle, DCC_FLAG_ADC_DATA);
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance == ADC1)
	{
		osThreadFlagsSet(dccTaskHandle, DCC_FLAG_ADC_ERROR);
	}
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

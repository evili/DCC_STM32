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
#include "tim.h"
#include "usart.h"
#include "dcc.h"
// #include "printf-stdarg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

DCC_Packet_Pump *pump;

//volatile uint32_t tim1_last_cnt;
//volatile uint32_t tim1_last_arr;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
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
  .stack_size = 128 * 4
};
/* Definitions for dccPacketQueue */
osMessageQueueId_t dccPacketQueueHandle;
const osMessageQueueAttr_t dccPacketQueue_attributes = {
  .name = "dccPacketQueue"
};
/* Definitions for commandQueue */
osMessageQueueId_t commandQueueHandle;
const osMessageQueueAttr_t commandQueue_attributes = {
  .name = "commandQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

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
  /* creation of dccPacketQueue */
  dccPacketQueueHandle = osMessageQueueNew (20, sizeof(DCC_Packet *), &dccPacketQueue_attributes);

  /* creation of commandQueue */
  commandQueueHandle = osMessageQueueNew (72, sizeof(char *), &commandQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	// Not Implemented Yet on FreeRTOS version of CMSIS v2
	/*
	 dccPacketPoolHandle = osMemoryPoolNew(DCC_QUEUE_LEN, sizeof(DCC_Packet), &dccPacketPool_attributes);
	 dccTaskArgument_t dccArgument = {
	 .queue = dccPacketQueueHandle,
	 .pool = dccPacketPoolHandle
	 };
	 */

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of dccTask */
  dccTaskHandle = osThreadNew(StartDccTask, (void*) &dccPacketQueueHandle, &dccTask_attributes);

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
	// unsigned int bit;
	osMessageQId dccQueue = *((osMessageQueueId_t*) argument);
	if (NULL == dccQueue) {
		// printf("\nBad Parameter for Queue: %u\n", (uint32_t) dccQueue);
		osDelay(1000);
		osThreadTerminate(osThreadGetId());
	}

	// printf("\nAllocating DCC Pump: %s\n", "OK");
	pump = pvPortMalloc(sizeof(DCC_Packet_Pump));
	if (NULL == pump) {
		// printf("\nDCC Pump allocation: %s\n", "FAILED");
		osDelay(1000);
		osThreadTerminate(osThreadGetId());
	}
	// printf("\nInitializing DCC Pump. %s\n", "OK");

	DCC_Packet_Pump_init(pump, dccQueue);

//	printf("\nIDLE PACKET: {%u, %u, {%u}, %u : %d}\n", pump->packet->data_len,
//			pump->packet->address, pump->packet->data[0], pump->packet->crc,
//			pump->packet->count);
	// Freeze TIM1 on debug
	// printf("\nPreparing TIMER%d for DCC.\n", 1);
	// Disable timer on debug
	__HAL_DBGMCU_FREEZE_TIM1();
	__HAL_DBGMCU_FREEZE_TIM7();
	// Activate channel 2
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	// Activate channel 1
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	// Stop Counter
	htim1.Instance->CR1 &= ~TIM_CR1_CEN;
	// Preload good values
	// htim1.Instance->PSC = 1000;
	// Preload values
	htim1.Instance->ARR = DCC_ZERO_ARR;
	htim1.Instance->CCR1 = DCC_ZERO_CCR;
	htim1.Instance->CCR2 = DCC_ZERO_CCR;
	// Trigger update (preload loaded)
	htim1.Instance->EGR &= TIM_EGR_UG;
	htim1.Instance->EGR &= TIM_EGR_UG;
	// Clear all Interrupts
	htim1.Instance->SR = 0ul;
	// Enable conmutation Interrupt ONLY
	htim1.Instance->DIER = TIM_DIER_CC1IE;
	// Enable timer
	htim1.Instance->CR1 |= TIM_CR1_CEN;
	// printf("Entering loop for DCC Pump. %s\n", "OK");
	/* Infinite loop */
	for (;;) {
		//		if ((DCC_PACKET_PREAMBLE == pump->status) && (0 == pump->bit)) {
		//			printf("\n\n%s\n", "PACKET:");
		//		}
		//		//printf("STATUS: %u, NBIT: %u, NDATA: %u, ", pump->status, pump->bit, pump->data_count);
		//		bit = (DCC_ONE_BIT_FREQ == DCC_Packet_Pump_next(pump));
		//		printf("%u", bit);
		//printf("%u", (DCC_ONE_BIT_FREQ == bit));
		HAL_GPIO_TogglePin(LED_Green_GPIO_Port, LED_Green_Pin);
		osDelay(125);
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

	for(;;);
  /* USER CODE END StartCommandTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM1) {
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			//tim1_last_cnt = htim->Instance->CNT;
			//tim1_last_arr = htim->Instance->ARR;
			if (DCC_Packet_Pump_next(pump) == DCC_ZERO) {
				htim->Instance->ARR = DCC_ZERO_ARR;
				htim->Instance->CCR1 = DCC_ZERO_CCR;
				htim->Instance->CCR2 = DCC_ZERO_CCR;
			} else {
				htim->Instance->ARR = DCC_ONE_ARR;
				htim->Instance->CCR1 = DCC_ONE_CCR;
				htim->Instance->CCR2 = DCC_ONE_CCR;
			}
		}
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART3) {
		//osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_IDLE);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART3) {
		//osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_IDLE);
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART3) {
		//osThreadFlagsSet(commandTaskHandle, COMMAND_FLAG_STOP);
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

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
#include "printf-stdarg.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "dcc.h"
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
  .priority = (osPriority_t) osPriorityHigh4,
  .stack_size = 128 * 4
};
/* Definitions for dccPAcketQueue */
osMessageQueueId_t dccPAcketQueueHandle;
const osMessageQueueAttr_t dccPAcketQueue_attributes = {
  .name = "dccPAcketQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartDccTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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
  /* creation of dccPAcketQueue */
  dccPAcketQueueHandle = osMessageQueueNew (DCC_QUEUE_LEN, sizeof(DCC_Packet *), &dccPAcketQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of dccTask */
  dccTaskHandle = osThreadNew(StartDccTask, (void*) &dccPAcketQueueHandle, &dccTask_attributes);

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
	// DCC_Packet *current;
	// DCC_Packet Idle = DCC_PACKET_IDLE;
	// DCC_Packet Loco_3 = {1, -1, 0x03, {0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
	// DCC_Packet_set_speed(&Loco_3, 55, 1);
	// osMessageQueuePut(dccPAcketQueueHandle, &Idle, 0U, osWaitForever);
	// osMessageQueuePut(dccPAcketQueueHandle, &Loco_3, 0U, osWaitForever);
	printf("Starting Default Task.\n");

	int j = 0;

	/* Infinite loop */
	for(;;)
	{
		HAL_GPIO_TogglePin(LED_Blue_GPIO_Port, LED_Blue_Pin);
		//printf("Main Task %d\n", j);
		j++;
		osDelay(1000);
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
	unsigned int bit;
	DCC_Packet_Pump *pump = pvPortMalloc(sizeof(DCC_Packet_Pump));
	if(NULL == pump)
		osThreadExit();
	osMessageQueueId_t *dccQueue = (osMessageQueueId_t *) argument;
	if (NULL == dccQueue) {
		vPortFree(pump);
		osThreadExit();
	}
	printf("Initializing DCC Pump. %s\n", "OK");
	DCC_Packet_Pump_init(pump, *dccQueue);
	printf("IDLE PACKET: {%u, %u, {%u}, %u : %d}\n",
			pump->packet->data_len,
			pump->packet->address,
			pump->packet->data[0],
			pump->packet->crc,
			pump->packet->count
			);
	printf("Entering loop for DCC Pump. %s\n", "OK");
	/* Infinite loop */
	for(;;)
	{
		if ((DCC_PACKET_PREAMBLE == pump->status) && (0 == pump->bit)) {
			printf("\n\n%s\n", "PACKET:");
		}
		printf("STATUS: %u, NBIT: %u, NDATA: %u, ", pump->status, pump->bit, pump->data_count);
		bit = (DCC_ONE_BIT_FREQ == DCC_Packet_Pump_next(pump));
		printf("BIT: %u\n", bit);
		//printf("%u", (DCC_ONE_BIT_FREQ == bit));

		HAL_GPIO_TogglePin(LED_Red_GPIO_Port, LED_Red_Pin);
		osDelay(500);
	}
  /* USER CODE END StartDccTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

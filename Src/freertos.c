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
#include "dcc.h"
#include "printf-stdarg.h"
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

// Not Implemented Yet on FreeRTOS version of CMSIS v2

/*
osMemoryPoolId_t dccPacketPoolHandle;
const osMemoryPoolAttr_t dccPacketPool_attributes = {
  .name = "dccPacketPool"
};
*/
osPoolId dccPacketPoolHandle;
osPoolDef (dccPacketPool, DCC_QUEUE_LEN, DCC_Packet);

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId dccTaskHandle;
osMessageQId dccPacketQueueHandle;
osMutexId dccFailMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartDccTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of dccFailMutex */
  osMutexDef(dccFailMutex);
  dccFailMutexHandle = osMutexCreate(osMutex(dccFailMutex));

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
  /* definition and creation of dccPacketQueue */
  osMessageQDef(dccPacketQueue, 20, DCC_Packet);
  dccPacketQueueHandle = osMessageCreate(osMessageQ(dccPacketQueue), NULL);

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
  dccPacketPoolHandle = osPoolCreate(osPool(dccPacketPool));
  dccTaskArgument_t dccArgument = {
		  .queue = dccPacketQueueHandle,
		  .pool = dccPacketPoolHandle
  };

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of dccTask */
  osThreadDef(dccTask, StartDccTask, osPriorityHigh, 0, 128);
  dccTaskHandle = osThreadCreate(osThread(dccTask), (void*) &dccArgument);

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
void StartDefaultTask(void const * argument)
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
void StartDccTask(void const * argument)
{
  /* USER CODE BEGIN StartDccTask */
	unsigned int bit;
	DCC_Packet_Pump *pump = pvPortMalloc(sizeof(DCC_Packet_Pump));
	if(NULL == pump)
		osThreadTerminate(osThreadGetId());
	dccTaskArgument_t *dccArgument = (dccTaskArgument_t *) argument;
	osMessageQId dccQueue = dccArgument->queue;
	osPoolId dccPool = dccArgument->pool;
	if ((NULL == dccQueue) || (NULL == dccPool)) {
		vPortFree(pump);
		osThreadTerminate(osThreadGetId());
	}
	printf("Initializing DCC Pump. %s\n", "OK");
	DCC_Packet_Pump_init(pump, dccQueue, dccPool);
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

		HAL_GPIO_TogglePin(LD_Green_GPIO_Port, LD_Green_Pin);
		osDelay(250);
	}
  /* USER CODE END StartDccTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

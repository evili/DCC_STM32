/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void assert_failed(uint8_t* file, uint32_t line);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DCC_ZERO_ARR 43200
#define DCC_ZERO_CCR 21600
#define DCC_ONE_ARR 25058
#define DCC_ONE_CCR 12529
#define DCC_QUEUE_LEN 20
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define USER_Btn_EXTI_IRQn EXTI15_10_IRQn
#define SENSE_B_Pin GPIO_PIN_0
#define SENSE_B_GPIO_Port GPIOC
#define SENSE_A_Pin GPIO_PIN_3
#define SENSE_A_GPIO_Port GPIOA
#define DCC_PROG_Pin GPIO_PIN_5
#define DCC_PROG_GPIO_Port GPIOA
#define DCC_MAIN_Pin GPIO_PIN_6
#define DCC_MAIN_GPIO_Port GPIOA
#define ENABLE_MAIN_Pin GPIO_PIN_7
#define ENABLE_MAIN_GPIO_Port GPIOA
#define LED_Green_Pin GPIO_PIN_0
#define LED_Green_GPIO_Port GPIOB
#define ENABLE_PROG_Pin GPIO_PIN_13
#define ENABLE_PROG_GPIO_Port GPIOE
#define LED_Red_Pin GPIO_PIN_14
#define LED_Red_GPIO_Port GPIOB
#define USB_Out_Pin GPIO_PIN_6
#define USB_Out_GPIO_Port GPIOG
#define USB_In_Pin GPIO_PIN_7
#define USB_In_GPIO_Port GPIOG
#define LED_Blue_Pin GPIO_PIN_7
#define LED_Blue_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define VERSION "1.2.1+-STM32F7"

#define MOTOR_SHIELD_TYPE_ARDUINO_V3 1
#define MOTOR_SHIELD_TYPE_IHM04A1    2
// Uncomment on of these
#define MOTOR_SHIELD_TYPE MOTOR_SHIELD_TYPE_ARDUINO_V3
// #define MOTOR_SHIELD_TYPE MOTOR_SHIELD_TYPE_IHM04A1

// Timers Definition
#if MOTOR_SHIELD_TYPE == MOTOR_SHIELD_TYPE_IHM04A1
// TIM1 == MAIN TIMER
#define MOTOR_SHIELD_NAME "IHM04A1 MOTOR SHIELD"
#define DCC_TIMER_MAIN htim1
#define DCC_TIMER_MAIN_INSTANCE TIM1
#define DCC_TIMER_MAIN_ACTIVE_CHANNEL_K HAL_TIM_ACTIVE_CHANNEL_1
#define DCC_TIMER_MAIN_ACTIVE_CHANNEL_L HAL_TIM_ACTIVE_CHANNEL_2
#define DCC_TIMER_MAIN_CHANNEL_K TIM_CHANNEL_1
#define DCC_TIMER_MAIN_CHANNEL_L TIM_CHANNEL_2
#define DCC_TIMER_MAIN_CCR_K DCC_TIMER_MAIN_INSTANCE->CCR1
#define DCC_TIMER_MAIN_CCR_L DCC_TIMER_MAIN_INSTANCE->CCR2
// TIM4 == PROG TIMER
#define DCC_TIMER_PROG htim1
#define DCC_TIMER_PROG_INSTANCE TIM4
#define DCC_TIMER_PROG_ACTIVE_CHANNEL_K HAL_TIM_ACTIVE_CHANNEL_3
#define DCC_TIMER_PROG_ACTIVE_CHANNEL_L HAL_TIM_ACTIVE_CHANNEL_4
#define DCC_TIMER_PROG_CHANNEL_K TIM_CHANNEL_3
#define DCC_TIMER_PROG_CHANNEL_L TIM_CHANNEL_4
#define DCC_TIMER_PROG_CCR_K DCC_TIMER_PROG_INSTANCE->CCR3
#define DCC_TIMER_PROG_CCR_L DCC_TIMER_PROG_INSTANCE->CCR4
#elif MOTOR_SHIELD_TYPE == MOTOR_SHIELD_TYPE_ARDUINO_V3
#define MOTOR_SHIELD_NAME "ARDUINO MOTOR SHIELD"
// TIM3 == MAIN TIMER
#define DCC_TIMER_MAIN htim3
#define DCC_TIMER_MAIN_INSTANCE TIM3
#define DCC_TIMER_MAIN_ACTIVE_CHANNEL_K HAL_TIM_ACTIVE_CHANNEL_1
//#define DCC_TIMER_MAIN_ACTIVE_CHANNEL_L
#define DCC_TIMER_MAIN_CHANNEL_K TIM_CHANNEL_1
//#define DCC_TIMER_MAIN_CHANNEL_L TIM_CHANNEL_2
#define DCC_TIMER_MAIN_CCR_K DCC_TIMER_MAIN_INSTANCE->CCR1
//#define DCC_TIMER_MAIN_CCR_L
// TIM2 == PROG TIMER
#define DCC_TIMER_PROG htim2
#define DCC_TIMER_PROG_INSTANCE TIM2
#define DCC_TIMER_PROG_ACTIVE_CHANNEL_K HAL_TIM_ACTIVE_CHANNEL_1
//#define DCC_TIMER_PROG_ACTIVE_CHANNEL_L
#define DCC_TIMER_PROG_CHANNEL_K TIM_CHANNEL_1
//#define DCC_TIMER_PROG_CHANNEL_L
#define DCC_TIMER_PROG_CCR_K DCC_TIMER_PROG_INSTANCE->CCR1
//#define DCC_TIMER_PROG_CCR_L
#endif // MOTOR_SHIELD_TYPE

#define USART_ICR_CLEAR_ALL ( USART_ICR_PECF    |\
                              USART_ICR_FECF    |\
                              USART_ICR_NCF     |\
                              USART_ICR_ORECF   |\
                              USART_ICR_IDLECF  |\
                              USART_ICR_TCCF    |\
                              USART_ICR_LBDCF   |\
                              USART_ICR_CTSCF   |\
                              USART_ICR_RTOCF   |\
                              USART_ICR_EOBCF   |\
                              USART_ICR_CMCF )

#define COMMAND_END_OF_LINE 0x0000000aul

#define COMMAND_FLAG_RECEIVE_OK  0x00000001ul
#define COMMAND_FLAG_TRANSMIT_OK 0x00000002ul
#define COMMAND_FLAG_ERROR       0x00000004ul
#define COMMAND_FLAGS       (COMMAND_FLAG_RECEIVE_OK | COMMAND_FLAG_TRANSMIT_OK | COMMAND_FLAG_ERROR)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

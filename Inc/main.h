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
#define LED_Green_Pin GPIO_PIN_0
#define LED_Green_GPIO_Port GPIOB
#define ENABLE_A_Pin GPIO_PIN_15
#define ENABLE_A_GPIO_Port GPIOF
#define DCC_K_Pin GPIO_PIN_9
#define DCC_K_GPIO_Port GPIOE
#define DCC_L_Pin GPIO_PIN_11
#define DCC_L_GPIO_Port GPIOE
#define LED_Red_Pin GPIO_PIN_14
#define LED_Red_GPIO_Port GPIOB
#define USB_Out_Pin GPIO_PIN_6
#define USB_Out_GPIO_Port GPIOG
#define USB_In_Pin GPIO_PIN_7
#define USB_In_GPIO_Port GPIOG
#define LED_Blue_Pin GPIO_PIN_7
#define LED_Blue_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "visEffect.h"

#include "ws2812b.h"
#include "nonvmem.h"
#include "irDecoder.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ledEffects.h"
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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
#define UART_BUFF_SIZE 50
 uint8_t UART_DmaBuffer[UART_BUFF_SIZE];
 uint8_t UART_CpltBuffer[UART_BUFF_SIZE];
 const uint8_t UART_GET_IR_SENTENCE[6] = {'G', 'E', 'T', '_', 'I','R'};
 const uint8_t UART_GET_IR_RESPONSE[] = {'\n', 'W','A','I','T','I','N','G', '_', 'F','O', 'R', '_', 'I', 'R', '\n'};
 const uint8_t UART_ROUTINES_READ_SENTENCE[13] =
 	 {'R', 'O', 'U', 'T', 'I', 'N', 'E', 'S', '_', 'R', 'E', 'A', 'D'};
 const uint8_t UART_ROUTINES_WRITE_SENTENCE[14] =
  	 {'R', 'O', 'U', 'T', 'I', 'N', 'E', 'S', '_', 'W', 'R', 'I', 'T', 'E'};
 const uint8_t UART_ROUTINES_WRITE_RESPONSE[] = {'\n', 'W','A','I','T','I','N','G'};
 const uint8_t UART_ROUTINES_SAVE_SENTENCE[11] =
   	 {'M', 'E', 'M', 'O', 'R', 'Y', '_', 'S', 'A', 'V', 'E'};
 const uint8_t UART_ROUTINES_PURGE_SAVED_SENTENCE[9] =
   	 {'P', 'U', 'R', 'G', 'E', '_', 'M', 'E', 'M'};
 const uint8_t UART_RESPONSE_OK[] = {'\n', 'o', 'k'};
 const uint8_t UART_RESPONSE_END[] = {'\n','o', 'k', '_', 'e', 'n','d'};

 const uint8_t UART_SENTENCE_TAIL_BYTES = 2;
 const uint8_t UART_SENTENCE_HEAD_BYTES = 3;
volatile uint16_t UART_BytesReceived = 0;

uint8_t GET_IR_FLAG = 0;
uint8_t ROUTINES_WRITE_FLAG = 0;
// Saving Variables
uint8_t SAVING_IR_POS = 0, SAVING_TIME_POS = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	irIRQ();
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size){

		if(huart->Instance == USART2)
		{
		//		HAL_UART_Transmit(&huart1, UART1_CpltBuffer, sizeof(UART1_CpltBuffer), 100);
			UART_BytesReceived = size;
			memcpy(UART_CpltBuffer, UART_DmaBuffer, UART_BUFF_SIZE);
			HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *) UART_DmaBuffer, UART_BUFF_SIZE);
			__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
		}

}
void SendRoutinesOverUart()
{
	HAL_UART_Transmit(&huart2, (uint8_t *)"\nIR:\n", 5, 100);
	for(int i = 0; i < MAX_IR_CODES; i++)
	{
		HAL_UART_Transmit(&huart2, (uint8_t *)"n:", 2, 100);

		for(int j = 0; j<BYTES_PER_IR_CODE; j++)
			HAL_UART_Transmit(&huart2, &IR_CODES_FLAGS[i][j], 1, 100);
		for(int k = 0; k<BYTES_PER_EFFECT; k++)
			HAL_UART_Transmit(&huart2, &IR_CODES_MACROS[i][k], 1, 100);

		HAL_UART_Transmit(&huart2, (uint8_t *)'\n', 1, 100);
	}
	HAL_UART_Transmit(&huart2, (uint8_t *)"\nEL:\n", 5, 100);

	for(int i = 0; i < MAX_ELE_INPUTS; i++)
	{
		HAL_UART_Transmit(&huart2, (uint8_t *)"n:", 2, 100);
		HAL_UART_Transmit(&huart2, (uint8_t *)i, 1, 100);

		for(int k = 0; k<BYTES_PER_EFFECT; k++)
			HAL_UART_Transmit(&huart2, &ELE_INPUT_MACROS[i][k], 1, 100);

		HAL_UART_Transmit(&huart2,(uint8_t *) '\n', 1, 100);
	}
	HAL_UART_Transmit(&huart2, (uint8_t *)"\nTM:\n", 5, 100);
	for(int i = 0; i < MAX_TIME_MACROS; i++)
	{
		HAL_UART_Transmit(&huart2, (uint8_t *)"n:", 2, 100);
		for(int j = 0; j<BYTES_PER_TIME_FLAG; j++)
			HAL_UART_Transmit(&huart2, &TIME_FLAGS[i][j], 1, 100);
		for(int k = 0; k<BYTES_PER_EFFECT; k++)
			HAL_UART_Transmit(&huart2, &TIME_MACROS[i][k], 1, 100);
		HAL_UART_Transmit(&huart2,(uint8_t *) '\n', 1, 100);
	}

}
void UartCheckReceived()
{
	if(UART_BytesReceived > 0)
	{
		if(ROUTINES_WRITE_FLAG == 1)
		{
			if(UART_BytesReceived >= 3 && UART_CpltBuffer[0] == 'E' && UART_CpltBuffer[1] == 'N' && UART_CpltBuffer[2] == 'D' )
			{
				HAL_UART_Transmit(&huart2, UART_RESPONSE_END, sizeof(UART_RESPONSE_END), 100);
				ROUTINES_WRITE_FLAG = 0;
				SAVING_IR_POS = 0;
				SAVING_TIME_POS = 0;

			}else if(UART_BytesReceived >= 14 && UART_CpltBuffer[0] == 'S' && UART_CpltBuffer[1] == 'V' && UART_CpltBuffer[2] == '+'){

				if(UART_BytesReceived >= 15 && UART_CpltBuffer[3] == 'I' && UART_CpltBuffer[4] == 'R')
				{
					if(SAVING_IR_POS < MAX_IR_CODES)
					{
						//IR ADDR
						IR_CODES_FLAGS[SAVING_IR_POS][0] = UART_CpltBuffer[5];
						//IR COMMAND
						IR_CODES_FLAGS[SAVING_IR_POS][1] = UART_CpltBuffer[6];
						//ACTIVE STRIPS
						IR_CODES_MACROS[SAVING_IR_POS][0] = UART_CpltBuffer[8];
						IR_CODES_MACROS[SAVING_IR_POS][1] = UART_CpltBuffer[9];
						//EFFECT
						IR_CODES_MACROS[SAVING_IR_POS][2] = UART_CpltBuffer[11];
						IR_CODES_MACROS[SAVING_IR_POS][3] = UART_CpltBuffer[12];
						IR_CODES_MACROS[SAVING_IR_POS][4] = UART_CpltBuffer[13];
						IR_CODES_MACROS[SAVING_IR_POS][5] = UART_CpltBuffer[14];

						SAVING_IR_POS++;
						HAL_UART_Transmit(&huart2, UART_RESPONSE_OK, sizeof(UART_RESPONSE_OK), 100);
					}
				} else if(UART_CpltBuffer[3] == 'E' && UART_CpltBuffer[4] == 'L')
				{
					if(UART_CpltBuffer[5] < MAX_ELE_INPUTS)
					{
						//ACTIVE STRIPS
						ELE_INPUT_MACROS[UART_CpltBuffer[5]][0] = UART_CpltBuffer[7];
						ELE_INPUT_MACROS[UART_CpltBuffer[5]][1] = UART_CpltBuffer[8];
						//EFFECT
						ELE_INPUT_MACROS[UART_CpltBuffer[5]][2] = UART_CpltBuffer[10];
						ELE_INPUT_MACROS[UART_CpltBuffer[5]][3] = UART_CpltBuffer[11];
						ELE_INPUT_MACROS[UART_CpltBuffer[5]][4] = UART_CpltBuffer[12];
						ELE_INPUT_MACROS[UART_CpltBuffer[5]][5] = UART_CpltBuffer[13];
						HAL_UART_Transmit(&huart2, UART_RESPONSE_OK, sizeof(UART_RESPONSE_OK), 100);
					}
				} else if(UART_BytesReceived >= 17 && UART_CpltBuffer[3] == 'T' && UART_CpltBuffer[4] == 'M')
				{
					if(SAVING_TIME_POS < MAX_TIME_MACROS)
					{
						//START TIME
						TIME_FLAGS[SAVING_TIME_POS][0] = UART_CpltBuffer[5];
						TIME_FLAGS[SAVING_TIME_POS][1] = UART_CpltBuffer[6];
						//FOR TIME (TWO 8BITS = 1 16BIT)
						TIME_FLAGS[SAVING_TIME_POS][2] = UART_CpltBuffer[7];
						TIME_FLAGS[SAVING_TIME_POS][3] = UART_CpltBuffer[8];
						//ACTIVE STRIPS
						TIME_MACROS[SAVING_TIME_POS][0] = UART_CpltBuffer[10];
						TIME_MACROS[SAVING_TIME_POS][1] = UART_CpltBuffer[11];
						//EFFECT
						TIME_MACROS[SAVING_TIME_POS][0] = UART_CpltBuffer[13];
						TIME_MACROS[SAVING_TIME_POS][1] = UART_CpltBuffer[13];
						TIME_MACROS[SAVING_TIME_POS][2] = UART_CpltBuffer[13];
						TIME_MACROS[SAVING_TIME_POS][3] = UART_CpltBuffer[13];

						SAVING_TIME_POS++;
						HAL_UART_Transmit(&huart2, UART_RESPONSE_OK, sizeof(UART_RESPONSE_OK), 100);
					}
				}

			}

		}else if(UART_BytesReceived == 4)
		{
			  if(UART_CpltBuffer[0] == 'A' && UART_CpltBuffer[1] == 'T' && UART_CpltBuffer[2] == '\r' && UART_CpltBuffer[3] == '\n')
			  {
				HAL_UART_Transmit(&huart2, UART_RESPONSE_OK, sizeof(UART_RESPONSE_OK), 100);
			  }
		}else if(UART_BytesReceived > 4 && UART_CpltBuffer[0] == 'A' && UART_CpltBuffer[1] == 'T' && UART_CpltBuffer[2] == '+')
		  	  {
			  	  if(UART_BytesReceived == UART_SENTENCE_HEAD_BYTES+UART_SENTENCE_TAIL_BYTES+(uint8_t)sizeof(UART_GET_IR_SENTENCE))
			  	  {
					  uint8_t UART_GET_IR_FLAG = 1;
					  for(int i = 0; i<sizeof(UART_GET_IR_SENTENCE); i++)
						  if(UART_CpltBuffer[i+UART_SENTENCE_HEAD_BYTES] !=  UART_GET_IR_SENTENCE[i])
							  UART_GET_IR_FLAG = 0;
					  GET_IR_FLAG = UART_GET_IR_FLAG;

					  HAL_UART_Transmit(&huart2, UART_GET_IR_RESPONSE, sizeof(UART_GET_IR_RESPONSE), 100);
			  	  }
			  	  if(UART_BytesReceived == UART_SENTENCE_HEAD_BYTES+UART_SENTENCE_TAIL_BYTES+(uint8_t)sizeof(UART_ROUTINES_READ_SENTENCE))
			  	  {
					  uint8_t UART_ROUTINES_READ_FLAG = 1;
					  for(int i = 0; i<sizeof(UART_ROUTINES_READ_SENTENCE); i++)
						  if(UART_CpltBuffer[i+UART_SENTENCE_HEAD_BYTES] !=  UART_ROUTINES_READ_SENTENCE[i])
							  UART_ROUTINES_READ_FLAG = 0;
					  if(UART_ROUTINES_READ_FLAG == 1)
						  SendRoutinesOverUart();
			  	  }
			  	  if(UART_BytesReceived == UART_SENTENCE_HEAD_BYTES+UART_SENTENCE_TAIL_BYTES+(uint8_t)sizeof(UART_ROUTINES_WRITE_SENTENCE))
			  	  {
					  uint8_t UART_ROUTINES_WRITE_FLAG = 1;
					  for(int i = 0; i<sizeof(UART_ROUTINES_WRITE_SENTENCE); i++)
						  if(UART_CpltBuffer[i+UART_SENTENCE_HEAD_BYTES] !=  UART_ROUTINES_WRITE_SENTENCE[i])
							  UART_ROUTINES_WRITE_FLAG = 0;
					  ROUTINES_WRITE_FLAG = UART_ROUTINES_WRITE_FLAG;
					  if(UART_ROUTINES_WRITE_FLAG == 1)
						  HAL_UART_Transmit(&huart2, UART_ROUTINES_WRITE_RESPONSE, sizeof(UART_ROUTINES_WRITE_RESPONSE), 100);
			  	  }
			  	  if(UART_BytesReceived == UART_SENTENCE_HEAD_BYTES+UART_SENTENCE_TAIL_BYTES+(uint8_t)sizeof(UART_ROUTINES_SAVE_SENTENCE))
			  	  {
			  		uint8_t UART_ROUTINES_SAVE_FLAG = 1;
			  		for(int i = 0; i<sizeof(UART_ROUTINES_SAVE_SENTENCE); i++)
			  			if(UART_CpltBuffer[i+UART_SENTENCE_HEAD_BYTES] !=  UART_ROUTINES_SAVE_SENTENCE[i])
			  				UART_ROUTINES_SAVE_FLAG = 0;
			  		if(UART_ROUTINES_SAVE_FLAG == 1)
			  			{
			  				saveMem();
			  				HAL_UART_Transmit(&huart2, UART_RESPONSE_OK, sizeof(UART_RESPONSE_OK), 100);
			  			}


			  	  }
			  	  if(UART_BytesReceived == UART_SENTENCE_HEAD_BYTES+UART_SENTENCE_TAIL_BYTES+(uint8_t)sizeof(UART_ROUTINES_PURGE_SAVED_SENTENCE))
			  	  {
			  		uint8_t UART_ROUTINES_PURGE_SAVED_FLAG = 1;
					for(int i = 0; i<sizeof(UART_ROUTINES_PURGE_SAVED_SENTENCE); i++)
						if(UART_CpltBuffer[i+UART_SENTENCE_HEAD_BYTES] !=  UART_ROUTINES_PURGE_SAVED_SENTENCE[i])
							UART_ROUTINES_PURGE_SAVED_FLAG = 0;
					if(UART_ROUTINES_PURGE_SAVED_FLAG == 1)
					{
						purgeSaved(2000);
						readMem();
						HAL_UART_Transmit(&huart2, UART_RESPONSE_OK, sizeof(UART_RESPONSE_OK), 100);
					}
			  	  }
		  	  }
		  UART_BytesReceived = 0;
	}
}
void irCallback(uint16_t addr, uint16_t comm)
{
	if(GET_IR_FLAG)
	{
		char irAsci[30] = {' '};
		sprintf(irAsci, "ADR: %d, COM: %d", (int)addr, (int)comm);
		HAL_UART_Transmit(&huart2, (uint8_t *)irAsci, strlen(irAsci), 100);
		GET_IR_FLAG = 0;
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    HAL_UART_Transmit(&huart1, UART1_rxBuffer, 4, 100);
//    HAL_UART_Receive_IT(&huart1, UART1_rxBuffer, 4);
//}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

//  irSetup(&htim3, &huart1);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *) UART_CpltBuffer, UART_BUFF_SIZE);
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
  irSetup(&htim3, &huart2);

//  *(__IO uint8_t *) (BKPSRAM_BASE + 1) = 'c';
//  uint8_t x = (*(__IO uint8_t *) (BKPSRAM_BASE + 1));
//  purgeSaved(2000);

//  IR_CODES_FLAGS[0][0] = 'J';
//  IR_CODES_FLAGS[0][1] = 'P';
//  saveMem();
  //readMem();
//  HAL_UART_Transmit(&huart1, &IR_CODES_FLAGS[0][0], sizeof(IR_CODES_FLAGS[0][0]), 100);
//  HAL_UART_Transmit(&huart1, &IR_CODES_FLAGS[0][1], sizeof(IR_CODES_FLAGS[0][1]), 100);
  //HAL_TIM_Base_Start_IT(&htim3);
//  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  for(uint8_t i = 0; i < WS2812_BUFFER_COUNT; i++)
	{
	  ws2812b.item[i].channel = i;
	  ws2812b.item[i].frameBufferPointer = &frameBufferArray[i][0];
	  ws2812b.item[i].frameBufferSize = WS2812B_NUMBER_OF_LEDS*3;
	}
  ws2812b_init();

  ACTIVE_EFFECT_TYPE = 1;
  ACTIVE_EFFECT_NB = 1;
  ELE_INPUT_MACROS[ACTIVE_EFFECT_NB][0] = 0xFF;
  ELE_INPUT_MACROS[ACTIVE_EFFECT_NB][1] = 0xFF;
  ELE_INPUT_MACROS[ACTIVE_EFFECT_NB][2] = 1;
  ELE_INPUT_MACROS[ACTIVE_EFFECT_NB][3] = 40;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  //irHandle(irCallback);
	  UartCheckReceived();
	  irHandle(irCallback);
	  effectsLoop();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 15;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

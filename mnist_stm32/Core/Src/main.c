/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
#include "main.h"
#include "crc.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"
#include "app_x-cube-ai.h"

#include <errno.h>
#include <sys/unistd.h>
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NN_INPUT_IMAGE_WIDTH				28
#define NN_INPUT_IMAGE_HEIGHT				28
#define LCD_INPUT_IMAGE_WIDTH				240
#define LCD_INPUT_IMAGE_HEIGHT				240

#define LCD_FRAME_BUFFER_LAYER0             (LCD_FRAME_BUFFER +0x130000)
#define LCD_FRAME_BUFFER_LAYER1 			LCD_FRAME_BUFFER
#define CONVERTED_FRAME_BUFFER              (LCD_FRAME_BUFFER +0x260000)

#define DRAW_IMG_X1							10
#define	DRAW_IMG_X2							230

#define DRAW_IMG_Y1							10
#define	DRAW_IMG_Y2							230

#define ERASE_BUTTON_X1						5
#define ERASE_BUTTON_Y1						285
#define ERASE_BUTTON_X2						ERASE_BUTTON_X1 +230
#define ERASE_BUTTON_Y2						ERASE_BUTTON_Y1 + 30

#define IMG_SCALED_X_SHIFT					200
#define IMG_SCALED_Y_SHIFT					250

#define  NUM_CLASSES						10
#define  PXL_SET							0.99

TS_StateTypeDef ui_state;

struct prediction_probability{
 ai_float prob;
 ai_u8  label;
};

typedef struct prediction_probability pred_probType;



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
void user_interface_init(void);
void user_interface_reset();
void touch_sensor_init();
void mnist_game_reset(ai_float** in_data, pred_probType * _1st_pred, pred_probType *_2nd_pred);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_CRC_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_X_CUBE_AI_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  BSP_PB_Init(BUTTON_KEY,0);
  touch_sensor_init();

  user_interface_init();
  user_interface_reset();

  ai_float nn_input_data[NN_INPUT_IMAGE_HEIGHT][NN_INPUT_IMAGE_WIDTH];
  ai_float nn_output_data[NUM_CLASSES];

  pred_probType _1st_pred, _2nd_pred;
  _1st_pred.prob = _2nd_pred.prob =0.0;

  char _1st_pred_str[10];
  //char _2nd_pred_str[10];

  while (1)
  {


	  BSP_TS_GetState(&ui_state);

	       if(ui_state.TouchDetected){

	        if((ui_state.X > DRAW_IMG_X1 && ui_state.X < DRAW_IMG_X2) && (ui_state.Y > DRAW_IMG_Y1 && ui_state.Y < DRAW_IMG_Y2))
	        {
	    	   BSP_LCD_FillCircle(ui_state.X,ui_state.Y,5);

	    	   int x  = ui_state.Y*((float)28/240);
	    	   int y  = ui_state.X*((float)28/240);

	    	   BSP_LCD_DrawPixel(y+1 + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x+1,255);
	    	   BSP_LCD_DrawPixel(y-1 + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x-1,255);
	    	   BSP_LCD_DrawPixel(y+1 + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x,255);
	    	   BSP_LCD_DrawPixel(y-1 + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x,255);

	    	   BSP_LCD_DrawPixel(y + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x,255);
	    	   BSP_LCD_DrawPixel(y + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x+1,255);
	    	   BSP_LCD_DrawPixel(y + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x-1,255);
	    	   BSP_LCD_DrawPixel(y+1 + IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x-1,255);
	    	   BSP_LCD_DrawPixel(y-1+ IMG_SCALED_X_SHIFT, IMG_SCALED_Y_SHIFT+x+1,255);

	    	  nn_input_data[x+1][y+1] = PXL_SET;
	    	  nn_input_data[x-1][y-1] = PXL_SET;
	    	  nn_input_data[x+1][y]   = PXL_SET;
	    	  nn_input_data[x-1][y]   = PXL_SET;
	    	  nn_input_data[x][y+1]   = PXL_SET;
	    	  nn_input_data[x][y-1]   = PXL_SET;
	    	  nn_input_data[x+1][y-1] = PXL_SET;
	    	  nn_input_data[x-1][y+1] = PXL_SET;
	    	  nn_input_data[x][y]     = PXL_SET;

	    	 printf("X =%d Y=%d\r\n", ui_state.X,ui_state.Y);


	        }

	        else if((ui_state.X  > ERASE_BUTTON_X1 && ui_state.X <ERASE_BUTTON_X2)  && (ui_state.Y >ERASE_BUTTON_Y1 && ui_state.Y < ERASE_BUTTON_Y2) )
	        {
	      	  user_interface_reset();
	      	  mnist_game_reset(&nn_input_data, &_1st_pred,&_2nd_pred);

	        }
	       }
	      HAL_Delay(1);

	       if(BSP_PB_GetState(BUTTON_KEY)){


	        MX_X_CUBE_AI_Process(nn_input_data,nn_output_data);

	        for(int i =0;i<NUM_CLASSES;i++){

	      	  if(_1st_pred.prob  < nn_output_data[i]){
	      		  _2nd_pred.label = _1st_pred.label;
	      		  _2nd_pred.prob  = _1st_pred.prob;
	      		  _1st_pred.prob  = nn_output_data[i];
	      		  _1st_pred.label =i;

	           }
	      	  else if(_2nd_pred.prob < nn_output_data[i]){
	      		  _2nd_pred.label = i;
	      		  _2nd_pred.prob =  nn_output_data[i];
	      	  }
	        }

	  	  printf("  AI First Guess %d  %f \n\r",_1st_pred.label,_1st_pred.prob);
	  	  printf("  AI Second Guess %d  %f \n\r",_2nd_pred.label,_2nd_pred.prob);

	  	  sprintf(_1st_pred_str,"%d",_1st_pred.label);

	  	  BSP_LCD_SetTextColor(LCD_COLOR_RED);
	  	  BSP_LCD_SetFont(&Font20);
	  	  BSP_LCD_SetBackColor(LCD_COLOR_LIGHTCYAN);
	  	  BSP_LCD_DisplayStringAt(130,255,(uint8_t *)_1st_pred_str,LEFT_MODE);


	  	  mnist_game_reset(&nn_input_data, &_1st_pred,&_2nd_pred);

	       }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 50;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void mnist_game_reset(ai_float** in_data, pred_probType * _1st_pred, pred_probType *_2nd_pred){

	memset(in_data,0.0,sizeof(in_data[0][0])*NN_INPUT_IMAGE_WIDTH *NN_INPUT_IMAGE_HEIGHT);
	_2nd_pred->label = _2nd_pred->prob = _1st_pred->label = _1st_pred->prob  =0.0;
}

void touch_sensor_init(){
if(BSP_TS_Init(240,320) == TS_ERROR){
	printf("Touch sensor failure\r\n");
 }
  HAL_Delay(100);
}
void user_interface_init(void){
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(1,LCD_FRAME_BUFFER_LAYER1);

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(LCD_COLOR_RED);
  BSP_LCD_SetColorKeying(1,LCD_COLOR_DARKCYAN);
  BSP_LCD_SetLayerVisible(1,DISABLE);

  BSP_LCD_LayerDefaultInit(0,LCD_FRAME_BUFFER_LAYER0);
  BSP_LCD_SelectLayer(0);
  BSP_LCD_DisplayOn();
  HAL_Delay(150);
  BSP_LCD_Clear(LCD_COLOR_LIGHTCYAN);
  HAL_Delay(100);

}


void user_interface_reset(){
  BSP_LCD_Clear(LCD_COLOR_LIGHTCYAN);
  //NOTE COLOR CHANGED FROM DARKYELLOW TO BLACK
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_SetBackColor(LCD_COLOR_LIGHTCYAN);

  BSP_LCD_DisplayStringAt(5,255,(uint8_t*) "NN  Guess:", LEFT_MODE);
  BSP_LCD_DrawRect(200,250,28,28);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);

  BSP_LCD_FillRect(5,285,230,30);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetFont(&Font16);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_DisplayStringAt(330,291,(uint8_t*) "ERASE", LEFT_MODE);
  BSP_LCD_DrawRect(2,2,237,237);
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
}

int _write(int fd, const void *buff, int c){

	 HAL_StatusTypeDef st;

	 st =  HAL_UART_Transmit(&huart1,(uint8_t *)buff,c,HAL_MAX_DELAY);
	 return (st == HAL_OK ? c :0);
}
/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

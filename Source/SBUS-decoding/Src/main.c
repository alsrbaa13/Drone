/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define COUNTOF(__BUFFER__) (sizeof(__BUFFER__)/sizeof(*(__BUFFER__)))
#define BUFFERSIZE(buffer) (COUNTOF(buffer))

#define SBUS_DATA_SIZE 23
#define UART_DATA_SIZE 25
#define START_BYTE 0x0f
#define END_BYTE 0x04



typedef struct _SBUS
{
  uint8_t         uart_rx_temp_buff[2];
  uint8_t         uart_rx_buff[UART_DATA_SIZE];
  uint16_t        data_buff[18];
  uint16_t        idx;
  
  volatile        uint8_t rx_flag;
  volatile        uint8_t start_flag;

}sbus_struct;

typedef struct _SBUS_PWM
{
  float pwm;
    
  float min_pwm;
  float max_pwm;
  float min_duty;
  float max_duty;
    
  
}sbus_pwm_struct;
  
  sbus_pwm_struct sbus_pwm;
  sbus_struct sbus;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void sbus_data_decoding();
void sbus_data_filtering();
void sbus_pwm_make(uint16_t F_value);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
 
  sbus_pwm.min_duty = 4598;
  sbus_pwm.max_duty = 8126;
  sbus_pwm.max_pwm = 1696;
  sbus_pwm.min_pwm = 352;
  
  sbus.idx = 0;
  sbus.rx_flag = 0;
  sbus.start_flag = 0;
  
  sbus_pwm.pwm = 0;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();

  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1,sbus.uart_rx_buff,1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {   
    if(sbus.rx_flag == 1)
    {
      sbus.rx_flag = 0;
      sbus_data_filtering();     
    }
  
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
void sbus_pwm_make(uint16_t F_value){
   //pwm = value / ((max_pwm - min_pwm) / (max_duty - min_duty)) + 3695
   float pulse;
   
   pulse = F_value / ((sbus_pwm.max_pwm - sbus_pwm.min_pwm) / (sbus_pwm.max_duty - sbus_pwm.min_duty)) + 3695;
   sbus_pwm.pwm = pulse;
   __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, sbus_pwm.pwm);
    
   htim1.Instance -> CCR1 = sbus_pwm.pwm;
   
   printf("%.4d %.4f \r\n", F_value, sbus_pwm.pwm);
}

void sbus_data_filtering()
{
  if(sbus.idx == 0 && sbus.uart_rx_temp_buff[0] != START_BYTE)
    sbus.start_flag = 0;
  
  if(sbus.idx == 0 && sbus.uart_rx_temp_buff[0] == START_BYTE && sbus.start_flag == 0)
    sbus.start_flag = 1;
  
  if(sbus.start_flag)
  {
    sbus.uart_rx_buff[sbus.idx++] = sbus.uart_rx_temp_buff[0];
  
    if(sbus.idx == UART_DATA_SIZE)
    {
      sbus.idx = 0;
      sbus.start_flag = 0;
      
      if((sbus.uart_rx_buff[24]&0x0f) == 0x04)
        sbus_data_decoding();
    }
  }
}

void sbus_data_decoding()
{ 
  sbus.data_buff[0] = sbus.uart_rx_buff[1] + (uint16_t)((sbus.uart_rx_buff[2]&0x07)<<8);
  sbus.data_buff[1] = (uint16_t)((sbus.uart_rx_buff[2]&0xf8)>>3) + (uint16_t)((sbus.uart_rx_buff[3]&0x3f)<<5);
  sbus.data_buff[2] = (uint16_t)((sbus.uart_rx_buff[3]&0xc0)>>6) + (uint16_t)(sbus.uart_rx_buff[4]<<2) + (uint16_t)((sbus.uart_rx_buff[5]&0x01)<<10);
  sbus.data_buff[3] = (uint16_t)((sbus.uart_rx_buff[5]&0xfe)>>1) + (uint16_t)((sbus.uart_rx_buff[6]&0x0f)<<7);
  sbus.data_buff[4] = (uint16_t)((sbus.uart_rx_buff[6]&0xf0)>>4) + (uint16_t)((sbus.uart_rx_buff[7]&0x7f)<<4);
  sbus.data_buff[5] = (uint16_t)((sbus.uart_rx_buff[7]&0x80)>>7) + (uint16_t)(sbus.uart_rx_buff[8]<<1) + (uint16_t)((sbus.uart_rx_buff[9]&0x03)<<9);
  sbus.data_buff[6] = (uint16_t)((sbus.uart_rx_buff[9]&0xfc)>>2) + (uint16_t)((sbus.uart_rx_buff[10]&0x1f)<<6);
   
  sbus_pwm_make(sbus.data_buff[0]);
  
  printf("%.4d %.4d %.4d %.4d %.4d %.4d %.4d \r\n",sbus.data_buff[0],sbus.data_buff[1], sbus.data_buff[2], sbus.data_buff[3], sbus.data_buff[4], sbus.data_buff[5], sbus.data_buff[6]);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{ 
  HAL_UART_Receive_IT(&huart1,sbus.uart_rx_temp_buff,1);
  
  if(!sbus.rx_flag)
    sbus.rx_flag = 1;
}

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF); 
  
  return ch;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to rep nort the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

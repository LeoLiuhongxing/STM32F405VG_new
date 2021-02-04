#include "types.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_uart.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"
#include "uart2.h"
#include "task.h"
#include "stdio.h"

UART_HandleTypeDef huart2;
/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void)
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

void put_data(u8* data, u16 size)
{
#if 1
    
    taskENTER_CRITICAL();
    HAL_UART_Transmit(&huart2, data, size, 4);
    taskEXIT_CRITICAL();
#endif
}

void put_char(u16 data)
{
    u8 temp[5] = {0,0,0,0,0};
    temp[0] = data/10000 + 0x30;
    temp[1] = data/1000%10 + 0x30;
    temp[2] = data/100%10 + 0x30;
    temp[3] = data/10%10 + 0x30;
    temp[4] = data%10 + 0x30;
    put_data(temp,5);
    temp[0] ='\r';
    temp[1] ='\n';
    put_data(temp,2);
}

void put_string(const char* string, u8 size)
{
    char temp[128];
    if(size > 128)
    {
        temp[0] = 'E';
        temp[1] = 'r';
        temp[2] = 'r';
    }else
    {
        sprintf(&(temp[0]),string,size);
        put_data((u8*)temp,size);
        temp[0] = '\r';
        temp[1] = '\n';
    } 
    temp[0] = '\r';
    temp[1] = '\n';  
    put_data((u8*)temp,2);
    
}

void receive_data(u8 *pData)
{
    taskENTER_CRITICAL();
    HAL_UART_Receive(&huart2, pData, 5, 2);
    taskEXIT_CRITICAL();
}

void test_uart2(void)
{
    u8 temp[5] = {0,0,0,0,0};
    receive_data(temp);
    if(temp[0] != 0)
        put_data(temp,5);
}
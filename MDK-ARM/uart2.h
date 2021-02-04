#ifndef __UART2_H
#define __UART2_H

#ifdef __cplusplus
 extern "C" {
#endif
     
extern UART_HandleTypeDef huart2;
void MX_USART2_UART_Init(void);
void put_data(u8* data, u16 size);
void put_char(u16 data);
void put_string(const char* string, u8 size);
void receive_data(u8 *pData);
void test_uart2(void);     
     
#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_FLASH_H */

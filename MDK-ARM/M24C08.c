/*
    ========================================================================
    Processor:      STM32F405VGT6 ,flash chip M24C08-DRE
    Author:         Leo Liu
    Date:           04/2020
    Module:         M24C08.c
    Description:    flash driver using I2c interface 
    =========================================================================
*/
#include "types.h"
#include "M24C08.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#define EEPROM_I2C_ADDRESS      (uint16_t)0xA6//0xAE

#define EEPROM_I2C_ADDRESS_ID   (uint16_t)0xB6//0xAE

#define EEPROM_ADDRESS          (uint16_t)0x0000
#define PAGE_SIZE               (uint16_t)2//bytes

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t master_write[PAGE_SIZE];
uint8_t master_read[PAGE_SIZE];
uint8_t flag_TX_complete = 1;
uint8_t flag_RX_complete = 1;

// ************************* functions: ***************************************

/*
    =====================================================================================
    Name:
    Author:         Leo Liu
    Date:           04/2020
    Parameter:
    Return value:
    Description:    init flash driver subsystem, after the I2C init.
    =====================================================================================
*/


void EEPROM_init(void)
{
     //MX_I2C1_Init();
    // read device ID:
    //flash_read_deviceid();
    // *DEVEL*
    //flash_test();
    //__nop();
}

void EEPROM_test(void)
{
}

_Bool EEPROM_hwid_ok(void)
{
    _Bool result = TRUE; 
    return result;
}

void EEPROM_read_data(uint32_t startadr, uint8_t* buffer, uint8_t count)
{
    
}


void EEPROM_write_data(uint32_t startadr, uint8_t* buffer, uint8_t count)
{
    
}


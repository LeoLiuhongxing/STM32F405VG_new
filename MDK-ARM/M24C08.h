/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __M24C08_H
#define __M24C08_H

#ifdef __cplusplus
extern "C" {
#endif

// * memory layout:
#define FLASH_COUNT_BYTES_PER_PAGE           16         // number of bytes per page
#define FLASH_PAGE_NUMBER_TOTAL              64         // number of bytes per "line" (arbitrary distinction to handle page wrap-around)
// * transfer count:
#define FLASH_COUNT_SINGLEPASS               16

#define MANUFACTURE_ID      0x20
#define FAMILY_CODE         0xE0
#define MEMORY_DENSITY_CODE 0x0A


    
typedef union
{
    uint8_t     data[3];

    struct
    {
        uint8_t     mf_id;          // manufacture ID       (0x20)
        uint8_t     mem_type;       // family code          (0xE0)
        uint8_t     mem_size;       // memory density code  (0x0A)
    } bytes;
} t_flash_rdid;

typedef struct
{
    t_flash_rdid    rdid;
} t_flash;
    
    
void flash_init(void);

void flash_test(void);
void flash_polling_test(void);
_Bool flash_hwid_ok(void);
  
// * private:

void flash_init_device(void);
void flash_read_deviceid(void);
uint8_t flash_read_status(void);

void flash_erase_sector(uint32_t sectorno);
void flash_erase_block(uint32_t blockno);
void flash_erase_chip(void);
void flash_write_wren(void);
void flash_write_wrdi(void);
void flash_read_data(uint32_t startadr, uint8_t* buffer, uint8_t count);
void flash_read_data_single(uint32_t startadr, uint8_t* writeptr, uint8_t count);
void flash_write_data(uint32_t startadr, uint8_t* buffer, uint8_t count);
void flash_write_data_single(uint32_t startadr, uint8_t* readptr, uint8_t count);
void flash_wait_for_write_complete(void);

#endif

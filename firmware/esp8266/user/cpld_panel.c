#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"
#include "user_config.h"
#include "driver/gpio16.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "user_interface.h"

static int holdoff=0;
volatile int fbdone=0;
static volatile int column=0;
static int ptr=0;
static int row=0;
extern uint8_t framebuffer[32*32];

#define CPLDCS 5 /* GPIO4?? */

#define HOLDOFF 6


LOCAL inline void myspi_master_9bit_write(uint8 spi_no, uint8 high_bit, uint8 low_8bit)
{
    uint32 regvalue;
    uint8 bytetemp;

    if (spi_no > 1) {
        return;
    }

    if (high_bit) {
        bytetemp = (low_8bit >> 1) | 0x80;
    } else {
        bytetemp = (low_8bit >> 1) & 0x7f;
    }

    regvalue = 0x80000000 | ((uint32)bytetemp);		//configure transmission variable,9bit transmission length and first 8 command bit

    if (low_8bit & 0x01) {
        regvalue |= BIT15;    //write the 9th bit
    }
    WRITE_PERI_REG(SPI_FLASH_USER2(spi_no), regvalue);				//write  command and command length into spi reg
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);		//transmission start
}


LOCAL inline void spi_select(int selected)
{
    GPIO_OUTPUT_SET(CPLDCS, selected);
}

LOCAL inline void spi_wait_transmission()
{
    while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);
}

void ICACHE_FLASH_ATTR spi_setup()
{
    spi_master_init(HSPI);
    uint32 regvalue = READ_PERI_REG(SPI_FLASH_CTRL2(HSPI));
    WRITE_PERI_REG(SPI_FLASH_CTRL2(HSPI),regvalue);
    WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI), 0x43043); //clear bit 31,set SPI clock div
}

#define FRC1_ENABLE_TIMER  BIT7

typedef enum {
    DIVDED_BY_1 = 0,
    DIVDED_BY_16 = 4,
    DIVDED_BY_256 = 8,
} TIMER_PREDIVED_MODE;

typedef enum {
    TM_LEVEL_INT = 1,
    TM_EDGE_INT   = 0,
} TIMER_INT_MODE;


static inline void strobe_set(int val)
{
    GPIO_OUTPUT_SET(12, val);
}

static inline void strobe()
{
    strobe_set(1);
    strobe_set(0);
}

LOCAL void tim1_intr_handler()
{
    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, 80);

    if (holdoff>0) {
        holdoff--;
        return;
    }

    if (column==0)
        spi_select(0);

    if (column<=31) {
        /* get pixel, upper row */
        uint32 regval = framebuffer[ptr];
        regval <<= 3;
        regval |= framebuffer[ptr+(16*32)] & 0x7;
        regval<<=1;
        regval|=0x80;
        myspi_master_9bit_write(HSPI, 1, regval);
        ptr++;
        column++;
    } else if (column==32) {
        // force clock high.
        myspi_master_9bit_write(HSPI, 1, 0x00);
        column++;
    } else if (column==33) {
        // Send row.
        myspi_master_9bit_write(HSPI, 0, row&0xf);
        // Strobe.
        column=0;
        // Disable OE
        GPIO_OUTPUT_SET(4, 1);

        strobe();
        // We need to wait here.
        while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);
        // Deselect.
        spi_select(1);
        // Enable OE again
        GPIO_OUTPUT_SET(4, 0);

        column=0;

        if ((row&0xf)==0xf) {
            ptr=0;
            holdoff = HOLDOFF * 32;
            fbdone=1;
        }
        row++;
    } 
}

void ICACHE_FLASH_ATTR timer_setup()
{
    ETS_FRC_TIMER1_INTR_ATTACH(tim1_intr_handler, NULL);
    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();

    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
    RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                  DIVDED_BY_16
                  | FRC1_ENABLE_TIMER
                  | TM_EDGE_INT);
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, 80);
}

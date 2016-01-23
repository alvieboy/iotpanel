#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"
#include "user_config.h"
#include "driver/gpio16.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "user_interface.h"
#include "clock.h"
#include "gfx.h"
#include "protos.h"
#include "driver/gpio_fast.h"

//static int holdoff=0;
volatile int fbdone=0;
static volatile int column=0;
//static int ptr=0;
static int row=0;
static int blank=24;
extern uint8_t framebuffer[32*32*HORIZONTAL_PANELS];
static unsigned int ticks = 0;

extern int uart_rx_one_char(uint8_t);

extern int tickcount;

static int lasttickcount;
static int detectcount = 0;

static uint8_t u4_to_gray[16] = {
    0,
    1,
    3,
    2,
    6,
    7,
    5,
    4,
    12,
    13,
    15,
    14,
    10,
    11,
    9,
    8
};

#undef USENMI

#define CPLDCS 5 /* GPIO4?? */

#define HOLDOFF 6
#define PRELOAD (36/HORIZONTAL_PANELS)

unsigned ICACHE_FLASH_ATTR getTicks()
{
    return ticks;
}

void ICACHE_FLASH_ATTR setBlanking(int a)
{
    blank = a;
}

int ICACHE_FLASH_ATTR getBlanking()
{
    return blank;
}

LOCAL inline void myspi_master_9bit_write(uint8 spi_no, uint8 high_bit, uint8 low_8bit)
{
    uint32 regvalue;
    uint8 bytetemp;

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
    GPIO_FAST_OUTPUT_SET(CPLDCS, selected);
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

    // CNT_L = 0x3, CNT_H = 0x01, CNT_N = 0x3, DIV_PRE=0x1
    // 01 000011 000001 000011

    WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI), 0x00003043);

    // WRITE_PERI_REG(SPI_FLASH_CLOCK(HSPI), 0x43043); //clear bit 31,set SPI clock div
}



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
    GPIO_FAST_OUTPUT_SET(12, val);
}

static inline void strobe()
{
    strobe_set(1);
    strobe_set(0);
}

static uint8_t realrow = 0;

extern void kick_watchdog();
extern void dump_stack(unsigned);

#if 0
LOCAL int my_uart_rx_one_char(uint8 uart_no) {
    if (READ_PERI_REG(UART_STATUS(uart_no)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
        return READ_PERI_REG(UART_FIFO(uart_no)) & 0xff;
    }
    return -1;
}
#endif

static int iter = 0;

#define BASETIMER 8
                                                   // 8 16 16
static uint16_t preloadtimings[] = { 12, 12, 12 }; //BASETIMER, BASETIMER<<1, BASETIMER<<2 };
static uint8_t  blanking[] = { 5, 12, 32 };

#define PRELOAD (36/HORIZONTAL_PANELS)

LOCAL void tim1_intr_handler()
{
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, preloadtimings[iter]);

    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
    kick_watchdog();
    ticks++;

    if (column==0)
        spi_select(0);

    if (column<=(32*HORIZONTAL_PANELS-1)) {

        uint8_t riter = iter+1;

        if (riter==3)
            riter=0;

        uint32_t regval = 0;//(bit&1) ? val: 0x00;

        uint8_t pixelH = framebuffer[column + (realrow*(32*HORIZONTAL_PANELS))];
        uint8_t pixelL = framebuffer[column + (realrow*(32*HORIZONTAL_PANELS)) + (16*32*HORIZONTAL_PANELS)];
        //uint8_t pixelH = 0xff;
        //uint8_t pixelL = 0x07;
        // Pixel order: BGR
        uint8_t mask = 1<<iter;
        regval |= (pixelH & mask)>>iter;
        mask<<=3;
        regval |= (pixelH & mask)>>iter+2;
        mask<<=3;
        regval |= (pixelH & mask)>>iter+4;
        regval<<=3;

        mask = 1<<iter;
        regval |= (pixelL & mask)>>iter;
        mask<<=3;
        regval |= (pixelL & mask)>>iter+2;
        mask<<=3;
        regval |= (pixelL & mask)>>iter+4;
        regval<<=1;

        regval|=0x80;
        myspi_master_9bit_write(HSPI, 1, regval);
#if 1
        if (column==blanking[iter]) {
            // Disable OE
            GPIO_FAST_OUTPUT_SET(4, 1);
        } /*else {
            GPIO_FAST_OUTPUT_SET(4, 0);
        } */
#endif
        column++;
    } else if (column==(32*HORIZONTAL_PANELS)) {
        // force clock high.
        myspi_master_9bit_write(HSPI, 1, 0x00);
        column++;
    } else if (column==(32*HORIZONTAL_PANELS)+1) {
        // Send row.
        // Disable OE
        GPIO_FAST_OUTPUT_SET(4, 1);

        myspi_master_9bit_write(HSPI, 0, realrow&0xf);
        // Strobe.
        column++;

    } else if (column==(32*HORIZONTAL_PANELS)+2) {
        strobe();
        spi_select(1);
        // Enable OE again
        column++;
    } else if (column==(32*HORIZONTAL_PANELS)+3) {

        GPIO_FAST_OUTPUT_SET(4, 0);

        column=0;
#if 1
        if ((row&0xf)==0xf) {
            //holdoff = HOLDOFF * (32*HORIZONTAL_PANELS);
            fbdone=1;
        }
#endif
        iter++;
        if (iter==3) {
            row++;
            realrow = u4_to_gray[ row & 0xf ];
            iter=0;
        }
        //ptr = realrow * (32*HORIZONTAL_PANELS);
#if 1
        // Debug stuff
        {
            if (lasttickcount == tickcount) {
                detectcount++;
                if (detectcount>=1000000) {
                    register unsigned sp asm("a1");
                    dump_stack(sp);
                }
            } else {
                detectcount =0;
            }

            lasttickcount = tickcount;
        }
#endif
    }
}

/*
 FRC1 clock is 5MHz
 Prescaler is 80

 PRELOAD is 36/2 == 18

 APB is 80MHz.
 80/16 == 5Mhz.

 277KHz interrupt rate.

*/

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_ENABLE_AUTOLOAD  BIT6
#define FRC1_TIMER_NMI BIT15

void ICACHE_FLASH_ATTR timer_setup()
{
    // Configure also GPIO.
#ifdef USENMI
    ETS_FRC_TIMER1_NMI_INTR_ATTACH(tim1_intr_handler);
#else
    ETS_FRC_TIMER1_INTR_ATTACH(tim1_intr_handler, NULL);
#endif

    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, PRELOAD);
    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
    RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                  DIVDED_BY_16
                  | FRC1_ENABLE_TIMER
                  | FRC1_ENABLE_AUTOLOAD
                  | TM_EDGE_INT
#ifdef USENMI
                  | FRC1_TIMER_NMI
#endif
                 );

    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();
}

void panel_stop()
{
    ETS_FRC1_INTR_DISABLE();
    GPIO_FAST_OUTPUT_SET(4, 1);
}

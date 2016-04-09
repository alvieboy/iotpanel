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
#include "framebuffer.h"

#define USEDATA
#undef USEGRAYCODE

//static int holdoff=0;
static int frames = 0;
static uint8_t column=0;
//static int ptr=0;
static int row=0;
static int blank=24;

unsigned int ticks = 0;

extern int uart_rx_one_char(uint8_t);

extern int tickcount;

//static int lasttickcount;
//static int detectcount = 0;
static uint8_t realrow = 0;

extern void kick_watchdog();
extern void dump_stack(unsigned);

static pixel_t *currentBuffer = NULL;
static uint8_t currentBufferId=1;

#ifdef USEGRAYCODE

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
#endif

#undef USENMI

#define CPLDCS 5 /* GPIO4?? */

#define HOLDOFF 6
#define PRELOAD (36/HORIZONTAL_PANELS)

#define TICKS_PER_BUFFER 1

#if 0
unsigned ICACHE_FLASH_ATTR getTicks()
{
    return ticks;
}
#endif

unsigned ICACHE_FLASH_ATTR getFrames()
{
    return frames;
}



void ICACHE_FLASH_ATTR setBlanking(int a)
{
    blank = a;
}

int ICACHE_FLASH_ATTR getBlanking()
{
    return blank;
}

LOCAL inline void spi_wait_transmission()
{
    while (READ_PERI_REG(SPI_FLASH_CMD(HSPI))&SPI_FLASH_USR);
}

LOCAL inline void myspi_master_32bit_write(uint8 spi_no, uint32 data)
{
    WRITE_PERI_REG(SPI_FLASH_USER1(spi_no), (31<<SPI_USR_OUT_BITLEN_S) | (31<<SPI_USR_DIN_BITLEN_S) );
    WRITE_PERI_REG(SPI_FLASH_C0(spi_no), data);
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);
}

LOCAL inline void myspi_master_64bit_write(uint8 spi_no, uint32 data1, uint32 data2)
{
    WRITE_PERI_REG(SPI_FLASH_USER1(spi_no), (63<<SPI_USR_OUT_BITLEN_S) | (63<<SPI_USR_DIN_BITLEN_S) );
    WRITE_PERI_REG(SPI_FLASH_C0(spi_no), data1);
    WRITE_PERI_REG(SPI_FLASH_C1(spi_no), data2);
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);
}

LOCAL inline void myspi_master_8bit_write(uint8 spi_no, uint8 data)
{
#ifdef USEDATA
    WRITE_PERI_REG(SPI_FLASH_USER1(spi_no), (7<<SPI_USR_OUT_BITLEN_S) | (7<<SPI_USR_DIN_BITLEN_S) );
    WRITE_PERI_REG(SPI_FLASH_C0(spi_no), data);
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);
#else
    uint32 regvalue;
    regvalue = 0x70000000 | ((uint32)data);
    WRITE_PERI_REG(SPI_FLASH_USER2(spi_no), regvalue);
    SET_PERI_REG_MASK(SPI_FLASH_CMD(spi_no), SPI_FLASH_USR);
#endif
}


LOCAL inline void spi_select(int selected)
{
    GPIO_FAST_OUTPUT_SET(CPLDCS, selected);
}

void ICACHE_FLASH_ATTR spi_setup()
{
    spi_master_init(HSPI);
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

static inline void switchToNextBuffer()
{
    uint8_t nextBufferId = (currentBufferId+1)&1;

    if ((bufferStatus[nextBufferId] == BUFFER_READY) && (ticks==TICKS_PER_BUFFER)) {
        bufferStatus[currentBufferId] = BUFFER_FREE;
        currentBufferId = nextBufferId;
        bufferStatus[currentBufferId] = BUFFER_DISPLAYING;
        currentBuffer = &framebuffers[currentBufferId][0];
        ticks = 0;
    } else {
        if (ticks<TICKS_PER_BUFFER) {
            ticks++;
        }
    }
}

static int iter = 0;

#define BASETIMER 8
                                                   
//static uint16_t preloadtimings[] = { 12, 12, 12 }; //BASETIMER, BASETIMER<<1, BASETIMER<<2 };
static uint16_t preloadtimings[] = { 40,40,40 }; //BASETIMER, BASETIMER<<1, BASETIMER<<2 };
static uint8_t  blanking[] = { 8, 16, 32 };

void setPreloadValues( uint16_t *values )
{
    preloadtimings[0] = values[0];
    preloadtimings[1] = values[1];
    preloadtimings[2] = values[2];
}

void setBlankValues( uint8_t *values )
{
    blanking[0] = values[0];
    blanking[1] = values[1];
    blanking[2] = values[2];
}

#define PRELOAD (36/HORIZONTAL_PANELS)

LOCAL void tim1_intr_handler()
{
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, preloadtimings[iter]);
    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);

    kick_watchdog();

    if (column==0)
        spi_select(0);

    if (column<=(32*HORIZONTAL_PANELS-1)) {

        uint8_t riter = iter+1;

        if (riter==3)
            riter=0;

        uint32_t regval = 0;

        if (currentBuffer==NULL) {
            column+=4;
            return;
        }
        const pixel_t *pixelHp = &currentBuffer[column + (realrow*(32*HORIZONTAL_PANELS))];
        const pixel_t *pixelLp = &currentBuffer[column + (realrow*(32*HORIZONTAL_PANELS)) + (16*32*HORIZONTAL_PANELS)];
        // Pixel order: BGR
        uint32_t pixelH = *(uint32*)pixelHp;
        uint32_t pixelL = *(uint32*)pixelLp;

        pixelH>>=riter;
        pixelL>>=riter;

        int notfirst = riter==0? 0:1;

        uint32_t mask = 0x01010101;
        /* Red */
        regval |= (pixelL & mask);
        regval |= (pixelH & mask)<<3;
        mask<<=3;
        /* Green */
        regval |= (pixelL & mask)>>2;
        regval |= (pixelH & mask)<<1;
        mask<<= notfirst ? 2:3;
        /* Blue */
        regval |= (pixelL & mask)>>(4-notfirst);
        regval |= (pixelH & mask)>>(1-notfirst);
        //regval <<=1;
        //regval >>= iter;
        regval |= 0x80808080;

        myspi_master_32bit_write(HSPI, regval);

        if (column==blanking[iter]) {
            // Disable OE
            GPIO_FAST_OUTPUT_SET(4, 1);
        }

        column+=4;

    } else if (column==(32*HORIZONTAL_PANELS)) {
        // force clock high.
        myspi_master_8bit_write(HSPI, 0x80);
        column++;
    } else if (column==(32*HORIZONTAL_PANELS)+1) {
        // Send row.
        // Disable OE
        GPIO_FAST_OUTPUT_SET(4, 1);

        myspi_master_8bit_write(HSPI, (realrow&0x0f)<<1);
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
        iter++;
        if (iter==3) {

            if ((row&0xf)==0xf) {
                switchToNextBuffer();
                frames++;
            }

            row++;
            row&=0xf;
#ifdef USEGRAYCODE
            realrow = u4_to_gray[ row & 0xf ];
#else
            realrow = row;
#endif
            iter=0;
        }
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

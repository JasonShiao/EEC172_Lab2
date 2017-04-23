//*****************************************************************************
//
//  Lab 2 Part I
//
//  Team member 1: Jau Shiuan Shiao
//  Team member 2: Teja Aluru
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Adafruit 1.5" OLED DEMO
// Application Overview - The demo application shows some primitive drawing and
//                          text printing.
//
//*****************************************************************************


//*****************************************************************************

// Standard includes
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"

#include "gpio.h"
#include "gpio_if.c"

// Common interface includes
#include "uart_if.h"
#include "pin_mux_config.h"

#include "test.h"
#include "Adafruit_SSD1351.h"
#include "Adafruit_GFX.h"


#define APPLICATION_VERSION     "1.1.1"
//*****************************************************************************
//
// Application Master/Slave mode selector macro
//
// MASTER_MODE = 1 : Application in master mode
// MASTER_MODE = 0 : Application in slave mode
//
//*****************************************************************************

#define SPI_IF_BIT_RATE  1000000
#define TR_BUFF_SIZE     100

#define MASTER_MSG       "This is CC3200 SPI Master Application\n\r"
#define SLAVE_MSG        "This is CC3200 SPI Slave Application\n\r"


// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static unsigned char g_ucTxBuff[TR_BUFF_SIZE];
//static unsigned char g_ucRxBuff[TR_BUFF_SIZE];
//static unsigned char ucTxBuffNdx;
//static unsigned char ucRxBuffNdx;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//*****************************************************************************

//*****************************************************************************
//
//! SPI Master mode main loop
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
void MasterMain()
{

    // Initialize the message
    memcpy(g_ucTxBuff,MASTER_MSG,sizeof(MASTER_MSG));

    // Reset SPI
    MAP_SPIReset(GSPI_BASE);

    // Configure SPI interface
    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));

    // Enable SPI for communication
    MAP_SPIEnable(GSPI_BASE);

    // Print mode on uart
    Message("Enabled SPI Interface in Master Mode\n\r");

    // Enable Chip select
    MAP_SPICSEnable(GSPI_BASE);

    Adafruit_Init(); // Call initialization function from Adafruit library- turns the screen on


    while(1){

        unsigned int i;
        unsigned int j;

        fillScreen(BLACK); //Fill screen with black

        // black/white all ascii char
        for(i=0; i < 12; i++){
           for(j=0; j<21; j++){
               if(21*i+j >= 255)
                   break;
              drawChar(j*6, 8*i, 21*i+j, BLACK, WHITE, 1);
           }
        }
        MAP_UtilsDelay(20000000);

        // make screen black again
        fillScreen(BLACK);
        /*drawChar(1, 5, 'H', BLACK, WHITE, 1);
        drawChar(7, 5, 'e', BLACK, WHITE, 1);
        drawChar(13, 5, 'l', BLACK, WHITE, 1);
        drawChar(19, 5, 'l', BLACK, WHITE, 1);
        drawChar(25, 5, 'o', BLACK, WHITE, 1);
        drawChar(31, 5, ' ', BLACK, WHITE, 1);
        drawChar(37, 5, 'W', BLACK, WHITE, 1);
        drawChar(43, 5, 'o', BLACK, WHITE, 1);
        drawChar(49, 5, 'r', BLACK, WHITE, 1);
        drawChar(55, 5, 'l', BLACK, WHITE, 1);
        drawChar(61, 5, 'd', BLACK, WHITE, 1);*/
        char* Str = "Hello World!";
        testPrint(5, 10 ,Str, WHITE, BLACK, 2); // print Hello World to screen
        MAP_UtilsDelay(20000000);
        fillScreen(BLACK); // Clear screen after delay

        lcdTestPattern(); // First LCD test pattern from Adafruit Library
        MAP_UtilsDelay(20000000);
        lcdTestPattern2(); // Second LCD test pattern from Adafruit Library
        MAP_UtilsDelay(20000000);

        //println("Hello world");

        // Draw a bunch of colored lines to the screen
        testlines(YELLOW);
        MAP_UtilsDelay(20000000);
        testfastlines(CYAN, MAGENTA);
        MAP_UtilsDelay(20000000);
        testdrawrects(GREEN);
        MAP_UtilsDelay(20000000);
        testfillrects(RED, BLUE);
        MAP_UtilsDelay(20000000);
        testfillcircles(5, YELLOW);
        MAP_UtilsDelay(20000000);
        testdrawcircles(8, RED);
        MAP_UtilsDelay(20000000);
        testroundrects();
        MAP_UtilsDelay(20000000);
        testtriangles();
        MAP_UtilsDelay(20000000);
        //Message("\n\r\r End of loop 1. \n\r");


    }

    MAP_SPICSDisable(GSPI_BASE); // Disable chip select after tests are done

}



//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! Main function for spi demo application
//!
//! \param none
//!
//! \return None.
//
//*****************************************************************************
void main()
{
    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Muxing UART and SPI lines.
    //
    PinMuxConfig();

    //
    // Enable the SPI module clock
    //
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    //
    // Initialising the Terminal.
    //
    InitTerm();

    //
    // Clearing the Terminal.
    //
    ClearTerm();

    //
    // Display the Banner
    //
    Message("\n\n\n\r");
    Message("\t\t   ********************************************\n\r");
    Message("\t\t   CC3200 Adafruit 16-bit Color 1.5\" OLED Application  \n\r");
    Message("\t\t   ********************************************\n\r");
    Message("\n\n\n\r");

    //
    // Reset the peripheral
    //
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // Function to begin SPI Communication with OLED
    MasterMain();



    while(1)
    {

    }

}


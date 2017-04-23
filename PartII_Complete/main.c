//*****************************************************************************
//
//  Lab 2 Part II
//
//  Team member 1: Jau Shiuan Shiao
//  Team member 2: Teja Aluru
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Accelerometer controlled Ball game
// Application Overview - Control the ball on OLED by tilting the launchpad
//
//*****************************************************************************

// Driverlib includes


// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "spi.h"

#include "pin_mux_config.h"
// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"

#include "gpio.h"
#include "gpio_if.c"

// OLED Libraries includes
#include "test.h"
#include "Adafruit_SSD1351.h"
#include "Adafruit_GFX.h"

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.1.1"
#define APP_NAME                "I2C Accelerometer Ball Game"
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}

#define BMA222_ADDRESS          0x18
#define BASE_OFFSET             0x2
#define X_OFFSET                0x3
#define Y_OFFSET                0x5

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define GREEN           0x07E0
#define CYAN            0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

// MASTER_MODE = 1 : Application in master mode
#define MASTER_MODE      1

#define SPI_IF_BIT_RATE  1000000
#define TR_BUFF_SIZE     100

#define MASTER_MSG       "This is CC3200 SPI Master Application\n\r"

#define BALL_RADIUS     2

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



//****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//****************************************************************************

//*****************************************************************************
void
DisplayUsage()
{
    UART_PRINT("Ball control \n\r");
    UART_PRINT("------------- \n\r");
    UART_PRINT("tilt the launchpad to control the ball\n\r");
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t      CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
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
//! Main function handling the I2C example
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
void main()
{

    BoardInit();    // Initialize board configurations

    PinMuxConfig(); // Configure the pinmux settings for the peripherals exercised

    // Enable the SPI module clock
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);

    InitTerm(); // Configuring UART
    ClearTerm();// Clearing the Terminal.

    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_GSPI);

    // I2C Init
    I2C_IF_Open(I2C_MASTER_MODE_FST);

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

    Adafruit_Init(); // Initialize OLED display

    fillScreen(BLACK); // clear screen with black color

    unsigned char ucRegOffset_base = BASE_OFFSET; // set register offset
    unsigned char aucRdDataBuf[256]; // data buffer

    char cTemp;
    int xVal, yVal;
    float vx, vy;
    // initialize ball position at center of the screen
    float xpos = 64, ypos = 64;
    int d_xpos=64, d_ypos=64;
    //unsigned char ucRegOffset_x = X_OFFSET;
    //unsigned char ucRegOffset_y = Y_OFFSET;

    // Display the banner followed by the usage description
    DisplayBanner(APP_NAME);
    //DisplayUsage();

    while(FOREVER)
    {
        //
        // Write the register address to be read from.
        // Stop bit implicitly assumed to be 0.
        //
        I2C_IF_Write(BMA222_ADDRESS,&ucRegOffset_base,1,0);
        //
        // Read the specified length of data (4 bits)
        //
        I2C_IF_Read(BMA222_ADDRESS, &aucRdDataBuf[0], 4);


        // Get X acceleration value, decoding two's complement
        cTemp = (char)(aucRdDataBuf[1]);
        xVal = (int)cTemp;
        if(xVal > 127)
            xVal = xVal-256;

        // Get Y acceleration value, decoding two's complement
        cTemp = (char)(aucRdDataBuf[3]);
        yVal = (int)cTemp;
        if(yVal > 127)
            yVal = yVal-256;


        //UART_PRINT("Read From address complete\n\r");
        //UART_PRINT("xVal = %d, yVal = %d \n\r", xVal, yVal);

        // Clear the previous ball
        fillCircle(d_xpos, d_ypos, BALL_RADIUS, BLACK);

        // Set the boundary for ball position, rounding x,y value to get screen position
        if(xpos < (float)(2 + BALL_RADIUS))
            xpos = 2 + BALL_RADIUS;
        else if(xpos > (float)(width()-1- BALL_RADIUS))
            xpos = width() - 1 - BALL_RADIUS;
        d_xpos = (int)(xpos+0.5);   // rounding

        if(ypos < (float)(2 + BALL_RADIUS))
            ypos = 2 + BALL_RADIUS;
        else if(ypos > (float)(height()-1- BALL_RADIUS))
            ypos = height() - 1 - BALL_RADIUS;
        d_ypos = (int)(ypos+0.5);   // rounding

        // update position derivatives
        vx = (float)xVal/64.0;  // scaling the value. max approx. 1
        vy = (float)yVal/64.0;  // scaling the value. max approx. 1

        //UART_PRINT("vx = %f, vy = %f \n\r", vx, vy);

        // update positions
        xpos = xpos +3*vy;
        ypos = ypos +3*vx;

        // draw the circle based off of latest values
        fillCircle(d_xpos, d_ypos, BALL_RADIUS, YELLOW);

        MAP_UtilsDelay(20000);


    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @
//
//*****************************************************************************



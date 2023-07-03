/*!
 * \file      main.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <string.h>
// #include "board.h"
// #include "gpio.h"
// #include "delay.h"
// #include "timer.h"
#include "radio.h"

#include "systick.h"
#include "gd32f303rct6_board.h"
#include <stdio.h>
// #include <string.h>

// #if defined( REGION_AS923 )

// #define RF_FREQUENCY                                923000000 // Hz

// #elif defined( REGION_AU915 )

// #define RF_FREQUENCY                                915000000 // Hz

// #elif defined( REGION_CN470 )

// #define RF_FREQUENCY                                470000000 // Hz

// #elif defined( REGION_CN779 )

// #define RF_FREQUENCY                                779000000 // Hz

// #elif defined( REGION_EU433 )

// #define RF_FREQUENCY                                433000000 // Hz

// #elif defined( REGION_EU868 )

// #define RF_FREQUENCY                                868000000 // Hz

// #elif defined( REGION_KR920 )

// #define RF_FREQUENCY                                920000000 // Hz

// #elif defined( REGION_IN865 )

// #define RF_FREQUENCY                                865000000 // Hz

// #elif defined( REGION_US915 )

// #define RF_FREQUENCY                                915000000 // Hz

// #elif defined( REGION_RU864 )

// #define RF_FREQUENCY                                864000000 // Hz

// #else
//     #error "Please define a frequency band in the compiler options."
// #endif

#define USE_MODEM_LORA

#define RF_FREQUENCY                                433000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                50000     // bps
#define FSK_BANDWIDTH                               50000     // Hz
#define FSK_AFC_BANDWIDTH                           83333     // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 255 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LED GPIO pins objects
 */
// extern Gpio_t Led1;
// extern Gpio_t Led2;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

/**
 * Main application entry point.
 */
int main( void )
{
    char bandw[10];
    bool isMaster = true;
    uint8_t i;
    uint32_t t;

    // Target board initialization
    // BoardInitMcu( );
    // BoardInitPeriph( );
   
    systick_config();
    nvic_configuration();
    Gpio_Init();
    Usart_Init();
    //Spi_Init();
    soft_spi_init();
    rtc_configuration();


    SX126xReset();
    register_test();
    
    
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

    Radio.SetMaxPayloadLength( MODEM_LORA, BUFFER_SIZE );

#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

    Radio.SetMaxPayloadLength( MODEM_FSK, BUFFER_SIZE );

#else
    #error "Please define a frequency band in the compiler options."
#endif
    if (LORA_BANDWIDTH == 0)
        strcpy(bandw, "125K");
    if (LORA_BANDWIDTH == 1)
        strcpy(bandw, "250K");
    if (LORA_BANDWIDTH == 2)
        strcpy(bandw, "500K");

    printf("Frequency=%d,SF=SF%d,Bandwidth=%s\n",RF_FREQUENCY,LORA_SPREADING_FACTOR,bandw);
    Sw1179_To_Rx();
    PA30dbm_To_Rx();

    Radio.Rx( RX_TIMEOUT_VALUE );
    printf("start rx packet...\n");
    State=LOWPOWER;
    rtc_set_alarm(2048);
    // State=TX;
    // printf("start tx packet...\n");

    while( 1 )
    {
        switch( State )
        {
        case RX:

            // Indicates on a LED that the received frame is a PING
            // GpioToggle( &Led1 );
            Sw1179_To_Rx();
            PA30dbm_To_Rx();
            printf("receive packet successfully\n");
            printf("RSSI=%d,SNR=%d  ",RssiValue,SnrValue);
            for (i = 0; i < BufferSize; i++)
            {
                printf("%02X ", Buffer[i]);
            }
            printf("\n");
            Radio.Rx(RX_TIMEOUT_VALUE);

            State = LOWPOWER;
            rtc_set_alarm(2048);
            break;
        case TX:
            // Indicates on a LED that we have sent a packet
            //GpioToggle( &Led2 );
            // Radio.Rx( RX_TIMEOUT_VALUE );
            Sw1179_To_Tx();
            PA30dbm_To_Tx();
            for ( i = 0; i < 48; i++)
            {
                Buffer[i]='A'+i;
            }
            BufferSize=10;
            DelayMs( 1 );
            Radio.Send( Buffer, BufferSize );

            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            Radio.Rx(RX_TIMEOUT_VALUE);
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
        default:
            // Set low power
            break;
        }

        //BoardLowPowerHandler( );
        // Process Radio IRQ
        if( Radio.IrqProcess != NULL )
        {
            // printf("eeeee\n");
            Radio.IrqProcess( );
        }
    }
}

void OnTxDone( void )
{
    printf("transmit packet successfully\n");
    Radio.Sleep( );
    State = TX;
    delay_1ms(1000);
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    rtc_reset_alarm();
   
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(DEBUG_COM, (uint8_t)ch);
    while(RESET == usart_flag_get(DEBUG_COM, USART_FLAG_TBE));

    return ch;
}

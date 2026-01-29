/*!
 * \file      main.c
 *
 * \brief     LLCC68 LoRa demo with configurable parameters
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "radio.h"
#include "systick.h"
#include "gd32f303rct6_board.h"
#include "gd32f303rct6_delay.h"
#include "sx126x.h"
#include "sx126x-board.h"
#include "main.h"

/* ========================== USER CONFIG ========================== */
#define APP_USE_HW_SPI                         0

#define APP_MODE_TX_PACKET                      0
#define APP_MODE_RX_PACKET                      1
#define APP_MODE_TX_CW                          2
#define APP_MODE                                APP_MODE_TX_PACKET

#define APP_RF_FREQUENCY_HZ                     471000000UL
#define APP_TX_POWER_DBM                        17

#define APP_LORA_BANDWIDTH                      0   /* 0:125kHz, 1:250kHz, 2:500kHz */
#define APP_LORA_SPREADING_FACTOR               7   /* SF7..SF12 */
#define APP_LORA_CODINGRATE                     1   /* 1:4/5, 2:4/6, 3:4/7, 4:4/8 */
#define APP_LORA_PREAMBLE_LENGTH                8

#define APP_LORA_HEADER_EXPLICIT                1   /* 1: explicit, 0: implicit */
#define APP_LORA_PACKET_FIXED_LEN               0   /* 1: fixed, 0: variable (same as header type) */
#define APP_LORA_CRC_ON                          1   /* 1: enable, 0: disable */
#define APP_LORA_IQ_INVERT                       0
#define APP_LORA_PAYLOAD_LEN                     16  /* used when fixed length */

#define APP_RX_TIMEOUT_MS                       1000
#define APP_TX_REPEAT_MS                        1000
#define APP_CW_TIME_SEC                          0xFFFF

#define APP_DIO2_AS_RF_SWITCH                    1
#define APP_DIO3_AS_TCXO_CTRL                    0
#define APP_TCXO_VOLTAGE                         TCXO_CTRL_1_8V
#define APP_TCXO_WAKEUP_TIME_MS                  5

#define APP_DIO1_IRQ_ENABLE                      1
#define APP_DIO1_IRQ_MASK                        ( IRQ_TX_DONE | IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT )
/* ================================================================= */

#if ( APP_LORA_HEADER_EXPLICIT && APP_LORA_PACKET_FIXED_LEN )
#error "Explicit header must use variable length packets"
#endif
#if ( !APP_LORA_HEADER_EXPLICIT && !APP_LORA_PACKET_FIXED_LEN )
#error "Implicit header must use fixed length packets"
#endif

#define BUFFER_SIZE                              255

static RadioEvents_t RadioEvents;
static uint8_t Buffer[BUFFER_SIZE];
static uint16_t BufferSize = 0;

static volatile bool g_tx_in_progress = false;
static volatile uint32_t g_tx_delay_ms = 0;

static void App_BoardInit( void );
static void App_RadioInit( void );
static void App_RadioConfig( void );
static void App_ApplyDioConfig( void );
static void App_SetRfTx( void );
static void App_SetRfRx( void );
static void App_SendPacket( void );
static void App_StartRx( void );
static void App_StartTxCw( void );

static void OnTxDone( void );
static void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
static void OnTxTimeout( void );
static void OnRxTimeout( void );
static void OnRxError( void );

int main( void )
{
    App_BoardInit();

    printf("================ LLCC68 APP START ================\n");
    printf("RF=%lu Hz, PWR=%d dBm, SF=%d, BW=%d, CR=%d\n",
           APP_RF_FREQUENCY_HZ, APP_TX_POWER_DBM,
           APP_LORA_SPREADING_FACTOR, APP_LORA_BANDWIDTH, APP_LORA_CODINGRATE);

    SX126xReset();
    por_sw_config();
    register_test();

    App_RadioInit();
    App_RadioConfig();
    App_ApplyDioConfig();

#if ( APP_MODE == APP_MODE_TX_CW )
    App_StartTxCw();
#endif

    while ( 1 )
    {
        if ( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess();
        }

#if ( APP_MODE == APP_MODE_TX_PACKET )
        if ( !g_tx_in_progress && ( g_tx_delay_ms == 0 ) )
        {
            App_SendPacket();
        }
        if ( g_tx_delay_ms > 0 )
        {
            DelayMs( 1 );
            g_tx_delay_ms--;
        }
#elif ( APP_MODE == APP_MODE_RX_PACKET )
        /* RX runs continuously, nothing to do here */
#endif

        iwdg_feed();
    }
}

static void App_BoardInit( void )
{
    systick_config();
    nvic_configuration();
    Gpio_Init();
    Usart_Init();

    Board_SetSpiMode( APP_USE_HW_SPI ? true : false );
    if ( APP_USE_HW_SPI )
    {
        Spi_Init();
    }
    else
    {
        soft_spi_init();
    }

    iwdg_config();
}

static void App_RadioInit( void )
{
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.CadDone = NULL;

    Radio.Init( &RadioEvents );
}

static void App_RadioConfig( void )
{
    bool fix_len = APP_LORA_PACKET_FIXED_LEN ? true : false;

    Radio.SetChannel( APP_RF_FREQUENCY_HZ );

    Radio.SetTxConfig_meter( MODEM_LORA, APP_TX_POWER_DBM, 0,
                             APP_LORA_BANDWIDTH, APP_LORA_SPREADING_FACTOR,
                             APP_LORA_CODINGRATE, APP_LORA_PREAMBLE_LENGTH,
                             fix_len, APP_LORA_CRC_ON,
                             0, 0, APP_LORA_IQ_INVERT, 3000 );

    Radio.SetRxConfig( MODEM_LORA, APP_LORA_BANDWIDTH, APP_LORA_SPREADING_FACTOR,
                       APP_LORA_CODINGRATE, 0, APP_LORA_PREAMBLE_LENGTH,
                       0, fix_len,
                       fix_len ? APP_LORA_PAYLOAD_LEN : 0,
                       APP_LORA_CRC_ON, 0, 0, APP_LORA_IQ_INVERT, true );

    SX126xSetLoRaSymbNumTimeout( 0x00 );
    SX126xSetRxTxFallbackMode( 0x40 );
    SX126xSetStopRxTimerOnPreambleDetect( false );
    SX126xSetStandby( STDBY_XOSC );

#if ( APP_MODE == APP_MODE_RX_PACKET )
    App_StartRx();
#endif
}

static void App_ApplyDioConfig( void )
{
#if ( APP_DIO2_AS_RF_SWITCH )
    SX126xSetDio2AsRfSwitchCtrl( true );
#else
    SX126xSetDio2AsRfSwitchCtrl( false );
#endif

#if ( APP_DIO3_AS_TCXO_CTRL )
    SX126xSetDio3AsTcxoCtrl( APP_TCXO_VOLTAGE, ( (uint32_t) APP_TCXO_WAKEUP_TIME_MS ) << 6 );
#endif

#if ( APP_DIO1_IRQ_ENABLE )
    SX126xSetDioIrqParams( APP_DIO1_IRQ_MASK, APP_DIO1_IRQ_MASK, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
#else
    SX126xSetDioIrqParams( IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
#endif
}

static void App_SetRfTx( void )
{
#if ( !APP_DIO3_AS_TCXO_CTRL )
    tx_by_pa_sw_config();
#endif
    Sw1179_To_Tx();
    PA30dbm_To_Tx();
}

static void App_SetRfRx( void )
{
#if ( !APP_DIO3_AS_TCXO_CTRL )
    rx_sw_config();
#endif
    Sw1179_To_Rx();
    PA30dbm_To_Rx();
}

static void App_SendPacket( void )
{
    uint8_t i;

    App_SetRfTx();
    Radio.Standby();

#if ( APP_LORA_PACKET_FIXED_LEN )
    BufferSize = APP_LORA_PAYLOAD_LEN;
    for ( i = 0; i < BufferSize; i++ )
    {
        Buffer[i] = (uint8_t)( 'A' + ( i % 26 ) );
    }
#else
    const char *msg = "PING";
    BufferSize = (uint16_t)strlen( msg );
    memcpy( Buffer, msg, BufferSize );
#endif

    g_tx_in_progress = true;
    Radio.Send( Buffer, (uint8_t)BufferSize );
}

static void App_StartRx( void )
{
    App_SetRfRx();
    Radio.Rx( APP_RX_TIMEOUT_MS );
    printf("RX started, timeout=%u ms\n", APP_RX_TIMEOUT_MS );
}

static void App_StartTxCw( void )
{
    App_SetRfTx();
    printf("CW TX: freq=%lu Hz, power=%d dBm\n", APP_RF_FREQUENCY_HZ, APP_TX_POWER_DBM );
    Radio.SetTxContinuousWave( APP_RF_FREQUENCY_HZ, APP_TX_POWER_DBM, APP_CW_TIME_SEC );
    while ( 1 )
    {
        iwdg_feed();
    }
}

static void OnTxDone( void )
{
    printf("TX done\n");
    Radio.Standby();
    g_tx_in_progress = false;
    g_tx_delay_ms = APP_TX_REPEAT_MS;
}

static void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    uint16_t i;

    Radio.Standby();
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );

    printf("RX done: RSSI=%d, SNR=%d, len=%u\n", rssi, snr, size );
    for ( i = 0; i < BufferSize; i++ )
    {
        printf("%02X ", Buffer[i] );
    }
    printf("\n");

    App_StartRx();
}

static void OnTxTimeout( void )
{
    printf("TX timeout\n");
    Radio.Standby();
    g_tx_in_progress = false;
    g_tx_delay_ms = APP_TX_REPEAT_MS;
}

static void OnRxTimeout( void )
{
    printf("RX timeout\n");
    App_StartRx();
}

static void OnRxError( void )
{
    printf("RX error\n");
    App_StartRx();
}

/* retarget the C library printf function to the USART */
int fputc( int ch, FILE *f )
{
    usart_data_transmit( DEBUG_COM, (uint8_t)ch );
    while( RESET == usart_flag_get( DEBUG_COM, USART_FLAG_TBE ) );
    return ch;
}

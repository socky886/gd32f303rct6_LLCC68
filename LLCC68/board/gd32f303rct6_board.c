#include <stdio.h>
#include "gd32f30x.h"
#include "gd32f303rct6_board.h"
#include "gd32f303rct6_gpio.h"
#include "gd32f303rct6_spi.h"
#include "gd32f303rct6_delay.h"
#include "systick.h"

extern int8_t user_pressed;

static volatile bool g_use_hw_spi = true;

Gpio_t sw1179_pin={
    .pinIndex=ANT_SW,
};

Gpio_t Led_Tx={LED_TX};
Gpio_t Led_Rx={LED_RX};
Gpio_t * Pa_tx_en=&Led_Rx;
Gpio_t * Pa_rx_en=&Led_Tx;
void Gpio_Init(void)
{
    // LED_TX
    /* enable the led clock */
    rcu_periph_clock_enable(LED_TX_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(LED_TX_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,LED_TX_PIN);
    GPIO_BC(LED_TX_GPIO_PORT) = LED_TX_PIN;

    // LED_RX
    /* enable the led clock */
    rcu_periph_clock_enable(LED_RX_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(LED_RX_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,LED_RX_PIN);
    GPIO_BC(LED_RX_GPIO_PORT) = LED_RX_PIN;

    // RESET output high
    /* enable the led clock */
    rcu_periph_clock_enable(RESET_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(RESET_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,RESET_PIN);
    GPIO_BOP(RESET_GPIO_PORT) = RESET_PIN;

    // ANT_SW output high
    /* enable the led clock */
    rcu_periph_clock_enable(ANT_SW_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(ANT_SW_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,ANT_SW_PIN);
    GPIO_BOP(ANT_SW_GPIO_PORT) = ANT_SW_PIN;

    // NSS_PIN output high
    /* enable the led clock */
    rcu_periph_clock_enable(NSS_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(NSS_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,NSS_PIN);
    GPIO_BOP(NSS_GPIO_PORT) = NSS_PIN;

    // BUSY_PIN input floating
    /* enable the led clock */
    rcu_periph_clock_enable(BUSY_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(BUSY_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,BUSY_PIN);
    //GPIO_BOP(BUSY_GPIO_PORT) = BUSY_PIN;

    // user key external interrupt init
    /* enable the key clock */
    rcu_periph_clock_enable(USER_KEY_GPIO_CLK);
    rcu_periph_clock_enable(RCU_AF);
    /* configure button pin as input */
    gpio_init(USER_KEY_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, USER_KEY_PIN);
    /* enable and set key EXTI interrupt to the lowest priority */
    nvic_irq_enable(USER_KEY_EXTI_IRQn, 2U, 0U);
    /* connect key EXTI line to key GPIO pin */
    gpio_exti_source_select(USER_KEY_EXTI_PORT_SOURCE, USER_KEY_EXTI_PIN_SOURCE);
    /* configure key EXTI line */
    exti_init(USER_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(USER_KEY_EXTI_LINE);
    
    // IRQ DIO1 external interrupt init
    /* enable the key clock */
    // rcu_periph_clock_enable(IRQ_DIO1_GPIO_CLK);
    // rcu_periph_clock_enable(RCU_AF);
    // /* configure button pin as input */
    // gpio_init(IRQ_DIO1_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, IRQ_DIO1_PIN);
    // /* enable and set key EXTI interrupt to the lowest priority */
    // nvic_irq_enable(IRQ_DIO1_EXTI_IRQn, 2U, 0U);
    // /* connect key EXTI line to key GPIO pin */
    // gpio_exti_source_select(IRQ_DIO1_EXTI_PORT_SOURCE, IRQ_DIO1_EXTI_PIN_SOURCE);
    // /* configure key EXTI line */
    // exti_init(IRQ_DIO1_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    // exti_interrupt_flag_clear(IRQ_DIO1_EXTI_LINE);
}

void soft_spi_init(void)
{
    // MOSI_PIN input floating
    /* enable the led clock */
    rcu_periph_clock_enable(MISO_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(MISO_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,MISO_PIN);

    // SCK output low level
    /* enable the led clock */
    rcu_periph_clock_enable(SCK_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(SCK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,SCK_PIN);
    GPIO_BC(SCK_GPIO_PORT) = SCK_PIN;

    // MOSI output low level
    /* enable the led clock */
    rcu_periph_clock_enable(MOSI_GPIO_CLK);
    /* configure led GPIO port */ 
    gpio_init(MOSI_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,MOSI_PIN);
    GPIO_BC(MOSI_GPIO_PORT) = MOSI_PIN;



}

void Board_SetSpiMode(bool use_hw_spi)
{
    g_use_hw_spi = use_hw_spi;
}

bool Board_GetSpiMode(void)
{
    return g_use_hw_spi ? true : false;
}
/**
 * @brief the default spi is SPI0
 * 
 */
void Spi_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    // rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SPI0);
    // rcu_periph_clock_enable(RCU_SPI2);
    rcu_periph_clock_enable(RCU_AF);

    /* SPI0 GPIO config:SCK/PA5, MISO/PA6, MOSI/PA7 */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    /* PA3 as NSS */
    //gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    
    // gpio_pin_remap_config(GPIO_SPI2_REMAP, ENABLE);
    // /* SPI2 GPIO config: NSS/PA4, SCK/PC10, MISO/PC11, MOSI/PC12 */
    // gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_12);
    // gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    // gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

    spi_parameter_struct spi_init_struct;

    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;//SPI_CK_PL_LOW_PH_1EDGE
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_256;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

    /* SPI2 parameter config */
    // spi_init_struct.device_mode = SPI_SLAVE;
    // spi_init_struct.nss         = SPI_NSS_HARD;
    // spi_init(SPI2, &spi_init_struct);

#if SPI_CRC_ENABLE
    /* configure SPI CRC function */
    spi_crc_polynomial_set(SPI0, 7);
    spi_crc_polynomial_set(SPI2, 7);
    spi_crc_on(SPI0);
    spi_crc_on(SPI2);
#endif /* enable CRC function */

    spi_enable(SPI0);


}
void Usart_Init(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(DEBUG_COM_GPIO_CLK);

    /* enable USART clock */
    rcu_periph_clock_enable(DEBUG_COM_CLK);

    /* connect port to USARTx_Tx */
    gpio_init(DEBUG_COM_GPIO_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, DEBUG_COM_TX_PIN);

    /* connect port to USARTx_Rx */
    gpio_init(DEBUG_COM_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, DEBUG_COM_RX_PIN);

    /* USART configure */
    usart_deinit(DEBUG_COM);
    usart_baudrate_set(DEBUG_COM, 115200U);
    usart_receive_config(DEBUG_COM, USART_RECEIVE_ENABLE);
    usart_transmit_config(DEBUG_COM, USART_TRANSMIT_ENABLE);
    usart_enable(DEBUG_COM);
}

void LED_On(GPIO_Index_TYPE led)
{
    if(led==LED_TX)
        GPIO_BOP(LED_TX_GPIO_PORT) = LED_TX_PIN;
    else
       GPIO_BOP(LED_RX_GPIO_PORT) = LED_RX_PIN; 
}
void LED_Off(GPIO_Index_TYPE led)
{
    if(led==LED_TX)
        GPIO_BC(LED_TX_GPIO_PORT) = LED_TX_PIN;
    else
       GPIO_BC(LED_RX_GPIO_PORT) = LED_RX_PIN; 

}

void EXTI4_IRQHandler()
{
    if(RESET != exti_interrupt_flag_get(IRQ_DIO1_EXTI_LINE)) {
       // gd_eval_led_toggle(LED3);
       printf("IRQ DIO1 Interrupt\n");
        exti_interrupt_flag_clear(IRQ_DIO1_EXTI_LINE);
    }

}
void EXTI10_15_IRQHandler()
{
    if(RESET != exti_interrupt_flag_get(USER_KEY_EXTI_LINE)) {
       // gd_eval_led_toggle(LED3);
        //printf("user key press\n");
        user_pressed=1;
        exti_interrupt_flag_clear(USER_KEY_EXTI_LINE);
    }

}

//----------------------------------------------------------------
//   interface function
//----------------------------------------------------------------
void DelayMs( uint32_t ms )
{
    delay_1ms(ms);
}

void GpioWrite( Gpio_t *obj, uint32_t value )
{
    if(obj->pinIndex==RADIO_RESET)
    {
        if(value)
            GPIO_BOP(RESET_GPIO_PORT) = RESET_PIN;
        else
            GPIO_BC(RESET_GPIO_PORT) = RESET_PIN; 
    }

    if(obj->pinIndex==ANT_SW)
    {
        if(value)
            GPIO_BOP(ANT_SW_GPIO_PORT) = ANT_SW_PIN;
        else
            GPIO_BC(ANT_SW_GPIO_PORT) = ANT_SW_PIN; 
    }

    if(obj->pinIndex==RADIO_NSS)
    {
        if(value)
            GPIO_BOP(NSS_GPIO_PORT) = NSS_PIN;
        else
            GPIO_BC(NSS_GPIO_PORT) = NSS_PIN; 
    }

    if(obj->pinIndex==LED_TX)
    {
        if(value)
            GPIO_BOP(LED_TX_GPIO_PORT) = LED_TX_PIN;
        else
            GPIO_BC(LED_TX_GPIO_PORT) = LED_TX_PIN; 
    }

    if(obj->pinIndex==LED_RX)
    {
        if(value)
            GPIO_BOP(LED_RX_GPIO_PORT) = LED_RX_PIN;
        else
            GPIO_BC(LED_RX_GPIO_PORT) = LED_RX_PIN; 
    }


}

uint32_t GpioRead( Gpio_t *obj )
{
    uint32_t val;
    if(obj->pinIndex==RADIO_BUSY)
        val=gpio_input_bit_get(BUSY_GPIO_PORT,BUSY_PIN);

    if(obj->pinIndex==RADIO_DIO1)
        val=gpio_input_bit_get(IRQ_DIO1_GPIO_PORT,IRQ_DIO1_PIN);

    return val;
}

void spi_clock_delay(void)
{
    int i;
    for ( i = 0; i < 50; i++)
    {
        __NOP();
    __NOP();
    __NOP();
    __NOP();
        
    }
    
    
}
uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
//uint16_t SpiInOut( 0, uint16_t outData )
{
    uint8_t rxData = 0;

    if ( g_use_hw_spi )
    {
        while ( RESET == spi_i2s_flag_get( SPI0, SPI_FLAG_TBE ) )
            ;
        spi_i2s_data_transmit( SPI0, outData );

        while ( RESET == spi_i2s_flag_get( SPI0, SPI_FLAG_RBNE ) )
            ;
        rxData = spi_i2s_data_receive( SPI0 );
    }
    else
    {
        int i;
        for ( i = 0; i < 8; i++ )
        {
            rxData <<= 1;
            if ( outData & 0x80 )
                GPIO_BOP( MOSI_GPIO_PORT ) = MOSI_PIN;
            else
                GPIO_BC( MOSI_GPIO_PORT ) = MOSI_PIN;

            GPIO_BC( SCK_GPIO_PORT ) = SCK_PIN;
            spi_clock_delay();
            GPIO_BOP( SCK_GPIO_PORT ) = SCK_PIN;
            if ( gpio_input_bit_get( MISO_GPIO_PORT, MISO_PIN ) )
                rxData |= 0x01;
            spi_clock_delay();

            outData <<= 1;
        }
    }

    return( rxData );
}

/**
 * @brief switch sw1179 to tx
 *  DIO2=1 in Tx mode
 *  antsw pin=0
 */
void Sw1179_To_Tx(void)
{
    GpioWrite(&sw1179_pin,0);
}
/**
 * @brief switch sw1179 to rx
 *  DIO2= 0 in Rx mode
 *  antsw pin =1
 */ 
void Sw1179_To_Rx(void)
{
    GpioWrite(&sw1179_pin,1);
}

/**
 * @brief 30dbm Pa switch to tx
 * TXEN_PIN=1, RXEN_PIN=0
 */
void PA30dbm_To_Tx(void)
{
    GpioWrite(Pa_tx_en, 1);
    GpioWrite(Pa_rx_en, 0);
}
/**
 * @brief 30dbm pa switch to rx
 * TXEN_PIN=0, RXEN_PIN=1
 */
void PA30dbm_To_Rx(void)
{
    GpioWrite(Pa_tx_en, 0);
    GpioWrite(Pa_rx_en, 1);
}

void nvic_configuration(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(RTC_IRQn,1,0);
}

void rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_ALARM);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1ms */
    rtc_prescaler_set(32);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set date and time to 0*/
    time_set();
}

void time_set(void)
{
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(0);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

void rtc_set_alarm(uint32_t ms)
{
    rtc_alarm_config(ms+rtc_counter_get());
}

void rtc_reset_alarm(void)
{
    rtc_alarm_config(0);
}

void iwdg_config(void)
 {
    /* Enable FWDGT write access */
    fwdgt_write_enable();

    /* Configure prescaler: FWDGT_PSC_DIV64 */
    if (fwdgt_prescaler_value_config(FWDGT_PSC_DIV64) != SUCCESS) {
        while(1); // Hang if config fails
    }

    /* Set reload value (max = 0x0FFF = 4095) for timeout ~6.5s */
    if (fwdgt_reload_value_config(0x0FFF) != SUCCESS) {
        while(1); // Hang if config fails
    }

    /* Start watchdog */
    fwdgt_enable();
}

void iwdg_feed(void) 
{
     /* Refresh the counter */
    fwdgt_counter_reload();
}


// 2025/09/23
/**
 * @brief config the switch for tx by pa
 * set DIO3 to output low
 * 
 */
void tx_by_pa_sw_config(void)
{
    
    SX126xWriteRegister(0x0580, SX126xReadRegister(0x0580) | 0x08);
    SX126xWriteRegister(0x0583, SX126xReadRegister(0x0583) & ~0x08);
    SX126xWriteRegister(0x0584, SX126xReadRegister(0x0584) & ~0x08);
    SX126xWriteRegister(0x0585, SX126xReadRegister(0x0585) & ~0x08);
    // set dio3 output low
    SX126xWriteRegister(0x0920, SX126xReadRegister(0x0920) & ~0x08);

}
/**
 * @brief confgi the switch for tx bypass pa
 *  set DIO3 to output high
 */
void tx_bypass_pa_sw_config(void)
{
    SX126xWriteRegister(0x0580, SX126xReadRegister(0x0580) | 0x08);
    SX126xWriteRegister(0x0583, SX126xReadRegister(0x0583) & ~0x08);
    SX126xWriteRegister(0x0584, SX126xReadRegister(0x0584) & ~0x08);
    SX126xWriteRegister(0x0585, SX126xReadRegister(0x0585) & ~0x08);
    // set dio3 output high
    SX126xWriteRegister(0x0920, SX126xReadRegister(0x0920) | 0x08);

}
/**
 * @brief config the switch for rx
 * set DIO3 to output high
 */
void rx_sw_config(void)
{
     SX126xWriteRegister(0x0580, SX126xReadRegister(0x0580) | 0x08);
    SX126xWriteRegister(0x0583, SX126xReadRegister(0x0583) & ~0x08);
    SX126xWriteRegister(0x0584, SX126xReadRegister(0x0584) & ~0x08);
    SX126xWriteRegister(0x0585, SX126xReadRegister(0x0585) & ~0x08);
    // set dio3 output high
    SX126xWriteRegister(0x0920, SX126xReadRegister(0x0920) | 0x08);
}
/**
 * @brief config the swtich for sleep mode
 *  set DIO3 to output high
 */
void sleep_sw_config(void)
{
    ;
}
/**
 * @brief config the switch for power on reset
 * set DIO3 to output high
 */
void por_sw_config(void)
{
    SX126xWriteRegister(0x0580, SX126xReadRegister(0x0580) | 0x08);
    SX126xWriteRegister(0x0583, SX126xReadRegister(0x0583) & ~0x08);
    SX126xWriteRegister(0x0584, SX126xReadRegister(0x0584) & ~0x08);
    SX126xWriteRegister(0x0585, SX126xReadRegister(0x0585) & ~0x08);
    // set dio3 output high
    SX126xWriteRegister(0x0920, SX126xReadRegister(0x0920) | 0x08);
}

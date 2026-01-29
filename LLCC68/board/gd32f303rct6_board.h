#ifndef __GD32F303RCT6__BOARD__H__
#define __GD32F303RCT6__BOARD__H__
#include "gd32f30x.h"
#include <stdbool.h>
#include "sx126x-board.h"

typedef enum{
    LED_TX=0,
    LED_RX,
    RADIO_NSS,
    RADIO_RESET,
    ANT_SW,
    RADIO_BUSY,
    RADIO_DIO1,

} GPIO_Index_TYPE;

/* Micro definition*/
#define ANT_SW_PIN                         GPIO_PIN_9
#define ANT_SW_GPIO_PORT                   GPIOA
#define ANT_SW_GPIO_CLK                    RCU_GPIOA

#define RESET_PIN                         GPIO_PIN_0
#define RESET_GPIO_PORT                   GPIOA
#define RESET_GPIO_CLK                    RCU_GPIOA

#define BUSY_PIN                         GPIO_PIN_3
#define BUSY_GPIO_PORT                   GPIOB
#define BUSY_GPIO_CLK                    RCU_GPIOB

#define NSS_PIN                         GPIO_PIN_8
#define NSS_GPIO_PORT                   GPIOA
#define NSS_GPIO_CLK                    RCU_GPIOA
//-------------
#define SCK_PIN                         GPIO_PIN_5
#define SCK_GPIO_PORT                   GPIOA
#define SCK_GPIO_CLK                    RCU_GPIOA

#define MOSI_PIN                         GPIO_PIN_7
#define MOSI_GPIO_PORT                   GPIOA
#define MOSI_GPIO_CLK                    RCU_GPIOA

#define MISO_PIN                         GPIO_PIN_6
#define MISO_GPIO_PORT                   GPIOA
#define MISO_GPIO_CLK                    RCU_GPIOA

//------------------

#define LED_TX_PIN                  GPIO_PIN_0
#define LED_TX_GPIO_PORT            GPIOC
#define LED_TX_GPIO_CLK            RCU_GPIOC

#define LED_RX_PIN                  GPIO_PIN_1
#define LED_RX_GPIO_PORT            GPIOC
#define LED_RX_GPIO_CLK             RCU_GPIOC

#define PA_RX_EN_PIN                LED_TX_PIN
#define PA_RX_EN_PORT               LED_TX_GPIO_PORT
#define PA_RX_EN_CLK                LED_TX_GPIO_CLK

#define PA_TX_EN_PIN                LED_RX_PIN
#define PA_TX_EN_PORT               LED_RX_GPIO_PORT
#define PA_TX_EN_CLK                LED_RX_GPIO_CLK

#define IRQ_DIO1_PIN                   GPIO_PIN_4
#define IRQ_DIO1_GPIO_PORT             GPIOB
#define IRQ_DIO1_GPIO_CLK              RCU_GPIOB
#define IRQ_DIO1_EXTI_LINE             EXTI_4
#define IRQ_DIO1_EXTI_PORT_SOURCE      GPIO_PORT_SOURCE_GPIOB
#define IRQ_DIO1_EXTI_PIN_SOURCE       GPIO_PIN_SOURCE_4
#define IRQ_DIO1_EXTI_IRQn             EXTI4_IRQn  

#define USER_KEY_PIN                   GPIO_PIN_13
#define USER_KEY_GPIO_PORT             GPIOC
#define USER_KEY_GPIO_CLK              RCU_GPIOC
#define USER_KEY_EXTI_LINE             EXTI_13
#define USER_KEY_EXTI_PORT_SOURCE      GPIO_PORT_SOURCE_GPIOC
#define USER_KEY_EXTI_PIN_SOURCE       GPIO_PIN_SOURCE_13
#define USER_KEY_EXTI_IRQn             EXTI10_15_IRQn

#define DEBUG_COM                        USART1
#define DEBUG_COM_CLK                    RCU_USART1
#define DEBUG_COM_TX_PIN                 GPIO_PIN_2
#define DEBUG_COM_RX_PIN                 GPIO_PIN_3
#define DEBUG_COM_GPIO_PORT              GPIOA
#define DEBUG_COM_GPIO_CLK               RCU_GPIOA

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      5


/* function declaration */
void Gpio_Init(void);
void Spi_Init(void);
void soft_spi_init(void);
void Usart_Init(void);

void Board_SetSpiMode(bool use_hw_spi);
bool Board_GetSpiMode(void);

void LED_On(GPIO_Index_TYPE led);
void LED_Off(GPIO_Index_TYPE led);

// void EXTI4_IRQHandler();
// void EXTI10_15_IRQHandler();

void Sw1179_To_Tx(void);
void Sw1179_To_Rx(void);
void PA30dbm_To_Tx(void);
void PA30dbm_To_Rx(void);

void nvic_configuration(void);
void rtc_configuration(void);
void time_set(void);
void rtc_set_alarm(uint32_t ms);
void rtc_reset_alarm(void);
void iwdg_config(void);
void iwdg_feed(void);

// 2025/09/23
void tx_by_pa_sw_config(void);
void tx_bypass_pa_sw_config(void);
void rx_sw_config(void);
void sleep_sw_config(void);
void por_sw_config(void);

#endif

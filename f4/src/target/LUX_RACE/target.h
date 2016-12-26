/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "LUX"

#define LED0 PC15
#define LED1 PC14
#define LED2 PC13

#define BEEPER PB13
#define BEEPER_INVERTED

#define MPU9250_CS_PIN                   PA4
#define MPU9250_SPI_INSTANCE             SPI1

#define USE_EXTI
#define MPU_INT_EXTI PA5
 
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PB3
#define SPI1_MISO_PIN           PB4
#define SPI1_MOSI_PIN           PB5

#define USABLE_TIMER_CHANNEL_COUNT 11


#define ACC
#define USE_ACC_MPU9250
#define USE_ACC_SPI_MPU9250
#define ACC_MPU9250_ALIGN CW270_DEG

#define GYRO
#define USE_GYRO_MPU9250
#define USE_GYRO_SPI_MPU9250
#define GYRO_MPU9250_ALIGN CW270_DEG

// MPU6500 interrupt
//#define DEBUG_MPU_DATA_READY_INTERRUPT
#define USE_MPU_DATA_READY_SIGNAL
#define ENSURE_MPU_DATA_READY_IS_LOW

// External I2C BARO
//#define BARO
//#define USE_BARO_MS5611

// External I2C MAG
//#define MAG
//#define USE_MAG_HMC5883
//#define USE_MAG_AK8975
//#define MAG_AK8975_ALIGN CW0_DEG_FLIP

#define USE_VCP

#define USE_USART1
#define UART1_TX_PIN        GPIO_Pin_4
#define UART1_RX_PIN        GPIO_Pin_5
#define UART1_GPIO          GPIOC
#define UART1_GPIO_AF       GPIO_AF_7
#define UART1_TX_PINSOURCE  GPIO_PinSource4
#define UART1_RX_PINSOURCE  GPIO_PinSource5

#define USE_USART2
#define UART2_TX_PIN        GPIO_Pin_14 //PA14
#define UART2_RX_PIN        GPIO_Pin_15 //PA15
#define UART2_GPIO          GPIOA
#define UART2_GPIO_AF       GPIO_AF_7
#define UART2_TX_PINSOURCE  GPIO_PinSource14
#define UART2_RX_PINSOURCE  GPIO_PinSource15

#define USE_USART3
#define UART3_TX_PIN        GPIO_Pin_10
#define UART3_RX_PIN        GPIO_Pin_11
#define UART3_GPIO          GPIOB
#define UART3_GPIO_AF       GPIO_AF_7
#define UART3_TX_PINSOURCE  GPIO_PinSource10
#define UART3_RX_PINSOURCE  GPIO_PinSource11

#define SERIAL_PORT_COUNT 4

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_HARDWARE 0 // PWM 1

#define USE_I2C
#define I2C_DEVICE (I2CDEV_2)

#define I2C2_SCL         PA9
#define I2C2_SDA         PA10

#define USE_ADC

#define ADC_INSTANCE                ADC1
#define ADC_AHB_PERIPHERAL          RCC_AHBPeriph_DMA1
#define ADC_DMA_CHANNEL             DMA1_Channel1

#define VBAT_ADC_PIN                PC0
#define VBAT_ADC_CHANNEL            ADC_Channel_6

#define CURRENT_METER_ADC_PIN       PC1
#define CURRENT_METER_ADC_CHANNEL   ADC_Channel_7

#define RSSI_ADC_PIN                PC2
#define RSSI_ADC_CHANNEL            ADC_Channel_8

#define EXTERNAL1_ADC_PIN           PC3
#define EXTERNAL1_ADC_CHANNEL       ADC_Channel_9

#define BLACKBOX
#define GPS
#define GTUNE


#define LED_STRIP

#define LED_STRIP_TIMER TIM16

#define USE_LED_STRIP_ON_DMA1_CHANNEL3
#define WS2811_GPIO                     GPIOA
#define WS2811_GPIO_AHB_PERIPHERAL      RCC_AHBPeriph_GPIOA
#define WS2811_GPIO_AF                  GPIO_AF_1
#define WS2811_PIN                      GPIO_Pin_6 // TIM16_CH1
#define WS2811_PIN_SOURCE               GPIO_PinSource6
#define WS2811_TIMER                    TIM16
#define WS2811_TIMER_APB2_PERIPHERAL    RCC_APB2Periph_TIM16
#define WS2811_DMA_CHANNEL              DMA1_Channel3
#define WS2811_IRQ                      DMA1_Channel3_IRQn

#define TELEMETRY
#define SERIAL_RX
#define USE_SERVOS
#define USE_CLI

#define SPEKTRUM_BIND
// USART1
#define BIND_PORT  GPIOC
#define BIND_PIN   Pin_5

#define USE_SERIAL_1WIRE
// USART2, RX is on USART1
#define S1W_TX_GPIO         GPIOB
#define S1W_TX_PIN          GPIO_Pin_10
#define S1W_RX_GPIO         GPIOB
#define S1W_RX_PIN          GPIO_Pin_11

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff

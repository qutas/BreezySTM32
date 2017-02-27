/*
target.c - board-specific routines 

Copyright (C) 2017 Simon D. Levy 

This file is part of BreezySTM32.

BreezySTM32 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BreezySTM32 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BreezySTM32.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>

#include "platform.h"

#include "system.h"
#include "dma.h"
#include "gpio.h"
#include "timer.h"
#include "serial.h"
#include "serial_uart.h"
#include "exti.h"

#include "serial_usb_vcp.h"

GPIO_TypeDef * gpio_type_from_pin(uint8_t pin)
{
    (void)pin;
    return  GPIOB;
}

uint16_t gpio_pin_from_pin(uint8_t pin)
{
    (void)pin;
    return Pin_8;
}

serialPort_t * serial0_open(void)
{
    return usbVcpOpen();
}

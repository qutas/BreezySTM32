/*
   drv_pwm.h : PWM support for STM32F103

   Adapted from https://github.com/multiwii/baseflight/blob/master/src/drv_pwm.h

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

#pragma once

enum {
    PWM1 = 0,
    PWM2,
    PWM3,
    PWM4,
    PWM5,
    PWM6,
    PWM7,
    PWM8,
    PWM9,
    PWM10,
    PWM11,
    PWM12,
    PWM13,
    PWM14,
    MAX_PORTS
};


void pwmInit(uint8_t k, uint8_t port, uint32_t motorPwmRate, uint16_t idlePulseUsec);

void pwmWriteBrushed(uint8_t index, uint16_t value);

void pwmWriteStandard(uint8_t index, uint16_t value);

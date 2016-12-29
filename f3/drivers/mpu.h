/*
   mpu6050.h : driver for Invensense MPU6050

   Adapted from https://github.com/multiwii/baseflight/blob/master/src/drv_mpu.h

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
#include <math.h>

// 16.4 dps/lsb scalefactor for all Invensense devices
#define MPU_GYRO_SCALE ((1.0f / 16.4f) * ((float)M_PI / 180.0f))

// Gyro scale constants
typedef enum gyro_fsr_e {
    INV_FSR_250DPS = 0,
    INV_FSR_500DPS,
    INV_FSR_1000DPS,
    INV_FSR_2000DPS,
    NUM_GYRO_FSR
} gyro_fsr_e;

// Accelerometer constants
typedef enum accel_fsr_e {
    INV_FSR_2G = 0,
    INV_FSR_4G,
    INV_FSR_8G,
    INV_FSR_16G,
    NUM_ACCEL_FSR
} accel_fsr_e;

void mpu6500_init(accel_fsr_e accelFSR, gyro_fsr_e gyroFSR);

// Blocking Read Functions
void mpu_read_accel(int16_t *accData);
void mpu_read_gyro(int16_t *gyroData);

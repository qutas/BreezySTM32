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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <platform.h>

#include "build/build_config.h"

#include "common/utils.h"
#include "common/streambuf.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/dma.h"
#include "drivers/serial.h"
#include "drivers/system.h"

#if defined(USE_SOFTSERIAL1) || defined(USE_SOFTSERIAL2)
#include "drivers/serial_softserial.h"
#endif
#include "drivers/gpio.h"
#include "drivers/light_led.h"

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3) || defined(USE_UART4) || defined(USE_UART5)
#include "drivers/serial_uart.h"
#endif

#if defined(USE_VCP)
#include "drivers/serial_usb_vcp.h"
#endif

#include "io/beeper.h"
#include "io/serial.h"

#include "fc/config.h"


#ifdef TELEMETRY
#include "telemetry/telemetry.h"
#endif

static serialPortUsage_t serialPortUsageList[SERIAL_PORT_COUNT];

const serialPortIdentifier_e serialPortIdentifiers[SERIAL_PORT_COUNT] = {
#ifdef USE_VCP
    SERIAL_PORT_USB_VCP,
#endif
#ifdef USE_UART1
    SERIAL_PORT_UART1,
#endif
#ifdef USE_UART2
    SERIAL_PORT_UART2,
#endif
#ifdef USE_UART3
    SERIAL_PORT_UART3,
#endif
#ifdef USE_UART4
    SERIAL_PORT_UART4,
#endif
#ifdef USE_UART5
    SERIAL_PORT_UART5,
#endif
#ifdef USE_SOFTSERIAL1
    SERIAL_PORT_SOFTSERIAL1,
#endif
#ifdef USE_SOFTSERIAL2
    SERIAL_PORT_SOFTSERIAL2,
#endif
};

STATIC_UNIT_TESTED uint8_t serialPortCount;

const uint32_t baudRates[] = {0, 9600, 19200, 38400, 57600, 115200, 230400, 250000}; // see baudRate_e

#define BAUD_RATE_COUNT (sizeof(baudRates) / sizeof(baudRates[0]))

baudRate_e lookupBaudRateIndex(uint32_t baudRate)
{
    uint8_t index;

    for (index = 0; index < BAUD_RATE_COUNT; index++) {
        if (baudRates[index] == baudRate) {
            return index;
        }
    }
    return BAUD_AUTO;
}

serialPortUsage_t *findSerialPortUsageByIdentifier(serialPortIdentifier_e identifier)
{
    uint8_t index;
    for (index = 0; index < SERIAL_PORT_COUNT; index++) {
        serialPortUsage_t *candidate = &serialPortUsageList[index];
        if (candidate->identifier == identifier) {
            return candidate;
        }
    }
    return NULL;
}

serialPortUsage_t *findSerialPortUsageByPort(serialPort_t *serialPort)
{
    uint8_t index;
    for (index = 0; index < SERIAL_PORT_COUNT; index++) {
        serialPortUsage_t *candidate = &serialPortUsageList[index];
        if (candidate->serialPort == serialPort) {
            return candidate;
        }
    }
    return NULL;
}

typedef struct findSerialPortConfigState_s {
    uint8_t lastIndex;
} findSerialPortConfigState_t;

static findSerialPortConfigState_t findSerialPortConfigState;

serialPortConfig_t *findSerialPortConfig(uint16_t mask)
{
    memset(&findSerialPortConfigState, 0, sizeof(findSerialPortConfigState));

    return findNextSerialPortConfig(mask);
}

serialPortConfig_t *findNextSerialPortConfig(uint16_t mask)
{
    while (findSerialPortConfigState.lastIndex < SERIAL_PORT_COUNT) {
        serialPortConfig_t *candidate = &serialConfig()->portConfigs[findSerialPortConfigState.lastIndex++];

        if (candidate->functionMask & mask) {
            return candidate;
        }
    }
    return NULL;
}

typedef struct findSharedSerialPortState_s {
    uint8_t lastIndex;
} findSharedSerialPortState_t;

portSharing_e determinePortSharing(serialPortConfig_t *portConfig, serialPortFunction_e function)
{
    if (!portConfig || (portConfig->functionMask & function) == 0) {
        return PORTSHARING_UNUSED;
    }
    return portConfig->functionMask == function ? PORTSHARING_NOT_SHARED : PORTSHARING_SHARED;
}

bool isSerialPortShared(serialPortConfig_t *portConfig, uint16_t functionMask, serialPortFunction_e sharedWithFunction)
{
    return (portConfig) && (portConfig->functionMask & sharedWithFunction) && (portConfig->functionMask & functionMask);
}

bool isSerialPortOpen(serialPortConfig_t *portConfig)
{
    serialPortUsage_t *serialPortUsage = findSerialPortUsageByIdentifier(portConfig->identifier);
    return serialPortUsage && serialPortUsage->function != FUNCTION_NONE;
}


static findSharedSerialPortState_t findSharedSerialPortState;

serialPort_t *findSharedSerialPort(uint16_t functionMask, serialPortFunction_e sharedWithFunction)
{
    memset(&findSharedSerialPortState, 0, sizeof(findSharedSerialPortState));

    return findNextSharedSerialPort(functionMask, sharedWithFunction);
}

serialPort_t *findNextSharedSerialPort(uint16_t functionMask, serialPortFunction_e sharedWithFunction)
{
    while (findSharedSerialPortState.lastIndex < SERIAL_PORT_COUNT) {
        serialPortConfig_t *candidate = &serialConfig()->portConfigs[findSharedSerialPortState.lastIndex++];

        if (isSerialPortShared(candidate, functionMask, sharedWithFunction)) {
            serialPortUsage_t *serialPortUsage = findSerialPortUsageByIdentifier(candidate->identifier);
            if (!serialPortUsage) {
                continue;
            }
            return serialPortUsage->serialPort;
        }
    }
    return NULL;
}

#define ALL_TELEMETRY_FUNCTIONS_MASK (FUNCTION_TELEMETRY_FRSKY | FUNCTION_TELEMETRY_HOTT | FUNCTION_TELEMETRY_SMARTPORT | FUNCTION_TELEMETRY_LTM | FUNCTION_TELEMETRY_MAVLINK | FUNCTION_TELEMETRY_IBUS)
#define ALL_FUNCTIONS_SHARABLE_WITH_MSP_SERVER (FUNCTION_BLACKBOX | ALL_TELEMETRY_FUNCTIONS_MASK)

bool isSerialConfigValid(serialConfig_t *serialConfigToCheck)
{
    UNUSED(serialConfigToCheck);
    /*
     * rules:
     * - 1 MSP port minimum, max MSP ports is defined and must be adhered to.
     * - Only MSP SERVER is allowed to be shared with EITHER any telemetry OR blackbox.
     * - No other sharing combinations are valid.
     */
    uint8_t mspPortCount = 0;

    uint8_t index;
    for (index = 0; index < SERIAL_PORT_COUNT; index++) {
        serialPortConfig_t *portConfig = &serialConfigToCheck->portConfigs[index];

        if (portConfig->functionMask & (FUNCTION_MSP_SERVER | FUNCTION_MSP_CLIENT)) {
            mspPortCount++;
        }

        uint8_t bitCount = BITCOUNT(portConfig->functionMask);
        if (bitCount > 1) {
            // shared
            if (bitCount > 2) {
                return false;
            }

            if (!(portConfig->functionMask & FUNCTION_MSP_SERVER)) {
                return false;
            }

            if (!(portConfig->functionMask & ALL_FUNCTIONS_SHARABLE_WITH_MSP_SERVER)) {
                // some other bit must have been set.
                return false;
            }
        }
    }

    return true;
}

serialPortConfig_t *serialFindPortConfiguration(serialPortIdentifier_e identifier)
{
    uint8_t index;
    for (index = 0; index < SERIAL_PORT_COUNT; index++) {
        serialPortConfig_t *candidate = &serialConfig()->portConfigs[index];
        if (candidate->identifier == identifier) {
            return candidate;
        }
    }
    return NULL;
}

bool doesConfigurationUsePort(serialPortIdentifier_e identifier)
{
    serialPortConfig_t *candidate = serialFindPortConfiguration(identifier);
    return candidate != NULL && candidate->functionMask;
}

serialPort_t *openSerialPort(
    serialPortIdentifier_e identifier,
    serialPortFunction_e function,
    serialReceiveCallbackPtr callback,
    uint32_t baudRate,
    portMode_t mode,
    portOptions_t options)
{
#if (!defined(USE_VCP) && !defined(USE_UART1) && !defined(USE_UART2) && !defined(USE_UART3) && !defined(USE_UART4) && !defined(USE_UART5) && !defined(USE_SOFTSERIAL1) && !defined(USE_SOFTSERIAL2))
    UNUSED(callback);
    UNUSED(baudRate);
    UNUSED(mode);
    UNUSED(options);
#endif

    serialPortUsage_t *serialPortUsage = findSerialPortUsageByIdentifier(identifier);
    if (!serialPortUsage || serialPortUsage->function != FUNCTION_NONE) {
        // not available / already in use
        return NULL;
    }

    serialPort_t *serialPort = NULL;

    switch(identifier) {
#ifdef USE_VCP
        case SERIAL_PORT_USB_VCP:
            serialPort = usbVcpOpen();
            break;
#endif
#ifdef USE_UART1
        case SERIAL_PORT_UART1:
            serialPort = uartOpen(USART1, callback, baudRate, mode, options);
            break;
#endif
#ifdef USE_UART2
        case SERIAL_PORT_UART2:
            serialPort = uartOpen(USART2, callback, baudRate, mode, options);
            break;
#endif
#ifdef USE_UART3
        case SERIAL_PORT_UART3:
            serialPort = uartOpen(USART3, callback, baudRate, mode, options);
            break;
#endif
#ifdef USE_UART4
        case SERIAL_PORT_UART4:
            serialPort = uartOpen(UART4, callback, baudRate, mode, options);
            break;
#endif
#ifdef USE_UART5
        case SERIAL_PORT_UART5:
            serialPort = uartOpen(UART5, callback, baudRate, mode, options);
            break;
#endif
#ifdef USE_SOFTSERIAL1
        case SERIAL_PORT_SOFTSERIAL1:
            serialPort = openSoftSerial(SOFTSERIAL1, callback, baudRate, options);
            serialSetMode(serialPort, mode);
            break;
#endif
#ifdef USE_SOFTSERIAL2
        case SERIAL_PORT_SOFTSERIAL2:
            serialPort = openSoftSerial(SOFTSERIAL2, callback, baudRate, options);
            serialSetMode(serialPort, mode);
            break;
#endif
        default:
            break;
    }

    if (!serialPort) {
        return NULL;
    }

    serialPort->identifier = identifier;

    serialPortUsage->function = function;
    serialPortUsage->serialPort = serialPort;

    return serialPort;
}

void closeSerialPort(serialPort_t *serialPort)
{
    serialPortUsage_t *serialPortUsage = findSerialPortUsageByPort(serialPort);
    if (!serialPortUsage) {
        // already closed
        return;
    }

    // TODO wait until data has been transmitted.

    serialPort->callback = NULL;

    serialPortUsage->function = FUNCTION_NONE;
    serialPortUsage->serialPort = NULL;
}

void serialInit(bool softserialEnabled)
{
    uint8_t index;

    serialPortCount = SERIAL_PORT_COUNT;
    memset(&serialPortUsageList, 0, sizeof(serialPortUsageList));

    for (index = 0; index < SERIAL_PORT_COUNT; index++) {
        serialPortUsageList[index].identifier = serialPortIdentifiers[index];

        if (!softserialEnabled) {
            if (0
#ifdef USE_SOFTSERIAL1
                || serialPortUsageList[index].identifier == SERIAL_PORT_SOFTSERIAL1
#endif
#ifdef USE_SOFTSERIAL2
                || serialPortUsageList[index].identifier == SERIAL_PORT_SOFTSERIAL2
#endif
            ) {
                serialPortUsageList[index].identifier = SERIAL_PORT_NONE;
                serialPortCount--;
            }
        }
    }
}

void serialRemovePort(serialPortIdentifier_e identifier)
{
    for (uint8_t index = 0; index < SERIAL_PORT_COUNT; index++) {
        if (serialPortUsageList[index].identifier == identifier) {
            serialPortUsageList[index].identifier = SERIAL_PORT_NONE;
            serialPortCount--;
        }
    }
}

uint8_t serialGetAvailablePortCount(void)
{
    return serialPortCount;
}

bool serialIsPortAvailable(serialPortIdentifier_e identifier)
{
    for (uint8_t index = 0; index < SERIAL_PORT_COUNT; index++) {
        if (serialPortUsageList[index].identifier == identifier) {
            return true;
        }
    }
    return false;
}

void waitForSerialPortToFinishTransmitting(serialPort_t *serialPort)
{
    while (!isSerialTransmitBufferEmpty(serialPort)) {
        delay(10);
    };
}

#if defined(GPS) || ! defined(SKIP_SERIAL_PASSTHROUGH)
// Default data consumer for serialPassThrough.
static void nopConsumer(uint8_t data)
{
    UNUSED(data);
}

/*
A high-level serial passthrough implementation. Used by cli to start an
arbitrary serial passthrough "proxy". Optional callbacks can be given to allow
for specialized data processing.
*/
void serialPassthrough(serialPort_t *left, serialPort_t *right, serialConsumer 
                       *leftC, serialConsumer *rightC)
{
#ifdef BEEPER
    // fix for buzzer beeping continuously when CPU is in endless loop
    beeperSilence();
#endif

    waitForSerialPortToFinishTransmitting(left);
    waitForSerialPortToFinishTransmitting(right);

    if (!leftC)
        leftC = &nopConsumer;
    if (!rightC)
        rightC = &nopConsumer;

    LED0_OFF;
    LED1_OFF;

    // 3 position ring buffer. Used to implement a `+++` escape sequence for
    // exiting passthrough mode.
    // https://en.wikipedia.org/wiki/Escape_sequence#Modem_control
    uint8_t last3[3];
    int pos = 0, bpos = -1;

    // Either port might be open in a mode other than MODE_RXTX. We rely on
    // serialRxBytesWaiting() to do the right thing for a TX only port. No
    // special handling is necessary OR performed.
    while(1) {
        // We do special handling of left -> right since left is likely our PC
        // connection and right is usually a peripheral. We need to look for an
        // escape sequence only from the PC connection.
        if (serialRxBytesWaiting(left)) {
            LED0_ON;
            last3[pos] = serialRead(left);
            if (last3[pos] == '+') {
                // We just received a +, so start or continue buffering.
                if (bpos == -1)
                    bpos = pos;
                if (last3[0] == '+' &&
                    last3[1] == '+' &&
                    last3[2] == '+') {
                    // We have received a possible escape sequence.
                    int exit = 0;
                    uint32_t ts = millis();
                    while (1) {
                        delay(1);
                        if (serialRxBytesWaiting(left)) {
                            break;
                        }
                        if (millis() - ts >= 1000) {
                            // We passed our guard time. We can exit passthrough
                            // mode.
                            exit = 1;
                            break;
                        }
                    }
                    if (exit) {
                        // Break from passthrough mode.
                        LED0_OFF;
                        break;
                    } else {
                        // We did not pass our guard time. Dump one char from
                        // the buffer.
                        serialWrite(right, last3[bpos]);
                        leftC(last3[bpos]);
                        // Continue buffering, advance bpos.
                        if (++bpos > 2)
                            bpos = 0;
                    }
                }
            } else if (bpos == -1) {
                // We received a regular char, we are not buffering, so pass it
                // through.
                serialWrite(right, last3[pos]);
                leftC(last3[pos]);
            } else {
                // We received a regular char, we are buffering, so dump the
                // whole buffer.
                while (1) {
                    if (++bpos > 2)
                        bpos = 0;
                    serialWrite(right, last3[bpos]);
                    leftC(last3[bpos]);
                    if (bpos == pos)
                        break;
                }
                // Stop buffering.
                bpos = -1;
            }
            // Advance our buffer.
            if (++pos > 2)
                pos = 0;
            LED0_OFF;
        }
        if (serialRxBytesWaiting(right)) {
            LED0_ON;
            uint8_t c = serialRead(right);
            serialWrite(left, c);
            rightC(c);
            LED0_OFF;
        }
    }
}
#endif

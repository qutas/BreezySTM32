#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"
#include "system.h"
#include "gpio.h"
#include "io.h"
#include "serial.h"
#include "timer.h"
#include "serial_usb_vcp.h"
#include "exti.h"
#include "ioserial.h"
#include "light_led.h"

extern void setup(void), loop(void);

serialPort_t * Serial1;

extern void SetSysClock(void);

#ifndef EXTERNAL_DEBUG
void debug(const char * fmt, ...)
{
    va_list ap;       

    va_start(ap, fmt);     

    char buf[1000];

    vsprintf(buf, fmt, ap);

    for (char * p = buf; *p; p++)
        serialWrite(Serial1, *p);

    va_end(ap);  
    
    while (!isSerialTransmitBufferEmpty(Serial1));
}
#endif

int main(void)
{


    SCB->CPACR = (0x3 << (10*2)) | (0x3 << (11*2)); // start FPU
    SetSysClock();

    systemInit();

    IOInitGlobal();

    Serial1 = usbVcpOpen();

    ledInit(false);

    setup();

    while (true) {

#ifndef EXTERNAL_DEBUG
        // support reboot from host computer
        while (serialRxBytesWaiting(Serial1)) {
            uint8_t c = serialRead(Serial1);
            if (c == 'R') 
                systemResetToBootloader();
         }
#endif

        loop();
    }
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "can.h"
#include "fault.h"
#include "heater_control.h"
#include "pump_control.h"
#include "pump_dac.h"
#include "sampling.h"

static const UARTConfig uartCfg =
{
    .txend1_cb = nullptr,
    .txend2_cb = nullptr,
    .rxend_cb = nullptr,
    .rxchar_cb = nullptr,
    .rxerr_cb = nullptr,
    .timeout_cb = nullptr,

    .timeout = 0,
    .speed = 500000,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
};

char strBuffer[200];

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads
    StartSampling();
    InitPumpDac();
    StartHeaterControl();
    StartPumpControl();

    uartStart(&UARTD1, &uartCfg);

    InitCan();

    while(true)
    {
        auto fault = getCurrentFault();

        switch (fault)
        {
            case Fault::None:
                // blue is off
                palClearPad(GPIOB, 5);

                // Green is blinking
                palTogglePad(GPIOB, 6);

                // Fast blink if closed loop, slow if not
                chThdSleepMilliseconds(IsRunningClosedLoop() ? 50 : 400);
                break;
            case Fault::SensorDidntHeat:
                // Blue is blinking
                palTogglePad(GPIOB, 5);

                // green is off
                palClearPad(GPIOB, 6);

                // fast blink
                chThdSleepMilliseconds(50);
                break;
        }
    }
}

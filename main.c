/* Copyright (c) Codethink Ltd. All rights reserved.
   Licensed under the MIT License. */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "lib/CPUFreq.h"
#include "lib/VectorTable.h"
#include "lib/NVIC.h"
#include "lib/GPIO.h"
#include "lib/mt3620/gpio.h"
#include "lib/GPT.h"
#include "lib/UART.h"
#include "lib/Print.h"
#include "dmesg.h"
#include "lib/GPT.h"


// period = on_period + off_period
// on_period = on_value * clock_tick
// off_period = off_value * clock_tick

// #define USE_2M_SOURCE
#define USE_32K_SOURCE

// #define BASIC_SAMPLE

const uint32_t notes_us[8] ={
    3822, // c
    3405, // d
    3033, // e
    2863, // f
    2551, // g
    2272, // a
    2024, // b
    1911, // c
};

#ifdef USE_32K_SOURCE



#ifdef BASIC_SAMPLE
_Noreturn void RTCoreMain(void)
{
    VectorTableInit();
    CPUFreq_Set(26000000);

    uint32_t base_frequency = MT3620_PWM_32k;
    uint32_t tick_frequency = 1000000/base_frequency; // each tick is roughly 31

    uint32_t on_period = 1000; // us
    uint32_t off_period = 1000; // us

    uint32_t scaled_on = on_period / tick_frequency;
    uint32_t scaled_off = off_period / tick_frequency;

    PWM_ConfigurePin(0, MT3620_PWM_32k, scaled_on, scaled_off);
    while(1);

}
#else

_Noreturn void RTCoreMain(void)
{
    VectorTableInit();
    CPUFreq_Set(26000000);

    GPT *timer = GPT_Open(MT3620_UNIT_GPT1, 32768, GPT_MODE_REPEAT);

    uint32_t base_frequency = MT3620_PWM_32k;
    uint32_t tick_frequency = 1000000/base_frequency; // each tick is roughly 31

    while(1) {
        for (int i = 0; i < 8; i++) {
            uint32_t scaled_ticks = (notes_us[i] / 2) / tick_frequency;
            PWM_ConfigurePin(0, MT3620_PWM_32k, scaled_ticks, scaled_ticks);
            GPT_WaitTimer_Blocking(timer, 500, GPT_UNITS_MILLISEC);
        }

        for (int i = 7; i >= 0; i--) {
            uint32_t scaled_ticks = (notes_us[i] / 2) / tick_frequency;
            PWM_ConfigurePin(0, MT3620_PWM_32k, scaled_ticks, scaled_ticks);
            GPT_WaitTimer_Blocking(timer, 500, GPT_UNITS_MILLISEC);
        }
    }

}
#endif
#endif

#ifdef USE_2M_SOURCE

#ifdef BASIC_SAMPLE
_Noreturn void RTCoreMain(void)
{
    VectorTableInit();
    CPUFreq_Set(26000000);

    uint32_t base_frequency = MT3620_PWM_2M;
    uint32_t tick_frequency_ns = 1000000000/base_frequency; // each tick is roughly 31

    uint32_t on_period = 1000; // us
    uint32_t off_period = 1000; // us

    uint32_t scaled_on = (on_period * 1000) / tick_frequency_ns; // multiply on_period by 1000 to scale to nanoseconds.
    uint32_t scaled_off = (off_period * 1000) / tick_frequency_ns; // multiply off_period by 1000 to scale to nanoseconds.

    PWM_ConfigurePin(0, MT3620_PWM_2M, scaled_on, scaled_off);
    while(1);
}
#else



_Noreturn void RTCoreMain(void)
{
    VectorTableInit();
    CPUFreq_Set(26000000);

    GPT *timer = GPT_Open(MT3620_UNIT_GPT1, 32768, GPT_MODE_REPEAT);

    uint32_t base_frequency = MT3620_PWM_2M;
    uint32_t tick_frequency_ns = 1000000000/base_frequency; // each tick is roughly 31

    while(1) {
        for (int i = 0; i < 8; i++) {
            uint32_t scaled_ticks = (notes_us[i] / 2 * 1000) / tick_frequency_ns;
            PWM_ConfigurePin(0, MT3620_PWM_2M, scaled_ticks, scaled_ticks);
            GPT_WaitTimer_Blocking(timer, 500, GPT_UNITS_MILLISEC);
        }

        for (int i = 7; i >= 0; i--) {
            uint32_t scaled_ticks = (notes_us[i] / 2 * 1000) / tick_frequency_ns;
            PWM_ConfigurePin(0, MT3620_PWM_2M, scaled_ticks, scaled_ticks);
            GPT_WaitTimer_Blocking(timer, 500, GPT_UNITS_MILLISEC);
        }
    }
}
#endif
#endif
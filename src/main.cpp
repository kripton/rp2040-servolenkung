extern "C" {
#include <stdio.h>
#include <stdint.h>
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "pico/stdlib.h"

#include "pins.h"
#include "picotool_binary_information.h"

}

#include "log.h"

// Super-globals (for all modules)
Log logger;

double outFreq = 0;

int main() {

    // Underclock the board to 50MHz for lower PWM frequencies
    set_sys_clock_khz(50000, true);

    stdio_init_all();

    // /!\ Do NOT use LOG() above this line! /!\

    logger.init();

    // Set up all LEDs as PWMs and outputs
    gpio_init(PIN_LED_A);
    gpio_init(PIN_LED_B);
    gpio_init(PIN_LED_C);
    gpio_init(PIN_LED_D);
    gpio_init(PIN_LED_E);
    gpio_init(PIN_LED_F);
    gpio_init(PIN_LED_G);
    gpio_init(PIN_LED_H);
    gpio_set_dir(PIN_LED_A, true);
    gpio_set_dir(PIN_LED_B, true);
    gpio_set_dir(PIN_LED_C, true);
    gpio_set_dir(PIN_LED_D, true);
    gpio_set_dir(PIN_LED_E, true);
    gpio_set_dir(PIN_LED_F, true);
    gpio_set_dir(PIN_LED_G, true);
    gpio_set_dir(PIN_LED_H, true);


    // Set up the PWM for the output
    pwm_config config = pwm_get_default_config();
    // "PWM config is free-running at system clock speed, no phase correction, wrapping at 0xffff, with standard polarities for channels A and B."
    pwm_config_set_clkdiv(&config, 4.f); // 250 MHz / 4 = 62.5MHz
                                         // Wrapping at 0xffff => 953,6888685435264 Hz
    pwm_init(0, &config, false);
    pwm_set_gpio_level(PIN_SIG_OUT, 0);
    gpio_set_function(PIN_SIG_OUT, GPIO_FUNC_PWM);

    // For DEBUG: On-Board LED
    config = pwm_get_default_config();
    pwm_init(4, &config, false);
    pwm_set_gpio_level(PIN_LED_PICO, 0);
    gpio_set_function(PIN_LED_PICO, GPIO_FUNC_PWM);

    gpio_put(PIN_LED_A, 1);

    // SETUP COMPLETE
    LOG("SYSTEM: SETUP COMPLETE");

    while (true) {
        outFreq += 1.0;
        if (outFreq > 255) {
            outFreq = 0.0;
        }

        // 50% duty cycle, variable frequency
        pwm_set_gpio_level(PIN_SIG_OUT, 32768);
        // Calculate and set the correct frequency of the PWM
        float divider = (double)clock_get_hz(clk_sys) / (outFreq * 65536.0f);
        double realFreq = (double)clock_get_hz(clk_sys) / (65536.0f * divider);
        LOG("IDEAL OutFreq: %f Hz. Divider: %f. REAL OutFreq: %f Hz", outFreq, divider, realFreq);
        pwm_set_clkdiv(0, divider);
        pwm_set_enabled(0, true);

        // DEBUG: PWM outFreq as duty cyclce (fixed freq) on PICO LED
        pwm_set_gpio_level(PIN_LED_PICO, outFreq * 255);
        pwm_set_enabled(4, true);

        // DEBUG: outFreq as bits on LEDs
        uint8_t iOutFreq = outFreq;
        gpio_put(PIN_LED_A, ((iOutFreq >> 0) & 0x01));
        gpio_put(PIN_LED_B, ((iOutFreq >> 1) & 0x01));
        gpio_put(PIN_LED_C, ((iOutFreq >> 2) & 0x01));
        gpio_put(PIN_LED_D, ((iOutFreq >> 3) & 0x01));
        gpio_put(PIN_LED_E, ((iOutFreq >> 4) & 0x01));
        gpio_put(PIN_LED_F, ((iOutFreq >> 5) & 0x01));
        gpio_put(PIN_LED_G, ((iOutFreq >> 6) & 0x01));
        gpio_put(PIN_LED_H, ((iOutFreq >> 7) & 0x01));
        LOG("LEDs: %d %d %d %d %d %d %d %d",
            gpio_get(PIN_LED_A),
            gpio_get(PIN_LED_B),
            gpio_get(PIN_LED_C),
            gpio_get(PIN_LED_D),
            gpio_get(PIN_LED_E),
            gpio_get(PIN_LED_F),
            gpio_get(PIN_LED_G),
            gpio_get(PIN_LED_H)
        );

        // One cycle should be ~10s. With 255 steps, sleep 40ms
        sleep_ms(40);
    }
};

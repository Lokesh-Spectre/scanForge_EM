#pragma once
#include "driver/gpio.h"

typedef struct {
    gpio_num_t in1;
    gpio_num_t in2;
    gpio_num_t in3;
    gpio_num_t in4;
    int step_delay_us;      // microseconds per half-step
} stepper_config_t;

// Commands sent to the driver task
typedef struct {
    int steps;   // +N = forward, -N = backward
} stepper_cmd_t;

#pragma once
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdbool.h>
#include "stepper/common.h"

// Initialize stepper and create task pinned to given core
void stepper_init(const stepper_config_t *cfg, UBaseType_t core_id);

// Queue a move command
bool stepper_move(int steps);

// Set speed (µs per step)
void stepper_set_speed(int delay_us);
void stepper_wait_till_done(void);
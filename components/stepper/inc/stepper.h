#pragma once
#include "stepper/stepper.h"
#include "stepper/common.h"

// Public interface
typedef struct{
    // Methods
    void (*init)(const stepper_config_t *cfg,UBaseType_t core_id);
    bool (*move)(int steps);
    void (*set_speed)(int delay_us);
    void (*wait)();
}Stepper_t;

extern Stepper_t stepper;
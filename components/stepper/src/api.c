#include "stepper.h"
#include "stepper/stepper.h"

Stepper_t stepper = {
    .init = stepper_init,
    .move = stepper_move,
    .set_speed = stepper_set_speed,
    .wait = stepper_wait_till_done
};


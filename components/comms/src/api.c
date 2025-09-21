#include "comms.h"
#include "comms/comms.h"

comms_module_t comms = {
    .init = &comms_init,
    .activate = &comms_start_advertising,
    .send=&comms_send_event,
    .is_connected=&comms_is_connected
};
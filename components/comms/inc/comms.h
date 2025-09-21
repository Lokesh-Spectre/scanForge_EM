#pragma once

#include <stdint.h>
#include "esp_err.h"

#include "comms/common.h"
#include "comms/comms.h"
/**
 * @brief Module interface for BLE comms
 *
 * Provides init, advertising activation, and event sending methods.
 */
typedef struct {
    esp_err_t (*init)(comms_cmd_event_handler_t event_handler);   ///< Initialize BLE + register service
    esp_err_t (*activate)(void);                        ///< Start advertising
    esp_err_t (*send)(comms_event_t event);             ///< Send event to Central
} comms_module_t;

/**
 * @brief Get global comms module instance
 *
 * Example usage:
 *   comms.get.init(event_handler);
 *   comms.get.activate();
 *   comms.get.send(COMMS_EVENT_SHOOT);
 */
extern comms_module_t comms;

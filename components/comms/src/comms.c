#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"
// #include "constants.h"
#include "freertos/semphr.h"
#include "string.h"

#include "comms/ble_config.h"
#include "comms/common.h"
#include "comms/private.h"

SemaphoreHandle_t adv_ready_sem;
static bool adv_ready = false;

#define COMMS_COMMAND_BUFFER_LENGTH 20

cmd_buff_t cmd_buffer;

comms_cmd_event_handler_t comms_cmd_event_handler= NULL;

void comms_gatt_write_event_handler(cmd_buff_t buffer){
    if(strcmp(buffer.buffer,"START") == 0) comms_cmd_event_handler(COMMS_CMD_START);
    if(strcmp(buffer.buffer,"STOP") == 0) comms_cmd_event_handler(COMMS_CMD_STOP);
    if(strcmp(buffer.buffer,"NEXT") == 0) comms_cmd_event_handler(COMMS_CMD_NEXT);
    if(strcmp(buffer.buffer,"RESUME") == 0) comms_cmd_event_handler(COMMS_CMD_RESUME);    
}

esp_err_t comms_init(comms_cmd_event_handler_t event_handler){

    cmd_buffer.buffer_length = COMMS_COMMAND_BUFFER_LENGTH;
    cmd_buffer.buffer = (char*)malloc(COMMS_COMMAND_BUFFER_LENGTH);
    cmd_buffer.data_length = 0;

    comms_cmd_event_handler=event_handler;
    adv_ready_sem = xSemaphoreCreateBinary();

    // bluedroid ble stack init
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    // register callbacks
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(PROFILE_A_APP_ID));
    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(500));
    return ESP_OK;
}
esp_err_t comms_deinit(){
    free(cmd_buffer.buffer);
    // stop ble stack
    return ESP_OK;
}
esp_err_t comms_start_advertising(void){
    if (!adv_ready) {
        // Wait indefinitely
        if (xSemaphoreTake(adv_ready_sem, pdMS_TO_TICKS(1000) ) != pdTRUE) {
            // BLE stack is not configured properly
            return ESP_FAIL;
        }
    }
    
    return esp_ble_gap_start_advertising(&adv_params);
}
esp_err_t comms_send_event(comms_event_t event){return ESP_OK;}

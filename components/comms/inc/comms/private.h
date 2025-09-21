#pragma once

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

#include "comms/common.h"


typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

extern prepare_type_env_t prepare_write_buff;

typedef struct{
    int buffer_length;
    char* buffer;
    int data_length;
}cmd_buff_t;

extern cmd_buff_t cmd_buffer;
extern comms_cmd_event_handler_t comms_cmd_event_handler;
extern SemaphoreHandle_t adv_ready_sem;

// BLE handlers
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

// void gatts_write_handler(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
// void gatts_write_exec_handler(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){

// comms handlers

void comms_gatt_write_event_handler(cmd_buff_t buffer);

#include "comms/common.h"
#include "comms/private.h"
#include "comms/ble_config.h"
#include "string.h"
#include "stdbool.h"

static char* TAG = "SF_BLE_HANDLERS";
bool adv_ready;
prepare_type_env_t prepare_write_buff;

void gatts_write_handler(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp){
        if (param->write.is_prep) {
            if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_OFFSET;
            } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_ATTR_LEN;
            }
            if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(TAG, "Gatt_server prep no mem");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            if (gatt_rsp) {
                gatt_rsp->attr_value.len = param->write.len;
                gatt_rsp->attr_value.handle = param->write.handle;
                gatt_rsp->attr_value.offset = param->write.offset;
                gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
                esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
                if (response_err != ESP_OK){
                    ESP_LOGE(TAG, "Send response error\n");
                }
                free(gatt_rsp);
            } else {
                ESP_LOGE(TAG, "malloc failed, no resource to send response error\n");
                status = ESP_GATT_NO_RESOURCES;
            }
            if (status != ESP_GATT_OK){
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;

        }else{
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
    // prevent data overflow
    if(cmd_buffer.buffer_length-1 > param->write.len ){
        cmd_buffer.data_length = param->write.len;
    }else{
        cmd_buffer.data_length = cmd_buffer.buffer_length-1;
    }
    
    memcpy(cmd_buffer.buffer, param->write.value,cmd_buffer.data_length);
    
    // ensure proper string ending
    cmd_buffer.buffer[cmd_buffer.data_length] = '\0';
    ESP_LOGI(TAG,"RECEIVED TEXT: %s @ %d",cmd_buffer.buffer,param->write.handle);
    // call event handler
    comms_gatt_write_event_handler(cmd_buffer);
}

void gatts_write_exec_handler(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
        ESP_LOG_BUFFER_HEX(TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(TAG,"Prepare write cancel");
    }

    if (prepare_write_env->prepare_buf) {
    
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    switch (event) {

    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            // esp_ble_gap_start_advertising(&adv_params);
            adv_ready = true;   // ✅ advertising is configured, ready to be started manually
            xSemaphoreGive(adv_ready_sem);   // notify waiting task
            ESP_LOGI(TAG, "Advertising data configured, ready to start.");
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            // esp_ble_gap_start_advertising(&adv_params);
            adv_ready = true;   // ✅ advertising is configured, ready to be started manually
            xSemaphoreGive(adv_ready_sem);   // notify waiting task
            ESP_LOGI(TAG, "Advertising data configured, ready to start.");
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising start failed, status %d", param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "Advertising start successfully");
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising stop failed, status %d", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(TAG, "Advertising stop successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(TAG, "Connection params update, status %d, conn_int %d, latency %d, timeout %d",
                  param->update_conn_params.status,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        ESP_LOGI(TAG, "Packet length update, status %d, rx %d, tx %d",
                  param->pkt_data_length_cmpl.status,
                  param->pkt_data_length_cmpl.params.rx_len,
                  param->pkt_data_length_cmpl.params.tx_len);
        break;
    default:
        break;
    }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile.gatts_if = gatts_if;
        } else {
            ESP_LOGI(TAG, "Reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gatts_if == gl_profile.gatts_if) {
                if (gl_profile.gatts_cb) {
                    gl_profile.gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

// void gatts_service_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {}
void gatts_service_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
        
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG, "GATT server register, status %d, app_id %d, gatts_if %d", param->reg.status, param->reg.app_id, gatts_if);
            gl_profile.service_id.is_primary = true;
            gl_profile.service_id.id.inst_id = 0x00;
            gl_profile.service_id.id.uuid.len = ESP_UUID_LEN_128;
            
            memcpy(gl_profile.service_id.id.uuid.uuid.uuid128,
                    GATTS_SERVICE_UUID,
                    ESP_UUID_LEN_128);
            
            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(test_device_name);
            if (set_dev_name_ret){
                ESP_LOGE(TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }//config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= adv_config_flag;
            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= scan_rsp_config_flag;
            esp_ble_gatts_create_service(gatts_if, &gl_profile.service_id, GATTS_SERVICE_ATTR_NUM);
            break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(TAG, "Characteristic read, conn_id %d, trans_id %" PRIu32 ", handle %d", param->read.conn_id, param->read.trans_id, param->read.handle);
            esp_gatt_rsp_t rsp;
            memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
            rsp.attr_value.handle = param->read.handle;
            rsp.attr_value.len = 4;
            rsp.attr_value.value[0] = 0xde;
            rsp.attr_value.value[1] = 0xed;
            rsp.attr_value.value[2] = 0xbe;
            rsp.attr_value.value[3] = 0xef;
            esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                        ESP_GATT_OK, &rsp);
            break;
        
        case ESP_GATTS_WRITE_EVT: 
            ESP_LOGI(TAG, "Characteristic write, conn_id %d, trans_id %" PRIu32 ", handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
            if (!param->write.is_prep){
                ESP_LOGI(TAG, "value len %d, value ", param->write.len);
                ESP_LOG_BUFFER_HEX(TAG, param->write.value, param->write.len);
                if (gl_profile.descr_handle == param->write.handle && param->write.len == 2){
                    uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                    if (descr_value == 0x0001){
                        if (scanner_status_characteristic_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                            ESP_LOGI(TAG, "Notification enable");
                            uint8_t notify_data[15];
                            for (int i = 0; i < sizeof(notify_data); ++i)
                            {
                                notify_data[i] = i%0xff;
                            }
                            //the size of notify_data[] need less than MTU size
                            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile.char_handle,
                                                    sizeof(notify_data), notify_data, false);
                        }
                    }else if (descr_value == 0x0002){
                        if (scanner_status_characteristic_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                            ESP_LOGI(TAG, "Indication enable");
                            uint8_t indicate_data[15];
                            for (int i = 0; i < sizeof(indicate_data); ++i)
                            {
                                indicate_data[i] = i%0xff;
                            }
                            //the size of indicate_data[] need less than MTU size
                            esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile.char_handle,
                                                    sizeof(indicate_data), indicate_data, true);
                        }
                    }
                    else if (descr_value == 0x0000){
                        ESP_LOGI(TAG, "Notification/Indication disable");
                    }else{
                        ESP_LOGE(TAG, "Unknown descriptor value");
                        ESP_LOG_BUFFER_HEX(TAG, param->write.value, param->write.len);
                    }

                }
            }
            gatts_write_handler(gatts_if, &prepare_write_buff, param);
            break;
        case ESP_GATTS_EXEC_WRITE_EVT:
            ESP_LOGI(TAG,"Execute write");
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            gatts_write_exec_handler(&prepare_write_buff, param);
            break;
        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(TAG, "MTU exchange, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_UNREG_EVT:
            break;
        case ESP_GATTS_CREATE_EVT:
            ESP_LOGI(TAG, "Service create, status %d, service_handle %d", param->create.status, param->create.service_handle);
            gl_profile.service_handle = param->create.service_handle;
            
            gl_profile.char_uuid.len = ESP_UUID_LEN_16;
            gl_profile.char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_COMMAND_CONTROL;
            command_control_characteristic_property = ESP_GATT_CHAR_PROP_BIT_WRITE;
            
            gl_profile.char_uuid_2.len = ESP_UUID_LEN_16;
            gl_profile.char_uuid_2.uuid.uuid16 = GATTS_CHAR_UUID_SCANNING_STATUS;
            scanner_status_characteristic_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

            esp_ble_gatts_start_service(gl_profile.service_handle);
            
            esp_err_t add_char_ret =    esp_ble_gatts_add_char(gl_profile.service_handle, &gl_profile.char_uuid,
                                                            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                            command_control_characteristic_property,
                                                            &gatts_demo_char1_val, NULL);
            
            add_char_ret = esp_ble_gatts_add_char(gl_profile.service_handle, &gl_profile.char_uuid_2,
                                                            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                            scanner_status_characteristic_property,
                                                            &gatts_demo_char1_val, NULL);
            if (add_char_ret){
                ESP_LOGE(TAG, "add char failed, error code =%x",add_char_ret);
            }
            break;
        case ESP_GATTS_ADD_INCL_SRVC_EVT:
            break;
        case ESP_GATTS_ADD_CHAR_EVT:
            uint16_t length = 0;
            const uint8_t *prf_char;

            ESP_LOGI(TAG, "Characteristic add, status %d, attr_handle %d, service_handle %d",
                    param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
            gl_profile.char_handle = param->add_char.attr_handle;
            if (param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_COMMAND_CONTROL){
                gl_profile.descr_uuid.len = ESP_UUID_LEN_16;
                gl_profile.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
                esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
                esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile.service_handle, &gl_profile.descr_uuid,
                                                                    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);    
                if (add_descr_ret){
                    ESP_LOGE(TAG, "add char descr failed, error code =%x", add_descr_ret);
                }
            }else if(param->add_char.char_uuid.uuid.uuid16 == GATTS_CHAR_UUID_SCANNING_STATUS){
                gl_profile.descr_uuid_2.len = ESP_UUID_LEN_16;
                gl_profile.descr_uuid_2.uuid.uuid16 = 0xFACE;
                esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
                esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile.service_handle, &gl_profile.descr_uuid_2,
                                                                    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
                if (add_descr_ret){
                    ESP_LOGE(TAG, "add char descr failed, error code =%x", add_descr_ret);
                }
            }
            // if (get_attr_ret == ESP_FAIL){
            //     ESP_LOGE(TAG, "ILLEGAL HANDLE");
            // }

            ESP_LOGI(TAG, "the gatts demo char length = %x", length);
            for(int i = 0; i < length; i++){
                ESP_LOGI(TAG, "prf_char[%x] =%x",i,prf_char[i]);
            }
            break;
        case ESP_GATTS_ADD_CHAR_DESCR_EVT:
            gl_profile.descr_handle = param->add_char_descr.attr_handle;
            ESP_LOGI(TAG, "Descriptor add, status %d, attr_handle %d, service_handle %d",
                    param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
            break;
        case ESP_GATTS_DELETE_EVT:
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(TAG, "Service start, status %d, service_handle %d",
                    param->start.status, param->start.service_handle);
            break;
        case ESP_GATTS_STOP_EVT:
            break;
        case ESP_GATTS_CONNECT_EVT:
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            ESP_LOGI(TAG, "Connected, conn_id %u, remote "ESP_BD_ADDR_STR"",
                    param->connect.conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));
            gl_profile.conn_id = param->connect.conn_id;
            //start sent the update connection parameters to the peer device.
            esp_ble_gap_update_conn_params(&conn_params);
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(TAG, "Disconnected, remote "ESP_BD_ADDR_STR", reason 0x%02x",
                    ESP_BD_ADDR_HEX(param->disconnect.remote_bda), param->disconnect.reason);
            esp_ble_gap_start_advertising(&adv_params);
            break;
        case ESP_GATTS_CONF_EVT:
            ESP_LOGI(TAG, "Confirm receive, status %d, attr_handle %d", param->conf.status, param->conf.handle);
            if (param->conf.status != ESP_GATT_OK){
                ESP_LOG_BUFFER_HEX(TAG, param->conf.value, param->conf.len);
            }
            break;
        case ESP_GATTS_OPEN_EVT:
            break;
        case ESP_GATTS_CANCEL_OPEN_EVT:
            break;
        case ESP_GATTS_CLOSE_EVT:
            break;
        case ESP_GATTS_LISTEN_EVT:
            break;
        case ESP_GATTS_CONGEST_EVT:
            break;
        default:
            break;
    };
}
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"

#include "comms.h"
#include "stepper.h"

static char* TAG = "SF_APP";
static comms_cmd_t command = COMMS_CMD_STOP;

stepper_config_t stepper_cfg = {
    .in1 = GPIO_NUM_7,
    .in2 = GPIO_NUM_8,
    .in3 = GPIO_NUM_9,
    .in4 = GPIO_NUM_10,
    .step_delay_us = 1000
};
#define PHOTO_COUNT 10

void comms_event_handler(comms_cmd_t event){
    command = event;
    switch(event){
        case COMMS_CMD_NEXT:
            ESP_LOGI(TAG,"RECEIVED NEXT COMMAND");
            break;
        case COMMS_CMD_START:
            ESP_LOGI(TAG,"RECEIVED START COMMAND");
            break;
        case COMMS_CMD_STOP:
            ESP_LOGI(TAG,"RECEIVED STOP COMMAND");
            break;
        case COMMS_CMD_RESUME:
            ESP_LOGI(TAG,"RECEIVED RESUME COMMAND");
            break;
    }
}

void test_sequence(){
    while(1){
        if (command == COMMS_CMD_START){
            for (int i=0;i<10;i++){
                stepper.move(4096/PHOTO_COUNT);
                stepper.wait();
                comms.send(COMMS_EVENT_SHOOT);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            comms.send(COMMS_EVENT_END);
            command = COMMS_CMD_STOP;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        
    }
}
void app_main(){
    esp_err_t ret;
    
    stepper.init(&stepper_cfg,1);

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    
    comms.init(comms_event_handler);
    comms.activate();
    test_sequence();
}
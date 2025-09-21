#include "comms.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "freeRTOS/timers.h"
#include "freertos/FreeRTOS.h"
static char* TAG = "SF_APP";

void comms_event_handler(comms_cmd_t event){
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

void app_main(){
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    
    comms.init(comms_event_handler);
    comms.activate();
    int x =0;
    while(1){
        
        if(comms.is_connected()){
            switch (x%3){
                case 0:
                    comms.send(COMMS_EVENT_SHOOT);
                    break;
                case 1:
                    comms.send(COMMS_EVENT_END);
                    break;
                case 2:
                    comms.send(COMMS_EVENT_STOPPED);
                    break;
            }
            x++;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
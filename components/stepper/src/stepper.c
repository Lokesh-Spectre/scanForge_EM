#include "stepper/stepper.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

static const char *TAG = "STEPPER";

static const int step_sequence[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

static stepper_config_t motor_cfg;
static int step_index = 0;
static QueueHandle_t stepper_queue;
static SemaphoreHandle_t stepper_done_sem;  // signals completion
static int delay_us = 1000;   // default = 1ms/step

typedef struct {
    int steps;
    bool notify;
} stepper_cmd_ex_t;

static void stepper_apply(int idx) {
    gpio_set_level(motor_cfg.in1, step_sequence[idx][0]);
    gpio_set_level(motor_cfg.in2, step_sequence[idx][1]);
    gpio_set_level(motor_cfg.in3, step_sequence[idx][2]);
    gpio_set_level(motor_cfg.in4, step_sequence[idx][3]);
}

static void stepper_task(void *arg) {
    stepper_cmd_ex_t cmd;
    while (1) {
        if (xQueueReceive(stepper_queue, &cmd, portMAX_DELAY)) {
            int steps = cmd.steps;
            ESP_LOGI(TAG, "Executing %d steps", steps);

            if (steps > 0) {
                for (int i = 0; i < steps; i++) {
                    step_index = (step_index + 1) % 8;
                    stepper_apply(step_index);
                    esp_rom_delay_us(delay_us);
                }
            } else {
                for (int i = 0; i < -steps; i++) {
                    step_index = (step_index + 7) % 8; // backwards
                    stepper_apply(step_index);
                    esp_rom_delay_us(delay_us);
                }
            }
            ESP_LOGI(TAG,"Stepper motor motion completed");
            // release semaphore if this move requested notification
            if (cmd.notify && stepper_done_sem) {
                xSemaphoreGive(stepper_done_sem);
            }
        }
    }
}

void stepper_init(const stepper_config_t *cfg, UBaseType_t core_id) {
    motor_cfg = *cfg;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << cfg->in1) |
                        (1ULL << cfg->in2) |
                        (1ULL << cfg->in3) |
                        (1ULL << cfg->in4),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);

    stepper_queue = xQueueCreate(5, sizeof(stepper_cmd_ex_t));
    stepper_done_sem = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(stepper_task, "stepper_task", 2048, NULL, 5, NULL, core_id);

    ESP_LOGI(TAG, "Stepper initialized on core %d", core_id);
}

bool stepper_move(int steps) {
    if (!stepper_queue) return false;
    stepper_cmd_ex_t cmd = { .steps = steps, .notify = true };
    ESP_LOGI(TAG, "Stepper motor starts to move: %d steps", steps);
    // clear any old signals before new move
    xSemaphoreTake(stepper_done_sem, 0);
    return xQueueSend(stepper_queue, &cmd, 0) == pdTRUE;
}

void stepper_wait_till_done(void) {
    if (stepper_done_sem) {
        xSemaphoreTake(stepper_done_sem, portMAX_DELAY);
    }
}

void stepper_set_speed(int delay) {
    delay_us = delay;
}

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#include "ds3231.h"

#define SCL_GPIO		19
#define SDA_GPIO		18

static const char *TAG = "DS3213";

RTC_DATA_ATTR static int boot_count = 0;

void test(void *pvParameters)
{
    // Initialize RTC
    i2c_dev_t dev;
    if (ds3231_init_desc(&dev, 0, SDA_GPIO, SCL_GPIO) != ESP_OK) {
        ESP_LOGE(pcTaskGetTaskName(0), "Could not init device descriptor.");
        while (1) { vTaskDelay(1); }
    }

    // setup datetime: 2016-10-09 13:50:10
    struct tm time = {
        .tm_year = 116, //since 1900 (2016 - 1900)
        .tm_mon  = 9,  // 0-based
        .tm_mday = 9,
        .tm_hour = 13,
        .tm_min  = 50,
        .tm_sec  = 10
    };

    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_sec=%d",time.tm_sec);
    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_min=%d",time.tm_min);
    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_hour=%d",time.tm_hour);
    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_wday=%d",time.tm_wday);
    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_mday=%d",time.tm_mday);
    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_mon=%d",time.tm_mon);
    ESP_LOGD(pcTaskGetTaskName(0), "timeinfo.tm_year=%d",time.tm_year);

    if (ds3231_set_time(&dev, &time) != ESP_OK) {
        ESP_LOGE(pcTaskGetTaskName(0), "Could not set time.");
        while (1) { vTaskDelay(1); }
    }
    ESP_LOGI(pcTaskGetTaskName(0), "Set initial date time done");


    // Initialise the xLastWakeTime variable with the current time.
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Get RTC date and time
    while (1) {
        float temp;
        struct tm rtcinfo;

        if (ds3231_get_temp_float(&dev, &temp) != ESP_OK) {
            ESP_LOGE(pcTaskGetTaskName(0), "Could not get temperature.");
            while (1) { vTaskDelay(1); }
        }

        if (ds3231_get_time(&dev, &rtcinfo) != ESP_OK) {
            ESP_LOGE(pcTaskGetTaskName(0), "Could not get time.");
            while (1) { vTaskDelay(1); }
        }

        ESP_LOGI(pcTaskGetTaskName(0), "%04d-%02d-%02d %02d:%02d:%02d, %.2f deg Cel", 
            rtcinfo.tm_year, rtcinfo.tm_mon + 1,
            rtcinfo.tm_mday, rtcinfo.tm_hour, rtcinfo.tm_min, rtcinfo.tm_sec, temp);
	vTaskDelayUntil(&xLastWakeTime, 100);
    }

}

void app_main()
{
    ++boot_count;
    ESP_LOGI(TAG, "CONFIG_SCL_GPIO = %d", SCL_GPIO);
    ESP_LOGI(TAG, "CONFIG_SDA_GPIO = %d", SDA_GPIO);
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    xTaskCreate(test, "test", 1024*4, NULL, 2, NULL);
}


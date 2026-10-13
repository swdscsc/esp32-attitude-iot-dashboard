#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "led.h"
#include "qmi.h"
#include "wifi_sta.h"
#include "mqtt_app.h"

#define WIFI_CONNECTED_BIT  BIT0
static EventGroupHandle_t s_wifi_ev = NULL;
static const char *TAG = "main";

// WiFi 连接结果回调（由 WIFI 组件通知）
void wifi_event_handler(WIFI_EV_e ev)
{
    if (ev == WIFI_CONNECTED) {
        xEventGroupSetBits(s_wifi_ev, WIFI_CONNECTED_BIT);
    }
}

// 姿态上云任务：等 WiFi -> 启动 MQTT -> 每 500ms 采集并发布一帧
static void attitude_cloud_task(void *arg)
{
    // 1. 等 WiFi 连上
    xEventGroupWaitBits(s_wifi_ev, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi 已连接，启动 MQTT");

    // 2. 启动 MQTT 并等 Broker 握手完成
    mqtt_app_start();
    while (!mqtt_app_is_connected()) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    ESP_LOGI(TAG, "MQTT 已就绪，开始每 500ms 上报姿态");

    // 3. 采集 + 发布循环
    t_sQMI8658 value;
    while (1) {
        Qmi8658_Read_Acc(&value);              // 读原始加速度
        qmi8658_fetch_angleFromAcc(&value);    // 换算三轴倾角
        mqtt_app_publish_attitude(value.AngleX, value.AngleY, value.AngleZ);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    led_init();
    IIC_Init();
    Qmi8658_Init();
    nvs_flash_init();

    s_wifi_ev = xEventGroupCreate();
    wifi_sta_init(wifi_event_handler);

    xTaskCreate(attitude_cloud_task, "attitude_cloud", 4096, NULL, 5, NULL);
}

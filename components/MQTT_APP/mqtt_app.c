#include "mqtt_app.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

static const char *TAG = "mqtt_app";

// LED 控制脚（与 components/LED/led.h 保持一致）
#define LED_GPIO  GPIO_NUM_1

static esp_mqtt_client_handle_t s_client = NULL;
static bool s_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        s_connected = true;
        ESP_LOGI(TAG, "已连接 Broker，订阅命令主题: %s", MQTT_TOPIC_CMD);
        esp_mqtt_client_subscribe(s_client, MQTT_TOPIC_CMD, 1);
        esp_mqtt_client_publish(s_client, MQTT_TOPIC_PREFIX "/status", "online", 0, 1, 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        s_connected = false;
        ESP_LOGW(TAG, "与 Broker 断开，SDK 会自动重连");
        break;

    case MQTT_EVENT_DATA:
        // 收到下行命令：led_on / led_off（保留生命周期 1 的遗嘱/状态主题思路）
        ESP_LOGI(TAG, "收到命令: %.*s", event->data_len, event->data);
        if (event->data_len == 6 && strncmp(event->data, "led_on", 6) == 0) {
            gpio_set_level(LED_GPIO, 1);
        } else if (event->data_len == 7 && strncmp(event->data, "led_off", 7) == 0) {
            gpio_set_level(LED_GPIO, 0);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT 错误");
        break;

    default:
        break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.client_id = "esp32s3-attitude-01",   // 同一 Broker 上客户端 ID 不能重复
        .session.keepalive = 30,
    };
    s_client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(s_client);   // 断线后 SDK 自动重连
}

bool mqtt_app_is_connected(void)
{
    return s_connected;
}

void mqtt_app_publish_attitude(float ax, float ay, float az)
{
    if (!s_connected || s_client == NULL) {
        return;   // 未连接时静默丢帧，由上层决定是否缓存
    }
    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"ax\":%.2f,\"ay\":%.2f,\"az\":%.2f}", ax, ay, az);
    esp_mqtt_client_publish(s_client, MQTT_TOPIC_DATA, payload, 0, 1, 0);
}

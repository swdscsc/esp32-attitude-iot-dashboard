#ifndef __MQTT_APP_H
#define __MQTT_APP_H

#include <stdbool.h>

/***************************  MQTT 配置（按需修改）  ***************************/
// 公共测试 Broker（EMQX 免费公共实例，无需注册）。
// 也可以换成 mqtt://test.mosquitto.org:1883 或自己搭的 mosquitto
#define MQTT_BROKER_URI      "mqtt://broker.emqx.io:1883"

// 主题前缀：务必改成自己的名字/学号，避免和公共 Broker 上其他人撞车
#define MQTT_TOPIC_PREFIX    "swdscsc/attitude"

// 两个主题：上行发姿态数据，下行收控制命令（演示双向通信）
#define MQTT_TOPIC_DATA      MQTT_TOPIC_PREFIX "/data"   // 设备 -> 云
#define MQTT_TOPIC_CMD       MQTT_TOPIC_PREFIX "/cmd"    // 云 -> 设备
/********************************************************************************/

// 启动 MQTT 客户端（内部会注册事件回调、连接 Broker、订阅命令主题）
void mqtt_app_start(void);

// 是否已连上 Broker（发布前判断用）
bool mqtt_app_is_connected(void);

// 发布一帧姿态角（单位：度），内部拼成 JSON：{"ax":..,"ay":..,"az":..}
void mqtt_app_publish_attitude(float ax, float ay, float az);

#endif

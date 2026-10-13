#ifndef __WIFI_STA_H
#define __WIFI_STA_H

#include "esp_err.h"

// ⚠️ 安全说明：真实 WiFi 账号密码不写入代码仓库。
// 编译前请复制 wifi_secrets_example.h 为 wifi_secrets.h 并填入自己的账号密码，
// wifi_secrets.h 已被 .gitignore 忽略，不会被提交。
#include "wifi_secrets.h"

//枚举
typedef enum{
    WIFI_DISCONNECTED,                            //wifi断开
    WIFI_CONNECTED,                               //wifi已连接
}WIFI_EV_e;
typedef void(*wifi_event_cb)(WIFI_EV_e);

//函数声明
esp_err_t wifi_sta_init(wifi_event_cb f);

#endif



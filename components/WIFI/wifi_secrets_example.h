#ifndef __WIFI_SECRETS_EXAMPLE_H
#define __WIFI_SECRETS_EXAMPLE_H

// ============================================================
// 使用说明：
//   1. 复制本文件为同目录下的 wifi_secrets.h
//   2. 在 wifi_secrets.h 中填入你自己的 WiFi 账号密码
//   3. wifi_secrets.h 已被 .gitignore 忽略，不会提交到仓库
//
// ⚠️ 请勿直接修改本文件提交真实密码
// ============================================================

// 待连接热点的账号密码（STA 模式）
#define  DEFAULT_WIFI_SSID       "YOUR_WIFI_SSID"
#define  DEFAULT_WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// 板子作为热点时的配置（AP 模式）
#define  ESP_WIFI_SSID           "ESP32_AP"
#define  ESP_WIFI_PASS           "YOUR_AP_PASSWORD"
#define  ESP_WIFI_CHANNEL        1
#define  MAX_STA_CONN            4

#endif

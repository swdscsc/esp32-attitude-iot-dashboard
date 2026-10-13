# esp32-attitude-iot-dashboard · ESP32 姿态数据上云 + Python 分析

把真实项目 [esp32-qmi8658-attitude-monitor](https://github.com/swdscsc/esp32-qmi8658-attitude-monitor) 的姿态数据，
从"只在本地屏幕上看"升级为 **MQTT 上云 → 云端采集入库 → pandas 分析出图** 的完整物联网链路，
并加入 **云端下发命令控制 LED** 的下行通道，演示双向通信。

## 架构

```
┌────────────────────── ESP32-S3（真实硬件）──────────────────────┐
│  QMI8658 六轴传感器                                             │
│    │ I2C 读取原始加速度                                         │
│    ▼                                                           │
│  姿态解算（三轴倾角）──► MQTT 客户端 ──┬──► 上行: .../data 500ms/帧 │
│                                      └──◄ 下行: .../cmd  LED 开关 │
└────────────────────────────────────────────────────────────────┘
                     │ MQTT (broker.emqx.io 公共 Broker)
                     ▼
┌────────────────────── Python 数据端（PC）─────────────────────┐
│  collector.py  订阅主题 → 解析 JSON → 写入 SQLite              │
│       ▼                                                       │
│  analyze.py    pandas 统计 + matplotlib 三轴时序图             │
│                输出 report.png / summary.csv                   │
└───────────────────────────────────────────────────────────────┘
```

## 项目状态（诚实标注，随进度更新）

- [x] 固件代码：WIFI / QMI / LED 组件复用自真实项目，新增 MQTT_APP 组件
- [x] Python 端：采集入库 + 统计分析脚本
- [ ] **上机验证：烧录固件 + 真实传感器收发数据**（进行中）
- [ ] 采集 10 分钟数据并产出分析报告
- [ ] 后续计划：断线缓存补传、接入_influxDB/时序数据库、异常姿态检测

## 目录结构

```
├── main/main.c                  # 主流程：等WiFi → 启动MQTT → 500ms采集发布循环
├── components/
│   ├── QMI/                     # 复用自真实项目：QMI8658 I2C 驱动 + 姿态解算
│   ├── WIFI/                    # 复用自真实项目：STA 联网 + 事件回调
│   ├── LED/                     # 复用自真实项目：板载 LED（下行命令控制对象）
│   └── MQTT_APP/                # 新增：esp-mqtt 客户端、JSON 组包、命令解析
├── python/
│   ├── collector.py             # MQTT 订阅 → SQLite 落库
│   ├── analyze.py               # pandas 统计 + matplotlib 出图
│   └── requirements.txt
└── docs/
    ├── 手把手教程.md             # 从烧录到出报告的完整操作步骤
    └── 架构讲解与面试问答.md      # 数据流、技术选型原因、常见追问
```

## 快速开始

固件端（需要 ESP-IDF 环境，详见 `docs/手把手教程.md`）：

```bash
# 1. 复制 WiFi 密钥模板并填入自己的账号（此文件已被 .gitignore 忽略）
cp components/WIFI/wifi_secrets_example.h components/WIFI/wifi_secrets.h

# 2. 编译烧录（ESP32-S3）
idf.py set-target esp32s3
idf.py build flash monitor
```

Python 端：

```bash
cd python
pip install -r requirements.txt
python collector.py      # 开始收数据，Ctrl+C 停止
python analyze.py        # 生成 report.png + summary.csv
```

想不写代码先看效果？用 [MQTTX](https://mqttx.app/) 客户端连 `broker.emqx.io`，
订阅 `swdscsc/attitude/data` 即可看到 JSON 数据流。

## 硬件平台

| 项目 | 参数 |
| --- | --- |
| MCU | ESP32-S3 |
| 传感器 | QMI8658 六轴（I2C：SCL=GPIO41，SDA=GPIO42） |
| 指示灯 | GPIO1 板载 LED（受云端命令控制） |
| 联网 | WiFi Station 模式 |

## 安全说明

- 真实 WiFi 账号密码不进仓库：`wifi_secrets.h` 已被 `.gitignore` 忽略，仓库内只保留 `wifi_secrets_example.h` 模板
- 公共 Broker 上的数据所有人可订阅（公开测试数据，不含隐私）；正式使用需自建 Broker + TLS + 账号认证

## License

MIT

# -*- coding: utf-8 -*-
"""
姿态数据采集器：订阅 ESP32 发布的 MQTT 姿态主题，解析 JSON 后写入 SQLite。

用法：
    python collector.py                 # 默认连接 broker.emqx.io
依赖：
    pip install paho-mqtt==1.6.1        # 见 requirements.txt
"""
import json
import sqlite3
import sys
import time

import paho.mqtt.client as mqtt

# ↓↓↓ 与 esp32/components/MQTT_APP/mqtt_app.h 中的配置保持一致 ↓↓↓
BROKER = "broker.emqx.io"
PORT = 1883
TOPIC = "swdscsc/attitude/data"
# ↑↑↑ 修改固件主题前缀时，这里要同步修改 ↑↑↑

DB_PATH = "attitude.db"
COMMIT_EVERY = 10  # 每 10 条提交一次，减少磁盘写入


def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute(
        """CREATE TABLE IF NOT EXISTS attitude(
               ts   REAL,   -- 收到数据的 Unix 时间戳（秒）
               ax   REAL,   -- X 轴倾角（度）
               ay   REAL,   -- Y 轴倾角（度）
               az   REAL)   -- Z 轴倾角（度）"""
    )
    conn.execute("CREATE INDEX IF NOT EXISTS idx_attitude_ts ON attitude(ts)")
    return conn


def main():
    conn = init_db()
    buf = []

    def on_connect(client, userdata, flags, rc):
        print(f"[collector] 已连接 Broker({BROKER}) rc={rc}，订阅 {TOPIC}")
        client.subscribe(TOPIC, qos=1)

    def on_message(client, userdata, msg):
        try:
            d = json.loads(msg.payload.decode("utf-8"))
            row = (time.time(), float(d["ax"]), float(d["ay"]), float(d["az"]))
            buf.append(row)
            print(f"  ax={row[1]:7.2f}  ay={row[2]:7.2f}  az={row[3]:7.2f}")
            if len(buf) >= COMMIT_EVERY:
                conn.executemany("INSERT INTO attitude VALUES (?,?,?,?)", buf)
                conn.commit()
                buf.clear()
        except (ValueError, KeyError, json.JSONDecodeError) as e:
            print(f"[collector] 丢弃异常报文: {msg.payload!r} ({e})")

    client = mqtt.Client(client_id="swd-dashboard-collector")
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(BROKER, PORT, keepalive=30)

    print("[collector] 按 Ctrl+C 停止；数据已写入", DB_PATH)
    try:
        client.loop_forever()
    except KeyboardInterrupt:
        pass
    finally:
        if buf:  # 退出前把没提交的尾部数据落库
            conn.executemany("INSERT INTO attitude VALUES (?,?,?,?)", buf)
            conn.commit()
        conn.close()
        print("[collector] 已退出，数据保存在", DB_PATH)


if __name__ == "__main__":
    main()

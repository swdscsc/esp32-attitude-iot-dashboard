# -*- coding: utf-8 -*-
"""
姿态数据分析：从 SQLite 读取采集数据，输出统计报表 + 三轴倾角时序图。

用法：
    python analyze.py               # 读取 attitude.db，生成 report.png + summary.csv
依赖：
    pip install pandas matplotlib   # 见 requirements.txt
"""
import sqlite3

import matplotlib
matplotlib.use("Agg")  # 无界面环境也能出图
import matplotlib.pyplot as plt
import pandas as pd

DB_PATH = "attitude.db"
PNG_PATH = "report.png"
CSV_PATH = "summary.csv"


def load_data() -> pd.DataFrame:
    conn = sqlite3.connect(DB_PATH)
    try:
        df = pd.read_sql_query("SELECT ts, ax, ay, az FROM attitude ORDER BY ts", conn)
    finally:
        conn.close()
    if df.empty:
        raise SystemExit("数据库为空：先运行 collector.py 采集数据，再运行本脚本")
    df["time"] = pd.to_datetime(df["ts"], unit="s")
    return df


def summarize(df: pd.DataFrame) -> pd.DataFrame:
    # 每轴统计：均值 / 标准差 / 极值 —— 衡量倾角波动幅度（是否抖动、是否超量程）
    stats = df[["ax", "ay", "az"]].agg(["mean", "std", "min", "max"]).T
    stats["range"] = stats["max"] - stats["min"]
    return stats.round(3)


def plot_axes(df: pd.DataFrame):
    fig, axes = plt.subplots(3, 1, figsize=(10, 8), sharex=True)
    titles = ["X 轴倾角 (°)", "Y 轴倾角 (°)", "Z 轴倾角 (°)"]
    for ax, col, title in zip(axes, ["ax", "ay", "az"], titles):
        ax.plot(df["time"], df[col], linewidth=0.8)
        ax.set_ylabel(title)
        ax.grid(alpha=0.3)
    axes[-1].set_xlabel("时间")
    fig.suptitle("ESP32 姿态数据时序曲线")
    fig.tight_layout()
    fig.savefig(PNG_PATH, dpi=120)


def main():
    df = load_data()
    print(f"[analyze] 共 {len(df)} 帧数据，"
          f"时间范围 {df['time'].iloc[0]} ~ {df['time'].iloc[-1]}")

    stats = summarize(df)
    print("\n=== 三轴倾角统计 ===")
    print(stats)

    stats.to_csv(CSV_PATH, encoding="utf-8-sig")
    plot_axes(df)
    print(f"\n[analyze] 已生成 {PNG_PATH}（时序图）和 {CSV_PATH}（统计表）")


if __name__ == "__main__":
    main()

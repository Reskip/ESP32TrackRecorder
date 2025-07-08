# ESP32TrackRecorder

## Project Background

In recent years, outdoor activities such as hiking, cycling, and fieldwork have placed higher demands on portable track recorders. However, most existing track recorders suffer from two main issues:

1. **Poor Positioning in Challenging Environments**: Many consumer-grade track recorders struggle with accurate and stable positioning in complex environments—such as during rapid movement, or under weak GNSS signal conditions (dense forests, urban canyons, tunnels, etc.). This can lead to lost tracks, large offsets, or incomplete trace logs.
2. **Limited Endurance**: High power consumption and insufficient optimization often result in short battery life, making it difficult for devices to support long-term, continuous outdoor use.

To address these challenges, we developed the ESP32TrackRecorder.

---

## Overview

ESP32TrackRecorder is an open-source, high-performance track logger built with the ESP32 microcontroller and the Ublox M9N GNSS chip. It aims to provide reliable, high-accuracy, and long-endurance track recording, even in challenging signal environments or during high-speed movement.

---

## Key Advantages

- **Advanced Hardware Design**:
  - **ESP32 as Main Controller**: Offers powerful processing, low power operation, Wi-Fi/Bluetooth connectivity, and flexible peripheral support.
  - **Ublox M9N GNSS Module**: One of the most advanced consumer GNSS chips, supporting concurrent multi-band, multi-system (GPS, Galileo, Glonass, BeiDou) positioning. This significantly improves fix speed, signal robustness, and accuracy—especially under weak or complex signal conditions compared to traditional single-system modules.

- **Enhanced Track Logging and Export**:
  - Accurate real-time positioning and GPX track export.
  - Designed for challenging environments and high-speed recording scenarios.

- **Extended Battery Life**:
  - Optimized power management and low-power operation for prolonged field use.

- **User-Friendly Features**:
  - Multi-page OLED UI for device status, satellite info, battery, settings, etc.
  - Built-in web server for real-time map display (Leaflet) and configuration.

---

## Features

- **High-Accuracy GNSS Tracking** with Ublox M9N
- **Long Battery Life** with optimized power management
- **Multi-system GNSS support** (GPS/GLONASS/Beidou/Galileo)
- **Multi-page OLED Display**: Status, GNSS, battery, settings, etc.
- **Web UI**: Real-time track display, device management, GPX export
- **Wi-Fi/Bluetooth Support**: For remote monitoring and configuration
- **Modular Codebase**: Clean, extensible project structure

---

## Usage

### Build & Flash

1. **Requirements**: ESP-IDF v4.x+, CMake, ESP32 board, Ublox M9N GNSS module.
2. Clone the repo and initialize submodules:
   ```bash
   git clone https://github.com/Reskip/ESP32TrackRecorder.git
   cd ESP32TrackRecorder
   ```
3. Configure ESP-IDF and set the project parameters for your hardware.
4. Build and flash:
   ```bash
   idf.py build
   idf.py -p /dev/ttyUSB0 flash
   ```
5. Configure Wi-Fi and timezone in `CONFIG.TXT` (SD card or SPI flash).

### Operation

- Use device buttons to navigate pages, start/stop recording, and enable/disable the web server.
- Access the web UI from a browser on the same Wi-Fi network (see device IP on OLED).
- Download GPX files or monitor tracks in real time.

---

## Project Structure

- `main/` - Main application logic, pages, status, utils, web page.
- `components/minmea/` - GNSS NMEA parser.
- `components/oled/` - OLED display driver.
- `main/page/` - Page logic and UI rendering.
- `main/status/` - Status modules (battery, GNSS, etc).
- `main/utils/` - Utilities (distance calculation, GPX export, etc).

---

## License

This project is licensed under the MIT License.

---

# ESP32轨迹记录仪

## 项目背景

近年来，徒步、骑行、野外等户外活动对便携轨迹记录仪提出了更高要求。然而，现有的轨迹记录器普遍存在以下问题：

1. **复杂环境下定位能力有限**：许多消费级轨迹记录器在复杂环境下（如快速移动、弱信号、丛林、城市峡谷、隧道等）难以实现准确、稳定的定位，导致轨迹丢失、偏移大或记录不全。
2. **续航能力较弱**：功耗高、系统未优化，导致难以长时间持续户外使用。

为了解决这些痛点，我们设计了 ESP32TrackRecorder 项目。

---

## 项目简介

ESP32TrackRecorder 是一款基于 ESP32 主控和 Ublox M9N GNSS 芯片的高性能开源轨迹记录仪。旨在复杂信号环境或高速移动场景下，提供稳定、精准、超长续航的轨迹记录能力。

---

## 硬件优势

- **ESP32 主控**：高性能、低功耗，支持 Wi-Fi/Bluetooth，丰富的外设扩展能力。
- **Ublox M9N 导航模块**：消费级中最先进的 GNSS 芯片之一，支持多系统（GPS、北斗、GLONASS、Galileo）和多频点并发定位。相比传统单系统芯片，M9N 在弱信号、复杂环境下具备更快的首次定位速度、更强的抗干扰能力和更高的定位精度，适合严苛户外应用。

---

## 功能特性

- **高精度 GNSS 轨迹记录**（支持多系统/多频点）
- **超长续航**（功耗优化/低功耗设计）
- **多页面 OLED 显示**（状态、卫星、电池、设置等）
- **内置网页管理**（实时地图轨迹显示、设备管理、GPX 导出）
- **Wi-Fi/Bluetooth 支持**（远程配置与监控）
- **模块化代码结构**（便于扩展和维护）

---

## 使用方法

### 编译与烧录

1. **环境要求**：ESP-IDF v4.x+，CMake，ESP32 主控板，Ublox M9N GNSS 模块。
2. 克隆仓库并初始化子模块：
   ```bash
   git clone https://github.com/Reskip/ESP32TrackRecorder.git
   cd ESP32TrackRecorder
   ```
3. 配置 ESP-IDF，设置硬件参数。
4. 编译并烧录固件：
   ```bash
   idf.py build
   idf.py -p /dev/ttyUSB0 flash
   ```
5. 在 SD 卡或 SPI 闪存中新建/编辑 `CONFIG.TXT` 以配置 Wi-Fi、时区等参数。

### 操作说明

- 通过设备按键切换页面、启动/停止记录、开关网页服务。
- 通过同一 Wi-Fi 网络的浏览器访问 Web UI（OLED 屏幕上可见 IP）。
- 实时查看轨迹、导出 GPX 文件。

---

## 目录结构

- `main/` - 主应用逻辑、页面、状态、工具、网页。
- `components/minmea/` - GNSS NMEA 解析。
- `components/oled/` - OLED 驱动。
- `main/page/` - 页面逻辑与 UI 渲染。
- `main/status/` - 状态模块（电池、GNSS 等）。
- `main/utils/` - 工具函数（距离计算、GPX 导出等）。

---

## 许可证

本项目采用 MIT 开源许可证。

---

# 26-H- 车载平衡滚球控制系统

本仓库包含 2026 年电赛小车滚球控制系统的两套嵌入式固件。系统采用三主控协作架构：TI 底盘负责循迹与运动执行，MC02 负责云台、电机安全状态机与调度，并可与上位视觉计算单元联动。

> 注意：本项目涉及电机、锂电池和机械运动。首次上电或调试时，请断开电机供电、抬起驱动轮，并遵循各子项目中的接线与调试说明。

## 项目结构

| 目录 | 平台 | 职责 |
| --- | --- | --- |
| [`mspm0dache/`](mspm0dache/) | TI LP-MSPM0G3507 | 后驱差速底盘控制、编码器闭环、八路循迹传感器、OLED 显示，以及与 MC02 的串口通信。 |
| [`gimbal_MC02/`](gimbal_MC02/) | STM32H723 / 达妙 MC-Board02 | DM-J4310 云台控制、CAN 通信、USB CDC 调试接口、供电安全保护，以及与 TI/Jetson 的协议接口。 |

## 系统协作关系

```text
Jetson（视觉/上位机）
        │ USB CDC
        ▼
MC02（STM32H723） ───── UART ───── TI 底盘（MSPM0G3507）
        │ CAN                              │ PWM / 编码器 / I²C
        ▼                                  ▼
DM-J4310 云台电机                    差速驱动电机、循迹传感器、OLED
```

## 快速开始

### 底盘固件：`mspm0dache`

该项目使用 TI MSPM0 SDK 与 TI Clang 工具链。打开项目目录后，可在 VS Code 中选择构建任务，或使用：

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . -j4
```

详细的编译、下载、接线和首次标定说明见：

- [`mspm0dache/README.md`](mspm0dache/README.md)
- [`mspm0dache/COMMISSIONING.md`](mspm0dache/COMMISSIONING.md)
- [`mspm0dache/调试与标定指南.md`](mspm0dache/调试与标定指南.md)

### 云台固件：`gimbal_MC02`

该项目使用 CMake、Ninja 和 `arm-none-eabi-gcc`。在项目目录下执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

生成的调试产物位于 `build/Debug/`。调试下载使用 J-Link/Ozone；完整的接线与安全上电流程见：

- [`gimbal_MC02/README.md`](gimbal_MC02/README.md)
- [`gimbal_MC02/docs/MC02_WIRING_CHECKLIST.md`](gimbal_MC02/docs/MC02_WIRING_CHECKLIST.md)
- [`gimbal_MC02/docs/MC02_TI_PROTOCOL_V2.md`](gimbal_MC02/docs/MC02_TI_PROTOCOL_V2.md)
- [`gimbal_MC02/docs/JETSON_MC02_PROTOCOL_V3.md`](gimbal_MC02/docs/JETSON_MC02_PROTOCOL_V3.md)

## 协议与安全

- MC02 与 TI 底盘通过 UART 进行状态与任务控制通信。
- MC02 与云台电机通过 CAN 通信，并在上电、使能、故障和超时路径中实施保护。
- 底盘固件支持编码器、循迹传感器和通信链路的安全监测；异常时会停止并解除电机使能。
- 任何带电运动测试前，都应先完成无负载验证，并保留随时切断主电源的能力。

## 说明

各子项目的 README 是对应硬件与固件的权威操作说明。本文档用于介绍仓库整体结构；具体引脚、构建环境、协议帧和调参方法请以子项目文档为准。

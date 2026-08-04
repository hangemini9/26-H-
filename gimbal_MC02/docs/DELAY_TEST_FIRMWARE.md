# 全环路延迟专用固件

适用 MC02 build `2026073106` 的 `DelayTest` 构建。它只用于
《现场测量手册》第三部分的 `total_delay_ms` 测量，不是比赛固件。

## 固定测试动作

- 管道指令：`0 -> -1.5 deg -> 0`
- `-1.5 deg` 保持：`500 ms`
- DM4310 `v_des`：`12.0 rad/s`
- V3 `0x81 GIMBAL_STATUS`：`100 Hz`
- V3 帧格式、字段、单位、符号和 CRC 完全不变
- 没有任意角度入口，没有方波或自动重复入口

按已测传动比 `pipe/motor = 0.044`，`-1.5 deg` 管道角对应约
`+34.1 deg` 电机角。这个速度和已装机构的实际安全性不能由编译证明，
所以固件强制先运行一次完全相同的空载脉冲，并等待人工确认。

## 构建和烧录

```powershell
cd C:\Users\Han\Desktop\dIANSAI\diansai(1)\gimbal_MC02
cmake --preset DelayTest
cmake --build --preset DelayTest --clean-first
```

烧录：

```text
build/DelayTest/gimbal_MC02.elf
```

上电运行后，在 Ozone Watch 确认：

```text
g_gimbal_debug.build_id = 2026073106
g_gimbal_debug.delay_test_enabled = 1
g_gimbal_debug.delay_test_phase = 0
```

`Debug`、`Release` 和 `Vehicle` 构建中的
`delay_test_enabled` 都必须为 `0`。不要把 `DelayTest` 镜像用于比赛。

## Ozone 请求值

Ozone 在 CPU 运行时不能直接编辑 Watched Data 的 Value 单元格。必须打开
`gimbal_MC02_DelayTest.jdebug`，在 Ozone Console 通过内置
`Script.Exec` 调用工程里的辅助函数：

```text
Script.Exec("DelayTestPrepare")
Script.Exec("DelayTestValidateEmpty")
Script.Exec("DelayTestAckMechanical")
Script.Exec("DelayTestMeasure")
Script.Exec("DelayTestStop")
```

前四个辅助函数会使用 `Target.WriteU32` 后台内存访问连续写入 key 和 request，
CPU 和全部看门狗保持运行；STOP 只写 request `5`。固件会自动清零 key 和
request。不要暂停 CPU、打断点或单步执行带电动作。

每次被接受的空载验证、机械确认或正式测量请求都会把有界 OUT1 保护重新设为
300 秒；被拒绝的请求不会续期。若操作者连续300秒没有执行有效阶段动作，固件
仍会进入 phase 8、关闭 OUT1。这样可以完成8～10次测量，同时不会取消无人值守
超时。

| request | 含义 | 接受条件 |
|---:|---|---|
| 1 | PREPARE | SAFE_IDLE、无故障、没有比赛/CALRUN |
| 2 | VALIDATE_EMPTY | phase=3，球已取走，机构运动区域净空 |
| 3 | ACK_MECHANICAL | phase=6，操作者确认空载动作无碰撞/卡滞 |
| 4 | MEASURE | phase=7，视觉有效、球在中心且基本静止 |
| 5 | STOP | 随时接受，不需要 key，立即失能并关闭 OUT1 |

`delay_test_phase`：

| phase | 含义 |
|---:|---|
| 0 | IDLE |
| 1 | 等待 OUT1/DM4310 反馈 |
| 2 | 3 秒倒计时并保持零位 |
| 3 | 等待空载验证脉冲 |
| 4 | `-1.5 deg` 脉冲进行中 |
| 5 | 返回零位并确认稳定 |
| 6 | 空载验证完成，等待人工确认 |
| 7 | 可以触发带球测量 |
| 8 | 测试故障，需先清故障 |

## 首次空载验证

1. 取走球，架空/固定整车，确认传动和管道周围无人、无工具和线缆干涉。
2. 保证 K8/总急停可立即操作；Ozone 保持 Run，禁止断点。
3. 在 Ozone Console 执行 `Script.Exec("DelayTestPrepare")`。
4. 等待 `delay_test_phase = 3`，同时确认：
   - `fault = 0`
   - `feedback_age_ms <= 80`
   - `motor_error = 1`
   - `level_zero_valid = 1`
5. 在 Ozone Console 执行
   `Script.Exec("DelayTestValidateEmpty")`。固件只执行一次
   `0 -> -1.5 deg（500 ms）-> 0`。
6. 等待 `delay_test_phase = 6`。检查无碰撞、无卡滞、无松脱、方向正确，
   并且最终回到零位。任何异常立即 request `5` 或按 K8，禁止确认。
7. 确认机械动作正常后，在 Console 执行
   `Script.Exec("DelayTestAckMechanical")`；
   `delay_test_phase` 应变为 `7`。

空载验证没有通过就不能进入带球测量。一次 STOP、故障、复位或重新烧录
都会清除这次确认，必须重新空载验证。

## 带球测量

1. Jetson 保持发送 V3 HEARTBEAT 和 VISION_SAMPLE，并确认相机、检测器、
   标定三项 ready。
2. 临时把
   `competition/competition/config/control.yaml` 中
   `estimator.r_meas_cm` 改为 `0.001`。
3. 把球放回 O 点并保持静止。MC02 要求 `|x| <= 10 mm`，连续至少
   8 个有效样本，并在连续 `300 ms` 内观察到的位置跨度不超过 `2 mm`。
   观察 `delay_test_stable_ready = 1` 后才能触发；视觉无效、超出中心、
   样本中断或位置跨度超限都会重新开始稳定计时。
4. 开始视觉侧数据记录。
5. 在 Console 执行 `Script.Exec("DelayTestMeasure")`。固件执行一次固定
   脉冲并自动返回零位。
6. 等待 `delay_test_phase` 回到 `7`；
   `delay_test_measurement_count` 应增加 1。
7. 将球重新放回 O 点并静止，重复步骤 4–6，共做 8–10 次。
8. 用视觉侧 `tools/meas_delay.py` 逐次计算，最终取中位数写入
   `total_delay_ms`。恢复真实的 `r_meas_cm`。

当前 Windows 交接副本中没有 `tools/meas_delay.py`，运行测量前应从视觉同学
取得与《现场测量手册》一致的脚本，不能用不存在的本地路径代替。

## 自动拒绝和停机条件

- 带球触发前视觉无效、未标定、置信度不足、位置或连续稳定窗口不满足：只拒绝，
  `delay_test_denied_count` 增加，不产生动作。
- 动作期间 DM4310 反馈超过 `80 ms`、电机报告异常、CAN 发送失败、
  Jetson/视觉丢失、返回零位超过 `2 s`：失能、关闭 OUT1并进入故障。
- TI K7/K8、安全停机、普通 Ozone STOP、delay request `5`、复位：
  都会终止测试并清除机械确认。

编译通过只说明源码可以生成镜像，不代表高速空载动作或全环路测量已经在
实车上验证。

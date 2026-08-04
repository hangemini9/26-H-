# gimbal_MC02

达妙 DM-MC-Board02（STM32H723VGT6）控制 DM-J4310-2EC V1.2 的云台固件。
当前版本只面向桌面安全联调；视觉闭环、五项现场测试状态机和 TI 底盘通信均未启用。

## 当前固定配置

- 电机：DM-J4310-2EC V1.2，24 V，减速比 10:1
- 电机模式：位置速度模式
- CAN：经典 CAN、标准帧、1 Mbit/s
- 电机 CAN ID：`0x01`
- 电机 Master ID（反馈帧 ID）：`0xF1`
- MC02 电机 CAN：FDCAN1，PD0=RX、PD1=TX
- Jetson 链路：MC02 USB CDC；接口定义见
  [`docs/JETSON_MC02_PROTOCOL_V3.md`](docs/JETSON_MC02_PROTOCOL_V3.md)，build `2026073106` 已实现
- TI 底盘链路：接口定义见
  [`docs/MC02_TI_PROTOCOL_V2.md`](docs/MC02_TI_PROTOCOL_V2.md)，当前为 MC02 `2026080107`（按用户要求回退第二问，其他问保留）与 TI `2026080104`；五问题号路径、K6底盘软件动力许可、加减速斜坡前馈、停车后至少五秒达妙闭环保持和三秒最终稳定握手均已实现
- 编译：VSCode + CMake + Ninja + arm-none-eabi-gcc
- 调试/下载：J-Link + Ozone，SWD，4 MHz

完整接线和分阶段上电清单见
[`docs/MC02_WIRING_CHECKLIST.md`](docs/MC02_WIRING_CHECKLIST.md)。

依据 MC02 V1.1 原理图，PC14/PC13 分别控制 OUT1/OUT2 的高边电源。构建
`2026073008`在启动时明确保持两路关闭；OUT2始终关闭。常规接口只允许通过
`PWRARM 4310`后3秒内发送`PWRON`开启OUT1；人工调试开关启用时，
`CALRUN 4310 deg`会在同一安全状态机内完成一次受限上电测试。冷启动阶段会在保持转矩禁用的前提下
持续请求反馈，最多等待15秒。OUT1最多开启300秒，`STOP`、`PWROFF`、
任意故障、非JOG动作超时或复位都会立即关闭。JOG超时会失能并回到OBSERVE，
但保持同一供电周期的电机角度坐标。第一次必须断开电机，在空载状态下
分别验证OUT1关闭和开启电压。

## 编译

在 VSCode 中打开本目录，然后运行默认生成任务 `MC02: Build Debug`；或在终端运行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

产物：

```text
build/Debug/gimbal_MC02.elf
build/Debug/gimbal_MC02.hex
build/Debug/gimbal_MC02.bin
```

全环路延迟测量必须使用独立的 `DelayTest` 预设，构建、Ozone 请求顺序和
强制空载验证见
[`docs/DELAY_TEST_FIRMWARE.md`](docs/DELAY_TEST_FIRMWARE.md)。该镜像不是比赛固件。

Ozone 打开 `gimbal_MC02.jdebug`，下载并复位运行。MC02 四针 SWD 插座按
说明书为：1=3.3 V/VTref、2=GND、3=SWCLK/PA14、4=SWDIO/PA13；该插座没有
NRST。不要用 J-Link 给 MC02 或电机供电。

## 上电安全行为

- 上电后状态为 `SAFE_IDLE`，不会自动使能电机。
- OUT1/OUT2上电默认关闭，OUT2没有开启路径。
- 常规OUT1供电需要`PWRARM 4310`与`PWRON`两步操作；人工调试`CALRUN`
  是常规 Debug 的受限例外。独立 `DelayTest` 构建另有专用 Ozone 解锁和
  空载确认流程；两者都只能在对应编译开关启用、SAFE_IDLE和无故障时启动。
- OUT1开启后等待500 ms，再发送5帧失能命令；完成前拒绝`OBSERVE`和运动。
- 启动时发送 5 帧失能命令，然后停止周期控制。
- 任何动作必须先收到新鲜 CAN 反馈，再执行一次性 `ARM`。
- `ARM` 只保留 3 秒，动作开始后立即失效。
- 当前角度被捕获为本次测试的局部零点。
- 最大相对角度固定为 ±10.0°。
- 位置速度模式的 `v_des` 固定为 0.20 rad/s。
- HOLD、STEP、SWEEP 分别在 15、3、6 秒后自动失能。
- 动作中 80 ms 没有电机反馈，立即进入故障并失能。
- 电机反馈状态不是 0（失能）或 1（使能）时，立即进入故障并失能。
- `STOP` 不需要解锁，任何时候都可执行。
- `STOP`、故障、非JOG动作超时和复位同时关闭OUT1；JOG超时失能并返回OBSERVE，
  OUT1仍受300秒总超时保护。

电机带电运动时不要让 CPU 停在断点；暂停 CPU 会同时暂停 CAN 心跳和软件保护。

## 桌面调试顺序

先固定电机和机构，保证 ±10° 内没有机械干涉，手能立即断开总电源。第一轮建议
先不装水管或移除钢球。

1. 在达妙助手确认模式、CAN ID、Master ID 和 1 Mbit/s。CAN Timeout 暂时保持 0。
2. 电机2+2插头保持断开，刷入 Debug ELF，复位后持续运行。
3. Ozone Watch 添加 `g_gimbal_debug`。应看到：
   - `build_id = 2026073008`
   - `state = 1`（SAFE_IDLE）
   - `fault = 0`
   - `can_tx_count` 启动后至少增加 5
   - `power_output_enabled = 0`
4. 第一次验证OUT1时保持电机2+2插头断开。发送`PWRARM 4310`后，在3秒内
   发送`PWRON`。测量空载OUT1应从约0 V变为主输入电压，并在300秒后自动回到
   约0 V。也要分别验证`PWROFF`和`STOP`能立即关闭OUT1。任一项不符合都不得
   连接电机。
5. 空载供电验证通过后，断电连接2+2电机线，再打开MC02 USB虚拟串口。
   USB CDC的终端波特率设置不影响链路，习惯上可选
   115200、8N1。依次发送：

```text
PWRARM 4310
PWRON
PING
OBSERVE
STATUS
```

`PWRON`后至少等待600 ms再发送`OBSERVE`，让电机启动并完成5帧失能命令。
`OBSERVE` 每 20 ms 发送位置速度模式失能帧来请求反馈，绝不使能电机。`STATUS` 应出现
`ONLINE=1`、`RX` 持续增加；失能时 `ERR=0x0`。若无反馈，不要继续动作测试。
如果 OBSERVE 收到状态1（使能）或任意故障反馈，固件会立即关闭 OUT1 并进入故障状态。

6. 第一次动作只做当前位置保持：

```text
ARM
HOLD
```

电机应转为绿色使能，但不应主动大幅转动；15 秒后自动失能。

7. 再做最小正反向测试：

```text
OBSERVE
ARM
STEP 0.5
OBSERVE
ARM
STEP -0.5
```

确认正负方向、反馈角度、机构刚度和电流正常后，才逐步增加到 1°、2°，最大 3°。

8. 最后才测试：

```text
OBSERVE
ARM
SWEEP 1.0
```

紧急停止命令：

```text
STOP
```

故障排除后发送 `CLEAR`，只会清故障并保持失能，不会恢复动作。

## 安装机构快速标定

坐标约定固定如下：钢球位置以中心 O 为零，朝电机所在的物理左端为正，朝铰链所在
的物理右端为负。水管角度则以水平为零、电机端升高为正、电机端降低为负。因此
当前机构的正电机偏移对应负水管角，并使钢球朝左侧正方向加速。日志中的
`CALRUN +deg`始终表示电机角度，不表示水管角度。
手机水平仪初测表明电机±45°对应水管倾角约∓2°，第一版映射可暂按
`水管角度≈-0.044×电机角度`使用，但必须保留为可调参数并由视觉闭环继续标定。

构建 `2026073008` 的下一项无球机构测试只使用一条命令：

```text
CALRUN 4310 45
```

发送前必须取出钢球、把水管置于水平、确认机构周围无人手或工具，并保证能够
随时断开总电源。该命令自行开启OUT1、等待电机反馈，提示后倒计时3秒，然后自动
以不超过0.20 rad/s执行`+45° -> 0° -> -45° -> 0°`，每段最多5秒并输出
起点、零点、目标和终点，最后失能并
关闭OUT1。任意阶段发送`STOP`都会立即中止并关闭OUT1。幅度参数只允许
0.1～45°。
构建`2026073008`修复了旧版运动循环仍二次限制在±10°的问题；本轮正确的两个
端点日志应接近`OFF=+785mrad`和`OFF=-785mrad`。若仍只出现约±174 mrad，
说明运行的不是修复后的固件，不得把它记为±45°测试通过。

这是纯人工桌面调试功能，由
`GIMBAL_ENABLE_MANUAL_CALRUN`控制。当前 Debug 调试构建为`1`；正式上车直接使用
`cmake --preset Vehicle`和`cmake --build --preset Vehicle`，该预设会可靠地把
`GIMBAL_MANUAL_CALRUN`设为`OFF`，使`CALRUN`只返回禁用提示。

如果15秒内仍没有电机反馈，流程会关闭OUT1并直接返回SAFE_IDLE，修复问题后可重新发送同一条命令，
无需先CLEAR。原有`ZERO/JOG/STEP/SWEEP`仍保留为底层单项诊断接口，并继续限制在
±10°和原有超时；只有人工CALRUN允许±45°和5秒单段。构建 `2026073008` 不改变
0.20 rad/s速度、80 ms反馈看门狗和断电保护。

先取出钢球，让底盘和水管处于水平状态。电机进入 `OBSERVE` 且反馈正常后发送：

```text
ZERO
STATUS
```

`ZERO`记录当前电机反馈为本次上电的水管水平零点。`STATUS`中的
`ZVALID=1`表示有效，`Z`是水平零点，`OFF`是当前电机相对水平零点的偏移。
随后发送：

```text
ARM
JOG 10
```

`JOG`始终以保存的水平零点为基准，而不是以每次动作前的位置为基准。
它仍然需要3秒内的`ARM`，并在3秒后失能、返回`OBSERVE`，但不切断OUT1，
以避免DM4310重启改变位置坐标。负向测试使用`JOG -10`。固件会自动报告
FROM、Z、TARGET以及动作结束时的P和OFF。为保证坐标一致，构建`2026073008`
会在STOP、故障、PWROFF、
300秒超时或其他OUT1掉电事件时自动作废零点，之后必须重新执行`ZERO`。
MC02复位同样会清除零点。

## Ozone 直接调参/触发

核心观测变量只有一个：`g_gimbal_debug`。状态和请求枚举见
`Core/Inc/gimbal_app.h`。

不要通过Ozone直接写PC14或GPIO寄存器开启OUT1。供电只允许使用USB命令。

直接触发云台动作时：

1. 将 `arm_key` 写为 `0xA55A5AA5`；
2. STEP/SWEEP 时把 `request_param_tenths_deg` 写成角度的 0.1° 单位，
   例如 `5` 表示 0.5°；
3. 最后写 `request`：STOP=1、OBSERVE=2、HOLD=3、STEP=4、SWEEP=5、
   CLEAR=6。

固件会消费并清零 `arm_key` 与 `request`。推荐先用 USB 命令完成首轮测试；
Ozone Watch 用于观察状态和后续快速调参。

## 目前还需要的实测信息

- 电机接到 MC02 的具体 FDCAN 口、CANH/CANL 和终端电阻实测；
- `STEP +10` 时水管实际顺/逆时针方向，用于冻结控制符号；
- 水管机械水平时的电机反馈角，以及本轮 ±45° CALRUN 端点的反馈角；
- build `2026072903`下OUT1空载关闭/开启/30秒超时/PWROFF/STOP的实测电压；
- 使用OUT1给DM4310供电时的6S电池电压、峰值电流和MC02输出温升。

构建 `2026073008` 保留已经验证的桌面 ±10° 限幅，并新增单指令标定，但这些量确认前仍不
启用视觉闭环，也不继续提高到 ±10° 以上。

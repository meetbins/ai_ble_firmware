# ST17H66 普通 BLE 灯控工程

基于 ST17H66 SDK 3.1.3.3 的独立普通 BLE 外设工程，不复用 Mesh 协议栈。
工程复用现有灯具板引脚：

| 功能 | 引脚 | 驱动方式 |
|---|---|---|
| WS2812 彩色灯 | P34 | SPI0 MOSI，3 Mbps |
| 亮度/指示通道 | P2 | PWM_CH2，低电平有效 |

## 工程入口

```text
st17h66_ble_light/example/ble_peripheral/ble_light/ble_light.uvprojx
```

工具链与现有固件一致：

- Keil uVision
- ARMCC 5.06 update 7
- CMSIS 5.8.0

构建后输出：

```text
Objects/ble_light.axf
bin/ble_light.hex
```

## GATT 协议

设备名：`BLE-LAMP`

广播中带 `GONENGO` manufacturer data，用于兼容 App 的旧版 BLE
设备列表（Android 端插件会去掉 2 字节 Company ID）。

Primary Service：

```text
FF000001-0000-1000-8000-00805F9B0001
```

| 能力 | Characteristic UUID | 写入格式 | 说明 |
|---|---|---|---|
| 开关 | `00003A01-0000-1000-8000-00805F9B0101` | `[on]` | 0 关灯，非 0 开灯 |
| 亮度 | `00003A01-0000-1000-8000-00815F9B0101` | `[0,0,0,brightness]` | brightness 范围 0–100 |
| 颜色 | `00003A01-0000-1000-8000-00825F9B0101` | `[0,0,a,r,g,b]` | 使用第 4–6 字节 RGB |
| 模式 | `00003A01-0000-1000-8000-00835F9B0101` | `[0,mode]` | 预留；当前为常亮模式 0 |

所有特征均支持 Read / Write / Write Without Response。写入颜色或亮度时会
更新保存状态；开灯后恢复最后一次非零 RGB 和亮度。关灯会同时关闭 P2 与
P34，但不会清掉保存的颜色和亮度。

## 手动验证

1. 烧录 `bin/ble_light.hex`。
2. 用 nRF Connect 扫描 `BLE-LAMP` 并连接。
3. 写开关特征 `01`，确认 P2 按默认 50% 亮度点亮，P34 输出白色。
4. 写亮度 `[0,0,0,80]`，确认 P2 亮度变化。
5. 写颜色 `[0,0,0,255,0,0]`，确认 P34 变红。
6. 写开关特征 `00`，确认两路熄灭；再写 `01`，确认恢复最后一次亮度/RGB。
7. 使用 App 旧版 BLE 入口扫描并控制，验证与上述 UUID/格式一致。

## 已验证结果

2026-10-30 在 LG G820N / Android 12 上完成单台端到端验证：

- App 扫描到 `BLE-LAMP` 并建立 GATT 连接。
- OnOff、亮度 87%、颜色和模式写入均返回 Android GATT `status=0`。
- App 显示 `已连接 1/8 台`；全部断开后可重新连接，无需再次扫描。
- 强制结束并冷启动 App 后，设备从 Hive 恢复并重新连接。
- 当前样机广播地址为 `FF:FF:FF:FF:FF:FF`，不能用于多台并发验收。

多台版本必须为每台灯提供唯一静态地址或可解析随机地址，再用至少 5 台实体
样机验证并发连接、批量控制、单台掉线和重连隔离。

## 构建

可用 Keil 打开工程直接 Rebuild，也可使用现场既有工具链：

```powershell
D:\Keil_v5_arm\UV4\UV4.exe -r `
  .\st17h66_ble_light\example\ble_peripheral\ble_light\ble_light.uvprojx `
  -j0 `
  -o .\st17h66_ble_light\example\ble_peripheral\ble_light\build.log
```

本工程只生成 BLE 灯控镜像，不包含 Mesh 配网、Proxy、NVS 或 Mesh 模型。
不要将它和 `example/ble_mesh/mesh_light` 的镜像混用。

# ST17H66 BLE Lamp Firmware

Standalone ST17H66 ordinary BLE lamp firmware. This repository is the AI BLE
firmware main repository for the non-Mesh lamp-control project.

## Project

The canonical Keil project is:

```text
st17h66_ble_light/example/ble_peripheral/ble_light/ble_light.uvprojx
```

It reuses the existing lamp board pins:

| Function | Pin | Driver |
|---|---|---|
| WS2812 colour LED | P34 | SPI0 MOSI, 3 Mbps |
| Brightness / indicator | P2 | PWM_CH2, active low |

The project targets the ST17H66 SDK 3.1.3.3 with Keil uVision, ARMCC 5.06
update 7 and CMSIS 5.8.0.

## SDK dependency

The vendor SDK is not duplicated here. This checkout references the SDK in the
sibling Mesh repository:

```text
E:\aicode\ai_blemesh_firmware\st17h66
```

That sibling repository retains the ST17H66 components, libraries and Mesh
project history. Do not copy the Mesh application into this repository. When
building on another machine, place the SDK checkout at the relative location
documented in [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md), or update the Keil
project's include and source paths to the local SDK.

## Build

Open the canonical project in Keil and run **Rebuild**. The generated image is
`Objects/ble_light.axf`; the project post-build command emits
`bin/ble_light.hex`.

Build outputs, logs, archives and generated IDE state are not tracked.

## GATT

Device name: `BLE-LAMP`

Primary service:

```text
FF000001-0000-1000-8000-00805F9B0001
```

| Capability | Characteristic UUID | Payload |
|---|---|---|
| On/off | `00003A01-0000-1000-8000-00805F9B0101` | `[on]` |
| Brightness | `00003A01-0000-1000-8000-00805F9B0101` with the `81` service byte | `[0,0,0,brightness]` |
| Colour | `00003A01-0000-1000-8000-00805F9B0101` with the `82` service byte | `[0,0,a,r,g,b]` |
| Mode | `00003A01-0000-1000-8000-00805F9B0101` with the `83` service byte | `[0,mode]` |

See the project README for the exact canonical UUIDs and manual test steps.

## Repository boundary

- `st17h66_ble_light/` is the ordinary BLE lamp application source.
- The sibling `ai_blemesh_firmware` repository is the SDK/Mesh dependency and
  is not a subdirectory or Git submodule of this repository.
- `tools/`, `tmp/`, `backup/` and local configuration are ignored.
- Do not commit HEX/AXF/OBJ/MAP files, Keil output directories, or serial
  captures.
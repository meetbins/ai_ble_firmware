# ST17H66 Ordinary BLE Lamp Handoff

- Date: 2026-11-03
- Bridge repository: `E:\aicode\ai_ble_firmware`
- Canonical project:
  `st17h66_ble_light/example/ble_peripheral/ble_light/ble_light.uvprojx`
- SDK/Mesh dependency: `E:\aicode\ai_blemesh_firmware\st17h66`

## Status

This repository contains the standalone ordinary BLE lamp application. The
Mesh firmware and its Git history remain in `ai_blemesh_firmware`; they are not
part of this repository.

The ordinary BLE project was previously verified on a single physical lamp:

- Device: `BLE-LAMP`
- App scan, GATT connection and disconnect/reconnect passed.
- On/off, brightness, colour and mode writes passed.
- Current sample advertisement address is `FF:FF:FF:FF:FF:FF`.

The fixed broadcast address prevents multiple lamps from being treated as
separate devices by Android. A unique static address or resolvable private
address must be provisioned before multi-device production validation.

## Build

Open the canonical Keil project and run **Rebuild**. The normal toolchain is
Keil uVision, ARMCC 5.06 update 7 and CMSIS 5.8.0.

The project references the sibling SDK checkout. If the workspace root or SDK
location changes, update the Keil include and source paths together. Do not
copy the Mesh application into this repository.

Expected local outputs, which are ignored:

```text
st17h66_ble_light/example/ble_peripheral/ble_light/Objects/ble_light.axf
st17h66_ble_light/example/ble_peripheral/ble_light/bin/ble_light.hex
```

## Acceptance checklist

1. Rebuild the canonical project and require 0 errors / 0 warnings.
2. Flash the generated ordinary BLE image; do not mix it with a Mesh image.
3. Scan for `BLE-LAMP` with nRF Connect or the companion App.
4. Verify on/off, 0-100 brightness, RGB colour and mode writes.
5. Verify off/on restores the last non-zero colour and brightness.
6. Verify disconnect/reconnect and App restart recovery.
7. Assign unique BLE addresses before testing five or more lamps, including
   batch control, one-device disconnection and reconnection isolation.

## Git boundary

The `ai_ble_firmware` repository is the ordinary BLE main repository. The
`ai_blemesh_firmware` repository is the SDK/Mesh repository. Neither repository
should contain build outputs, serial captures, keys or local tooling.
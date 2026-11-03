# AI BLE Firmware Project Structure

- Date: 2026-11-03
- Scope: `E:\aicode\ai_ble_firmware`
- Principle: this is the standalone ordinary BLE lamp repository; the Mesh
  application remains in the sibling `ai_blemesh_firmware` repository.

## Directory layout

```text
ai_ble_firmware/
├─ README.md
├─ HANDOFF.md
├─ PROJECT_STRUCTURE.md
├─ .gitignore
└─ st17h66_ble_light/
   └─ example/
      └─ ble_peripheral/
         └─ ble_light/
            ├─ ble_light.uvprojx
            ├─ main.c
            ├─ source/
            └─ RTE/
```

The Keil project references the vendor SDK through the sibling checkout:

```text
ai_ble_firmware/st17h66_ble_light/example/ble_peripheral/ble_light/
  ../../../../../ai_blemesh_firmware/st17h66/
```

That relative path resolves to `E:\aicode\ai_blemesh_firmware\st17h66` in the
standard workspace. The SDK/Mesh repository is a build dependency, not a Git
submodule and not part of this project's source tree.

## Repository boundaries

- `st17h66_ble_light/` contains the ordinary BLE lamp application, Keil
  project, startup files and handoff documentation.
- Mesh source, Mesh acceptance projects and Mesh Git history stay in
  `E:\aicode\ai_blemesh_firmware`.
- Do not add a second copy of the SDK here unless the project explicitly
  changes its dependency model.
- `tools/`, `tmp/`, `backup/` and local configuration are local-only and
  ignored by Git.
- Build outputs and files that may contain provisioning keys or serial
  captures are never committed.

## Verification

Before committing:

1. Confirm `st17h66_ble_light/example/ble_peripheral/ble_light/ble_light.uvprojx`
   opens in Keil.
2. Run Rebuild and require 0 errors / 0 warnings.
3. Confirm `Objects/ble_light.axf` and `bin/ble_light.hex` were generated and
   are ignored by Git.
4. Confirm the sibling `ai_blemesh_firmware` Git worktree remains clean.
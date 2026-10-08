# Slot-18 Android Companion — Planning / NOT STARTED

No Android code, installable APK, app build or user-verified baseline exists as of 2026-10-07. Preserve the existing ESP32 firmware candidate.

Planned native Android application: live radar positions, presence, distances, remote station maps, battery states, thermal/LiDAR when available, diagnostics and authorized settings. Use authenticated Wi-Fi local WebSocket/HTTP for rich data; BLE GATT for nearby pairing, configuration and low-rate status; optional securely configured internet gateway for remote usage.

Follow Master Instruction Library CROSS_PLATFORM_COLLABORATION_STANDARD.md. Read shared/FEATURE_CATALOG.md, shared/DATA_FORMATS_AND_INTERFACES.md, docs/REMOTE_ACCESS_AND_CLIENTS.md and project checkpoints. Establish separate Android roadmap, diagnostics, guided tests, build identity and LAST USER VERIFIED baseline before development. Android tests never make ESP32 code verified. No secret credentials in source.

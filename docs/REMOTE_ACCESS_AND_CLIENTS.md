# Remote Access, Android App and Web Dashboard

Status: PLANNED ONLY (2026-10-07). User has not tested ESP32 firmware or hardware. No application, network service, or remote connection exists yet.

## Product goal
Provide the same normalized sensor measurements to the 7-inch ESP32-S3 display, a native Android app, and a browser dashboard. Support local offline operation, five or more remote sensor stations, and opt-in internet viewing. Keep each station's ID, timestamp, battery, last-seen and measurement uncertainty. Never turn presence-only reports into precise target coordinates.

## Connection modes
- Bluetooth Low Energy (BLE): the ESP32-S3 does not support Bluetooth Classic. Native Android GATT connection is suitable for discovery, secure pairing, Wi-Fi provisioning, small status messages, settings and low-rate alerts. Test bandwidth before attempting full graphics or thermal data.
- Direct Wi-Fi SoftAP: main ESP32 creates password-protected access point. Android or computer connects directly, without internet/router, and opens an authenticated local browser dashboard or app stream.
- Wi-Fi STA: ESP32 joins ordinary router or phone hotspot; viewers on same LAN access the local web/API endpoint. No internet required for same-LAN access.
- Optional internet: remote access requires internet service plus either an authenticated VPN gateway on supported router/PC/Raspberry Pi, or an outbound encrypted relay/message broker, with secure Android/web login. Being Wi-Fi-connected does not by itself make the ESP32 reachable through NAT/cellular networks. Do not expose a public unsecured ESP32 HTTP port.
- Multiple radio modes: BLE/Wi-Fi/ESP-NOW share 2.4 GHz ESP32-S3 radio resources. SoftAP+STA exists but channel sharing, mesh compatibility, memory and throughput must be measured. Do not assume all combinations can operate simultaneously at full speed.

## Architecture
Local sensor UART/I2C -> model-specific decoder -> shared normalized event bus -> on-device LVGL view and authenticated transport adapters -> Android and web. Do not duplicate radar parsing in clients.

Wi-Fi: propose versioned JSON summaries, WebSocket streams for real-time positions and state, REST-like settings/history endpoints, rate limits, backpressure, reconnection and clear STALE states.
BLE: compact GATT services/characteristics for enrollment, low-rate status, and safe settings. No arbitrary web browser BLE assumption; Android native handles BLE.
Internet: secure, explicitly enabled gateway/relay, TLS, view-only by default, revocable credentials, least privilege and recorded command request/result.

## Separate implementation workspaces
- firmware/esp-idf: main firmware, native touchscreen and local API (currently v0.2.0 candidate; source untested by user).
- web/: planned local-browser dashboard; optional installable PWA, no code implemented.
- android/: planned native Android APK; Wi-Fi dashboard plus BLE GATT discovery; no code implemented.
- shared/: versioned data contract and portable feature descriptions; preserve cross-platform coordination and separate verified baselines.

## Recommended development order
1. Verify the physical screen/display v0.2.0 and first radar input separately.
2. Build a password-protected direct Wi-Fi local read-only browser view, with actual data-source and stale indicators; no internet.
3. Add Android app using same sensor feed over Wi-Fi, then BLE GATT setup/lightweight viewing and auto/manual connection selection.
4. Add existing-network STA and multiple authorized viewers; validate coexistence with remote ESP32 mesh.
5. Evaluate optional remote internet via authenticated VPN gateway or outbound TLS relay, only after local operation and security tests.

## Future guided tests
LOCAL-01 Connect Android phone directly to ESP32 Wi-Fi with no internet, open browser and see appropriately labeled telemetry.
LOCAL-02 Live hardware vs simulator, sensor ID/origin and uncertainty are correct; disconnect shows STALE, reconnect restores.
BLE-01 Pair/deny/revoke, low-rate data, Wi-Fi switch; test radio coexistence and malformed packets.
LAN-01 Multiple viewers on router/hotspot, authentication, permissions, rate limits, memory and battery usage.
WAN-01 Unauthorized access refused, remote TLS/auth succeeds through provisioned gateway, loss of internet shows offline/stale; no direct unsecured public services.
All tests require diagnostics, PASS/FAIL reports and user verification, not claims of implementation.

## Important boundary
This ChatGPT conversation itself has no automatic live access to the ESP32. Building a networked UI for the owner does not authorize AI access; any future in-chat device action requires an additional supported connector/integration and explicit authorization.

# Slot-18 Web Dashboard — Planning / NOT CONNECTED

Web client is PLANNED. The existing prototype/ radar demo remains offline SIMULATED data; it is not currently an ESP32-hosted website or real device feed.

Future plan: password-protected ESP32 SoftAP and a local responsive read-only web dashboard. Connect Android phone, tablet or computer over direct Wi-Fi without internet; later access on a shared router LAN; optional installable PWA. Show sensors, station map, ranges, actual target XY only when supported, uncertainty, mesh links, battery, last-seen and simulated-vs-live status.

Internet remote access is separate and requires deliberate secure VPN/gateway or encrypted outbound relay with authenticated clients. Do not expose open ESP32 HTTP endpoints publicly. Avoid promising that a web browser universally supports BLE. Shared protocol in shared/DATA_FORMATS_AND_INTERFACES.md; security/testing requirements in docs/REMOTE_ACCESS_AND_CLIENTS.md.

Keep web-specific status, diagnostics, guided tests and verification separate from Android and ESP-IDF firmware.

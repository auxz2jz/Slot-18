# Shared Telemetry / Client Interface — Design Contract, NOT Implemented

One normalized data model consumed by the ESP32 LVGL interface, a web browser, a native Android app, and eventually a remote gateway. All data is capability-specific. Version the contract and keep the same semantics on all platforms.

Example synthetic event (never claim measured):
```json
{
  "schema_version": 1,
  "event_type": "sensor_update",
  "source": "simulation",
  "station_id": "station-01",
  "sensor_id": "ld2450-front",
  "capability": "track_xy",
  "sequence": 25,
  "elapsed_ms": 18546,
  "detected": true,
  "position_known": true,
  "local_x_right_m": 1.2,
  "local_y_forward_m": 3.0,
  "distance_m": 3.23,
  "station_location": null,
  "station_heading_degrees": null,
  "location_accuracy_m": null,
  "stale": false
}
```

Presence/range-only updates MUST mark position_known=false and both XY fields null. GNSS estimates must carry uncertainty. Unsynchronized device uptime is never a UTC timestamp. Mesh-hop relays preserve original station/sensor identity, source sequence and sample time to enable deduplication and stale detection.

Planned interfaces:
- ESP32 direct/local Wi-Fi: authenticated read-only snapshot plus WebSocket stream of event reports, with negotiated rate and version.
- Android native: same Wi-Fi contract for rich display; BLE GATT for pairing, setup and small status reports, not a naïve copy of web streaming.
- Remote internet: gateway/VPN or authenticated encrypted outbound relay; local function must not depend on any outside service.
- Commands: device, authorization role, request_id, requested setting, accepted/denied result, before/after state and correlated diagnostic events. Read-only default for remote users.
- Client status: connected, reconnecting, stale/offline, last_seen, firmware version, battery voltage/charge and link metrics.
- No embedded fixed credentials or public open HTTP port. Privacy: don't expose location/history except to intentionally authorized clients.

Implementation and testing state as of 2026-10-07: all of the above PLANNED ONLY. See docs/REMOTE_ACCESS_AND_CLIENTS.md.

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

## Planned removable battery bay telemetry
Battery status reports from remote nodes and main unit should retain per-source station identity, timestamp, pack insertion/removal sequence and bay index, supporting two remote bays or four handheld bays without changing the client schema:
- station_id, power_source (usb_wall / battery / external), battery_bay_count, battery_present_count
- bays[]: bay_id, pack_id (if known), present, active, charging, fault, state_of_charge_pct (nullable), state_of_health_pct (nullable), voltage_mV, current_mA (if sensed), temperature_C (if available), remaining_energy_Wh_estimate, estimated_minutes_left (nullable), quality/reason.
- aggregate: usable_energy_Wh_estimate, estimated_minutes_left, number of active power paths, low_battery_alert, hot_swap_safe_for_bay where hardware validated.
- Device estimated minutes must be labeled estimates; no reading from absent gauge can be invented. Emit BATTERY_INSERT, BATTERY_REMOVE, POWER_SOURCE_CHANGE, CHARGE_START/STOP, BATTERY_LOW, GAUGE_UNAVAILABLE, BATTERY_FAULT as actual diagnostic events with source/correlation IDs.
- See docs/HOT_SWAPPABLE_LIPO_POWER_SYSTEM.md. DESIGN ONLY; not implemented or verified.

## Power-input / solar / multi-cartridge state (2026-10-07, DESIGN ONLY)
In addition to the per-bay records above, define external_source_present, external_source_type (usb_c_dc / solar / none), external_input_voltage_mV, external_input_power_mW (if measured), system_power_source (usb_c_dc / solar / cartridges / mixed_supplement), system_load_power_mW, solar_panel_power_mW (if measured), charging_total_power_mW, individual_pack_charge_current_mA and charging_limited_by_source boolean, estimated_minutes_to_full (nullable). Preserve power-source changes and USB/PV absence in logs. Device load has priority over charging; sunny conditions or attached USB do not guarantee charge if input power is insufficient. All per-pack fields should appear in main ESP32 sensor stream and future Android/web clients. Never confuse cartridge hot-swap with individual live cell replacement. Full electrical requirements at docs/USB_SOLAR_MULTIPACK_POWER_ARCHITECTURE.md.

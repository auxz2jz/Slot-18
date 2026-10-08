# System Architecture v0.1 — capability-driven and honest about uncertainty

## Main devices
MAIN: Waveshare 7in ESP32-S3 touchscreen. Displays UI and consumes remote/local telemetry.
NODE: ESP32 remote board with sensor adapter(s), orientation, optional GNSS, battery telemetry; exchanges data with MAIN via local Wi-Fi or ESP-NOW; evaluate RS485 for wired links.
SENSORS: interchangeable driver family: track_xy, presence, range_speed, thermal_grid, lidar_scan, depth_grid, gnss, orientation, door_contact, vibration, PIR, camera_metadata and environment. Only claim what real hardware outputs.

## Data processing
physical sensor -> transport adapter (UART/I2C/etc) -> packet/frame validator -> model-specific decoder -> normalized measurement with provenance -> optional coordinate transform -> state store -> visualization + diagnostics.
Do not fuse two reports into a single identified person without explicit association evidence. Show uncertain estimates with sector, ring or "unknown position".

## Normalized event schema (concept)
{
 "version":1, "time_ms":12345, "source":"simulation|hardware",
 "station_id":"handheld-01", "sensor_id":"ld2450-front",
 "sensor_type":"LD2450", "capability":"track_xy|presence|range_speed",
 "valid":true, "distance_m":3.0,
 "local_x_right_m":1.0, "local_y_forward_m":2.0,
 "heading_deg_cw_from_north":0.0,
 "position_known":true, "confidence":null, "raw_ref":null
}
For presence-only: "position_known":false, X/Y absent, distance optional. Use SI base units internally. Capture packet/raw_ref for diagnostics when available and safe. Never invent confidence, height, identity, GPS fix or target location.

## Coordinates
Sensor-local X is right, Y is forward. Heading is clockwise from geographic North.
east = x*cos(heading) + y*sin(heading)
north = -x*sin(heading) + y*cos(heading)
Then add surveyed station/sensor offset, if known. Handle magnetic vs true north, tilt/pitch/roll and reference datum explicitly. Implement indoor floor plans with relative meters and calibrated anchor points; geolocation of tracks is unavailable until station position/heading/relative track data and uncertainties are sufficient.

## Multi-direction vision
Initial three module types co-located, faced one direction. UI may simulate N/E/S/W selection. Later 4 groups of 3 (12 total), optional 8 groups of 3 (24 total), only after interference and coverage tests. Strong preference for smaller node controllers rather than one giant bundle of 24 UART channels. Hardware 3-UART direct connection assumptions are not guaranteed. 24 GHz mutual interference is a major feasibility gate.

## User mobility
Handhold or pole mount can cause body/hand shadowing. Walking changes relative target velocity and may invalidate occupancy assumptions. IMU/GNSS can aid pose awareness but don't magically compensate for every radar. UI must distinguish walking/static and show confidence/limitations.

## Rendering
Precise on-map dots only for valid XY observations. Presence/range sensors show selected sector, distance band and status; thermal shows low-resolution heat tiles, lidar outline of visible walls. Map station dots must include GNSS/survey accuracy circle. Do not claim an object is human, car, animal or a particular person without separate validated classification.

## Performance, transport, security
Constrain report rates and ring buffers, limit GUI redraw, preserve touch responsiveness and serial ingestion. Include per-sensor sequence, age/stale state, disconnect detection. For remote stations consider packet replay protection, device pairing, key management, rotating session keys if transport supports, and offline-first mode. Keep raw location and detections local by default and avoid publishing live geolocation. GPS, battery and thermal are optional capabilities.

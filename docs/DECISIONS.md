# Decisions / Assumptions / Open Questions

- 2026-10-07: First destination is the next truly empty slot: auxz2jz/Slot-18. Existing occupied slots and master library untouched.
- 2026-10-07: Canonical instructions repo was renamed to auxz2jz/master-instruction-library, confirmed via its INSTRUCTION_INDEX.md.
- 2026-10-07: Initial hardware candidates: Waveshare ESP32-S3-Touch-LCD-7 with LD2450, C4001 SEN0609 and C4002 SEN0691; expansion candidate SC16IS752. None assumed received.
- 2026-10-07: Prioritize browser-hosted software-only interactive radar UI to agree on UX/data contract before copying hardware-specific vendor display/serial code. Intended final firmware stack: Waveshare supported ESP-IDF + LVGL.
- 2026-10-07: Sensor adapters organized by capability (XY / presence / distance), not model-specific drawing. Presence-only will not draw false precise location.
- 2026-10-07: Four cardinal sectors before eight; 12 total modules only after initial three coexist, interference checks; 24 aspirational, not committed hardware promise.
- 2026-10-07: Five remote battery stations are a product requirement; stations can use GNSS, surveyed coordinates, calibrated heading and optional IMU; map with uncertainty.
- 2026-10-07: Use vendor board examples and confirmed pinout, not guessed UART GPIO. C4001 I2C requires module-specific proof. ESP32 vs PC software hosting is an implementation choice, not a permanent requirement for every subsystem.
- 2026-10-07: Battery and wall sensing claims will be conservative. Do not present through-wall people detection as reliable.
- Open: final sensor mounting spacing, interference, actual battery sizing, weatherproof geometry, chosen bus expansion, GPS/heading calibration, map sources and floor-plan capture.

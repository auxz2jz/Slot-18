# Slot-18 — Handheld motion tracker UI vision (user-requested, 2026-11-10)

## Design goal

User wants a **handheld radar/motion tracker**, styled after the recognizable 1986 *Aliens* semicircular motion tracker display: dark display, luminous sweeping lines/arcs, brief blip/trail marks, range rings, sound feedback later, and a separate clear settings page. This is a custom science-fiction-*inspired* interface, not a claim that a scan animation indicates actual radar angle information. User has ADHD and explicitly wants **visible, tangible incremental milestones**, short instructions, and immediate on-screen payoff. Favor iterative live UI prototypes over lengthy abstract configuration-only phases.

## Proposed 800×480 Waveshare GUI

1. **RADAR / LIVE** (main page): big animated semi-circular fan / concentric range rings; central forward arrow and orientation/heading (from IMU when installed). When no IMU, sweep animation is *decorative* and should be plainly marked. Blips drawn ONLY from sensors that return legitimate 2D target positions, or from manually demonstrated mock data clearly identified as DEMO; use plotted detection-distance bands for C4002 rather than fictional precision left/right coordinates.
2. **Target/readout pane:** nearest reliable active distance, motion vs presence state, speed when reported, number of current tracks, link/packet quality. Optional audio pulse only after preferences.
3. **SETTINGS** page: per-sensor enable, min/max range (C4002 0–1100cm), motion/presence sensitivity groups, disappearance hold (user's baseline15s), 80/20cm resolution after separately verified setter, radar screen range rings / visual scan rate and decay, IMU heading alignment/zero, audible alerts.
4. **CALIBRATION** page: prominently distinguish *IMU orientation zero/calibrate* from *C4002 environmental calibration*. C4002 environment calibration requires an appropriate **empty test area, explicit confirmation and backup of sensor configuration**, never triggered by simply entering the page. Avoid automatic sensor parameter writes on startup.
5. **DIAGNOSTICS** page: per-sensor connectivity, raw frames/errors, and current readbacks; user can visually verify progress.

## Actual sensors and limits

- C4002 SEN0691: 24GHz 120°×120° broad field-of-view (DFRobot spec), reports presence/motion states, distance, radial motion, energy/ambient light, but does NOT provide left/right bearing or XY. Strong presence energy is NOT a positional coordinate. The previously verified radar read/write controls are separate and current original baselines restored.
- Existing HLK-LD2450 (Slot-18 user's project sensor, earlier simulated): per manufacturer it sends X/Y position, speed for **up to three moving tracked targets** within typical **6m** and approximately ±60° horizontal sector, native UART often at **256000bps**. **Not yet live-verified on this Waveshare integrated firmware**. Priority is to make LD2450 work as its own bounded serial milestone before merging into LCD page. Its positions are relative to the sensor, not an Earth map. Do not claim stationary people are tracked by LD2450.
- Add a **9-axis IMU** for handheld orientation/compass (accelerometer, gyro, magnetometer) and potentially body movement classification; can rotate or stabilize view based on estimated heading. **An IMU alone cannot give reliable 2D translation/walking position or force the C4002 to provide bearing**; gyro yaw drifts unless calibrated/assisted; magnetometer unreliable near metal/magnets. The BNO085/086 family can provide fused rotation vectors but **Adafruit warns about BNO08x I2C and ESP32-S3 reliability**; select device/interface only after compatibility validation (possibly SPI/UART or alternative IMU).
- The Waveshare ESP32-S3-Touch-LCD-7 is 800×480 touch LCD; shared I²C GPIO8 SDA / GPIO9 SCL also serves onboard touch and external bridge 0x4D. Avoid changing user-verified power/wiring while app display is untested.

## Milestone order (visibly testable, one at a time)

1. User compiles/tests already-built **display-only** LCD dashboard `diagnostics/arduino/slot18_c4002_lcd_dashboard/` and shows actual Waveshare photo. Fix that single step if it fails. Touch currently intentionally disabled in this candidate, radar config write/read unchanged.
2. Add *semicircular Aliens-inspired radar sweep* to LCD as graphics with honest "scan graphic" framing, driven at first by live C4002 state/distance rings; avoid simulated moving positional blips misrepresented as real detected angles.
3. Add touch tabs RADAR, SETTINGS, CALIBRATION, DIAGNOSTICS and display verified current C4002 settings first; only then operator-confirmed writes from touch.
4. Test LD2450 live UART/XY independently (including bridge throughput and 256000bps support). Show true positional blips for valid moving target reports within its own 6m range.
5. Evaluate 9DOF IMU model/interface compatibility with ESP32-S3 and existing I2C bridge; show on-screen live heading/pitch/roll and zeroing before merging with scanner.
6. Investigate multi-sensor fusion, stationary detection mapping, scans while walking, and optional alerts; no unsupported claims about accurate mapped locations without external reference/tracking.

## Sources
- Waveshare 7in spec/I2C: https://docs.waveshare.com/ESP32-S3-Touch-LCD-7
- DFRobot C4002: https://wiki.dfrobot.com/sen0691/
- HLK-LD2450 instruction manual: https://manuals.plus/m/374d7b6608cbce20e803b3aa4f405637826960c1ed42a858dedfaa97a3253478
- Adafruit BNO085 caveat: https://learn.adafruit.com/adafruit-9-dof-orientation-imu-fusion-breakout-bno085

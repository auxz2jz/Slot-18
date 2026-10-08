# First hardware test — 7-inch ESP32-S3 and one LD2450
**Prepared:** 2026-10-07. **Hardware state:** hypothetical; no user-confirmed device test. **Firmware candidate:** v0.2.0. **Last user-verified baseline:** none.

## A. What you need on the workbench
- Waveshare ESP32-S3-Touch-LCD-7 with capacitive touch (the exact 800x480 model, not 7B).
- USB data cable to Windows PC for firmware upload; matching USB-C plug.
- Regulated 5V USB wall charger with ample headroom for the display (consider 5V/3A unit), plugged into a normal wall outlet for bench use **after flashing**. Do not connect AC mains to electronics.
- One radar later: Hi-Link LD2450, compatible 4-wire UART leads and power as defined on its module's actual connector.
- No UART expander, battery, GPS, thermal sensor, Wi-Fi/router, or mesh station is needed for the first screen test.

## B. First run the manufacturer's unmodified screen demonstration
1. Confirm rear-board model and hardware revision. Check touchscreen model, 16 MB flash, 8 MB PSRAM and matching demo.
2. On PC, install VS Code + official Espressif IDF extension and matching ESP-IDF 5.x (project CI initially uses v5.4.2).
3. Refer to https://docs.waveshare.com/ESP32-S3-Touch-LCD-7 and the vendor ESP-IDF LVGL v9 project:
   https://github.com/waveshareteam/ESP32-S3-Touch-LCD-7/tree/main/examples/ESP-IDF/09_lvgl_v9_demo
4. Connect the correct **programming USB-C** port to PC, select actual COM port. If port is missing, follow Waveshare BOOT/RESET procedure, not random GPIO rewiring.
5. Build and flash the Waveshare example *unchanged* first; verify colors, screen, touch, and serial output. Capture results. This creates a trusted physical reference but is not our product's verified version.

## C. Next test our Slot-18 program
Source: firmware/esp-idf
- Open its directory as an ESP-IDF project.
- Run idf.py set-target esp32s3, then idf.py build.
- Flash using the actual port after a successful build. Example Windows command: idf.py -p COM7 flash monitor (replace COM7; this is NOT a presumed port).
- A photo or user observation should confirm that the top banner says **SIMULATED ONLY**, the green dot moves, and Facing/Pause touch buttons respond.
- Tap TEST then do the 3 guided steps. Use PASS or FAIL honestly for each; these are manual visual checks, not automatic proof.
- Press LOGS while serial monitor is connected to inspect structured diagnostic messages stored in the bounded NVS event ring. Note the initial timestamps are *uptime*, not UTC.
- If any failure happens: report first actual error in serial output, board selection, exact compiled tool versions, the on-screen behavior and reproduction step. Stop before replacing working board code.

## D. Only after the screen test passes: connect one LD2450
- Power everything down before adding wiring.
- Verify the radar's exact VCC requirement and signal-level voltage from its datasheet and connector marking. Do not assume the board's VCC or supply is interchangeable with 3.3V TX/RX logic.
- UART data topology: LD2450 TX -> chosen ESP32 RX, LD2450 RX -> chosen ESP32 TX; GNDs common; power via verified rated supply.
- Waveshare UART1 USB and UART2 HY2.0 header share **ESP32 UART0 GPIO43/44** selected by switch; they are not independent. If UART0 is occupied by LD2450, serial logging / programming could conflict. Firmware v0.3 will explicitly decide whether to move console to native USB CDC, use a bridge, or use another verified interface; we will not blindly pick screen GPIOs.
- Vendor ESP-IDF UART example's default GPIO4/GPIO5 values cannot be reused blindly: GPIO4 is related to touch and GPIO5 is LCD DE on this device.
- Configure LD2450 UART protocol and decode framed packets; start by showing **real measured distance and XY position**, then add reliability/stale indicators. Validate ranges using measured reference distances.
- Presence-only radar cannot supply exact point coordinates; add C4001/C4002 only after LD2450 tested alone.

## E. How the user reports a guided test
- Firmware candidate version/commit and date
- Board model and revision; ESP-IDF compiler version
- PASS / FAIL for power/boot, display, touch, simulator dot, heading, pause, logs
- First error message or symptom; ideally full diagnostic excerpt or exported file
- Only user-confirmed physical operation moves Last Verified Baseline.

## F. Networking and batteries come later
For bench tests, main unit is USB wall powered. In the remote phase, ESP-NOW/Wi-Fi direct links will precede mesh routing; battery power and relay awake time will be measured in real tests. Never assume through-wall occupancy detection or real-time human identity.

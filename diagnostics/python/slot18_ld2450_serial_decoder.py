"""Read-only HLK-LD2450 256000-baud, 30-byte frame decoder for Windows.
Install: py -m pip install pyserial
Run:     py slot18_ld2450_serial_decoder.py COM4
No commands are ever written to the radar.
"""
import sys
import time

HEADER = bytes.fromhex("AA FF 03 00")
FOOTER = bytes.fromhex("55 CC")
FRAME_SIZE = 30

def signed_magnitude(value):
    magnitude = value & 0x7FFF
    return magnitude if value & 0x8000 else -magnitude

def decode_targets(frame):
    if len(frame) != 30 or frame[:4] != HEADER or frame[-2:] != FOOTER:
        raise ValueError("Invalid frame")
    results = []
    for index in range(3):
        item = frame[4 + index * 8:12 + index * 8]
        if item == bytes(8):
            continue
        x = signed_magnitude(int.from_bytes(item[:2], "little"))
        y = signed_magnitude(int.from_bytes(item[2:4], "little"))
        speed = signed_magnitude(int.from_bytes(item[4:6], "little"))
        resolution = int.from_bytes(item[6:8], "little")
        results.append((index + 1, x, y, speed, resolution))
    return results

def self_test():
    frame = bytes.fromhex("AA FF 03 00 0E 03 B1 86 10 00 40 01 "
                          "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 CC")
    assert len(frame) == FRAME_SIZE
    assert decode_targets(frame) == [(1, -782, 1713, -16, 320)]
    print("LD2450 manufacturer test frame: PASS")

def main(port):
    try:
        import serial
    except ImportError:
        print("Install the serial library first: py -m pip install pyserial")
        return 2
    print(f"Reading LD2450 from {port} @ 256000 (read-only). Ctrl+C to stop.")
    print("Close PuTTY and other serial programs before starting.\n")
    try:
        link = serial.Serial(port, 256000, timeout=0.25)
    except serial.SerialException as error:
        print(f"Could not open {port}: {error}")
        return 1
    pending = bytearray()
    frames, discarded, received = 0, 0, 0
    last_good = last_display = time.monotonic()
    last_warning = 0
    try:
        with link:
            while True:
                chunk = link.read(link.in_waiting or 1)
                if chunk:
                    received += len(chunk)
                    pending.extend(chunk)
                if len(pending) > 4096:
                    discarded += len(pending) - 256
                    del pending[:-256]
                while len(pending) >= 4:
                    offset = pending.find(HEADER)
                    if offset < 0:
                        discarded += max(0, len(pending) - 3)
                        del pending[:-3]
                        break
                    if offset:
                        discarded += offset
                        del pending[:offset]
                    if len(pending) < FRAME_SIZE:
                        break
                    if pending[28:30] != FOOTER:
                        discarded += 1
                        del pending[:1]
                        continue
                    frame = bytes(pending[:FRAME_SIZE])
                    del pending[:FRAME_SIZE]
                    frames += 1
                    now = time.monotonic()
                    last_good = now
                    if now - last_display >= 0.5:
                        targets = decode_targets(frame)
                        if targets:
                            detail = " | ".join(
                                f"Target {i}: X={x/1000:+.2f}m Y={y/1000:+.2f}m Speed={speed:+d}cm/s Gate={gate}mm"
                                for i, x, y, speed, gate in targets)
                        else:
                            detail = "No targets tracked"
                        print(f"Frame {frames}: {detail}", flush=True)
                        last_display = now
                now = time.monotonic()
                if now - last_good > 3 and now - last_warning > 3:
                    print(f"Waiting for valid frames (read {received} bytes, discarded {discarded})", flush=True)
                    last_warning = now
    except KeyboardInterrupt:
        print(f"\nStopped: {frames} valid frames, {discarded} discarded bytes")
        return 0
    except serial.SerialException as error:
        print(f"Serial read error: {error}")
        return 1

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--self-test":
        self_test()
    else:
        sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "COM4"))

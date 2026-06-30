#!/usr/bin/env python3
import serial
import time
from datetime import datetime

PORT = "/dev/ttyUSB0"   # Pi UART alias
BAUD = 9600
TIMEOUT = 1.0

def main():
    # Open UART
    ser = serial.Serial(
        PORT,
        BAUD,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=TIMEOUT
    )

    print(f"[OK] Listening on {PORT} @ {BAUD} baud (Ctrl+C to stop).")
    print("Tip: If dsPIC sends only 'L' without newline, you may not see 'readline' output.\n")

    try:
        while True:
            # Read any available bytes (non-blocking-ish because of timeout)
            data = ser.read(256)
            if data:
                ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]

                # Print as text if possible; otherwise show hex too
                text = data.decode("utf-8", errors="replace")
                # Show printable characters clearly; keep it simple
                print(f"{ts}  RX: {text!r}   ({len(data)} bytes)")
            else:
                # no data received in this timeout window
                pass

    except KeyboardInterrupt:
        print("\n[STOP] Exiting.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()

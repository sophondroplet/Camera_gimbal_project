UART Music Streamer
===================

This folder is intended to hold small host-side utilities to interact with the gimbal firmware over UART. The firmware accepts ASCII lines of the form `P,D\n` to update PID gains live and echoes received commands. It also toggles torque for a music generator exposed through `TIM6`.

Planned / existing scripts
- `music_streamer.py` — (currently empty) a simple Python/pyserial script would be useful to send sequences of PID updates or play notes by changing `prescaler` values over UART.

How to create a quick helper (example)
1. Install Python 3.8+ and pyserial: `pip install pyserial`
2. Use a script like below (replace COM port / baud rate):

```python
import serial
import time

ser = serial.Serial('COM3', 115200, timeout=1)

# Set P and D
ser.write(b'0.5,0.01\n')
time.sleep(0.1)

# Play a sequence of gains
for p in [0.2, 0.4, 0.6]:
    ser.write(f"{p},0.01\n".encode())
    time.sleep(0.5)

ser.close()
```

If you want, I can implement `music_streamer.py` here to provide a friendly CLI for sending gains, streaming note sequences, and logging MCU responses.

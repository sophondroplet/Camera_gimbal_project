UART Music Streamer
===================

This folder contains `music_streamer.py`, a host-side utility that converts MIDI files (or simple test scales) into realtime motor frequency/torque commands and streams them to the STM32 firmware over a serial link.

What the script does
- Parses MIDI files using `mido` and converts note_on/note_off events into a time-series of (frequency, amplitude) samples.
- Streams those samples at a configurable sample rate to the MCU using an ASCII protocol: `FREQ:<frequency>,AMP:<amplitude>\n` (example: `FREQ:440.00,AMP:0.150\n`).
- Provides a `--scale` test mode to play a C major scale for quick hardware verification.

Dependencies
- Python 3.8+
- pyserial
- mido
- numpy

Install (Windows PowerShell example):

```powershell
python -m pip install --user pyserial mido numpy
```

Usage
- Play a MIDI file over serial:

```powershell
python .\music_streamer.py --port COM3 --baud 115200 path\to\file.mid
```

- Play a test scale:

```powershell
python .\music_streamer.py --port COM3 --baud 115200 --scale
```

Serial protocol emitted by the script
- ASCII line per sample terminated with `\n`.
- Format: `FREQ:<frequency_Hz>,AMP:<amplitude_0_to_1>\n`
- Example: `FREQ:440.00,AMP:0.150\n`

Compatibility and firmware notes
- The `music_streamer.py` script sends `FREQ/AMP` command lines. The firmware in `Software/STM32G431CUB6_firmware` originally parsed `P,D\n` for PID tuning. To use `music_streamer.py` directly you must run firmware that accepts `FREQ/AMP` lines or add a small firmware parser that maps frequency/amplitude to the motor control variables.

Suggested minimal firmware parsing approach
1) Add globals to share the host command (example in a new header `Core/Inc/music_cmd.h`):

```c
// Example globals
extern float host_command_frequency_hz; // Hz
extern float host_command_amplitude;    // 0..1
extern volatile uint8_t host_command_new; // flag set when new command arrives
```

2) In your UART handler (or a small command processing task) parse `FREQ:` lines. Example pseudocode for a simple parser:

```c
void process_serial_line(char *line){
    float f = 0.0f, a = 0.0f;
    if (sscanf(line, "FREQ:%f,AMP:%f", &f, &a) == 2){
        if (f < 0.0f) f = 0.0f;
        if (a < 0.0f) a = 0.0f;
        if (a > 1.0f) a = 1.0f;

        host_command_frequency_hz = f;
        host_command_amplitude = a;
        host_command_new = 1;
    }
}
```

3) Consume `host_command_new` in a safe, periodic context and map frequency to a timer prescaler or to a torque variable. Be conservative with amplitude and frequency limits when driving a motor.

Safety
- The script includes host-side frequency/amplitude constraints, but you should also enforce safe limits in firmware and test with the motor disconnected first.

If you want, I can add the small MCU-side parser and a safe mapping from frequency to `TIM6` prescaler or torque variable — say the word and I will implement it.

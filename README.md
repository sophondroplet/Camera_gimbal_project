## Camera Gimbal Project

This repository contains the mechanical CAD, drawings and embedded firmware for a small 2-axis/3-phase BLDC camera gimbal prototype. The project uses an STM32G431 MCU to read an MPU6050 IMU, an AS5048A-style PWM encoder input for rotor position, and drives a 3-phase BLDC (FOC-style) motor using TIM1 complementary PWM outputs.

Folders
- `CAD_files/` – STEP files and drawings for the 3D printed / machined parts and full assembly.
- `assets/` – photos, renders and PCB images used in documentation.
- `Software/STM32G431CUB6_firmware/` – STM32CubeIDE / Makefile firmware project. Contains HAL drivers, MPU6050 code, motor control and UART interface.
- `Software/UART_music_streamer/` – host-side Python utility (`music_streamer.py`) to convert MIDI files or a test scale to motor frequency/torque commands and stream them to the MCU over serial.

High level behaviour
- The MCU reads orientation (roll/pitch/yaw) from the MPU6050 via I2C using DMA.
- An absolute rotor position source (AS5048A PWM-style duty encoder) is read using TIM input capture on `TIM2`.
- A simple PID computes a torque command to follow the IMU-derived pitch setpoint; torque/d (direct/quadrature frame) are converted to 3-phase PWM using inverse Park/Clarke then written to `TIM1` CCR registers.
- UART is used for tuning PID gains at runtime by sending ASCII `P,D\n` lines (see firmware README). Additionally, the host audio streamer sends `FREQ/AMP` lines to play music on the motor (see `Software/UART_music_streamer/README.md`).

Quick start (recommended)
1. Install STM32CubeIDE (recommended) and the STM32G4 device support package.
2. Open the firmware project: `Software/STM32G431CUB6_firmware/gorgon test.ioc` or open the project workspace in STM32CubeIDE.
3. Build the project (Project → Build). Connect an ST-Link and flash using the IDE or STM32CubeProgrammer.

Command-line build & flash (advanced)
- This project contains generated makefiles in the `Debug/` folder. You can build with an ARM GNU toolchain (arm-none-eabi + make). Example (PowerShell):

```powershell
cd .\Software\STM32G431CUB6_firmware\Debug
make all
```

- For flashing, use STM32CubeProgrammer (GUI) or the CLI: point it at the produced ELF/BIN in `Debug/`.

UART protocols in this repo
- PID tuning: send ASCII `P,D\n` lines (example `0.5,0.01\n`) — original firmware parsing behaviour.
- Motor audio streaming: `music_streamer.py` sends `FREQ:<Hz>,AMP:<0..1>\n` lines (example `FREQ:440.00,AMP:0.150\n`). The firmware must be updated to accept `FREQ/AMP` lines or a host proxy must translate them.

Safety and notes
- This firmware drives a BLDC motor. Only connect power and the motor after you have verified wiring and the gate driver. Use low gain values for initial tests and keep motor disconnected while verifying encoder/IMU readings.
- The firmware contains HAL drivers (STM32 HAL and CMSIS). See `Software/STM32G431CUB6_firmware/Drivers/` for license files (Apache/BSD as appropriate).

Resources and where to look next
- Firmware source: `Software/STM32G431CUB6_firmware/Core/Src` and `Core/Inc`.
- IMU code: `MPU6050.c`, `MPU6050_D.c` and headers.
- Motor/FOC code & PID: `main.c`, `music.c`, and the BLDC conversion helpers.
- Host audio streamer: `Software/UART_music_streamer/music_streamer.py` and this folder's README.

If you want, I can also:
- Add the small MCU-side parser for `FREQ/AMP` and a conservative mapping to `TIM6` prescaler or `torque` so `music_streamer.py` works out-of-the-box.
- Populate inline documentation in key source files or add a LICENSE and CONTRIBUTING guide.

License
- The repository contains third-party code (STM32Cube HAL, CMSIS) under their respective license texts included in `Software/STM32G431CUB6_firmware/Drivers/`. The project code inside `Core/` is provided AS-IS unless you tell me otherwise; add or edit a LICENSE file if you want an explicit project license.

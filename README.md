## Camera Gimbal Project

This repository contains the mechanical CAD, drawings and embedded firmware for a small 2-axis/3-phase BLDC camera gimbal prototype. The project uses an STM32G431 MCU to read an MPU6050 IMU, an AS5048A-style PWM encoder input for rotor position, and drives a 3-phase BLDC (FOC-style) motor using TIM1 complementary PWM outputs.

Folders
- `CAD_files/` – STEP files and drawings for the 3D printed / machined parts and full assembly.
- `assets/` – photos, renders and PCB images used in documentation.
- `Software/STM32G431CUB6_firmware/` – STM32CubeIDE / Makefile firmware project. Contains HAL drivers, MPU6050 code, motor control and UART interface.
- `Software/UART_music_streamer/` – helper Python scripts (a simple serial client) to interact with the MCU over UART.

High level behaviour
- The MCU reads orientation (roll/pitch/yaw) from the MPU6050 via I2C using DMA.
- An absolute rotor position source (AS5048A PWM-style duty encoder) is read using TIM input capture on `TIM2`.
- A simple PID computes a torque command to follow the IMU-derived pitch setpoint; torque/d (direct/quadrature frame) are converted to 3-phase PWM using inverse Park/Clarke then written to `TIM1` CCR registers.
- UART is used for tuning PID gains at runtime by sending ASCII "P,D\n" lines (see firmware README below).

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

UART protocol
- Send ASCII lines terminated with `\n`.
- To set PID gains send: `<P_GAIN>,<D_GAIN>\n` for example: `0.5,0.01\n` — the firmware will parse the two floats and update the internal `P` and `D` gains and echo the received line.

Safety and notes
- This firmware drives a BLDC motor. Only connect power and the motor after you have verified wiring and the gate driver. Use low gain values for initial tests and keep motor disconnected while verifying encoder/IMU readings.
- The firmware contains HAL drivers (STM32 HAL and CMSIS). See `Software/STM32G431CUB6_firmware/Drivers/` for license files (Apache/BSD as appropriate).

Resources and where to look next
- Firmware source: `Software/STM32G431CUB6_firmware/Core/Src` and `Core/Inc`.
- IMU code: `MPU6050.c`, `MPU6050_D.c` and headers.
- Motor/FOC code & PID: `main.c`, `music.c`, and the BLDC conversion helpers.
- CAD drawings: `CAD_files/drawings/Camera_gimbal_drawing.pdf`

If you want, I can also:
- Add a small serial helper script to set PID gains from the host (Python + pyserial).
- Populate inline documentation in key source files.

License
- The repository contains third-party code (STM32Cube HAL, CMSIS) under their respective license texts included in `Software/STM32G431CUB6_firmware/Drivers/`. The project code inside `Core/` is provided AS-IS unless you tell me otherwise; add or edit a LICENSE file if you want an explicit project license.

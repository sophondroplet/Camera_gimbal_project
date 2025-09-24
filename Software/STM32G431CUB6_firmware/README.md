STM32G431CUB6 Firmware
======================

This folder contains an STM32CubeIDE project (Gorgon test) which implements motor control and IMU-based stabilization for the camera gimbal prototype. The project targets an STM32G431CBU6 microcontroller and uses HAL drivers with some custom code for the MPU6050 and motor output.

What it implements
- MPU6050 IMU driver (I2C, DMA read) with simple complementary filter fusion (see `Core/Src/MPU6050.c` and `MPU6050_D.c`).
- PWM encoder reading using TIM2 input capture to measure duty cycle and derive absolute rotor angle (used like an AS5048A PWM output).
- Simple PID controller on pitch/angle that computes a torque command which is converted to 3-phase PWM values using inverse Park/Clarke (`main.c`).
- UART-based live PID tuning via ASCII `P,D\n` commands (existing behaviour).
- Optional host-driven audio: the host `music_streamer.py` can stream `FREQ/AMP` commands to the MCU to play music on the motor (see `Software/UART_music_streamer/` README for host-side details). The firmware must accept `FREQ:...,AMP:...` lines or you must add a small parser to convert these commands into internal variables.
- Small music/tone generator that toggles torque with `TIM6` to play notes (see `Core/Inc/music.h` and `Core/Src/music.c`).

Wiring (conceptual)
- Power: motor driver/gate driver and MCU power rails. Ensure correct supply voltages and common grounds.
- TIM1 PWM outputs (CH1/CH2/CH3 and complementary pins) go to the motor driver inputs. Confirm pin mapping in `mx` generated init code.
- TIM2 CH1 (rising capture) and CH2 (falling capture) read the PWM-encoded rotor position (device that outputs PWM duty proportional to angle, typically AS5048A in PWM mode).
- MPU6050: I2C SDA/SCL to `I2C1` pins, interrupt pin to `IMU_INT` GPIO (used to detect data ready). Pull-ups as required.
- UART: USART2 TX/RX used for debug and PID tuning. Connect to USB-serial or ST-Link VCP.

Build & flash
- Recommended: Open the project in STM32CubeIDE (the `.ioc` file is in the folder) and use the IDE to build/flash.
- Command-line: there are makefiles in `Debug/`. Use an ARM GCC toolchain compatible with the flags in the makefile (arm-none-eabi-gcc, make). Example (PowerShell):

```powershell
cd .\Software\STM32G431CUB6_firmware\Debug
make all
```

- The resulting ELF/BIN will be under `Debug/` and can be flashed with STM32CubeProgrammer or the IDE.

Runtime / Tuning
- By default the firmware listens on USART2 for ASCII lines ending in `\n`.
- Send `P,D\n` where `P` and `D` are floating point numbers (no spaces required). Example: `0.5,0.02\n`.
- The firmware echoes the received string and updates the running P and D gains immediately.
- To support `music_streamer.py` you can add a small `FREQ/AMP` parser (see the UART_music_streamer README for a suggested parsing approach). The parsed frequency and amplitude should be safely clamped in firmware before use.

Calibration and safety checklist
- Start with motors disconnected or use a bench load.
- Verify IMU readings first: with motor power off, observe UART debug prints (if enabled) for pitch/roll/yaw values.
- Verify encoder input: check TIM capture values and derived `angle` (look for a 0..2*pi range mapping).
- Use small `P` gains initially and confirm the movement direction is correct.
- Implement emergency stop or physical kill switch before powering motors under load.

Files of interest
- `Core/Inc` – headers for project variables and constants.
- `Core/Src/main.c` – main loop, PID and motor output conversions.
- `Core/Src/MPU6050.c`, `MPU6050_D.c` – IMU drivers.
- `Core/Inc/music.h`, `Core/Src/music.c` – simple tone generator used for debugging.

Licenses
- STM32 HAL and CMSIS code under their included licenses (see `Drivers/`).


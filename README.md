# OmniWrist OS ⌚

**OmniWrist OS** is a modular, open-source smartwatch operating system built around the Espressif ESP32-S3 microcontroller.

> **Current Status: Phase 1 (Proof of Concept / Hardware Validation)**
> The project is currently in the prototyping phase using a development board and jumper wires. The primary goal of this phase is to validate shared-bus hardware integration (SPI/I2C), develop stable drivers with Digital Signal Processing (DSP), and build a robust, object-oriented software architecture. Once the software foundation and hardware constraints are fully validated, Phase 2 will involve designing a custom wearable PCB.

## 🏗️ Software Architecture

The OS is built in C++ using **PlatformIO** and heavily relies on Object-Oriented Programming (OOP) principles to keep the codebase clean, scalable, and memory-efficient.

- **Modular UI System:** The interface is built on a `UI_Card` paradigm. Each application or sensor view (e.g., `HeartRateCard`, `PedometerCard`) manages its own lifecycle (`begin`, `onShow`, `onUpdate`, `onHide`) and sleep-blocking logic.
- **Interrupt-Driven Power Management:** To maximize battery life, polling is avoided where possible. The system wakes up from deep sleep via hardware interrupts from the touch screen, rotary encoder, or IMU (Tilt-to-Wake).
- **Global Managers:** Core functionalities are decoupled into independent managers (`DisplayManager`, `MotionManager`, `BacklightManager`, `EncoderManager`).

## ⚙️ Hardware Stack

The current PoC validates the coexistence of the following components on shared SPI and I2C buses:

| Component          | Interface  | Function                                        |
| :----------------- | :--------- | :---------------------------------------------- |
| **ESP32-S3**       | Core       | Main processor (Dual-core, WiFi/BLE enabled)    |
| **TFT Display**    | SPI        | Main visual interface                           |
| **XPT2046**        | SPI        | Touch screen controller                         |
| **MPU6050**        | I2C        | 6-Axis IMU (Tilt-to-wake & Pedometer)           |
| **BME280**         | I2C        | Environmental sensor (Temp, Humidity, Pressure) |
| **MAX30102**       | I2C        | Biometric sensor (Heart Rate & SpO2)            |
| **BH1750**         | I2C        | Ambient light sensor (Auto-brightness)          |
| **Rotary Encoder** | GPIO (IRQ) | Physical navigation & wake button               |
| **TP4056**         | Power      | Li-Po/Li-Ion charging and protection            |

_For exact pin mappings, please refer to [PINOUT.md](PINOUT.md)._

## ✨ Key Features (Implemented)

- **Advanced Motion Processing:** Custom software low-pass filters and hysteresis for accurate step counting (Pedometer) using the MPU6050 raw accelerometer data.
- **Biometric DSP:** Custom digital signal processing (DC removal, adaptive thresholding) to extract stable BPM readings from the MAX30102 raw optical data.
- **Adaptive Display:** The BH1750 sensor continuously monitors ambient light and smoothly adjusts the TFT backlight via PWM for optimal visibility and battery savings.
- **Smart Sleep:** System automatically enters deep sleep after inactivity, unless a specific UI Card (like active heart rate measurement) overrides the sleep timer.
- **Dual Navigation:** Seamless switching between touch gestures (swipes/taps) and physical rotary encoder inputs.

## 🚀 Future Roadmap (Phase 2 & Beyond)

- [ ] I2S Audio integration (MAX98357A Amplifier + INMP441 Microphone).
- [ ] Centralized App Drawer / Main Menu via Rotary Encoder.
- [ ] Bluetooth Low Energy (BLE) connection to smartphone for notifications.
- [ ] Add battery with power protection.
- [ ] Add switches for control.
- [ ] Custom PCB design and 3D-printed wearable enclosure.
- [ ] Advanced SpO2 calculation algorithms.

---

_The current development heavily embraces 'vibe coding' (rapid prototyping) to quickly validate hardware integration and achieve a working Proof of Concept. Once Phase 1 is complete, the entire codebase will be refactored and rewritten from the ground up, with a deep engineering analysis of every module._

# OmniWrist OS - Hardware Pinout Matrix

this document serves as the master reference for all physical connections on the esp32-s3.
when adding new modules, always verify against this list to avoid bus collisions.

## Core Buses (Shared)

| Pin (GPIO) | Name       | Bus Type | Description                                        |
| :--------- | :--------- | :------- | :------------------------------------------------- |
| **11**     | `SPI_MOSI` | SPI      | shared master-out line (display, touch, sd card)   |
| **12**     | `SPI_SCK`  | SPI      | shared clock line (display, touch, sd card)        |
| **13**     | `SPI_MISO` | SPI      | shared master-in line (touch, sd card)             |
| **4**      | `I2C_SDA`  | I2C      | shared data line (max30102, bme280, mpu6050, rtc)  |
| **5**      | `I2C_SCL`  | I2C      | shared clock line (max30102, bme280, mpu6050, rtc) |

## Dedicated Chip Selects (CS) & Interrupts

| Pin (GPIO) | Name       | Target Module | Description                                    |
| :--------- | :--------- | :------------ | :--------------------------------------------- |
| **10**     | `TFT_CS`   | Display       | display chip select (active low)               |
| **7**      | `TOUCH_CS` | Touch         | xpt2046 touch chip select (active low)         |
| **6**      | `SD_CS`    | Storage       | microsd card chip select (active low)          |
| **8**      | `IMU_INT`  | Sensor        | hardware interrupt from mpu6050 (tilt-to-wake) |

## Display Specific

| Pin (GPIO) | Name        | Target Module | Description                                       |
| :--------- | :---------- | :------------ | :------------------------------------------------ |
| **9**      | `TFT_DC`    | Display       | data/command toggle                               |
| **14**     | `TFT_RST`   | Display       | hardware reset                                    |
| **21**     | `TFT_BL`    | Display       | backlight control (use pwm for brightness)        |
| **2**      | `TOUCH_IRQ` | Touch         | hardware interrupt from touch screen (active low) |

## I2S Audio System (Proposed)

| Pin (GPIO) | Name       | Target Module | Description                             |
| :--------- | :--------- | :------------ | :-------------------------------------- |
| **15**     | `I2S_BCLK` | Audio         | bit clock (shared amp & mic)            |
| **16**     | `I2S_LRC`  | Audio         | word select / left-right clock (shared) |
| **17**     | `I2S_DOUT` | Audio         | data out (to max98357a amplifier)       |
| **18**     | `I2S_DIN`  | Audio         | data in (from inmp441 microphone)       |

## Power & Excluded Pins

- **3V3 / GND:** primary logic power.
- **5V / VBUS:** external power input (currently floating/unusable on this specific board).
- **GPIO 0, 3, 45, 46:** strapping pins (do not use for i/o).
- **GPIO 19, 20:** reserved for native usb d- / d+.

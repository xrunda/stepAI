# Button Demo Example

| Board             | Support Status |
| ----------------- | -------------- |
| ESP32-S3-BOX      | NO             |
| ESP32-S3-BOX-Lite | NO             |
| ESP32-S3-BOX-3    | YES            |

This example demonstrates how to control a GPIO pin on the ESP32-S3-BOX-3-DOCK PMOD interface using a touch switch on the LCD screen.

## Features

- **Touch Switch UI**: A switch button displayed on the LCD screen
- **GPIO Control**: Controls the GPIO pin state (HIGH/LOW) on DOCK's PMOD interface
- **Real-time Feedback**: Visual feedback showing the current GPIO state

## Hardware Required

* An ESP32-S3-BOX-3 development board
* ESP32-S3-BOX-3-DOCK (for PMOD interface access)
* A USB Type-C cable for power supply and programming
* Optional: LED or other device to connect to the controlled GPIO pin

## GPIO Pin Used

This example uses **PMOD2 row1[0]** (the first pin of PMOD2 interface) as the control pin. You can modify the code to use any other PMOD pin by changing:

```c
g_control_pin = brd->PMOD2->row1[0];  // Change to PMOD1->row1[0], PMOD2->row2[1], etc.
```

## Pinout Reference

The DOCK has two PMOD interfaces (PMOD1 and PMOD2), each with:
- **row1**: 4 GPIO pins (row1[0] to row1[3])
- **row2**: 4 GPIO pins (row2[0] to row2[3])

Total: 16 programmable GPIO pins available on the DOCK.

## How to Use

### Build and Flash

1. Navigate to the example directory:
```bash
cd examples/button_demo
```

2. Set the target:
```bash
idf.py set-target esp32s3
```

3. Build and flash:
```bash
idf.py build flash monitor
```

Once a complete flash process has been performed, you can use `idf.py app-flash monitor` to reduce the flash time.

(To exit the serial monitor, type `Ctrl-]`)

### Usage

1. After flashing, the LCD screen will display a switch button
2. Touch the switch to toggle the GPIO pin state:
   - **ON**: GPIO pin set to HIGH (3.3V)
   - **OFF**: GPIO pin set to LOW (0V)
3. The state label will update to show "ON" (green) or "OFF" (white)
4. Connect an LED or other device to the PMOD pin to see the effect

## Example Connection

To test with an LED:
1. Connect LED anode (long leg) to the controlled GPIO pin (PMOD2 row1[0])
2. Connect LED cathode (short leg) to GND (available on PMOD interface)
3. Add a current-limiting resistor (220Ω recommended) in series

## Customization

### Change the Control Pin

Edit `button_demo.c` and modify the GPIO initialization:

```c
// Use PMOD1 row1[0] instead
g_control_pin = brd->PMOD1->row1[0];

// Or use PMOD2 row2[1]
g_control_pin = brd->PMOD2->row2[1];
```

### Change Initial State

Modify the initial GPIO state in `gpio_init()`:

```c
// Set initial state to HIGH
gpio_set_level(g_control_pin, 1);
g_pin_state = true;
```

## Troubleshooting

* **GPIO not working**: Make sure ESP32-S3-BOX-3 is properly connected to DOCK
* **Switch not responding**: Check if touch screen is working properly
* **No visual feedback**: Verify display initialization in serial monitor logs

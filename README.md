# StepAI Example

| Board             | Support Status |
| ----------------- | -------------- |
| ESP32-S3-BOX      | NO             |
| ESP32-S3-BOX-Lite | NO             |
| ESP32-S3-BOX-3    | YES            |

This example demonstrates how to fetch step data from a REST API, parse JSON response, calculate total steps, and display the information on the LCD screen with a beautiful and minimalist UI design.

## Features

- **HTTP Client**: Fetches step data from REST API
- **JSON Parsing**: Parses JSON response using cJSON library
- **Step Calculation**: Accumulates all steps from 24-hour data
- **Beautiful UI**: Minimalist and elegant LVGL interface displaying:
  - Date of the step data
  - Total step count
  - Connection status

## Hardware Required

* An ESP32-S3-BOX-3 development board
* A USB Type-C cable for power supply and programming
* WiFi network access

## API Endpoint

The example fetches data from:
```
https://res.xrunda.com/ai-test/step.json
```

The API returns JSON data with the following structure:
```json
{
  "status": "success",
  "message": "拉取2025-12-13全天步数数据成功",
  "data": [
    {
      "_id": 501,
      "_begin_time": 1755068400000,
      "_end_time": 1755068700000,
      "_mode": 2,
      "_steps": 320
    },
    ...
  ],
  "total_count": 31,
  "date": "2025-12-13",
  "sort_order": "_id asc"
}
```

## Configuration

### WiFi Configuration

Before building, you need to configure WiFi credentials. You can do this in two ways:

**Method 1: Using menuconfig (Recommended)**
```bash
idf.py menuconfig
```
Navigate to:
- `Component config` → `Example Configuration` → Set WiFi SSID and Password

**Method 2: Edit the source code**
Edit `main/stepAI.c` and modify:
```c
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

## How to Use

### Build and Flash

1. Navigate to the example directory:
```bash
cd examples/stepAI
```

2. Configure WiFi (if using menuconfig):
```bash
idf.py menuconfig
```

3. Set the target:
```bash
idf.py set-target esp32s3
```

4. Build and flash:
```bash
idf.py build flash monitor
```

Once a complete flash process has been performed, you can use `idf.py app-flash monitor` to reduce the flash time.

(To exit the serial monitor, type `Ctrl-]`)

### Usage

1. After flashing, the device will:
   - Initialize WiFi and connect to the configured network
   - Display "初始化中..." (Initializing...) on screen
   - Once connected, fetch step data from the API
   - Display the date and total step count

2. The UI will show:
   - **Date**: The date of the step data (e.g., "2025-12-13")
   - **Total Steps**: Accumulated steps from all records in 24 hours
   - **Status**: Connection and data fetch status

3. Data refresh:
   - The device automatically refreshes data every 5 minutes
   - Status indicator shows:
     - Yellow: "正在获取数据..." (Fetching data...)
     - Green: "数据已更新" (Data updated)
     - Red: "获取失败" (Fetch failed)

## UI Design

The UI features a minimalist and elegant design:
- **Gradient Background**: Dark blue gradient (1a1a2e → 16213e)
- **Date Display**: Cyan-colored date in a rounded container
- **Steps Display**: Large green number in a highlighted container with border
- **Status Indicator**: Color-coded status messages at the bottom

## Troubleshooting

* **WiFi not connecting**: 
  - Check WiFi credentials in menuconfig or source code
  - Ensure WiFi network is 2.4GHz (ESP32-S3 doesn't support 5GHz)
  - Check serial monitor for connection errors

* **API request fails**:
  - Verify internet connectivity
  - Check if the API endpoint is accessible
  - Review serial monitor logs for HTTP errors

* **No data displayed**:
  - Wait for WiFi connection (status will show "正在获取数据...")
  - Check serial monitor for JSON parsing errors
  - Verify API response format

* **Display not working**:
  - Ensure display initialization completed
  - Check serial monitor for LVGL errors

## Customization

### Change Refresh Interval

Modify the refresh delay in `refresh_data_task()`:
```c
// Refresh every 5 minutes (default)
vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));

// Change to 1 minute
vTaskDelay(pdMS_TO_TICKS(60 * 1000));
```

### Change API URL

Modify the API endpoint:
```c
#define STEP_API_URL "https://your-api-url.com/step.json"
```

### Customize UI Colors

Edit the color values in `create_ui()`:
```c
lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x1a1a2e), LV_PART_MAIN);
lv_obj_set_style_text_color(g_steps_label, lv_color_hex(0x00FF88), LV_PART_MAIN);
```


/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include <string.h>

static const char *TAG = "stepAI";

#define STEPS_PER_MINUTE 1000  // 1000步 = 1分钟电量

// Data
static uint32_t g_total_steps = 5000;
static uint32_t g_exchanged_minutes = 0;
static uint32_t g_exchangeable_minutes = 0;

// UI elements
static lv_obj_t *g_exchange_value_label = NULL;
static lv_obj_t *g_total_steps_label = NULL;
static lv_obj_t *g_exchanged_minutes_label = NULL;
static lv_obj_t *g_status_label = NULL;

// Calculate exchangeable minutes
static void calculate_exchangeable(void)
{
    if (g_total_steps >= g_exchanged_minutes * STEPS_PER_MINUTE) {
        g_exchangeable_minutes = (g_total_steps - g_exchanged_minutes * STEPS_PER_MINUTE) / STEPS_PER_MINUTE;
    } else {
        g_exchangeable_minutes = 0;
    }
}

// Update UI with current values
static void update_ui(void)
{
    if (g_exchange_value_label) {
        char value_text[16];
        snprintf(value_text, sizeof(value_text), "%lu", g_exchangeable_minutes);
        lv_label_set_text(g_exchange_value_label, value_text);
    }
    
    if (g_total_steps_label) {
        char steps_text[32];
        snprintf(steps_text, sizeof(steps_text), "Total: %lu steps", g_total_steps);
        lv_label_set_text(g_total_steps_label, steps_text);
    }
    
    if (g_exchanged_minutes_label) {
        char exchanged_text[32];
        snprintf(exchanged_text, sizeof(exchanged_text), "Exchanged: %lu min", g_exchanged_minutes);
        lv_label_set_text(g_exchanged_minutes_label, exchanged_text);
    }
}

// Exchange button event callback
static void exchange_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_exchangeable_minutes > 0) {
            // Full exchange: exchange all available minutes
            g_exchanged_minutes += g_exchangeable_minutes;
            g_exchangeable_minutes = 0;
            
            // Update UI
            update_ui();
            
            if (g_status_label) {
                lv_label_set_text(g_status_label, "Success");
                lv_obj_set_style_text_color(g_status_label, lv_color_hex(0x10B981), LV_PART_MAIN);  // Emerald green
            }
            
            ESP_LOGI(TAG, "Exchanged %lu minutes", g_exchanged_minutes);
        } else {
            if (g_status_label) {
                lv_label_set_text(g_status_label, "No Power");
                lv_obj_set_style_text_color(g_status_label, lv_color_hex(0xEF4444), LV_PART_MAIN);  // Soft red
            }
        }
    }
}

/**
 * @brief Create UI with step exchange interface
 */
static void create_ui(void)
{
    // Create a container for the content
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 320, 240);
    lv_obj_center(cont);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1A0B2E), LV_PART_MAIN);  // Deep purple background
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t *title_label = lv_label_create(cont);
    lv_label_set_text(title_label, "Step Exchange");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xE9D5FF), LV_PART_MAIN);  // Light purple text
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 8);

    // Exchange container (purple border box)
    lv_obj_t *exchange_container = lv_obj_create(cont);
    lv_obj_set_size(exchange_container, 280, 100);
    lv_obj_align(exchange_container, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_color(exchange_container, lv_color_hex(0x2D1B4E), LV_PART_MAIN);  // Dark purple container
    lv_obj_set_style_bg_opa(exchange_container, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_border_color(exchange_container, lv_color_hex(0xA855F7), LV_PART_MAIN);  // Purple border
    lv_obj_set_style_border_width(exchange_container, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(exchange_container, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(exchange_container, 20, LV_PART_MAIN);
    lv_obj_clear_flag(exchange_container, LV_OBJ_FLAG_SCROLLABLE);

    // Exchange label ("Exchangeable")
    lv_obj_t *exchange_label = lv_label_create(exchange_container);
    lv_label_set_text(exchange_label, "Exchangeable");
    lv_obj_set_style_text_color(exchange_label, lv_color_hex(0xC4B5FD), LV_PART_MAIN);  // Light purple-gray
    lv_obj_set_style_text_font(exchange_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(exchange_label, LV_ALIGN_TOP_MID, 0, 0);

    // Exchange value (large purple number)
    g_exchange_value_label = lv_label_create(exchange_container);
    char value_text[16];
    snprintf(value_text, sizeof(value_text), "%lu", g_exchangeable_minutes);
    lv_label_set_text(g_exchange_value_label, value_text);
    lv_obj_set_style_text_color(g_exchange_value_label, lv_color_hex(0xA855F7), LV_PART_MAIN);  // Purple
    lv_obj_set_style_text_font(g_exchange_value_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(g_exchange_value_label, 3, LV_PART_MAIN);
    lv_obj_align(g_exchange_value_label, LV_ALIGN_CENTER, -20, 5);

    // Exchange unit ("min")
    lv_obj_t *exchange_unit_label = lv_label_create(exchange_container);
    lv_label_set_text(exchange_unit_label, "min");
    lv_obj_set_style_text_color(exchange_unit_label, lv_color_hex(0xA855F7), LV_PART_MAIN);  // Purple
    lv_obj_set_style_text_font(exchange_unit_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align_to(exchange_unit_label, g_exchange_value_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // Exchange button
    lv_obj_t *exchange_btn = lv_btn_create(cont);
    lv_obj_set_size(exchange_btn, 200, 40);
    lv_obj_align(exchange_btn, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_bg_color(exchange_btn, lv_color_hex(0x8B5CF6), LV_PART_MAIN);  // Purple button
    lv_obj_set_style_bg_opa(exchange_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(exchange_btn, 12, LV_PART_MAIN);  // Rounded button
    lv_obj_add_event_cb(exchange_btn, exchange_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_label = lv_label_create(exchange_btn);
    lv_label_set_text(btn_label, "Exchange");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(btn_label);

    // Info row (total steps and exchanged minutes)
    // Total steps (left)
    g_total_steps_label = lv_label_create(cont);
    char steps_text[32];
    snprintf(steps_text, sizeof(steps_text), "Total: %lu steps", g_total_steps);
    lv_label_set_text(g_total_steps_label, steps_text);
    lv_obj_set_style_text_color(g_total_steps_label, lv_color_hex(0xA78BFA), LV_PART_MAIN);  // Medium purple
    lv_obj_set_style_text_font(g_total_steps_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(g_total_steps_label, LV_ALIGN_BOTTOM_LEFT, 10, -30);

    // Exchanged minutes (right)
    g_exchanged_minutes_label = lv_label_create(cont);
    char exchanged_text[32];
    snprintf(exchanged_text, sizeof(exchanged_text), "Exchanged: %lu min", g_exchanged_minutes);
    lv_label_set_text(g_exchanged_minutes_label, exchanged_text);
    lv_obj_set_style_text_color(g_exchanged_minutes_label, lv_color_hex(0xA78BFA), LV_PART_MAIN);  // Medium purple
    lv_obj_set_style_text_font(g_exchanged_minutes_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(g_exchanged_minutes_label, LV_ALIGN_BOTTOM_RIGHT, -10, -30);

    // Status label
    g_status_label = lv_label_create(cont);
    lv_label_set_text(g_status_label, "Ready");
    lv_obj_set_style_text_color(g_status_label, lv_color_hex(0xC4B5FD), LV_PART_MAIN);  // Light purple
    lv_obj_set_style_text_font(g_status_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(g_status_label, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void app_main(void)
{
    ESP_LOGI(TAG, "StepAI Example Started");

    // Use mock data (NVS disabled for now)
    g_exchanged_minutes = 0;
    g_total_steps = 5000;

    /* Initialize I2C (for touch and audio) */
    ESP_LOGI(TAG, "Initializing I2C...");
    bsp_i2c_init();
    ESP_LOGI(TAG, "I2C initialized");

    /* Initialize display and LVGL */
    ESP_LOGI(TAG, "Initializing display...");
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = 0,
        .flags = {
            .buff_dma = true,
        }
    };
    lv_disp_t *disp = bsp_display_start_with_config(&cfg);
    if (disp == NULL) {
        ESP_LOGE(TAG, "Display initialization failed!");
        return;
    }
    ESP_LOGI(TAG, "Display initialized successfully");

    /* Wait a bit for display to stabilize before turning on backlight */
    vTaskDelay(pdMS_TO_TICKS(200));

    /* Set display brightness to 100% */
    ESP_LOGI(TAG, "Setting display brightness to 100%%...");
    esp_err_t ret = bsp_display_backlight_on();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to turn on backlight: %s", esp_err_to_name(ret));
        /* Try setting brightness directly as fallback */
        ESP_LOGI(TAG, "Trying bsp_display_brightness_set(100)...");
        ret = bsp_display_brightness_set(100);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set brightness: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "Brightness set successfully");
        }
    } else {
        ESP_LOGI(TAG, "Backlight turned on successfully");
    }

    /* Wait a bit more for backlight to stabilize */
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Create UI */
    ESP_LOGI(TAG, "Creating UI...");
    create_ui();
    
    // Calculate and update UI with initial values
    calculate_exchangeable();
    update_ui();

    /* LVGL will automatically refresh the display via its timer task */

    /* Wait a bit more for display to update */
    vTaskDelay(pdMS_TO_TICKS(200));

    ESP_LOGI(TAG, "StepAI Example Initialized");
}

/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "bsp/esp-bsp.h"
#include "bsp_board.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lvgl.h"

static const char *TAG = "stepAI";

static lv_obj_t *g_switch_btn = NULL;
static gpio_num_t g_control_pin = GPIO_NUM_NC;
static bool g_pin_state = false;

/**
 * @brief Initialize GPIO pin for control
 */
static void gpio_init(void)
{
    const board_res_desc_t *brd = bsp_board_get_description();
    
    if (brd == NULL || brd->PMOD2 == NULL) {
        ESP_LOGE(TAG, "Failed to get board description or PMOD2 not available");
        return;
    }

    // Use PMOD2 row1[0] as control pin (first pin of PMOD2)
    g_control_pin = brd->PMOD2->row1[0];
    
    ESP_LOGI(TAG, "Initializing GPIO pin: %d", g_control_pin);

    // Configure GPIO as output
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << g_control_pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    // Set initial state to LOW
    gpio_set_level(g_control_pin, 0);
    g_pin_state = false;
    
    ESP_LOGI(TAG, "GPIO pin %d initialized, initial state: LOW", g_control_pin);
}

/**
 * @brief Update GPIO pin state
 */
static void update_gpio_state(bool state)
{
    if (g_control_pin == GPIO_NUM_NC) {
        ESP_LOGE(TAG, "GPIO pin not initialized");
        return;
    }

    g_pin_state = state;
    gpio_set_level(g_control_pin, state ? 1 : 0);
    ESP_LOGI(TAG, "GPIO pin %d set to %s", g_control_pin, state ? "HIGH" : "LOW");
}

/**
 * @brief Switch button event callback
 */
static void switch_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
    
    ESP_LOGI(TAG, "Switch button clicked, state: %s", checked ? "ON" : "OFF");
    
    // Update GPIO pin state
    update_gpio_state(checked);
    
    // Update state label text
    lv_obj_t *state_label = (lv_obj_t *)lv_obj_get_user_data(obj);
    if (state_label) {
        lv_label_set_text(state_label, checked ? "ON" : "OFF");
        // Change color based on state
        lv_obj_set_style_text_color(state_label, 
                                    checked ? lv_color_hex(0x00FF00) : lv_color_hex(0xFFFFFF), 
                                    LV_PART_MAIN);
    }
}

/**
 * @brief Create UI with switch button
 */
static void create_ui(void)
{
    // Create a container for the switch
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 200, 100);
    lv_obj_center(cont);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 20, LV_PART_MAIN);

    // Create label
    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, "StepAI Test");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    // Create switch button
    g_switch_btn = lv_switch_create(cont);
    lv_obj_set_size(g_switch_btn, 60, 30);
    lv_obj_align(g_switch_btn, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_event_cb(g_switch_btn, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Create state label
    lv_obj_t *state_label = lv_label_create(cont);
    lv_label_set_text(state_label, "OFF");
    lv_obj_set_style_text_color(state_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(state_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(state_label, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    // Store state label reference in switch button user data
    lv_obj_set_user_data(g_switch_btn, state_label);

    // Create pin info label
    if (g_control_pin != GPIO_NUM_NC) {
        char pin_info[32];
        snprintf(pin_info, sizeof(pin_info), "Pin: GPIO%d", g_control_pin);
        lv_obj_t *pin_label = lv_label_create(cont);
        lv_label_set_text(pin_label, pin_info);
        lv_obj_set_style_text_color(pin_label, lv_color_hex(0x888888), LV_PART_MAIN);
        lv_obj_set_style_text_font(pin_label, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_align(pin_label, LV_ALIGN_BOTTOM_MID, 0, -25);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Button Demo Example Started");

    /* Initialize I2C (for touch and audio) */
    bsp_i2c_init();

    /* Initialize display and LVGL */
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = 0,
        .flags = {
            .buff_dma = true,
        }
    };
    bsp_display_start_with_config(&cfg);

    /* Set display brightness to 100% */
    bsp_display_backlight_on();

    /* Initialize GPIO */
    gpio_init();

    /* Create UI */
    create_ui();

    ESP_LOGI(TAG, "Button Demo Example Initialized");
    ESP_LOGI(TAG, "Touch the switch on screen to control GPIO pin");
}

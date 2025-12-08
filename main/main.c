#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "adc.h"
#include "mppt_pno.h"

static const char *TAG = "MPPT";

// Defaults (can be moved to Kconfig later)
#define PWM_GPIO        3       // GPIO3 ESP32-C6 (LEDC)
#define PWM_FREQ_HZ     50000   // 50 kHz
#define LEDC_RES_BITS   LEDC_TIMER_13_BIT
#define LEDC_MAX_DUTY   8191

// Generic ADC config defaults
#define ADC_V_GPIO      1
#define ADC_I_GPIO      2
#define V_DIV_RATIO     11.0f   // e.g., R1=100k, R2=10k
#define I_SCALE         1.0f    // placeholder A/V
#define I_OFFSET_V      0.0f

static void pwm_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_RES_BITS,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = PWM_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channel = {
        .gpio_num       = PWM_GPIO,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

static void set_pwm_duty_ticks(uint32_t ticks) {
    if (ticks > LEDC_MAX_DUTY) ticks = LEDC_MAX_DUTY;
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, ticks));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
}

static uint32_t duty_to_ticks(float duty_frac) {
    if (duty_frac < 0) duty_frac = 0; if (duty_frac > 1) duty_frac = 1;
    return (uint32_t)(duty_frac * (float)LEDC_MAX_DUTY);
}

static void mppt_task(void *arg) {
    ESP_LOGI(TAG, "MPPT task started");

    adc_cfg_t acfg = {
        .gpio_v = ADC_V_GPIO,
        .gpio_i = ADC_I_GPIO,
        .v_div_ratio = V_DIV_RATIO,
        .i_scale = I_SCALE,
        .i_offset_v = I_OFFSET_V,
    };
    adc_init(&acfg);

    mppt_pno_cfg_t mcfg = {
        .duty_min = 0.05f,
        .duty_max = 0.95f,
        .duty_step = 0.01f,
        .filter_n = 8,
    };
    mppt_pno_t pno;
    mppt_pno_init(&pno, &mcfg, 0.3f);

    while (1) {
        float v,i; adc_read_vi(&v,&i);
        float duty = mppt_pno_update(&pno, v, i);
        set_pwm_duty_ticks(duty_to_ticks(duty));
        ESP_LOGI(TAG, "V=%.2fV I=%.2fA P=%.2fW duty=%.0f%%", v, i, v*i, duty*100.0f);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Boot MPPT on ESP32-C6");
    pwm_init();
    xTaskCreate(mppt_task, "mppt_task", 4096, NULL, 5, NULL);
}

#include "adc.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

static const char *TAG = "ADC";
static adc_cfg_t s_cfg;
static esp_adc_cal_characteristics_t s_chars;

esp_err_t adc_init(const adc_cfg_t *cfg) {
    s_cfg = *cfg;
    // Configure ADC1 channels from gpio mapping (ESP32-C6: ADC1 channels on specific GPIOs)
    // NOTE: For simplicity, we assume gpio_v -> ADC1_CH1, gpio_i -> ADC1_CH2. Adjust as needed.
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_1, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_2, ADC_ATTEN_DB_11);

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &s_chars);
    ESP_LOGI(TAG, "ADC characterized, type=%d", val_type);
    return ESP_OK;
}

static float read_channel_mv(adc_channel_t ch) {
    uint32_t accum = 0;
    const int samples = 16;
    for (int i = 0; i < samples; ++i) {
        accum += adc1_get_raw(ch);
    }
    uint32_t raw = accum / samples;
    uint32_t mv = 0;
    esp_adc_cal_get_voltage(raw, &s_chars, &mv);
    return (float)mv / 1000.0f;
}

esp_err_t adc_read_vi(float *v_volts, float *i_amps) {
    float v_adc = read_channel_mv(ADC_CHANNEL_1);
    float i_adc = read_channel_mv(ADC_CHANNEL_2);
    *v_volts = v_adc * s_cfg.v_div_ratio;
    *i_amps  = (i_adc - s_cfg.i_offset_v) * s_cfg.i_scale;
    return ESP_OK;
}

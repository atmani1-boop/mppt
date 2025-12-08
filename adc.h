#ifndef MPPT_ADC_H
#define MPPT_ADC_H
#include <stdint.h>
#include "esp_err.h"

typedef struct {
    int gpio_v;
    int gpio_i;
    float v_div_ratio;   // Vpanel = Vadc * v_div_ratio
    float i_scale;       // I = (Vadc - offset) * i_scale
    float i_offset_v;    // offset en volts à soustraire
} adc_cfg_t;

esp_err_t adc_init(const adc_cfg_t *cfg);
esp_err_t adc_read_vi(float *v_volts, float *i_amps);

#endif // MPPT_ADC_H

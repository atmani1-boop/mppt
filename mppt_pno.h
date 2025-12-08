#ifndef MPPT_PNO_H
#define MPPT_PNO_H
#include <stdint.h>

typedef struct {
    float duty_min;
    float duty_max;
    float duty_step;
    int   filter_n;   // moving average window
} mppt_pno_cfg_t;

typedef struct {
    mppt_pno_cfg_t cfg;
    float duty;
    float p_prev;
    float v_avg;
    float i_avg;
    float p_avg;
} mppt_pno_t;

void mppt_pno_init(mppt_pno_t *s, const mppt_pno_cfg_t *cfg, float duty_init);
float mppt_pno_update(mppt_pno_t *s, float v, float i);

#endif // MPPT_PNO_H

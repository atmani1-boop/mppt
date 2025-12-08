#include "mppt_pno.h"

static float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }

void mppt_pno_init(mppt_pno_t *s, const mppt_pno_cfg_t *cfg, float duty_init) {
    s->cfg = *cfg;
    s->duty = clampf(duty_init, cfg->duty_min, cfg->duty_max);
    s->p_prev = 0.0f;
    s->v_avg = 0.0f;
    s->i_avg = 0.0f;
    s->p_avg = 0.0f;
}

static float ma_update(float prev, float x, int n) {
    // simple IIR equivalent of moving average
    if (n <= 1) return x;
    float alpha = 1.0f / (float)n;
    return prev + alpha * (x - prev);
}

float mppt_pno_update(mppt_pno_t *s, float v, float i) {
    s->v_avg = ma_update(s->v_avg, v, s->cfg.filter_n);
    s->i_avg = ma_update(s->i_avg, i, s->cfg.filter_n);
    float p = s->v_avg * s->i_avg;
    s->p_avg = ma_update(s->p_avg, p, s->cfg.filter_n);

    float delta_p = s->p_avg - s->p_prev;
    s->p_prev = s->p_avg;

    if (delta_p > 0) {
        s->duty += s->cfg.duty_step;
    } else {
        s->duty -= s->cfg.duty_step;
    }
    s->duty = clampf(s->duty, s->cfg.duty_min, s->cfg.duty_max);
    return s->duty;
}

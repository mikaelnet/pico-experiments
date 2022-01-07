
#ifndef KALMAN_H_
#define KALMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __kalman_filter_t {
    float Q;    // 0.022
    float R;    // 0.617

    float last_estimation;
    float last_prediction;
} kalman_filter_t;

extern void kalman_init (kalman_filter_t* filter, float firstMeasure);
extern float kalman_filter (kalman_filter_t* filter, float measure);

#ifdef __cplusplus
}
#endif

#endif /* KALMAN_H_ */


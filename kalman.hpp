
#ifndef _KALMAN_HPP
#define _KALMAN_HPP

class KalmanFilter
{
    private:
        float _last_estimation;
        float _last_prediction;
        bool _initialized = false;

    public:
        float Q;
        float R;

        KalmanFilter (float q = 0.022f, float r = 0.617f);

        void init (float firstMeasure);
        float filter (float measure);
};


typedef struct __kalman_filter_t {
    float Q;    // 0.022
    float R;    // 0.617

    float last_estimation;
    float last_prediction;
} kalman_filter_t;

#endif /* _KALMAN_HPP */


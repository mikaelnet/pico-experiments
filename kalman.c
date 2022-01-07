
#include "kalman.h"

#ifdef __cplusplus
extern "C" {
#endif

void kalman_init (kalman_filter_t* filter, float firstMeasure)
{
    filter->Q = 0.022;
    filter->R = 0.617;

    filter->last_estimation = firstMeasure;
    filter->last_prediction = 0;
}

float kalman_filter (kalman_filter_t* filter, float measure)
{
    // Do a prediction
    float prediction = filter->last_prediction + filter->Q;

    // Calculate the Kalman gain
    float kalmanGain = prediction * (1.0 / (prediction + filter->R));

    // Correct the measurement
    float estimation = filter->last_estimation + kalmanGain * (measure - filter->last_estimation);
    filter->last_prediction = (1 - kalmanGain) * prediction;
    filter->last_estimation = estimation;

    return estimation;
}

#ifdef __cplusplus
}
#endif

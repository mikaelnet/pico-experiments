
#include "kalman.hpp"

KalmanFilter::KalmanFilter (float q, float r)
{
    Q = q;
    R = r;
}

void KalmanFilter::init (float firstMeasure)
{
    _last_estimation = firstMeasure;
    _last_prediction = 0;
}

float KalmanFilter::filter (float measure)
{
    if (!_initialized) {
        init(measure);
        return measure;
    }

    // Do a prediction
    float prediction = _last_prediction + Q;

    // Calculate the Kalman gain
    float kalmanGain = prediction * (1.0 / (prediction + R));

    // Correct the measurement
    float estimation = _last_estimation + kalmanGain * (measure - _last_estimation);
    _last_prediction = (1 - kalmanGain) * prediction;
    _last_estimation = estimation;

    return estimation;
}

#ifndef MU_AUDIO_AUDIOMATHUTILS_H
#define MU_AUDIO_AUDIOMATHUTILS_H

#include <cmath>

#include "audiotypes.h"

namespace mu::audio {
inline float balanceGain(const balance_t balance, const int audioChannelNumber)
{
    return 0.5f * balance * ((audioChannelNumber * 2.f) - 1) + 0.5f;
}

inline float gainFromDecibels(const volume_dbfs_t volumeLevelDb)
{
    return std::pow(10.0f, volumeLevelDb * 0.05f);
}

inline volume_dbfs_t dbFullScaleFromSample(const float signalValue)
{
    return 20 * std::log10(std::abs(signalValue));
}

inline float samplesRootMeanSquare(float&& squaredSum, const samples_t sampleCount)
{
    return std::sqrt(squaredSum / sampleCount);
}
}

#endif // MU_AUDIO_AUDIOMATHUTILS_H

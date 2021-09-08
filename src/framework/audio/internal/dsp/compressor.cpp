#include "compressor.h"

#include "internal/audiomathutils.h"

using namespace mu::audio;
using namespace mu::audio::dsp;

static constexpr float HALF_KNEE = 3.f;
static constexpr float RATIO = 1.f / 60.f;
static constexpr volume_db_t THRESHOLD = -10.f;
static constexpr float UPPER_KNEE_BOUNDARY = THRESHOLD + HALF_KNEE;
static constexpr float LOWER_KNEE_BOUNDARY = THRESHOLD - HALF_KNEE;
static constexpr float QUADRATIC_LOWER_KNEE_BOUNDARY = LOWER_KNEE_BOUNDARY * LOWER_KNEE_BOUNDARY;
static constexpr float ATTACK_PHASE = 10.f;
static constexpr float RELEASE_PHASE = 100.f;
static constexpr float DIRECTION = -1.f;

inline float calculateGainReduction(const volume_db_t& currentAmplitudeDb)
{
    if (currentAmplitudeDb < UPPER_KNEE_BOUNDARY) {
        return gainFromDecibels(currentAmplitudeDb + QUADRATIC_LOWER_KNEE_BOUNDARY * (RATIO - 1.f) / (4.f * HALF_KNEE));
    } else {
        return gainFromDecibels(THRESHOLD + (currentAmplitudeDb - THRESHOLD) * RATIO);
    }
}

void Compressor::process(float* buffer, const samples_t& samplesPerChannel, const float& audioChannelRms,
                         const audioch_t channelNumber, const audioch_t audioChannelsCount)
{
    volume_dbfs_t currentAmplitudeDb = dbFullScaleFromSample(audioChannelRms);

    if (currentAmplitudeDb < LOWER_KNEE_BOUNDARY) {
        return;
    }

    float gainReduction = calculateGainReduction(currentAmplitudeDb);

    for (samples_t i = 0; i < samplesPerChannel; ++i) {
        int idx = i * audioChannelsCount + channelNumber;

        buffer[idx] = buffer[idx] * gainReduction;
    }
}

#include "waveformmodel.h"

using namespace mu::audio;

static const volume_dbfs_t MAX_DISPLAYED_DBFS = 0.f; // 100%
static const volume_dbfs_t MIN_DISPLAYED_DBFS = -60.f; // 0%

WaveFormModel::WaveFormModel(QObject* parent)
    : QObject(parent)
{
    playback()->audioIO()->masterSignalAmplitudeChanged().onReceive(this, [this](const audioch_t, const float amplitude) {
        setCurrentSignalAmplitude(amplitude);
    });

    playback()->audioIO()->masterVolumePressureChanged().onReceive(this, [this](const audioch_t, const volume_dbfs_t pressure) {
        if (pressure < MIN_DISPLAYED_DBFS) {
            setCurrentVolumePressure(MIN_DISPLAYED_DBFS);
        } else if (pressure > MAX_DISPLAYED_DBFS) {
            setCurrentVolumePressure(MIN_DISPLAYED_DBFS);
        } else {
            setCurrentVolumePressure(pressure);
        }
    });
}

QStringList WaveFormModel::availableSources() const
{
    return m_availableSources;
}

QString WaveFormModel::currentSourceName() const
{
    return m_currentSourceName;
}

float WaveFormModel::currentSignalAmplitude() const
{
    return m_currentSignalAmplitude;
}

volume_dbfs_t WaveFormModel::currentVolumePressure() const
{
    return m_currentVolumePressure;
}

void WaveFormModel::setAvailableSources(QStringList availableSources)
{
    if (m_availableSources == availableSources) {
        return;
    }

    m_availableSources = availableSources;
    emit availableSourcesChanged(m_availableSources);
}

void WaveFormModel::setCurrentSourceName(QString currentSourceName)
{
    if (m_currentSourceName == currentSourceName) {
        return;
    }

    m_currentSourceName = currentSourceName;
    emit currentSourceNameChanged(m_currentSourceName);
}

void WaveFormModel::setCurrentSignalAmplitude(float currentSignalAmplitude)
{
    m_currentSignalAmplitude = currentSignalAmplitude;
    emit currentSignalAmplitudeChanged(m_currentSignalAmplitude);
}

void WaveFormModel::setCurrentVolumePressure(float currentVolumePressure)
{
    m_currentVolumePressure = currentVolumePressure;
    emit currentVolumePressureChanged(m_currentVolumePressure);
}

float WaveFormModel::minDisplayedDbfs() const
{
    return MIN_DISPLAYED_DBFS;
}

float WaveFormModel::maxDisplayedDbfs() const
{
    return MAX_DISPLAYED_DBFS;
}

#ifndef MU_PLAYBACK_MIXERCHANNELITEM_H
#define MU_PLAYBACK_MIXERCHANNELITEM_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/iaudiooutput.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"

namespace mu::playback {
class MixerChannelItem : public QObject
{
    Q_OBJECT

    INJECT(playback, audio::IPlayback, playback)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

    Q_PROPERTY(float leftChannelPressure READ leftChannelPressure NOTIFY leftChannelPressureChanged)
    Q_PROPERTY(float rightChannelPressure READ rightChannelPressure NOTIFY rightChannelPressureChanged)

    Q_PROPERTY(float volumeLevel READ volumeLevel WRITE setVolumeLevel NOTIFY volumeLevelChanged)
    Q_PROPERTY(float balance READ balance WRITE setBalance NOTIFY balanceChanged)

    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool solo READ solo WRITE setSolo NOTIFY soloChanged)

public:
    explicit MixerChannelItem(QObject* parent);

    QString title() const;

    float leftChannelPressure() const;
    float rightChannelPressure() const;

    float volumeLevel() const;
    float balance() const;

    bool muted() const;
    bool solo() const;

    void setInputParams(audio::AudioInputParams&& inputParams);
    void setOutputParams(audio::AudioOutputParams&& outParams);

public slots:
    void setTitle(QString title);

    void setLeftChannelPressure(float leftChannelPressure);
    void setRightChannelPressure(float rightChannelPressure);

    void setVolumeLevel(float volumeLevel);
    void setBalance(float balance);

    void setMuted(bool muted);
    void setSolo(bool solo);

signals:
    void titleChanged(QString title);

    void leftChannelPressureChanged(float leftChannelPressure);
    void rightChannelPressureChanged(float rightChannelPressure);

    void volumeLevelChanged(float volumeLevel);
    void balanceChanged(float balance);

    void mutedChanged(bool muted);
    void soloChanged(bool solo);

private:
    audio::AudioInputParams m_inputParams;
    audio::AudioOutputParams m_outParams;

    QString m_title;

    float m_leftChannelPressure = 0.0;
    float m_rightChannelPressure = 0.0;

    float m_volumeLevel = 0.0;
    float m_balance = 0.0;

    bool m_muted = false;
    bool m_solo = false;
};
}

#endif // MU_PLAYBACK_MIXERCHANNELITEM_H

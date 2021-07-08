#ifndef MIXERPANELMODEL_H
#define MIXERPANELMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/itracks.h"
#include "audio/iplayback.h"

#include "internal/mixerchannelitem.h"

namespace mu::playback {
class MixerPanelModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(playback, audio::IPlayback, playback)

public:
    explicit MixerPanelModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    void clear();

    MixerChannelItem* buildChannelItem(const audio::TrackSequenceId& sequenceId, const audio::TrackId& trackId);
    MixerChannelItem* buildMasterChannelItem();

    QList<MixerChannelItem*> m_mixerChannelList;
};
}

#endif // MIXERPANELMODEL_H

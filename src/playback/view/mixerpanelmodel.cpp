#include "mixerpanelmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::playback;
using namespace mu::audio;

MixerPanelModel::MixerPanelModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void MixerPanelModel::load()
{
    if (!m_mixerChannelList.empty()) {
        clear();
    }

    playback()->sequenceIdList().onResolve(this, [this](const TrackSequenceIdList& idList) {
        for (const TrackSequenceId& sequenceId : idList) {
            playback()->tracks()->trackIdList(sequenceId).onResolve(this, [this, sequenceId](const TrackIdList& trackIdList) {
                for (TrackId trackId : trackIdList) {
                    m_mixerChannelList.append(buildChannelItem(sequenceId, trackId));
                }
            });
        }

        m_mixerChannelList.append(buildMasterChannelItem());
    });
}

QVariant MixerPanelModel::data(const QModelIndex& index, int) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    return QVariant::fromValue(m_mixerChannelList.at(index.row()));
}

int MixerPanelModel::rowCount(const QModelIndex&) const
{
    return m_mixerChannelList.count();
}

void MixerPanelModel::clear()
{
    qDeleteAll(m_mixerChannelList);
    m_mixerChannelList.clear();
}

MixerChannelItem* MixerPanelModel::buildChannelItem(const audio::TrackSequenceId& sequenceId, const audio::TrackId& trackId)
{
    MixerChannelItem* item = new MixerChannelItem(this);

    playback()->tracks()->inputParams(sequenceId, trackId)
    .onResolve(this, [item](AudioInputParams inParams) {
        item->setInputParams(std::move(inParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    playback()->audioOutput()->outputParams(sequenceId, trackId)
    .onResolve(this, [item](AudioOutputParams outParams) {
        item->setOutputParams(std::move(outParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get track output parameters, error code: " << errCode
               << ", " << text;
    });

    return item;
}

MixerChannelItem* MixerPanelModel::buildMasterChannelItem()
{
    MixerChannelItem* item = new MixerChannelItem(this);

    item->setTitle(qtrc("playback", "Master"));

    playback()->audioOutput()->masterOutputParams().onResolve(this, [item](AudioOutputParams outParams) {
        item->setOutputParams(std::move(outParams));
    })
    .onReject(this, [](int errCode, std::string text) {
        LOGE() << "unable to get master output parameters, error code: " << errCode
               << ", " << text;
    });

    return item;
}

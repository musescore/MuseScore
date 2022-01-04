/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "vstpluginlistmodelexample.h"
#include "log.h"

using namespace mu::vst;
using namespace mu::audio;

VstPluginListModelExample::VstPluginListModelExample(QObject* parent)
    : QObject(parent)
{
}

void VstPluginListModelExample::load()
{
    playback()->sequenceIdList()
    .onResolve(this, [this](TrackSequenceIdList idList) {
        QVariantList list;

        for (const TrackSequenceId id : idList) {
            QVariantMap item = { { "value", id } };
            list << item;
        }
        setSequenceIdList(list);
    })
    .onReject(this, [](int errCode, const std::string& text) {
        LOGE() << "unable to resolve track sequences, "
               << "error code: " << errCode
               << "error text: " << text;
    });

    playback()->tracks()->availableInputResources()
    .onResolve(this, [this](AudioResourceMetaList resourceList) {
        QVariantList list;

        for (const auto& meta : resourceList) {
            if (meta.type != AudioResourceType::VstPlugin) {
                continue;
            }

            QVariantMap item = { { "value", QString::fromStdString(meta.id) } };
            list << item;
        }
        setAvailableSynthResources(list);
    })
    .onReject(this, [](int errCode, const std::string& text) {
        LOGE() << "unable to resolve vsti audio resources, "
               << "error code: " << errCode
               << "error text: " << text;
    });

    playback()->audioOutput()->availableOutputResources()
    .onResolve(this, [this](AudioResourceMetaList resources) {
        QVariantList list;

        for (const auto& meta : resources) {
            QVariantMap item = { { "value", QString::fromStdString(meta.id) } };
            list << item;
        }
        setAvailableFxResources(list);
    })
    .onReject(this, [](int errCode, const std::string& text) {
        LOGE() << "unable to resolve vstfx audio resources, "
               << "error code: " << errCode
               << "error text: " << text;
    });
}

void VstPluginListModelExample::showSynthPluginEditor()
{
    QString uri = QString("musescore://vsti/editor?sync=false&floating=true&trackId=%1&resourceId=%2")
                  .arg(m_currentTrackId)
                  .arg(m_currentSynthResource);

    interactive()->open(uri.toStdString());
}

void VstPluginListModelExample::showFxPluginEditor()
{
    QString uri = QString("musescore://vstfx/editor?sync=false&floating=true&trackId=%1&resourceId=%2")
                  .arg(m_currentTrackId)
                  .arg(m_currentFxResource);

    interactive()->open(uri.toStdString());
}

const QVariantList& VstPluginListModelExample::sequenceIdList() const
{
    return m_sequenceIdList;
}

void VstPluginListModelExample::setSequenceIdList(const QVariantList& newSequenceIdList)
{
    if (m_sequenceIdList == newSequenceIdList) {
        return;
    }
    m_sequenceIdList = newSequenceIdList;
    emit sequenceIdListChanged();
}

const QVariantList& VstPluginListModelExample::trackIdList() const
{
    return m_trackIdList;
}

void VstPluginListModelExample::setTrackIdList(const QVariantList& newTrackIdList)
{
    if (m_trackIdList == newTrackIdList) {
        return;
    }
    m_trackIdList = newTrackIdList;
    emit trackIdListChanged();
}

const QVariantList& VstPluginListModelExample::availableFxResources() const
{
    return m_availableFxResources;
}

void VstPluginListModelExample::setAvailableFxResources(const QVariantList& newAvailableFxResources)
{
    if (m_availableFxResources == newAvailableFxResources) {
        return;
    }
    m_availableFxResources = newAvailableFxResources;
    emit availableFxResourcesChanged();
}

const QVariantList& VstPluginListModelExample::availableSynthResources() const
{
    return m_availableSynthResources;
}

void VstPluginListModelExample::setAvailableSynthResources(const QVariantList& newAvailableSynthResources)
{
    if (m_availableSynthResources == newAvailableSynthResources) {
        return;
    }
    m_availableSynthResources = newAvailableSynthResources;
    emit availableSynthResourcesChanged();
}

int VstPluginListModelExample::currentSequenceId() const
{
    return m_currentSequenceId;
}

void VstPluginListModelExample::setCurrentSequenceId(int newCurrentSequenceId)
{
    if (m_currentSequenceId == newCurrentSequenceId) {
        return;
    }
    m_currentSequenceId = newCurrentSequenceId;

    playback()->tracks()->trackIdList(m_currentSequenceId)
    .onResolve(this, [this](const TrackIdList idList) {
        QVariantList list;

        for (const TrackId id : idList) {
            QVariantMap item = { { "value", id } };
            list << item;
        }
        setTrackIdList(list);
    })
    .onReject(this, [](int errCode, const std::string& text) {
        LOGE() << "unable to resolve tracks, "
               << "error code: " << errCode
               << "error text: " << text;
    });

    emit currentSequenceIdChanged();
}

int VstPluginListModelExample::currentTrackId() const
{
    return m_currentTrackId;
}

void VstPluginListModelExample::setCurrentTrackId(int newCurrentTrackId)
{
    if (m_currentTrackId == newCurrentTrackId) {
        return;
    }
    m_currentTrackId = newCurrentTrackId;
    emit currentTrackIdChanged();
}

const QString& VstPluginListModelExample::currentSynthResource() const
{
    return m_currentSynthResource;
}

void VstPluginListModelExample::applyNewInputParams()
{
    AudioInputParams inputParams;
    inputParams.resourceMeta.id = m_currentSynthResource.toStdString();

    playback()->tracks()->setInputParams(m_currentSequenceId, m_currentTrackId, inputParams);
}

void VstPluginListModelExample::applyNewOutputParams()
{
    AudioFxParams fxParams;
    fxParams.resourceMeta.id = m_currentFxResource.toStdString();

    AudioOutputParams outputParams;
    outputParams.fxChain.emplace(0, fxParams);

    playback()->audioOutput()->setOutputParams(m_currentSequenceId, m_currentTrackId, outputParams);
}

void VstPluginListModelExample::setCurrentSynthResource(const QString& newCurrentSynthResource)
{
    if (m_currentSynthResource == newCurrentSynthResource) {
        return;
    }
    m_currentSynthResource = newCurrentSynthResource;

    applyNewInputParams();
    emit currentSynthResourceChanged();
}

const QString& VstPluginListModelExample::currentFxResource() const
{
    return m_currentFxResource;
}

void VstPluginListModelExample::setCurrentFxResource(const QString& newCurrentFxResource)
{
    if (m_currentFxResource == newCurrentFxResource) {
        return;
    }
    m_currentFxResource = newCurrentFxResource;
    applyNewOutputParams();
    emit currentFxResourceChanged();
}

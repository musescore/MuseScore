/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include <QJsonArray>

#include "notationsolomutestate.h"
#include "notation.h"

using namespace mu;
using namespace mu::notation;

Ret NotationSoloMuteState::read(const engraving::MscReader& reader, const io::path_t& pathPrefix)
{
    ByteArray json = reader.readAudioSettingsJsonFile(pathPrefix);

    if (json.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    QJsonObject rootObj = QJsonDocument::fromJson(json.toQByteArrayNoCopy()).object();

    QJsonArray tracksArray = rootObj.value("tracks").toArray();
    for (const QJsonValue track : tracksArray) {
        QJsonObject trackObject = track.toObject();

        ID partId = trackObject.value("partId").toString();
        std::string instrumentId = trackObject.value("instrumentId").toString().toStdString();
        InstrumentTrackId id = { partId, instrumentId };

        QJsonObject soloMuteObj = trackObject.value("soloMuteState").toObject();
        SoloMuteState soloMuteState;
        soloMuteState.mute = soloMuteObj.value("mute").toBool();
        soloMuteState.solo = soloMuteObj.value("solo").toBool();

        m_trackSoloMuteStatesMap.emplace(id, std::move(soloMuteState));
    }

    return make_ret(Ret::Code::Ok);
}

Ret NotationSoloMuteState::write(io::IODevice* out)
{
    QJsonObject rootObj;
    QJsonArray tracksArray;

    for (auto pair : m_trackSoloMuteStatesMap) {
        QJsonObject currentTrack;
        currentTrack["instrumentId"] = QString::fromStdString(pair.first.instrumentId);
        currentTrack["partId"] = pair.first.partId.toQString();

        QJsonObject soloMuteStateObject;
        soloMuteStateObject["mute"] = pair.second.mute;
        soloMuteStateObject["solo"] = pair.second.solo;

        currentTrack["soloMuteState"] = soloMuteStateObject;
        tracksArray.append(currentTrack);
    }

    rootObj["tracks"] = tracksArray;

    out->write(QJsonDocument(rootObj).toJson());

    return make_ret(Ret::Code::Ok);
}

bool NotationSoloMuteState::trackSoloMuteStateExists(const engraving::InstrumentTrackId& partId) const
{
    auto search = m_trackSoloMuteStatesMap.find(partId);
    return search != m_trackSoloMuteStatesMap.end();
}

mu::notation::INotationSoloMuteState::SoloMuteState NotationSoloMuteState::trackSoloMuteState(const InstrumentTrackId& partId) const
{
    auto search = m_trackSoloMuteStatesMap.find(partId);

    if (search == m_trackSoloMuteStatesMap.end()) {
        return {};
    }

    return search->second;
}

void NotationSoloMuteState::setTrackSoloMuteState(const InstrumentTrackId& partId, const SoloMuteState& state)
{
    auto it = m_trackSoloMuteStatesMap.find(partId);
    if (it != m_trackSoloMuteStatesMap.end() && it->second == state) {
        return;
    }

    m_trackSoloMuteStatesMap.insert_or_assign(partId, state);
    m_trackSoloMuteStateChanged.send(partId, state);
}

void NotationSoloMuteState::removeTrackSoloMuteState(const engraving::InstrumentTrackId& trackId)
{
    auto soloMuteSearch = m_trackSoloMuteStatesMap.find(trackId);
    if (soloMuteSearch != m_trackSoloMuteStatesMap.end()) {
        m_trackSoloMuteStatesMap.erase(soloMuteSearch);
    }
}

mu::async::Channel<InstrumentTrackId, INotationSoloMuteState::SoloMuteState> NotationSoloMuteState::trackSoloMuteStateChanged() const
{
    return m_trackSoloMuteStateChanged;
}

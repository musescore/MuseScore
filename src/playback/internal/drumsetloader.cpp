/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "drumsetloader.h"

#include "notation/notationtypes.h"

#include "engraving/rw/xmlreader.h"

#include "global/types/bytearray.h"

using namespace mu::playback;
using namespace muse::audio;
using namespace mu::notation;
using namespace mu::engraving;

static void readDrumset(const muse::ByteArray& drumMapping, Drumset& drumset)
{
    XmlReader reader(drumMapping);

    while (reader.readNextStartElement()) {
        if (reader.name() == "museScore") {
            while (reader.readNextStartElement()) {
                if (reader.name() == "Drum") {
                    drumset.load(reader);
                } else {
                    reader.unknown();
                }
            }
        }
    }
}

void DrumsetLoader::loadDrumset(INotationPtr notation, const InstrumentTrackId& trackId, const AudioResourceMeta& resourceMeta)
{
    TRACEFUNC;

    if (!notation) {
        return;
    }

    // restore the default drumset when changing from MuseSounds to MS Basic / VST
    if (resourceMeta.type != AudioResourceType::MuseSamplerSoundPack) {
        const InstrumentTemplate& templ = instrumentsRepository()->instrumentTemplate(trackId.instrumentId);
        if (!templ.useDrumset) {
            return;
        }

        if (templ.drumset) {
            replaceDrumset(notation, trackId, *templ.drumset);
        } else if (mu::engraving::smDrumset) {
            replaceDrumset(notation, trackId, *mu::engraving::smDrumset);
        }

        return;
    }

    int instrumentId = resourceMeta.attributeVal(u"museUID").toInt();

    auto it = m_drumsetCache.find(instrumentId);
    if (it != m_drumsetCache.end()) {
        if (it->second == std::nullopt) {
            return; // drumset mapping not available
        }

        replaceDrumset(notation, trackId, it->second.value());
        return;
    }

    muse::ByteArray drumMapping = museSampler()->drumMapping(instrumentId);
    if (drumMapping.empty()) {
        m_drumsetCache.emplace(instrumentId, std::nullopt);
        return;
    }

    Drumset drumset;
    readDrumset(drumMapping, drumset);
    replaceDrumset(notation, trackId, drumset);

    m_drumsetCache.emplace(instrumentId, std::move(drumset));
}

void DrumsetLoader::replaceDrumset(INotationPtr notation, const InstrumentTrackId& trackId, const Drumset& drumset)
{
    const Part* part = notation->parts()->part(trackId.partId);
    if (!part) {
        return;
    }

    QString instrumentId = trackId.instrumentId.toQString();
    const mu::engraving::InstrumentList& instruments = part->instruments();

    for (auto it = instruments.cbegin(); it != instruments.cend(); ++it) {
        if (it->second->id() != trackId.instrumentId) {
            continue;
        }

        InstrumentKey instrumentKey;
        instrumentKey.instrumentId = instrumentId;
        instrumentKey.partId = trackId.partId;
        instrumentKey.tick = Fraction::fromTicks(it->first);

        notation->parts()->replaceDrumset(instrumentKey, drumset, false /*undoable*/);
    }
}

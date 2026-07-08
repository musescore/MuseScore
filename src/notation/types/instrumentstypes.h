/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "types/id.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/part.h"
#include "engraving/dom/scoreorder.h"
#include "engraving/types/types.h"

namespace mu::notation {
static constexpr int MAX_STAVES  = 4;

using Instrument = mu::engraving::Instrument;
using InstrumentTemplate = mu::engraving::InstrumentTemplate;
using InstrumentTrait = mu::engraving::Trait;
using TraitType = mu::engraving::TraitType;
using ScoreOrder = mu::engraving::ScoreOrder;
using InstrumentGenre = mu::engraving::InstrumentGenre;
using InstrumentGroup = mu::engraving::InstrumentGroup;

using Drumset = mu::engraving::Drumset;
using StringData = mu::engraving::StringData;

using SharpFlat = mu::engraving::PreferSharpFlat;

using InstrumentTemplateList = std::vector<const InstrumentTemplate*>;
using InstrumentGenreList = std::vector<const InstrumentGenre*>;
using ScoreOrderList = std::vector<mu::engraving::ScoreOrder>;
using InstrumentGroupList = std::vector<const InstrumentGroup*>;
using InstrumentTrackId = mu::engraving::InstrumentTrackId;
using InstrumentTrackIdSet = mu::engraving::InstrumentTrackIdSet;

static const muse::String COMMON_GENRE_ID(u"common");

struct InstrumentKey
{
    muse::String instrumentId;
    muse::ID partId;
    engraving::Fraction tick = engraving::Fraction(0, 1);
};

inline bool isMainInstrumentForPart(const InstrumentKey& instrumentKey, const engraving::Part* part)
{
    return instrumentKey.instrumentId == part->instrumentId() && instrumentKey.tick == engraving::Part::MAIN_INSTRUMENT_TICK;
}

inline QString formatInstrumentTitle(const QString& instrumentName, const InstrumentTrait& trait)
{
    // Comments for translators start with //:
    switch (trait.type) {
    case TraitType::Tuning:
        //: %1=tuning ("D"), %2=name ("Tin Whistle"). Example: "D Tin Whistle"
        return muse::qtrc("notation", "%1 %2", "Tuned instrument displayed in the UI")
               .arg(trait.name, instrumentName);
    case TraitType::Transposition:
        //: %1=name ("Horn"), %2=transposition ("C alto"). Example: "Horn in C alto"
        return muse::qtrc("notation", "%1 in %2", "Transposing instrument displayed in the UI")
               .arg(instrumentName, trait.name);
    case TraitType::Course:
        //: %1=name ("Tenor Lute"), %2=course/strings ("7-course"). Example: "Tenor Lute (7-course)"
        return muse::qtrc("notation", "%1 (%2)", "String instrument displayed in the UI")
               .arg(instrumentName, trait.name);
    case TraitType::Unknown:
        return instrumentName; // Example: "Flute"
    }
    Q_UNREACHABLE();
}

inline QString formatInstrumentTitle(const QString& instrumentName, const InstrumentTrait& trait, int instrumentNumber)
{
    if (instrumentNumber == 0) {
        // Only one instance of this instrument in the score
        return formatInstrumentTitle(instrumentName, trait);
    }

    QString number = QString::number(instrumentNumber);

    // Comments for translators start with //:
    switch (trait.type) {
    case TraitType::Tuning:
        //: %1=tuning ("D"), %2=name ("Tin Whistle"), %3=number ("2"). Example: "D Tin Whistle 2"
        return muse::qtrc("notation", "%1 %2 %3", "One of several tuned instruments displayed in the UI")
               .arg(trait.name, instrumentName, number);
    case TraitType::Transposition:
        //: %1=name ("Horn"), %2=transposition ("C alto"), %3=number ("2"). Example: "Horn in C alto 2"
        return muse::qtrc("notation", "%1 in %2 %3", "One of several transposing instruments displayed in the UI")
               .arg(instrumentName, trait.name, number);
    case TraitType::Course:
        //: %1=name ("Tenor Lute"), %2=course/strings ("7-course"), %3=number ("2"). Example: "Tenor Lute (7-course) 2"
        return muse::qtrc("notation", "%1 (%2) %3", "One of several string instruments displayed in the UI")
               .arg(instrumentName, trait.name, number);
    case TraitType::Unknown:
        //: %1=name ("Flute"), %2=number ("2"). Example: "Flute 2"
        return muse::qtrc("notation", "%1 %2", "One of several instruments displayed in the UI")
               .arg(instrumentName, number);
    }
    Q_UNREACHABLE();
}

struct PartInstrument
{
    muse::ID partId;
    InstrumentTemplate instrumentTemplate;

    bool isExistingPart = false;
    bool isSoloist = false;
};

using PartInstrumentList = QList<PartInstrument>;

struct PartInstrumentListScoreOrder
{
    PartInstrumentList instruments;
    ScoreOrder scoreOrder;
};

inline const ScoreOrder& customOrder()
{
    static ScoreOrder order;
    order.id = "custom";
    order.name = muse::TranslatableString("engraving/scoreorder", "Custom");

    return order;
}
}

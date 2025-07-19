/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "continiouselementsbuilder.h"

#include "engraving/dom/trill.h"
#include "engraving/dom/score.h"
#include "engraving/dom/ottava.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/textline.h"
#include "engraving/dom/vibrato.h"
#include "engraving/dom/factory.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
ContiniousElementsBuilder::ContiniousElementsBuilder(Score* score)
    : m_score(score)
{
}

static mu::engraving::ElementType muTypeFromImportType(ContiniousElementsBuilder::ImportType importType)
{
    using import_t = ContiniousElementsBuilder::ImportType;

    switch (importType) {
    case import_t::LET_RING:
        return ElementType::LET_RING;
    case import_t::PALM_MUTE:
        return ElementType::PALM_MUTE;
    case import_t::WHAMMY_BAR:
        return ElementType::WHAMMY_BAR;
    case import_t::RASGUEADO:
        return ElementType::RASGUEADO;
    case import_t::PICK_SCRAPE:
        return ElementType::PICK_SCRAPE;

    case import_t::HARMONIC_ARTIFICIAL:
    case import_t::HARMONIC_PINCH:
    case import_t::HARMONIC_TAP:
    case import_t::HARMONIC_SEMI:
    case import_t::HARMONIC_FEEDBACK:
        return ElementType::HARMONIC_MARK;

    case import_t::OTTAVA_MA15:
    case import_t::OTTAVA_VA8:
    case import_t::OTTAVA_VB8:
    case import_t::OTTAVA_MB15:
        return ElementType::OTTAVA;

    case import_t::TRILL:
        return ElementType::TRILL;
    case import_t::HAMMER_ON_PULL_OFF:
        return ElementType::HAMMER_ON_PULL_OFF;

    case import_t::HAIRPIN_CRESCENDO:
    case import_t::HAIRPIN_DIMINUENDO:
        return ElementType::HAIRPIN;

    case import_t::VIBRATO_LEFT_HAND_SLIGHT:
    case import_t::VIBRATO_LEFT_HAND_WIDE:
    case import_t::VIBRATO_W_TREM_BAR_SLIGHT:
    case import_t::VIBRATO_W_TREM_BAR_WIDE:
        return ElementType::VIBRATO;

    case import_t::NONE:
        return ElementType::INVALID;
    }

    return ElementType::INVALID;
}

static String harmonicText(ContiniousElementsBuilder::ImportType type)
{
    static const std::unordered_map<ContiniousElementsBuilder::ImportType, String> names {
        { ContiniousElementsBuilder::ImportType::HARMONIC_ARTIFICIAL, u"AH" },
        { ContiniousElementsBuilder::ImportType::HARMONIC_PINCH, u"PH" },
        { ContiniousElementsBuilder::ImportType::HARMONIC_TAP, u"TH" },
        { ContiniousElementsBuilder::ImportType::HARMONIC_SEMI, u"SH" },
        { ContiniousElementsBuilder::ImportType::HARMONIC_FEEDBACK, u"Fdbk" },
    };

    auto it = names.find(type);
    if (it != names.end()) {
        return it->second;
    }

    LOGE() << "wrong harmonic type";
    return String();
}

static mu::engraving::VibratoType vibratoTypeFromImportType(ContiniousElementsBuilder::ImportType type)
{
    static const std::unordered_map<ContiniousElementsBuilder::ImportType, VibratoType> names {
        { ContiniousElementsBuilder::ImportType::VIBRATO_LEFT_HAND_SLIGHT, VibratoType::GUITAR_VIBRATO },
        { ContiniousElementsBuilder::ImportType::VIBRATO_LEFT_HAND_WIDE, VibratoType::GUITAR_VIBRATO_WIDE },
        { ContiniousElementsBuilder::ImportType::VIBRATO_W_TREM_BAR_SLIGHT, VibratoType::VIBRATO_SAWTOOTH },
        { ContiniousElementsBuilder::ImportType::VIBRATO_W_TREM_BAR_WIDE, VibratoType::VIBRATO_SAWTOOTH_WIDE }
    };

    auto it = names.find(type);
    if (it != names.end()) {
        return it->second;
    }

    LOGE() << "wrong vibrato type";
    return VibratoType::GUITAR_VIBRATO;
}

static std::pair<bool, mu::engraving::OttavaType> ottavaType(ContiniousElementsBuilder::ImportType type)
{
    static const std::unordered_map<ContiniousElementsBuilder::ImportType, mu::engraving::OttavaType> types {
        { ContiniousElementsBuilder::ImportType::OTTAVA_VA8,  mu::engraving::OttavaType::OTTAVA_8VA },
        { ContiniousElementsBuilder::ImportType::OTTAVA_VB8,  mu::engraving::OttavaType::OTTAVA_8VB },
        { ContiniousElementsBuilder::ImportType::OTTAVA_MA15, mu::engraving::OttavaType::OTTAVA_15MA },
        { ContiniousElementsBuilder::ImportType::OTTAVA_MB15, mu::engraving::OttavaType::OTTAVA_15MB }
    };

    auto it = types.find(type);
    if (it != types.end()) {
        return { true, it->second };
    }

    return { false, mu::engraving::OttavaType::OTTAVA_8VA };
}

/// whenever the continious line elements should end on rest or rest should be included in it
static bool shouldSplitByRests(mu::engraving::ElementType muType)
{
    switch (muType) {
    case ElementType::PICK_SCRAPE:
    case ElementType::TRILL:
    case ElementType::HARMONIC_MARK:
    case ElementType::VIBRATO:
    case ElementType::HAMMER_ON_PULL_OFF:
        return true;

    default:
        return false;
    }
}

void ContiniousElementsBuilder::buildContiniousElement(ChordRest* cr, ElementType muType, ImportType importType, bool elemExists,
                                                       sub_type_t subType)
{
    auto setStartCR = [](Spanner* elem, ChordRest* cr) {
        elem->setTick(cr->tick());
        elem->setStartElement(cr);
    };

    auto setEndCR = [](Spanner* elem, ChordRest* cr) {
        elem->setTick2(cr->tick() + cr->actualTicks());
        elem->setEndElement(cr);
    };

    bool splitByRests = shouldSplitByRests(muType);

    track_idx_t track = cr->track();

    auto& elements = m_elementsByType[importType];
    while (elements.size() < track + 1) {
        elements.push_back(nullptr);
    }

    auto& elem = elements[track];
    auto& lastTypeForTrack = m_lastImportTypes[muType][track][subType];

    ContiniousElementState state = calculateState(cr->isRest(), elemExists, splitByRests, lastTypeForTrack != importType);

    /// handling the continious element according to its state
    switch (state) {
    case ContiniousElementState::CHORD_NO_ELEMENT:
        lastTypeForTrack = ImportType::NONE;
        elem = nullptr;
        break;

    case ContiniousElementState::REST_BREAK:
        lastTypeForTrack = ImportType::NONE;
        elem = nullptr;
        break;

    case ContiniousElementState::REST_CONTINUE:
    case ContiniousElementState::ELEMENT_ON_REST: // currently it's mistake: we cannot indicate Rest with any of continious elements in guitar pro
        if (lastTypeForTrack != ImportType::NONE) {
            m_elementsToAddToScore[track][lastTypeForTrack].endedOnRest = true;
        }

        break;

    case ContiniousElementState::CONTINUE_CURRENT_LINE:
    {
        ChordRest* lastCR = elem->endCR();
        if (lastCR == cr) {
            break;
        }

        if (elem->tick2() < cr->tick()) {
            if (lastTypeForTrack != ImportType::NONE) {
                auto& lastTypeElementsToAdd = m_elementsToAddToScore[track][lastTypeForTrack];

                /// removing info about the Rest and updating last element's ticks
                if (lastTypeElementsToAdd.endedOnRest) {
                    lastTypeElementsToAdd.endedOnRest = false;
                    Spanner* prevElem = lastTypeElementsToAdd.elements.back();
                    if (!prevElem) {
                        LOGE() << "error while importing";
                        return;
                    }

                    elem = prevElem;
                    setEndCR(elem, cr);
                } else {
                    // Simile mark case
                    setEndCR(elem, cr);
                }
            }
        } else {
            setEndCR(elem, cr);
        }

        break;
    }

    case ContiniousElementState::CREATE_NEW_ITEM:
    {
        EngravingItem* engItem = Factory::createItem(muType, m_score->dummy());

        Spanner* newElem = dynamic_cast<Spanner*>(engItem);
        IF_ASSERT_FAILED(newElem) {
            return;
        }

        elem = newElem;

        setStartCR(newElem, cr);
        setEndCR(newElem, cr);

        newElem->setTrack(track);
        newElem->setTrack2(track);

        m_elementsToAddToScore[track][importType].elements.push_back(newElem);
        m_orderedAddedElements.push_back(newElem);
        lastTypeForTrack = importType;
        break;
    }

    default:
        break;
    }

    if (!cr->isChord()) {
        return;
    }

    setupAddedElement(cr->track(), importType);
}

void ContiniousElementsBuilder::notifyUncompletedMeasure()
{
    for (auto& trackMaps : m_elementsToAddToScore) {
        for (auto& typeMaps : trackMaps.second) {
            typeMaps.second.endedOnRest = true;
        }
    }
}

static Chord* searchEndChord(Chord* startChord)
{
    ChordRest* nextCr = nullptr;
    if (startChord->isGrace()) {
        //! this case when start note is a grace note so end note can be next note in grace notes
        //! or parent note of grace notes
        Chord* parentGrace = static_cast<Chord*>(startChord->parent());

        auto it = parentGrace->graceNotes().begin();
        for (; it != parentGrace->graceNotes().end(); ++it) {
            if (*it == startChord) {
                break;
            }
        }

        if (it == parentGrace->graceNotes().end()) {
            nextCr = nullptr;
        } else if (std::next(it) == parentGrace->graceNotes().end()) {
            nextCr = parentGrace;
        } else {
            nextCr = *(++it);
        }
    } else {
        nextCr = startChord->segment()->next1()->nextChordRest(startChord->track());
        if (!nextCr) {
            return nullptr;
        }

        if (nextCr->isChord() && !static_cast<Chord*>(nextCr)->graceNotes().empty()) {
            nextCr = static_cast<Chord*>(nextCr)->graceNotes().front();
        }
    }

    return (nextCr && nextCr->isChord()) ? toChord(nextCr) : nullptr;
}

void ContiniousElementsBuilder::addElementsToScore()
{
    for (auto& trackMaps: m_spannersWithoutEndElement) {
        for (auto& typeMaps : trackMaps.second) {
            for (Spanner* elem : typeMaps.second) {
                elem->setEndElement(searchEndChord(elem->endChord()));
            }
        }
    }

    for (Spanner* elem : m_orderedAddedElements) {
        m_score->addElement(elem);
    }
}

/// indicating the type of behaviour for continious element
ContiniousElementsBuilder::ContiniousElementState ContiniousElementsBuilder::calculateState(bool isRest, bool elemExists, bool splitByRests,
                                                                                            bool importTypeChanged) const
{
    ContiniousElementState state = ContiniousElementState::UNDEFINED;

    if (isRest && splitByRests) {
        state = ContiniousElementState::REST_BREAK;
    } else {
        if (!elemExists) {
            if (!isRest) {
                state = ContiniousElementState::CHORD_NO_ELEMENT;
            } else {
                state = ContiniousElementState::REST_CONTINUE;
            }
        } else {
            // element exists on current beat
            if (isRest) {
                state = ContiniousElementState::ELEMENT_ON_REST;
            } else if (importTypeChanged) {
                state = ContiniousElementState::CREATE_NEW_ITEM;
            } else {
                state = ContiniousElementState::CONTINUE_CURRENT_LINE;
            }
        }
    }

    return state;
}

void ContiniousElementsBuilder::setupAddedElement(track_idx_t trackIdx, ImportType importType)
{
    ElementType muType = muTypeFromImportType(importType);

    Spanner* lineElem = m_elementsByType[importType][trackIdx];
    if (!lineElem) {
        return;
    }

    switch (muType) {
    case ElementType::TRILL:
    {
        if (lineElem->isTrill()) {
            toTrill(lineElem)->setTrillType(TrillType::TRILL_LINE);
        }

        break;
    }
    case ElementType::HAIRPIN:
    {
        if (lineElem->isHairpin()) {
            toHairpin(lineElem)->setHairpinType(
                importType == ImportType::HAIRPIN_CRESCENDO ? HairpinType::CRESC_HAIRPIN : HairpinType::DECRESC_HAIRPIN);
        }

        break;
    }
    case ElementType::HARMONIC_MARK:
    {
        if (lineElem->isTextLineBase()) {
            const String& text = harmonicText(importType);
            toTextLineBase(lineElem)->setBeginText(text);
            toTextLineBase(lineElem)->setContinueText(text);
        }

        break;
    }
    case ElementType::OTTAVA:
    {
        const auto& [foundOttava, muOttavaType] = ottavaType(importType);
        if (!foundOttava) {
            return;
        }

        if (lineElem->isOttava()) {
            toOttava(lineElem)->setOttavaType(muOttavaType);
        }

        break;
    }
    case ElementType::VIBRATO:
        if (lineElem->isVibrato()) {
            toVibrato(lineElem)->setVibratoType(vibratoTypeFromImportType(importType));
        }

        break;
    case ElementType::HAMMER_ON_PULL_OFF:
        m_spannersWithoutEndElement[trackIdx][importType].insert(lineElem);
        break;
    default:
        break;
    }
}
} // namespace mu::iex::guitarpro

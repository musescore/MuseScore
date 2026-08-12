/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "readcontext.h"

#include "dom/linkedobjects.h"
#include "dom/masterscore.h"
#include "dom/note.h"
#include "dom/noteline.h"
#include "dom/score.h"
#include "dom/trill.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::read500;

ReadContext::ReadContext(Score* score, bool pasteMode)
    : m_score(score), _pasteMode(pasteMode)
{
}

ReadContext::~ReadContext()
{
    IF_ASSERT_FAILED(m_incompleteNoteAnchoredSpanners.empty()) {
        for (auto& [spanner, startEndEIDs] : m_incompleteNoteAnchoredSpanners) {
            delete spanner;
        }

        m_incompleteNoteAnchoredSpanners.clear();
    }
}

Score* ReadContext::score() const
{
    return m_score;
}

void ReadContext::setScore(Score* score)
{
    m_score = score;
}

const MStyle& ReadContext::style() const
{
    return score()->style();
}

std::shared_ptr<IEngravingFontsProvider> ReadContext::engravingFonts() const
{
    return score()->engravingFonts();
}

String ReadContext::mscoreVersion() const
{
    return m_score->mscoreVersion();
}

int ReadContext::mscVersion() const
{
    return m_score->mscVersion();
}

int ReadContext::fileDivision() const
{
    return m_score->fileDivision();
}

int ReadContext::fileDivision(int t) const
{
    return m_score->fileDivision(t);
}

double ReadContext::spatium() const
{
    return m_score->style().spatium();
}

void ReadContext::setSpatium(double v)
{
    m_score->style().set(Sid::spatium, v);
}

compat::DummyElement* ReadContext::dummy() const
{
    return m_score->dummy();
}

Staff* ReadContext::staff(staff_idx_t n)
{
    return m_score->staff(n);
}

bool ReadContext::isSameScore(const EngravingObject* obj) const
{
    return obj->score() == m_score;
}

Fraction ReadContext::rtick() const
{
    return _curMeasure ? _tick - _curMeasure->tick() : _tick;
}

void ReadContext::setTick(const Fraction& f)
{
    _tick = f.reduced();
    _intTick = _tick.ticks();
}

void ReadContext::incTick(const Fraction& f)
{
    _tick += f;
    _tick.reduce();
    _intTick += f.ticks();
}

Location ReadContext::location(bool forceAbsFrac) const
{
    Location l = Location::absolute();
    fillLocation(l, forceAbsFrac);
    return l;
}

//---------------------------------------------------------
//   fillLocation
//    fills location fields which have default values with
//    values relevant for the current reader's position.
//    When in paste mode (or forceAbsFrac is true) absolute
//    fraction values are used and measure number is set to
//    zero.
//---------------------------------------------------------

void ReadContext::fillLocation(Location& l, bool forceAbsFrac) const
{
    constexpr Location defaults = Location::absolute();
    const bool absFrac = (pasteMode() || forceAbsFrac);
    if (l.track() == defaults.track()) {
        l.setTrack(static_cast<int>(track()));
    }
    if (l.frac() == defaults.frac()) {
        l.setFrac(absFrac ? tick() : rtick());
    }
    if (l.measure() == defaults.measure()) {
        l.setMeasure(absFrac ? 0 : currentMeasureIndex());
    }
}

//---------------------------------------------------------
//   setLocation
//    sets a new reading location, taking into account its
//    type (absolute or relative).
//---------------------------------------------------------

void ReadContext::setLocation(const Location& l)
{
    if (l.isRelative()) {
        Location newLoc = l;
        newLoc.toAbsolute(location());
        int intTicks = l.frac().ticks();
        if (_tick == Fraction::fromTicks(_intTick + intTicks)) {
            _intTick += intTicks;
            setTrack(newLoc.track() - _trackOffset);
            return;
        }
        setLocation(newLoc); // recursion
        return;
    }
    setTrack(l.track() - _trackOffset);
    setTick(l.frac() - _tickOffset);
    if (!pasteMode()) {
        assert(l.measure() == currentMeasureIndex());
        incTick(currentMeasure()->tick());
    }
}

void ReadContext::addPartAudioSettingCompat(PartAudioSettingsCompat partAudioSetting)
{
    if (_settingsCompat.audioSettings.count(partAudioSetting.instrumentId.partId) == 0) {
        _settingsCompat.audioSettings.insert({ partAudioSetting.instrumentId.partId, partAudioSetting });
    }
}

void ReadContext::addMMRestEndMeasureEID(Measure* mmrest, EID lastMeasureEID)
{
    DO_ASSERT(lastMeasureEID.isValid());
    m_mmRestEndMeasures.emplace(mmrest, lastMeasureEID);
}

void ReadContext::setMMRestEndMeasures()
{
    EIDRegister* eidRegister = score()->masterScore()->eidRegister();
    for (auto& [mmrest, lastMeasureEID] : m_mmRestEndMeasures) {
        EngravingObject* linkedElement = eidRegister->itemFromEID(lastMeasureEID);
        IF_ASSERT_FAILED(linkedElement && linkedElement->isMeasure()) {
            LOGE() << "No valid end measure found for MMRest at " << mmrest->tick().toString();
            continue;
        }

        Measure* lastMeasure = toMeasure(linkedElement);
        MeasureBase* nextMB = lastMeasure->next();
        mmrest->setNext(nextMB);
    }
}

void ReadContext::registerPastedEID(const EID& clipboardEid, const EID& fileEid)
{
    m_pastedEIDs.emplace(clipboardEid, fileEid);
}

EID ReadContext::resolvePastedEID(const EID& clipboardEid) const
{
    if (!_pasteMode) {
        return clipboardEid;
    }

    auto it = m_pastedEIDs.find(clipboardEid);
    IF_ASSERT_FAILED(it != m_pastedEIDs.end()) {
        return clipboardEid;
    }

    return it->second;
}

void ReadContext::addNoteAnchoredSpannerStartEl(Spanner* spanner, EID startElementEID)
{
    auto it = m_incompleteNoteAnchoredSpanners.find(spanner);
    if (it != m_incompleteNoteAnchoredSpanners.end()) {
        it->second.first = startElementEID;
    } else {
        m_incompleteNoteAnchoredSpanners.emplace(spanner, std::make_pair(startElementEID, EID::invalid()));
    }
}

void ReadContext::addNoteAnchoredSpannerEndEl(Spanner* spanner, EID endElementEID)
{
    auto it = m_incompleteNoteAnchoredSpanners.find(spanner);
    if (it != m_incompleteNoteAnchoredSpanners.end()) {
        it->second.second = endElementEID;
    } else {
        m_incompleteNoteAnchoredSpanners.emplace(spanner, std::make_pair(EID::invalid(), endElementEID));
    }
}

void ReadContext::connectNoteAnchoredSpanners()
{
    EIDRegister* eidRegister = score()->masterScore()->eidRegister();
    for (auto& [spanner, startEndEIDs] : m_incompleteNoteAnchoredSpanners) {
        EID startElementEID = startEndEIDs.first;
        EID endElementEID = startEndEIDs.second;
        EngravingItem* startElement = toEngravingItem(eidRegister->itemFromEID(startElementEID));
        EngravingItem* endElement = toEngravingItem(eidRegister->itemFromEID(endElementEID));
        IF_ASSERT_FAILED(startElement && startElement->isNote()) {
            LOGD() << "Could not find note element for " << spanner->typeName();
            delete spanner;
            continue;
        }
        IF_ASSERT_FAILED(endElement && endElement->isNote()) {
            LOGD() << "Could not find end note for " << spanner->typeName();
            delete spanner;
            continue;
        }

        Note* startNote = toNote(startElement);
        Note* endNote = toNote(endElement);
        spanner->setNoteSpan(startNote, endNote);

        if (spanner->isTie()) {
            Tie* tie = toTie(spanner);
            startNote->setTieFor(tie);
            endNote->setTieBack(tie);
        } else {
            score()->addElement(spanner);
        }
    }
    m_incompleteNoteAnchoredSpanners.clear();
}

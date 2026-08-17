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

#pragma once

#include <vector>

#include "iengravingfontsprovider.h"

#include "../types/types.h"

#include "dom/interval.h"
#include "dom/location.h"
#include "../infrastructure/eid.h"

namespace mu::engraving {
class Beam;
class EngravingObject;
class LinkedObjects;
class Measure;
class MStyle;
class Score;
class Spanner;
class Staff;
class Tuplet;
}

namespace mu::engraving::compat {
class DummyElement;
}

namespace mu::engraving::read500 {
struct TextStyleMap {
    String name;
    TextStyleType ss;
};

class ReadContext
{
public:

    ReadContext(Score* score, bool pasteMode = false);
    ~ReadContext();

    Score* score() const;
    void setScore(Score* score);

    const MStyle& style() const;
    std::shared_ptr<IEngravingFontsProvider> engravingFonts() const;

    bool pasteMode() const { return _pasteMode; }

    String mscoreVersion() const;
    int mscVersion() const;

    int fileDivision() const;
    int fileDivision(int t) const;

    double spatium() const;
    void setSpatium(double v);
    void setPropertiesToSkip(const PropertyIdSet& propertiesToSkip) { m_propertiesToSkip = propertiesToSkip; }
    bool shouldSkipProperty(Pid pid) const { return muse::contains(m_propertiesToSkip, pid); }

    bool forcePageMode() const { return m_forcePageMode; }
    void setForcePageMode(bool v) { m_forcePageMode = v; }

    compat::DummyElement* dummy() const;

    Staff* staff(staff_idx_t n);

    bool isSameScore(const EngravingObject* obj) const;

    Fraction tick()  const { return _tick + _tickOffset; }
    Fraction rtick()  const;
    Fraction tickOffset() const { return _tickOffset; }
    void setTick(const Fraction& f);
    void incTick(const Fraction& f);
    void setTickOffset(const Fraction& val) { _tickOffset = val; }

    track_idx_t track() const { return _track + _trackOffset; }
    void setTrackOffset(int val) { _trackOffset = val; }
    int trackOffset() const { return _trackOffset; }
    void setTrack(track_idx_t val) { _track = val; }

    void setCurrentMeasure(Measure* m) { _curMeasure = m; }
    Measure* currentMeasure() const { return _curMeasure; }
    void setLastMeasure(Measure* m) { _lastMeasure = m; }
    Measure* lastMeasure() const { return _lastMeasure; }
    void setCurrentMeasureIndex(int idx) { _curMeasureIdx = idx; }
    int currentMeasureIndex() const { return _curMeasureIdx; }

    Interval transpose() const { return _transpose; }
    void setTransposeChromatic(int8_t v) { _transpose.chromatic = v; }
    void setTransposeDiatonic(int8_t v) { _transpose.diatonic = v; }

    TracksMap& tracks() { return _tracks; }

    void addPartAudioSettingCompat(PartAudioSettingsCompat partAudioSetting);
    const SettingsCompat& settingCompat() { return _settingsCompat; }

    Fraction timeSigForNextMeasure() { return m_timeSigForNextMeasure; }
    void setTimeSigForNextMeasure(Fraction timeSig) { m_timeSigForNextMeasure = timeSig; }

    Location location(bool forceAbsFrac = false) const;
    void fillLocation(Location&, bool forceAbsFrac = false) const;
    void setLocation(const Location&);   // sets a new reading point, taking into account its type (absolute or relative).

    void addMMRestEndMeasureEID(Measure* mmrest, EID lastMeasureEID);
    void setMMRestEndMeasures();

    void registerPastedEID(const EID& clipboardEid, const EID& fileEid);
    EID resolvePastedEID(const EID& clipboardEid) const;

    void addNoteAnchoredSpannerStartEl(Spanner* spanner, EID startElementEID);
    void addNoteAnchoredSpannerEndEl(Spanner* spanner, EID endElementEID);
    void connectNoteAnchoredSpanners();

private:
    Score* m_score = nullptr;

    bool _pasteMode = false;  // modifies read behaviour on paste operation
    bool m_forcePageMode = false;

    Fraction _tick             { Fraction(0, 1) };
    Fraction _tickOffset       { Fraction(0, 1) };
    int _intTick = 0;

    int _trackOffset = 0;
    track_idx_t _track = 0;

    Measure* _curMeasure = nullptr;
    Measure* _lastMeasure = nullptr;
    int _curMeasureIdx = 0;

    Interval _transpose;
    TracksMap _tracks;

    SettingsCompat _settingsCompat;

    PropertyIdSet m_propertiesToSkip;

    Fraction m_timeSigForNextMeasure = Fraction(0, 1);

    std::unordered_map<Measure*, EID> m_mmRestEndMeasures;

    // On pasting we must create new EIDs for the elements, as the serialized EIDs could be used elsewhere in the score
    // This map preserves the link between the old EID used to reference items within the section of the score which was copied and the newly created EID
    std::unordered_map<EID, EID> m_pastedEIDs;

    std::map<Spanner*, std::pair<EID, EID> > m_incompleteNoteAnchoredSpanners;
};
}

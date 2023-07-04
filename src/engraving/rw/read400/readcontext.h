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

#ifndef MU_ENGRAVING_READCONTEXT_H
#define MU_ENGRAVING_READCONTEXT_H

#include <map>

#include "global/modularity/ioc.h"
#include "iengravingfontsprovider.h"

#include "types/types.h"

#include "libmscore/connector.h"
#include "libmscore/interval.h"
#include "libmscore/location.h"
#include "libmscore/sig.h"

#include "../linksindexer.h"
#include "../inoutdata.h"

#include "connectorinforeader.h"

namespace mu::engraving {
class Beam;
class EngravingObject;
class LinkedObjects;
class Measure;
class Score;
class Spanner;
class Staff;
class Tuplet;
class MStyle;
}

namespace mu::engraving::compat {
class DummyElement;
}

namespace mu::engraving::read400 {
struct SpannerValues {
    int spannerId;
    Fraction tick2;
    track_idx_t track2;
};

struct TextStyleMap {
    String name;
    TextStyleType ss;
};

class ReadContext
{
    INJECT(IEngravingFontsProvider, engravingFonts)
public:

    ReadContext() = default;
    ReadContext(Score* score);
    ~ReadContext();

    void setScore(Score* score);
    Score* score() const;
    bool isMasterScore() const;

    void setMasterCtx(ReadContext* ctx);
    ReadContext* masterCtx();
    const ReadContext* masterCtx() const;

    const MStyle& style() const;

    bool pasteMode() const { return _pasteMode; }
    void setPasteMode(bool v) { _pasteMode = v; }

    String mscoreVersion() const;
    int mscVersion() const;

    int fileDivision() const;
    int fileDivision(int t) const;

    double spatium() const;

    compat::DummyElement* dummy() const;

    Staff* staff(int n);

    void appendStaff(Staff* staff);
    void addSpanner(Spanner* s);

    bool undoStackActive() const;

    bool isSameScore(const EngravingObject* obj) const;

    bool hasAccidental = false; // used for userAccidental backward compatibility

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

    void addBeam(Beam* s);
    Beam* findBeam(int id) const { return mu::value(_beams, id, nullptr); }

    void addTuplet(Tuplet* s);
    Tuplet* findTuplet(int id) const { return mu::value(_tuplets, id, nullptr); }
    std::unordered_map<int, Tuplet*>& tuplets() { return _tuplets; }
    void checkTuplets();

    void removeSpanner(const Spanner*);
    void addSpanner(int id, Spanner*);
    Spanner* findSpanner(int id);

    int spannerId(const Spanner*);        // returns spanner id, allocates new one if none exists

    void addSpannerValues(const SpannerValues& sv) { _spannerValues.push_back(sv); }
    const SpannerValues* spannerValues(int id) const;

    Interval transpose() const { return _transpose; }
    void setTransposeChromatic(int8_t v) { _transpose.chromatic = v; }
    void setTransposeDiatonic(int8_t v) { _transpose.diatonic = v; }

    TracksMap& tracks() { return _tracks; }

    TextStyleType addUserTextStyle(const String& name);
    TextStyleType lookupUserTextStyle(const String& name) const;
    void clearUserTextStyles() { userTextStyles.clear(); }

    std::list<std::pair<EngravingItem*, mu::PointF> >& fixOffsets() { return _fixOffsets; }

    void addPartAudioSettingCompat(PartAudioSettingsCompat partAudioSetting);
    const SettingsCompat& settingCompat() { return _settingsCompat; }

    TimeSigMap* compatTimeSigMap();

    Location location(bool forceAbsFrac = false) const;
    void fillLocation(Location&, bool forceAbsFrac = false) const;
    void setLocation(const Location&);   // sets a new reading point, taking into account its type (absolute or relative).

    rw::ReadLinks readLinks() const;
    void initLinks(const rw::ReadLinks& l);
    void addLink(Staff* staff, LinkedObjects* link, const Location& location);
    LinkedObjects* getLink(bool isMasterScore, const Location& location, int localIndexDiff);
    std::map<int, std::vector<std::pair<LinkedObjects*, Location> > >& staffLinkedElements();
    std::map<int, LinkedObjects*>& linkIds();

    void addConnectorInfoLater(std::shared_ptr<read400::ConnectorInfoReader> c);   // add connector info to be checked after calling checkConnectors()
    void checkConnectors();
    void reconnectBrokenConnectors();
    void clearOrphanedConnectors();

private:

    Location doLocation(bool forceAbsFrac = false) const;
    void doFillLocation(Location&, bool forceAbsFrac = false) const;
    void doSetLocation(const Location&);

    rw::ReadLinks doReadLinks() const;
    void doInitLinks(const rw::ReadLinks& l);
    void doAddLink(Staff* staff, LinkedObjects* link, const Location& location);
    LinkedObjects* doGetLink(bool isMasterScore, const Location& location, int localIndexDiff);

    void doCheckConnectors();
    void doReconnectBrokenConnectors();

    void addConnectorInfo(std::shared_ptr<read400::ConnectorInfoReader>);
    void removeConnector(const read400::ConnectorInfoReader*);   // Removes the whole ConnectorInfo chain from the connectors list.

    Score* m_score = nullptr;
    ReadContext* m_masterCtx = nullptr;

    bool _pasteMode = false;  // modifies read behaviour on paste operation

    std::map<int /*staffIndex*/, std::vector<std::pair<LinkedObjects*, Location> > > m_staffLinkedElements; // one list per staff
    LinksIndexer m_linksIndexer;

    std::map<int, LinkedObjects*> _elinks;       // for reading old files (< 3.01)

    std::vector<std::shared_ptr<read400::ConnectorInfoReader> > _connectors;
    std::vector<std::shared_ptr<read400::ConnectorInfoReader> > _pendingConnectors;  // connectors that are pending to be updated and added to _connectors. That will happen when checkConnectors() is called.

    Fraction _tick             { Fraction(0, 1) };
    Fraction _tickOffset       { Fraction(0, 1) };
    int _intTick = 0;

    track_idx_t _track = 0;
    int _trackOffset = 0;

    Measure* _curMeasure = nullptr;
    Measure* _lastMeasure = nullptr;
    int _curMeasureIdx = 0;

    std::unordered_map<int, Beam*> _beams;
    std::unordered_map<int, Tuplet*> _tuplets;

    std::list<SpannerValues> _spannerValues;
    std::list<std::pair<int, Spanner*> > _spanner;

    Interval _transpose;
    TracksMap _tracks;

    std::list<TextStyleMap> userTextStyles;

    std::list<std::pair<EngravingItem*, mu::PointF> > _fixOffsets;
    SettingsCompat _settingsCompat;

    TimeSigMap m_compatTimeSigMap;
};
}

#endif // MU_ENGRAVING_READCONTEXT_H

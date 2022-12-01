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

#ifndef __STAFF_H__
#define __STAFF_H__

#include <map>
#include <vector>

#include "engravingitem.h"

#include "draw/types/color.h"

#include "changeMap.h"
#include "cleflist.h"
#include "groups.h"
#include "keylist.h"
#include "pitch.h"
#include "stafftypelist.h"

namespace mu::engraving {
class BracketItem;
class Clef;
class Factory;
class InstrumentTemplate;
class KeyList;
class Note;
class Part;
class Score;
class StaffType;
class TimeSig;

class XmlReader;
class XmlWriter;

enum class Key;

//---------------------------------------------------------
//   SwingParameters
//---------------------------------------------------------

struct SwingParameters {
    int swingUnit = 0;
    int swingRatio = 0;

    bool isOn() const { return swingUnit != 0; }
};

//---------------------------------------------------------
//    Staff
///    Global staff data not directly related to drawing.
//---------------------------------------------------------

class Staff final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Staff)
public:
    enum class HideMode {
        AUTO, ALWAYS, NEVER, INSTRUMENT
    };

private:
    ID _id = INVALID_ID;
    Part* _part = nullptr;

    ClefList clefs;
    ClefTypeList _defaultClefType;

    KeyList _keys;
    std::map<int, TimeSig*> timesigs;

    std::vector<BracketItem*> _brackets;
    int _barLineSpan         { false };       ///< true - span barline to next staff
    int _barLineFrom         { 0 };          ///< line of start staff to draw the barline from (0 = staff top line, ...)
    int _barLineTo           { 0 };          ///< line of end staff to draw the bar line to (0= staff bottom line, ...)

    bool _cutaway            { false };
    bool _showIfEmpty        { false };         ///< show this staff if system is empty and hideEmptyStaves is true
    bool _hideSystemBarLine  { false };         // no system barline if not preceded by staff with barline
    bool _mergeMatchingRests { false };         // merge matching rests in multiple voices
    HideMode _hideWhenEmpty  { HideMode::AUTO };      // hide empty staves

    mu::draw::Color _color   { engravingConfiguration()->defaultColor() };
    Millimetre _userDist     { Millimetre(0.0) };           ///< user edited extra distance

    StaffTypeList _staffTypeList;

    std::map<int, int> _channelList[VOICES];
    std::map<int, SwingParameters> _swingList;
    std::map<int, int> _capoList;
    bool _playbackVoice[VOICES] { true, true, true, true };
    std::array<bool, VOICES> _visibilityVoices { true, true, true, true };

    ChangeMap _velocities;           ///< cached value
    ChangeMap _velocityMultiplications;         ///< cached value
    PitchList _pitchOffsets;        ///< cached value

    friend class Factory;
    Staff(Part* parent);
    Staff(const Staff& staff);

    void fillBrackets(size_t idx);
    void cleanBrackets();

    double staffMag(const StaffType*) const;

    friend class Excerpt;
    void setVoiceVisible(voice_idx_t voice, bool visible);
    void updateVisibilityVoices(Staff* masterStaff, const TracksMap& tracks);

public:
    Staff* clone() const override;

    void init(const InstrumentTemplate*, const StaffType* staffType, int);
    void initFromStaffType(const StaffType* staffType);
    void init(const Staff*);

    const ID& id() const;
    void setId(const ID& id);

    void setScore(Score* score) override;

    bool isTop() const;
    String partName() const;
    staff_idx_t rstaff() const;
    staff_idx_t idx() const;
    void read(XmlReader&) override;
    bool readProperties(XmlReader&) override;
    void write(XmlWriter& xml) const override;
    Part* part() const { return _part; }
    void setPart(Part* p) { _part = p; }

    BracketType bracketType(size_t idx) const;
    size_t bracketSpan(size_t idx) const;
    void setBracketType(size_t idx, BracketType val);
    void setBracketSpan(size_t idx, size_t val);
    void swapBracket(size_t oldIdx, size_t newIdx);
    void changeBracketColumn(size_t oldColumn, size_t newColumn);
    void addBracket(BracketItem*);
    const std::vector<BracketItem*>& brackets() const { return _brackets; }
    std::vector<BracketItem*>& brackets() { return _brackets; }
    void cleanupBrackets();
    size_t bracketLevels() const;

    ClefList& clefList() { return clefs; }
    ClefTypeList clefType(const Fraction&) const;
    ClefTypeList defaultClefType() const { return _defaultClefType; }
    void setDefaultClefType(const ClefTypeList& l) { _defaultClefType = l; }
    ClefType clef(const Fraction&) const;
    Fraction nextClefTick(const Fraction&) const;
    Fraction currentClefTick(const Fraction&) const;

    String staffName() const;

    void setClef(Clef*);
    void removeClef(const Clef*);

    void addTimeSig(TimeSig*);
    void removeTimeSig(TimeSig*);
    void clearTimeSig();
    Fraction timeStretch(const Fraction&) const;
    TimeSig* timeSig(const Fraction&) const;
    TimeSig* nextTimeSig(const Fraction&) const;
    Fraction currentTimeSigTick(const Fraction&) const;

    bool isLocalTimeSignature(const Fraction& tick) { return timeStretch(tick) != Fraction(1, 1); }

    const Groups& group(const Fraction&) const;

    KeyList* keyList() { return &_keys; }
    Key key(const Fraction& tick) const { return keySigEvent(tick).key(); }
    KeySigEvent keySigEvent(const Fraction&) const;
    Fraction nextKeyTick(const Fraction&) const;
    Fraction currentKeyTick(const Fraction&) const;
    KeySigEvent prevKey(const Fraction&) const;
    void setKey(const Fraction&, KeySigEvent);
    void removeKey(const Fraction&);

    bool show() const;
    bool stemless(const Fraction&) const;
    bool cutaway() const { return _cutaway; }
    void setCutaway(bool val) { _cutaway = val; }
    bool showIfEmpty() const { return _showIfEmpty; }
    void setShowIfEmpty(bool val) { _showIfEmpty = val; }

    bool hideSystemBarLine() const { return _hideSystemBarLine; }
    void setHideSystemBarLine(bool val) { _hideSystemBarLine = val; }
    HideMode hideWhenEmpty() const { return _hideWhenEmpty; }
    void setHideWhenEmpty(HideMode v) { _hideWhenEmpty = v; }
    bool mergeMatchingRests() const { return _mergeMatchingRests; }
    void setMergeMatchingRests(bool val) { _mergeMatchingRests = val; }

    int barLineSpan() const { return _barLineSpan; }
    int barLineFrom() const { return _barLineFrom; }
    int barLineTo() const { return _barLineTo; }
    void setBarLineSpan(int val) { _barLineSpan = val; }
    void setBarLineFrom(int val) { _barLineFrom = val; }
    void setBarLineTo(int val) { _barLineTo = val; }
    double height() const override;

    int channel(const Fraction&, voice_idx_t voice) const;

    std::list<Note*> getNotes() const;
    void addChord(std::list<Note*>& list, Chord* chord, voice_idx_t voice) const;

    void clearChannelList(voice_idx_t voice) { _channelList[voice].clear(); }
    void insertIntoChannelList(voice_idx_t voice, const Fraction& tick, int channelId)
    {
        _channelList[voice].insert({ tick.ticks(), channelId });
    }

    SwingParameters swing(const Fraction&)  const;
    void clearSwingList() { _swingList.clear(); }
    void insertIntoSwingList(const Fraction& tick, SwingParameters sp) { _swingList.insert({ tick.ticks(), sp }); }

    int capo(const Fraction&) const;
    void clearCapoList() { _capoList.clear(); }
    void insertIntoCapoList(const Fraction& tick, int fretId) { _capoList.insert({ tick.ticks(), fretId }); }

    //==== staff type helper function
    const StaffType* staffType(const Fraction& = Fraction(0, 1)) const;
    const StaffType* constStaffType(const Fraction&) const;
    const StaffType* staffTypeForElement(const EngravingItem*) const;
    bool isStaffTypeStartFrom(const Fraction& = Fraction(0, 1)) const;
    void moveStaffType(const Fraction& from, const Fraction& to);
    StaffType* staffType(const Fraction&);
    StaffType* setStaffType(const Fraction&, const StaffType&);
    void removeStaffType(const Fraction&);
    void staffTypeListChanged(const Fraction&);

    bool isPitchedStaff(const Fraction&) const;
    bool isTabStaff(const Fraction&) const;
    bool isDrumStaff(const Fraction&) const;

    int lines(const Fraction&) const;
    void setLines(const Fraction&, int lines);
    double lineDistance(const Fraction&) const;

    bool isLinesInvisible(const Fraction&) const;
    void setIsLinesInvisible(const Fraction&, bool val);

    void setSlashStyle(const Fraction&, bool val);
    int middleLine(const Fraction&) const;
    int bottomLine(const Fraction&) const;

    double staffMag(const Fraction&) const;
    double staffMag(const EngravingItem* element) const;
    double spatium(const Fraction&) const;
    double spatium(const EngravingItem*) const;
    //===========

    ChangeMap& velocities() { return _velocities; }
    ChangeMap& velocityMultiplications() { return _velocityMultiplications; }
    PitchList& pitchOffsets() { return _pitchOffsets; }

    int pitchOffset(const Fraction& tick) { return _pitchOffsets.pitchOffset(tick.ticks()); }
    void updateOttava();

    std::list<Staff*> staffList() const;
    Staff* primaryStaff() const;
    bool isPrimaryStaff() const;

    Millimetre userDist() const { return _userDist; }
    void setUserDist(Millimetre val) { _userDist = val; }

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;
    void setLocalSpatium(double oldVal, double newVal, Fraction tick);
    bool genKeySig();
    bool showLedgerLines(const Fraction&) const;

    using EngravingItem::color;
    using EngravingItem::setColor;
    mu::draw::Color color(const Fraction&) const;
    void setColor(const Fraction&, const mu::draw::Color& val);
    void undoSetColor(const mu::draw::Color& val);
    void insertTime(const Fraction&, const Fraction& len);

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    BracketType innerBracket() const;

    bool playbackVoice(int voice) const;
    void setPlaybackVoice(int voice, bool val);

    std::array<bool, VOICES> visibilityVoices() const;
    bool isVoiceVisible(voice_idx_t voice) const;
    bool canDisableVoice() const;

#ifndef NDEBUG
    void dumpClefs(const char* title) const;
    void dumpKeys(const char* title) const;
    void dumpTimeSigs(const char*) const;
#else
    void dumpClefs(const char*) const {}
    void dumpKeys(const char*) const {}
    void dumpTimeSigs(const char*) const {}
#endif

    void triggerLayout() const override;
    void triggerLayout(const Fraction& tick);
};
} // namespace mu::engraving
#endif

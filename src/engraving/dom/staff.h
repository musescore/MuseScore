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

#pragma once

#include <map>
#include <vector>

#include "engravingitem.h"

#include "draw/types/color.h"

#include "cleflist.h"
#include "groups.h"
#include "keylist.h"
#include "pitch.h"
#include "stafftypelist.h"

#include "../types/types.h"

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

enum class Key : signed char;

//---------------------------------------------------------
//    Staff
///    Global staff data not directly related to drawing.
//---------------------------------------------------------

class Staff final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Staff)

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

    Part* part() const { return m_part; }
    void setPart(Part* p) { m_part = p; }

    BracketType bracketType(size_t idx) const;
    size_t bracketSpan(size_t idx) const;
    void setBracketType(size_t idx, BracketType val);
    void setBracketSpan(size_t idx, size_t val);
    void setBracketVisible(size_t idx, bool v);
    void swapBracket(size_t oldIdx, size_t newIdx);
    void changeBracketColumn(size_t oldColumn, size_t newColumn);
    void addBracket(BracketItem*);
    const std::vector<BracketItem*>& brackets() const { return m_brackets; }
    std::vector<BracketItem*>& brackets() { return m_brackets; }
    void cleanupBrackets();
    size_t bracketLevels() const;

    ClefList& clefList() { return m_clefs; }
    ClefTypeList clefType(const Fraction&) const;
    ClefTypeList defaultClefType() const { return m_defaultClefType; }
    void setDefaultClefType(const ClefTypeList& l) { m_defaultClefType = l; }
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

    Interval transpose(const Fraction& tick) const;

    KeyList* keyList() { return &m_keys; }
    Key key(const Fraction& tick) const { return keySigEvent(tick).key(); }
    Key concertKey(const Fraction& tick) const { return keySigEvent(tick).concertKey(); }
    KeySigEvent keySigEvent(const Fraction&) const;
    Fraction nextKeyTick(const Fraction&) const;
    Fraction currentKeyTick(const Fraction&) const;
    KeySigEvent prevKey(const Fraction&) const;
    void setKey(const Fraction&, KeySigEvent);
    void removeKey(const Fraction&);

    bool show() const;
    bool stemless(const Fraction&) const;
    bool cutaway() const { return m_cutaway; }
    void setCutaway(bool val) { m_cutaway = val; }
    bool showIfEntireSystemEmpty() const { return m_showIfEntireSystemEmpty; }
    void setShowIfEntireSystemEmpty(bool val) { m_showIfEntireSystemEmpty = val; }

    bool hideSystemBarLine() const { return m_hideSystemBarLine; }
    void setHideSystemBarLine(bool val) { m_hideSystemBarLine = val; }
    AutoOnOff hideWhenEmpty() const { return m_hideWhenEmpty; }
    void setHideWhenEmpty(AutoOnOff v) { m_hideWhenEmpty = v; }
    AutoOnOff mergeMatchingRests() const { return m_mergeMatchingRests; }
    void setMergeMatchingRests(AutoOnOff val) { m_mergeMatchingRests = val; }
    bool shouldMergeMatchingRests() const;

    int barLineSpan() const { return m_barLineSpan; }
    int barLineFrom() const { return m_barLineFrom; }
    int barLineTo() const { return m_barLineTo; }
    void setBarLineSpan(int val) { m_barLineSpan = val; }
    void setBarLineFrom(int val) { m_barLineFrom = val; }
    void setBarLineTo(int val) { m_barLineTo = val; }
    double staffHeight() const;
    double staffHeight(const Fraction& tick) const;

    int channel(const Fraction&, voice_idx_t voice) const;

    void clearChannelList(voice_idx_t voice) { m_channelList[voice].clear(); }
    void insertIntoChannelList(voice_idx_t voice, const Fraction& tick, int channelId)
    {
        m_channelList[voice].insert({ tick.ticks(), channelId });
    }

    SwingParameters swing(const Fraction&)  const;
    void clearSwingList() { m_swingList.clear(); }
    void insertIntoSwingList(const Fraction& tick, SwingParameters sp) { m_swingList.insert({ tick.ticks(), sp }); }

    const CapoParams& capo(const Fraction&) const;
    void insertCapoParams(const Fraction& tick, const CapoParams& params);
    void clearCapoParams();

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

    std::pair<int, int> staffTypeRange(const Fraction&) const;

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

    PitchList& pitchOffsets() { return m_pitchOffsets; }

    int pitchOffset(const Fraction& tick) const { return m_pitchOffsets.pitchOffset(tick.ticks()); }
    void updateOttava();

    std::list<Staff*> staffList() const;
    Staff* primaryStaff() const;
    bool isPrimaryStaff() const;

    Spatium userDist() const { return m_userDist; }
    void setUserDist(Spatium val) { m_userDist = val; }

    void setLocalSpatium(double oldVal, double newVal, Fraction tick);
    bool genKeySig();
    bool showLedgerLines(const Fraction&) const;

    using EngravingItem::color;
    using EngravingItem::setColor;
    Color color(const Fraction&) const;
    void setColor(const Fraction&, const Color& val);
    void undoSetColor(const Color& val);
    void insertTime(const Fraction&, const Fraction& len);

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    BracketType innerBracket() const;

    bool playbackVoice(int voice) const;
    void setPlaybackVoice(int voice, bool val);

    const std::array<bool, VOICES>& visibilityVoices() const;
    bool isVoiceVisible(voice_idx_t voice) const;
    bool canDisableVoice() const;

    bool reflectTranspositionInLinkedTab() const;
    void setReflectTranspositionInLinkedTab(bool reflect);

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

    Staff* findLinkedInScore(const Score* score) const override;

    track_idx_t getLinkedTrackInStaff(const Staff* linkedStaff, const track_idx_t strack) const;
    bool trackHasLinksInVoiceZero(track_idx_t track);

    void undoSetShowMeasureNumbers(bool show);
    bool shouldShowMeasureNumbers() const;
    bool shouldShowPlayCount() const;

    bool isSystemObjectStaff() const;
    bool hasSystemObjectsBelowBottomStaff() const;

private:

    friend class Factory;
    Staff(Part* parent);
    Staff(const Staff& staff);

    void fillBrackets(size_t idx);
    void cleanBrackets();

    double staffMag(const StaffType*) const;

    friend class Excerpt;
    void setVoiceVisible(voice_idx_t voice, bool visible);
    void updateVisibilityVoices(const Staff* masterStaff, const TracksMap& tracks);

    ID m_id = INVALID_ID;
    Part* m_part = nullptr;

    ClefList m_clefs;
    ClefTypeList m_defaultClefType;

    KeyList m_keys;
    std::map<int, TimeSig*> m_timesigs;

    std::vector<BracketItem*> m_brackets;
    int m_barLineSpan = false;          // true - span barline to next staff
    int m_barLineFrom = 0;              // line of start staff to draw the barline from (0 = staff top line, ...)
    int m_barLineTo = 0;                // line of end staff to draw the bar line to (0= staff bottom line, ...)

    bool m_cutaway = false;
    bool m_showIfEntireSystemEmpty = false;             // show this staff if system is empty and hideEmptyStaves is true
    bool m_hideSystemBarLine = false;       // no system barline if not preceded by staff with barline
    AutoOnOff m_mergeMatchingRests = AutoOnOff::AUTO;      // merge matching rests in multiple voices
    AutoOnOff m_hideWhenEmpty = AutoOnOff::AUTO;      // hide empty staves

    Color m_color;
    Spatium m_userDist     { Spatium(0.0) };           ///< user edited extra distance

    StaffTypeList m_staffTypeList;

    std::map<int, int> m_channelList[VOICES];
    std::map<int, SwingParameters> m_swingList;
    std::map<int, CapoParams> m_capoMap;
    bool m_playbackVoice[VOICES] { true, true, true, true };
    std::array<bool, VOICES> m_visibilityVoices { true, true, true, true };

    PitchList m_pitchOffsets;               // cached value

    bool m_reflectTranspositionInLinkedTab = true;

    AutoOnOff m_showMeasureNumbers = AutoOnOff::AUTO;
};
}

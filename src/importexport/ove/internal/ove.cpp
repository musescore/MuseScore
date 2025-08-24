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

#include "ove.h"

#include <QTextCodec>
#include <QMap>

#include "log.h"

namespace ovebase {
/*
template <class T>
inline void deleteVector(QList<T*>& vec) {
    for (int i = 0; i < vec.size(); ++i) {
        delete vec[i];
    }
    vec.clear();
}
*/

TickElement::TickElement()
{
    m_tick = 0;
}

void TickElement::setTick(int tick)
{
    m_tick = tick;
}

int TickElement::getTick(void) const
{
    return m_tick;
}

MeasurePos::MeasurePos()
{
    m_measure = 0;
    m_offset = 0;
}

void MeasurePos::setMeasure(int measure)
{
    m_measure = measure;
}

int MeasurePos::getMeasure() const
{
    return m_measure;
}

void MeasurePos::setOffset(int offset)
{
    m_offset = offset;
}

int MeasurePos::getOffset() const
{
    return m_offset;
}

MeasurePos MeasurePos::shiftMeasure(int measure) const
{
    MeasurePos mp;
    mp.setMeasure(getMeasure() + measure);
    mp.setOffset(getOffset());

    return mp;
}

MeasurePos MeasurePos::shiftOffset(int offset) const
{
    MeasurePos mp;
    mp.setMeasure(getMeasure());
    mp.setOffset(getOffset() + offset);

    return mp;
}

bool MeasurePos::operator==(const MeasurePos& mp) const
{
    return getMeasure() == mp.getMeasure() && getOffset() == mp.getOffset();
}

bool MeasurePos::operator!=(const MeasurePos& mp) const
{
    return !(*this == mp);
}

bool MeasurePos::operator<(const MeasurePos& mp) const
{
    if (getMeasure() != mp.getMeasure()) {
        return getMeasure() < mp.getMeasure();
    }

    return getOffset() < mp.getOffset();
}

bool MeasurePos::operator<=(const MeasurePos& mp) const
{
    if (getMeasure() != mp.getMeasure()) {
        return getMeasure() <= mp.getMeasure();
    }

    return getOffset() <= mp.getOffset();
}

bool MeasurePos::operator>(const MeasurePos& mp) const
{
    return !(*this <= mp);
}

bool MeasurePos::operator>=(const MeasurePos& mp) const
{
    return !(*this < mp);
}

PairElement::PairElement()
{
    m_start = new MeasurePos();
    m_stop = new MeasurePos();
}

PairElement::~PairElement()
{
    delete m_start;
    delete m_stop;
}

MeasurePos* PairElement::start() const
{
    return m_start;
}

MeasurePos* PairElement::stop() const
{
    return m_stop;
}

PairEnds::PairEnds()
{
    m_leftLine = new LineElement();
    m_rightLine = new LineElement();
    m_leftShoulder = new OffsetElement();
    m_rightShoulder = new OffsetElement();
}

PairEnds::~PairEnds()
{
    delete m_leftLine;
    delete m_rightLine;
    delete m_leftShoulder;
    delete m_rightShoulder;
}

LineElement* PairEnds::getLeftLine() const
{
    return m_leftLine;
}

LineElement* PairEnds::getRightLine() const
{
    return m_rightLine;
}

OffsetElement* PairEnds::getLeftShoulder() const
{
    return m_leftShoulder;
}

OffsetElement* PairEnds::getRightShoulder() const
{
    return m_rightShoulder;
}

LineElement::LineElement()
{
    m_line = 0;
}

void LineElement::setLine(int line)
{
    m_line = line;
}

int LineElement::getLine(void) const
{
    return m_line;
}

OffsetElement::OffsetElement()
{
    m_xOffset = 0;
    m_yOffset = 0;
}

void OffsetElement::setXOffset(int offset)
{
    m_xOffset = offset;
}

int OffsetElement::getXOffset() const
{
    return m_xOffset;
}

void OffsetElement::setYOffset(int offset)
{
    m_yOffset = offset;
}

int OffsetElement::getYOffset() const
{
    return m_yOffset;
}

LengthElement::LengthElement()
{
    m_length = 0;
}

void LengthElement::setLength(int length)
{
    m_length = length;
}

int LengthElement::getLength() const
{
    return m_length;
}

MusicData::MusicData()
{
    m_musicDataType = MusicDataType::None;
    m_show = true;
    m_color = 0;
    m_voice = 0;
}

MusicDataType MusicData::getMusicDataType() const
{
    return m_musicDataType;
}

MusicData::XmlDataType MusicData::getXmlDataType(MusicDataType type)
{
    XmlDataType xmlType = XmlDataType::None;

    switch (type) {
    case MusicDataType::Measure_Repeat: {
        xmlType = XmlDataType::Attributes;
        break;
    }
    case MusicDataType::Beam: {
        xmlType = XmlDataType::NoteBeam;
        break;
    }
    case MusicDataType::Slur:
    case MusicDataType::Glissando:
    case MusicDataType::Tuplet:
    case MusicDataType::Tie: {
        xmlType = XmlDataType::Notations;
        break;
    }
    case MusicDataType::Text:
    case MusicDataType::Repeat:
    case MusicDataType::Wedge:
    case MusicDataType::Dynamics:
    case MusicDataType::Pedal:
    case MusicDataType::OctaveShift_EndPoint: {
        xmlType = XmlDataType::Direction;
        break;
    }
    default: {
        xmlType = XmlDataType::None;
        break;
    }
    }

    return xmlType;
}

/*
bool MusicData::get_is_pair_element(MusicDataType type)
{
    bool pair = false;

    switch (type) {
    case MusicDataType::Numeric_Ending:
    case MusicDataType::Measure_Repeat:
    case MusicDataType::Wedge:
    case MusicDataType::OctaveShift:
    // case MusicDataType::OctaveShift_EndPoint:
    case MusicDataType::Pedal:
    case MusicDataType::Beam:
    case MusicDataType::Glissando:
    case MusicDataType::Slur:
    case MusicDataType::Tie:
    case MusicDataType::Tuplet: {
        pair = true;
        break;
    }
    default:
        break;
    }

    return pair;
}
*/

void MusicData::setShow(bool show)
{
    m_show = show;
}

bool MusicData::getShow() const
{
    return m_show;
}

void MusicData::setColor(unsigned int color)
{
    m_color = color;
}

unsigned int MusicData::getColor() const
{
    return m_color;
}

void MusicData::setVoice(unsigned int voice)
{
    m_voice = voice;
}

unsigned int MusicData::getVoice() const
{
    return m_voice;
}

void MusicData::copyCommonBlock(const MusicData& source)
{
    setTick(source.getTick());
    start()->setOffset(source.start()->getOffset());
    setColor(source.getColor());
}

MidiData::MidiData()
{
    m_midiType = MidiType::None;
}

MidiType MidiData::getMidiType() const
{
    return m_midiType;
}

OveSong::OveSong()
    : m_codec(0)
{
    clear();
}

OveSong::~OveSong()
{
    clear();
}

void OveSong::setIsVersion4(bool version4)
{
    m_version4 = version4;
}

bool OveSong::getIsVersion4() const
{
    return m_version4;
}

void OveSong::setQuarter(int tick)
{
    m_quarter = tick;
}

int OveSong::getQuarter(void) const
{
    return m_quarter;
}

void OveSong::setShowPageMargin(bool show)
{
    m_showPageMargin = show;
}

bool OveSong::getShowPageMargin() const
{
    return m_showPageMargin;
}

void OveSong::setShowTransposeTrack(bool show)
{
    m_showTransposeTrack = show;
}

bool OveSong::getShowTransposeTrack() const
{
    return m_showTransposeTrack;
}

void OveSong::setShowLineBreak(bool show)
{
    m_showLineBreak = show;
}

bool OveSong::getShowLineBreak() const
{
    return m_showLineBreak;
}

void OveSong::setShowRuler(bool show)
{
    m_showRuler = show;
}

bool OveSong::getShowRuler() const
{
    return m_showRuler;
}

void OveSong::setShowColor(bool show)
{
    m_showColor = show;
}

bool OveSong::getShowColor() const
{
    return m_showColor;
}

void OveSong::setPlayRepeat(bool play)
{
    m_playRepeat = play;
}

bool OveSong::getPlayRepeat() const
{
    return m_playRepeat;
}

void OveSong::setPlayStyle(PlayStyle style)
{
    m_playStyle = style;
}

OveSong::PlayStyle OveSong::getPlayStyle() const
{
    return m_playStyle;
}

void OveSong::addTitle(const QString& str)
{
    m_titles.push_back(str);
}

QList<QString> OveSong::getTitles(void) const
{
    return m_titles;
}

void OveSong::addAnnotate(const QString& str)
{
    m_annotates.push_back(str);
}

QList<QString> OveSong::getAnnotates(void) const
{
    return m_annotates;
}

void OveSong::addWriter(const QString& str)
{
    m_writers.push_back(str);
}

QList<QString> OveSong::getWriters(void) const
{
    return m_writers;
}

void OveSong::addCopyright(const QString& str)
{
    m_copyrights.push_back(str);
}

QList<QString> OveSong::getCopyrights(void) const
{
    return m_copyrights;
}

void OveSong::addHeader(const QString& str)
{
    m_headers.push_back(str);
}

QList<QString> OveSong::getHeaders(void) const
{
    return m_headers;
}

void OveSong::addFooter(const QString& str)
{
    m_footers.push_back(str);
}

QList<QString> OveSong::getFooters(void) const
{
    return m_footers;
}

void OveSong::addTrack(Track* ptr)
{
    m_tracks.push_back(ptr);
}

int OveSong::getTrackCount(void) const
{
    return m_tracks.size();
}

QList<Track*> OveSong::getTracks() const
{
    return m_tracks;
}

void OveSong::setTrackBarCount(int count)
{
    m_trackBarCount = count;
}

int OveSong::getTrackBarCount() const
{
    return m_trackBarCount;
}

Track* OveSong::getTrack(int part, int staff) const
{
    int trackId = partStaffToTrack(part, staff);

    if (trackId >= 0 && trackId < (int)m_tracks.size()) {
        return m_tracks[trackId];
    }

    return 0;
}

bool OveSong::addPage(Page* page)
{
    m_pages.push_back(page);
    return true;
}

int OveSong::getPageCount() const
{
    return m_pages.size();
}

Page* OveSong::getPage(int idx)
{
    if (idx >= 0 && idx < (int)m_pages.size()) {
        return m_pages[idx];
    }

    return 0;
}

void OveSong::addLine(Line* ptr)
{
    m_lines.push_back(ptr);
}

int OveSong::getLineCount() const
{
    return m_lines.size();
}

Line* OveSong::getLine(int idx) const
{
    if (idx >= 0 && idx < (int)m_lines.size()) {
        return m_lines[idx];
    }

    return 0;
}

void OveSong::addMeasure(Measure* ptr)
{
    m_measures.push_back(ptr);
}

int OveSong::getMeasureCount(void) const
{
    return m_measures.size();
}

Measure* OveSong::getMeasure(int bar) const
{
    if (bar >= 0 && bar < (int)m_measures.size()) {
        return m_measures[bar];
    }

    return 0;
}

void OveSong::addMeasureData(MeasureData* ptr)
{
    m_measureDatas.push_back(ptr);
}

int OveSong::getMeasureDataCount(void) const
{
    return m_measureDatas.size();
}

MeasureData* OveSong::getMeasureData(int part, int staff /* = 0 */, int bar) const
{
    int trackId = partStaffToTrack(part, staff);
    int trackBarCount = getTrackBarCount();

    if (bar >= 0 && bar < trackBarCount) {
        int measureId = trackBarCount * trackId + bar;

        if (measureId >= 0 && measureId < (int)m_measureDatas.size()) {
            return m_measureDatas[measureId];
        }
    }

    return 0;
}

MeasureData* OveSong::getMeasureData(int track, int bar) const
{
    int id = m_trackBarCount * track + bar;

    if (id >= 0 && id < (int)m_measureDatas.size()) {
        return m_measureDatas[id];
    }

    return 0;
}

void OveSong::setPartStaffCounts(const QList<int>& partStaffCounts)
{
    // m_partStaffCounts.assign(partStaffCounts.begin(), partStaffCounts.end());
    for (int i = 0; i < partStaffCounts.size(); ++i) {
        m_partStaffCounts.push_back(partStaffCounts[i]);
    }
}

int OveSong::getPartCount() const
{
    return m_partStaffCounts.size();
}

int OveSong::getStaffCount(int part) const
{
    if (part >= 0 && part < (int)m_partStaffCounts.size()) {
        return m_partStaffCounts[part];
    }

    return 0;
}

int OveSong::getPartBarCount() const
{
    return m_measureDatas.size() / m_tracks.size();
}

QPair<int, int> OveSong::trackToPartStaff(int track) const
{
    int i;
    int staffCount = 0;

    for (i = 0; i < m_partStaffCounts.size(); ++i) {
        if (staffCount + m_partStaffCounts[i] > track) {
            return qMakePair((int)i, track - staffCount);
        }

        staffCount += m_partStaffCounts[i];
    }

    return qMakePair((int)m_partStaffCounts.size(), 0);
}

int OveSong::partStaffToTrack(int part, int staff) const
{
    int i;
    unsigned int staffCount = 0;

    for (i = 0; i < m_partStaffCounts.size(); ++i) {
        if (part == (int)i && staff >= 0 && staff < (int)m_partStaffCounts[i]) {
            int trackId = staffCount + staff;

            if (trackId >= 0 && trackId < (int)m_tracks.size()) {
                return trackId;
            }
        }

        staffCount += m_partStaffCounts[i];
    }

    return m_tracks.size();
}

void OveSong::setTextCodecName(const QString& codecName)
{
    m_codec = QTextCodec::codecForName(codecName.toLatin1());
}

QString OveSong::getCodecString(const QByteArray& text)
{
    QString s;
    if (m_codec == NULL) {
        s = QString(text);
    } else {
        s = m_codec->toUnicode(text);
    }

    s = s.trimmed();
    return s;
}

void OveSong::clear(void)
{
    m_version4 = true;
    m_quarter = 480;
    m_showPageMargin = false;
    m_showTransposeTrack = false;
    m_showLineBreak = false;
    m_showRuler = false;
    m_showColor = true;
    m_playRepeat = true;
    m_playStyle = PlayStyle::Record;

    m_annotates.clear();
    m_copyrights.clear();
    m_footers.clear();
    m_headers.clear();
    m_titles.clear();
    m_writers.clear();

    // deleteVector(m_tracks);
    for (int i = 0; i < m_tracks.size(); ++i) {
        delete m_tracks[i];
    }
    for (int i = 0; i < m_pages.size(); ++i) {
        delete m_pages[i];
    }
    for (int i = 0; i < m_lines.size(); ++i) {
        delete m_lines[i];
    }
    for (int i = 0; i < m_measures.size(); ++i) {
        delete m_measures[i];
    }
    for (int i = 0; i < m_measureDatas.size(); ++i) {
        delete m_measureDatas[i];
    }
    m_tracks.clear();
    m_pages.clear();
    m_lines.clear();
    m_measures.clear();
    m_measureDatas.clear();
    m_trackBarCount = 0;
    m_partStaffCounts.clear();
}

Voice::Voice()
{
    m_channel = 0;
    m_volume = -1;
    m_pitchShift = 0;
    m_pan = 0;
    m_patch = 0;
    m_stemType = 0;
}

void Voice::setChannel(int channel)
{
    m_channel = channel;
}

int Voice::getChannel() const
{
    return m_channel;
}

void Voice::setVolume(int volume)
{
    m_volume = volume;
}

int Voice::getVolume() const
{
    return m_volume;
}

void Voice::setPitchShift(int pitchShift)
{
    m_pitchShift = pitchShift;
}

int Voice::getPitchShift() const
{
    return m_pitchShift;
}

void Voice::setPan(int pan)
{
    m_pan = pan;
}

int Voice::getPan() const
{
    return m_pan;
}

void Voice::setPatch(int patch)
{
    m_patch = patch;
}

int Voice::getPatch() const
{
    return m_patch;
}

void Voice::setStemType(int stemType)
{
    m_stemType = stemType;
}

int Voice::getStemType() const
{
    return m_stemType;
}

int Voice::getDefaultPatch()
{
    return -1;
}

int Voice::getDefaultVolume()
{
    return -1;
}

Track::Track()
{
    clear();
}

Track::~Track()
{
    clear();
}

void Track::setName(const QString& str)
{
    m_name = str;
}

QString Track::getName(void) const
{
    return m_name;
}

void Track::setBriefName(const QString& str)
{
    m_briefName = str;
}

QString Track::getBriefName(void) const
{
    return m_briefName;
}

void Track::setPatch(unsigned int patch)
{
    m_patch = patch;
}

unsigned int Track::getPatch() const
{
    return m_patch;
}

void Track::setChannel(int channel)
{
    m_channel = channel;
}

int Track::getChannel() const
{
    return m_channel;
}

void Track::setShowName(bool show)
{
    m_showName = show;
}

bool Track::getShowName() const
{
    return m_showName;
}

void Track::setShowBriefName(bool show)
{
    m_showBriefName = show;
}

bool Track::getShowBriefName() const
{
    return m_showBriefName;
}

void Track::setMute(bool mute)
{
    m_mute = mute;
}

bool Track::getMute() const
{
    return m_mute;
}

void Track::setSolo(bool solo)
{
    m_solo = solo;
}

bool Track::getSolo() const
{
    return m_solo;
}

void Track::setShowKeyEachLine(bool show)
{
    m_showKeyEachLine = show;
}

bool Track::getShowKeyEachLine() const
{
    return m_showKeyEachLine;
}

void Track::setVoiceCount(int voices)
{
    m_voiceCount = voices;
}

int Track::getVoiceCount() const
{
    return m_voiceCount;
}

void Track::addVoice(Voice* voice)
{
    m_voices.push_back(voice);
}

QList<Voice*> Track::getVoices() const
{
    return m_voices;
}

void Track::setShowTranspose(bool show)
{
    m_showTranspose = show;
}

bool Track::getShowTranspose() const
{
    return m_showTranspose;
}

void Track::setTranspose(int transpose)
{
    m_transpose = transpose;
}

int Track::getTranspose() const
{
    return m_transpose;
}

void Track::setNoteShift(int shift)
{
    m_noteShift = shift;
}

int Track::getNoteShift() const
{
    return m_noteShift;
}

void Track::setStartClef(int clef /* in Clef */)
{
    m_startClef = ClefType(clef);
}

ClefType Track::getStartClef() const
{
    return m_startClef;
}

void Track::setTransposeClef(int clef)
{
    m_transposeClef = ClefType(clef);
}

ClefType Track::getTransposeClef() const
{
    return m_transposeClef;
}

void Track::setStartKey(int key)
{
    m_startKey = key;
}

int Track::getStartKey() const
{
    return m_startKey;
}

void Track::setDisplayPercent(unsigned int percent /* 25~100? */)
{
    m_displayPercent = percent;
}

unsigned int Track::getDisplayPercent() const
{
    return m_displayPercent;
}

void Track::setShowLegerLine(bool show)
{
    m_showLegerLine = show;
}

bool Track::getShowLegerLine() const
{
    return m_showLegerLine;
}

void Track::setShowClef(bool show)
{
    m_showClef = show;
}

bool Track::getShowClef() const
{
    return m_showClef;
}

void Track::setShowTimeSignature(bool show)
{
    m_showTimeSignature = show;
}

bool Track::getShowTimeSignature() const
{
    return m_showTimeSignature;
}

void Track::setShowKeySignature(bool show)
{
    m_showKeySignature = show;
}

bool Track::getShowKeySignature() const
{
    return m_showKeySignature;
}

void Track::setShowBarline(bool show)
{
    m_showBarline = show;
}

bool Track::getShowBarline() const
{
    return m_showBarline;
}

void Track::setFillWithRest(bool fill)
{
    m_fillWithRest = fill;
}

bool Track::getFillWithRest() const
{
    return m_fillWithRest;
}

void Track::setFlatTail(bool flat)
{
    m_flatTail = flat;
}

bool Track::getFlatTail() const
{
    return m_flatTail;
}

void Track::setShowClefEachLine(bool show)
{
    m_showClefEachLine = show;
}

bool Track::getShowClefEachLine() const
{
    return m_showClefEachLine;
}

void Track::addDrum(const DrumNode& node)
{
    /*
    DrumNode node;
    node.m_line = line;
    node.m_headType = headType;
    node.m_pitch = pitch;
    node.m_voice = voice;
    */
    m_drumKit.push_back(node);
}

QList<Track::DrumNode> Track::getDrumKit() const
{
    return m_drumKit;
}

void Track::setPart(int part)
{
    m_part = part;
}

int Track::getPart() const
{
    return m_part;
}

void Track::clear(void)
{
    m_number = 0;

    m_name = QString();

    m_patch = 0;
    m_channel = 0;
    m_transpose = 0;
    m_showTranspose = false;
    m_noteShift = 0;
    m_startClef = ClefType::Treble;
    m_transposeClef = ClefType::Treble;
    m_displayPercent = 100;
    m_startKey = 0;
    m_voiceCount = 8;

    m_showName = true;
    m_showBriefName = false;
    m_showKeyEachLine = false;
    m_showLegerLine = true;
    m_showClef = true;
    m_showTimeSignature = true;
    m_showKeySignature = true;
    m_showBarline = true;
    m_showClefEachLine = false;

    m_fillWithRest = true;
    m_flatTail = false;

    m_mute = false;
    m_solo = false;

    m_drumKit.clear();

    m_part = 0;

    for (int i = 0; i < m_voices.size(); ++i) {
        delete m_voices[i];
    }
    m_voices.clear();
}

Page::Page()
{
    m_beginLine = 0;
    m_lineCount = 0;

    m_lineInterval = 9;
    m_staffInterval = 7;
    m_staffInlineInterval = 6;

    m_lineBarCount = 4;
    m_pageLineCount = 5;

    m_leftMargin = 0xA8;
    m_topMargin = 0xA8;
    m_rightMargin = 0xA8;
    m_bottomMargin = 0xA8;

    m_pageWidth = 0x0B40;
    m_pageHeight = 0x0E90;
}

void Page::setBeginLine(int line)
{
    m_beginLine = line;
}

int Page::getBeginLine() const
{
    return m_beginLine;
}

void Page::setLineCount(int count)
{
    m_lineCount = count;
}

int Page::getLineCount() const
{
    return m_lineCount;
}

void Page::setLineInterval(int interval)
{
    m_lineInterval = interval;
}

int Page::getLineInterval() const
{
    return m_lineInterval;
}

void Page::setStaffInterval(int interval)
{
    m_staffInterval = interval;
}

int Page::getStaffInterval() const
{
    return m_staffInterval;
}

void Page::setStaffInlineInterval(int interval)
{
    m_staffInlineInterval = interval;
}

int Page::getStaffInlineInterval() const
{
    return m_staffInlineInterval;
}

void Page::setLineBarCount(int count)
{
    m_lineBarCount = count;
}

int Page::getLineBarCount() const
{
    return m_lineBarCount;
}

void Page::setPageLineCount(int count)
{
    m_pageLineCount = count;
}

int Page::getPageLineCount() const
{
    return m_pageLineCount;
}

void Page::setLeftMargin(int margin)
{
    m_leftMargin = margin;
}

int Page::getLeftMargin() const
{
    return m_leftMargin;
}

void Page::setTopMargin(int margin)
{
    m_topMargin = margin;
}

int Page::getTopMargin() const
{
    return m_topMargin;
}

void Page::setRightMargin(int margin)
{
    m_rightMargin = margin;
}

int Page::getRightMargin() const
{
    return m_rightMargin;
}

void Page::setBottomMargin(int margin)
{
    m_bottomMargin = margin;
}

int Page::getBottomMargin() const
{
    return m_bottomMargin;
}

void Page::setPageWidth(int width)
{
    m_pageWidth = width;
}

int Page::getPageWidth() const
{
    return m_pageWidth;
}

void Page::setPageHeight(int height)
{
    m_pageHeight = height;
}

int Page::getPageHeight() const
{
    return m_pageHeight;
}

Line::Line()
{
    m_beginBar = 0;
    m_barCount = 0;
    m_yOffset = 0;
    m_leftXOffset = 0;
    m_rightXOffset = 0;
}

Line::~Line()
{
    for (int i = 0; i < m_staves.size(); ++i) {
        delete m_staves[i];
    }
    m_staves.clear();
}

void Line::addStaff(Staff* staff)
{
    m_staves.push_back(staff);
}

int Line::getStaffCount() const
{
    return m_staves.size();
}

Staff* Line::getStaff(int idx) const
{
    if (idx >= 0 && idx < static_cast<int>(m_staves.size())) {
        return m_staves[idx];
    }

    return 0;
}

void Line::setBeginBar(unsigned int bar)
{
    m_beginBar = bar;
}

unsigned int Line::getBeginBar() const
{
    return m_beginBar;
}

void Line::setBarCount(unsigned int count)
{
    m_barCount = count;
}

unsigned int Line::getBarCount() const
{
    return m_barCount;
}

void Line::setYOffset(int offset)
{
    m_yOffset = offset;
}

int Line::getYOffset() const
{
    return m_yOffset;
}

void Line::setLeftXOffset(int offset)
{
    m_leftXOffset = offset;
}

int Line::getLeftXOffset() const
{
    return m_leftXOffset;
}

void Line::setRightXOffset(int offset)
{
    m_rightXOffset = offset;
}

int Line::getRightXOffset() const
{
    return m_rightXOffset;
}

Staff::Staff()
{
    m_clef = ClefType::Treble;
    m_key = 0;
    m_visible = true;
    m_groupType = GroupType::None;
    m_groupStaffCount = 0;
}

void Staff::setClefType(int clef)
{
    m_clef = (ClefType)clef;
}

ClefType Staff::getClefType() const
{
    return m_clef;
}

void Staff::setKeyType(int key)
{
    m_key = key;
}

int Staff::getKeyType() const
{
    return m_key;
}

void Staff::setVisible(bool visible)
{
    m_visible = visible;
}

bool Staff::setVisible() const
{
    return m_visible;
}

void Staff::setGroupType(GroupType type)
{
    m_groupType = type;
}

GroupType Staff::getGroupType() const
{
    return m_groupType;
}

void Staff::setGroupStaffCount(int count)
{
    m_groupStaffCount = count;
}

int Staff::getGroupStaffCount() const
{
    return m_groupStaffCount;
}

Note::Note()
{
    m_rest = false;
    m_note = 60;
    m_accidental = AccidentalType::Normal;
    m_showAccidental = false;
    m_offVelocity = 0x40;
    m_onVelocity = 0x50;
    m_headType = NoteHeadType::Standard;
    m_tiePos = TiePos::None;
    m_offsetStaff = 0;
    m_show = true;
    m_offsetTick = 0;
}

void Note::setIsRest(bool rest)
{
    m_rest = rest;
}

bool Note::getIsRest() const
{
    return m_rest;
}

void Note::setNote(unsigned int note)
{
    m_note = note;
}

unsigned int Note::getNote() const
{
    return m_note;
}

void Note::setAccidental(int type)
{
    m_accidental = (AccidentalType)type;
}

AccidentalType Note::getAccidental() const
{
    return m_accidental;
}

void Note::setShowAccidental(bool show)
{
    m_showAccidental = show;
}

bool Note::getShowAccidental() const
{
    return m_showAccidental;
}

void Note::setOnVelocity(unsigned int velocity)
{
    m_onVelocity = velocity;
}

unsigned int Note::getOnVelocity() const
{
    return m_onVelocity;
}

void Note::setOffVelocity(unsigned int velocity)
{
    m_offVelocity = velocity;
}

unsigned int Note::getOffVelocity() const
{
    return m_offVelocity;
}

void Note::setHeadType(int type)
{
    m_headType = (NoteHeadType)type;
}

NoteHeadType Note::getHeadType() const
{
    return m_headType;
}

void Note::setTiePos(int tiePos)
{
    m_tiePos = (TiePos)tiePos;
}

TiePos Note::getTiePos() const
{
    return m_tiePos;
}

void Note::setOffsetStaff(int offset)
{
    m_offsetStaff = offset;
}

int Note::getOffsetStaff() const
{
    return m_offsetStaff;
}

void Note::setShow(bool show)
{
    m_show = show;
}

bool Note::getShow() const
{
    return m_show;
}

void Note::setOffsetTick(int offset)
{
    m_offsetTick = offset;
}

int Note::getOffsetTick() const
{
    return m_offsetTick;
}

Articulation::Articulation()
{
    m_type = ArticulationType::Marcato;
    m_above = true;

    m_changeSoundEffect = false;
    m_changeLength = false;
    m_changeVelocity = false;
    m_changeExtraLength = false;

    m_soundEffect = qMakePair(0, 0);
    m_lengthPercentage = 100;
    m_velocityType = VelocityType::Offset;
    m_velocityValue = 0;
    m_extraLength = 0;

    m_trillNoteLength = 60;
    m_trillRate = NoteType::Note_Sixteen;
    m_accelerateType = AccelerateType::None;
    m_auxiliaryFirst = false;
    m_trillInterval = TrillInterval::Chromatic;
}

void Articulation::setArtType(int type)
{
    m_type = (ArticulationType)type;
}

ArticulationType Articulation::getArtType() const
{
    return m_type;
}

void Articulation::setPlacementAbove(bool above)
{
    m_above = above;
}

bool Articulation::getPlacementAbove() const
{
    return m_above;
}

bool Articulation::getChangeSoundEffect() const
{
    return m_changeSoundEffect;
}

void Articulation::setSoundEffect(int soundFrom, int soundTo)
{
    m_soundEffect = qMakePair(soundFrom, soundTo);
    m_changeSoundEffect = true;
}

QPair<int, int> Articulation::getSoundEffect() const
{
    return m_soundEffect;
}

bool Articulation::getChangeLength() const
{
    return m_changeLength;
}

void Articulation::setLengthPercentage(int percentage)
{
    m_lengthPercentage = percentage;
    m_changeLength = true;
}

int Articulation::getLengthPercentage() const
{
    return m_lengthPercentage;
}

bool Articulation::getChangeVelocity() const
{
    return m_changeVelocity;
}

void Articulation::setVelocityType(VelocityType type)
{
    m_velocityType = type;
    m_changeVelocity = true;
}

Articulation::VelocityType Articulation::getVelocityType() const
{
    return m_velocityType;
}

void Articulation::setVelocityValue(int value)
{
    m_velocityValue = value;
}

int Articulation::getVelocityValue() const
{
    return m_velocityValue;
}

bool Articulation::getChangeExtraLength() const
{
    return m_changeExtraLength;
}

void Articulation::setExtraLength(int length)
{
    m_extraLength = length;
    m_changeExtraLength = true;
}

int Articulation::getExtraLength() const
{
    return m_extraLength;
}

void Articulation::setTrillNoteLength(int length)
{
    m_trillNoteLength = length;
}

int Articulation::getTrillNoteLength() const
{
    return m_trillNoteLength;
}

void Articulation::setTrillRate(NoteType rate)
{
    m_trillRate = rate;
}

NoteType Articulation::getTrillRate() const
{
    return m_trillRate;
}

void Articulation::setAccelerateType(int type)
{
    m_accelerateType = (AccelerateType)type;
}

Articulation::AccelerateType Articulation::getAccelerateType() const
{
    return m_accelerateType;
}

void Articulation::setAuxiliaryFirst(bool first)
{
    m_auxiliaryFirst = first;
}

bool Articulation::getAuxiliaryFirst() const
{
    return m_auxiliaryFirst;
}

void Articulation::setTrillInterval(int interval)
{
    m_trillInterval = (TrillInterval)interval;
}

Articulation::TrillInterval Articulation::getTrillInterval() const
{
    return m_trillInterval;
}

bool Articulation::willAffectNotes() const
{
    bool affect = false;

    switch (getArtType()) {
    case ArticulationType::Major_Trill:
    case ArticulationType::Minor_Trill:
    case ArticulationType::Trill_Section:
    case ArticulationType::Inverted_Short_Mordent:
    case ArticulationType::Inverted_Long_Mordent:
    case ArticulationType::Short_Mordent:
    case ArticulationType::Turn:

    case ArticulationType::Arpeggio:
    case ArticulationType::Tremolo_Eighth:
    case ArticulationType::Tremolo_Sixteenth:
    case ArticulationType::Tremolo_Thirty_Second:
    case ArticulationType::Tremolo_Sixty_Fourth: {
        affect = true;
        break;
    }
    case ArticulationType::Finger_1:
    case ArticulationType::Finger_2:
    case ArticulationType::Finger_3:
    case ArticulationType::Finger_4:
    case ArticulationType::Finger_5:
    case ArticulationType::Flat_Accidental_For_Trill:
    case ArticulationType::Sharp_Accidental_For_Trill:
    case ArticulationType::Natural_Accidental_For_Trill:
    case ArticulationType::Marcato:
    case ArticulationType::Marcato_Dot:
    case ArticulationType::Heavy_Attack:
    case ArticulationType::SForzando:
    case ArticulationType::SForzando_Dot:
    case ArticulationType::Heavier_Attack:
    case ArticulationType::SForzando_Inverted:
    case ArticulationType::SForzando_Dot_Inverted:
    case ArticulationType::Staccatissimo:
    case ArticulationType::Staccato:
    case ArticulationType::Tenuto:
    case ArticulationType::Up_Bow:
    case ArticulationType::Down_Bow:
    case ArticulationType::Up_Bow_Inverted:
    case ArticulationType::Down_Bow_Inverted:
    case ArticulationType::Natural_Harmonic:
    case ArticulationType::Artificial_Harmonic:
    case ArticulationType::Plus_Sign:
    case ArticulationType::Fermata:
    case ArticulationType::Fermata_Inverted:
    case ArticulationType::Pedal_Down:
    case ArticulationType::Pedal_Up:
    case ArticulationType::Pause:
    case ArticulationType::Grand_Pause:
    case ArticulationType::Toe_Pedal:
    case ArticulationType::Heel_Pedal:
    case ArticulationType::Toe_To_Heel_Pedal:
    case ArticulationType::Heel_To_Toe_Pedal:
    case ArticulationType::Open_String:
    case ArticulationType::Guitar_Lift:
    case ArticulationType::Guitar_Slide_Up:
    case ArticulationType::Guitar_Rip:
    case ArticulationType::Guitar_Fall_Off:
    case ArticulationType::Guitar_Slide_Down:
    case ArticulationType::Guitar_Spill:
    case ArticulationType::Guitar_Flip:
    case ArticulationType::Guitar_Smear:
    case ArticulationType::Guitar_Bend:
    case ArticulationType::Guitar_Doit:
    case ArticulationType::Guitar_Plop:
    case ArticulationType::Guitar_Wow_Wow:
    case ArticulationType::Guitar_Thumb:
    case ArticulationType::Guitar_Index_Finger:
    case ArticulationType::Guitar_Middle_Finger:
    case ArticulationType::Guitar_Ring_Finger:
    case ArticulationType::Guitar_Pinky_Finger:
    case ArticulationType::Guitar_Tap:
    case ArticulationType::Guitar_Hammer:
    case ArticulationType::Guitar_Pluck: {
        break;
    }
    default:
        break;
    }

    return affect;
}

bool Articulation::isTrill(ArticulationType type)
{
    bool isTrill = false;

    switch (type) {
    case ArticulationType::Major_Trill:
    case ArticulationType::Minor_Trill:
    case ArticulationType::Trill_Section: {
        isTrill = true;
        break;
    }
    default:
        break;
    }

    return isTrill;
}

Articulation::XmlType Articulation::getXmlType() const
{
    XmlType xmlType = XmlType::Unknown;

    switch (m_type) {
    case ArticulationType::Major_Trill:
    case ArticulationType::Minor_Trill:
    case ArticulationType::Trill_Section:
    case ArticulationType::Inverted_Short_Mordent:
    case ArticulationType::Inverted_Long_Mordent:
    case ArticulationType::Short_Mordent:
    case ArticulationType::Turn:
    // case ArticulationType::Flat_Accidental_For_Trill:
    // case ArticulationType::Sharp_Accidental_For_Trill:
    // case ArticulationType::Natural_Accidental_For_Trill:
    case ArticulationType::Tremolo_Eighth:
    case ArticulationType::Tremolo_Sixteenth:
    case ArticulationType::Tremolo_Thirty_Second:
    case ArticulationType::Tremolo_Sixty_Fourth: {
        xmlType = XmlType::Ornament;
        break;
    }
    case ArticulationType::Marcato:
    case ArticulationType::Marcato_Dot:
    case ArticulationType::Heavy_Attack:
    case ArticulationType::SForzando:
    case ArticulationType::SForzando_Inverted:
    case ArticulationType::SForzando_Dot:
    case ArticulationType::SForzando_Dot_Inverted:
    case ArticulationType::Heavier_Attack:
    case ArticulationType::Staccatissimo:
    case ArticulationType::Staccato:
    case ArticulationType::Tenuto:
    case ArticulationType::Pause:
    case ArticulationType::Grand_Pause: {
        xmlType = XmlType::Articulation;
        break;
    }
    case ArticulationType::Up_Bow:
    case ArticulationType::Down_Bow:
    case ArticulationType::Up_Bow_Inverted:
    case ArticulationType::Down_Bow_Inverted:
    case ArticulationType::Natural_Harmonic:
    case ArticulationType::Artificial_Harmonic:
    case ArticulationType::Finger_1:
    case ArticulationType::Finger_2:
    case ArticulationType::Finger_3:
    case ArticulationType::Finger_4:
    case ArticulationType::Finger_5:
    case ArticulationType::Plus_Sign: {
        xmlType = XmlType::Technical;
        break;
    }
    case ArticulationType::Arpeggio: {
        xmlType = XmlType::Arpeggiate;
        break;
    }
    case ArticulationType::Fermata:
    case ArticulationType::Fermata_Inverted: {
        xmlType = XmlType::Fermata;
        break;
    }
    case ArticulationType::Pedal_Down:
    case ArticulationType::Pedal_Up: {
        xmlType = XmlType::Direction;
        break;
    }
    // case ArticulationType::Toe_Pedal:
    // case ArticulationType::Heel_Pedal:
    // case ArticulationType::Toe_To_Heel_Pedal:
    // case ArticulationType::Heel_To_Toe_Pedal:
    // case ArticulationType::Open_String:
    default:
        break;
    }

    return xmlType;
}

NoteContainer::NoteContainer()
{
    m_musicDataType = MusicDataType::Note_Container;

    m_grace = false;
    m_cue = false;
    m_rest = false;
    m_raw = false;
    m_noteType = NoteType::Note_Quarter;
    m_dot = 0;
    m_graceNoteType = NoteType::Note_Eight;
    m_stemUp = true;
    m_showStem = true;
    m_stemLength = 7;
    m_inBeam = false;
    m_tuplet = 0;
    m_space = 2;  //div by 0
    m_noteShift = 0;
}

NoteContainer::~NoteContainer()
{
    for (int i = 0; i < m_notes.size(); ++i) {
        delete m_notes[i];
    }
    for (int i = 0; i < m_articulations.size(); ++i) {
        delete m_articulations[i];
    }
    m_notes.clear();
    m_articulations.clear();
}

void NoteContainer::setIsGrace(bool grace)
{
    m_grace = grace;
}

bool NoteContainer::getIsGrace() const
{
    return m_grace;
}

void NoteContainer::setIsCue(bool cue)
{
    m_cue = cue;
}

bool NoteContainer::getIsCue() const
{
    return m_cue;
}

void NoteContainer::setIsRest(bool rest)
{
    m_rest = rest;
}

bool NoteContainer::getIsRest() const
{
    return m_rest;
}

void NoteContainer::setIsRaw(bool raw)
{
    m_raw = raw;
}

bool NoteContainer::getIsRaw() const
{
    return m_raw;
}

void NoteContainer::setNoteType(NoteType type)
{
    m_noteType = NoteType::Note_Quarter;

    switch (type) {
    case NoteType::Note_DoubleWhole:
    case NoteType::Note_Whole:
    case NoteType::Note_Half:
    case NoteType::Note_Quarter:
    case NoteType::Note_Eight:
    case NoteType::Note_Sixteen:
    case NoteType::Note_32:
    case NoteType::Note_64:
    case NoteType::Note_128:
    case NoteType::Note_256: {
//  case NoteType::Note_512:
//  case NoteType::Note_1024: {
        m_noteType = type;
        break;
    }
    default: {
        break;
    }
    }
}

NoteType NoteContainer::getNoteType() const
{
    return m_noteType;
}

void NoteContainer::setDot(int dot)
{
    m_dot = dot;
}

int NoteContainer::getDot() const
{
    return m_dot;
}

void NoteContainer::setGraceNoteType(NoteType type)
{
    m_graceNoteType = type;
}

NoteType NoteContainer::getGraceNoteType() const
{
    return m_graceNoteType;
}

void NoteContainer::setInBeam(bool in)
{
    m_inBeam = in;
}

bool NoteContainer::getInBeam() const
{
    return m_inBeam;
}

void NoteContainer::setStemUp(bool up)
{
    m_stemUp = up;
}

bool NoteContainer::getStemUp(void) const
{
    return m_stemUp;
}

void NoteContainer::setShowStem(bool show)
{
    m_showStem = show;
}

bool NoteContainer::getShowStem() const
{
    return m_showStem;
}

void NoteContainer::setStemLength(int line)
{
    m_stemLength = line;
}

int NoteContainer::getStemLength() const
{
    return m_stemLength;
}

void NoteContainer::setTuplet(int tuplet)
{
    m_tuplet = tuplet;
}

int NoteContainer::getTuplet() const
{
    return m_tuplet;
}

void NoteContainer::setSpace(int space)
{
    m_space = space;
}

int NoteContainer::getSpace() const
{
    return m_space;
}

void NoteContainer::addNoteRest(Note* note)
{
    m_notes.push_back(note);
}

QList<Note*> NoteContainer::getNotesRests() const
{
    return m_notes;
}

void NoteContainer::addArticulation(Articulation* art)
{
    m_articulations.push_back(art);
}

QList<Articulation*> NoteContainer::getArticulations() const
{
    return m_articulations;
}

void NoteContainer::setNoteShift(int octave)
{
    m_noteShift = octave;
}

int NoteContainer::getNoteShift() const
{
    return m_noteShift;
}

int NoteContainer::getOffsetStaff() const
{
    if (getIsRest()) {
        return 0;
    }

    int staffMove = 0;
    QList<Note*> notes = getNotesRests();
    for (int i = 0; i < notes.size(); ++i) {
        Note* notePtr = notes[i];
        staffMove = notePtr->getOffsetStaff();
    }

    return staffMove;
}

int NoteContainer::getDuration() const
{
    int duration = static_cast<int>(NoteDuration::D_4);

    switch (m_noteType) {
    case NoteType::Note_DoubleWhole: {
        duration = static_cast<int>(NoteDuration::D_Double_Whole);
        break;
    }
    case NoteType::Note_Whole: {
        duration = static_cast<int>(NoteDuration::D_Whole);
        break;
    }
    case NoteType::Note_Half: {
        duration = static_cast<int>(NoteDuration::D_2);
        break;
    }
    case NoteType::Note_Quarter: {
        duration = static_cast<int>(NoteDuration::D_4);
        break;
    }
    case NoteType::Note_Eight: {
        duration = static_cast<int>(NoteDuration::D_8);
        break;
    }
    case NoteType::Note_Sixteen: {
        duration = static_cast<int>(NoteDuration::D_16);
        break;
    }
    case NoteType::Note_32: {
        duration = static_cast<int>(NoteDuration::D_32);
        break;
    }
    case NoteType::Note_64: {
        duration = static_cast<int>(NoteDuration::D_64);
        break;
    }
    case NoteType::Note_128: {
        duration = static_cast<int>(NoteDuration::D_128);
        break;
    }
    case NoteType::Note_256: {
        duration = static_cast<int>(NoteDuration::D_256);
        break;
    }
//  case NoteType::Note_512: {
//      duration = static_cast<int>(NoteDuration::D_512);
//      break;
//  }
//  case NoteType::Note_1024: {
//      duration = static_cast<int>(NoteDuration::D_1024);
//      break;
//  }
    default:
        break;
    }

    int dotLength = duration;

    for (int i = 0; i < m_dot; ++i) {
        dotLength /= 2;
    }

    dotLength = duration - dotLength;

    duration += dotLength;

    return duration;
}

Beam::Beam()
{
    m_musicDataType = MusicDataType::Beam;
    m_grace = false;
}

void Beam::setIsGrace(bool grace)
{
    m_grace = grace;
}

bool Beam::getIsGrace() const
{
    return m_grace;
}

void Beam::addLine(const MeasurePos& startMp, const MeasurePos& endMp)
{
    m_lines.push_back(qMakePair(startMp, endMp));
}

const QList<QPair<MeasurePos, MeasurePos> > Beam::getLines() const
{
    return m_lines;
}

Tie::Tie()
{
    m_musicDataType = MusicDataType::Tie;

    m_showOnTop = true;
    m_note = 72;
    m_height = 24;
}

void Tie::setShowOnTop(bool top)
{
    m_showOnTop = top;
}

bool Tie::getShowOnTop() const
{
    return m_showOnTop;
}

void Tie::setNote(int note)
{
    m_note = note;
}

int Tie::getNote() const
{
    return m_note;
}

void Tie::setHeight(int height)
{
    m_height = height;
}

int Tie::getHeight() const
{
    return m_height;
}

Glissando::Glissando()
{
    m_musicDataType = MusicDataType::Glissando;

    m_straight = true;
    m_text = "gliss.";
    m_lineThick = 8;
}

void Glissando::setStraightWavy(bool straight)
{
    m_straight = straight;
}

bool Glissando::getStraightWavy() const
{
    return m_straight;
}

void Glissando::setText(const QString& text)
{
    m_text = text;
}

QString Glissando::getText() const
{
    return m_text;
}

void Glissando::setLineThick(int thick)
{
    m_lineThick = thick;
}

int Glissando::getLineThick() const
{
    return m_lineThick;
}

Decorator::Decorator()
    : m_decoratorType(Type::Articulation),
    m_artType(ArticulationType::Marcato)
{
    m_musicDataType = MusicDataType::Decorator;
}

void Decorator::setDecoratorType(Type type)
{
    m_decoratorType = type;
}

Decorator::Type Decorator::getDecoratorType() const
{
    return m_decoratorType;
}

void Decorator::setArticulationType(ArticulationType type)
{
    m_artType = type;
}

ArticulationType Decorator::getArticulationType() const
{
    return m_artType;
}

MeasureRepeat::MeasureRepeat()
{
    m_musicDataType = MusicDataType::Measure_Repeat;
    m_singleRepeat = true;
}

void MeasureRepeat::setSingleRepeat(bool single)
{
    m_singleRepeat = single;

    start()->setMeasure(0);
    start()->setOffset(0);
    stop()->setMeasure(single ? 1 : 2);
    stop()->setOffset(0);
}

bool MeasureRepeat::getSingleRepeat() const
{
    return m_singleRepeat;
}

Tuplet::Tuplet()
    : m_tuplet(3), m_space(2), m_height(0), m_noteType(NoteType::Note_Quarter)
{
    m_musicDataType = MusicDataType::Tuplet;
    m_mark = new OffsetElement();
}

Tuplet::~Tuplet()
{
    delete m_mark;
}

void Tuplet::setTuplet(int tuplet)
{
    m_tuplet = tuplet;
}

int Tuplet::getTuplet() const
{
    return m_tuplet;
}

void Tuplet::setSpace(int space)
{
    m_space = space;
}

int Tuplet::getSpace() const
{
    return m_space;
}

OffsetElement* Tuplet::getMarkHandle() const
{
    return m_mark;
}

void Tuplet::setHeight(int height)
{
    m_height = height;
}

int Tuplet::getHeight() const
{
    return m_height;
}

void Tuplet::setNoteType(NoteType type)
{
    m_noteType = type;
}

NoteType Tuplet::getNoteType() const
{
    return m_noteType;
}

Harmony::Harmony()
{
    m_musicDataType = MusicDataType::Harmony;

    m_harmonyType = "";
    m_root = 0;
    m_bass = -1; // 0xff
    m_alterRoot = 0;
    m_alterBass = 0;
    m_bassOnBottom = false;
    m_angle = 0;
}

void Harmony::setHarmonyType(QString type)
{
    m_harmonyType = type;
}

QString Harmony::getHarmonyType() const
{
    return m_harmonyType;
}

void Harmony::setRoot(int root)
{
    m_root = root;
}

int Harmony::getRoot() const
{
    return m_root;
}

void Harmony::setAlterRoot(int val)
{
    m_alterRoot = val;
}

int Harmony::getAlterRoot() const
{
    return m_alterRoot;
}

void Harmony::setBass(int bass)
{
    m_bass = bass;
}

int Harmony::getBass() const
{
    return m_bass;
}

void Harmony::setAlterBass(int val)
{
    m_alterBass = val;
}

int Harmony::getAlterBass() const
{
    return m_alterBass;
}

void Harmony::setBassOnBottom(bool on)
{
    m_bassOnBottom = on;
}

bool Harmony::getBassOnBottom() const
{
    return m_bassOnBottom;
}

void Harmony::setAngle(int angle)
{
    m_angle = angle;
}

int Harmony::getAngle() const
{
    return m_angle;
}

Clef::Clef()
{
    m_musicDataType = MusicDataType::Clef;

    m_clefType = ClefType::Treble;
}

void Clef::setClefType(int type)
{
    m_clefType = (ClefType)type;
}

ClefType Clef::getClefType() const
{
    return m_clefType;
}

Lyric::Lyric()
{
    m_musicDataType = MusicDataType::Lyric;

    m_lyric = QString();
    m_verse = 0;
}

void Lyric::setLyric(const QString& lyricText)
{
    m_lyric = lyricText;
}

QString Lyric::getLyric() const
{
    return m_lyric;
}

void Lyric::setVerse(int verse)
{
    m_verse = verse;
}

int Lyric::getVerse() const
{
    return m_verse;
}

Slur::Slur()
{
    m_musicDataType = MusicDataType::Slur;

    m_containerCount = 1;
    m_showOnTop = true;
    m_noteTimePercent = 100;

    m_handle_2 = new OffsetElement();
    m_handle_3 = new OffsetElement();
}

Slur::~Slur()
{
    delete m_handle_2;
    delete m_handle_3;
}

void Slur::setContainerCount(int count)
{
    m_containerCount = count;
}

int Slur::getContainerCount() const
{
    return m_containerCount;
}

void Slur::setShowOnTop(bool top)
{
    m_showOnTop = top;
}

bool Slur::getShowOnTop() const
{
    return m_showOnTop;
}

OffsetElement* Slur::getHandle2() const
{
    return m_handle_2;
}

OffsetElement* Slur::getHandle3() const
{
    return m_handle_3;
}

void Slur::setNoteTimePercent(int percent)
{
    m_noteTimePercent = percent;
}

int Slur::getNoteTimePercent() const
{
    return m_noteTimePercent;
}

Dynamics::Dynamics()
{
    m_musicDataType = MusicDataType::Dynamics;

    m_dynamicsType = DynamicsType::PPPP;
    m_playback = true;
    m_velocity = 30;
}

void Dynamics::setDynamicsType(int type)
{
    m_dynamicsType = DynamicsType(type);
}

DynamicsType Dynamics::getDynamicsType() const
{
    return m_dynamicsType;
}

void Dynamics::setIsPlayback(bool play)
{
    m_playback = play;
}

bool Dynamics::getIsPlayback() const
{
    return m_playback;
}

void Dynamics::setVelocity(int vel)
{
    m_velocity = vel;
}

int Dynamics::getVelocity() const
{
    return m_velocity;
}

WedgeEndPoint::WedgeEndPoint()
{
    m_musicDataType = MusicDataType::Wedge_EndPoint;

    m_wedgeType = WedgeType::Cresc;
    m_height = 24;
    m_wedgeStart = true;
}

void WedgeEndPoint::setWedgeType(WedgeType type)
{
    m_wedgeType = type;
}

WedgeType WedgeEndPoint::getWedgeType() const
{
    return m_wedgeType;
}

void WedgeEndPoint::setHeight(int height)
{
    m_height = height;
}

int WedgeEndPoint::getHeight() const
{
    return m_height;
}

void WedgeEndPoint::setWedgeStart(bool wedgeStart)
{
    m_wedgeStart = wedgeStart;
}

bool WedgeEndPoint::getWedgeStart() const
{
    return m_wedgeStart;
}

Wedge::Wedge()
{
    m_musicDataType = MusicDataType::Wedge;

    m_wedgeType = WedgeType::Cresc;
    m_height = 24;
}

void Wedge::setWedgeType(WedgeType type)
{
    m_wedgeType = type;
}

WedgeType Wedge::getWedgeType() const
{
    return m_wedgeType;
}

void Wedge::setHeight(int height)
{
    m_height = height;
}

int Wedge::getHeight() const
{
    return m_height;
}

Pedal::Pedal()
{
    m_musicDataType = MusicDataType::Pedal;

    m_half = false;
    m_playback = false;
    m_playOffset = 0;

    m_pedalHandle = new OffsetElement();
}

Pedal::~Pedal()
{
    delete m_pedalHandle;
}

void Pedal::setHalf(bool half)
{
    m_half = half;
}

bool Pedal::getHalf() const
{
    return m_half;
}

OffsetElement* Pedal::getPedalHandle() const
{
    return m_pedalHandle;
}

void Pedal::setIsPlayback(bool playback)
{
    m_playback = playback;
}

bool Pedal::getIsPlayback() const
{
    return m_playback;
}

void Pedal::setPlayOffset(int offset)
{
    m_playOffset = offset;
}

int Pedal::getPlayOffset() const
{
    return m_playOffset;
}

KuoHao::KuoHao()
{
    m_musicDataType = MusicDataType::KuoHao;

    m_kuohaoType = KuoHaoType::Parentheses;
    m_height = 0;
}

void KuoHao::setHeight(int height)
{
    m_height = height;
}

int KuoHao::getHeight() const
{
    return m_height;
}

void KuoHao::setKuohaoType(int type)
{
    m_kuohaoType = (KuoHaoType)type;
}

KuoHaoType KuoHao::getKuohaoType() const
{
    return m_kuohaoType;
}

Expressions::Expressions()
{
    m_musicDataType = MusicDataType::Expressions;

    m_text = QString();
}

void Expressions::setText(const QString& str)
{
    m_text = str;
}

QString Expressions::getText() const
{
    return m_text;
}

HarpPedal::HarpPedal()
    : m_showType(0), m_showCharFlag(0)
{
    m_musicDataType = MusicDataType::Harp_Pedal;
}

void HarpPedal::setShowType(int type)
{
    m_showType = type;
}

int HarpPedal::getShowType() const
{
    return m_showType;
}

void HarpPedal::setShowCharFlag(int flag)
{
    m_showCharFlag = flag;
}

int HarpPedal::getShowCharFlag() const
{
    return m_showCharFlag;
}

OctaveShift::OctaveShift()
    : m_octaveShiftType(OctaveShiftType::OS_8),
    m_octaveShiftPosition(OctaveShiftPosition::Start),
    m_endTick(0)
{
    m_musicDataType = MusicDataType::OctaveShift;
}

void OctaveShift::setOctaveShiftType(OctaveShiftType type)
{
    m_octaveShiftType = type;
}

OctaveShiftType OctaveShift::getOctaveShiftType() const
{
    return m_octaveShiftType;
}

int OctaveShift::getNoteShift() const
{
    int shift = 12;

    switch (getOctaveShiftType()) {
    case OctaveShiftType::OS_8: {
        shift = 12;
        break;
    }
    case OctaveShiftType::OS_Minus_8: {
        shift = -12;
        break;
    }
    case OctaveShiftType::OS_15: {
        shift = 24;
        break;
    }
    case OctaveShiftType::OS_Minus_15: {
        shift = -24;
        break;
    }
    default:
        break;
    }

    return shift;
}

void OctaveShift::setEndTick(int tick)
{
    m_endTick = tick;
}

int OctaveShift::getEndTick() const
{
    return m_endTick;
}

void OctaveShift::setOctaveShiftPosition(OctaveShiftPosition position)
{
    m_octaveShiftPosition = position;
}

OctaveShiftPosition OctaveShift::getOctaveShiftPosition() const
{
    return m_octaveShiftPosition;
}

OctaveShiftEndPoint::OctaveShiftEndPoint()
{
    m_musicDataType = MusicDataType::OctaveShift_EndPoint;

    m_octaveShiftType = OctaveShiftType::OS_8;
    m_octaveShiftPosition = OctaveShiftPosition::Start;
    m_endTick = 0;
}

void OctaveShiftEndPoint::setOctaveShiftType(OctaveShiftType type)
{
    m_octaveShiftType = type;
}

OctaveShiftType OctaveShiftEndPoint::getOctaveShiftType() const
{
    return m_octaveShiftType;
}

void OctaveShiftEndPoint::setOctaveShiftPosition(OctaveShiftPosition position)
{
    m_octaveShiftPosition = position;
}

OctaveShiftPosition OctaveShiftEndPoint::getOctaveShiftPosition() const
{
    return m_octaveShiftPosition;
}

void OctaveShiftEndPoint::setEndTick(int tick)
{
    m_endTick = tick;
}

int OctaveShiftEndPoint::getEndTick() const
{
    return m_endTick;
}

MultiMeasureRest::MultiMeasureRest()
{
    m_musicDataType = MusicDataType::Multi_Measure_Rest;
    m_measureCount = 0;
}

void MultiMeasureRest::setMeasureCount(int count)
{
    m_measureCount = count;
}

int MultiMeasureRest::getMeasureCount() const
{
    return m_measureCount;
}

Tempo::Tempo()
{
    m_musicDataType = MusicDataType::Tempo;

    m_leftNoteType = 3;
    m_showMark = false;
    m_showText = false;
    m_showParenthesis = false;
    m_typeTempo = 96;
    m_leftText = QString();
    m_rightText = QString();
    m_swingEighth = false;
    m_rightNoteType = 3;
    m_leftNoteDot = false;
    m_rightNoteDot = false;
    m_rightSideType = 0;
}

void Tempo::setLeftNoteType(int type)
{
    m_leftNoteType = type;
}

NoteType Tempo::getLeftNoteType() const
{
    return (NoteType)m_leftNoteType;
}

void Tempo::setShowMark(bool show)
{
    m_showMark = show;
}

bool Tempo::getShowMark() const
{
    return m_showMark;
}

void Tempo::setShowBeforeText(bool show)
{
    m_showText = show;
}

bool Tempo::getShowBeforeText() const
{
    return m_showText;
}

void Tempo::setShowParenthesis(bool show)
{
    m_showParenthesis = show;
}

bool Tempo::getShowParenthesis() const
{
    return m_showParenthesis;
}

void Tempo::setTypeTempo(double tempo)
{
    m_typeTempo = tempo;
}

double Tempo::getTypeTempo() const
{
    return m_typeTempo;
}

double Tempo::getQuarterTempo() const
{
    double factor = pow(2.0, int(NoteType::Note_Quarter) - int(getLeftNoteType()));
    if (getLeftNoteDot()) {
        factor *= 3.0 / 2.0;
    }
    double tempo = getTypeTempo() * factor;

    return tempo;
}

void Tempo::setLeftText(const QString& str)
{
    m_leftText = str;
}

QString Tempo::getLeftText() const
{
    return m_leftText;
}

void Tempo::setRightText(const QString& str)
{
    m_rightText = str;
}

QString Tempo::getRightText() const
{
    return m_rightText;
}

void Tempo::setSwingEighth(bool swing)
{
    m_swingEighth = swing;
}

bool Tempo::getSwingEighth() const
{
    return m_swingEighth;
}

void Tempo::setRightNoteType(int type)
{
    m_rightNoteType = type;
}

NoteType Tempo::getRightNoteType() const
{
    return (NoteType)m_rightNoteType;
}

void Tempo::setLeftNoteDot(bool showDot)
{
    m_leftNoteDot = showDot;
}

bool Tempo::getLeftNoteDot() const
{
    return m_leftNoteDot;
}

void Tempo::setRightNoteDot(bool showDot)
{
    m_rightNoteDot = showDot;
}

bool Tempo::getRightNoteDot() const
{
    return m_rightNoteDot;
}

void Tempo::setRightSideType(int type)
{
    m_rightSideType = type;
}

int Tempo::getRightSideType() const
{
    return m_rightSideType;
}

Text::Text()
{
    m_musicDataType = MusicDataType::Text;

    m_textType = Type::Rehearsal;
    m_horiMargin = 8;
    m_vertMargin = 8;
    m_lineThick = 4;
    m_text = QString();
    m_width = 0;
    m_height = 0;
}

void Text::setTextType(Type type)
{
    m_textType = type;
}

Text::Type Text::getTextType() const
{
    return m_textType;
}

void Text::setHorizontalMargin(int margin)
{
    m_horiMargin = margin;
}

int Text::getHorizontalMargin() const
{
    return m_horiMargin;
}

void Text::setVerticalMargin(int margin)
{
    m_vertMargin = margin;
}

int Text::getVerticalMargin() const
{
    return m_vertMargin;
}

void Text::setLineThick(int thick)
{
    m_lineThick = thick;
}

int Text::getLineThick() const
{
    return m_lineThick;
}

void Text::setText(const QString& text)
{
    m_text = text;
}

QString Text::getText() const
{
    return m_text;
}

void Text::setWidth(int width)
{
    m_width = width;
}

int Text::getWidth() const
{
    return m_width;
}

void Text::setHeight(int height)
{
    m_height = height;
}

int Text::getHeight() const
{
    return m_height;
}

TimeSignature::TimeSignature()
{
    m_numerator = 4;
    m_denominator = 4;
    m_isSymbol = false;
    m_beatLength = 480;
    m_barLength = 1920;
    m_barLengthUnits = 0x400;
    m_replaceFont = false;
    m_showBeatGroup = false;

    m_groupNumerator1 = 0;
    m_groupNumerator2 = 0;
    m_groupNumerator3 = 0;
    m_groupDenominator1 = 4;
    m_groupDenominator2 = 4;
    m_groupDenominator3 = 4;

    m_beamGroup1 = 4;
    m_beamGroup2 = 0;
    m_beamGroup3 = 0;
    m_beamGroup4 = 0;

    m_beamCount16th = 4;
    m_beamCount32nd = 1;
}

void TimeSignature::setNumerator(int numerator)
{
    m_numerator = numerator;
}

int TimeSignature::getNumerator() const
{
    return m_numerator;
}

void TimeSignature::setDenominator(int denominator)
{
    m_denominator = denominator;
}

int TimeSignature::getDenominator() const
{
    return m_denominator;
}

void TimeSignature::setIsSymbol(bool symbol)
{
    m_isSymbol = symbol;
}

bool TimeSignature::getIsSymbol() const
{
    if (m_numerator == 2 && m_denominator == 2) {
        return true;
    }

    return m_isSymbol;
}

void TimeSignature::setBeatLength(int length)
{
    m_beatLength = length;
}

int TimeSignature::getBeatLength() const
{
    return m_beatLength;
}

void TimeSignature::setBarLength(int length)
{
    m_barLength = length;
}

int TimeSignature::getBarLength() const
{
    return m_barLength;
}

void TimeSignature::addBeat(int startUnit, int lengthUnit, int startTick)
{
    BeatNode node;
    node.m_startUnit = startUnit;
    node.m_lengthUnit = lengthUnit;
    node.m_startTick = startTick;
    m_beats.push_back(node);
}

void TimeSignature::endAddBeat()
{
    int i;
    m_barLengthUnits = 0;

    for (i = 0; i < m_beats.size(); ++i) {
        m_barLengthUnits += m_beats[i].m_lengthUnit;
    }
}

int TimeSignature::getUnits() const
{
    return m_barLengthUnits;
}

void TimeSignature::setReplaceFont(bool replace)
{
    m_replaceFont = replace;
}

bool TimeSignature::getReplaceFont() const
{
    return m_replaceFont;
}

void TimeSignature::setShowBeatGroup(bool show)
{
    m_showBeatGroup = show;
}

bool TimeSignature::getShowBeatGroup() const
{
    return m_showBeatGroup;
}

void TimeSignature::setGroupNumerator1(int numerator)
{
    m_groupNumerator1 = numerator;
}

void TimeSignature::setGroupNumerator2(int numerator)
{
    m_groupNumerator2 = numerator;
}

void TimeSignature::setGroupNumerator3(int numerator)
{
    m_groupNumerator3 = numerator;
}

void TimeSignature::setGroupDenominator1(int denominator)
{
    m_groupDenominator1 = denominator;
}

void TimeSignature::setGroupDenominator2(int denominator)
{
    m_groupDenominator2 = denominator;
}

void TimeSignature::setGroupDenominator3(int denominator)
{
    m_groupDenominator3 = denominator;
}

void TimeSignature::setBeamGroup1(int count)
{
    m_beamGroup1 = count;
}

void TimeSignature::setBeamGroup2(int count)
{
    m_beamGroup2 = count;
}

void TimeSignature::setBeamGroup3(int count)
{
    m_beamGroup3 = count;
}

void TimeSignature::setBeamGroup4(int count)
{
    m_beamGroup4 = count;
}

void TimeSignature::set16thBeamCount(int count)
{
    m_beamCount16th = count;
}

void TimeSignature::set32ndBeamCount(int count)
{
    m_beamCount32nd = count;
}

Key::Key()
{
    m_key = 0;
    m_set = false;
    m_previousKey = 0;
    m_symbolCount = 0;
}

void Key::setKey(int key)
{
    m_key = key;
    m_set = true;
}

int Key::getKey() const
{
    return m_key;
}

bool Key::getSetKey() const
{
    return m_set;
}

void Key::setPreviousKey(int key)
{
    m_previousKey = key;
}

int Key::getPreviousKey() const
{
    return m_previousKey;
}

void Key::setSymbolCount(int count)
{
    m_symbolCount = count;
}

int Key::getSymbolCount() const
{
    return m_symbolCount;
}

RepeatSymbol::RepeatSymbol()
    : m_text("#1"), m_repeatType(RepeatType::Segno)
{
    m_musicDataType = MusicDataType::Repeat;
}

void RepeatSymbol::setText(const QString& text)
{
    m_text = text;
}

QString RepeatSymbol::getText() const
{
    return m_text;
}

void RepeatSymbol::setRepeatType(int repeatType)
{
    m_repeatType = (RepeatType)repeatType;
}

RepeatType RepeatSymbol::getRepeatType() const
{
    return m_repeatType;
}

NumericEnding::NumericEnding()
{
    m_musicDataType = MusicDataType::Numeric_Ending;

    m_height = 0;
    m_text = QString();
    m_numericHandle = new OffsetElement();
}

NumericEnding::~NumericEnding()
{
    delete m_numericHandle;
}

OffsetElement* NumericEnding::getNumericHandle() const
{
    return m_numericHandle;
}

void NumericEnding::setHeight(int height)
{
    m_height = height;
}

int NumericEnding::getHeight() const
{
    return m_height;
}

void NumericEnding::setText(const QString& text)
{
    m_text = text;
}

QString NumericEnding::getText() const
{
    return m_text;
}

QList<int> NumericEnding::getNumbers() const
{
    int i;
    QStringList strs = m_text.split(",", Qt::SkipEmptyParts);
    QList<int> endings;

    for (i = 0; i < strs.size(); ++i) {
        bool ok;
        int num = strs[i].toInt(&ok);
        endings.push_back(num);
    }

    return endings;
}

int NumericEnding::getJumpCount() const
{
    QList<int> numbers = getNumbers();
    int count = 0;

    for (int i = 0; i < numbers.size(); ++i) {
        if ((int)i + 1 != numbers[i]) {
            break;
        }

        count = i + 1;
    }

    return count;
}

BarNumber::BarNumber()
{
    m_index = 0;
    m_showOnParagraphStart = false;
    m_align = 0;
    m_showFlag = 1;   // staff
    m_barRange = 1;   // can't be 0
    m_prefix = QString();
}

void BarNumber::setIndex(int index)
{
    m_index = index;
}

int BarNumber::getIndex() const
{
    return m_index;
}

void BarNumber::setShowOnParagraphStart(bool show)
{
    m_showOnParagraphStart = show;
}

bool BarNumber::getShowOnParagraphStart() const
{
    return m_showOnParagraphStart;
}

void BarNumber::setAlign(int align) // 0: left, 1: center, 2: right
{
    m_align = align;
}

int BarNumber::getAlign() const
{
    return m_align;
}

void BarNumber::setShowFlag(int flag)
{
    m_showFlag = flag;
}

int BarNumber::getShowFlag() const
{
    return m_showFlag;
}

void BarNumber::setShowEveryBarCount(int count)
{
    m_barRange = count;
}

int BarNumber::getShowEveryBarCount() const
{
    return m_barRange;
}

void BarNumber::setPrefix(const QString& str)
{
    m_prefix = str;
}

QString BarNumber::getPrefix() const
{
    return m_prefix;
}

MidiController::MidiController()
{
    m_midiType = MidiType::Controller;
    m_controller = 64; // pedal
    m_value = 0;
}

void MidiController::setController(int number)
{
    m_controller = number;
}

int MidiController::getController() const
{
    return m_controller;
}

void MidiController::setValue(int value)
{
    m_value = value;
}

int MidiController::getValue() const
{
    return m_value;
}

MidiProgramChange::MidiProgramChange()
{
    m_midiType = MidiType::Program_Change;
    m_patch = 0; // grand piano
}

void MidiProgramChange::setPatch(int patch)
{
    m_patch = patch;
}

int MidiProgramChange::getPatch() const
{
    return m_patch;
}

MidiChannelPressure::MidiChannelPressure()
    : m_pressure(0)
{
    m_midiType = MidiType::Channel_Pressure;
}

void MidiChannelPressure::setPressure(int pressure)
{
    m_pressure = pressure;
}

int MidiChannelPressure::getPressure() const
{
    return m_pressure;
}

MidiPitchWheel::MidiPitchWheel()
{
    m_midiType = MidiType::Pitch_Wheel;
    m_value = 0;
}

void MidiPitchWheel::setValue(int value)
{
    m_value = value;
}

int MidiPitchWheel::getValue() const
{
    return m_value;
}

Measure::Measure(int index)
{
    m_barNumber = new BarNumber();
    m_barNumber->setIndex(index);
    m_time = new TimeSignature();

    clear();
}

Measure::~Measure()
{
    clear();

    delete m_barNumber;
    delete m_time;
}

BarNumber* Measure::getBarNumber() const
{
    return m_barNumber;
}

TimeSignature* Measure::getTime() const
{
    return m_time;
}

void Measure::setLeftBarline(int barline)
{
    m_leftBarline = (BarLineType)barline;
}

BarLineType Measure::getLeftBarline() const
{
    return m_leftBarline;
}

void Measure::setRightBarline(int barline)
{
    m_rightBarline = (BarLineType)barline;
}

BarLineType Measure::getRightBarline() const
{
    return m_rightBarline;
}

void Measure::setBackwardRepeatCount(int repeatCount)
{
    m_repeatCount = repeatCount;
}

int Measure::getBackwardRepeatCount() const
{
    return m_repeatCount;
}

void Measure::setTypeTempo(double tempo)
{
    m_typeTempo = tempo;
}

double Measure::getTypeTempo() const
{
    return m_typeTempo;
}

void Measure::setIsPickup(bool pickup)
{
    m_pickup = pickup;
}

bool Measure::getIsPickup() const
{
    return m_pickup;
}

void Measure::setIsMultiMeasureRest(bool rest)
{
    m_multiMeasureRest = rest;
}

bool Measure::getIsMultiMeasureRest() const
{
    return m_multiMeasureRest;
}

void Measure::setMultiMeasureRestCount(int count)
{
    m_multiMeasureRestCount = count;
}

int Measure::getMultiMeasureRestCount() const
{
    return m_multiMeasureRestCount;
}

void Measure::clear()
{
    m_leftBarline = BarLineType::Default;
    m_rightBarline = BarLineType::Default;
    m_repeatCount = 1;
    m_typeTempo = 96.00;
    setLength(0x780);   //time = 4/4
    m_pickup = false;
    m_multiMeasureRest = false;
    m_multiMeasureRestCount = 0;
}

MeasureData::MeasureData()
{
    m_key = new Key();
    m_clef = new Clef();
}

MeasureData::~MeasureData()
{
    int i;
    for (i = 0; i < m_musicDatas.size(); ++i) {
        delete m_musicDatas[i];
    }
    m_musicDatas.clear();

    // m_noteContainers also in m_musicDatas, no need to destroy
    m_noteContainers.clear();

    // only delete at element start
    for (i = 0; i < m_crossMeasureElements.size(); ++i) {
        if (m_crossMeasureElements[i].second) {
            delete m_crossMeasureElements[i].first;
        }
    }
    m_crossMeasureElements.clear();

    for (i = 0; i < m_midiDatas.size(); ++i) {
        delete m_midiDatas[i];
    }
    m_midiDatas.clear();

    delete m_key;
    delete m_clef;
}

Key* MeasureData::getKey() const
{
    return m_key;
}

Clef* MeasureData::getClef() const
{
    return m_clef;
}

void MeasureData::addNoteContainer(NoteContainer* ptr)
{
    m_noteContainers.push_back(ptr);
}

QList<NoteContainer*> MeasureData::getNoteContainers() const
{
    return m_noteContainers;
}

void MeasureData::addMusicData(MusicData* ptr)
{
    m_musicDatas.push_back(ptr);
}

QList<MusicData*> MeasureData::getMusicDatas(MusicDataType type)
{
    int i;
    QList<MusicData*> notations;

    for (i = 0; i < m_musicDatas.size(); ++i) {
        if (type == MusicDataType::None || m_musicDatas[i]->getMusicDataType() == type) {
            notations.push_back(m_musicDatas[i]);
        }
    }

    return notations;
}

void MeasureData::addCrossMeasureElement(MusicData* ptr, bool start)
{
    m_crossMeasureElements.push_back(qMakePair(ptr, start));
}

QList<MusicData*> MeasureData::getCrossMeasureElements(
    MusicDataType type, PairType pairType)
{
    int i;
    QList<MusicData*> pairs;

    for (i = 0; i < m_crossMeasureElements.size(); ++i) {
        if ((type == MusicDataType::None || m_crossMeasureElements[i].first->getMusicDataType() == type)
            && (pairType == PairType::All || ((m_crossMeasureElements[i].second && pairType == PairType::Start)
                                              || (!m_crossMeasureElements[i].second && pairType == PairType::Stop)))) {
            pairs.push_back(m_crossMeasureElements[i].first);
        }
    }

    return pairs;
}

void MeasureData::addMidiData(MidiData* ptr)
{
    m_midiDatas.push_back(ptr);
}

QList<MidiData*> MeasureData::getMidiDatas(MidiType type)
{
    int i;
    QList<MidiData*> datas;

    for (i = 0; i < m_midiDatas.size(); ++i) {
        if (type == MidiType::None || m_midiDatas[i]->getMidiType() == type) {
            datas.push_back(m_midiDatas[i]);
        }
    }

    return datas;
}

StreamHandle::StreamHandle()
    : m_size(0), m_curPos(0), m_point(NULL)
{
}

StreamHandle::StreamHandle(unsigned char* p, int size)
    : m_size(size), m_curPos(0), m_point(p)
{
}

StreamHandle::~StreamHandle()
{
    m_point = NULL;
}

bool StreamHandle::read(char* buff, int size)
{
    if (m_point != NULL && m_curPos + size <= m_size) {
        memcpy(buff, m_point + m_curPos, size);
        m_curPos += size;

        return true;
    }

    return false;
}

bool StreamHandle::write(char* /* buff */, int /* size */)
{
    return true;
}

Block::Block()
{
    doResize(0);
}

Block::Block(unsigned int count)
{
    doResize(count);
}

void Block::resize(unsigned int count)
{
    doResize(count);
}

void Block::doResize(unsigned int count)
{
    m_data.clear();
    for (unsigned int i = 0; i < count; ++i) {
        m_data.push_back('\0');
    }
    // m_data.resize(count);
}

const unsigned char* Block::data() const
{
    // return const_cast<unsigned char*>(&m_data.front());
    return &m_data.front();
}

unsigned char* Block::data()
{
    return &m_data.front();
}

int Block::size() const
{
    return m_data.size();
}

bool Block::toBoolean() const
{
    if (data() == NULL) {
        return false;
    }

    return size() == 1 && data()[0] == 0x01;
}

unsigned int Block::toUnsignedInt() const
{
    if (data() == NULL) {
        return 0;
    }

    int num = 0;

    for (int i = 0; i < (int)sizeof(int) && i < size(); ++i) {
        num = (num << 8) + *(data() + i);
    }

    return num;
}

int Block::toInt() const
{
    if (data() == NULL) {
        return 0;
    }

    int i;
    int num = 0;

    for (i = 0; i < (int)sizeof(int) && i < size(); ++i) {
        num = (num << 8) + static_cast<int>(*(data() + i));
    }

    std::size_t minSize = sizeof(int);
    if (size() < static_cast<int>(minSize)) {
        minSize = size();
    }

    if ((*(data()) & 0x80) == 0x80) {
        // same as int(pow(2, int(minSize) * 8))
        int maxNum = 1 << (static_cast<int>(minSize) * 8);
        num -= maxNum;
        // num *= -1;
    }

    return num;
}

QByteArray Block::toStrByteArray() const
{
    if (data() == NULL) {
        return QByteArray();
    }

    QByteArray arr((char*)data(), size());

    return arr;
}

QByteArray Block::fixedSizeBufferToStrByteArray() const
{
    QByteArray str;

    for (int i = 0; i < size(); ++i) {
        if (*(data() + i) == '\0') {
            break;
        }

        str += (char)*(data() + i);
    }

    return str;
}

bool Block::operator==(const Block& block) const
{
    if (size() != block.size()) {
        return false;
    }

    for (int i = 0; i < size() && i < block.size(); ++i) {
        if (*(data() + i) != *(block.data() + i)) {
            return false;
        }
    }

    return true;
}

bool Block::operator!=(const Block& block) const
{
    return !(*this == block);
}

FixedBlock::FixedBlock()
    : Block()
{
}

FixedBlock::FixedBlock(unsigned int count)
    : Block(count)
{
}

void FixedBlock::resize(unsigned int /* count */)
{
    // Block::resize(size);
}

SizeBlock::SizeBlock()
    : FixedBlock(4)
{
}

unsigned int SizeBlock::toSize() const
{
    unsigned int i;
    unsigned int num(0);
    const unsigned int SIZE = 4;

    for (i = 0; i < SIZE; ++i) {
        num = (num << 8) + *(data() + i);
    }

    return num;
}

/*
void SizeBlock::fromUnsignedInt(unsigned int count)
{
    unsigned_int_to_char_buffer(count, data());
}
*/

NameBlock::NameBlock()
    : FixedBlock(4)
{
}

/*
void NameBlock::setValue(const char* const name)
{
    unsigned int i;

    for (i = 0; i < size() && *(name + i) != '\0'; ++i) {
        *(data() + i) = *(name + i);
    }
}
*/

bool NameBlock::isEqual(const QString& name) const
{
    int nsize = name.size();

    if (nsize != size()) {
        return false;
    }

    for (int i = 0; i < size() && nsize; ++i) {
        if (name[i] != QChar(data()[i])) {
            return false;
        }
    }

    return true;
}

CountBlock::CountBlock()
    : FixedBlock(2)
{
}

/*
void CountBlock::setValue(unsigned short count)
{
    unsigned int i;
    unsigned int SIZE = sizeof(unsigned short);

    for (i = 0; i < SIZE; ++i) {
        data()[SIZE - 1 - i] = count % 256;
        count /= 256;
    }
}
*/

unsigned short CountBlock::toCount() const
{
    unsigned short num = 0;

    for (int i = 0; i < size() && i < (int)sizeof(unsigned short); ++i) {
        num = (num << 8) + *(data() + i);
    }

    return num;
}

const QString Chunk::TrackName   = "TRAK";
const QString Chunk::PageName    = "PAGE";
const QString Chunk::LineName    = "LINE";
const QString Chunk::StaffName   = "STAF";
const QString Chunk::MeasureName = "MEAS";
const QString Chunk::ConductName = "COND";
const QString Chunk::BdatName    = "BDAT";

Chunk::Chunk()
{
}

NameBlock Chunk::getName() const
{
    return m_nameBlock;
}

const unsigned int SizeChunk::version3TrackSize = 0x13a;

SizeChunk::SizeChunk()
    : Chunk()
{
    m_sizeBlock = new SizeBlock();
    m_dataBlock = new Block();
}

SizeChunk::~SizeChunk()
{
    delete m_sizeBlock;
    delete m_dataBlock;
}

SizeBlock* SizeChunk::getSizeBlock() const
{
    return m_sizeBlock;
}

Block* SizeChunk::getDataBlock() const
{
    return m_dataBlock;
}

GroupChunk::GroupChunk()
    : Chunk()
{
    m_childCount = new CountBlock();
}

GroupChunk::~GroupChunk()
{
    delete m_childCount;
}

CountBlock* GroupChunk::getCountBlock() const
{
    return m_childCount;
}

unsigned int getHighNibble(unsigned int byte)
{
    return byte / 16;
}

unsigned int getLowNibble(unsigned int byte)
{
    return byte % 16;
}

int oveKeyToKey(int oveKey)
{
    int key = 0;

    if (oveKey == 0) {
        key = 0;
    } else if (oveKey > 7) {
        key = oveKey - 7;
    } else if (oveKey <= 7) {
        key = oveKey * (-1);
    }

    return key;
}

BasicParse::BasicParse(OveSong* ove)
    : m_ove(ove), m_handle(NULL), m_notify(NULL)
{
}

BasicParse::BasicParse()
    : m_ove(NULL), m_handle(NULL), m_notify(NULL)
{
}

BasicParse::~BasicParse()
{
    m_ove = NULL;
    m_handle = NULL;
    m_notify = NULL;
}

void BasicParse::setNotify(IOveNotify* notify)
{
    m_notify = notify;
}

bool BasicParse::parse()
{
    return false;
}

bool BasicParse::readBuffer(Block& placeHolder, int size)
{
    if (m_handle == NULL) {
        return false;
    }
    if (placeHolder.size() != size) {
        placeHolder.resize(size);
    }

    if (size > 0) {
        return m_handle->read((char*)placeHolder.data(), placeHolder.size());
    }

    return true;
}

bool BasicParse::jump(int offset)
{
    if (m_handle == NULL || offset < 0) {
        return false;
    }

    if (offset > 0) {
        Block placeHolder(offset);
        return m_handle->read((char*)placeHolder.data(), placeHolder.size());
    }

    return true;
}

void BasicParse::messageOut(const QString& str)
{
    if (m_notify != NULL) {
        m_notify->loadInfo(str);
    }
}

OvscParse::OvscParse(OveSong* ove)
    : BasicParse(ove), m_chunk(NULL)
{
}

OvscParse::~OvscParse()
{
    m_chunk = NULL;
}

void OvscParse::setOvsc(SizeChunk* chunk)
{
    m_chunk = chunk;
}

bool OvscParse::parse()
{
    Block* dataBlock = m_chunk->getDataBlock();
    unsigned int blockSize = m_chunk->getSizeBlock()->toSize();
    StreamHandle handle(dataBlock->data(), blockSize);
    Block placeHolder;

    m_handle = &handle;

    // version
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    bool version4 = placeHolder.toUnsignedInt() == 4;
    m_ove->setIsVersion4(version4);

    QString str = QString("This file is created by Overture ") + (version4 ? "4" : "3") + "\n";
    messageOut(str);

    if (!jump(6)) {
        return false;
    }

    // show page margin
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    m_ove->setShowPageMargin(placeHolder.toBoolean());

    if (!jump(1)) {
        return false;
    }

    // transpose track
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    m_ove->setShowTransposeTrack(placeHolder.toBoolean());

    // play repeat
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    m_ove->setPlayRepeat(placeHolder.toBoolean());

    // play style
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    OveSong::PlayStyle style = OveSong::PlayStyle::Record;
    if (placeHolder.toUnsignedInt() == 1) {
        style = OveSong::PlayStyle::Swing;
    } else if (placeHolder.toUnsignedInt() == 2) {
        style = OveSong::PlayStyle::Notation;
    }
    m_ove->setPlayStyle(style);

    // show line break
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    m_ove->setShowLineBreak(placeHolder.toBoolean());

    // show ruler
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    m_ove->setShowRuler(placeHolder.toBoolean());

    // show color
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    m_ove->setShowColor(placeHolder.toBoolean());

    return true;
}

TrackParse::TrackParse(OveSong* ove)
    : BasicParse(ove)
{
}

TrackParse::~TrackParse()
{
}

void TrackParse::setTrack(SizeChunk* chunk)
{
    m_chunk = chunk;
}

bool TrackParse::parse()
{
    Block* dataBlock = m_chunk->getDataBlock();
    unsigned int blockSize = m_ove->getIsVersion4() ? m_chunk->getSizeBlock()->toSize() : SizeChunk::version3TrackSize;
    StreamHandle handle(dataBlock->data(), blockSize);
    Block placeHolder;

    m_handle = &handle;

    Track* oveTrack = new Track();
    m_ove->addTrack(oveTrack);

    // 2 32bytes long track name buffer
    if (!readBuffer(placeHolder, 32)) {
        return false;
    }
    oveTrack->setName(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    if (!readBuffer(placeHolder, 32)) {
        return false;
    }
    oveTrack->setBriefName(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    if (!jump(8)) {
        return false;
    } // 0xfffa0012 fffa0012
    if (!jump(1)) {
        return false;
    }

    // patch
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int thisByte = placeHolder.toInt();
    oveTrack->setPatch(thisByte & 0x7f);

    // show name
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowName(placeHolder.toBoolean());

    // show brief name
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowBriefName(placeHolder.toBoolean());

    if (!jump(1)) {
        return false;
    }

    // show transpose
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowTranspose(placeHolder.toBoolean());

    if (!jump(1)) {
        return false;
    }

    // mute
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setMute(placeHolder.toBoolean());

    // solo
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setSolo(placeHolder.toBoolean());

    if (!jump(1)) {
        return false;
    }

    // show key each line
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowKeyEachLine(placeHolder.toBoolean());

    // voice count
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setVoiceCount(placeHolder.toUnsignedInt());

    if (!jump(3)) {
        return false;
    }

    // transpose value [-127, 127]
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setTranspose(placeHolder.toInt());

    if (!jump(2)) {
        return false;
    }

    // start clef
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setStartClef(placeHolder.toUnsignedInt());

    // transpose celf
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setTransposeClef(placeHolder.toUnsignedInt());

    // start key
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setStartKey(placeHolder.toUnsignedInt());

    // display percent
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setDisplayPercent(placeHolder.toUnsignedInt());

    // show leger line
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowLegerLine(placeHolder.toBoolean());

    // show clef
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowClef(placeHolder.toBoolean());

    // show time signature
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowTimeSignature(placeHolder.toBoolean());

    // show key signature
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowKeySignature(placeHolder.toBoolean());

    // show barline
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowBarline(placeHolder.toBoolean());

    // fill with rest
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setFillWithRest(placeHolder.toBoolean());

    // flat tail
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setFlatTail(placeHolder.toBoolean());

    // show clef each line
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setShowClefEachLine(placeHolder.toBoolean());

    if (!jump(12)) {
        return false;
    }

    // 8 voices
    int i;
    QList<Voice*> voices;
    for (i = 0; i < 8; ++i) {
        Voice* voicePtr = new Voice();

        if (!jump(5)) {
            return false;
        }

        // channel
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        voicePtr->setChannel(placeHolder.toUnsignedInt());

        // volume
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        voicePtr->setVolume(placeHolder.toInt());

        // pitch shift
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        voicePtr->setPitchShift(placeHolder.toInt());

        // pan
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        voicePtr->setPan(placeHolder.toInt());

        if (!jump(6)) {
            return false;
        }

        // patch
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        voicePtr->setPatch(placeHolder.toInt());

        voices.push_back(voicePtr);
    }

    // stem type
    for (i = 0; i < 8; ++i) {
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        voices[i]->setStemType(placeHolder.toUnsignedInt());

        oveTrack->addVoice(voices[i]);
    }

    // percussion define
    QList<Track::DrumNode> nodes;
    for (i = 0; i < 16; ++i) {
        nodes.push_back(Track::DrumNode());
    }

    // line
    for (i = 0; i < 16; ++i) {
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        nodes[i].m_line = placeHolder.toInt();
    }

    // head type
    for (i = 0; i < 16; ++i) {
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        nodes[i].m_headType = placeHolder.toUnsignedInt();
    }

    // pitch
    for (i = 0; i < 16; ++i) {
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        nodes[i].m_pitch = placeHolder.toUnsignedInt();
    }

    // voice
    for (i = 0; i < 16; ++i) {
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        nodes[i].m_voice = placeHolder.toUnsignedInt();
    }

    for (i = 0; i < nodes.size(); ++i) {
        oveTrack->addDrum(nodes[i]);
    }

    /*
    if (!Jump(17)) {
        return false;
    }

    // voice 0 channel
    if (!ReadBuffer(placeHolder, 1)) {
        return false;
    }
    oveTrack->setChannel(placeHolder.toUnsignedInt());

    // to be continued. if anything important...*/

    return true;
}

GroupParse::GroupParse(OveSong* ove)
    : BasicParse(ove)
{
}

GroupParse::~GroupParse()
{
    m_sizeChunks.clear();
}

void GroupParse::addSizeChunk(SizeChunk* sizeChunk)
{
    m_sizeChunks.push_back(sizeChunk);
}

bool GroupParse::parse()
{
    return false;
}

PageGroupParse::PageGroupParse(OveSong* ove)
    : BasicParse(ove)
{
}

PageGroupParse::~PageGroupParse()
{
    m_pageChunks.clear();
}

void PageGroupParse::addPage(SizeChunk* chunk)
{
    m_pageChunks.push_back(chunk);
}

bool PageGroupParse::parse()
{
    if (m_pageChunks.isEmpty()) {
        return false;
    }

    int i;
    for (i = 0; i < m_pageChunks.size(); ++i) {
        Page* page = new Page();
        m_ove->addPage(page);

        if (!parsePage(m_pageChunks[i], page)) {
            return false;
        }
    }

    return true;
}

bool PageGroupParse::parsePage(SizeChunk* chunk, Page* page)
{
    Block placeHolder(2);
    StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

    m_handle = &handle;

    // begin line
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setBeginLine(placeHolder.toUnsignedInt());

    // line count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setLineCount(placeHolder.toUnsignedInt());

    if (!jump(4)) {
        return false;
    }

    // staff interval
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setStaffInterval(placeHolder.toUnsignedInt());

    // line interval
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setLineInterval(placeHolder.toUnsignedInt());

    // staff inline interval
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setStaffInlineInterval(placeHolder.toUnsignedInt());

    // line bar count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setLineBarCount(placeHolder.toUnsignedInt());

    // page line count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    page->setPageLineCount(placeHolder.toUnsignedInt());

    // left margin
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    page->setLeftMargin(placeHolder.toUnsignedInt());

    // top margin
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    page->setTopMargin(placeHolder.toUnsignedInt());

    // right margin
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    page->setRightMargin(placeHolder.toUnsignedInt());

    // bottom margin
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    page->setBottomMargin(placeHolder.toUnsignedInt());

    // page width
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    page->setPageWidth(placeHolder.toUnsignedInt());

    // page height
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    page->setPageHeight(placeHolder.toUnsignedInt());

    m_handle = NULL;

    return true;
}

StaffCountGetter::StaffCountGetter(OveSong* ove)
    : BasicParse(ove)
{
}

unsigned int StaffCountGetter::getStaffCount(SizeChunk* chunk)
{
    StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());
    Block placeHolder;

    m_handle = &handle;

    if (!jump(6)) {
        return false;
    }

    // staff count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    return placeHolder.toUnsignedInt();
}

LineGroupParse::LineGroupParse(OveSong* ove)
    : BasicParse(ove), m_chunk(NULL)
{
}

LineGroupParse::~LineGroupParse()
{
    m_chunk = NULL;
    m_lineChunks.clear();
    m_staffChunks.clear();
}

void LineGroupParse::setLineGroup(GroupChunk* chunk)
{
    m_chunk = chunk;
}

void LineGroupParse::addLine(SizeChunk* chunk)
{
    m_lineChunks.push_back(chunk);
}

void LineGroupParse::addStaff(SizeChunk* chunk)
{
    m_staffChunks.push_back(chunk);
}

bool LineGroupParse::parse()
{
    if (m_lineChunks.isEmpty() || m_staffChunks.size() % m_lineChunks.size() != 0) {
        return false;
    }

    int i;
    unsigned int j;
    unsigned int lineStaffCount = m_staffChunks.size() / m_lineChunks.size();

    for (i = 0; i < m_lineChunks.size(); ++i) {
        Line* linePtr = new Line();

        m_ove->addLine(linePtr);

        if (!parseLine(m_lineChunks[i], linePtr)) {
            return false;
        }

        for (j = lineStaffCount * i; j < lineStaffCount * (i + 1); ++j) {
            Staff* staffPtr = new Staff();

            linePtr->addStaff(staffPtr);

            if (!parseStaff(m_staffChunks[j], staffPtr)) {
                return false;
            }
        }
    }

    return true;
}

bool LineGroupParse::parseLine(SizeChunk* chunk, Line* line)
{
    Block placeHolder;

    StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

    m_handle = &handle;

    if (!jump(2)) {
        return false;
    }

    // begin bar
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    line->setBeginBar(placeHolder.toUnsignedInt());

    // bar count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    line->setBarCount(placeHolder.toUnsignedInt());

    if (!jump(6)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    line->setYOffset(placeHolder.toInt());

    // left x offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    line->setLeftXOffset(placeHolder.toInt());

    // right x offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    line->setRightXOffset(placeHolder.toInt());

    if (!jump(4)) {
        return false;
    }

    m_handle = NULL;

    return true;
}

bool LineGroupParse::parseStaff(SizeChunk* chunk, Staff* staff)
{
    Block placeHolder;

    StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

    m_handle = &handle;

    if (!jump(7)) {
        return false;
    }

    // clef
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    staff->setClefType(placeHolder.toUnsignedInt());

    // key
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    staff->setKeyType(oveKeyToKey(placeHolder.toUnsignedInt()));

    if (!jump(2)) {
        return false;
    }

    // visible
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    staff->setVisible(placeHolder.toBoolean());

    if (!jump(12)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    staff->setYOffset(placeHolder.toInt());

    int jumpAmount = m_ove->getIsVersion4() ? 26 : 18;
    if (!jump(jumpAmount)) {
        return false;
    }

    // group type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    GroupType groupType = GroupType::None;
    if (placeHolder.toUnsignedInt() == 1) {
        groupType = GroupType::Brace;
    } else if (placeHolder.toUnsignedInt() == 2) {
        groupType = GroupType::Bracket;
    }
    staff->setGroupType(groupType);

    // group staff count
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    staff->setGroupStaffCount(placeHolder.toUnsignedInt());

    m_handle = NULL;

    return true;
}

BarsParse::BarsParse(OveSong* ove)
    : BasicParse(ove)
{
}

BarsParse::~BarsParse()
{
    m_measureChunks.clear();
    m_conductChunks.clear();
    m_bdatChunks.clear();
}

void BarsParse::addMeasure(SizeChunk* chunk)
{
    m_measureChunks.push_back(chunk);
}

void BarsParse::addConduct(SizeChunk* chunk)
{
    m_conductChunks.push_back(chunk);
}

void BarsParse::addBdat(SizeChunk* chunk)
{
    m_bdatChunks.push_back(chunk);
}

bool BarsParse::parse()
{
    int i;
    int trackMeasureCount = m_ove->getTrackBarCount();
    int trackCount = m_ove->getTrackCount();
    int measureDataCount = trackCount * m_measureChunks.size();
    QList<Measure*> measures;
    QList<MeasureData*> measureDatas;

    if (m_measureChunks.isEmpty()
        || m_measureChunks.size() != m_conductChunks.size()
        || m_bdatChunks.size() != measureDataCount) {
        return false;
    }

    // add to ove
    for (i = 0; i < m_measureChunks.size(); ++i) {
        Measure* measure = new Measure(i);

        measures.push_back(measure);
        m_ove->addMeasure(measure);
    }

    for (i = 0; i < measureDataCount; ++i) {
        MeasureData* oveMeasureData = new MeasureData();

        measureDatas.push_back(oveMeasureData);
        m_ove->addMeasureData(oveMeasureData);
    }

    for (i = 0; i < m_measureChunks.size(); ++i) {
        Measure* measure = measures[i];

        // MEAS
        if (!parseMeas(measure, m_measureChunks[i])) {
            QString ss = QString("failed in parse MEAS %1\n").arg(i);
            messageOut(ss);

            return false;
        }
    }

    for (i = 0; i < m_conductChunks.size(); ++i) {
        // COND
        if (!parseCond(measures[i], measureDatas[i], m_conductChunks[i])) {
            QString ss = QString("failed in parse COND %1\n").arg(i);
            messageOut(ss);

            return false;
        }
    }

    for (i = 0; i < m_bdatChunks.size(); ++i) {
        int measId = i % trackMeasureCount;

        // BDAT
        if (!parseBdat(measures[measId], measureDatas[i], m_bdatChunks[i])) {
            QString ss = QString("failed in parse BDAT %1\n").arg(i);
            messageOut(ss);

            return false;
        }

        if (m_notify != NULL) {
            int measureID = i % trackMeasureCount;
            int trackID = i / trackMeasureCount;

            //msg.m_msg = OVE_IMPORT_POS;
            //msg.m_param1 = (measureID<<16) + trackMeasureCount;
            //msg.m_param2 = (trackID<<16) + trackCount;

            m_notify->loadPosition(measureID, trackMeasureCount, trackID, trackCount);
        }
    }

    return true;
}

bool BarsParse::parseMeas(Measure* measure, SizeChunk* chunk)
{
    Block placeHolder;

    StreamHandle measureHandle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

    m_handle = &measureHandle;

    if (!jump(2)) {
        return false;
    }

    // multi-measure rest
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    measure->setIsMultiMeasureRest(placeHolder.toBoolean());

    // pickup
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    measure->setIsPickup(placeHolder.toBoolean());

    if (!jump(4)) {
        return false;
    }

    // left barline
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    measure->setLeftBarline(placeHolder.toUnsignedInt());

    // right barline
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    measure->setRightBarline(placeHolder.toUnsignedInt());

    // tempo
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    double tempo = ((double)placeHolder.toUnsignedInt());
    if (m_ove->getIsVersion4()) {
        tempo /= 100.0;
    }
    measure->setTypeTempo(tempo);

    // bar length(tick)
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    measure->setLength(placeHolder.toUnsignedInt());

    if (!jump(6)) {
        return false;
    }

    // bar number offset
    if (!parseOffsetElement(measure->getBarNumber())) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // multi-measure rest count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    measure->setMultiMeasureRestCount(placeHolder.toUnsignedInt());

    m_handle = NULL;

    return true;
}

bool BarsParse::parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk)
{
    Block placeHolder;

    StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

    m_handle = &handle;

    // item count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    unsigned int cnt = placeHolder.toUnsignedInt();

    if (!parseTimeSignature(measure, 36)) {
        return false;
    }

    for (unsigned int i = 0; i < cnt; ++i) {
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        unsigned int twoByte = placeHolder.toUnsignedInt();
        unsigned int oldBlockSize = twoByte - 11;
        unsigned int newBlockSize = twoByte - 7;

        // type id
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        unsigned int thisByte = placeHolder.toUnsignedInt();
        CondType type;

        if (!getCondElementType(thisByte, type)) {
            return false;
        }

        switch (type) {
        case CondType::Bar_Number: {
            if (!parseBarNumber(measure, twoByte - 1)) {
                return false;
            }
            break;
        }
        case CondType::Repeat: {
            if (!parseRepeatSymbol(measureData, oldBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Numeric_Ending: {
            if (!parseNumericEndings(measureData, oldBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Decorator: {
            if (!parseDecorators(measureData, newBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Tempo: {
            if (!parseTempo(measureData, newBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Text: {
            if (!parseText(measureData, newBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Expression: {
            if (!parseExpressions(measureData, newBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Time_Parameters: {
            if (!parseTimeSignatureParameters(measure, newBlockSize)) {
                return false;
            }
            break;
        }
        case CondType::Barline_Parameters: {
            if (!parseBarlineParameters(measure, newBlockSize)) {
                return false;
            }
            break;
        }
        default: {
            if (!jump(newBlockSize)) {
                return false;
            }
            break;
        }
        }
    }

    m_handle = NULL;

    return true;
}

bool BarsParse::parseTimeSignature(Measure* measure, int /* length */)
{
    Block placeHolder;

    TimeSignature* timeSignature = measure->getTime();

    // numerator
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setNumerator(placeHolder.toUnsignedInt());

    // denominator
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setDenominator(placeHolder.toUnsignedInt());

    if (!jump(2)) {
        return false;
    }

    // beat length
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    timeSignature->setBeatLength(placeHolder.toUnsignedInt());

    // bar length
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    timeSignature->setBarLength(placeHolder.toUnsignedInt());

    if (!jump(4)) {
        return false;
    }

    // is symbol
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setIsSymbol(placeHolder.toBoolean());

    if (!jump(1)) {
        return false;
    }

    // replace font
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setReplaceFont(placeHolder.toBoolean());

    // color
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setColor(placeHolder.toUnsignedInt());

    // show
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setShow(placeHolder.toBoolean());

    // show beat group
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setShowBeatGroup(placeHolder.toBoolean());

    if (!jump(6)) {
        return false;
    }

    // numerator 1, 2, 3
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setGroupNumerator1(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setGroupNumerator2(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setGroupNumerator3(placeHolder.toUnsignedInt());

    // denominator
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setGroupDenominator1(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setGroupDenominator2(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setGroupDenominator3(placeHolder.toUnsignedInt());

    // beam group 1~4
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setBeamGroup1(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setBeamGroup2(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setBeamGroup3(placeHolder.toUnsignedInt());
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->setBeamGroup4(placeHolder.toUnsignedInt());

    // beam 16th
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->set16thBeamCount(placeHolder.toUnsignedInt());

    // beam 32nd
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    timeSignature->set32ndBeamCount(placeHolder.toUnsignedInt());

    return true;
}

bool BarsParse::parseTimeSignatureParameters(Measure* measure, int length)
{
    Block placeHolder;
    TimeSignature* ts = measure->getTime();

    int cursor = m_ove->getIsVersion4() ? 10 : 8;
    if (!jump(cursor)) {
        return false;
    }

    // numerator
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int numerator = placeHolder.toUnsignedInt();

    cursor = m_ove->getIsVersion4() ? 11 : 9;
    if ((length - cursor) % 8 != 0 || (length - cursor) / 8 != (int)numerator) {
        return false;
    }

    for (unsigned int i =0; i < numerator; ++i) {
        // beat start unit
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        int beatStart = placeHolder.toUnsignedInt();

        // beat length unit
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        int beatLength = placeHolder.toUnsignedInt();

        if (!jump(2)) {
            return false;
        }

        // beat start tick
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        int beatStartTick = placeHolder.toUnsignedInt();

        ts->addBeat(beatStart, beatLength, beatStartTick);
    }

    ts->endAddBeat();

    return true;
}

bool BarsParse::parseBarlineParameters(Measure* measure, int /* length */)
{
    Block placeHolder;

    int cursor = m_ove->getIsVersion4() ? 12 : 10;
    if (!jump(cursor)) {
        return false;
    }

    // repeat count
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    int repeatCount = placeHolder.toUnsignedInt();

    measure->setBackwardRepeatCount(repeatCount);

    if (!jump(6)) {
        return false;
    }

    return true;
}

bool BarsParse::parseNumericEndings(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    NumericEnding* numeric = new NumericEnding();
    measureData->addCrossMeasureElement(numeric, true);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(numeric)) {
        return false;
    }

    if (!jump(6)) {
        return false;
    }

    // measure count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    // int offsetMeasure = placeHolder.toUnsignedInt() - 1;
    int offsetMeasure = placeHolder.toUnsignedInt();
    numeric->stop()->setMeasure(offsetMeasure);

    if (!jump(2)) {
        return false;
    }

    // left x offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    numeric->getLeftShoulder()->setXOffset(placeHolder.toInt());

    // height
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    numeric->setHeight(placeHolder.toUnsignedInt());

    // left x offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    numeric->getRightShoulder()->setXOffset(placeHolder.toInt());

    if (!jump(2)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    numeric->getLeftShoulder()->setYOffset(placeHolder.toInt());
    numeric->getRightShoulder()->setYOffset(placeHolder.toInt());

    // number offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    numeric->getNumericHandle()->setXOffset(placeHolder.toInt());
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    numeric->getNumericHandle()->setYOffset(placeHolder.toInt());

    if (!jump(6)) {
        return false;
    }

    // text size
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int size = placeHolder.toUnsignedInt();

    // text: size maybe a huge value
    if (!readBuffer(placeHolder, size)) {
        return false;
    }
    numeric->setText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    // fix for wedding march.ove
    if (size % 2 == 0) {
        if (!jump(1)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseTempo(MeasureData* measureData, int /* length */)
{
    Block placeHolder;
    unsigned int thisByte;

    Tempo* tempo = new Tempo();
    measureData->addMusicData(tempo);
    if (!jump(3)) {
        return false;
    }
    // common
    if (!parseCommonBlock(tempo)) {
        return false;
    }
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    thisByte = placeHolder.toUnsignedInt();
    // show tempo
    tempo->setShowMark((getHighNibble(thisByte) & 0x4) == 0x4);
    // show before text
    tempo->setShowBeforeText((getHighNibble(thisByte) & 0x8) == 0x8);
    // show parenthesis
    tempo->setShowParenthesis((getHighNibble(thisByte) & 0x1) == 0x1);
    // left note type
    tempo->setLeftNoteType(getLowNibble(thisByte));
    // left note dot
    tempo->setLeftNoteDot((getHighNibble(thisByte) & 0x2) == 0x2);
    if (!jump(1)) { // dimension of the note symbol
        return false;
    }
    if (m_ove->getIsVersion4()) {
        if (!jump(2)) {
            return false;
        }
        // tempo
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        tempo->setTypeTempo(((double)placeHolder.toUnsignedInt()) / 100.0);
    } else {
        // tempo
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        tempo->setTypeTempo((double)placeHolder.toUnsignedInt());
        if (!jump(2)) {
            return false;
        }
    }
    // offset
    if (!parseOffsetElement(tempo)) {
        return false;
    }
    if (!jump(16)) {
        return false;
    }
    // 31 bytes left text
    if (!readBuffer(placeHolder, 31)) {
        return false;
    }
    tempo->setLeftText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    thisByte = placeHolder.toUnsignedInt();
    // swing eighth
    tempo->setSwingEighth((getHighNibble(thisByte) & 0x4) == 0x4);
    // right note dot
    tempo->setRightNoteDot((getHighNibble(thisByte) & 0x1) == 0x1);
    // compatibility with v3 files ?
    tempo->setRightSideType(static_cast<int>(getHighNibble(thisByte) & 0x2));
    // right note type
    tempo->setRightNoteType(getLowNibble(thisByte));
    // right text
    if (m_ove->getIsVersion4()) {
        if (!readBuffer(placeHolder, 31)) {
            return false;
        }
        tempo->setRightText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        // 00 -> float; 03 -> integer(floor); 01 -> notetype; 02 -> text
        tempo->setRightSideType(placeHolder.toInt());
    }

    return true;
}

bool BarsParse::parseBarNumber(Measure* measure, int /* length */)
{
    Block placeHolder;

    BarNumber* barNumber = measure->getBarNumber();

    if (!jump(2)) {
        return false;
    }

    // show on paragraph start
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    barNumber->setShowOnParagraphStart(getLowNibble(placeHolder.toUnsignedInt()) == 8);

    unsigned int blankSize = m_ove->getIsVersion4() ? 9 : 7;
    if (!jump(blankSize)) {
        return false;
    }

    // text align
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    barNumber->setAlign(placeHolder.toUnsignedInt());

    if (!jump(4)) {
        return false;
    }

    // show flag
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    barNumber->setShowFlag(placeHolder.toUnsignedInt());

    if (!jump(10)) {
        return false;
    }

    // bar range
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    barNumber->setShowEveryBarCount(placeHolder.toUnsignedInt());

    // prefix
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    barNumber->setPrefix(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    if (!jump(18)) {
        return false;
    }

    return true;
}

bool BarsParse::parseText(MeasureData* measureData, int length)
{
    Block placeHolder;

    Text* text = new Text();
    measureData->addMusicData(text);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(text)) {
        return false;
    }

    // type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int thisByte = placeHolder.toUnsignedInt();
    bool includeLineBreak = ((getHighNibble(thisByte) & 0x2) != 0x2);
    unsigned int id = getLowNibble(thisByte);
    Text::Type textType = Text::Type::Rehearsal;

    if (id == 0) {
        textType = Text::Type::MeasureText;
    } else if (id == 1) {
        textType = Text::Type::SystemText;
    } else {       // id ==2
        textType = Text::Type::Rehearsal;
    }

    text->setTextType(textType);

    if (!jump(1)) {
        return false;
    }

    // x offset
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    text->setXOffset(placeHolder.toInt());

    // y offset
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    text->setYOffset(placeHolder.toInt());

    // width
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    text->setWidth(placeHolder.toUnsignedInt());

    // height
    if (!readBuffer(placeHolder, 4)) {
        return false;
    }
    text->setHeight(placeHolder.toUnsignedInt());

    if (!jump(7)) {
        return false;
    }

    // horizontal margin
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    text->setHorizontalMargin(placeHolder.toUnsignedInt());

    if (!jump(1)) {
        return false;
    }

    // vertical margin
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    text->setVerticalMargin(placeHolder.toUnsignedInt());

    if (!jump(1)) {
        return false;
    }

    // line thick
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    text->setLineThick(placeHolder.toUnsignedInt());

    if (!jump(2)) {
        return false;
    }

    // text size
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    unsigned int size = placeHolder.toUnsignedInt();

    // text string, maybe huge
    if (!readBuffer(placeHolder, size)) {
        return false;
    }
    text->setText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    if (!includeLineBreak) {
        if (!jump(6)) {
            return false;
        }
    } else {
        unsigned int cursor = m_ove->getIsVersion4() ? 43 : 41;
        cursor += size;

        // multi lines of text
        for (unsigned int i = 0; i < 2; ++i) {
            if ((int)cursor < length) {
                // line parameters count
                if (!readBuffer(placeHolder, 2)) {
                    return false;
                }
                unsigned int lineCount = placeHolder.toUnsignedInt();

                if (i == 0 && int(cursor + 2 + 8 * lineCount) > length) {
                    return false;
                }

                if (i == 1 && int(cursor + 2 + 8 * lineCount) != length) {
                    return false;
                }

                if (!jump(8 * lineCount)) {
                    return false;
                }

                cursor += 2 + 8 * lineCount;
            }
        }
    }

    return true;
}

bool BarsParse::parseRepeatSymbol(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    RepeatSymbol* repeat = new RepeatSymbol();
    measureData->addMusicData(repeat);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(repeat)) {
        return false;
    }

    // RepeatType
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    repeat->setRepeatType(placeHolder.toUnsignedInt());

    if (!jump(13)) {
        return false;
    }

    // offset
    if (!parseOffsetElement(repeat)) {
        return false;
    }

    if (!jump(15)) {
        return false;
    }

    // size
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    unsigned int size = placeHolder.toUnsignedInt();

    // text, maybe huge
    if (!readBuffer(placeHolder, size)) {
        return false;
    }
    repeat->setText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

    // last 0
    if (size % 2 == 0) {
        if (!jump(1)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseBdat(Measure* /* measure */, MeasureData* measureData, SizeChunk* chunk)
{
    Block placeHolder;
    StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

    m_handle = &handle;

    // parse here
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    unsigned int cnt = placeHolder.toUnsignedInt();

    for (unsigned int i = 0; i < cnt; ++i) {
        // 0x0028 or 0x0016 or 0x002C
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        unsigned int count = placeHolder.toUnsignedInt() - 7;

        // type id
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        unsigned int thisByte = placeHolder.toUnsignedInt();
        BdatType type;

        if (!getBdatElementType(thisByte, type)) {
            return false;
        }

        switch (type) {
        case BdatType::Raw_Note:
        case BdatType::Rest:
        case BdatType::Note: {
            if (!parseNoteRest(measureData, count, type)) {
                return false;
            }
            break;
        }
        case BdatType::Beam: {
            if (!parseBeam(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Harmony: {
            if (!parseHarmony(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Clef: {
            if (!parseClef(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Dynamics: {
            if (!parseDynamics(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Wedge: {
            if (!parseWedge(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Glissando: {
            if (!parseGlissando(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Decorator: {
            if (!parseDecorators(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Key: {
            if (!parseKey(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Lyric: {
            if (!parseLyric(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Octave_Shift: {
            if (!parseOctaveShift(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Slur: {
            if (!parseSlur(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Text: {
            if (!parseText(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Tie: {
            if (!parseTie(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Tuplet: {
            if (!parseTuplet(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Guitar_Bend:
        case BdatType::Guitar_Barre: {
            if (!parseSizeBlock(count)) {
                return false;
            }
            break;
        }
        case BdatType::Pedal: {
            if (!parsePedal(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::KuoHao: {
            if (!parseKuohao(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Expressions: {
            if (!parseExpressions(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Harp_Pedal: {
            if (!parseHarpPedal(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Multi_Measure_Rest: {
            if (!parseMultiMeasureRest(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Harmony_GuitarFrame: {
            if (!parseHarmonyGuitarFrame(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Graphics_40:
        case BdatType::Graphics_RoundRect:
        case BdatType::Graphics_Rect:
        case BdatType::Graphics_Round:
        case BdatType::Graphics_Line:
        case BdatType::Graphics_Curve:
        case BdatType::Graphics_WedgeSymbol: {
            if (!parseSizeBlock(count)) {
                return false;
            }
            break;
        }
        case BdatType::Midi_Controller: {
            if (!parseMidiController(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Midi_Program_Change: {
            if (!parseMidiProgramChange(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Midi_Channel_Pressure: {
            if (!parseMidiChannelPressure(measureData, count)) {
                return false;
            }
            break;
        }
        case BdatType::Midi_Pitch_Wheel: {
            if (!parseMidiPitchWheel(measureData, count)) {
                return false;
            }
            break;
        }
        default: {
            if (!jump(count)) {
                return false;
            }
            break;
        }
        }

        // if i == count - 1 then is bar end place holder
    }

    m_handle = NULL;

    return true;
}

int getInt(int byte, int bits)
{
    int num = 0;

    if (bits > 0) {
        // same as int(pow(2, bits - 1))
        int factor = 1 << (bits - 1);
        num = (byte % (factor * 2));

        if ((byte & factor) == factor) {
            num -= factor * 2;
        }
    }

    return num;
}

bool BarsParse::parseNoteRest(MeasureData* measureData, int length, BdatType type)
{
    NoteContainer* container = new NoteContainer();
    Block placeHolder;
    unsigned int thisByte;

    measureData->addNoteContainer(container);
    measureData->addMusicData(container);

    // note|rest & grace
    container->setIsRest(type == BdatType::Rest);
    container->setIsRaw(type == BdatType::Raw_Note);

    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    thisByte = placeHolder.toUnsignedInt();
    container->setIsGrace(thisByte == 0x3C00);
    container->setIsCue(thisByte == 0x4B40 || thisByte == 0x3240);

    // show / hide
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    thisByte = placeHolder.toUnsignedInt();
    container->setShow(getLowNibble(thisByte) != 0x8);

    // voice
    container->setVoice(getLowNibble(thisByte) & 0x7);

    // common
    if (!parseCommonBlock(container)) {
        return false;
    }

    // tuplet
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    container->setTuplet(placeHolder.toUnsignedInt());

    // space
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    container->setSpace(placeHolder.toUnsignedInt());

    // in beam
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    thisByte = placeHolder.toUnsignedInt();
    bool inBeam = (getHighNibble(thisByte) & 0x1) == 0x1;
    container->setInBeam(inBeam);

    // grace NoteType
    container->setGraceNoteType((NoteType)getHighNibble(thisByte));

    // dot
    container->setDot(getLowNibble(thisByte) & 0x03);

    // NoteType
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    thisByte = placeHolder.toUnsignedInt();
    container->setNoteType((NoteType)getLowNibble(thisByte));

    int cursor = 0;

    if (type == BdatType::Rest) {
        Note* restPtr = new Note();
        container->addNoteRest(restPtr);
        restPtr->setIsRest(true);

        // line
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        restPtr->setLine(placeHolder.toInt());

        if (!jump(1)) {
            return false;
        }

        cursor = m_ove->getIsVersion4() ? 16 : 14;
    } else { // type == Bdat_Note || type == Bdat_Raw_Note
        // stem up 0x80, stem down 0x00
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        thisByte = placeHolder.toUnsignedInt();
        container->setStemUp((getHighNibble(thisByte) & 0x8) == 0x8);

        // stem length
        int stemOffset = thisByte % 0x80;
        container->setStemLength(getInt(stemOffset, 7) + 7 /* 3.5 line span */);

        // show stem 0x00, hide stem 0x40
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        bool hideStem = getHighNibble(thisByte) == 0x4;
        container->setShowStem(!hideStem);

        if (!jump(1)) {
            return false;
        }

        // note count
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        unsigned int noteCount = placeHolder.toUnsignedInt();
        unsigned int i;

        // each note 16 bytes
        for (i = 0; i < noteCount; ++i) {
            Note* notePtr = new Note();
            notePtr->setIsRest(false);

            container->addNoteRest(notePtr);

            // note show / hide
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            thisByte = placeHolder.toUnsignedInt();
            notePtr->setShow((thisByte & 0x80) != 0x80);

            // notehead type
            notePtr->setHeadType(thisByte & 0x7f);

            // tie pos
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            thisByte = placeHolder.toUnsignedInt();
            notePtr->setTiePos(getHighNibble(thisByte));

            // offset staff, in { -1, 0, 1 }
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            thisByte = getLowNibble(placeHolder.toUnsignedInt());
            int offsetStaff = 0;
            if (thisByte == 1) {
                offsetStaff = 1;
            }
            if (thisByte == 7) {
                offsetStaff = -1;
            }
            notePtr->setOffsetStaff(offsetStaff);

            // accidental
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            thisByte = placeHolder.toUnsignedInt();
            notePtr->setAccidental(getLowNibble(thisByte));
            // accidental 0: influenced by key, 4: influenced by previous accidental in measure
            // bool notShow = (getHighNibble(thisByte) == 0) || (getHighNibble(thisByte) == 4);
            bool notShow = !(getHighNibble(thisByte) & 0x1);
            notePtr->setShowAccidental(!notShow);

            if (!jump(1)) {
                return false;
            }

            // line
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            notePtr->setLine(placeHolder.toInt());

            if (!jump(1)) {
                return false;
            }

            // note
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            unsigned int note = placeHolder.toUnsignedInt();
            notePtr->setNote(note);

            // note on velocity
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            unsigned int onVelocity = placeHolder.toUnsignedInt();
            notePtr->setOnVelocity(onVelocity);

            // note off velocity
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            unsigned int offVelocity = placeHolder.toUnsignedInt();
            notePtr->setOffVelocity(offVelocity);

            if (!jump(2)) {
                return false;
            }

            // length (tick)
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            container->setLength(placeHolder.toUnsignedInt());

            // offset tick
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            notePtr->setOffsetTick(placeHolder.toInt());
        }

        cursor = m_ove->getIsVersion4() ? 18 : 16;
        cursor += noteCount * 16 /* note size */;
    }

    // articulation
    while (cursor < length + 1 /* 0x70 || 0x80 || 0x90 */) {
        Articulation* art = new Articulation();
        container->addArticulation(art);

        // block size
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        int blockSize = placeHolder.toUnsignedInt();

        // articulation type
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        art->setArtType(placeHolder.toUnsignedInt());

        // placement
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        art->setPlacementAbove(placeHolder.toUnsignedInt() != 0x00); // 0x00: below, 0x30: above

        // offset
        if (!parseOffsetElement(art)) {
            return false;
        }

        if (!m_ove->getIsVersion4()) {
            if (blockSize - 8 > 0) {
                if (!jump(blockSize - 8)) {
                    return false;
                }
            }
        } else {
            // setting
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            thisByte = placeHolder.toUnsignedInt();
            const bool changeSoundEffect = ((thisByte & 0x1) == 0x1);
            const bool changeLength = ((thisByte & 0x2) == 0x2);
            const bool changeVelocity = ((thisByte & 0x4) == 0x4);
            // const bool changeExtraLength = ((thisByte & 0x20) == 0x20);

            if (!jump(8)) {
                return false;
            }

            // velocity type
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            thisByte = placeHolder.toUnsignedInt();
            if (changeVelocity) {
                art->setVelocityType((Articulation::VelocityType)thisByte);
            }

            if (!jump(14)) {
                return false;
            }

            // sound effect
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            int from = placeHolder.toInt();
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            int to = placeHolder.toInt();
            if (changeSoundEffect) {
                art->setSoundEffect(from, to);
            }

            if (!jump(1)) {
                return false;
            }

            // length percentage
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            if (changeLength) {
                art->setLengthPercentage(placeHolder.toUnsignedInt());
            }

            // velocity
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            if (changeVelocity) {
                art->setVelocityValue(placeHolder.toInt());
            }

            if (Articulation::isTrill(art->getArtType())) {
                if (!jump(8)) {
                    return false;
                }

                // trill note length
                if (!readBuffer(placeHolder, 1)) {
                    return false;
                }
                art->setTrillNoteLength(placeHolder.toUnsignedInt());

                // trill rate
                if (!readBuffer(placeHolder, 1)) {
                    return false;
                }
                thisByte = placeHolder.toUnsignedInt();
                NoteType trillNoteType = NoteType::Note_Sixteen;
                switch (getHighNibble(thisByte)) {
                case 0:
                    trillNoteType = NoteType::Note_None;
                    break;
                case 1:
                    trillNoteType = NoteType::Note_Sixteen;
                    break;
                case 2:
                    trillNoteType = NoteType::Note_32;
                    break;
                case 3:
                    trillNoteType = NoteType::Note_64;
                    break;
                case 4:
                    trillNoteType = NoteType::Note_128;
                    break;
                default:
                    break;
                }
                art->setTrillRate(trillNoteType);

                // accelerate type
                art->setAccelerateType(thisByte & 0xf);

                if (!jump(1)) {
                    return false;
                }

                // auxiliary first
                if (!readBuffer(placeHolder, 1)) {
                    return false;
                }
                art->setAuxiliaryFirst(placeHolder.toBoolean());

                if (!jump(1)) {
                    return false;
                }

                // trill interval
                if (!readBuffer(placeHolder, 1)) {
                    return false;
                }
                art->setTrillInterval(placeHolder.toUnsignedInt());
            } else {
                if (blockSize > 40) {
                    if (!jump(blockSize - 40)) {
                        return false;
                    }
                }
            }
        }

        cursor += blockSize;
    }

    return true;
}

int tupletToSpace(int tuplet)
{
    int a(1);

    while (a * 2 < tuplet) {
        a *= 2;
    }

    return a;
}

bool BarsParse::parseBeam(MeasureData* measureData, int length)
{
    int i;
    Block placeHolder;

    Beam* beam = new Beam();
    measureData->addCrossMeasureElement(beam, true);

    // maybe create tuplet, for < quarter & tool 3(
    bool createTuplet = false;
    int maxEndUnit = 0;
    Tuplet* tuplet = new Tuplet();

    // is grace
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    beam->setIsGrace(placeHolder.toBoolean());

    if (!jump(1)) {
        return false;
    }

    // voice
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    beam->setVoice(getLowNibble(placeHolder.toUnsignedInt()) & 0x7);

    // common
    if (!parseCommonBlock(beam)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // beam count
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    int beamCount = placeHolder.toUnsignedInt();

    if (!jump(1)) {
        return false;
    }

    // left line
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    beam->getLeftLine()->setLine(placeHolder.toInt());

    // right line
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    beam->getRightLine()->setLine(placeHolder.toInt());

    if (m_ove->getIsVersion4()) {
        if (!jump(8)) {
            return false;
        }
    }

    int currentCursor = m_ove->getIsVersion4() ? 23 : 13;
    int count = (length - currentCursor) / 16;

    if (count != beamCount) {
        return false;
    }

    for (i = 0; i < count; ++i) {
        if (!jump(1)) {
            return false;
        }

        // tuplet
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        int tupletCount = placeHolder.toUnsignedInt();
        if (tupletCount > 0) {
            createTuplet = true;
            tuplet->setTuplet(tupletCount);
            tuplet->setSpace(tupletToSpace(tupletCount));
        }

        // start / stop measure
        // line i start end position
        MeasurePos startMp;
        MeasurePos stopMp;

        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        startMp.setMeasure(placeHolder.toUnsignedInt());
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        stopMp.setMeasure(placeHolder.toUnsignedInt());

        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        startMp.setOffset(placeHolder.toInt());
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        stopMp.setOffset(placeHolder.toInt());

        beam->addLine(startMp, stopMp);

        if (stopMp.getOffset() > maxEndUnit) {
            maxEndUnit = stopMp.getOffset();
        }

        if (i == 0) {
            if (!jump(4)) {
                return false;
            }

            // left offset up + 4, down - 4
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            beam->getLeftShoulder()->setYOffset(placeHolder.toInt());

            // right offset up + 4, down - 4
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            beam->getRightShoulder()->setYOffset(placeHolder.toInt());
        } else {
            if (!jump(8)) {
                return false;
            }
        }
    }

    const QList<QPair<MeasurePos, MeasurePos> > lines = beam->getLines();
    MeasurePos offsetMp;

    for (i = 0; i < lines.size(); ++i) {
        if (lines[i].second > offsetMp) {
            offsetMp = lines[i].second;
        }
    }

    beam->stop()->setMeasure(offsetMp.getMeasure());
    beam->stop()->setOffset(offsetMp.getOffset());

    // a case that Tuplet block don't exist, and hide inside beam
    if (createTuplet) {
        tuplet->copyCommonBlock(*beam);
        tuplet->getLeftLine()->setLine(beam->getLeftLine()->getLine());
        tuplet->getRightLine()->setLine(beam->getRightLine()->getLine());
        tuplet->stop()->setMeasure(beam->stop()->getMeasure());
        tuplet->stop()->setOffset(maxEndUnit);

        measureData->addCrossMeasureElement(tuplet, true);
    } else {
        delete tuplet;
    }

    return true;
}

bool BarsParse::parseTie(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Tie* tie = new Tie();
    measureData->addCrossMeasureElement(tie, true);

    if (!jump(3)) {
        return false;
    }

    // start common
    if (!parseCommonBlock(tie)) {
        return false;
    }

    if (!jump(1)) {
        return false;
    }

    // note
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    tie->setNote(placeHolder.toUnsignedInt());

    // pair lines
    if (!parsePairLinesBlock(tie)) {
        return false;
    }

    // offset common
    if (!parseOffsetCommonBlock(tie)) {
        return false;
    }

    // left shoulder offset
    if (!parseOffsetElement(tie->getLeftShoulder())) {
        return false;
    }

    // right shoulder offset
    if (!parseOffsetElement(tie->getRightShoulder())) {
        return false;
    }

    // height
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    tie->setHeight(placeHolder.toUnsignedInt());

    return true;
}

bool BarsParse::parseTuplet(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Tuplet* tuplet = new Tuplet();
    measureData->addCrossMeasureElement(tuplet, true);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(tuplet)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // pair lines
    if (!parsePairLinesBlock(tuplet)) {
        return false;
    }

    // offset common
    if (!parseOffsetCommonBlock(tuplet)) {
        return false;
    }

    // left shoulder offset
    if (!parseOffsetElement(tuplet->getLeftShoulder())) {
        return false;
    }

    // right shoulder offset
    if (!parseOffsetElement(tuplet->getRightShoulder())) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // height
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    tuplet->setHeight(placeHolder.toUnsignedInt());

    // tuplet
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    tuplet->setTuplet(placeHolder.toUnsignedInt());

    // space
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    tuplet->setSpace(placeHolder.toUnsignedInt());

    // mark offset
    if (!parseOffsetElement(tuplet->getMarkHandle())) {
        return false;
    }

    return true;
}

QString binaryToHarmonyType(int bin)
{
    QString type = "";
    switch (bin) {
    case 0x0005: {
        type = "add9(no3)";
        break;
    }
    case 0x0009: {
        type = "min(no5)";
        break;
    }
    case 0x0011: {
        type = "(no5)";
        break;
    }
    case 0x0021: {
        type = "sus(no5)";
        break;
    }
    case 0x0025: {
        type = "24";
        break;
    }
    case 0x0029: {
        type = "min4(no5)";
        break;
    }
    case 0x0049: {
        type = "dim";
        break;
    }
    case 0x0051: {
        type = "(b5)";
        break;
    }
    case 0x0055: {
        type = "2#4(no5)";
        break;
    }
    case 0x0081: {
        type = "(no3)";
        break;
    }
    case 0x0085: {
        type = "2";
        break;
    }
    case 0x0089: {
        type = "min";
        break;
    }
    case 0x008D: {
        type = "min(add9)";
        break;
    }
    case 0x0091: {
        type = "";
        break;
    }
    case 0x0093: {
        type = "addb9";
        break;
    }
    case 0x0095: {
        type = "add9";
        break;
    }
    case 0x00A1: {
        type = "sus4";
        break;
    }
    case 0x00A5: {
        type = "sus(add9)";
        break;
    }
    case 0x00A9: {
        type = "min4";
        break;
    }
    case 0x00D5: {
        type = "2#4";
        break;
    }
    case 0x0111: {
        type = "aug";
        break;
    }
    case 0x0115: {
        type = "aug(add9)";
        break;
    }
    case 0x0151: {
        type = "(b5b6)";
        break;
    }
    case 0x0155: {
        type = "+add9#11";
        break;
    }
    case 0x0189: {
        type = "minb6";
        break;
    }
    case 0x018D: {
        type = "min2b6";
        break;
    }
    case 0x0191: {
        type = "(b6)";
        break;
    }
    case 0x0199: {
        type = "(add9)b6";
        break;
    }
    case 0x0205: {
        type = "26";
        break;
    }
    case 0x020D: {
        type = "min69";
        break;
    }
    case 0x0211: {
        type = "6";
        break;
    }
    case 0x0215: {
        type = "69";
        break;
    }
    case 0x022D: {
        type = "min69 11";
        break;
    }
    case 0x0249: {
        type = "dim7";
        break;
    }
    case 0x0251: {
        type = "6#11";
        break;
    }
    case 0x0255: {
        type = "13#11";
        break;
    }
    case 0x0281: {
        type = "6(no3)";
        break;
    }
    case 0x0285: {
        type = "69(no3)";
        break;
    }
    case 0x0289: {
        type = "min6";
        break;
    }
    case 0x028D: {
        type = "min69";
        break;
    }
    case 0x0291: {
        type = "6";
        break;
    }
    case 0x0293: {
        type = "6b9";
        break;
    }
    case 0x0295: {
        type = "69";
        break;
    }
    case 0x02AD: {
        type = "min69 11";
        break;
    }
    case 0x02C5: {
        type = "69#11(no3)";
        break;
    }
    case 0x02D5: {
        type = "69#11";
        break;
    }
    case 0x040D: {
        type = "min9(no5)";
        break;
    }
    case 0x0411: {
        type = "7(no5)";
        break;
    }
    case 0x0413: {
        type = "7b9";
        break;
    }
    case 0x0415: {
        type = "9";
        break;
    }
    case 0x0419: {
        type = "7#9";
        break;
    }
    case 0x041B: {
        type = "7b9#9";
        break;
    }
    case 0x0421: {
        type = "sus7";
        break;
    }
    case 0x0429: {
        type = "min11";
        break;
    }
    case 0x042D: {
        type = "min11";
        break;
    }
    case 0x0445: {
        type = "9b5(no3)";
        break;
    }
    case 0x0449: {
        type = "min7b5";
        break;
    }
    case 0x044D: {
        type = "min9b5";
        break;
    }
    case 0x0451: {
        type = "7b5";
        break;
    }
    case 0x0453: {
        type = "7b9b5";
        break;
    }
    case 0x0455: {
        type = "9b5";
        break;
    }
    case 0x045B: {
        type = "7b5b9#9";
        break;
    }
    case 0x0461: {
        type = "sus7b5";
        break;
    }
    case 0x0465: {
        type = "sus9b5";
        break;
    }
    case 0x0469: {
        type = "min11b5";
        break;
    }
    case 0x046D: {
        type = "min11b5";
        break;
    }
    case 0x0481: {
        type = "7(no3)";
        break;
    }
    case 0x0489: {
        type = "min7";
        break;
    }
    case 0x048D: {
        type = "min9";
        break;
    }
    case 0x0491: {
        type = "7";
        break;
    }
    case 0x0493: {
        type = "7b9";
        break;
    }
    case 0x0495: {
        type = "9";
        break;
    }
    case 0x0499: {
        type = "7#9";
        break;
    }
    case 0x049B: {
        type = "7b9#9";
        break;
    }
    case 0x04A1: {
        type = "sus7";
        break;
    }
    case 0x04A5: {
        type = "sus9";
        break;
    }
    case 0x04A9: {
        type = "min11";
        break;
    }
    case 0x04AD: {
        type = "min11";
        break;
    }
    case 0x04B5: {
        type = "11";
        break;
    }
    case 0x04D5: {
        type = "9#11";
        break;
    }
    case 0x0509: {
        type = "min7#5";
        break;
    }
    case 0x0511: {
        type = "aug7";
        break;
    }
    case 0x0513: {
        type = "7#5b9";
        break;
    }
    case 0x0515: {
        type = "aug9";
        break;
    }
    case 0x0519: {
        type = "7#5#9";
        break;
    }
    case 0x0529: {
        type = "min11b13";
        break;
    }
    case 0x0533: {
        type = "11b9#5";
        break;
    }
    case 0x0551: {
        type = "aug7#11";
        break;
    }
    case 0x0553: {
        type = "7b5b9b13";
        break;
    }
    case 0x0555: {
        type = "aug9#11";
        break;
    }
    case 0x0559: {
        type = "aug7#9#11";
        break;
    }
    case 0x0609: {
        type = "min13";
        break;
    }
    case 0x0611: {
        type = "13";
        break;
    }
    case 0x0613: {
        type = "13b9";
        break;
    }
    case 0x0615: {
        type = "13";
        break;
    }
    case 0x0619: {
        type = "13#9";
        break;
    }
    case 0x061B: {
        type = "13b9#9";
        break;
    }
    case 0x0621: {
        type = "sus13";
        break;
    }
    case 0x062D: {
        type = "min13(11)";
        break;
    }
    case 0x0633: {
        type = "13b9add4";
        break;
    }
    case 0x0635: {
        type = "13";
        break;
    }
    case 0x0645: {
        type = "13#11(no3)";
        break;
    }
    case 0x0651: {
        type = "13b5";
        break;
    }
    case 0x0653: {
        type = "13b9#11";
        break;
    }
    case 0x0655: {
        type = "13#11";
        break;
    }
    case 0x0659: {
        type = "13#9b5";
        break;
    }
    case 0x065B: {
        type = "13b5b9#9";
        break;
    }
    case 0x0685: {
        type = "13(no3)";
        break;
    }
    case 0x068D: {
        type = "min13";
        break;
    }
    case 0x0691: {
        type = "13";
        break;
    }
    case 0x0693: {
        type = "13b9";
        break;
    }
    case 0x0695: {
        type = "13";
        break;
    }
    case 0x0699: {
        type = "13#9";
        break;
    }
    case 0x06A5: {
        type = "sus13";
        break;
    }
    case 0x06AD: {
        type = "min13(11)";
        break;
    }
    case 0x06B5: {
        type = "13";
        break;
    }
    case 0x06D5: {
        type = "13#11";
        break;
    }
    case 0x0813: {
        type = "maj7b9";
        break;
    }
    case 0x0851: {
        type = "maj7#11";
        break;
    }
    case 0x0855: {
        type = "maj9#11";
        break;
    }
    case 0x0881: {
        type = "maj7(no3)";
        break;
    }
    case 0x0889: {
        type = "min(\u266e7)";
        break;
    } // "min(<sym>accidentalNatural</sym>7)"
    case 0x088D: {
        type = "min9(\u266e7)";
        break;
    } // "min9(<sym>accidentalNatural</sym>7)"
    case 0x0891: {
        type = "maj7";
        break;
    }
    case 0x0895: {
        type = "maj9";
        break;
    }
    case 0x08C9: {
        type = "dim7(add maj 7)";
        break;
    }
    case 0x08D1: {
        type = "maj7#11";
        break;
    }
    case 0x08D5: {
        type = "maj9#11";
        break;
    }
    case 0x0911: {
        type = "maj7#5";
        break;
    }
    case 0x0991: {
        type = "maj7#5";
        break;
    }
    case 0x0995: {
        type = "maj9#5";
        break;
    }
    case 0x0A0D: {
        type = "min69(\u266e7)";
        break;
    } // "min69(<sym>accidentalNatural</sym>7)"
    case 0x0A11: {
        type = "maj13";
        break;
    }
    case 0x0A15: {
        type = "maj13";
        break;
    }
    case 0x0A51: {
        type = "maj13#11";
        break;
    }
    case 0x0A55: {
        type = "maj13#11";
        break;
    }
    case 0x0A85: {
        type = "maj13(no3)";
        break;
    }
    case 0x0A89: {
        type = "min13(\u266e7)";
        break;
    } // "min13(<sym>accidentalNatural</sym>7)"
    case 0x0A8D: {
        type = "min69(\u266e7)";
        break;
    } // "min69(<sym>accidentalNatural</sym>7)"
    case 0x0A91: {
        type = "maj13";
        break;
    }
    case 0x0A95: {
        type = "maj13";
        break;
    }
    case 0x0AAD: {
        type = "min13(\u266e7)";
        break;
    } // "min13(<sym>accidentalNatural</sym>7)"
    case 0x0AD5: {
        type = "maj13#11";
        break;
    }
    case 0x0B45: {
        type = "maj13#5#11(no4)";
        break;
    }
    default: {
        LOGD("Unrecognized harmony type: %04X", bin);
        type = "";
        break;
    }
    }
    return type;
}

bool BarsParse::parseHarmony(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Harmony* harmony = new Harmony();
    measureData->addMusicData(harmony);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(harmony)) {
        return false;
    }

    // bass on bottom
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    harmony->setBassOnBottom((getHighNibble(placeHolder.toUnsignedInt()) & 0x4));

    // root alteration
    switch (placeHolder.toUnsignedInt() & 0x18) {
    case 0: {
        harmony->setAlterRoot(0); // natural
        break;
    }
    case 16: {
        harmony->setAlterRoot(-1); // flat
        break;
    }
    case 8: {
        harmony->setAlterRoot(1); // sharp
        break;
    }
    default: {
        harmony->setAlterRoot(0);
        break;
    }
    }

    // bass alteration
    switch (placeHolder.toUnsignedInt() & 0x3) {
    case 0: {
        harmony->setAlterBass(0); // natural
        break;
    }
    case 2: {
        harmony->setAlterBass(-1); // flat
        break;
    }
    case 1: {
        harmony->setAlterBass(1); // sharp
        break;
    }
    default: {
        harmony->setAlterBass(0);
        break;
    }
    }

    // show bass
    bool useBass = placeHolder.toUnsignedInt() & 0x80;

    if (!jump(1)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    harmony->setYOffset(placeHolder.toInt());

    // harmony type
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    harmony->setHarmonyType(binaryToHarmonyType(placeHolder.toUnsignedInt()));

    // root
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    harmony->setRoot(placeHolder.toInt());

    // bass
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    if (useBass) {
        harmony->setBass(placeHolder.toInt());
    }

    // angle
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    harmony->setAngle(placeHolder.toInt());

    if (m_ove->getIsVersion4()) {
        // length (tick)
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        harmony->setLength(placeHolder.toUnsignedInt());

        if (!jump(4)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseClef(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Clef* clef = new Clef();
    measureData->addMusicData(clef);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(clef)) {
        return false;
    }

    // clef type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    clef->setClefType(placeHolder.toUnsignedInt());

    // line
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    clef->setLine(placeHolder.toInt());

    if (!jump(2)) {
        return false;
    }

    return true;
}

bool BarsParse::parseLyric(MeasureData* measureData, int length)
{
    Block placeHolder;

    Lyric* lyric = new Lyric();
    measureData->addMusicData(lyric);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(lyric)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // offset
    if (!parseOffsetElement(lyric)) {
        return false;
    }

    if (!jump(7)) {
        return false;
    }

    // verse
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    lyric->setVerse(placeHolder.toUnsignedInt());

    if (m_ove->getIsVersion4()) {
        if (!jump(6)) {
            return false;
        }

        // lyric
        if (length > 29) {
            if (!readBuffer(placeHolder, length - 29)) {
                return false;
            }
            lyric->setLyric(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
        }
    }

    return true;
}

bool BarsParse::parseSlur(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Slur* slur = new Slur();
    measureData->addCrossMeasureElement(slur, true);

    if (!jump(2)) {
        return false;
    }

    // voice
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    slur->setVoice(getLowNibble(placeHolder.toUnsignedInt()) & 0x7);

    // common
    if (!parseCommonBlock(slur)) {
        return false;
    }

    // show on top
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    slur->setShowOnTop(getHighNibble(placeHolder.toUnsignedInt()) == 0x8);

    if (!jump(1)) {
        return false;
    }

    // pair lines
    if (!parsePairLinesBlock(slur)) {
        return false;
    }

    // offset common
    if (!parseOffsetCommonBlock(slur)) {
        return false;
    }

    // handle 1
    if (!parseOffsetElement(slur->getLeftShoulder())) {
        return false;
    }

    // handle 4
    if (!parseOffsetElement(slur->getRightShoulder())) {
        return false;
    }

    // handle 2
    if (!parseOffsetElement(slur->getHandle2())) {
        return false;
    }

    // handle 3
    if (!parseOffsetElement(slur->getHandle3())) {
        return false;
    }

    if (m_ove->getIsVersion4()) {
        if (!jump(3)) {
            return false;
        }

        // note time percent
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        slur->setNoteTimePercent(placeHolder.toUnsignedInt());

        if (!jump(36)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseGlissando(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Glissando* glissando = new Glissando();
    measureData->addCrossMeasureElement(glissando, true);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(glissando)) {
        return false;
    }

    // straight or wavy
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int thisByte = placeHolder.toUnsignedInt();
    glissando->setStraightWavy(getHighNibble(thisByte) == 4);

    if (!jump(1)) {
        return false;
    }

    // pair lines
    if (!parsePairLinesBlock(glissando)) {
        return false;
    }

    // offset common
    if (!parseOffsetCommonBlock(glissando)) {
        return false;
    }

    // left shoulder
    if (!parseOffsetElement(glissando->getLeftShoulder())) {
        return false;
    }

    // right shoulder
    if (!parseOffsetElement(glissando->getRightShoulder())) {
        return false;
    }

    if (m_ove->getIsVersion4()) {
        if (!jump(1)) {
            return false;
        }

        // line thick
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        glissando->setLineThick(placeHolder.toUnsignedInt());

        if (!jump(12)) {
            return false;
        }

        // text 32 bytes
        if (!readBuffer(placeHolder, 32)) {
            return false;
        }
        glissando->setText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

        if (!jump(6)) {
            return false;
        }
    }

    return true;
}

bool getDecoratorType(
    unsigned int thisByte,
    bool& measureRepeat,
    Decorator::Type& decoratorType,
    bool& singleRepeat,
    ArticulationType& artType)
{
    measureRepeat = false;
    decoratorType = Decorator::Type::Articulation;
    singleRepeat = true;
    artType = ArticulationType::None;

    switch (thisByte) {
    case 0x00: {
        decoratorType = Decorator::Type::Dotted_Barline;
        break;
    }
    case 0x30: {
        artType = ArticulationType::Open_String;
        break;
    }
    case 0x31: {
        artType = ArticulationType::Finger_1;
        break;
    }
    case 0x32: {
        artType = ArticulationType::Finger_2;
        break;
    }
    case 0x33: {
        artType = ArticulationType::Finger_3;
        break;
    }
    case 0x34: {
        artType = ArticulationType::Finger_4;
        break;
    }
    case 0x35: {
        artType = ArticulationType::Finger_5;
        break;
    }
    case 0x6B: {
        artType = ArticulationType::Flat_Accidental_For_Trill;
        break;
    }
    case 0x6C: {
        artType = ArticulationType::Sharp_Accidental_For_Trill;
        break;
    }
    case 0x6D: {
        artType = ArticulationType::Natural_Accidental_For_Trill;
        break;
    }
    case 0x8d: {
        measureRepeat = true;
        singleRepeat = true;
        break;
    }
    case 0x8e: {
        measureRepeat = true;
        singleRepeat = false;
        break;
    }
    case 0xA0: {
        artType = ArticulationType::Minor_Trill;
        break;
    }
    case 0xA1: {
        artType = ArticulationType::Major_Trill;
        break;
    }
    case 0xA2: {
        artType = ArticulationType::Trill_Section;
        break;
    }
    case 0xA6: {
        artType = ArticulationType::Turn;
        break;
    }
    case 0xA8: {
        artType = ArticulationType::Tremolo_Eighth;
        break;
    }
    case 0xA9: {
        artType = ArticulationType::Tremolo_Sixteenth;
        break;
    }
    case 0xAA: {
        artType = ArticulationType::Tremolo_Thirty_Second;
        break;
    }
    case 0xAB: {
        artType = ArticulationType::Tremolo_Sixty_Fourth;
        break;
    }
    case 0xB2: {
        artType = ArticulationType::Fermata;
        break;
    }
    case 0xB3: {
        artType = ArticulationType::Fermata_Inverted;
        break;
    }
    case 0xB9: {
        artType = ArticulationType::Pause;
        break;
    }
    case 0xBA: {
        artType = ArticulationType::Grand_Pause;
        break;
    }
    case 0xC0: {
        artType = ArticulationType::Marcato;
        break;
    }
    case 0xC1: {
        artType = ArticulationType::Marcato_Dot;
        break;
    }
    case 0xC2: {
        artType = ArticulationType::SForzando;
        break;
    }
    case 0xC3: {
        artType = ArticulationType::SForzando_Dot;
        break;
    }
    case 0xC4: {
        artType = ArticulationType::SForzando_Inverted;
        break;
    }
    case 0xC5: {
        artType = ArticulationType::SForzando_Dot_Inverted;
        break;
    }
    case 0xC6: {
        artType = ArticulationType::Staccatissimo;
        break;
    }
    case 0xC7: {
        artType = ArticulationType::Staccato;
        break;
    }
    case 0xC8: {
        artType = ArticulationType::Tenuto;
        break;
    }
    case 0xC9: {
        artType = ArticulationType::Natural_Harmonic;
        break;
    }
    case 0xCA: {
        artType = ArticulationType::Artificial_Harmonic;
        break;
    }
    case 0xCB: {
        artType = ArticulationType::Plus_Sign;
        break;
    }
    case 0xCC: {
        artType = ArticulationType::Up_Bow;
        break;
    }
    case 0xCD: {
        artType = ArticulationType::Down_Bow;
        break;
    }
    case 0xCE: {
        artType = ArticulationType::Up_Bow_Inverted;
        break;
    }
    case 0xCF: {
        artType = ArticulationType::Down_Bow_Inverted;
        break;
    }
    case 0xD0: {
        artType = ArticulationType::Pedal_Down;
        break;
    }
    case 0xD1: {
        artType = ArticulationType::Pedal_Up;
        break;
    }
    case 0xD6: {
        artType = ArticulationType::Heavy_Attack;
        break;
    }
    case 0xD7: {
        artType = ArticulationType::Heavier_Attack;
        break;
    }
    default:
        return false;
        break;
    }

    return true;
}

bool BarsParse::parseDecorators(MeasureData* measureData, int length)
{
    Block placeHolder;
    MusicData* musicData = new MusicData();

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(musicData)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    musicData->setYOffset(placeHolder.toInt());

    if (!jump(2)) {
        return false;
    }

    // measure repeat | piano pedal | dotted barline | articulation
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int thisByte = placeHolder.toUnsignedInt();

    Decorator::Type decoratorType;
    bool isMeasureRepeat;
    bool isSingleRepeat = true;
    ArticulationType artType = ArticulationType::None;

    getDecoratorType(thisByte, isMeasureRepeat, decoratorType, isSingleRepeat, artType);

    if (isMeasureRepeat) {
        MeasureRepeat* measureRepeat = new MeasureRepeat();
        measureData->addCrossMeasureElement(measureRepeat, true);

        measureRepeat->copyCommonBlock(*musicData);
        measureRepeat->setYOffset(musicData->getYOffset());

        measureRepeat->setSingleRepeat(isSingleRepeat);
    } else {
        Decorator* decorator = new Decorator();
        measureData->addMusicData(decorator);

        decorator->copyCommonBlock(*musicData);
        decorator->setYOffset(musicData->getYOffset());

        decorator->setDecoratorType(decoratorType);
        decorator->setArticulationType(artType);
    }

    int cursor = m_ove->getIsVersion4() ? 16 : 14;
    if (!jump(length - cursor)) {
        return false;
    }

    return true;
}

bool BarsParse::parseWedge(MeasureData* measureData, int length)
{
    Block placeHolder;
    Wedge* wedge = new Wedge();

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(wedge)) {
        return false;
    }

    // wedge type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    WedgeType wedgeType = WedgeType::Cresc_Line;
    bool wedgeOrExpression = true;
    unsigned int highHalfByte = getHighNibble(placeHolder.toUnsignedInt());
    unsigned int lowHalfByte = getLowNibble(placeHolder.toUnsignedInt());

    switch (highHalfByte) {
    case 0x0: {
        wedgeType = WedgeType::Cresc_Line;
        wedgeOrExpression = true;
        break;
    }
    case 0x4: {
        wedgeType = WedgeType::Dim_Line;
        wedgeOrExpression = true;
        break;
    }
    case 0x6: {
        wedgeType = WedgeType::Dim;
        wedgeOrExpression = false;
        break;
    }
    case 0x2: {
        wedgeType = WedgeType::Cresc;
        wedgeOrExpression = false;
        break;
    }
    default:
        break;
    }

    // 0xb | 0x8(ove3) , else 3, 0(ove3)
    if ((lowHalfByte & 0x8) == 0x8) {
        wedgeType = WedgeType::Double_Line;
        wedgeOrExpression = true;
    }

    if (!jump(1)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    wedge->setYOffset(placeHolder.toInt());

    // wedge
    if (wedgeOrExpression) {
        measureData->addCrossMeasureElement(wedge, true);
        wedge->setWedgeType(wedgeType);

        if (!jump(2)) {
            return false;
        }

        // height
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        wedge->setHeight(placeHolder.toUnsignedInt());

        // offset common
        if (!parseOffsetCommonBlock(wedge)) {
            return false;
        }

        int cursor = m_ove->getIsVersion4() ? 21 : 19;
        if (!jump(length - cursor)) {
            return false;
        }
    }
    // expression : cresc, dim
    else {
        Expressions* express = new Expressions();
        measureData->addMusicData(express);

        express->copyCommonBlock(*wedge);
        express->setYOffset(wedge->getYOffset());

        if (!jump(4)) {
            return false;
        }

        // offset common
        if (!parseOffsetCommonBlock(express)) {
            return false;
        }

        if (m_ove->getIsVersion4()) {
            if (!jump(18)) {
                return false;
            }

            // words
            if (length > 39) {
                if (!readBuffer(placeHolder, length - 39)) {
                    return false;
                }
                express->setText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
            }
        } else {
            QString str = wedgeType == WedgeType::Cresc ? "cresc" : "dim";
            express->setText(str);

            if (!jump(8)) {
                return false;
            }
        }
    }

    return true;
}

bool BarsParse::parseDynamics(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    Dynamics* dynamics = new Dynamics();
    measureData->addMusicData(dynamics);

    if (!jump(1)) {
        return false;
    }

    // is playback
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    dynamics->setIsPlayback(getHighNibble(placeHolder.toUnsignedInt()) != 0x4);

    if (!jump(1)) {
        return false;
    }

    // common
    if (!parseCommonBlock(dynamics)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    dynamics->setYOffset(placeHolder.toInt());

    // dynamics type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    dynamics->setDynamicsType(getLowNibble(placeHolder.toUnsignedInt()));

    // velocity
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    dynamics->setVelocity(placeHolder.toUnsignedInt());

    int cursor = m_ove->getIsVersion4() ? 4 : 2;

    if (!jump(cursor)) {
        return false;
    }

    return true;
}

bool BarsParse::parseKey(MeasureData* measureData, int /* length */)
{
    Block placeHolder;
    Key* key = measureData->getKey();
    int cursor = m_ove->getIsVersion4() ? 9 : 7;

    if (!jump(cursor)) {
        return false;
    }

    // key
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    key->setKey(oveKeyToKey(placeHolder.toUnsignedInt()));

    // previous key
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    key->setPreviousKey(oveKeyToKey(placeHolder.toUnsignedInt()));

    if (!jump(3)) {
        return false;
    }

    // symbol count
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    key->setSymbolCount(placeHolder.toUnsignedInt());

    if (!jump(4)) {
        return false;
    }

    return true;
}

bool BarsParse::parsePedal(MeasureData* measureData, int length)
{
    Block placeHolder;

    Pedal* pedal = new Pedal();
    // measureData->addMusicData(pedal); // can't remember why
    measureData->addCrossMeasureElement(pedal, true);

    if (!jump(1)) {
        return false;
    }

    // is playback
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    pedal->setIsPlayback(getHighNibble(placeHolder.toUnsignedInt()) != 4);

    if (!jump(1)) {
        return false;
    }

    // common
    if (!parseCommonBlock(pedal)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // pair lines
    if (!parsePairLinesBlock(pedal)) {
        return false;
    }

    // offset common
    if (!parseOffsetCommonBlock(pedal)) {
        return false;
    }

    // left shoulder
    if (!parseOffsetElement(pedal->getLeftShoulder())) {
        return false;
    }

    // right shoulder
    if (!parseOffsetElement(pedal->getRightShoulder())) {
        return false;
    }

    int cursor = m_ove->getIsVersion4() ? 0x45 : 0x23;
    int blankCount = m_ove->getIsVersion4() ? 42 : 10;

    pedal->setHalf(length > cursor);

    if (!jump(blankCount)) {
        return false;
    }

    if (length > cursor) {
        if (!jump(2)) {
            return false;
        }

        // handle x offset
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        pedal->getPedalHandle()->setXOffset(placeHolder.toInt());

        if (!jump(6)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseKuohao(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    KuoHao* kuoHao = new KuoHao();
    measureData->addMusicData(kuoHao);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(kuoHao)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // pair lines
    if (!parsePairLinesBlock(kuoHao)) {
        return false;
    }

    if (!jump(4)) {
        return false;
    }

    // left shoulder
    if (!parseOffsetElement(kuoHao->getLeftShoulder())) {
        return false;
    }

    // right shoulder
    if (!parseOffsetElement(kuoHao->getRightShoulder())) {
        return false;
    }

    // kuohao type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    kuoHao->setKuohaoType(placeHolder.toUnsignedInt());

    // height
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    kuoHao->setHeight(placeHolder.toUnsignedInt());

    int jumpAmount = m_ove->getIsVersion4() ? 40 : 8;
    if (!jump(jumpAmount)) {
        return false;
    }

    return true;
}

bool BarsParse::parseExpressions(MeasureData* measureData, int length)
{
    Block placeHolder;

    Expressions* expressions = new Expressions();
    measureData->addMusicData(expressions);

    if (!jump(3)) {
        return false;
    }

    // common00
    if (!parseCommonBlock(expressions)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    expressions->setYOffset(placeHolder.toInt());

    // range bar offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    // int barOffset = placeHolder.toUnsignedInt();

    if (!jump(10)) {
        return false;
    }

    // tempo 1
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    // double tempo1 = ((double)placeHolder.toUnsignedInt()) / 100.0;

    // tempo 2
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    // double tempo2 = ((double)placeHolder.toUnsignedInt()) / 100.0;

    if (!jump(6)) {
        return false;
    }

    // text
    int cursor = m_ove->getIsVersion4() ? 35 : 33;
    if (length > cursor) {
        if (!readBuffer(placeHolder, length - cursor)) {
            return false;
        }
        expressions->setText(m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
    }

    return true;
}

bool BarsParse::parseHarpPedal(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    HarpPedal* harpPedal = new HarpPedal();
    measureData->addMusicData(harpPedal);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(harpPedal)) {
        return false;
    }

    if (!jump(2)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    harpPedal->setYOffset(placeHolder.toInt());

    // show type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    harpPedal->setShowType(placeHolder.toUnsignedInt());

    // show char flag
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    harpPedal->setShowCharFlag(placeHolder.toUnsignedInt());

    if (!jump(8)) {
        return false;
    }

    return true;
}

bool BarsParse::parseMultiMeasureRest(MeasureData* measureData, int /* length */)
{
    Block placeHolder(2);
    MultiMeasureRest* measureRest = new MultiMeasureRest();
    measureData->addMusicData(measureRest);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(measureRest)) {
        return false;
    }

    if (!jump(6)) {
        return false;
    }

    return true;
}

bool BarsParse::parseHarmonyGuitarFrame(MeasureData* measureData, int length)
{
    Block placeHolder;

    Harmony* harmony = new Harmony();
    measureData->addMusicData(harmony);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(harmony)) {
        return false;
    }

    // root
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    harmony->setRoot(placeHolder.toUnsignedInt());

    // type
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }

    // bass
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    harmony->setBass(placeHolder.toUnsignedInt());

    int jumpAmount = m_ove->getIsVersion4() ? length - 12 : length - 10;
    if (!jump(jumpAmount)) {
        return false;
    }

    return true;
}

void extractOctave(unsigned int Bits, OctaveShiftType& octaveShiftType, QList<OctaveShiftPosition>& positions)
{
    octaveShiftType = OctaveShiftType::OS_8;
    positions.clear();

    switch (Bits) {
    case 0x0: {
        octaveShiftType = OctaveShiftType::OS_8;
        positions.push_back(OctaveShiftPosition::Continue);
        break;
    }
    case 0x1: {
        octaveShiftType = OctaveShiftType::OS_Minus_8;
        positions.push_back(OctaveShiftPosition::Continue);
        break;
    }
    case 0x2: {
        octaveShiftType = OctaveShiftType::OS_15;
        positions.push_back(OctaveShiftPosition::Continue);
        break;
    }
    case 0x3: {
        octaveShiftType = OctaveShiftType::OS_Minus_15;
        positions.push_back(OctaveShiftPosition::Continue);
        break;
    }
    case 0x4: {
        octaveShiftType = OctaveShiftType::OS_8;
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0x5: {
        octaveShiftType = OctaveShiftType::OS_Minus_8;
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0x6: {
        octaveShiftType = OctaveShiftType::OS_15;
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0x7: {
        octaveShiftType = OctaveShiftType::OS_Minus_15;
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0x8: {
        octaveShiftType = OctaveShiftType::OS_8;
        positions.push_back(OctaveShiftPosition::Start);
        break;
    }
    case 0x9: {
        octaveShiftType = OctaveShiftType::OS_Minus_8;
        positions.push_back(OctaveShiftPosition::Start);
        break;
    }
    case 0xA: {
        octaveShiftType = OctaveShiftType::OS_15;
        positions.push_back(OctaveShiftPosition::Start);
        break;
    }
    case 0xB: {
        octaveShiftType = OctaveShiftType::OS_Minus_15;
        positions.push_back(OctaveShiftPosition::Start);
        break;
    }
    case 0xC: {
        octaveShiftType = OctaveShiftType::OS_8;
        positions.push_back(OctaveShiftPosition::Start);
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0xD: {
        octaveShiftType = OctaveShiftType::OS_Minus_8;
        positions.push_back(OctaveShiftPosition::Start);
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0xE: {
        octaveShiftType = OctaveShiftType::OS_15;
        positions.push_back(OctaveShiftPosition::Start);
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    case 0xF: {
        octaveShiftType = OctaveShiftType::OS_Minus_15;
        positions.push_back(OctaveShiftPosition::Start);
        positions.push_back(OctaveShiftPosition::Stop);
        break;
    }
    default:
        break;
    }
}

bool BarsParse::parseOctaveShift(MeasureData* measureData, int /* length */)
{
    Block placeHolder;

    OctaveShift* octave = new OctaveShift();
    measureData->addCrossMeasureElement(octave, true);

    if (!jump(3)) {
        return false;
    }

    // common
    if (!parseCommonBlock(octave)) {
        return false;
    }

    // octave
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    unsigned int type = getLowNibble(placeHolder.toUnsignedInt());
    OctaveShiftType octaveShiftType = OctaveShiftType::OS_8;
    QList<OctaveShiftPosition> positions;
    extractOctave(type, octaveShiftType, positions);

    octave->setOctaveShiftType(octaveShiftType);

    if (!jump(1)) {
        return false;
    }

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    octave->setYOffset(placeHolder.toInt());

    if (!jump(4)) {
        return false;
    }

    // length
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    octave->setLength(placeHolder.toUnsignedInt());

    // end tick
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    octave->setEndTick(placeHolder.toUnsignedInt());

    // start & stop maybe appear in same measure
    for (int i = 0; i < positions.size(); ++i) {
        OctaveShiftPosition position = positions[i];
        OctaveShiftEndPoint* octavePoint = new OctaveShiftEndPoint();
        measureData->addMusicData(octavePoint);

        octavePoint->copyCommonBlock(*octave);
        octavePoint->setOctaveShiftType(octaveShiftType);
        octavePoint->setOctaveShiftPosition(position);
        octavePoint->setEndTick(octave->getEndTick());

        // stop
        if (i == 0 && position == OctaveShiftPosition::Stop) {
            octavePoint->start()->setOffset(octave->start()->getOffset() + octave->getLength());
        }

        // end point
        if (i > 0) {
            octavePoint->start()->setOffset(octave->start()->getOffset() + octave->getLength());
            octavePoint->setTick(octave->getEndTick());
        }
    }

    return true;
}

bool BarsParse::parseMidiController(MeasureData* measureData, int /* length */)
{
    Block placeHolder;
    MidiController* controller = new MidiController();
    measureData->addMidiData(controller);

    parseMidiCommon(controller);

    // value [0, 128)
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    controller->setValue(placeHolder.toUnsignedInt());

    // controller number
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    controller->setController(placeHolder.toUnsignedInt());

    if (m_ove->getIsVersion4()) {
        if (!jump(2)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseMidiProgramChange(MeasureData* measureData, int /* length */)
{
    Block placeHolder;
    MidiProgramChange* program = new MidiProgramChange();
    measureData->addMidiData(program);

    parseMidiCommon(program);

    if (!jump(1)) {
        return false;
    }

    // patch
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    program->setPatch(placeHolder.toUnsignedInt());

    if (m_ove->getIsVersion4()) {
        if (!jump(2)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseMidiChannelPressure(MeasureData* measureData, int /* length */)
{
    Block placeHolder;
    MidiChannelPressure* pressure = new MidiChannelPressure();
    measureData->addMidiData(pressure);

    parseMidiCommon(pressure);

    if (!jump(1)) {
        return false;
    }

    // pressure
    if (!readBuffer(placeHolder, 1)) {
        return false;
    }
    pressure->setPressure(placeHolder.toUnsignedInt());

    if (m_ove->getIsVersion4()) {
        if (!jump(2)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseMidiPitchWheel(MeasureData* measureData, int /* length */)
{
    Block placeHolder;
    MidiPitchWheel* wheel = new MidiPitchWheel();
    measureData->addMidiData(wheel);

    parseMidiCommon(wheel);

    // pitch wheel
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    int value = placeHolder.toUnsignedInt();
    wheel->setValue(value);

    if (m_ove->getIsVersion4()) {
        if (!jump(2)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseSizeBlock(int length)
{
    if (!jump(length)) {
        return false;
    }

    return true;
}

bool BarsParse::parseMidiCommon(MidiData* ptr)
{
    Block placeHolder;

    if (!jump(3)) {
        return false;
    }

    // start position
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->setTick(placeHolder.toUnsignedInt());

    return true;
}

bool BarsParse::parseCommonBlock(MusicData* ptr)
{
    Block placeHolder;

    // start tick
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->setTick(placeHolder.toInt());

    // start unit
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->start()->setOffset(placeHolder.toInt());

    if (m_ove->getIsVersion4()) {
        // color
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        ptr->setColor(placeHolder.toUnsignedInt());

        if (!jump(1)) {
            return false;
        }
    }

    return true;
}

bool BarsParse::parseOffsetCommonBlock(MusicData* ptr)
{
    Block placeHolder;

    // offset measure
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->stop()->setMeasure(placeHolder.toUnsignedInt());

    // end unit
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->stop()->setOffset(placeHolder.toInt());

    return true;
}

bool BarsParse::parsePairLinesBlock(PairEnds* ptr)
{
    Block placeHolder;

    // left line
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->getLeftLine()->setLine(placeHolder.toInt());

    // right line
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->getRightLine()->setLine(placeHolder.toInt());

    return true;
}

bool BarsParse::parseOffsetElement(OffsetElement* ptr)
{
    Block placeHolder;

    // x offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->setXOffset(placeHolder.toInt());

    // y offset
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    ptr->setYOffset(placeHolder.toInt());

    return true;
}

bool BarsParse::getCondElementType(unsigned int byteData, CondType& type)
{
    if (byteData == 0x09) {
        type = CondType::Time_Parameters;
    } else if (byteData == 0x0A) {
        type = CondType::Bar_Number;
    } else if (byteData == 0x16) {
        type = CondType::Decorator;
    } else if (byteData == 0x1C) {
        type = CondType::Tempo;
    } else if (byteData == 0x1D) {
        type = CondType::Text;
    } else if (byteData == 0x25) {
        type = CondType::Expression;
    } else if (byteData == 0x30) {
        type = CondType::Barline_Parameters;
    } else if (byteData == 0x31) {
        type = CondType::Repeat;
    } else if (byteData == 0x32) {
        type = CondType::Numeric_Ending;
    } else {
        return false;
    }

    return true;
}

bool BarsParse::getBdatElementType(unsigned int byteData, BdatType& type)
{
    if (byteData == 0x70) {
        type = BdatType::Raw_Note;
    } else if (byteData == 0x80) {
        type = BdatType::Rest;
    } else if (byteData == 0x90) {
        type = BdatType::Note;
    } else if (byteData == 0x10) {
        type = BdatType::Beam;
    } else if (byteData == 0x11) {
        type = BdatType::Harmony;
    } else if (byteData == 0x12) {
        type = BdatType::Clef;
    } else if (byteData == 0x13) {
        type = BdatType::Wedge;
    } else if (byteData == 0x14) {
        type = BdatType::Dynamics;
    } else if (byteData == 0x15) {
        type = BdatType::Glissando;
    } else if (byteData == 0x16) {
        type = BdatType::Decorator;
    } else if (byteData == 0x17) {
        type = BdatType::Key;
    } else if (byteData == 0x18) {
        type = BdatType::Lyric;
    } else if (byteData == 0x19) {
        type = BdatType::Octave_Shift;
    } else if (byteData == 0x1B) {
        type = BdatType::Slur;
    } else if (byteData == 0x1D) {
        type = BdatType::Text;
    } else if (byteData == 0x1E) {
        type = BdatType::Tie;
    } else if (byteData == 0x1F) {
        type = BdatType::Tuplet;
    } else if (byteData == 0x21) {
        type = BdatType::Guitar_Bend;
    } else if (byteData == 0x22) {
        type = BdatType::Guitar_Barre;
    } else if (byteData == 0x23) {
        type = BdatType::Pedal;
    } else if (byteData == 0x24) {
        type = BdatType::KuoHao;
    } else if (byteData == 0x25) {
        type = BdatType::Expressions;
    } else if (byteData == 0x26) {
        type = BdatType::Harp_Pedal;
    } else if (byteData == 0x27) {
        type = BdatType::Multi_Measure_Rest;
    } else if (byteData == 0x28) {
        type = BdatType::Harmony_GuitarFrame;
    } else if (byteData == 0x40) {
        type = BdatType::Graphics_40;
    } else if (byteData == 0x41) {
        type = BdatType::Graphics_RoundRect;
    } else if (byteData == 0x42) {
        type = BdatType::Graphics_Rect;
    } else if (byteData == 0x43) {
        type = BdatType::Graphics_Round;
    } else if (byteData == 0x44) {
        type = BdatType::Graphics_Line;
    } else if (byteData == 0x45) {
        type = BdatType::Graphics_Curve;
    } else if (byteData == 0x46) {
        type = BdatType::Graphics_WedgeSymbol;
    } else if (byteData == 0xAB) {
        type = BdatType::Midi_Controller;
    } else if (byteData == 0xAC) {
        type = BdatType::Midi_Program_Change;
    } else if (byteData == 0xAD) {
        type = BdatType::Midi_Channel_Pressure;
    } else if (byteData == 0xAE) {
        type = BdatType::Midi_Pitch_Wheel;
    } else if (byteData == 0xFF) {
        type = BdatType::Bar_End;
    } else {
        return false;
    }

    return true;
}

LyricChunkParse::LyricChunkParse(OveSong* ove)
    : BasicParse(ove)
{
}

void LyricChunkParse::setLyricChunk(SizeChunk* chunk)
{
    m_chunk = chunk;
}

// only ove3 has this chunk
bool LyricChunkParse::parse()
{
    unsigned int i;
    Block* dataBlock = m_chunk->getDataBlock();
    unsigned int blockSize = m_chunk->getSizeBlock()->toSize();
    StreamHandle handle(dataBlock->data(), blockSize);
    Block placeHolder;

    m_handle = &handle;

    if (!jump(4)) {
        return false;
    }

    // Lyric count
    if (!readBuffer(placeHolder, 2)) {
        return false;
    }
    unsigned int count = placeHolder.toUnsignedInt();

    for (i = 0; i < count; ++i) {
        LyricInfo info;

        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        // unsigned int size = placeHolder.toUnsignedInt();

        // 0x0D00
        if (!jump(2)) {
            return false;
        }

        // voice
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        info.m_voice = placeHolder.toUnsignedInt();

        // verse
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        info.m_verse = placeHolder.toUnsignedInt();

        // track
        if (!readBuffer(placeHolder, 1)) {
            return false;
        }
        info.m_track = placeHolder.toUnsignedInt();

        if (!jump(1)) {
            return false;
        }

        // measure
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        info.m_measure = placeHolder.toUnsignedInt();

        // word count
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        info.m_wordCount = placeHolder.toUnsignedInt();

        // lyric size
        if (!readBuffer(placeHolder, 2)) {
            return false;
        }
        info.m_lyricSize = placeHolder.toUnsignedInt();

        if (!jump(6)) {
            return false;
        }

        // name
        if (!readBuffer(placeHolder, 32)) {
            return false;
        }
        info.m_name = m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray());

        if (info.m_lyricSize > 0) {
            // lyric
            if (!readBuffer(placeHolder, info.m_lyricSize)) {
                return false;
            }
            info.m_lyric = m_ove->getCodecString(placeHolder.fixedSizeBufferToStrByteArray());

            if (!jump(4)) {
                return false;
            }

            // font
            if (!readBuffer(placeHolder, 2)) {
                return false;
            }
            info.m_font = placeHolder.toUnsignedInt();

            if (!jump(1)) {
                return false;
            }

            // font size
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            info.m_fontSize = placeHolder.toUnsignedInt();

            // font style
            if (!readBuffer(placeHolder, 1)) {
                return false;
            }
            info.m_fontStyle = placeHolder.toUnsignedInt();

            if (!jump(1)) {
                return false;
            }

            for (int j = 0; j < info.m_wordCount; ++j) {
                if (!jump(8)) {
                    return false;
                }
            }
        }

        processLyricInfo(info);
    }

    return true;
}

bool isSpace(char c)
{
    return c == ' ' || c == '\n';
}

void LyricChunkParse::processLyricInfo(const LyricInfo& info)
{
    int i;
    int j;
    int index = 0; // words

    int measureId = info.m_measure - 1;
    bool changeMeasure = true;
    MeasureData* measureData = 0;
    int trackMeasureCount = m_ove->getTrackBarCount();
    QStringList words = info.m_lyric.split(" ", Qt::SkipEmptyParts);

    while (index < words.size() && measureId + 1 < trackMeasureCount) {
        if (changeMeasure) {
            ++measureId;
            measureData = m_ove->getMeasureData(info.m_track, measureId);
            changeMeasure = false;
        }

        if (measureData == 0) {
            return;
        }
        QList<NoteContainer*> containers = measureData->getNoteContainers();
        QList<MusicData*> lyrics = measureData->getMusicDatas(MusicDataType::Lyric);

        for (i = 0; i < containers.size() && index < words.size(); ++i) {
            if (containers[i]->getIsRest()) {
                continue;
            }

            for (j = 0; j < lyrics.size(); ++j) {
                Lyric* lyric = static_cast<Lyric*>(lyrics[j]);

                if (containers[i]->start()->getOffset() == lyric->start()->getOffset()
                    && (int)containers[i]->getVoice() == info.m_voice
                    && lyric->getVerse() == info.m_verse) {
                    if (index < words.size()) {
                        QString l = words[index].trimmed();
                        if (!l.isEmpty()) {
                            lyric->setLyric(l);
                            lyric->setVoice(info.m_voice);
                        }
                    }

                    ++index;
                }
            }
        }

        changeMeasure = true;
    }
}

TitleChunkParse::TitleChunkParse(OveSong* ove)
    : BasicParse(ove)
{
    m_titleType = 0x00000001;
    m_annotateType = 0x00010000;
    m_writerType = 0x00020002;
    m_copyrightType = 0x00030001;
    m_headerType = 0x00040000;
    m_footerType = 0x00050002;
}

void TitleChunkParse::setTitleChunk(SizeChunk* chunk)
{
    m_chunk = chunk;
}

QByteArray getByteArray(const Block& block)
{
    QByteArray array((char*)block.data(), block.size());
    int index0 = array.indexOf('\0');
    array = array.left(index0);

    return array;
}

bool TitleChunkParse::parse()
{
    Block* dataBlockP = m_chunk->getDataBlock();
    unsigned int blockSize = m_chunk->getSizeBlock()->toSize();
    StreamHandle handle(dataBlockP->data(), blockSize);
    Block typeBlock;
    unsigned int titleType;

    m_handle = &handle;

    if (!readBuffer(typeBlock, 4)) {
        return false;
    }

    titleType = typeBlock.toUnsignedInt();

    if (titleType == m_titleType || titleType == m_annotateType || titleType == m_writerType
        || titleType == m_copyrightType) {
        Block offsetBlock;

        if (!readBuffer(offsetBlock, 4)) {
            return false;
        }

        const unsigned int itemCount = 4;
        unsigned int i;

        for (i = 0; i < itemCount; ++i) {
            if (i > 0) {
                // 0x00 AB 00 0C 00 00
                if (!jump(6)) {
                    return false;
                }
            }

            Block countBlock;
            if (!readBuffer(countBlock, 2)) {
                return false;
            }
            unsigned int titleSize = countBlock.toUnsignedInt();

            Block dataBlock;
            if (!readBuffer(dataBlock, titleSize)) {
                return false;
            }

            QByteArray array = getByteArray(dataBlock);
            if (!array.isEmpty()) {
                addToOve(m_ove->getCodecString(array), titleType);
            }
        }

        return true;
    }

    if (titleType == m_headerType || titleType == m_footerType) {
        if (!jump(10)) {
            return false;
        }

        Block countBlock;
        if (!readBuffer(countBlock, 2)) {
            return false;
        }
        unsigned int titleSize = countBlock.toUnsignedInt();

        Block dataBlock;
        if (!readBuffer(dataBlock, titleSize)) {
            return false;
        }

        QByteArray array = getByteArray(dataBlock);
        addToOve(m_ove->getCodecString(array), titleType);

        // 0x00 AB 00 0C 00 00
        if (!jump(6)) {
            return false;
        }

        return true;
    }

    return false;
}

void TitleChunkParse::addToOve(const QString& str, unsigned int titleType)
{
    if (str.isEmpty()) {
        return;
    }

    if (titleType == m_titleType) {
        m_ove->addTitle(str);
    }

    if (titleType == m_annotateType) {
        m_ove->addAnnotate(str);
    }

    if (titleType == m_writerType) {
        m_ove->addWriter(str);
    }

    if (titleType == m_copyrightType) {
        m_ove->addCopyright(str);
    }

    if (titleType == m_headerType) {
        m_ove->addHeader(str);
    }

    if (titleType == m_footerType) {
        m_ove->addFooter(str);
    }
}

OveOrganizer::OveOrganizer(OveSong* ove)
{
    m_ove = ove;
}

void OveOrganizer::organize()
{
    if (m_ove == NULL) {
        return;
    }

    organizeTracks();
    organizeAttributes();
    organizeMeasures();
}

void OveOrganizer::organizeAttributes()
{
    int i;
    int j;
    int k;

    // key
    if (m_ove->getLineCount() > 0) {
        Line* line = m_ove->getLine(0);
        int partBarCount = m_ove->getPartBarCount();
        int lastKey = 0;

        if (line != 0) {
            for (i = 0; i < line->getStaffCount(); ++i) {
                QPair<int, int> partStaff = m_ove->trackToPartStaff(i);
                Staff* staff = line->getStaff(i);
                lastKey = staff->getKeyType();

                for (j = 0; j < partBarCount; ++j) {
                    MeasureData* measureData = m_ove->getMeasureData(partStaff.first, partStaff.second, j);

                    if (measureData != 0) {
                        Key* key = measureData->getKey();

                        if (j == 0) {
                            key->setKey(lastKey);
                            key->setPreviousKey(lastKey);
                        }

                        if (!key->getSetKey()) {
                            key->setKey(lastKey);
                            key->setPreviousKey(lastKey);
                        } else {
                            if (key->getKey() != lastKey) {
                                lastKey = key->getKey();
                            }
                        }
                    }
                }
            }
        }
    }

    // clef
    if (m_ove->getLineCount() > 0) {
        Line* line = m_ove->getLine(0);
        int partBarCount = m_ove->getPartBarCount();
        ClefType lastClefType = ClefType::Treble;

        if (line != 0) {
            for (i = 0; i < line->getStaffCount(); ++i) {
                QPair<int, int> partStaff = m_ove->trackToPartStaff(i);
                Staff* staff = line->getStaff(i);
                lastClefType = staff->getClefType();

                for (j = 0; j < partBarCount; ++j) {
                    MeasureData* measureData = m_ove->getMeasureData(partStaff.first, partStaff.second, j);

                    if (measureData != 0) {
                        Clef* clefPtr = measureData->getClef();
                        clefPtr->setClefType((int)lastClefType);

                        const QList<MusicData*>& clefs = measureData->getMusicDatas(MusicDataType::Clef);

                        for (k = 0; k < clefs.size(); ++k) {
                            Clef* clef = static_cast<Clef*>(clefs[k]);
                            lastClefType = clef->getClefType();
                        }
                    }
                }
            }
        }
    }
}

Staff* getStaff(OveSong* ove, int track)
{
    if (ove->getLineCount() > 0) {
        Line* line = ove->getLine(0);
        if (line != 0 && line->getStaffCount() > 0) {
            Staff* staff = line->getStaff(track);
            return staff;
        }
    }

    return 0;
}

void OveOrganizer::organizeTracks()
{
    int i;
    // QList<QPair<ClefType, int>> trackChannels;
    QList<Track*> tracks = m_ove->getTracks();
    QList<bool> comboStaveStarts;

    for (i = 0; i < tracks.size(); ++i) {
        comboStaveStarts.push_back(false);
    }

    for (i = 0; i < tracks.size(); ++i) {
        Staff* staff = getStaff(m_ove, i);
        if (staff != 0) {
            if (staff->getGroupType() == GroupType::Brace && staff->getGroupStaffCount() == 1) {
                comboStaveStarts[i] = true;
            }
        }

        /*
        if (i < tracks.size() - 1) {
            if (tracks[i]->getStartClef() == ClefType::Treble &&
                tracks[i + 1]->getStartClef() == ClefType::Bass &&
                tracks[i]->getChannel() == tracks[i + 1]->get_channel()) {
            }
        }
        */
    }

    int trackId = 0;
    QList<int> partStaffCounts;

    while (trackId < (int)tracks.size()) {
        int partTrackCount = 1;

        if (comboStaveStarts[trackId]) {
            partTrackCount = 2;
        }

        partStaffCounts.push_back(partTrackCount);
        trackId += partTrackCount;
    }

    m_ove->setPartStaffCounts(partStaffCounts);
}

void OveOrganizer::organizeMeasures()
{
    int trackBarCount = m_ove->getTrackBarCount();

    for (int i = 0; i < m_ove->getPartCount(); ++i) {
        int partStaffCount = m_ove->getStaffCount(i);

        for (int j = 0; j < partStaffCount; ++j) {
            for (int k = 0; k < trackBarCount; ++k) {
                Measure* measure = m_ove->getMeasure(k);
                MeasureData* measureData = m_ove->getMeasureData(i, j, k);

                organizeMeasure(i, j, measure, measureData);
            }
        }
    }
}

void OveOrganizer::organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData)
{
    // note containers
    organizeContainers(part, track, measure, measureData);

    // single end data
    organizeMusicDatas(part, track, measure, measureData);

    // cross measure elements
    organizeCrossMeasureElements(part, track, measure, measureData);
}

void addToList(QList<int>& list, int number)
{
    for (int i = 0; i < list.size(); ++i) {
        if (list[i] == number) {
            return;
        }
    }

    list.push_back(number);
}

void OveOrganizer::organizeContainers(int /* part */, int /* track */,
                                      Measure* measure, MeasureData* measureData)
{
    int i;
    QList<NoteContainer*> containers = measureData->getNoteContainers();
    int barUnits = measure->getTime()->getUnits();
    QList<int> voices;

    for (i = 0; i < containers.size(); ++i) {
        int endUnit = barUnits;
        if (i < containers.size() - 1) {
            endUnit = containers[i + 1]->start()->getOffset();
        }

        containers[i]->stop()->setOffset(endUnit);
        addToList(voices, containers[i]->getVoice());
    }

    // shift voices
    std::sort(voices.begin(), voices.end());

    for (i = 0; i < voices.size(); ++i) {
        int voice = voices[i];
        // voice -> i
        for (int j = 0; j < (int)containers.size(); ++j) {
            int avoice = containers[j]->getVoice();
            if (avoice == voice && avoice != i) {
                containers[j]->setVoice(i);
            }
        }
    }
}

void OveOrganizer::organizeMusicDatas(int /* part */, int /* track */, Measure* measure, MeasureData* measureData)
{
    int i;
    int barIndex = measure->getBarNumber()->getIndex();
    QList<MusicData*> datas = measureData->getMusicDatas(MusicDataType::None);

    for (i = 0; i < datas.size(); ++i) {
        datas[i]->start()->setMeasure(barIndex);
    }
}

void OveOrganizer::organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData)
{
    int i;
    QList<MusicData*> pairs = measureData->getCrossMeasureElements(MusicDataType::None, MeasureData::PairType::Start);

    for (i = 0; i < pairs.size(); ++i) {
        MusicData* pair = pairs[i];

        switch (pair->getMusicDataType()) {
        case MusicDataType::Beam:
        case MusicDataType::Glissando:
        case MusicDataType::Slur:
        case MusicDataType::Tie:
        case MusicDataType::Tuplet:
        case MusicDataType::Pedal:
        case MusicDataType::Numeric_Ending:
        // case MusicDataType::OctaveShift_EndPoint:
        case MusicDataType::Measure_Repeat: {
            organizePairElement(pair, part, track, measure, measureData);
            break;
        }
        case MusicDataType::OctaveShift: {
            OctaveShift* octave = static_cast<OctaveShift*>(pair);
            organizeOctaveShift(octave, measure, measureData);
            break;
        }
        case MusicDataType::Wedge: {
            Wedge* wedge = static_cast<Wedge*>(pair);
            organizeWedge(wedge, part, track, measure, measureData);
            break;
        }
        default:
            break;
        }
    }
}

void OveOrganizer::organizePairElement(
    MusicData* data,
    int part,
    int track,
    Measure* measure,
    MeasureData* measureData)
{
    int bar1Index = measure->getBarNumber()->getIndex();
    int bar2Index = bar1Index + data->stop()->getMeasure();
    MeasureData* measureData2 = m_ove->getMeasureData(part, track, bar2Index);

    data->start()->setMeasure(bar1Index);

    if (measureData2 != 0 && measureData != measureData2) {
        measureData2->addCrossMeasureElement(data, false);
    }

    if (data->getMusicDataType() == MusicDataType::Tuplet) {
        Tuplet* tuplet = static_cast<Tuplet*>(data);
        const QList<NoteContainer*> containers = measureData->getNoteContainers();

        for (int i = 0; i < containers.size(); ++i) {
            if (containers[i]->getTick() > tuplet->getTick()) {
                break;
            }

            if (containers[i]->getTick() == tuplet->getTick()) {
                tuplet->setNoteType(containers[i]->getNoteType());
            }
        }

        int tupletTick = NoteTypeToTick(tuplet->getNoteType(), m_ove->getQuarter()) * tuplet->getSpace();
        if (tuplet->getTick() % tupletTick != 0) {
            int newStartTick = (tuplet->getTick() / tupletTick) * tupletTick;

            for (int i = 0; i < containers.size(); ++i) {
                if (containers[i]->getTick() == newStartTick
                    && containers[i]->getTuplet() == tuplet->getTuplet()) {
                    tuplet->setTick(containers[i]->getTick());
                    tuplet->start()->setOffset(containers[i]->start()->getOffset());
                }
            }
        }
    }
}

void OveOrganizer::organizeOctaveShift(
    OctaveShift* octave,
    Measure* measure,
    MeasureData* measureData)
{
    // octave shift
    int i;
    const QList<NoteContainer*> containers = measureData->getNoteContainers();
    int barIndex = measure->getBarNumber()->getIndex();

    octave->start()->setMeasure(barIndex);

    for (i = 0; i < containers.size(); ++i) {
        int noteShift = octave->getNoteShift();
        int containerTick = containers[i]->getTick();

        if (octave->getTick() <= containerTick && octave->getEndTick() > containerTick) {
            containers[i]->setNoteShift(noteShift);
        }
    }
}

bool getMiddleUnit(
    OveSong* ove, int /* part */, int /* track */,
    Measure* measure1, Measure* measure2, int unit1, int /* unit2 */,
    Measure* middleMeasure, int& middleUnit)
{
    Q_UNUSED(middleMeasure); // avoid (bogus?) warning to show up
    QList<int> barUnits;
    int i;
    int bar1Index = measure1->getBarNumber()->getIndex();
    int bar2Index = measure2->getBarNumber()->getIndex();
    int sumUnit = 0;

    for (int j = bar1Index; j <= bar2Index; ++j) {
        Measure* measure = ove->getMeasure(j);
        barUnits.push_back(measure->getTime()->getUnits());
        sumUnit += measure->getTime()->getUnits();
    }

    int currentSumUnit = 0;
    for (i = 0; i < barUnits.size(); ++i) {
        int barUnit = barUnits[i];

        if (i == 0) {
            barUnit = barUnits[i] - unit1;
        }

        if (currentSumUnit + barUnit < sumUnit / 2) {
            currentSumUnit += barUnit;
        } else {
            break;
        }
    }

    if (i < barUnits.size()) {
        int barMiddleIndex = bar1Index + i;
        middleMeasure = ove->getMeasure(barMiddleIndex);
        middleUnit = sumUnit / 2 - currentSumUnit;

        return true;
    }

    return false;
}

void OveOrganizer::organizeWedge(Wedge* wedge, int part, int track, Measure* measure, MeasureData* measureData)
{
    int bar1Index = measure->getBarNumber()->getIndex();
    int bar2Index = bar1Index + wedge->stop()->getMeasure();
    MeasureData* measureData2 = m_ove->getMeasureData(part, track, bar2Index);
    WedgeType wedgeType = wedge->getWedgeType();

    if (wedge->getWedgeType() == WedgeType::Double_Line) {
        wedgeType = WedgeType::Cresc_Line;
    }

    wedge->start()->setMeasure(bar1Index);

    WedgeEndPoint* startPoint = new WedgeEndPoint();
    measureData->addMusicData(startPoint);

    startPoint->setTick(wedge->getTick());
    startPoint->start()->setOffset(wedge->start()->getOffset());
    startPoint->setWedgeStart(true);
    startPoint->setWedgeType(wedgeType);
    startPoint->setHeight(wedge->getHeight());

    WedgeEndPoint* stopPoint = new WedgeEndPoint();

    stopPoint->setTick(wedge->getTick());
    stopPoint->start()->setOffset(wedge->stop()->getOffset());
    stopPoint->setWedgeStart(false);
    stopPoint->setWedgeType(wedgeType);
    stopPoint->setHeight(wedge->getHeight());

    if (measureData2 != 0) {
        measureData2->addMusicData(stopPoint);
    }

    if (wedge->getWedgeType() == WedgeType::Double_Line) {
        Measure* middleMeasure = NULL;
        int middleUnit = 0;

        getMiddleUnit(
            m_ove, part, track,
            measure, m_ove->getMeasure(bar2Index),
            wedge->start()->getOffset(), wedge->stop()->getOffset(),
            middleMeasure, middleUnit);

        if (middleUnit != 0) {
            WedgeEndPoint* midStopPoint = new WedgeEndPoint();
            measureData->addMusicData(midStopPoint);

            midStopPoint->setTick(wedge->getTick());
            midStopPoint->start()->setOffset(middleUnit);
            midStopPoint->setWedgeStart(false);
            midStopPoint->setWedgeType(WedgeType::Cresc_Line);
            midStopPoint->setHeight(wedge->getHeight());

            WedgeEndPoint* midStartPoint = new WedgeEndPoint();
            measureData->addMusicData(midStartPoint);

            midStartPoint->setTick(wedge->getTick());
            midStartPoint->start()->setOffset(middleUnit);
            midStartPoint->setWedgeStart(true);
            midStartPoint->setWedgeType(WedgeType::Dim_Line);
            midStartPoint->setHeight(wedge->getHeight());
        }
    }
}

enum class ChunkType : char {
    OVSC = 00,
    TRKL,
    TRAK,
    PAGL,
    PAGE,
    LINL,
    LINE,
    STAF,
    BARL,
    MEAS,
    COND,
    BDAT,
    PACH,
    FNTS,
    ODEV,
    TITL,
    ALOT,
    ENGR,
    FMAP,
    PCPR,

    // Overture 3.6
    LYRC,

    NONE
};

ChunkType nameToChunkType(const NameBlock& name)
{
    ChunkType type = ChunkType::NONE;

    if (name.isEqual("OVSC")) {
        type = ChunkType::OVSC;
    } else if (name.isEqual("TRKL")) {
        type = ChunkType::TRKL;
    } else if (name.isEqual("TRAK")) {
        type = ChunkType::TRAK;
    } else if (name.isEqual("PAGL")) {
        type = ChunkType::PAGL;
    } else if (name.isEqual("PAGE")) {
        type = ChunkType::PAGE;
    } else if (name.isEqual("LINL")) {
        type = ChunkType::LINL;
    } else if (name.isEqual("LINE")) {
        type = ChunkType::LINE;
    } else if (name.isEqual("STAF")) {
        type = ChunkType::STAF;
    } else if (name.isEqual("BARL")) {
        type = ChunkType::BARL;
    } else if (name.isEqual("MEAS")) {
        type = ChunkType::MEAS;
    } else if (name.isEqual("COND")) {
        type = ChunkType::COND;
    } else if (name.isEqual("BDAT")) {
        type = ChunkType::BDAT;
    } else if (name.isEqual("PACH")) {
        type = ChunkType::PACH;
    } else if (name.isEqual("FNTS")) {
        type = ChunkType::FNTS;
    } else if (name.isEqual("ODEV")) {
        type = ChunkType::ODEV;
    } else if (name.isEqual("TITL")) {
        type = ChunkType::TITL;
    } else if (name.isEqual("ALOT")) {
        type = ChunkType::ALOT;
    } else if (name.isEqual("ENGR")) {
        type = ChunkType::ENGR;
    } else if (name.isEqual("FMAP")) {
        type = ChunkType::FMAP;
    } else if (name.isEqual("PCPR")) {
        type = ChunkType::PCPR;
    } else if (name.isEqual("LYRC")) {
        type = ChunkType::LYRC;
    }

    return type;
}

int chunkTypeToMaxTimes(ChunkType type)
{
    int maxTimes = -1; // no limit

    switch (type) {
    case ChunkType::OVSC: {
        maxTimes = 1;
        break;
    }
    case ChunkType::TRKL: { // case ChunkType::TRAK:
        maxTimes = 1;
        break;
    }
    case ChunkType::PAGL: { // case ChunkType::PAGE:
        maxTimes = 1;
        break;
    }
    // case ChunkType::LINE:
    // case ChunkType::STAF:
    case ChunkType::LINL: {
        maxTimes = 1;
        break;
    }
    // case ChunkType::MEAS:
    // case ChunkType::COND:
    // case ChunkType::BDAT:
    case ChunkType::BARL: {
        maxTimes = 1;
        break;
    }
    case ChunkType::PACH:
    case ChunkType::FNTS:
    case ChunkType::ODEV:
    case ChunkType::ALOT:
    case ChunkType::ENGR:
    case ChunkType::FMAP:
    case ChunkType::PCPR: {
        maxTimes = 1;
        break;
    }
    case ChunkType::TITL: {
        maxTimes = 8;
        break;
    }
    case ChunkType::LYRC: {
        maxTimes = 1;
        break;
    }
    // case ChunkType::NONE:
    default:
        break;
    }

    return maxTimes;
}

OveSerialize::OveSerialize()
    : m_ove(0),
    m_streamHandle(0),
    m_notify(0)
{
}

OveSerialize::~OveSerialize()
{
    if (m_streamHandle != 0) {
        delete m_streamHandle;
        m_streamHandle = 0;
    }
}

void OveSerialize::setOve(OveSong* ove)
{
    m_ove = ove;
}

void OveSerialize::setFileStream(unsigned char* buffer, unsigned int size)
{
    m_streamHandle = new StreamHandle(buffer, size);
}

void OveSerialize::setNotify(IOveNotify* notify)
{
    m_notify = notify;
}

void OveSerialize::messageOutError()
{
    if (m_notify != NULL) {
        m_notify->loadError();
    }
}

void OveSerialize::messageOut(const QString& str)
{
    if (m_notify != NULL) {
        m_notify->loadInfo(str);
    }
}

bool OveSerialize::load(void)
{
    if (m_streamHandle == 0) {
        return false;
    }

    if (!readHeader()) {
        messageOutError();
        return false;
    }

    unsigned int i;
    QMap<ChunkType, int> chunkTimes;
    //bool firstEnter = true;

    for (i = (int)ChunkType::OVSC; i < (int)ChunkType::NONE; ++i) {
        chunkTimes[(ChunkType)i] = 0;
    }

    ChunkType chunkType = ChunkType::NONE;

    do {
        NameBlock nameBlock;
        SizeChunk sizeChunk;

        if (!readNameBlock(nameBlock)) {
            return false;
        }

        chunkType = nameToChunkType(nameBlock);
        ++chunkTimes[chunkType];
        int maxTime = chunkTypeToMaxTimes(chunkType);

        if (maxTime > 0 && chunkTimes[chunkType] > maxTime) {
            messageOut("format not support, chunk appear more than accept.\n");
            return false;
        }

        switch (chunkType) {
        /*
        case ChunkType::OVSC: {
            if (!readHeadData(&sizeChunk)) {
                messageOut_error();
                return false;
            }
            break;
        }
        */
        case ChunkType::TRKL: {
            if (!readTracksData()) {
                messageOutError();
                return false;
            }

            break;
        }
        case ChunkType::PAGL: {
            if (!readPagesData()) {
                messageOutError();
                return false;
            }

            break;
        }
        case ChunkType::LINL: {
            if (!readLinesData()) {
                messageOutError();
                return false;
            }

            break;
        }
        case ChunkType::BARL: {
            if (!readBarsData()) {
                messageOutError();
                return false;
            }

            break;
        }
        case ChunkType::TRAK:
        case ChunkType::PAGE:
        case ChunkType::LINE:
        case ChunkType::STAF:
        case ChunkType::MEAS:
        case ChunkType::COND:
        case ChunkType::BDAT: {
            return false;
            break;
        }
        case ChunkType::LYRC: {
            SizeChunk lyricChunk;
            if (!readSizeChunk(&lyricChunk)) {
                messageOutError();
                return false;
            }

            LyricChunkParse parse(m_ove);

            parse.setLyricChunk(&lyricChunk);
            parse.parse();

            break;
        }
        case ChunkType::TITL: {
            SizeChunk titleChunk;
            if (!readSizeChunk(&titleChunk)) {
                messageOutError();
                return false;
            }

            TitleChunkParse titleChunkParse(m_ove);

            titleChunkParse.setTitleChunk(&titleChunk);
            titleChunkParse.parse();

            break;
        }
        case ChunkType::PACH:
        case ChunkType::FNTS:
        case ChunkType::ODEV:
        case ChunkType::ALOT:
        case ChunkType::ENGR:
        case ChunkType::FMAP:
        case ChunkType::PCPR: {
            if (!readSizeChunk(&sizeChunk)) {
                messageOutError();
                return false;
            }

            break;
        }
        default:
            /*
            if (firstEnter) {
                QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.\n";
                messageOut(info);
                messageOutError();

                return false;
            }
            */

            break;
        }

        // firstEnter = false;
    } while (chunkType != ChunkType::NONE);

    /*
    if (!readOveEnd()) {
        return false;
    }
    */

    // organize OveData
    OveOrganizer organizer(m_ove);
    organizer.organize();

    return true;
}

void OveSerialize::release()
{
    delete this;
}

bool OveSerialize::readHeader()
{
    ChunkType chunkType = ChunkType::NONE;
    NameBlock nameBlock;
    SizeChunk sizeChunk;

    if (!readNameBlock(nameBlock)) {
        return false;
    }

    chunkType = nameToChunkType(nameBlock);
    // int maxTime = chunkTypeToMaxTimes(chunkType);

    if (chunkType == ChunkType::OVSC) {
        if (readHeadData(&sizeChunk)) {
            return true;
        }
    }

    QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.\n";
    messageOut(info);

    return false;
}

bool OveSerialize::readHeadData(SizeChunk* ovscChunk)
{
    if (!readSizeChunk(ovscChunk)) {
        return false;
    }

    OvscParse ovscParse(m_ove);

    ovscParse.setNotify(m_notify);
    ovscParse.setOvsc(ovscChunk);

    return ovscParse.parse();
}

bool OveSerialize::readTracksData()
{
    GroupChunk trackGroupChunk;

    if (!readGroupChunk(&trackGroupChunk)) {
        return false;
    }

    unsigned int i;
    unsigned short trackCount = trackGroupChunk.getCountBlock()->toCount();

    for (i = 0; i < trackCount; ++i) {
        SizeChunk* trackChunk = new SizeChunk();

        if (m_ove->getIsVersion4()) {
            if (!readChunkName(trackChunk, Chunk::TrackName)) {
                return false;
            }
            if (!readSizeChunk(trackChunk)) {
                return false;
            }
        } else {
            if (!readDataChunk(trackChunk->getDataBlock(),
                               SizeChunk::version3TrackSize)) {
                return false;
            }
        }

        TrackParse trackParse(m_ove);

        trackParse.setTrack(trackChunk);
        trackParse.parse();
    }

    return true;
}

bool OveSerialize::readPagesData()
{
    GroupChunk pageGroupChunk;

    if (!readGroupChunk(&pageGroupChunk)) {
        return false;
    }

    unsigned short pageCount = pageGroupChunk.getCountBlock()->toCount();
    unsigned int i;
    PageGroupParse parse(m_ove);

    for (i = 0; i < pageCount; ++i) {
        SizeChunk* pageChunk = new SizeChunk();

        if (!readChunkName(pageChunk, Chunk::PageName)) {
            return false;
        }
        if (!readSizeChunk(pageChunk)) {
            return false;
        }

        parse.addPage(pageChunk);
    }

    if (!parse.parse()) {
        return false;
    }

    return true;
}

bool OveSerialize::readLinesData()
{
    GroupChunk lineGroupChunk;
    if (!readGroupChunk(&lineGroupChunk)) {
        return false;
    }

    unsigned short lineCount = lineGroupChunk.getCountBlock()->toCount();
    int i;
    unsigned int j;
    QList<SizeChunk*> lineChunks;
    QList<SizeChunk*> staffChunks;

    for (i = 0; i < lineCount; ++i) {
        SizeChunk* lineChunk = new SizeChunk();

        if (!readChunkName(lineChunk, Chunk::LineName)) {
            return false;
        }
        if (!readSizeChunk(lineChunk)) {
            return false;
        }

        lineChunks.push_back(lineChunk);

        StaffCountGetter getter(m_ove);
        unsigned int staffCount = getter.getStaffCount(lineChunk);

        for (j = 0; j < staffCount; ++j) {
            SizeChunk* staffChunk = new SizeChunk();

            if (!readChunkName(staffChunk, Chunk::StaffName)) {
                return false;
            }
            if (!readSizeChunk(staffChunk)) {
                return false;
            }

            staffChunks.push_back(staffChunk);
        }
    }

    LineGroupParse parse(m_ove);

    parse.setLineGroup(&lineGroupChunk);

    for (i = 0; i < lineChunks.size(); ++i) {
        parse.addLine(lineChunks[i]);
    }

    for (i = 0; i < staffChunks.size(); ++i) {
        parse.addStaff(staffChunks[i]);
    }

    if (!parse.parse()) {
        return false;
    }

    return true;
}

bool OveSerialize::readBarsData()
{
    GroupChunk barGroupChunk;
    if (!readGroupChunk(&barGroupChunk)) {
        return false;
    }

    unsigned short measCount = barGroupChunk.getCountBlock()->toCount();
    int i;

    QList<SizeChunk*> measureChunks;
    QList<SizeChunk*> conductChunks;
    QList<SizeChunk*> bdatChunks;

    m_ove->setTrackBarCount(measCount);

    // read chunks
    for (i = 0; i < measCount; ++i) {
        SizeChunk* measureChunkPtr = new SizeChunk();

        if (!readChunkName(measureChunkPtr, Chunk::MeasureName)) {
            return false;
        }
        if (!readSizeChunk(measureChunkPtr)) {
            return false;
        }

        measureChunks.push_back(measureChunkPtr);
    }

    for (i = 0; i < measCount; ++i) {
        SizeChunk* conductChunkPtr = new SizeChunk();

        if (!readChunkName(conductChunkPtr, Chunk::ConductName)) {
            return false;
        }

        if (!readSizeChunk(conductChunkPtr)) {
            return false;
        }

        conductChunks.push_back(conductChunkPtr);
    }

    int bdatCount = m_ove->getTrackCount() * measCount;
    for (i = 0; i < bdatCount; ++i) {
        SizeChunk* batChunkPtr = new SizeChunk();

        if (!readChunkName(batChunkPtr, Chunk::BdatName)) {
            return false;
        }
        if (!readSizeChunk(batChunkPtr)) {
            return false;
        }

        bdatChunks.push_back(batChunkPtr);
    }

    // parse bars
    BarsParse barsParse(m_ove);

    for (i = 0; i < measureChunks.size(); ++i) {
        barsParse.addMeasure(measureChunks[i]);
    }

    for (i = 0; i < conductChunks.size(); ++i) {
        barsParse.addConduct(conductChunks[i]);
    }

    for (i = 0; i < bdatChunks.size(); ++i) {
        barsParse.addBdat(bdatChunks[i]);
    }

    barsParse.setNotify(m_notify);
    if (!barsParse.parse()) {
        return false;
    }

    return true;
}

bool OveSerialize::readOveEnd()
{
    if (m_streamHandle == 0) {
        return false;
    }

    const unsigned int END_OVE1 = 0xffffffff;
    const unsigned int END_OVE2 = 0x00000000;
    unsigned int buffer;

    if (!m_streamHandle->read((char*)&buffer, sizeof(unsigned int))) {
        return false;
    }

    if (buffer != END_OVE1) {
        return false;
    }

    if (!m_streamHandle->read((char*)&buffer, sizeof(unsigned int))) {
        return false;
    }

    if (buffer != END_OVE2) {
        return false;
    }

    return true;
}

bool OveSerialize::readNameBlock(NameBlock& nameBlock)
{
    if (m_streamHandle == 0) {
        return false;
    }

    if (!m_streamHandle->read((char*)nameBlock.data(), nameBlock.size())) {
        return false;
    }

    return true;
}

bool OveSerialize::readChunkName(Chunk* /*chunk*/, const QString& name)
{
    if (m_streamHandle == 0) {
        return false;
    }

    NameBlock nameBlock;

    if (!m_streamHandle->read((char*)nameBlock.data(), nameBlock.size())) {
        return false;
    }

    if (!(nameBlock.toStrByteArray() == name)) {
        return false;
    }

    return true;
}

bool OveSerialize::readSizeChunk(SizeChunk* sizeChunk)
{
    if (m_streamHandle == 0) {
        return false;
    }

    SizeBlock* sizeBlock = sizeChunk->getSizeBlock();

    if (!m_streamHandle->read((char*)sizeBlock->data(), sizeBlock->size())) {
        return false;
    }

    unsigned int blockSize = sizeBlock->toSize();

    sizeChunk->getDataBlock()->resize(blockSize);

    Block* dataBlock = sizeChunk->getDataBlock();

    if (!m_streamHandle->read((char*)dataBlock->data(), blockSize)) {
        return false;
    }

    return true;
}

bool OveSerialize::readDataChunk(Block* block, unsigned int size)
{
    if (m_streamHandle == 0) {
        return false;
    }

    block->resize(size);

    if (!m_streamHandle->read((char*)block->data(), size)) {
        return false;
    }

    return true;
}

bool OveSerialize::readGroupChunk(GroupChunk* groupChunk)
{
    if (m_streamHandle == 0) {
        return false;
    }

    CountBlock* countBlock = groupChunk->getCountBlock();

    if (!m_streamHandle->read((char*)countBlock->data(), countBlock->size())) {
        return false;
    }

    return true;
}

IOVEStreamLoader* createOveStreamLoader()
{
    return new OveSerialize;
}
}

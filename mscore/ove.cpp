//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "ove.h"

namespace OVE {

/*template <class T>
inline void deleteVector(QList<T*>& vec) {
   for (int i=0; i<vec.size(); ++i)
      delete vec[i];
   }
   //vec.clear();
}*/

///////////////////////////////////////////////////////////////////////////////
TickElement::TickElement() {
      tick_ = 0;
      }

void TickElement::setTick(int tick) {
      tick_ = tick;
      }

int TickElement::getTick(void) const {
      return tick_;
      }

///////////////////////////////////////////////////////////////////////////////
MeasurePos::MeasurePos() {
      measure_ = 0;
      offset_ = 0;
      }

void MeasurePos::setMeasure(int measure) {
      measure_ = measure;
      }

int MeasurePos::getMeasure() const {
      return measure_;
      }

void MeasurePos::setOffset(int offset) {
      offset_ = offset;
      }

int MeasurePos::getOffset() const {
      return offset_;
      }

MeasurePos MeasurePos::shiftMeasure(int measure) const {
      MeasurePos mp;
      mp.setMeasure(getMeasure() + measure);
      mp.setOffset(getOffset());

      return mp;
      }

MeasurePos MeasurePos::shiftOffset(int offset) const {
      MeasurePos mp;
      mp.setMeasure(getMeasure());
      mp.setOffset(getOffset() + offset);

      return mp;
      }

bool MeasurePos::operator ==(const MeasurePos& mp) const {
      return getMeasure() == mp.getMeasure() && getOffset() == mp.getOffset();
      }

bool MeasurePos::operator !=(const MeasurePos& mp) const {
      return !(*this == mp);
      }

bool MeasurePos::operator <(const MeasurePos& mp) const {
      if (getMeasure() != mp.getMeasure()) {
            return getMeasure() < mp.getMeasure();
            }

      return getOffset() < mp.getOffset();
      }

bool MeasurePos::operator <=(const MeasurePos& mp) const {
      if (getMeasure() != mp.getMeasure()) {
            return getMeasure() <= mp.getMeasure();
            }

      return getOffset() <= mp.getOffset();
      }

bool MeasurePos::operator >(const MeasurePos& mp) const {
      return !(*this <= mp);
      }

bool MeasurePos::operator >=(const MeasurePos& mp) const {
      return !(*this < mp);
      }

///////////////////////////////////////////////////////////////////////////////
PairElement::PairElement() {
      start_ = new MeasurePos();
      stop_ = new MeasurePos();
      }

PairElement::~PairElement(){
      delete start_;
      delete stop_;
      }

MeasurePos* PairElement::start() const {
      return start_;
      }

MeasurePos* PairElement::stop() const {
      return stop_;
      }

///////////////////////////////////////////////////////////////////////////////
PairEnds::PairEnds() {
      leftLine_ = new LineElement();
      rightLine_ = new LineElement();
      leftShoulder_ = new OffsetElement();
      rightShoulder_ = new OffsetElement();
      }

PairEnds::~PairEnds(){
      delete leftLine_;
      delete rightLine_;
      delete leftShoulder_;
      delete rightShoulder_;
      }

LineElement* PairEnds::getLeftLine() const {
      return leftLine_;
      }

LineElement* PairEnds::getRightLine() const {
      return rightLine_;
      }

OffsetElement* PairEnds::getLeftShoulder() const {
      return leftShoulder_;
      }

OffsetElement* PairEnds::getRightShoulder() const {
      return rightShoulder_;
      }

///////////////////////////////////////////////////////////////////////////////
LineElement::LineElement() {
      line_ = 0;
      }

void LineElement::setLine(int line) {
      line_ = line;
      }

int LineElement::getLine(void) const {
      return line_;
      }

///////////////////////////////////////////////////////////////////////////////
OffsetElement::OffsetElement() {
      xOffset_ = 0;
      yOffset_ = 0;
      }

void OffsetElement::setXOffset(int offset) {
      xOffset_ = offset;
      }

int OffsetElement::getXOffset() const {
      return xOffset_;
      }

void OffsetElement::setYOffset(int offset) {
      yOffset_ = offset;
      }

int OffsetElement::getYOffset() const {
      return yOffset_;
      }

///////////////////////////////////////////////////////////////////////////////
LengthElement::LengthElement() {
      length_ = 0;
      }

void LengthElement::setLength(int length) {
      length_ = length;
      }

int LengthElement::getLength() const {
      return length_;
      }

///////////////////////////////////////////////////////////////////////////////
MusicData::MusicData() {
      musicDataType_ = MusicDataType::None;
      show_ = true;
      color_ = 0;
      voice_ = 0;
      }

MusicDataType MusicData::getMusicDataType() const {
      return musicDataType_;
      }

MusicData::XmlDataType MusicData::getXmlDataType(MusicDataType type) {
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

/*bool MusicData::get_is_pair_element(MusicDataType type)
 {
 bool pair = false;

 switch ( type )
 {
 case MusicDataType::Numeric_Ending :
 case MusicDataType::Measure_Repeat :
 case MusicDataType::Wedge :
 case MusicDataType::OctaveShift :
 //case MusicDataType::OctaveShift_EndPoint :
 case MusicDataType::Pedal :
 case MusicDataType::Beam :
 case MusicDataType::Glissando :
 case MusicDataType::Slur :
 case MusicDataType::Tie :
 case MusicDataType::Tuplet :
 {
 pair = true;
 break;
 }
 default:
 break;
 }

 return pair;
 }*/

void MusicData::setShow(bool show) {
      show_ = show;
      }

bool MusicData::getShow() const {
      return show_;
      }

void MusicData::setColor(unsigned int color) {
      color_ = color;
      }

unsigned int MusicData::getColor() const {
      return color_;
      }

void MusicData::setVoice(unsigned int voice) {
      voice_ = voice;
      }

unsigned int MusicData::getVoice() const {
      return voice_;
      }

void MusicData::copyCommonBlock(const MusicData& source) {
      setTick(source.getTick());
      start()->setOffset(source.start()->getOffset());
      setColor(source.getColor());
      }

///////////////////////////////////////////////////////////////////////////////
MidiData::MidiData() {
      midiType_ = MidiType::None;
      }

MidiType MidiData::getMidiType() const {
      return midiType_;
      }

///////////////////////////////////////////////////////////////////////////////
OveSong::OveSong() :
      codec_(0) {
      clear();
      }

OveSong::~OveSong() {
      clear();
      }

void OveSong::setIsVersion4(bool version4){
      version4_ = version4;
      }

bool OveSong::getIsVersion4() const {
      return version4_;
      }

void OveSong::setQuarter(int tick) {
      quarter_ = tick;
      }

int OveSong::getQuarter(void) const {
      return quarter_;
      }

void OveSong::setShowPageMargin(bool show){
      showPageMargin_ = show;
      }

bool OveSong::getShowPageMargin() const {
      return showPageMargin_;
      }

void OveSong::setShowTransposeTrack(bool show) {
      showTransposeTrack = show;
      }

bool OveSong::getShowTransposeTrack() const {
      return showTransposeTrack;
      }

void OveSong::setShowLineBreak(bool show) {
      showLineBreak_ = show;
      }

bool OveSong::getShowLineBreak() const {
      return showLineBreak_;
      }

void OveSong::setShowRuler(bool show) {
      showRuler_ = show;
      }

bool OveSong::getShowRuler() const {
      return showRuler_;
      }

void OveSong::setShowColor(bool show) {
      showColor_ = show;
      }

bool OveSong::getShowColor() const {
      return showColor_;
      }

void OveSong::setPlayRepeat(bool play) {
      playRepeat_ = play;
      }

bool OveSong::getPlayRepeat() const {
      return playRepeat_;
      }

void OveSong::setPlayStyle(PlayStyle style) {
      playStyle_ = style;
      }

OveSong::PlayStyle OveSong::getPlayStyle() const {
      return playStyle_;
      }

void OveSong::addTitle(const QString& str) {
      titles_.push_back(str);
      }

QList<QString> OveSong::getTitles(void) const {
      return titles_;
      }

void OveSong::addAnnotate(const QString& str) {
      annotates_.push_back(str);
      }

QList<QString> OveSong::getAnnotates(void) const {
      return annotates_;
      }

void OveSong::addWriter(const QString& str) {
      writers_.push_back(str);
      }

QList<QString> OveSong::getWriters(void) const {
      return writers_;
      }

void OveSong::addCopyright(const QString& str) {
      copyrights_.push_back(str);
      }

QList<QString> OveSong::getCopyrights(void) const {
      return copyrights_;
      }

void OveSong::addHeader(const QString& str) {
      headers_.push_back(str);
      }

QList<QString> OveSong::getHeaders(void) const {
      return headers_;
      }

void OveSong::addFooter(const QString& str) {
      footers_.push_back(str);
      }

QList<QString> OveSong::getFooters(void) const {
      return footers_;
      }

void OveSong::addTrack(Track* ptr) {
      tracks_.push_back(ptr);
      }

int OveSong::getTrackCount(void) const {
      return tracks_.size();
      }

QList<Track*> OveSong::getTracks() const {
      return tracks_;
      }

void OveSong::setTrackBarCount(int count) {
      trackBarCount_ = count;
      }

int OveSong::getTrackBarCount() const {
      return trackBarCount_;
      }

Track* OveSong::getTrack(int part, int staff) const {
      int trackId = partStaffToTrack(part, staff);

      if( trackId >=0 && trackId < (int)tracks_.size() ) {
            return tracks_[trackId];
            }

      return 0;
      }

bool OveSong::addPage(Page* page) {
      pages_.push_back(page);
      return true;
      }

int OveSong::getPageCount() const {
      return pages_.size();
      }

Page* OveSong::getPage(int idx) {
      if( idx>=0 && idx<(int)pages_.size() ) {
            return pages_[idx];
            }

      return 0;
      }

void OveSong::addLine(Line* ptr) {
      lines_.push_back(ptr);
      }

int OveSong::getLineCount() const {
      return lines_.size();
      }

Line* OveSong::getLine(int idx) const {
      if( idx >=0 && idx<(int)lines_.size() ) {
            return lines_[idx];
            }

      return 0;
      }

void OveSong::addMeasure(Measure* ptr) {
      measures_.push_back(ptr);
      }

int OveSong::getMeasureCount(void) const {
      return measures_.size();
      }

Measure* OveSong::getMeasure(int bar) const {
      if( bar >= 0 && bar < (int)measures_.size() ) {
            return measures_[bar];
            }

      return 0;
      }

void OveSong::addMeasureData(MeasureData* ptr) {
      measureDatas_.push_back(ptr);
      }

int OveSong::getMeasureDataCount(void) const {
      return measureDatas_.size();
      }

MeasureData* OveSong::getMeasureData(int part, int staff/*=0*/, int bar) const {
      int trackId = partStaffToTrack(part, staff);
      int trackBarCount = getTrackBarCount();

      if( bar >= 0 && bar < trackBarCount ) {
            int measureId = trackBarCount * trackId + bar;

            if( measureId >=0 && measureId < (int)measureDatas_.size() ) {
                  return measureDatas_[measureId];
                  }
            }

      return 0;
      }

MeasureData* OveSong::getMeasureData(int track, int bar) const {
      int id = trackBarCount_*track + bar;

      if( id >=0 && id < (int)measureDatas_.size() ) {
            return measureDatas_[id];
            }

      return 0;
      }

void OveSong::setPartStaffCounts(const QList<int>& partStaffCounts) {
      //partStaffCounts_.assign(partStaffCounts.begin(), partStaffCounts.end());
      for(int i=0; i<partStaffCounts.size(); ++i) {
            partStaffCounts_.push_back(partStaffCounts[i]);
            }
      }

int OveSong::getPartCount() const {
      return partStaffCounts_.size();
      }

int OveSong::getStaffCount(int part) const {
      if( part>=0 && part<(int)partStaffCounts_.size() ) {
            return partStaffCounts_[part];
            }

      return 0;
      }

int OveSong::getPartBarCount() const {
      return measureDatas_.size() / tracks_.size();
      }

QPair<int, int> OveSong::trackToPartStaff(int track) const {
      QPair<int, int> partStaff;
      int i;
      int staffCount = 0;

      for( i=0; i<partStaffCounts_.size(); ++i ) {
            if( staffCount + partStaffCounts_[i] > track ) {
                  return qMakePair((int)i, track-staffCount);
                  }

            staffCount += partStaffCounts_[i];
            }

      return qMakePair((int)partStaffCounts_.size(), 0);
      }

int OveSong::partStaffToTrack(int part, int staff) const {
      int i;
      unsigned int staffCount = 0;

      for( i=0; i<partStaffCounts_.size(); ++i ) {
            if( part == (int)i && staff>=0 && staff<(int)partStaffCounts_[i] ) {
                  int trackId = staffCount + staff;

                  if( trackId >=0 && trackId < (int)tracks_.size() ) {
                        return trackId;
                        }
                  }

            staffCount += partStaffCounts_[i];
            }

      return tracks_.size();
      }

void OveSong::setTextCodecName(const QString& codecName) {
      codec_ = QTextCodec::codecForName(codecName.toLatin1());
      }

QString OveSong::getCodecString(const QByteArray& text) {
      QString s;
      if (codec_ == NULL)
            s = QString(text);
      else
            s = codec_->toUnicode(text);

      s = s.trimmed();
      return s;
      }

void OveSong::clear(void)
      {
      version4_ = true;
      quarter_ = 480;
      showPageMargin_ = false;
      showTransposeTrack = false;
      showLineBreak_ = false;
      showRuler_ = false;
      showColor_ = true;
      playRepeat_ = true;
      playStyle_ = PlayStyle::Record;

      annotates_.clear();
      copyrights_.clear();
      footers_.clear();
      headers_.clear();
      titles_.clear();
      writers_.clear();

      // deleteVector(tracks_);
      for(int i=0; i<tracks_.size(); ++i){
            delete tracks_[i];
            }
      for(int i=0; i<pages_.size(); ++i){
            delete pages_[i];
            }
      for(int i=0; i<lines_.size(); ++i){
            delete lines_[i];
            }
      for(int i=0; i<measures_.size(); ++i){
            delete measures_[i];
            }
      for(int i=0; i<measureDatas_.size(); ++i){
            delete measureDatas_[i];
            }
      tracks_.clear();
      pages_.clear();
      lines_.clear();
      measures_.clear();
      measureDatas_.clear();
      trackBarCount_ = 0;
      partStaffCounts_.clear();
      }

///////////////////////////////////////////////////////////////////////////////
Voice::Voice() {
      channel_ = 0;
      volume_ = -1;
      pitchShift_ = 0;
      pan_ = 0;
      patch_ = 0;
      stemType_ = 0;
      }

void Voice::setChannel(int channel) {
      channel_ = channel;
      }

int Voice::getChannel() const {
      return channel_;
      }

void Voice::setVolume(int volume) {
      volume_ = volume;
      }

int Voice::getVolume() const {
      return volume_;
      }

void Voice::setPitchShift(int pitchShift) {
      pitchShift_ = pitchShift;
      }

int Voice::getPitchShift() const {
      return pitchShift_;
      }

void Voice::setPan(int pan) {
      pan_ = pan;
      }

int Voice::getPan() const {
      return pan_;
      }

void Voice::setPatch(int patch) {
      patch_ = patch;
      }

int Voice::getPatch() const {
      return patch_;
      }

void Voice::setStemType(int stemType) {
      stemType_ = stemType;
      }

int Voice::getStemType() const {
      return stemType_;
      }

int Voice::getDefaultPatch() {
      return -1;
      }

int Voice::getDefaultVolume() {
      return -1;
      }

///////////////////////////////////////////////////////////////////////////////
Track::Track() {
      clear();
      }

Track::~Track() {
      clear();
      }

void Track::setName(const QString& str) {
      name_ = str;
      }

QString Track::getName(void) const {
      return name_;
      }

void Track::setBriefName(const QString& str) {
      briefName_ = str;
      }

QString Track::getBriefName(void) const {
      return briefName_;
      }

void Track::setPatch(unsigned int patch) {
      patch_ = patch;
      }

unsigned int Track::getPatch() const {
      return patch_;
      }

void Track::setChannel(int channel) {
      channel_ = channel;
      }

int Track::getChannel() const {
      return channel_;
      }

void Track::setShowName(bool show) {
      showName_ = show;
      }

bool Track::getShowName() const {
      return showName_;
      }

void Track::setShowBriefName(bool show) {
      showBriefName_ = show;
      }

bool Track::getShowBriefName() const {
      return showBriefName_;
      }

void Track::setMute(bool mute) {
      mute_ = mute;
      }

bool Track::getMute() const {
      return mute_;
      }

void Track::setSolo(bool solo) {
      solo_ = solo;
      }

bool Track::getSolo() const {
      return solo_;
      }

void Track::setShowKeyEachLine(bool show) {
      showKeyEachLine_ = show;
      }

bool Track::getShowKeyEachLine() const {
      return showKeyEachLine_;
      }

void Track::setVoiceCount(int voices) {
      voiceCount_ = voices;
      }

int Track::getVoiceCount() const {
      return voiceCount_;
      }

void Track::addVoice(Voice* voice) {
      voices_.push_back(voice);
      }

QList<Voice*> Track::getVoices() const {
      return voices_;
      }

void Track::setShowTranspose(bool show) {
      showTranspose_ = show;
      }

bool Track::getShowTranspose() const {
      return showTranspose_;
      }

void Track::setTranspose(int transpose) {
      transpose_ = transpose;
      }

int Track::getTranspose() const {
      return transpose_;
      }

void Track::setNoteShift(int shift) {
      noteShift_ = shift;
      }

int Track::getNoteShift() const {
      return noteShift_;
      }

void Track::setStartClef(int clef/*in Clef*/) {
      startClef_ = ClefType(clef);
      }

ClefType Track::getStartClef() const {
      return startClef_;
      }

void Track::setTransposeClef(int clef) {
      transposeClef_ = ClefType(clef);
      }

ClefType Track::getTansposeClef() const {
      return transposeClef_;
      }

void Track::setStartKey(int key) {
      startKey_ = key;
      }

int Track::getStartKey() const {
      return startKey_;
      }

void Track::setDisplayPercent(unsigned int percent/*25~100?*/) {
      displayPercent_ = percent;
      }

unsigned int Track::getDisplayPercent() const {
      return displayPercent_;
      }

void Track::setShowLegerLine(bool show) {
      showLegerLine_ = show;
      }

bool Track::getShowLegerLine() const {
      return showLegerLine_;
      }

void Track::setShowClef(bool show) {
      showClef_ = show;
      }

bool Track::getShowClef() const {
      return showClef_;
      }

void Track::setShowTimeSignature(bool show) {
      showTimeSignature_ = show;
      }

bool Track::getShowTimeSignature() const {
      return showTimeSignature_;
      }

void Track::setShowKeySignature(bool show) {
      showKeySignature_ = show;
      }

bool Track::getShowKeySignature() const {
      return showKeySignature_;
      }

void Track::setShowBarline(bool show) {
      showBarline_ = show;
      }

bool Track::getShowBarline() const {
      return showBarline_;
      }

void Track::setFillWithRest(bool fill) {
      fillWithRest_ = fill;
      }

bool Track::getFillWithRest() const {
      return fillWithRest_;
      }

void Track::setFlatTail(bool flat) {
      flatTail_ = flat;
      }

bool Track::getFlatTail() const {
      return flatTail_;
      }

void Track::setShowClefEachLine(bool show) {
      showClefEachLine_ = show;
      }

bool Track::getShowClefEachLine() const {
      return showClefEachLine_;
      }

void Track::addDrum(const DrumNode& node) {
      /*DrumNode node;
   node.line_ = line;
   node.headType_ = headType;
   node.pitch_ = pitch;
   node.voice_ = voice;*/
      drumKit_.push_back(node);
      }

QList<Track::DrumNode> Track::getDrumKit() const {
      return drumKit_;
      }

void Track::setPart(int part) {
      part_ = part;
      }

int Track::getPart() const {
      return part_;
      }

void Track::clear(void) {
      number_ = 0;

      name_ = QString();

      patch_ = 0;
      channel_ = 0;
      transpose_ = 0;
      showTranspose_ = false;
      noteShift_ = 0;
      startClef_ = ClefType::Treble;
      transposeClef_ = ClefType::Treble;
      displayPercent_ = 100;
      startKey_ = 0;
      voiceCount_ = 8;

      showName_ = true;
      showBriefName_ = false;
      showKeyEachLine_ = false;
      showLegerLine_ = true;
      showClef_ = true;
      showTimeSignature_ = true;
      showKeySignature_ = true;
      showBarline_ = true;
      showClefEachLine_ = false;

      fillWithRest_ = true;
      flatTail_ = false;

      mute_ = false;
      solo_ = false;

      drumKit_.clear();

      part_ = 0;

      for(int i=0; i<voices_.size(); ++i){
            delete voices_[i];
            }
      voices_.clear();
      }

///////////////////////////////////////////////////////////////////////////////
Page::Page() {
      beginLine_ = 0;
      lineCount_ = 0;

      lineInterval_ = 9;
      staffInterval_ = 7;
      staffInlineInterval_ = 6;

      lineBarCount_ = 4;
      pageLineCount_ = 5;

      leftMargin_ = 0xA8;
      topMargin_ = 0xA8;
      rightMargin_ = 0xA8;
      bottomMargin_ = 0xA8;

      pageWidth_ = 0x0B40;
      pageHeight_ = 0x0E90;
      }

void Page::setBeginLine(int line) {
      beginLine_ = line;
      }

int Page::getBeginLine() const {
      return beginLine_;
      }

void Page::setLineCount(int count) {
      lineCount_ = count;
      }

int Page::getLineCount() const {
      return lineCount_;
      }

void Page::setLineInterval(int interval) {
      lineInterval_ = interval;
      }

int Page::getLineInterval() const {
      return lineInterval_;
      }

void Page::setStaffInterval(int interval) {
      staffInterval_ = interval;
      }

int Page::getStaffInterval() const {
      return staffInterval_;
      }

void Page::setStaffInlineInterval(int interval) {
      staffInlineInterval_ = interval;
      }

int Page::getStaffInlineInterval() const {
      return staffInlineInterval_;
      }

void Page::setLineBarCount(int count) {
      lineBarCount_ = count;
      }

int Page::getLineBarCount() const {
      return lineBarCount_;
      }

void Page::setPageLineCount(int count) {
      pageLineCount_ = count;
      }

int Page::getPageLineCount() const {
      return pageLineCount_;
      }

void Page::setLeftMargin(int margin) {
      leftMargin_ = margin;
      }

int Page::getLeftMargin() const {
      return leftMargin_;
      }

void Page::setTopMargin(int margin) {
      topMargin_ = margin;
      }

int Page::getTopMargin() const {
      return topMargin_;
      }

void Page::setRightMargin(int margin) {
      rightMargin_ = margin;
      }

int Page::getRightMargin() const {
      return rightMargin_;
      }

void Page::setBottomMargin(int margin) {
      bottomMargin_ = margin;
      }

int Page::getBottomMargin() const {
      return bottomMargin_;
      }

void Page::setPageWidth(int width) {
      pageWidth_ = width;
      }

int Page::getPageWidth() const {
      return pageWidth_;
      }

void Page::setPageHeight(int height) {
      pageHeight_ = height;
      }

int Page::getPageHeight() const {
      return pageHeight_;
      }

///////////////////////////////////////////////////////////////////////////////
Line::Line() {
      beginBar_ = 0;
      barCount_ = 0;
      yOffset_ = 0;
      leftXOffset_ = 0;
      rightXOffset_ = 0;
      }

Line::~Line() {
      for(int i=0; i<staffs_.size(); ++i){
            delete staffs_[i];
            }
      staffs_.clear();
      }

void Line::addStaff(Staff* staff) {
      staffs_.push_back(staff);
      }

int Line::getStaffCount() const {
      return staffs_.size();
      }

Staff* Line::getStaff(int idx) const {
      if (idx >= 0 && idx < (int) staffs_.size()) {
            return staffs_[idx];
            }

      return 0;
      }

void Line::setBeginBar(unsigned int bar) {
      beginBar_ = bar;
      }

unsigned int Line::getBeginBar() const {
      return beginBar_;
      }

void Line::setBarCount(unsigned int count) {
      barCount_ = count;
      }

unsigned int Line::getBarCount() const {
      return barCount_;
      }

void Line::setYOffset(int offset) {
      yOffset_ = offset;
      }

int Line::getYOffset() const {
      return yOffset_;
      }

void Line::setLeftXOffset(int offset) {
      leftXOffset_ = offset;
      }

int Line::getLeftXOffset() const {
      return leftXOffset_;
      }

void Line::setRightXOffset(int offset) {
      rightXOffset_ = offset;
      }

int Line::getRightXOffset() const {
      return rightXOffset_;
      }

///////////////////////////////////////////////////////////////////////////////
Staff::Staff() {
      clef_ = ClefType::Treble;
      key_ = 0;
      visible_ = true;
      groupType_ = GroupType::None;
      groupStaffCount_ = 0;
      }

void Staff::setClefType(int clef) {
      clef_ = (ClefType) clef;
      }

ClefType Staff::getClefType() const {
      return clef_;
      }

void Staff::setKeyType(int key) {
      key_ = key;
      }

int Staff::getKeyType() const {
      return key_;
      }

void Staff::setVisible(bool visible) {
      visible_ = visible;
      }

bool Staff::setVisible() const {
      return visible_;
      }

void Staff::setGroupType(GroupType type){
      groupType_ = type;
      }

GroupType Staff::getGroupType() const {
      return groupType_;
      }

void Staff::setGroupStaffCount(int count) {
      groupStaffCount_ = count;
      }

int Staff::getGroupStaffCount() const {
      return groupStaffCount_;
      }

///////////////////////////////////////////////////////////////////////////////
Note::Note() {
      rest_ = false;
      note_ = 60;
      accidental_ = AccidentalType::Normal;
      showAccidental_ = false;
      offVelocity_ = 0x40;
      onVelocity_ = 0x50;
      headType_ = NoteHeadType::Standard;
      tiePos_ = TiePos::None;
      offsetStaff_ = 0;
      show_ = true;
      offsetTick_ = 0;
      }

void Note::setIsRest(bool rest) {
      rest_ = rest;
      }

bool Note::getIsRest() const {
      return rest_;
      }

void Note::setNote(unsigned int note) {
      note_ = note;
      }

unsigned int Note::getNote() const {
      return note_;
      }

void Note::setAccidental(int type) {
      accidental_ = (AccidentalType) type;
      }

AccidentalType Note::getAccidental() const {
      return accidental_;
      }

void Note::setShowAccidental(bool show) {
      showAccidental_ = show;
      }

bool Note::getShowAccidental() const {
      return showAccidental_;
      }

void Note::setOnVelocity(unsigned int velocity) {
      onVelocity_ = velocity;
      }

unsigned int Note::getOnVelocity() const {
      return onVelocity_;
      }

void Note::setOffVelocity(unsigned int velocity) {
      offVelocity_ = velocity;
      }

unsigned int Note::getOffVelocity() const {
      return offVelocity_;
      }

void Note::setHeadType(int type) {
      headType_ = (NoteHeadType) type;
      }

NoteHeadType Note::getHeadType() const {
      return headType_;
      }

void Note::setTiePos(int tiePos) {
      tiePos_ = (TiePos) tiePos;
      }

TiePos Note::getTiePos() const {
      return tiePos_;
      }

void Note::setOffsetStaff(int offset) {
      offsetStaff_ = offset;
      }

int Note::getOffsetStaff() const {
      return offsetStaff_;
      }

void Note::setShow(bool show) {
      show_ = show;
      }

bool Note::getShow() const {
      return show_;
      }

void Note::setOffsetTick(int offset) {
      offsetTick_ = offset;
      }

int Note::getOffsetTick() const {
      return offsetTick_;
      }

///////////////////////////////////////////////////////////////////////////////
Articulation::Articulation() {
      type_ = ArticulationType::Marcato;
      above_ = true;

      changeSoundEffect_ = false;
      changeLength_ = false;
      changeVelocity_ = false;
      changeExtraLength_ = false;

      soundEffect_ = qMakePair(0, 0);
      lengthPercentage_ = 100;
      velocityType_ = VelocityType::Offset;
      velocityValue_ = 0;
      extraLength_ = 0;

      trillNoteLength_ = 60;
      trillRate_ = NoteType::Note_Sixteen;
      accelerateType_ = AccelerateType::None;
      auxiliaryFirst_ = false;
      trillInterval_ = TrillInterval::Chromatic;
      }

void Articulation::setArtType(int type) {
      type_ = (ArticulationType) type;
      }

ArticulationType Articulation::getArtType() const {
      return type_;
      }

void Articulation::setPlacementAbove(bool above) {
      above_ = above;
      }

bool Articulation::getPlacementAbove() const {
      return above_;
      }

bool Articulation::getChangeSoundEffect() const {
      return changeSoundEffect_;
      }

void Articulation::setSoundEffect(int soundFrom, int soundTo) {
      soundEffect_ = qMakePair(soundFrom, soundTo);
      changeSoundEffect_ = true;
      }

QPair<int, int> Articulation::getSoundEffect() const {
      return soundEffect_;
      }

bool Articulation::getChangeLength() const {
      return changeLength_;
      }

void Articulation::setLengthPercentage(int percentage) {
      lengthPercentage_ = percentage;
      changeLength_ = true;
      }

int Articulation::getLengthPercentage() const {
      return lengthPercentage_;
      }

bool Articulation::getChangeVelocity() const {
      return changeVelocity_;
      }

void Articulation::setVelocityType(VelocityType type) {
      velocityType_ = type;
      changeVelocity_ = true;
      }

Articulation::VelocityType Articulation::getVelocityType() const {
      return velocityType_;
      }

void Articulation::setVelocityValue(int value) {
      velocityValue_ = value;
      }

int Articulation::getVelocityValue() const {
      return velocityValue_;
      }

bool Articulation::getChangeExtraLength() const {
      return changeExtraLength_;
      }

void Articulation::setExtraLength(int length) {
      extraLength_ = length;
      changeExtraLength_ = true;
      }

int Articulation::getExtraLength() const {
      return extraLength_;
      }

void Articulation::setTrillNoteLength(int length) {
      trillNoteLength_ = length;
      }

int Articulation::getTrillNoteLength() const {
      return trillNoteLength_;
      }

void Articulation::setTrillRate(NoteType rate) {
      trillRate_ = rate;
      }

NoteType Articulation::getTrillRate() const {
      return trillRate_;
      }

void Articulation::setAccelerateType(int type) {
      accelerateType_ = (AccelerateType) type;
      }

Articulation::AccelerateType Articulation::getAccelerateType() const {
      return accelerateType_;
      }

void Articulation::setAuxiliaryFirst(bool first) {
      auxiliaryFirst_ = first;
      }

bool Articulation::getAuxiliaryFirst() const {
      return auxiliaryFirst_;
      }

void Articulation::setTrillInterval(int interval) {
      trillInterval_ = (TrillInterval) interval;
      }

Articulation::TrillInterval Articulation::getTrillInterval() const {
      return trillInterval_;
      }

bool Articulation::willAffectNotes() const {
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

bool Articulation::isTrill(ArticulationType type) {
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

Articulation::XmlType Articulation::getXmlType() const {
      XmlType xmlType = XmlType::Unknown;

      switch (type_) {
            case ArticulationType::Major_Trill:
            case ArticulationType::Minor_Trill:
            case ArticulationType::Trill_Section:
            case ArticulationType::Inverted_Short_Mordent:
            case ArticulationType::Inverted_Long_Mordent:
            case ArticulationType::Short_Mordent:
            case ArticulationType::Turn:
                  // case ArticulationType::Flat_Accidental_For_Trill :
                  // case ArticulationType::Sharp_Accidental_For_Trill :
                  // case ArticulationType::Natural_Accidental_For_Trill :
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
                  // case ArticulationType::Toe_Pedal :
                  // case ArticulationType::Heel_Pedal :
                  // case ArticulationType::Toe_To_Heel_Pedal :
                  // case ArticulationType::Heel_To_Toe_Pedal :
                  // case ArticulationType::Open_String :
            default:
                  break;
            }

      return xmlType;
      }

///////////////////////////////////////////////////////////////////////////////
NoteContainer::NoteContainer() {
      musicDataType_ = MusicDataType::Note_Container;

      grace_ = false;
      cue_ = false;
      rest_ = false;
      raw_ = false;
      noteType_ = NoteType::Note_Quarter;
      dot_ = 0;
      graceNoteType_ = NoteType::Note_Eight;
      stemUp_ = true;
      showStem_ = true;
      stemLength_ = 7;
      inBeam_ = false;
      tuplet_ = 0;
      space_ = 2;//div by 0
      noteShift_ = 0;
      }

NoteContainer::~NoteContainer(){
      for(int i=0; i<notes_.size(); ++i){
            delete notes_[i];
            }
      for(int i=0; i<articulations_.size(); ++i){
            delete articulations_[i];
            }
      notes_.clear();
      articulations_.clear();
      }

void NoteContainer::setIsGrace(bool grace) {
      grace_ = grace;
      }

bool NoteContainer::getIsGrace() const {
      return grace_;
      }

void NoteContainer::setIsCue(bool cue) {
      cue_ = cue;
      }

bool NoteContainer::getIsCue() const {
      return cue_;
      }

void NoteContainer::setIsRest(bool rest) {
      rest_ = rest;
      }

bool NoteContainer::getIsRest() const {
      return rest_;
      }

void NoteContainer::setIsRaw(bool raw) {
      raw_ = raw;
      }

bool NoteContainer::getIsRaw() const {
      return raw_;
      }

void NoteContainer::setNoteType(NoteType type) {
      noteType_ = NoteType::Note_Quarter;

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
                  noteType_ = type;
                  break;
                  }
            default: {
                  break;
                  }
            }
      }

NoteType NoteContainer::getNoteType() const {
      return noteType_;
      }

void NoteContainer::setDot(int dot) {
      dot_ = dot;
      }

int NoteContainer::getDot() const {
      return dot_;
      }

void NoteContainer::setGraceNoteType(NoteType type) {
      graceNoteType_ = type;
      }

NoteType NoteContainer::getGraceNoteType() const {
      return graceNoteType_;
      }

void NoteContainer::setInBeam(bool in) {
      inBeam_ = in;
      }

bool NoteContainer::getInBeam() const {
      return inBeam_;
      }

void NoteContainer::setStemUp(bool up) {
      stemUp_ = up;
      }

bool NoteContainer::getStemUp(void) const {
      return stemUp_;
      }

void NoteContainer::setShowStem(bool show) {
      showStem_ = show;
      }

bool NoteContainer::getShowStem() const {
      return showStem_;
      }

void NoteContainer::setStemLength(int line) {
      stemLength_ = line;
      }

int NoteContainer::getStemLength() const {
      return stemLength_;
      }

void NoteContainer::setTuplet(int tuplet) {
      tuplet_ = tuplet;
      }

int NoteContainer::getTuplet() const {
      return tuplet_;
      }

void NoteContainer::setSpace(int space) {
      space_ = space;
      }

int NoteContainer::getSpace() const {
      return space_;
      }

void NoteContainer::addNoteRest(Note* note) {
      notes_.push_back(note);
      }

QList<Note*> NoteContainer::getNotesRests() const {
      return notes_;
      }

void NoteContainer::addArticulation(Articulation* art) {
      articulations_.push_back(art);
      }

QList<Articulation*> NoteContainer::getArticulations() const {
      return articulations_;
      }

void NoteContainer::setNoteShift(int octave) {
      noteShift_ = octave;
      }

int NoteContainer::getNoteShift() const {
      return noteShift_;
      }

int NoteContainer::getOffsetStaff() const {
      if(getIsRest())
            return 0;

      int staffMove = 0;
      QList<OVE::Note*> notes = getNotesRests();
      for (int i = 0; i < notes.size(); ++i) {
            OVE::Note* notePtr = notes[i];
            staffMove = notePtr->getOffsetStaff();
            }

      return staffMove;
      }

int NoteContainer::getDuration() const {
      int duration = (int) NoteDuration::D_4;

      switch (noteType_) {
            case NoteType::Note_DoubleWhole: {
                  duration = (int) NoteDuration::D_Double_Whole;
                  break;
                  }
            case NoteType::Note_Whole: {
                  duration = (int) NoteDuration::D_Whole;
                  break;
                  }
            case NoteType::Note_Half: {
                  duration = (int) NoteDuration::D_2;
                  break;
                  }
            case NoteType::Note_Quarter: {
                  duration = (int) NoteDuration::D_4;
                  break;
                  }
            case NoteType::Note_Eight: {
                  duration = (int) NoteDuration::D_8;
                  break;
                  }
            case NoteType::Note_Sixteen: {
                  duration = (int) NoteDuration::D_16;
                  break;
                  }
            case NoteType::Note_32: {
                  duration = (int) NoteDuration::D_32;
                  break;
                  }
            case NoteType::Note_64: {
                  duration = (int) NoteDuration::D_64;
                  break;
                  }
            case NoteType::Note_128: {
                  duration = (int) NoteDuration::D_128;
                  break;
                  }
            case NoteType::Note_256: {
                  duration = (int) NoteDuration::D_256;
                  break;
                  }
            default:
                  break;
            }

      int dotLength = duration;

      for (int i = 0; i < dot_; ++i) {
            dotLength /= 2;
            }

      dotLength = duration - dotLength;

      duration += dotLength;

      return duration;
      }

///////////////////////////////////////////////////////////////////////////////
Beam::Beam() {
      musicDataType_ = MusicDataType::Beam;
      grace_ = false;
      }

void Beam::setIsGrace(bool grace) {
      grace_ = grace;
      }

bool Beam::getIsGrace() const {
      return grace_;
      }

void Beam::addLine(const MeasurePos& startMp, const MeasurePos& endMp) {
      lines_.push_back(qMakePair(startMp, endMp));
      }

const QList<QPair<MeasurePos, MeasurePos> > Beam::getLines() const {
      return lines_;
      }

///////////////////////////////////////////////////////////////////////////////
Tie::Tie() {
      musicDataType_ = MusicDataType::Tie;

      showOnTop_ = true;
      note_ = 72;
      height_ = 24;
      }

void Tie::setShowOnTop(bool top) {
      showOnTop_ = top;
      }

bool Tie::getShowOnTop() const {
      return showOnTop_;
      }

void Tie::setNote(int note) {
      note_ = note;
      }

int Tie::getNote() const {
      return note_;
      }

void Tie::setHeight(int height) {
      height_ = height;
      }

int Tie::getHeight() const {
      return height_;
      }

///////////////////////////////////////////////////////////////////////////////
Glissando::Glissando() {
      musicDataType_ = MusicDataType::Glissando;

      straight_ = true;
      text_ = "gliss.";
      lineThick_ = 8;
      }

void Glissando::setStraightWavy(bool straight) {
      straight_ = straight;
      }

bool Glissando::getStraightWavy() const {
      return straight_;
      }

void Glissando::setText(const QString& text) {
      text_ = text;
      }

QString Glissando::getText() const {
      return text_;
      }

void Glissando::setLineThick(int thick) {
      lineThick_ = thick;
      }

int Glissando::getLineThick() const {
      return lineThick_;
      }

///////////////////////////////////////////////////////////////////////////////
Decorator::Decorator() :
      decoratorType_(Type::Articulation),
      artType_(ArticulationType::Marcato) {
      musicDataType_ = MusicDataType::Decorator;
      }

void Decorator::setDecoratorType(Type type) {
      decoratorType_ = type;
      }

Decorator::Type Decorator::getDecoratorType() const {
      return decoratorType_;
      }

void Decorator::setArticulationType(ArticulationType type) {
      artType_ = type;
      }

ArticulationType Decorator::getArticulationType() const {
      return artType_;
      }

///////////////////////////////////////////////////////////////////////////////
MeasureRepeat::MeasureRepeat() {
      musicDataType_ = MusicDataType::Measure_Repeat;
      singleRepeat_ = true;
      }

void MeasureRepeat::setSingleRepeat(bool single) {
      singleRepeat_ = single;

      start()->setMeasure(0);
      start()->setOffset(0);
      stop()->setMeasure(single ? 1 : 2);
      stop()->setOffset(0);
      }

bool MeasureRepeat::getSingleRepeat() const {
      return singleRepeat_;
      }

///////////////////////////////////////////////////////////////////////////////
Tuplet::Tuplet() :
      tuplet_(3), space_(2), height_(0), noteType_(NoteType::Note_Quarter){
      musicDataType_ = MusicDataType::Tuplet;
      mark_ = new OffsetElement();
      }

Tuplet::~Tuplet(){
      delete mark_;
      }

void Tuplet::setTuplet(int tuplet) {
      tuplet_ = tuplet;
      }

int Tuplet::getTuplet() const {
      return tuplet_;
      }

void Tuplet::setSpace(int space) {
      space_ = space;
      }

int Tuplet::getSpace() const {
      return space_;
      }

OffsetElement* Tuplet::getMarkHandle() const {
      return mark_;
      }

void Tuplet::setHeight(int height) {
      height_ = height;
      }

int Tuplet::getHeight() const {
      return height_;
      }

void Tuplet::setNoteType(NoteType type) {
      noteType_ = type;
      }

NoteType Tuplet::getNoteType() const {
      return noteType_;
      }

///////////////////////////////////////////////////////////////////////////////
Harmony::Harmony() {
      musicDataType_ = MusicDataType::Harmony;

      harmonyType_ = "";
      root_ = 0;
      bass_ = -1; //0xff
      alterRoot_ = 0;
      alterBass_ = 0;
      bassOnBottom_ = false;
      angle_ = 0;
      }

void Harmony::setHarmonyType(QString type) {
      harmonyType_ = type;
      }

QString Harmony::getHarmonyType() const {
      return harmonyType_;
      }

void Harmony::setRoot(int root) {
      root_ = root;
      }

int Harmony::getRoot() const {
      return root_;
      }

void Harmony::setAlterRoot(int val) {
      alterRoot_ = val;
      }

int Harmony::getAlterRoot() const {
      return alterRoot_;
      }

void Harmony::setBass(int bass) {
      bass_ = bass;
      }

int Harmony::getBass() const {
      return bass_;
      }

void Harmony::setAlterBass(int val) {
      alterBass_ = val;
      }

int Harmony::getAlterBass() const {
      return alterBass_;
      }

void Harmony::setBassOnBottom(bool on) {
      bassOnBottom_ = on;
      }

bool Harmony::getBassOnBottom() const {
      return bassOnBottom_;
      }

void Harmony::setAngle(int angle) {
      angle_ = angle;
      }

int Harmony::getAngle() const {
      return angle_;
      }

///////////////////////////////////////////////////////////////////////////////
Clef::Clef() {
      musicDataType_ = MusicDataType::Clef;

      clefType_ = ClefType::Treble;
      }

void Clef::setClefType(int type) {
      clefType_ = (ClefType) type;
      }

ClefType Clef::getClefType() const {
      return clefType_;
      }

///////////////////////////////////////////////////////////////////////////////
Lyric::Lyric() {
      musicDataType_ = MusicDataType::Lyric;

      lyric_ = QString();
      verse_ = 0;
      }

void Lyric::setLyric(const QString& lyricText) {
      lyric_ = lyricText;
      }

QString Lyric::getLyric() const {
      return lyric_;
      }

void Lyric::setVerse(int verse) {
      verse_ = verse;
      }

int Lyric::getVerse() const {
      return verse_;
      }

///////////////////////////////////////////////////////////////////////////////
Slur::Slur() {
      musicDataType_ = MusicDataType::Slur;

      containerCount_ = 1;
      showOnTop_ = true;
      noteTimePercent_ = 100;

      handle_2_ = new OffsetElement();
      handle_3_ = new OffsetElement();
      }

Slur::~Slur() {
      delete handle_2_;
      delete handle_3_;
      }

void Slur::setContainerCount(int count) {
      containerCount_ = count;
      }

int Slur::getContainerCount() const {
      return containerCount_;
      }

void Slur::setShowOnTop(bool top) {
      showOnTop_ = top;
      }

bool Slur::getShowOnTop() const {
      return showOnTop_;
      }

OffsetElement* Slur::getHandle2() const {
      return handle_2_;
      }

OffsetElement* Slur::getHandle3() const {
      return handle_3_;
      }

void Slur::setNoteTimePercent(int percent) {
      noteTimePercent_ = percent;
      }

int Slur::getNoteTimePercent() const {
      return noteTimePercent_;
      }

///////////////////////////////////////////////////////////////////////////////
Dynamics::Dynamics() {
      musicDataType_ = MusicDataType::Dynamics;

      dynamicsType_ = DynamicsType::PPPP;
      playback_ = true;
      velocity_ = 30;
      }

void Dynamics::setDynamicsType(int type) {
      dynamicsType_ = DynamicsType(type);
      }

DynamicsType Dynamics::getDynamicsType() const {
      return dynamicsType_;
      }

void Dynamics::setIsPlayback(bool play) {
      playback_ = play;
      }

bool Dynamics::getIsPlayback() const {
      return playback_;
      }

void Dynamics::setVelocity(int vel) {
      velocity_ = vel;
      }

int Dynamics::getVelocity() const {
      return velocity_;
      }

///////////////////////////////////////////////////////////////////////////////
WedgeEndPoint::WedgeEndPoint() {
      musicDataType_ = MusicDataType::Wedge_EndPoint;

      wedgeType_ = WedgeType::Cres;
      height_ = 24;
      wedgeStart_ = true;
      }

void WedgeEndPoint::setWedgeType(WedgeType type) {
      wedgeType_ = type;
      }

WedgeType WedgeEndPoint::getWedgeType() const {
      return wedgeType_;
      }

void WedgeEndPoint::setHeight(int height) {
      height_ = height;
      }

int WedgeEndPoint::getHeight() const {
      return height_;
      }

void WedgeEndPoint::setWedgeStart(bool wedgeStart) {
      wedgeStart_ = wedgeStart;
      }

bool WedgeEndPoint::getWedgeStart() const {
      return wedgeStart_;
      }

///////////////////////////////////////////////////////////////////////////////
Wedge::Wedge() {
      musicDataType_ = MusicDataType::Wedge;

      wedgeType_ = WedgeType::Cres;
      height_ = 24;
      }

void Wedge::setWedgeType(WedgeType type) {
      wedgeType_ = type;
      }

WedgeType Wedge::getWedgeType() const {
      return wedgeType_;
      }

void Wedge::setHeight(int height) {
      height_ = height;
      }

int Wedge::getHeight() const {
      return height_;
      }

///////////////////////////////////////////////////////////////////////////////
Pedal::Pedal() {
      musicDataType_ = MusicDataType::Pedal;

      half_ = false;
      playback_ = false;
      playOffset_ = 0;

      pedalHandle_ = new OffsetElement();
      }

Pedal::~Pedal() {
      delete pedalHandle_;
      }

void Pedal::setHalf(bool half) {
      half_ = half;
      }

bool Pedal::getHalf() const {
      return half_;
      }

OffsetElement* Pedal::getPedalHandle() const {
      return pedalHandle_;
      }

void Pedal::setIsPlayback(bool playback) {
      playback_ = playback;
      }

bool Pedal::getIsPlayback() const {
      return playback_;
      }

void Pedal::setPlayOffset(int offset) {
      playOffset_ = offset;
      }

int Pedal::getPlayOffset() const {
      return playOffset_;
      }

///////////////////////////////////////////////////////////////////////////////
KuoHao::KuoHao() {
      musicDataType_ = MusicDataType::KuoHao;

      kuohaoType_ = KuoHaoType::Parentheses;
      height_ = 0;
      }

void KuoHao::setHeight(int height) {
      height_ = height;
      }

int KuoHao::getHeight() const {
      return height_;
      }

void KuoHao::setKuohaoType(int type) {
      kuohaoType_ = (KuoHaoType) type;
      }

KuoHaoType KuoHao::getKuohaoType() const {
      return kuohaoType_;
      }

///////////////////////////////////////////////////////////////////////////////
Expressions::Expressions() {
      musicDataType_ = MusicDataType::Expressions;

      text_ = QString();
      }

void Expressions::setText(const QString& str) {
      text_ = str;
      }

QString Expressions::getText() const {
      return text_;
      }

///////////////////////////////////////////////////////////////////////////////
HarpPedal::HarpPedal() :
      showType_(0),
      showCharFlag_(0) {
      musicDataType_ = MusicDataType::Harp_Pedal;
      }

void HarpPedal::setShowType(int type) {
      showType_ = type;
      }

int HarpPedal::getShowType() const {
      return showType_;
      }

void HarpPedal::setShowCharFlag(int flag) {
      showCharFlag_ = flag;
      }

int HarpPedal::getShowCharFlag() const {
      return showCharFlag_;
      }

///////////////////////////////////////////////////////////////////////////////
OctaveShift::OctaveShift() :
      octaveShiftType_(OctaveShiftType::OS_8),
      octaveShiftPosition_(OctaveShiftPosition::Start),
      endTick_(0) {
      musicDataType_ = MusicDataType::OctaveShift;
      }

void OctaveShift::setOctaveShiftType(OctaveShiftType type) {
      octaveShiftType_ = type;
      }

OctaveShiftType OctaveShift::getOctaveShiftType() const {
      return octaveShiftType_;
      }

int OctaveShift::getNoteShift() const {
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

void OctaveShift::setEndTick(int tick) {
      endTick_ = tick;
      }

int OctaveShift::getEndTick() const {
      return endTick_;
      }

void OctaveShift::setOctaveShiftPosition(OctaveShiftPosition position) {
      octaveShiftPosition_ = position;
      }

OctaveShiftPosition OctaveShift::getOctaveShiftPosition() const {
      return octaveShiftPosition_;
      }

///////////////////////////////////////////////////////////////////////////////
OctaveShiftEndPoint::OctaveShiftEndPoint() {
      musicDataType_ = MusicDataType::OctaveShift_EndPoint;

      octaveShiftType_ = OctaveShiftType::OS_8;
      octaveShiftPosition_ = OctaveShiftPosition::Start;
      endTick_ = 0;
      }

void OctaveShiftEndPoint::setOctaveShiftType(OctaveShiftType type) {
      octaveShiftType_ = type;
      }

OctaveShiftType OctaveShiftEndPoint::getOctaveShiftType() const {
      return octaveShiftType_;
      }

void OctaveShiftEndPoint::setOctaveShiftPosition(OctaveShiftPosition position) {
      octaveShiftPosition_ = position;
      }

OctaveShiftPosition OctaveShiftEndPoint::getOctaveShiftPosition() const {
      return octaveShiftPosition_;
      }

void OctaveShiftEndPoint::setEndTick(int tick) {
      endTick_ = tick;
      }

int OctaveShiftEndPoint::getEndTick() const {
      return endTick_;
      }

///////////////////////////////////////////////////////////////////////////////
MultiMeasureRest::MultiMeasureRest() {
      musicDataType_ = MusicDataType::Multi_Measure_Rest;
      measureCount_ = 0;
      }

void MultiMeasureRest::setMeasureCount(int count) {
      measureCount_ = count;
      }

int MultiMeasureRest::getMeasureCount() const {
      return measureCount_;
      }

///////////////////////////////////////////////////////////////////////////////
Tempo::Tempo() {
      musicDataType_ = MusicDataType::Tempo;

      leftNoteType_ = 3;
      showMark_ = false;
      showText_ = false;
      showParenthesis_ = false;
      typeTempo_ = 96;
      leftText_ = QString();
      rightText_ = QString();
      swingEighth_ = false;
      rightNoteType_ = 3;
      leftNoteDot_ = false;
      rightNoteDot_ = false;
      rightSideType_ = 0;
      }

void Tempo::setLeftNoteType(int type) {
      leftNoteType_ = type;
      }

NoteType Tempo::getLeftNoteType() const {
      return (NoteType) leftNoteType_;
      }

void Tempo::setShowMark(bool show) {
      showMark_ = show;
      }

bool Tempo::getShowMark() const {
      return showMark_;
      }

void Tempo::setShowBeforeText(bool show) {
      showText_ = show;
      }

bool Tempo::getShowBeforeText() const {
      return showText_;
      }

void Tempo::setShowParenthesis(bool show) {
      showParenthesis_ = show;
      }

bool Tempo::getShowParenthesis() const {
      return showParenthesis_;
      }

void Tempo::setTypeTempo(double tempo) {
      typeTempo_ = tempo;
      }

double Tempo::getTypeTempo() const {
      return typeTempo_;
      }

double Tempo::getQuarterTempo() const {
      double factor = pow(2.0, int(NoteType::Note_Quarter) - int(getLeftNoteType()));
      if (getLeftNoteDot())
            factor *= 3.0/2.0;
      double tempo = getTypeTempo() * factor;

      return tempo;
      }

void Tempo::setLeftText(const QString& str) {
      leftText_ = str;
      }

QString Tempo::getLeftText() const {
      return leftText_;
      }

void Tempo::setRightText(const QString& str) {
      rightText_ = str;
      }

QString Tempo::getRightText() const {
      return rightText_;
      }

void Tempo::setSwingEighth(bool swing) {
      swingEighth_ = swing;
      }

bool Tempo::getSwingEighth() const {
      return swingEighth_;
      }

void Tempo::setRightNoteType(int type) {
      rightNoteType_ = type;
      }

NoteType Tempo::getRightNoteType() const {
      return (NoteType) rightNoteType_;
      }

void Tempo::setLeftNoteDot(bool showDot) {
      leftNoteDot_ = showDot;
      }

bool Tempo::getLeftNoteDot() const {
      return leftNoteDot_;
      }

void Tempo::setRightNoteDot(bool showDot) {
      rightNoteDot_ = showDot;
      }

bool Tempo::getRightNoteDot() const {
      return rightNoteDot_;
      }

void Tempo::setRightSideType(int type) {
      rightSideType_ = type;
      }

int Tempo::getRightSideType() const {
      return rightSideType_;
      }

///////////////////////////////////////////////////////////////////////////////
Text::Text() {
      musicDataType_ = MusicDataType::Text;

      textType_ = Type::Rehearsal;
      horiMargin_ = 8;
      vertMargin_ = 8;
      lineThick_ = 4;
      text_ = QString();
      width_ = 0;
      height_ = 0;
      }

void Text::setTextType(Type type) {
      textType_ = type;
      }

Text::Type Text::getTextType() const {
      return textType_;
      }

void Text::setHorizontalMargin(int margin) {
      horiMargin_ = margin;
      }

int Text::getHorizontalMargin() const {
      return horiMargin_;
      }

void Text::setVerticalMargin(int margin) {
      vertMargin_ = margin;
      }

int Text::getVerticalMargin() const {
      return vertMargin_;
      }

void Text::setLineThick(int thick) {
      lineThick_ = thick;
      }

int Text::getLineThick() const {
      return lineThick_;
      }

void Text::setText(const QString& text) {
      text_ = text;
      }

QString Text::getText() const {
      return text_;
      }

void Text::setWidth(int width) {
      width_ = width;
      }

int Text::getWidth() const {
      return width_;
      }

void Text::setHeight(int height) {
      height_ = height;
      }

int Text::getHeight() const {
      return height_;
      }

///////////////////////////////////////////////////////////////////////////////
TimeSignature::TimeSignature() {
      numerator_ = 4;
      denominator_ = 4;
      isSymbol_ = false;
      beatLength_ = 480;
      barLength_ = 1920;
      barLengthUnits_ = 0x400;
      replaceFont_ = false;
      showBeatGroup_ = false;

      groupNumerator1_ = 0;
      groupNumerator2_ = 0;
      groupNumerator3_ = 0;
      groupDenominator1_ = 4;
      groupDenominator2_ = 4;
      groupDenominator3_ = 4;

      beamGroup1_ = 4;
      beamGroup2_ = 0;
      beamGroup3_ = 0;
      beamGroup4_ = 0;

      beamCount16th_ = 4;
      beamCount32th_ = 1;
      }

void TimeSignature::setNumerator(int numerator) {
      numerator_ = numerator;
      }

int TimeSignature::getNumerator() const {
      return numerator_;
      }

void TimeSignature::setDenominator(int denominator) {
      denominator_ = denominator;
      }

int TimeSignature::getDenominator() const {
      return denominator_;
      }

void TimeSignature::setIsSymbol(bool symbol) {
      isSymbol_ = symbol;
      }

bool TimeSignature::getIsSymbol() const {
      if (numerator_ == 2 && denominator_ == 2) {
            return true;
            }

      return isSymbol_;
      }

void TimeSignature::setBeatLength(int length) {
      beatLength_ = length;
      }

int TimeSignature::getBeatLength() const {
      return beatLength_;
      }

void TimeSignature::setBarLength(int length) {
      barLength_ = length;
      }

int TimeSignature::getBarLength() const {
      return barLength_;
      }

void TimeSignature::addBeat(int startUnit, int lengthUnit, int startTick) {
      BeatNode node;
      node.startUnit_ = startUnit;
      node.lengthUnit_ = lengthUnit;
      node.startTick_ = startTick;
      beats_.push_back(node);
      }

void TimeSignature::endAddBeat() {
      int i;
      barLengthUnits_ = 0;

      for (i = 0; i < beats_.size(); ++i) {
            barLengthUnits_ += beats_[i].lengthUnit_;
            }
      }

int TimeSignature::getUnits() const {
      return barLengthUnits_;
      }

void TimeSignature::setReplaceFont(bool replace) {
      replaceFont_ = replace;
      }

bool TimeSignature::getReplaceFont() const {
      return replaceFont_;
      }

void TimeSignature::setShowBeatGroup(bool show) {
      showBeatGroup_ = show;
      }

bool TimeSignature::getShowBeatGroup() const {
      return showBeatGroup_;
      }

void TimeSignature::setGroupNumerator1(int numerator) {
      groupNumerator1_ = numerator;
      }

void TimeSignature::setGroupNumerator2(int numerator) {
      groupNumerator2_ = numerator;
      }

void TimeSignature::setGroupNumerator3(int numerator) {
      groupNumerator3_ = numerator;
      }

void TimeSignature::setGroupDenominator1(int denominator) {
      groupDenominator1_ = denominator;
      }

void TimeSignature::setGroupDenominator2(int denominator) {
      groupDenominator2_ = denominator;
      }

void TimeSignature::setGroupDenominator3(int denominator) {
      groupDenominator3_ = denominator;
      }

void TimeSignature::setBeamGroup1(int count) {
      beamGroup1_ = count;
      }

void TimeSignature::setBeamGroup2(int count) {
      beamGroup2_ = count;
      }

void TimeSignature::setBeamGroup3(int count) {
      beamGroup3_ = count;
      }

void TimeSignature::setBeamGroup4(int count) {
      beamGroup4_ = count;
      }

void TimeSignature::set16thBeamCount(int count) {
      beamCount16th_ = count;
      }

void TimeSignature::set32thBeamCount(int count) {
      beamCount32th_ = count;
      }

///////////////////////////////////////////////////////////////////////////////
Key::Key() {
      key_ = 0;
      set_ = false;
      previousKey_ = 0;
      symbolCount_ = 0;
      }

void Key::setKey(int key) {
      key_ = key;
      set_ = true;
      }

int Key::getKey() const {
      return key_;
      }

bool Key::getSetKey() const {
      return set_;
      }

void Key::setPreviousKey(int key) {
      previousKey_ = key;
      }

int Key::getPreviousKey() const {
      return previousKey_;
      }

void Key::setSymbolCount(int count) {
      symbolCount_ = count;
      }

int Key::getSymbolCount() const {
      return symbolCount_;
      }

///////////////////////////////////////////////////////////////////////////////
RepeatSymbol::RepeatSymbol() :
      text_("#1"), repeatType_(RepeatType::Segno) {
      musicDataType_ = MusicDataType::Repeat;
      }

void RepeatSymbol::setText(const QString& text) {
      text_ = text;
      }

QString RepeatSymbol::getText() const {
      return text_;
      }

void RepeatSymbol::setRepeatType(int repeatType) {
      repeatType_ = (RepeatType) repeatType;
      }

RepeatType RepeatSymbol::getRepeatType() const {
      return repeatType_;
      }

///////////////////////////////////////////////////////////////////////////////
NumericEnding::NumericEnding() {
      musicDataType_ = MusicDataType::Numeric_Ending;

      height_ = 0;
      text_ = QString();
      numericHandle_ = new OffsetElement();
      }

NumericEnding::~NumericEnding() {
      delete numericHandle_;
      }

OffsetElement* NumericEnding::getNumericHandle() const {
      return numericHandle_;
      }

void NumericEnding::setHeight(int height) {
      height_ = height;
      }

int NumericEnding::getHeight() const {
      return height_;
      }

void NumericEnding::setText(const QString& text) {
      text_ = text;
      }

QString NumericEnding::getText() const {
      return text_;
      }

QList<int> NumericEnding::getNumbers() const {
      int i;
      QStringList strs = text_.split(",", QString::SkipEmptyParts);
      QList<int> endings;

      for (i = 0; i < strs.size(); ++i) {
            bool ok;
            int num = strs[i].toInt(&ok);
            endings.push_back(num);
            }

      return endings;
      }

int NumericEnding::getJumpCount() const {
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

///////////////////////////////////////////////////////////////////////////////
BarNumber::BarNumber() {
      index_ = 0;
      showOnParagraphStart_ = false;
      align_ = 0;
      showFlag_ = 1; // staff
      barRange_ = 1; // can't be 0
      prefix_ = QString();
      }

void BarNumber::setIndex(int index) {
      index_ = index;
      }

int BarNumber::getIndex() const {
      return index_;
      }

void BarNumber::setShowOnParagraphStart(bool show) {
      showOnParagraphStart_ = show;
      }

bool BarNumber::getShowOnParagraphStart() const {
      return showOnParagraphStart_;
      }

void BarNumber::setAlign(int align)// 0:left, 1:center, 2:right
      {
      align_ = align;
      }

int BarNumber::getAlign() const {
      return align_;
      }

void BarNumber::setShowFlag(int flag) {
      showFlag_ = flag;
      }

int BarNumber::getShowFlag() const {
      return showFlag_;
      }

void BarNumber::setShowEveryBarCount(int count) {
      barRange_ = count;
      }

int BarNumber::getShowEveryBarCount() const {
      return barRange_;
      }

void BarNumber::setPrefix(const QString& str) {
      prefix_ = str;
      }

QString BarNumber::getPrefix() const {
      return prefix_;
      }

///////////////////////////////////////////////////////////////////////////////
MidiController::MidiController() {
      midiType_ = MidiType::Controller;
      controller_ = 64; // pedal
      value_ = 0;
      }

void MidiController::setController(int number) {
      controller_ = number;
      }

int MidiController::getController() const {
      return controller_;
      }

void MidiController::setValue(int value) {
      value_ = value;
      }

int MidiController::getValue() const {
      return value_;
      }

///////////////////////////////////////////////////////////////////////////////
MidiProgramChange::MidiProgramChange() {
      midiType_ = MidiType::Program_Change;
      patch_ = 0; // grand piano
      }

void MidiProgramChange::setPatch(int patch) {
      patch_ = patch;
      }

int MidiProgramChange::getPatch() const {
      return patch_;
      }

///////////////////////////////////////////////////////////////////////////////
MidiChannelPressure::MidiChannelPressure() :
      pressure_(0) {
      midiType_ = MidiType::Channel_Pressure;
      }

void MidiChannelPressure::setPressure(int pressure) {
      pressure_ = pressure;
      }

int MidiChannelPressure::getPressure() const {
      return pressure_;
      }

///////////////////////////////////////////////////////////////////////////////
MidiPitchWheel::MidiPitchWheel() {
      midiType_ = MidiType::Pitch_Wheel;
      value_ = 0;
      }

void MidiPitchWheel::setValue(int value) {
      value_ = value;
      }

int MidiPitchWheel::getValue() const {
      return value_;
      }

///////////////////////////////////////////////////////////////////////////////
Measure::Measure(int index) {
      barNumber_ = new BarNumber();
      barNumber_->setIndex(index);
      time_ = new TimeSignature();

      clear();
      }

Measure::~Measure(){
      clear();

      delete barNumber_;
      delete time_;
      }

BarNumber* Measure::getBarNumber() const {
      return barNumber_;
      }

TimeSignature* Measure::getTime() const {
      return time_;
      }

void Measure::setLeftBarline(int barline) {
      leftBarline_ = (BarLineType) barline;
      }

BarLineType Measure::getLeftBarline() const {
      return leftBarline_;
      }

void Measure::setRightBarline(int barline) {
      rightBarline_ = (BarLineType) barline;
      }

BarLineType Measure::getRightBarline() const {
      return rightBarline_;
      }

void Measure::setBackwardRepeatCount(int repeatCount) {
      repeatCount_ = repeatCount;
      }

int Measure::getBackwardRepeatCount() const {
      return repeatCount_;
      }

void Measure::setTypeTempo(double tempo) {
      typeTempo_ = tempo;
      }

double Measure::getTypeTempo() const {
      return typeTempo_;
      }

void Measure::setIsPickup(bool pickup) {
      pickup_ = pickup;
      }

bool Measure::getIsPickup() const {
      return pickup_;
      }

void Measure::setIsMultiMeasureRest(bool rest) {
      multiMeasureRest_ = rest;
      }

bool Measure::getIsMultiMeasureRest() const {
      return multiMeasureRest_;
      }

void Measure::setMultiMeasureRestCount(int count) {
      multiMeasureRestCount_ = count;
      }

int Measure::getMultiMeasureRestCount() const {
      return multiMeasureRestCount_;
      }

void Measure::clear() {
      leftBarline_ = BarLineType::Default;
      rightBarline_ = BarLineType::Default;
      repeatCount_ = 1;
      typeTempo_ = 96.00;
      setLength(0x780); //time = 4/4
      pickup_ = false;
      multiMeasureRest_ = false;
      multiMeasureRestCount_ = 0;
      }

///////////////////////////////////////////////////////////////////////////////
MeasureData::MeasureData() {
      key_ = new Key();
      clef_ = new Clef();
      }

MeasureData::~MeasureData(){
      int i;
      for(i=0; i<musicDatas_.size(); ++i){
            delete musicDatas_[i];
            }
      musicDatas_.clear();

      // noteContainers_ also in musicDatas_, no need to destroy
      noteContainers_.clear();

      // only delete at element start
      for(i=0; i<crossMeasureElements_.size(); ++i){
            if(crossMeasureElements_[i].second){
                  delete crossMeasureElements_[i].first;
                  }
            }
      crossMeasureElements_.clear();

      for(i=0; i<midiDatas_.size(); ++i){
            delete midiDatas_[i];
            }
      midiDatas_.clear();

      delete key_;
      delete clef_;
      }

Key* MeasureData::getKey() const {
      return key_;
      }

Clef* MeasureData::getClef() const {
      return clef_;
      }

void MeasureData::addNoteContainer(NoteContainer* ptr) {
      noteContainers_.push_back(ptr);
      }

QList<NoteContainer*> MeasureData::getNoteContainers() const {
      return noteContainers_;
      }

void MeasureData::addMusicData(MusicData* ptr) {
      musicDatas_.push_back(ptr);
      }

QList<MusicData*> MeasureData::getMusicDatas(MusicDataType type) {
      int i;
      QList<MusicData*> notations;

      for (i = 0; i < musicDatas_.size(); ++i) {
            if (type == MusicDataType::None || musicDatas_[i]->getMusicDataType() == type) {
                  notations.push_back(musicDatas_[i]);
                  }
            }

      return notations;
      }

void MeasureData::addCrossMeasureElement(MusicData* ptr, bool start) {
      crossMeasureElements_.push_back(qMakePair(ptr, start));
      }

QList<MusicData*> MeasureData::getCrossMeasureElements(
            MusicDataType type, PairType pairType)
      {
      int i;
      QList<MusicData*> pairs;

      for (i = 0; i < crossMeasureElements_.size(); ++i) {
            if ((type == MusicDataType::None || crossMeasureElements_[i].first->getMusicDataType() == type)
                && (pairType == PairType::All || ((crossMeasureElements_[i].second && pairType == PairType::Start)
                                                 || (!crossMeasureElements_[i].second && pairType == PairType::Stop)))) {
                  pairs.push_back(crossMeasureElements_[i].first);
                  }
            }

      return pairs;
      }

void MeasureData::addMidiData(MidiData* ptr) {
      midiDatas_.push_back(ptr);
      }

QList<MidiData*> MeasureData::getMidiDatas(MidiType type) {
      int i;
      QList<MidiData*> datas;

      for (i = 0; i < midiDatas_.size(); ++i) {
            if (type == MidiType::None || midiDatas_[i]->getMidiType() == type) {
                  datas.push_back(midiDatas_[i]);
                  }
            }

      return datas;
      }

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StreamHandle::StreamHandle() :
      size_(0), curPos_(0), point_(NULL) {
      }

StreamHandle::StreamHandle(unsigned char* p, int size) :
      size_(size), curPos_(0), point_(p) {
      }

StreamHandle::~StreamHandle() {
      point_ = NULL;
      }

bool StreamHandle::read(char* buff, int size) {
      if (point_ != NULL && curPos_ + size <= size_) {
            memcpy(buff, point_ + curPos_, size);
            curPos_ += size;

            return true;
            }

      return false;
      }

bool StreamHandle::write(char* /*buff*/, int /*size*/) {
      return true;
      }

// Block.cpp
///////////////////////////////////////////////////////////////////////////////////////////////////
Block::Block() {
      doResize(0);
      }

Block::Block(unsigned int count) {
      doResize(count);
      }

void Block::resize(unsigned int count) {
      doResize(count);
      }

void Block::doResize(unsigned int count) {
      data_.clear();
      for(unsigned int i=0; i<count; ++i) {
            data_.push_back('\0');
            }
      //data_.resize(count);
      }

const unsigned char* Block::data() const {
      //return const_cast<unsigned char*>(&data_.front());
      return &data_.front();
      }

unsigned char* Block::data() {
      return &data_.front();
      }

int Block::size() const {
      return data_.size();
      }

bool Block::toBoolean() const {
      if (data() == NULL) {
            return false;
            }

      return size() == 1 && data()[0] == 0x01;
      }

unsigned int Block::toUnsignedInt() const {
      if (data() == NULL) {
            return 0;
            }

      int num = 0;

      for (int i = 0; i < (int)sizeof(int) && i < size(); ++i) {
            num = (num << 8) + *(data() + i);
            }

      return num;
      }

int Block::toInt() const {
      if (data() == NULL) {
            return 0;
            }

      int i;
      int num = 0;

      for (i = 0; i < (int)sizeof(int) && i < size(); ++i) {
            num = (num << 8) + (int) *(data() + i);
            }

      std::size_t minSize = sizeof(int);
      if (size() < (int)minSize) {
            minSize = size();
            }

      if ((*(data()) & 0x80) == 0x80) {
            int maxNum = int(pow(2.0, (int) minSize * 8));
            num -= maxNum;
            //num *= -1;
            }

      return num;
      }

QByteArray Block::toStrByteArray() const {
      if (data() == NULL) {
            return QByteArray();
            }

      QByteArray arr((char*) data(), size());

      return arr;
      }

QByteArray Block::fixedSizeBufferToStrByteArray() const {
      QByteArray str;

      for (int i = 0; i < size(); ++i) {
            if (*(data() + i) == '\0') {
                  break;
                  }

            str += (char) *(data() + i);
            }

      return str;
      }

bool Block::operator ==(const Block& block) const {
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

bool Block::operator !=(const Block& block) const {
      return !(*this == block);
      }

///////////////////////////////////////////////////////////////////////////////////////////////////
FixedBlock::FixedBlock() :
      Block() {
      }

FixedBlock::FixedBlock(unsigned int count) :
      Block(count) {
      }

void FixedBlock::resize(unsigned int /*count*/) {
      // Block::resize(size);
      }

///////////////////////////////////////////////////////////////////////////////////////////////////
SizeBlock::SizeBlock() :
      FixedBlock(4) {
      }

unsigned int SizeBlock::toSize() const {
      unsigned int i;
      unsigned int num(0);
      const unsigned int SIZE = 4;

      for (i = 0; i < SIZE; ++i) {
            num = (num << 8) + *(data() + i);
            }

      return num;
      }

/*void SizeBlock::fromUnsignedInt(unsigned int count)
 {
 unsigned_int_to_char_buffer(count, data());
 }*/

///////////////////////////////////////////////////////////////////////////////////////////////////
NameBlock::NameBlock() :
      FixedBlock(4) {
      }

/*void NameBlock::setValue(const char* const name)
 {
 unsigned int i;

 for( i=0; i<size() && *(name+i)!='\0'; ++i )
 {
 *(data()+i) = *(name+i);
 }
 }*/

bool NameBlock::isEqual(const QString& name) const {
      int nsize = name.size();

      if (nsize != size()) {
            return false;
            }

      for (int i = 0; i < size() && nsize; ++i) {
            if (data()[i] != name[i]) {
                  return false;
                  }
            }

      return true;
      }

///////////////////////////////////////////////////////////////////////////////////////////////////
CountBlock::CountBlock() :
      FixedBlock(2) {
      }

/*void CountBlock::setValue(unsigned short count)
 {
 unsigned int i;
 unsigned int SIZE = sizeof(unsigned short);

 for( i=0; i<SIZE; ++i )
 {
 data()[SIZE-1-i] = count % 256;
 count /= 256;
 }
 }*/

unsigned short CountBlock::toCount() const {
      unsigned short num = 0;

      for (int i = 0; i < size() && i < (int)sizeof(unsigned short); ++i) {
            num = (num << 8) + *(data() + i);
            }

      return num;
      }

// Chunk.cpp
const QString Chunk::TrackName   = "TRAK";
const QString Chunk::PageName    = "PAGE";
const QString Chunk::LineName    = "LINE";
const QString Chunk::StaffName   = "STAF";
const QString Chunk::MeasureName = "MEAS";
const QString Chunk::ConductName = "COND";
const QString Chunk::BdatName    = "BDAT";

Chunk::Chunk() {
      }

NameBlock Chunk::getName() const {
      return nameBlock_;
      }

////////////////////////////////////////////////////////////////////////////////
const unsigned int SizeChunk::version3TrackSize = 0x13a;

SizeChunk::SizeChunk() :
      Chunk() {
      sizeBlock_ = new SizeBlock();
      dataBlock_ = new Block();
      }

SizeChunk::~SizeChunk() {
      delete sizeBlock_;
      delete dataBlock_;
      }

SizeBlock* SizeChunk::getSizeBlock() const {
      return sizeBlock_;
      }

Block* SizeChunk::getDataBlock() const {
      return dataBlock_;
      }

/////////////////////////////////////////////////////////////////////////////////
GroupChunk::GroupChunk() : Chunk() {
      childCount_ = new CountBlock();
      }

GroupChunk::~GroupChunk() {
      delete childCount_;
      }

CountBlock* GroupChunk::getCountBlock() const {
      return childCount_;
      }

// ChunkParse.cpp
unsigned int getHighNibble(unsigned int byte) {
      return byte / 16;
      }

unsigned int getLowNibble(unsigned int byte) {
      return byte % 16;
      }

int oveKeyToKey(int oveKey) {
      int key = 0;

      if( oveKey == 0 ) {
            key = 0;
            }
      else if( oveKey > 7 ) {
            key = oveKey - 7;
            }
      else if( oveKey <= 7 ) {
            key = oveKey * (-1);
            }

      return key;
      }

///////////////////////////////////////////////////////////////////////////////
BasicParse::BasicParse(OveSong* ove) :
      ove_(ove), handle_(NULL), notify_(NULL) {
      }

BasicParse::BasicParse() :
      ove_(NULL), handle_(NULL), notify_(NULL) {
      }

BasicParse::~BasicParse() {
      ove_ = NULL;
      handle_ = NULL;
      notify_ = NULL;
      }

void BasicParse::setNotify(IOveNotify* notify) {
      notify_ = notify;
      }

bool BasicParse::parse() {
      return false;
      }

bool BasicParse::readBuffer(Block& placeHolder, int size) {
      if (handle_ == NULL) {
            return false;
            }
      if (placeHolder.size() != size) {
            placeHolder.resize(size);
            }

      if (size > 0) {
            return handle_->read((char*) placeHolder.data(), placeHolder.size());
            }

      return true;
      }

bool BasicParse::jump(int offset) {
      if (handle_ == NULL || offset < 0) {
            return false;
            }

      if (offset > 0) {
            Block placeHolder(offset);
            return handle_->read((char*) placeHolder.data(), placeHolder.size());
            }

      return true;
      }

void BasicParse::messageOut(const QString& str) {
      if (notify_ != NULL) {
            notify_->loadInfo(str);
            }
      }

///////////////////////////////////////////////////////////////////////////////
OvscParse::OvscParse(OveSong* ove) :
      BasicParse(ove), chunk_(NULL) {
      }

OvscParse::~OvscParse() {
      chunk_ = NULL;
      }

void OvscParse::setOvsc(SizeChunk* chunk) {
      chunk_ = chunk;
      }

bool OvscParse::parse() {
      Block* dataBlock = chunk_->getDataBlock();
      unsigned int blockSize = chunk_->getSizeBlock()->toSize();
      StreamHandle handle(dataBlock->data(), blockSize);
      Block placeHolder;

      handle_ = &handle;

      // version
      if (!readBuffer(placeHolder, 1)) { return false; }
      bool version4 = placeHolder.toUnsignedInt() == 4;
      ove_->setIsVersion4(version4);

      QString str = QString("This file is created by Overture ") + (version4 ? "4" : "3") + "\n";
      messageOut(str);

      if( !jump(6) ) { return false; }

      // show page margin
      if (!readBuffer(placeHolder, 1)) { return false; }
      ove_->setShowPageMargin(placeHolder.toBoolean());

      if( !jump(1) ) { return false; }

      // transpose track
      if (!readBuffer(placeHolder, 1)) { return false; }
      ove_->setShowTransposeTrack(placeHolder.toBoolean());

      // play repeat
      if (!readBuffer(placeHolder, 1)) { return false; }
      ove_->setPlayRepeat(placeHolder.toBoolean());

      // play style
      if (!readBuffer(placeHolder, 1)) { return false; }
      OveSong::PlayStyle style = OveSong::PlayStyle::Record;
      if(placeHolder.toUnsignedInt() == 1){
            style = OveSong::PlayStyle::Swing;
            }
      else if(placeHolder.toUnsignedInt() == 2){
            style = OveSong::PlayStyle::Notation;
            }
      ove_->setPlayStyle(style);

      // show line break
      if (!readBuffer(placeHolder, 1)) { return false; }
      ove_->setShowLineBreak(placeHolder.toBoolean());

      // show ruler
      if (!readBuffer(placeHolder, 1)) { return false; }
      ove_->setShowRuler(placeHolder.toBoolean());

      // show color
      if (!readBuffer(placeHolder, 1)) { return false; }
      ove_->setShowColor(placeHolder.toBoolean());

      return true;
      }

///////////////////////////////////////////////////////////////////////////////
TrackParse::TrackParse(OveSong* ove)
      :BasicParse(ove) {
      }

TrackParse::~TrackParse() {
      }

void TrackParse::setTrack(SizeChunk* chunk) {
      chunk_ = chunk;
      }

bool TrackParse::parse() {
      Block* dataBlock = chunk_->getDataBlock();
      unsigned int blockSize = ove_->getIsVersion4() ? chunk_->getSizeBlock()->toSize() : SizeChunk::version3TrackSize;
      StreamHandle handle(dataBlock->data(), blockSize);
      Block placeHolder;

      handle_ = &handle;

      Track* oveTrack = new Track();
      ove_->addTrack(oveTrack);

      // 2 32bytes long track name buffer
      if( !readBuffer(placeHolder, 32) ) { return false; }
      oveTrack->setName(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if( !readBuffer(placeHolder, 32) ) { return false; }
      oveTrack->setBriefName(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if( !jump(8) ) { return false; } //0x fffa0012 fffa0012
      if( !jump(1) ) { return false; }

      // patch
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int thisByte = placeHolder.toInt();
      oveTrack->setPatch(thisByte&0x7f);

      // show name
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowName(placeHolder.toBoolean());

      // show brief name
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowBriefName(placeHolder.toBoolean());

      if( !jump(1) ) { return false; }

      // show transpose
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowTranspose(placeHolder.toBoolean());

      if( !jump(1) ) { return false; }

      // mute
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setMute(placeHolder.toBoolean());

      // solo
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setSolo(placeHolder.toBoolean());

      if( !jump(1) ) { return false; }

      // show key each line
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowKeyEachLine(placeHolder.toBoolean());

      // voice count
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setVoiceCount(placeHolder.toUnsignedInt());

      if( !jump(3) ) { return false; }

      // transpose value [-127, 127]
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setTranspose(placeHolder.toInt());

      if( !jump(2) ) { return false; }

      // start clef
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setStartClef(placeHolder.toUnsignedInt());

      // transpose celf
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setTransposeClef(placeHolder.toUnsignedInt());

      // start key
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setStartKey(placeHolder.toUnsignedInt());

      // display percent
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setDisplayPercent(placeHolder.toUnsignedInt());

      // show leger line
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowLegerLine(placeHolder.toBoolean());

      // show clef
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowClef(placeHolder.toBoolean());

      // show time signature
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowTimeSignature(placeHolder.toBoolean());

      // show key signature
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowKeySignature(placeHolder.toBoolean());

      // show barline
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowBarline(placeHolder.toBoolean());

      // fill with rest
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setFillWithRest(placeHolder.toBoolean());

      // flat tail
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setFlatTail(placeHolder.toBoolean());

      // show clef each line
      if( !readBuffer(placeHolder, 1) ) { return false; }
      oveTrack->setShowClefEachLine(placeHolder.toBoolean());

      if( !jump(12) ) { return false; }

      // 8 voices
      int i;
      QList<Voice*> voices;
      for( i=0; i<8; ++i ) {
            Voice* voicePtr = new Voice();

            if( !jump(5) ) { return false; }

            // channel
            if( !readBuffer(placeHolder, 1) ) { return false; }
            voicePtr->setChannel(placeHolder.toUnsignedInt());

            // volume
            if( !readBuffer(placeHolder, 1) ) { return false; }
            voicePtr->setVolume(placeHolder.toInt());

            // pitch shift
            if( !readBuffer(placeHolder, 1) ) { return false; }
            voicePtr->setPitchShift(placeHolder.toInt());

            // pan
            if( !readBuffer(placeHolder, 1) ) { return false; }
            voicePtr->setPan(placeHolder.toInt());

            if( !jump(6) ) { return false; }

            // patch
            if( !readBuffer(placeHolder, 1) ) { return false; }
            voicePtr->setPatch(placeHolder.toInt());

            voices.push_back(voicePtr);
            }

      // stem type
      for( i=0; i<8; ++i ) {
            if( !readBuffer(placeHolder, 1) ) { return false; }
            voices[i]->setStemType(placeHolder.toUnsignedInt());

            oveTrack->addVoice(voices[i]);
            }

      // percussion define
      QList<Track::DrumNode> nodes;
      for(i=0; i<16; ++i) {
            nodes.push_back(Track::DrumNode());
            }

      // line
      for( i=0; i<16; ++i ) {
            if( !readBuffer(placeHolder, 1) ) { return false; }
            nodes[i].line_ = placeHolder.toInt();
            }

      // head type
      for( i=0; i<16; ++i ) {
            if( !readBuffer(placeHolder, 1) ) { return false; }
            nodes[i].headType_ = placeHolder.toUnsignedInt();
            }

      // pitch
      for( i=0; i<16; ++i ) {
            if( !readBuffer(placeHolder, 1) ) { return false; }
            nodes[i].pitch_ = placeHolder.toUnsignedInt();
            }

      // voice
      for( i=0; i<16; ++i ) {
            if( !readBuffer(placeHolder, 1) ) { return false; }
            nodes[i].voice_ = placeHolder.toUnsignedInt();
            }

      for( i=0; i<nodes.size(); ++i ) {
            oveTrack->addDrum(nodes[i]);
            }

      /* if( !Jump(17) ) { return false; }

   // voice 0 channel
   if( !ReadBuffer(placeHolder, 1) ) { return false; }
   oveTrack->setChannel(placeHolder.toUnsignedInt());

   // to be continued. if anything important...*/

      return true;
      }

///////////////////////////////////////////////////////////////////////////////
GroupParse::GroupParse(OveSong* ove)
      :BasicParse(ove) {
      }

GroupParse::~GroupParse(){
      sizeChunks_.clear();
      }

void GroupParse::addSizeChunk(SizeChunk* sizeChunk) {
      sizeChunks_.push_back(sizeChunk);
      }

bool GroupParse::parse() {
      return false;
      }

///////////////////////////////////////////////////////////////////////////////
PageGroupParse::PageGroupParse(OveSong* ove)
      :BasicParse(ove) {
      }

PageGroupParse::~PageGroupParse(){
      pageChunks_.clear();
      }

void PageGroupParse::addPage(SizeChunk* chunk) {
      pageChunks_.push_back(chunk);
      }

bool PageGroupParse::parse() {
      if( pageChunks_.isEmpty() ) {
            return false;
            }

      int i;
      for( i=0; i<pageChunks_.size(); ++i ) {
            Page* page = new Page();
            ove_->addPage(page);

            if( !parsePage(pageChunks_[i], page) ) { return false; }
            }

      return true;
      }

bool PageGroupParse::parsePage(SizeChunk* chunk, Page* page) {
      Block placeHolder(2);
      StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

      handle_ = &handle;

      // begin line
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setBeginLine(placeHolder.toUnsignedInt());

      // line count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setLineCount(placeHolder.toUnsignedInt());

      if( !jump(4) ) { return false; }

      // staff interval
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setStaffInterval(placeHolder.toUnsignedInt());

      // line interval
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setLineInterval(placeHolder.toUnsignedInt());

      // staff inline interval
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setStaffInlineInterval(placeHolder.toUnsignedInt());

      // line bar count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setLineBarCount(placeHolder.toUnsignedInt());

      // page line count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      page->setPageLineCount(placeHolder.toUnsignedInt());

      // left margin
      if( !readBuffer(placeHolder, 4) ) { return false; }
      page->setLeftMargin(placeHolder.toUnsignedInt());

      // top margin
      if( !readBuffer(placeHolder, 4) ) { return false; }
      page->setTopMargin(placeHolder.toUnsignedInt());

      // right margin
      if( !readBuffer(placeHolder, 4) ) { return false; }
      page->setRightMargin(placeHolder.toUnsignedInt());

      // bottom margin
      if( !readBuffer(placeHolder, 4) ) { return false; }
      page->setBottomMargin(placeHolder.toUnsignedInt());

      // page width
      if( !readBuffer(placeHolder, 4) ) { return false; }
      page->setPageWidth(placeHolder.toUnsignedInt());

      // page height
      if( !readBuffer(placeHolder, 4) ) { return false; }
      page->setPageHeight(placeHolder.toUnsignedInt());

      handle_ = NULL;

      return true;
      }

///////////////////////////////////////////////////////////////////////////////
StaffCountGetter::StaffCountGetter(OveSong* ove)
      :BasicParse(ove) {
      }

unsigned int StaffCountGetter::getStaffCount(SizeChunk* chunk) {
      StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());
      Block placeHolder;

      handle_ = &handle;

      if( !jump(6) ) { return false; }

      // staff count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      return placeHolder.toUnsignedInt();
      }

///////////////////////////////////////////////////////////////////////////////
LineGroupParse::LineGroupParse(OveSong* ove) :
      BasicParse(ove), chunk_(NULL) {
      }

LineGroupParse::~LineGroupParse(){
      chunk_ = NULL;
      lineChunks_.clear();
      staffChunks_.clear();
      }

void LineGroupParse::setLineGroup(GroupChunk* chunk) {
      chunk_ = chunk;
      }

void LineGroupParse::addLine(SizeChunk* chunk) {
      lineChunks_.push_back(chunk);
      }

void LineGroupParse::addStaff(SizeChunk* chunk) {
      staffChunks_.push_back(chunk);
      }

bool LineGroupParse::parse() {
      if( lineChunks_.isEmpty() || staffChunks_.size() % lineChunks_.size() != 0 ) { return false; }

      int i;
      unsigned int j;
      unsigned int lineStaffCount = staffChunks_.size() / lineChunks_.size();

      for( i=0; i<lineChunks_.size(); ++i ) {
            Line* linePtr = new Line();

            ove_->addLine(linePtr);

            if( !parseLine(lineChunks_[i], linePtr) ) { return false; }

            for( j=lineStaffCount*i; j<lineStaffCount*(i+1); ++j ) {
                  Staff* staffPtr = new Staff();

                  linePtr->addStaff(staffPtr);

                  if( !parseStaff(staffChunks_[j], staffPtr) ) { return false; }
                  }
            }

      return true;
      }

bool LineGroupParse::parseLine(SizeChunk* chunk, Line* line) {
      Block placeHolder;

      StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

      handle_ = &handle;

      if( !jump(2) ) { return false; }

      // begin bar
      if( !readBuffer(placeHolder, 2) ) { return false; }
      line->setBeginBar(placeHolder.toUnsignedInt());

      // bar count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      line->setBarCount(placeHolder.toUnsignedInt());

      if( !jump(6) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      line->setYOffset(placeHolder.toInt());

      // left x offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      line->setLeftXOffset(placeHolder.toInt());

      // right x offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      line->setRightXOffset(placeHolder.toInt());

      if( !jump(4) ) { return false; }

      handle_ = NULL;

      return true;
      }

bool LineGroupParse::parseStaff(SizeChunk* chunk, Staff* staff) {
      Block placeHolder;

      StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

      handle_ = &handle;

      if( !jump(7) ) { return false; }

      // clef
      if( !readBuffer(placeHolder, 1) ) { return false; }
      staff->setClefType(placeHolder.toUnsignedInt());

      // key
      if( !readBuffer(placeHolder, 1) ) { return false; }
      staff->setKeyType(oveKeyToKey(placeHolder.toUnsignedInt()));

      if( !jump(2) ) { return false; }

      // visible
      if( !readBuffer(placeHolder, 1) ) { return false; }
      staff->setVisible(placeHolder.toBoolean());

      if( !jump(12) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      staff->setYOffset(placeHolder.toInt());

      int jumpAmount = ove_->getIsVersion4() ? 26 : 18;
      if( !jump(jumpAmount) ) { return false; }

      // group type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      GroupType groupType = GroupType::None;
      if(placeHolder.toUnsignedInt() == 1) {
            groupType = GroupType::Brace;
            } else if(placeHolder.toUnsignedInt() == 2) {
            groupType = GroupType::Bracket;
            }
      staff->setGroupType(groupType);

      // group staff count
      if( !readBuffer(placeHolder, 1) ) { return false; }
      staff->setGroupStaffCount(placeHolder.toUnsignedInt());

      handle_ = NULL;

      return true;
      }

///////////////////////////////////////////////////////////////////////////////
BarsParse::BarsParse(OveSong* ove) :
      BasicParse(ove) {
      }

BarsParse::~BarsParse(){
      measureChunks_.clear();
      conductChunks_.clear();
      bdatChunks_.clear();
      }

void BarsParse::addMeasure(SizeChunk* chunk) {
      measureChunks_.push_back(chunk);
      }

void BarsParse::addConduct(SizeChunk* chunk) {
      conductChunks_.push_back(chunk);
      }

void BarsParse::addBdat(SizeChunk* chunk) {
      bdatChunks_.push_back(chunk);
      }

bool BarsParse::parse() {
      int i;
      int trackMeasureCount = ove_->getTrackBarCount();
      int trackCount = ove_->getTrackCount();
      int measureDataCount = trackCount * measureChunks_.size();
      QList<Measure*> measures;
      QList<MeasureData*> measureDatas;

      if( measureChunks_.isEmpty() ||
          measureChunks_.size() != conductChunks_.size() ||
          (int)bdatChunks_.size() != measureDataCount ) {
            return false;
            }

      // add to ove
      for ( i=0; i<(int)measureChunks_.size(); ++i ) {
            Measure* measure = new Measure(i);

            measures.push_back(measure);
            ove_->addMeasure(measure);
            }

      for ( i=0; i<measureDataCount; ++i ) {
            MeasureData* oveMeasureData = new MeasureData();

            measureDatas.push_back(oveMeasureData);
            ove_->addMeasureData(oveMeasureData);
            }

      for( i=0; i<(int)measureChunks_.size(); ++i ) {
            Measure* measure = measures[i];

            // MEAS
            if( !parseMeas(measure, measureChunks_[i]) ) {
                  QString ss = QString("failed in parse MEAS %1\n").arg(i);
                  messageOut(ss);

                  return false;
                  }
            }

      for( i=0; i<(int)conductChunks_.size(); ++i ) {
            // COND
            if( !parseCond(measures[i], measureDatas[i], conductChunks_[i]) ) {
                  QString ss = QString("failed in parse COND %1\n").arg(i);
                  messageOut(ss);

                  return false;
                  }
            }

      for( i=0; i<(int)bdatChunks_.size(); ++i ) {
            int measId = i % trackMeasureCount;

            // BDAT
            if( !parseBdat(measures[measId], measureDatas[i], bdatChunks_[i]) ) {
                  QString ss = QString("failed in parse BDAT %1\n").arg(i);
                  messageOut(ss);

                  return false;
                  }

            if( notify_ != NULL ) {
                  int measureID = i % trackMeasureCount;
                  int trackID = i / trackMeasureCount;

                  //msg.msg_ = OVE_IMPORT_POS;
                  //msg.param1_ = (measureID<<16) + trackMeasureCount;
                  //msg.param2_ = (trackID<<16) + trackCount;

                  notify_->loadPosition(measureID, trackMeasureCount, trackID, trackCount);
                  }
            }

      return true;
      }

bool BarsParse::parseMeas(Measure* measure, SizeChunk* chunk) {
      Block placeHolder;

      StreamHandle measureHandle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

      handle_ = &measureHandle;

      if( !jump(2) ) { return false; }

      // multi-measure rest
      if( !readBuffer(placeHolder, 1) ) { return false; }
      measure->setIsMultiMeasureRest(placeHolder.toBoolean());

      // pickup
      if( !readBuffer(placeHolder, 1) ) { return false; }
      measure->setIsPickup(placeHolder.toBoolean());

      if( !jump(4) ) { return false; }

      // left barline
      if( !readBuffer(placeHolder, 1) ) { return false; }
      measure->setLeftBarline(placeHolder.toUnsignedInt());

      // right barline
      if( !readBuffer(placeHolder, 1) ) { return false; }
      measure->setRightBarline(placeHolder.toUnsignedInt());

      // tempo
      if( !readBuffer(placeHolder, 2) ) { return false; }
      double tempo = ((double)placeHolder.toUnsignedInt());
      if( ove_->getIsVersion4() ) {
            tempo /= 100.0;
            }
      measure->setTypeTempo(tempo);

      // bar length(tick)
      if( !readBuffer(placeHolder, 2) ) { return false; }
      measure->setLength(placeHolder.toUnsignedInt());

      if( !jump(6) ) { return false; }

      // bar number offset
      if( !parseOffsetElement(measure->getBarNumber()) ) { return false; }

      if( !jump(2) ) { return false; }

      // multi-measure rest count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      measure->setMultiMeasureRestCount(placeHolder.toUnsignedInt());

      handle_ = NULL;

      return true;
      }

bool BarsParse::parseCond(Measure* measure, MeasureData* measureData, SizeChunk* chunk) {
      Block placeHolder;

      StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

      handle_ = &handle;

      // item count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      unsigned int cnt = placeHolder.toUnsignedInt();

      if( !parseTimeSignature(measure, 36) ) { return false; }

      for( unsigned int i=0; i<cnt; ++i ) {
            if( !readBuffer(placeHolder, 2) ) { return false; }
            unsigned int twoByte = placeHolder.toUnsignedInt();
            unsigned int oldBlockSize = twoByte - 11;
            unsigned int newBlockSize = twoByte - 7;

            // type id
            if( !readBuffer(placeHolder, 1) ) { return false; }
            unsigned int thisByte = placeHolder.toUnsignedInt();
            CondType type;

            if( !getCondElementType(thisByte, type) ) { return false; }

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

      handle_ = NULL;

      return true;
      }

bool BarsParse::parseTimeSignature(Measure* measure, int /*length*/) {
      Block placeHolder;

      TimeSignature* timeSignature = measure->getTime();

      // numerator
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setNumerator(placeHolder.toUnsignedInt());

      // denominator
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setDenominator(placeHolder.toUnsignedInt());

      if( !jump(2) ) { return false; }

      // beat length
      if( !readBuffer(placeHolder, 2) ) { return false; }
      timeSignature->setBeatLength(placeHolder.toUnsignedInt());

      // bar length
      if( !readBuffer(placeHolder, 2) ) { return false; }
      timeSignature->setBarLength(placeHolder.toUnsignedInt());

      if( !jump(4) ) { return false; }

      // is symbol
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setIsSymbol(placeHolder.toBoolean());

      if( !jump(1) ) { return false; }

      // replace font
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setReplaceFont(placeHolder.toBoolean());

      // color
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setColor(placeHolder.toUnsignedInt());

      // show
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setShow(placeHolder.toBoolean());

      // show beat group
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setShowBeatGroup(placeHolder.toBoolean());

      if( !jump(6) ) { return false; }

      // numerator 1, 2, 3
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setGroupNumerator1(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setGroupNumerator2(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setGroupNumerator3(placeHolder.toUnsignedInt());

      // denominator
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setGroupDenominator1(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setGroupDenominator2(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setGroupDenominator3(placeHolder.toUnsignedInt());

      // beam group 1~4
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setBeamGroup1(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setBeamGroup2(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setBeamGroup3(placeHolder.toUnsignedInt());
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->setBeamGroup4(placeHolder.toUnsignedInt());

      // beam 16th
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->set16thBeamCount(placeHolder.toUnsignedInt());

      // beam 32th
      if( !readBuffer(placeHolder, 1) ) { return false; }
      timeSignature->set32thBeamCount(placeHolder.toUnsignedInt());

      return true;
      }

bool BarsParse::parseTimeSignatureParameters(Measure* measure, int length) {
      Block placeHolder;
      TimeSignature* ts = measure->getTime();

      int cursor = ove_->getIsVersion4() ? 10 : 8;
      if( !jump(cursor) ) { return false; }

      // numerator
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int numerator = placeHolder.toUnsignedInt();

      cursor = ove_->getIsVersion4() ? 11 : 9;
      if( ( length - cursor ) % 8 != 0 || (length - cursor) / 8 != (int)numerator ) {
            return false;
            }

      for( unsigned int i =0; i<numerator; ++i ) {
            // beat start unit
            if( !readBuffer(placeHolder, 2) ) { return false; }
            int beatStart = placeHolder.toUnsignedInt();

            // beat length unit
            if( !readBuffer(placeHolder, 2) ) { return false; }
            int beatLength = placeHolder.toUnsignedInt();

            if( !jump(2) ) { return false; }

            // beat start tick
            if( !readBuffer(placeHolder, 2) ) { return false; }
            int beatStartTick = placeHolder.toUnsignedInt();

            ts->addBeat(beatStart, beatLength, beatStartTick);
            }

      ts->endAddBeat();

      return true;
      }

bool BarsParse::parseBarlineParameters(Measure* measure, int /*length*/) {
      Block placeHolder;

      int cursor = ove_->getIsVersion4() ? 12 : 10;
      if( !jump(cursor) ) { return false; }

      // repeat count
      if( !readBuffer(placeHolder, 1) ) { return false; }
      int repeatCount = placeHolder.toUnsignedInt();

      measure->setBackwardRepeatCount(repeatCount);

      if( !jump(6) ) { return false; }

      return true;
      }

bool BarsParse::parseNumericEndings(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      NumericEnding* numeric = new NumericEnding();
      measureData->addCrossMeasureElement(numeric, true);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(numeric) ) { return false; }

      if( !jump(6) ) { return false; }

      // measure count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      //int offsetMeasure = placeHolder.toUnsignedInt() - 1;
      int offsetMeasure = placeHolder.toUnsignedInt();
      numeric->stop()->setMeasure(offsetMeasure);

      if( !jump(2) ) { return false; }

      // left x offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      numeric->getLeftShoulder()->setXOffset(placeHolder.toInt());

      // height
      if( !readBuffer(placeHolder, 2) ) { return false; }
      numeric->setHeight(placeHolder.toUnsignedInt());

      // left x offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      numeric->getRightShoulder()->setXOffset(placeHolder.toInt());

      if( !jump(2) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      numeric->getLeftShoulder()->setYOffset(placeHolder.toInt());
      numeric->getRightShoulder()->setYOffset(placeHolder.toInt());

      // number offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      numeric->getNumericHandle()->setXOffset(placeHolder.toInt());
      if( !readBuffer(placeHolder, 2) ) { return false; }
      numeric->getNumericHandle()->setYOffset(placeHolder.toInt());

      if( !jump(6) ) { return false; }

      // text size
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int size = placeHolder.toUnsignedInt();

      // text : size maybe a huge value
      if( !readBuffer(placeHolder, size) ) { return false; }
      numeric->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      // fix for wedding march.ove
      if( size % 2 == 0 ) {
            if( !jump(1) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseTempo(MeasureData* measureData, int /*length*/) {
      Block placeHolder;
      unsigned int thisByte;

      Tempo* tempo = new Tempo();
      measureData->addMusicData(tempo);
      if( !jump(3) )
            return false;
      // common
      if( !parseCommonBlock(tempo) )
            return false;
      if( !readBuffer(placeHolder, 1) )
            return false;
      thisByte = placeHolder.toUnsignedInt();
      // show tempo
      tempo->setShowMark( (getHighNibble(thisByte) & 0x4) == 0x4 );
      // show before text
      tempo->setShowBeforeText( (getHighNibble(thisByte) & 0x8 ) == 0x8 ) ;
      // show parenthesis
      tempo->setShowParenthesis( (getHighNibble(thisByte) & 0x1 ) == 0x1 );
      // left note type
      tempo->setLeftNoteType( getLowNibble(thisByte) );
      // left note dot
      tempo->setLeftNoteDot((getHighNibble(thisByte) & 0x2 ) == 0x2 );
      if( !jump(1) )  // dimension of the note symbol
            return false;
      if( ove_->getIsVersion4() ) {
            if( !jump(2) )
                  return false;
            // tempo
            if( !readBuffer(placeHolder, 2) )
                  return false;
            tempo->setTypeTempo(((double)placeHolder.toUnsignedInt())/100.0);
            }
      else {
            // tempo
            if( !readBuffer(placeHolder, 2) )
                  return false;
            tempo->setTypeTempo((double)placeHolder.toUnsignedInt());
            if( !jump(2) )
                  return false;
            }
      // offset
      if( !parseOffsetElement(tempo) )
            return false;
      if( !jump(16) )
            return false;
      // 31 bytes left text
      if( !readBuffer(placeHolder, 31) )
            return false;
      tempo->setLeftText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if( !readBuffer(placeHolder, 1) )
            return false;
      thisByte = placeHolder.toUnsignedInt();
      // swing eighth
      tempo->setSwingEighth((getHighNibble(thisByte) & 0x4 ) == 0x4 );
      // right note dot
      tempo->setRightNoteDot((getHighNibble(thisByte) & 0x1 ) == 0x1 );
      // compatibility with v3 files ?
      tempo->setRightSideType((int)(getHighNibble(thisByte) & 0x2));
      // right note type
      tempo->setRightNoteType(getLowNibble(thisByte));
      // right text
      if( ove_->getIsVersion4() ) {
            if( !readBuffer(placeHolder, 31) )
                  return false;
            tempo->setRightText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
            if( !readBuffer(placeHolder, 1) )
                  return false;
            // 00 -> float      03 -> integer(floor)     01 -> notetype    02 -> text
            tempo->setRightSideType(placeHolder.toInt());
            }

      return true;
      }

bool BarsParse::parseBarNumber(Measure* measure, int /*length*/) {
      Block placeHolder;

      BarNumber* barNumber = measure->getBarNumber();

      if( !jump(2) ) { return false; }

      // show on paragraph start
      if( !readBuffer(placeHolder, 1) ) { return false; }
      barNumber->setShowOnParagraphStart(getLowNibble(placeHolder.toUnsignedInt())==8);

      unsigned int blankSize = ove_->getIsVersion4() ? 9 : 7;
      if( !jump(blankSize) ) { return false; }

      // text align
      if( !readBuffer(placeHolder, 1) ) { return false; }
      barNumber->setAlign(placeHolder.toUnsignedInt());

      if( !jump(4) ) { return false; }

      // show flag
      if( !readBuffer(placeHolder, 1) ) { return false; }
      barNumber->setShowFlag(placeHolder.toUnsignedInt());

      if( !jump(10) ) { return false; }

      // bar range
      if( !readBuffer(placeHolder, 1) ) { return false; }
      barNumber->setShowEveryBarCount(placeHolder.toUnsignedInt());

      // prefix
      if( !readBuffer(placeHolder, 2) ) { return false; }
      barNumber->setPrefix(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if( !jump(18) ) { return false; }

      return true;
      }

bool BarsParse::parseText(MeasureData* measureData, int length) {
      Block placeHolder;

      Text* text = new Text();
      measureData->addMusicData(text);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(text) ) { return false; }

      // type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int thisByte = placeHolder.toUnsignedInt();
      bool includeLineBreak = ( (getHighNibble(thisByte)&0x2) != 0x2 );
      unsigned int id = getLowNibble(thisByte);
      Text::Type textType = Text::Type::Rehearsal;

      if (id == 0) {
            textType = Text::Type::MeasureText;
            } else if (id == 1) {
            textType = Text::Type::SystemText;
            } else // id ==2
            {
            textType = Text::Type::Rehearsal;
            }

      text->setTextType(textType);

      if( !jump(1) ) { return false; }

      // x offset
      if( !readBuffer(placeHolder, 4) ) { return false; }
      text->setXOffset(placeHolder.toInt());

      // y offset
      if( !readBuffer(placeHolder, 4) ) { return false; }
      text->setYOffset(placeHolder.toInt());

      // width
      if( !readBuffer(placeHolder, 4) ) { return false; }
      text->setWidth(placeHolder.toUnsignedInt());

      // height
      if( !readBuffer(placeHolder, 4) ) { return false; }
      text->setHeight(placeHolder.toUnsignedInt());

      if( !jump(7) ) { return false; }

      // horizontal margin
      if( !readBuffer(placeHolder, 1) ) { return false; }
      text->setHorizontalMargin(placeHolder.toUnsignedInt());

      if( !jump(1) ) { return false; }

      // vertical margin
      if( !readBuffer(placeHolder, 1) ) { return false; }
      text->setVerticalMargin(placeHolder.toUnsignedInt());

      if( !jump(1) ) { return false; }

      // line thick
      if( !readBuffer(placeHolder, 1) ) { return false; }
      text->setLineThick(placeHolder.toUnsignedInt());

      if( !jump(2) ) { return false; }

      // text size
      if( !readBuffer(placeHolder, 2) ) { return false; }
      unsigned int size = placeHolder.toUnsignedInt();

      // text string, maybe huge
      if( !readBuffer(placeHolder, size) ) { return false; }
      text->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      if( !includeLineBreak ) {
            if( !jump(6) ) { return false; }
            } else {
            unsigned int cursor = ove_->getIsVersion4() ? 43 : 41;
            cursor += size;

            // multi lines of text
            for( unsigned int i=0; i<2; ++i ) {
                  if( (int)cursor < length ) {
                        // line parameters count
                        if( !readBuffer(placeHolder, 2) ) { return false; }
                        unsigned int lineCount = placeHolder.toUnsignedInt();

                        if( i==0 && int(cursor + 2 + 8*lineCount) > length ) {
                              return false;
                              }

                        if( i==1 && int(cursor + 2 + 8*lineCount) != length ) {
                              return false;
                              }

                        if( !jump(8*lineCount) ) { return false; }

                        cursor += 2 + 8*lineCount;
                        }
                  }
            }

      return true;
      }

bool BarsParse::parseRepeatSymbol(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      RepeatSymbol* repeat = new RepeatSymbol();
      measureData->addMusicData(repeat);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(repeat) ) { return false; }

      // RepeatType
      if( !readBuffer(placeHolder, 1) ) { return false; }
      repeat->setRepeatType(placeHolder.toUnsignedInt());

      if( !jump(13) ) { return false; }

      // offset
      if( !parseOffsetElement(repeat) ) { return false; }

      if( !jump(15) ) { return false; }

      // size
      if( !readBuffer(placeHolder, 2) ) { return false; }
      unsigned int size = placeHolder.toUnsignedInt();

      // text, maybe huge
      if( !readBuffer(placeHolder, size) ) { return false; }
      repeat->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

      // last 0
      if( size % 2 == 0 ) {
            if( !jump(1) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseBdat(Measure* /*measure*/, MeasureData* measureData, SizeChunk* chunk) {
      Block placeHolder;
      StreamHandle handle(chunk->getDataBlock()->data(), chunk->getSizeBlock()->toSize());

      handle_ = &handle;

      // parse here
      if( !readBuffer(placeHolder, 2) ) { return false; }
      unsigned int cnt = placeHolder.toUnsignedInt();

      for( unsigned int i=0; i<cnt; ++i ) {
            // 0x0028 or 0x0016 or 0x002C
            if( !readBuffer(placeHolder, 2) ) { return false; }
            unsigned int count = placeHolder.toUnsignedInt() - 7;

            // type id
            if( !readBuffer(placeHolder, 1) ) { return false; }
            unsigned int thisByte = placeHolder.toUnsignedInt();
            BdatType type;

            if( !getBdatElementType(thisByte, type) ) { return false; }

            switch( type ) {
                  case BdatType::Raw_Note :
                  case BdatType::Rest :
                  case BdatType::Note : {
                        if( !parseNoteRest(measureData, count, type) ) { return false; }
                        break;
                        }
                  case BdatType::Beam : {
                        if( !parseBeam(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Harmony : {
                        if( !parseHarmony(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Clef : {
                        if( !parseClef(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Dynamics : {
                        if( !parseDynamics(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Wedge : {
                        if( !parseWedge(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Glissando : {
                        if( !parseGlissando(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Decorator : {
                        if( !parseDecorators(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Key : {
                        if( !parseKey(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Lyric : {
                        if( !parseLyric(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Octave_Shift: {
                        if( !parseOctaveShift(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Slur : {
                        if( !parseSlur(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Text : {
                        if( !parseText(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Tie : {
                        if( !parseTie(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Tuplet : {
                        if( !parseTuplet(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Guitar_Bend :
                  case BdatType::Guitar_Barre : {
                        if( !parseSizeBlock(count) ) { return false; }
                        break;
                        }
                  case BdatType::Pedal: {
                        if( !parsePedal(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::KuoHao: {
                        if( !parseKuohao(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Expressions: {
                        if( !parseExpressions(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Harp_Pedal: {
                        if( !parseHarpPedal(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Multi_Measure_Rest: {
                        if( !parseMultiMeasureRest(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Harmony_GuitarFrame: {
                        if( !parseHarmonyGuitarFrame(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Graphics_40:
                  case BdatType::Graphics_RoundRect:
                  case BdatType::Graphics_Rect:
                  case BdatType::Graphics_Round:
                  case BdatType::Graphics_Line:
                  case BdatType::Graphics_Curve:
                  case BdatType::Graphics_WedgeSymbol: {
                        if( !parseSizeBlock(count) ) { return false; }
                        break;
                        }
                  case BdatType::Midi_Controller : {
                        if( !parseMidiController(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Midi_Program_Change : {
                        if( !parseMidiProgramChange(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Midi_Channel_Pressure : {
                        if( !parseMidiChannelPressure(measureData, count) ) { return false; }
                        break;
                        }
                  case BdatType::Midi_Pitch_Wheel : {
                        if( !parseMidiPitchWheel(measureData, count) ) { return false; }
                        break;
                        }
                  default: {
                        if( !jump(count) ) { return false; }
                        break;
                        }
                  }

            // if i==count-1 then is bar end place holder
            }

      handle_ = NULL;

      return true;
      }

int getInt(int byte, int bits) {
      int num = 0;

      if( bits > 0 ) {
            int factor = int(pow(2.0, bits-1));
            num = (byte % (factor*2));

            if ( (byte & factor) == factor ) {
                  num -= factor*2;
                  }
            }

      return num;
      }

bool BarsParse::parseNoteRest(MeasureData* measureData, int length, BdatType type) {
      NoteContainer* container = new NoteContainer();
      Block placeHolder;
      unsigned int thisByte;

      measureData->addNoteContainer(container);
      measureData->addMusicData(container);

      // note|rest & grace
      container->setIsRest(type==BdatType::Rest);
      container->setIsRaw(type==BdatType::Raw_Note);

      if( !readBuffer(placeHolder, 2) ) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      container->setIsGrace( thisByte == 0x3C00 );
      container->setIsCue( thisByte == 0x4B40 || thisByte == 0x3240 );

      // show / hide
      if( !readBuffer(placeHolder, 1) ) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      container->setShow(getLowNibble(thisByte)!=0x8);

      // voice
      container->setVoice(getLowNibble(thisByte)&0x7);

      // common
      if( !parseCommonBlock(container) ) { return false; }

      // tuplet
      if( !readBuffer(placeHolder, 1) ) { return false; }
      container->setTuplet(placeHolder.toUnsignedInt());

      // space
      if( !readBuffer(placeHolder, 1) ) { return false; }
      container->setSpace(placeHolder.toUnsignedInt());

      // in beam
      if( !readBuffer(placeHolder, 1) ) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      bool inBeam = ( getHighNibble(thisByte) & 0x1 ) == 0x1;
      container->setInBeam(inBeam);

      // grace NoteType
      container->setGraceNoteType((NoteType)getHighNibble(thisByte));

      // dot
      container->setDot(getLowNibble(thisByte)&0x03);

      // NoteType
      if( !readBuffer(placeHolder, 1) ) { return false; }
      thisByte = placeHolder.toUnsignedInt();
      container->setNoteType((NoteType)getLowNibble(thisByte));

      int cursor = 0;

      if( type == BdatType::Rest ) {
            Note* restPtr = new Note();
            container->addNoteRest(restPtr);
            restPtr->setIsRest(true);

            // line
            if( !readBuffer(placeHolder, 1) ) { return false; }
            restPtr->setLine(placeHolder.toInt());

            if( !jump(1) ) { return false; }

            cursor = ove_->getIsVersion4() ? 16 : 14;
            } else // type == Bdat_Note || type == Bdat_Raw_Note
            {
            // stem up 0x80, stem down 0x00
            if( !readBuffer(placeHolder, 1) ) { return false; }
            thisByte = placeHolder.toUnsignedInt();
            container->setStemUp((getHighNibble(thisByte)&0x8)==0x8);

            // stem length
            int stemOffset = thisByte%0x80;
            container->setStemLength(getInt(stemOffset, 7)+7/*3.5 line span*/);

            // show stem 0x00, hide stem 0x40
            if( !readBuffer(placeHolder, 1) ) { return false; }
            bool hideStem = getHighNibble(thisByte)==0x4;
            container->setShowStem(!hideStem);

            if( !jump(1) ) { return false; }

            // note count
            if( !readBuffer(placeHolder, 1) ) { return false; }
            unsigned int noteCount = placeHolder.toUnsignedInt();
            unsigned int i;

            // each note 16 bytes
            for( i=0; i<noteCount; ++i ) {
                  Note* notePtr = new Note();
                  notePtr->setIsRest(false);

                  container->addNoteRest(notePtr);

                  // note show / hide
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  notePtr->setShow((thisByte&0x80) != 0x80);

                  // notehead type
                  notePtr->setHeadType(thisByte&0x7f);

                  // tie pos
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  notePtr->setTiePos(getHighNibble(thisByte));

                  // offset staff, in {-1, 0, 1}
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  thisByte = getLowNibble(placeHolder.toUnsignedInt());
                  int offsetStaff = 0;
                  if( thisByte == 1 ) { offsetStaff = 1; }
                  if( thisByte == 7 ) { offsetStaff = -1; }
                  notePtr->setOffsetStaff(offsetStaff);

                  // accidental
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  notePtr->setAccidental(getLowNibble(thisByte));
                  // accidental 0: influenced by key, 4: influenced by previous accidental in measure
                  //bool notShow = ( getHighNibble(thisByte) == 0 ) || ( getHighNibble(thisByte) == 4 );
                  bool notShow = !(getHighNibble(thisByte)&0x1);
                  notePtr->setShowAccidental(!notShow);

                  if( !jump(1) ) { return false; }

                  // line
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  notePtr->setLine(placeHolder.toInt());

                  if( !jump(1) ) { return false; }

                  // note
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  unsigned int note = placeHolder.toUnsignedInt();
                  notePtr->setNote(note);

                  // note on velocity
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  unsigned int onVelocity = placeHolder.toUnsignedInt();
                  notePtr->setOnVelocity(onVelocity);

                  // note off velocity
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  unsigned int offVelocity = placeHolder.toUnsignedInt();
                  notePtr->setOffVelocity(offVelocity);

                  if( !jump(2) ) { return false; }

                  // length (tick)
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  container->setLength(placeHolder.toUnsignedInt());

                  // offset tick
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  notePtr->setOffsetTick(placeHolder.toInt());
                  }

            cursor = ove_->getIsVersion4() ? 18 : 16;
            cursor += noteCount * 16/*note size*/;
            }

      // articulation
      while ( cursor < length + 1/* 0x70 || 0x80 || 0x90 */ ) {
            Articulation* art = new Articulation();
            container->addArticulation(art);

            // block size
            if( !readBuffer(placeHolder, 2) ) { return false; }
            int blockSize = placeHolder.toUnsignedInt();

            // articulation type
            if( !readBuffer(placeHolder, 1) ) { return false; }
            art->setArtType(placeHolder.toUnsignedInt());

            // placement
            if( !readBuffer(placeHolder, 1) ) { return false; }
            art->setPlacementAbove(placeHolder.toUnsignedInt()!=0x00); //0x00:below, 0x30:above

            // offset
            if( !parseOffsetElement(art) ) { return false; }

            if( !ove_->getIsVersion4() ) {
                  if( blockSize - 8 > 0 ) {
                        if( !jump(blockSize-8) ) { return false; }
                        }
                  } else {
                  // setting
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  const bool changeSoundEffect = ( ( thisByte & 0x1 ) == 0x1 );
                  const bool changeLength = ( ( thisByte & 0x2 ) == 0x2 );
                  const bool changeVelocity = ( ( thisByte & 0x4 ) == 0x4 );
                  //const bool changeExtraLength = ( ( thisByte & 0x20 ) == 0x20 );

                  if( !jump(8) ) { return false; }

                  // velocity type
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  thisByte = placeHolder.toUnsignedInt();
                  if( changeVelocity ) {
                        art->setVelocityType((Articulation::VelocityType)thisByte);
                        }

                  if( !jump(14) ) { return false; }

                  // sound effect
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  int from = placeHolder.toInt();
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  int to = placeHolder.toInt();
                  if( changeSoundEffect ) {
                        art->setSoundEffect(from, to);
                        }

                  if( !jump(1) ) { return false; }

                  // length percentage
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  if( changeLength ) {
                        art->setLengthPercentage(placeHolder.toUnsignedInt());
                        }

                  // velocity
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  if( changeVelocity ) {
                        art->setVelocityValue(placeHolder.toInt());
                        }

                  if( Articulation::isTrill(art->getArtType()) ) {
                        if( !jump(8) ) { return false; }

                        // trill note length
                        if( !readBuffer(placeHolder, 1) ) { return false; }
                        art->setTrillNoteLength(placeHolder.toUnsignedInt());

                        // trill rate
                        if( !readBuffer(placeHolder, 1) ) { return false; }
                        thisByte = placeHolder.toUnsignedInt();
                        NoteType trillNoteType = NoteType::Note_Sixteen;
                        switch ( getHighNibble(thisByte) ) {
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
                        art->setAccelerateType(thisByte&0xf);

                        if( !jump(1) ) { return false; }

                        // auxiliary first
                        if( !readBuffer(placeHolder, 1) ) { return false; }
                        art->setAuxiliaryFirst(placeHolder.toBoolean());

                        if( !jump(1) ) { return false; }

                        // trill interval
                        if( !readBuffer(placeHolder, 1) ) { return false; }
                        art->setTrillInterval(placeHolder.toUnsignedInt());
                        } else {
                        if( blockSize > 40 ) {
                              if( !jump( blockSize - 40 ) ) { return false; }
                              }
                        }
                  }

            cursor += blockSize;
            }

      return true;
      }

int tupletToSpace(int tuplet) {
      int a(1);

      while( a*2 < tuplet ) {
            a *= 2;
            }

      return a;
      }

bool BarsParse::parseBeam(MeasureData* measureData, int length) {
      int i;
      Block placeHolder;

      Beam* beam = new Beam();
      measureData->addCrossMeasureElement(beam, true);

      // maybe create tuplet, for < quarter & tool 3(
      bool createTuplet = false;
      int maxEndUnit = 0;
      Tuplet* tuplet = new Tuplet();

      // is grace
      if( !readBuffer(placeHolder, 1) ) { return false; }
      beam->setIsGrace(placeHolder.toBoolean());

      if( !jump(1) ) { return false; }

      // voice
      if( !readBuffer(placeHolder, 1) ) { return false; }
      beam->setVoice(getLowNibble(placeHolder.toUnsignedInt())&0x7);

      // common
      if( !parseCommonBlock(beam) ) { return false; }

      if( !jump(2) ) { return false; }

      // beam count
      if( !readBuffer(placeHolder, 1) ) { return false; }
      int beamCount = placeHolder.toUnsignedInt();

      if( !jump(1) ) { return false; }

      // left line
      if( !readBuffer(placeHolder, 1) ) { return false; }
      beam->getLeftLine()->setLine(placeHolder.toInt());

      // right line
      if( !readBuffer(placeHolder, 1) ) { return false; }
      beam->getRightLine()->setLine(placeHolder.toInt());

      if( ove_->getIsVersion4() ) {
            if( !jump(8) ) { return false; }
            }

      int currentCursor = ove_->getIsVersion4() ? 23 : 13;
      int count = (length - currentCursor)/16;

      if( count != beamCount ) { return false; }

      for( i=0; i<count; ++i ) {
            if( !jump(1) ) { return false; }

            // tuplet
            if( !readBuffer(placeHolder, 1) ) { return false; }
            int tupletCount = placeHolder.toUnsignedInt();
            if( tupletCount > 0 ) {
                  createTuplet = true;
                  tuplet->setTuplet(tupletCount);
                  tuplet->setSpace(tupletToSpace(tupletCount));
                  }

            // start / stop measure
            // line i start end position
            MeasurePos startMp;
            MeasurePos stopMp;

            if( !readBuffer(placeHolder, 1) ) { return false; }
            startMp.setMeasure(placeHolder.toUnsignedInt());
            if( !readBuffer(placeHolder, 1) ) { return false; }
            stopMp.setMeasure(placeHolder.toUnsignedInt());

            if( !readBuffer(placeHolder, 2) ) { return false; }
            startMp.setOffset(placeHolder.toInt());
            if( !readBuffer(placeHolder, 2) ) { return false; }
            stopMp.setOffset(placeHolder.toInt());

            beam->addLine(startMp, stopMp);

            if( stopMp.getOffset() > maxEndUnit ) {
                  maxEndUnit = stopMp.getOffset();
                  }

            if( i == 0 ) {
                  if( !jump(4) ) { return false; }

                  // left offset up+4, down-4
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  beam->getLeftShoulder()->setYOffset(placeHolder.toInt());

                  // right offset up+4, down-4
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  beam->getRightShoulder()->setYOffset(placeHolder.toInt());
                  } else {
                  if( !jump(8) ) { return false; }
                  }
            }

      const QList<QPair<MeasurePos, MeasurePos> > lines = beam->getLines();
      MeasurePos offsetMp;

      for( i=0; i<lines.size(); ++i ) {
            if( lines[i].second > offsetMp ) {
                  offsetMp = lines[i].second;
                  }
            }

      beam->stop()->setMeasure(offsetMp.getMeasure());
      beam->stop()->setOffset(offsetMp.getOffset());

      // a case that Tuplet block don't exist, and hide inside beam
      if( createTuplet ) {
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

bool BarsParse::parseTie(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Tie* tie = new Tie();
      measureData->addCrossMeasureElement(tie, true);

      if( !jump(3) ) { return false; }

      // start common
      if( !parseCommonBlock(tie) ) { return false; }

      if( !jump(1) ) { return false; }

      // note
      if( !readBuffer(placeHolder, 1) ) { return false; }
      tie->setNote(placeHolder.toUnsignedInt());

      // pair lines
      if( !parsePairLinesBlock(tie) ) { return false; }

      // offset common
      if( !parseOffsetCommonBlock(tie) ) { return false; }

      // left shoulder offset
      if( !parseOffsetElement(tie->getLeftShoulder()) ) { return false; }

      // right shoulder offset
      if( !parseOffsetElement(tie->getRightShoulder()) ) { return false; }

      // height
      if( !readBuffer(placeHolder, 2) ) { return false; }
      tie->setHeight(placeHolder.toUnsignedInt());

      return true;
      }

bool BarsParse::parseTuplet(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Tuplet* tuplet = new Tuplet();
      measureData->addCrossMeasureElement(tuplet, true);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(tuplet) ) { return false; }

      if( !jump(2) ) { return false; }

      // pair lines
      if( !parsePairLinesBlock(tuplet) ) { return false; }

      // offset common
      if( !parseOffsetCommonBlock(tuplet) ) { return false; }

      // left shoulder offset
      if( !parseOffsetElement(tuplet->getLeftShoulder()) ) { return false; }

      // right shoulder offset
      if( !parseOffsetElement(tuplet->getRightShoulder()) ) { return false; }

      if( !jump(2) ) { return false; }

      // height
      if( !readBuffer(placeHolder, 2) ) { return false; }
      tuplet->setHeight(placeHolder.toUnsignedInt());

      // tuplet
      if( !readBuffer(placeHolder, 1) ) { return false; }
      tuplet->setTuplet(placeHolder.toUnsignedInt());

      // space
      if( !readBuffer(placeHolder, 1) ) { return false; }
      tuplet->setSpace(placeHolder.toUnsignedInt());

      // mark offset
      if( !parseOffsetElement(tuplet->getMarkHandle()) ) { return false; }

      return true;
      }

QString binaryToHarmonyType(int bin) {
      QString type = "";
      switch (bin) {
            case 0x0005: { type = "add9(no3)";        break; }
            case 0x0009: { type = "min(no5)";         break; }
            case 0x0011: { type = "(no5)";            break; }
            case 0x0021: { type = "sus(no5)";         break; }
            case 0x0025: { type = "24";               break; }
            case 0x0029: { type = "min4(no5)";        break; }
            case 0x0049: { type = "dim";              break; }
            case 0x0051: { type = "(b5)";             break; }
            case 0x0055: { type = "2#4(no5)";         break; }
            case 0x0081: { type = "(no3)";            break; }
            case 0x0085: { type = "2";                break; }
            case 0x0089: { type = "min";              break; }
            case 0x008D: { type = "min(add9)";        break; }
            case 0x0091: { type = "";                 break; }
            case 0x0093: { type = "addb9";            break; }
            case 0x0095: { type = "add9";             break; }
            case 0x00A1: { type = "sus4";             break; }
            case 0x00A5: { type = "sus(add9)";        break; }
            case 0x00A9: { type = "min4";             break; }
            case 0x00D5: { type = "2#4";              break; }
            case 0x0111: { type = "aug";              break; }
            case 0x0115: { type = "aug(add9)";        break; }
            case 0x0151: { type = "(b5b6)";           break; }
            case 0x0155: { type = "+add9#11";         break; }
            case 0x0189: { type = "minb6";            break; }
            case 0x018D: { type = "min2b6";           break; }
            case 0x0191: { type = "(b6)";             break; }
            case 0x0199: { type = "(add9)b6";         break; }
            case 0x0205: { type = "26";               break; }
            case 0x020D: { type = "min69";            break; }
            case 0x0211: { type = "6";                break; }
            case 0x0215: { type = "69";               break; }
            case 0x022D: { type = "min69 11";         break; }
            case 0x0249: { type = "dim7";             break; }
            case 0x0251: { type = "6#11";             break; }
            case 0x0255: { type = "13#11";            break; }
            case 0x0281: { type = "6(no3)";           break; }
            case 0x0285: { type = "69(no3)";          break; }
            case 0x0289: { type = "min6";             break; }
            case 0x028D: { type = "min69";            break; }
            case 0x0291: { type = "6";                break; }
            case 0x0293: { type = "6b9";              break; }
            case 0x0295: { type = "69";               break; }
            case 0x02AD: { type = "min69 11";         break; }
            case 0x02C5: { type = "69#11(no3)";       break; }
            case 0x02D5: { type = "69#11";            break; }
            case 0x040D: { type = "min9(no5)";        break; }
            case 0x0411: { type = "7(no5)";           break; }
            case 0x0413: { type = "7b9";              break; }
            case 0x0415: { type = "9";                break; }
            case 0x0419: { type = "7#9";              break; }
            case 0x041B: { type = "7b9#9";            break; }
            case 0x0421: { type = "sus7";             break; }
            case 0x0429: { type = "min11";            break; }
            case 0x042D: { type = "min11";            break; }
            case 0x0445: { type = "9b5(no3)";         break; }
            case 0x0449: { type = "min7b5";           break; }
            case 0x044D: { type = "min9b5";           break; }
            case 0x0451: { type = "7b5";              break; }
            case 0x0453: { type = "7b9b5";            break; }
            case 0x0455: { type = "9b5";              break; }
            case 0x045B: { type = "7b5b9#9";          break; }
            case 0x0461: { type = "sus7b5";           break; }
            case 0x0465: { type = "sus9b5";           break; }
            case 0x0469: { type = "min11b5";          break; }
            case 0x046D: { type = "min11b5";          break; }
            case 0x0481: { type = "7(no3)";           break; }
            case 0x0489: { type = "min7";             break; }
            case 0x048D: { type = "min9";             break; }
            case 0x0491: { type = "7";                break; }
            case 0x0493: { type = "7b9";              break; }
            case 0x0495: { type = "9";                break; }
            case 0x0499: { type = "7#9";              break; }
            case 0x049B: { type = "7b9#9";            break; }
            case 0x04A1: { type = "sus7";             break; }
            case 0x04A5: { type = "sus9";             break; }
            case 0x04A9: { type = "min11";            break; }
            case 0x04AD: { type = "min11";            break; }
            case 0x04B5: { type = "11";               break; }
            case 0x04D5: { type = "9#11";             break; }
            case 0x0509: { type = "min7#5";           break; }
            case 0x0511: { type = "aug7";             break; }
            case 0x0513: { type = "7#5b9";            break; }
            case 0x0515: { type = "aug9";             break; }
            case 0x0519: { type = "7#5#9";            break; }
            case 0x0529: { type = "min11b13";         break; }
            case 0x0533: { type = "11b9#5";           break; }
            case 0x0551: { type = "aug7#11";          break; }
            case 0x0553: { type = "7b5b9b13";         break; }
            case 0x0555: { type = "aug9#11";          break; }
            case 0x0559: { type = "aug7#9#11";        break; }
            case 0x0609: { type = "min13";            break; }
            case 0x0611: { type = "13";               break; }
            case 0x0613: { type = "13b9";             break; }
            case 0x0615: { type = "13";               break; }
            case 0x0619: { type = "13#9";             break; }
            case 0x061B: { type = "13b9#9";           break; }
            case 0x0621: { type = "sus13";            break; }
            case 0x062D: { type = "min13(11)";        break; }
            case 0x0633: { type = "13b9add4";         break; }
            case 0x0635: { type = "13";               break; }
            case 0x0645: { type = "13#11(no3)";       break; }
            case 0x0651: { type = "13b5";             break; }
            case 0x0653: { type = "13b9#11";          break; }
            case 0x0655: { type = "13#11";            break; }
            case 0x0659: { type = "13#9b5";           break; }
            case 0x065B: { type = "13b5b9#9";         break; }
            case 0x0685: { type = "13(no3)";          break; }
            case 0x068D: { type = "min13";            break; }
            case 0x0691: { type = "13";               break; }
            case 0x0693: { type = "13b9";             break; }
            case 0x0695: { type = "13";               break; }
            case 0x0699: { type = "13#9";             break; }
            case 0x06A5: { type = "sus13";            break; }
            case 0x06AD: { type = "min13(11)";        break; }
            case 0x06B5: { type = "13";               break; }
            case 0x06D5: { type = "13#11";            break; }
            case 0x0813: { type = "maj7b9";           break; }
            case 0x0851: { type = "maj7#11";          break; }
            case 0x0855: { type = "maj9#11";          break; }
            case 0x0881: { type = "maj7(no3)";        break; }
            case 0x0889: { type = "min(\u266e7)";     break; }   // "min(<sym>accidentalNatural</sym>7)"
            case 0x088D: { type = "min9(\u266e7)";    break; }   // "min9(<sym>accidentalNatural</sym>7)"
            case 0x0891: { type = "maj7";             break; }
            case 0x0895: { type = "maj9";             break; }
            case 0x08C9: { type = "dim7(add maj 7)";  break; }
            case 0x08D1: { type = "maj7#11";          break; }
            case 0x08D5: { type = "maj9#11";          break; }
            case 0x0911: { type = "maj7#5";           break; }
            case 0x0991: { type = "maj7#5";           break; }
            case 0x0995: { type = "maj9#5";            break; }
            case 0x0A0D: { type = "min69(\u266e7)";   break; }   // "min69(<sym>accidentalNatural</sym>7)"
            case 0x0A11: { type = "maj13";            break; }
            case 0x0A15: { type = "maj13";            break; }
            case 0x0A51: { type = "maj13#11";         break; }
            case 0x0A55: { type = "maj13#11";         break; }
            case 0x0A85: { type = "maj13(no3)";       break; }
            case 0x0A89: { type = "min13(\u266e7)";   break; }   // "min13(<sym>accidentalNatural</sym>7)"
            case 0x0A8D: { type = "min69(\u266e7)";   break; }   // "min69(<sym>accidentalNatural</sym>7)"
            case 0x0A91: { type = "maj13";            break; }
            case 0x0A95: { type = "maj13";            break; }
            case 0x0AAD: { type = "min13(\u266e7)";   break; }   // "min13(<sym>accidentalNatural</sym>7)"
            case 0x0AD5: { type = "maj13#11";         break; }
            case 0x0B45: { type = "maj13#5#11(no4)";  break; }
            default: {
                  qDebug("Unrecognized harmony type: %04X",bin);
                  type = "";
                  break;
                  }
            }
      return type;
      }

bool BarsParse::parseHarmony(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Harmony* harmony = new Harmony();
      measureData->addMusicData(harmony);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(harmony) ) { return false; }

      // bass on bottom
      if( !readBuffer(placeHolder, 1) ) { return false; }
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

      if( !jump(1) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      harmony->setYOffset(placeHolder.toInt());

      // harmony type
      if( !readBuffer(placeHolder, 2) ) { return false; }
      harmony->setHarmonyType(binaryToHarmonyType(placeHolder.toUnsignedInt()));

      // root
      if( !readBuffer(placeHolder, 1) ) { return false; }
      harmony->setRoot(placeHolder.toInt());

      // bass
      if( !readBuffer(placeHolder, 1) ) { return false; }
      if (useBass)
            harmony->setBass(placeHolder.toInt());

      // angle
      if( !readBuffer(placeHolder, 2) ) { return false; }
      harmony->setAngle(placeHolder.toInt());

      if( ove_->getIsVersion4() ) {
            // length (tick)
            if( !readBuffer(placeHolder, 2) ) { return false; }
            harmony->setLength(placeHolder.toUnsignedInt());

            if( !jump(4) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseClef(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Clef* clef = new Clef();
      measureData->addMusicData(clef);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(clef) ) { return false; }

      // clef type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      clef->setClefType(placeHolder.toUnsignedInt());

      // line
      if( !readBuffer(placeHolder, 1) ) { return false; }
      clef->setLine(placeHolder.toInt());

      if( !jump(2) ) { return false; }

      return true;
      }

bool BarsParse::parseLyric(MeasureData* measureData, int length) {
      Block placeHolder;

      Lyric* lyric = new Lyric();
      measureData->addMusicData(lyric);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(lyric) ) { return false; }

      if( !jump(2) ) { return false; }

      // offset
      if( !parseOffsetElement(lyric) ) { return false; }

      if( !jump(7) ) { return false; }

      // verse
      if( !readBuffer(placeHolder, 1) ) { return false; }
      lyric->setVerse(placeHolder.toUnsignedInt());

      if( ove_->getIsVersion4() ) {
            if( !jump(6) ) { return false; }

            // lyric
            if( length > 29 ) {
                  if( !readBuffer(placeHolder, length-29) ) { return false; }
                  lyric->setLyric(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
                  }
            }

      return true;
      }

bool BarsParse::parseSlur(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Slur* slur = new Slur();
      measureData->addCrossMeasureElement(slur, true);

      if( !jump(2) ) { return false; }

      // voice
      if( !readBuffer(placeHolder, 1) ) { return false; }
      slur->setVoice(getLowNibble(placeHolder.toUnsignedInt())&0x7);

      // common
      if( !parseCommonBlock(slur) ) { return false; }

      // show on top
      if( !readBuffer(placeHolder, 1) ) { return false; }
      slur->setShowOnTop(getHighNibble(placeHolder.toUnsignedInt())==0x8);

      if( !jump(1) ) { return false; }

      // pair lines
      if( !parsePairLinesBlock(slur) ) { return false; }

      // offset common
      if( !parseOffsetCommonBlock(slur) ) { return false; }

      // handle 1
      if( !parseOffsetElement(slur->getLeftShoulder()) ) { return false; }

      // handle 4
      if( !parseOffsetElement(slur->getRightShoulder()) ) { return false; }

      // handle 2
      if( !parseOffsetElement(slur->getHandle2()) ) { return false; }

      // handle 3
      if( !parseOffsetElement(slur->getHandle3()) ) { return false; }

      if( ove_->getIsVersion4() ) {
            if( !jump(3) ) { return false; }

            // note time percent
            if( !readBuffer(placeHolder, 1) ) { return false; }
            slur->setNoteTimePercent(placeHolder.toUnsignedInt());

            if( !jump(36) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseGlissando(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Glissando* glissando = new Glissando();
      measureData->addCrossMeasureElement(glissando, true);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(glissando) ) { return false; }

      // straight or wavy
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int thisByte = placeHolder.toUnsignedInt();
      glissando->setStraightWavy(getHighNibble(thisByte)==4);

      if( !jump(1) ) { return false; }

      // pair lines
      if( !parsePairLinesBlock(glissando) ) { return false; }

      // offset common
      if( !parseOffsetCommonBlock(glissando) ) { return false; }

      // left shoulder
      if( !parseOffsetElement(glissando->getLeftShoulder()) ) { return false; }

      // right shoulder
      if( !parseOffsetElement(glissando->getRightShoulder()) ) { return false; }

      if( ove_->getIsVersion4() ) {
            if( !jump(1) ) { return false; }

            // line thick
            if( !readBuffer(placeHolder, 1) ) { return false; }
            glissando->setLineThick(placeHolder.toUnsignedInt());

            if( !jump(12) ) { return false; }

            // text 32 bytes
            if( !readBuffer(placeHolder, 32) ) { return false; }
            glissando->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));

            if( !jump(6) ) { return false; }
            }

      return true;
      }

bool getDecoratorType(
            unsigned int thisByte,
            bool& measureRepeat,
            Decorator::Type& decoratorType,
            bool& singleRepeat,
            ArticulationType& artType) {
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

bool BarsParse::parseDecorators(MeasureData* measureData, int length) {
      Block placeHolder;
      MusicData* musicData = new MusicData();

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(musicData) ) { return false; }

      if( !jump(2) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      musicData->setYOffset(placeHolder.toInt());

      if( !jump(2) ) { return false; }

      // measure repeat | piano pedal | dotted barline | articulation
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int thisByte = placeHolder.toUnsignedInt();

      Decorator::Type decoratorType;
      bool isMeasureRepeat;
      bool isSingleRepeat = true;
      ArticulationType artType = ArticulationType::None;

      getDecoratorType(thisByte, isMeasureRepeat, decoratorType, isSingleRepeat, artType);

      if( isMeasureRepeat ) {
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

      int cursor = ove_->getIsVersion4() ? 16 : 14;
      if( !jump(length-cursor) ) { return false; }

      return true;
      }

bool BarsParse::parseWedge(MeasureData* measureData, int length) {
      Block placeHolder;
      Wedge* wedge = new Wedge();

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(wedge) ) { return false; }

      // wedge type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      WedgeType wedgeType = WedgeType::Cres_Line;
      bool wedgeOrExpression = true;
      unsigned int highHalfByte = getHighNibble(placeHolder.toUnsignedInt());
      unsigned int lowHalfByte = getLowNibble(placeHolder.toUnsignedInt());

      switch (highHalfByte) {
            case 0x0: {
                  wedgeType = WedgeType::Cres_Line;
                  wedgeOrExpression = true;
                  break;
                  }
            case 0x4: {
                  wedgeType = WedgeType::Decresc_Line;
                  wedgeOrExpression = true;
                  break;
                  }
            case 0x6: {
                  wedgeType = WedgeType::Decresc;
                  wedgeOrExpression = false;
                  break;
                  }
            case 0x2: {
                  wedgeType = WedgeType::Cres;
                  wedgeOrExpression = false;
                  break;
                  }
            default:
                  break;
            }

      // 0xb | 0x8(ove3) , else 3, 0(ove3)
      if( (lowHalfByte & 0x8) == 0x8 ) {
            wedgeType = WedgeType::Double_Line;
            wedgeOrExpression = true;
            }

      if( !jump(1) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      wedge->setYOffset(placeHolder.toInt());

      // wedge
      if( wedgeOrExpression ) {
            measureData->addCrossMeasureElement(wedge, true);
            wedge->setWedgeType(wedgeType);

            if( !jump(2) ) { return false; }

            // height
            if( !readBuffer(placeHolder, 2) ) { return false; }
            wedge->setHeight(placeHolder.toUnsignedInt());

            // offset common
            if( !parseOffsetCommonBlock(wedge) ) { return false; }

            int cursor = ove_->getIsVersion4() ? 21 : 19;
            if( !jump(length-cursor) ) { return false; }
            }
      // expression : cresc, decresc
      else {
            Expressions* express = new Expressions();
            measureData->addMusicData(express);

            express->copyCommonBlock(*wedge);
            express->setYOffset(wedge->getYOffset());

            if( !jump(4) ) { return false; }

            // offset common
            if( !parseOffsetCommonBlock(express) ) { return false; }

            if( ove_->getIsVersion4() ) {
                  if( !jump(18) ) { return false; }

                  // words
                  if( length > 39 ) {
                        if( !readBuffer(placeHolder, length-39) ) { return false; }
                        express->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
                        }
                  } else {
                  QString str = wedgeType==WedgeType::Cres ? "cresc" : "decresc";
                  express->setText(str);

                  if( !jump(8) ) { return false; }
                  }
            }

      return true;
      }

bool BarsParse::parseDynamics(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      Dynamics* dynamics = new Dynamics();
      measureData->addMusicData(dynamics);

      if( !jump(1) ) { return false; }

      // is playback
      if( !readBuffer(placeHolder, 1) ) { return false; }
      dynamics->setIsPlayback(getHighNibble(placeHolder.toUnsignedInt())!=0x4);

      if( !jump(1) ) { return false; }

      // common
      if( !parseCommonBlock(dynamics) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      dynamics->setYOffset(placeHolder.toInt());

      // dynamics type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      dynamics->setDynamicsType(getLowNibble(placeHolder.toUnsignedInt()));

      // velocity
      if( !readBuffer(placeHolder, 1) ) { return false; }
      dynamics->setVelocity(placeHolder.toUnsignedInt());

      int cursor = ove_->getIsVersion4() ? 4 : 2;

      if( !jump(cursor) ) { return false; }

      return true;
      }

bool BarsParse::parseKey(MeasureData* measureData, int /*length*/) {
      Block placeHolder;
      Key* key = measureData->getKey();
      int cursor = ove_->getIsVersion4() ? 9 : 7;

      if( !jump(cursor) ) { return false; }

      // key
      if( !readBuffer(placeHolder, 1) ) { return false; }
      key->setKey(oveKeyToKey(placeHolder.toUnsignedInt()));

      // previous key
      if( !readBuffer(placeHolder, 1) ) { return false; }
      key->setPreviousKey(oveKeyToKey(placeHolder.toUnsignedInt()));

      if( !jump(3) ) { return false; }

      // symbol count
      if( !readBuffer(placeHolder, 1) ) { return false; }
      key->setSymbolCount(placeHolder.toUnsignedInt());

      if( !jump(4) ) { return false; }

      return true;
      }

bool BarsParse::parsePedal(MeasureData* measureData, int length) {
      Block placeHolder;

      Pedal* pedal = new Pedal();
      //measureData->addMusicData(pedal); //can't remember why
      measureData->addCrossMeasureElement(pedal, true);

      if( !jump(1) ) { return false; }

      // is playback
      if( !readBuffer(placeHolder, 1) ) { return false; }
      pedal->setIsPlayback(getHighNibble(placeHolder.toUnsignedInt())!=4);

      if( !jump(1) ) { return false; }

      // common
      if( !parseCommonBlock(pedal) ) { return false; }

      if( !jump(2) ) { return false; }

      // pair lines
      if( !parsePairLinesBlock(pedal) ) { return false; }

      // offset common
      if( !parseOffsetCommonBlock(pedal) ) { return false; }

      // left shoulder
      if( !parseOffsetElement(pedal->getLeftShoulder()) ) { return false; }

      // right shoulder
      if( !parseOffsetElement(pedal->getRightShoulder()) ) { return false; }

      int cursor = ove_->getIsVersion4() ? 0x45 : 0x23;
      int blankCount = ove_->getIsVersion4() ? 42 : 10;

      pedal->setHalf( length > cursor );

      if( !jump(blankCount) ) { return false; }

      if( length > cursor ) {
            if( !jump(2) ) { return false; }

            // handle x offset
            if( !readBuffer(placeHolder, 2) ) { return false; }
            pedal->getPedalHandle()->setXOffset(placeHolder.toInt());

            if( !jump(6) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseKuohao(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      KuoHao* kuoHao = new KuoHao();
      measureData->addMusicData(kuoHao);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(kuoHao) ) { return false; }

      if( !jump(2) ) { return false; }

      // pair lines
      if( !parsePairLinesBlock(kuoHao) ) { return false; }

      if( !jump(4) ) { return false; }

      // left shoulder
      if( !parseOffsetElement(kuoHao->getLeftShoulder()) ) { return false; }

      // right shoulder
      if( !parseOffsetElement(kuoHao->getRightShoulder()) ) { return false; }

      // kuohao type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      kuoHao->setKuohaoType(placeHolder.toUnsignedInt());

      // height
      if( !readBuffer(placeHolder, 1) ) { return false; }
      kuoHao->setHeight(placeHolder.toUnsignedInt());

      int jumpAmount = ove_->getIsVersion4() ? 40 : 8;
      if( !jump(jumpAmount) ) { return false; }

      return true;
      }

bool BarsParse::parseExpressions(MeasureData* measureData, int length) {
      Block placeHolder;

      Expressions* expressions = new Expressions();
      measureData->addMusicData(expressions);

      if( !jump(3) ) { return false; }

      // common00
      if( !parseCommonBlock(expressions) ) { return false; }

      if( !jump(2) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      expressions->setYOffset(placeHolder.toInt());

      // range bar offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      //int barOffset = placeHolder.toUnsignedInt();

      if( !jump(10) ) { return false; }

      // tempo 1
      if( !readBuffer(placeHolder, 2) ) { return false; }
      //double tempo1 = ((double)placeHolder.toUnsignedInt()) / 100.0;

      // tempo 2
      if( !readBuffer(placeHolder, 2) ) { return false; }
      //double tempo2 = ((double)placeHolder.toUnsignedInt()) / 100.0;

      if( !jump(6) ) { return false; }

      // text
      int cursor = ove_->getIsVersion4() ? 35 : 33;
      if( length > cursor ) {
            if( !readBuffer(placeHolder, length-cursor) ) { return false; }
            expressions->setText(ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray()));
            }

      return true;
      }

bool BarsParse::parseHarpPedal(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      HarpPedal* harpPedal = new HarpPedal();
      measureData->addMusicData(harpPedal);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(harpPedal) ) { return false; }

      if( !jump(2) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      harpPedal->setYOffset(placeHolder.toInt());

      // show type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      harpPedal->setShowType(placeHolder.toUnsignedInt());

      // show char flag
      if( !readBuffer(placeHolder, 1) ) { return false; }
      harpPedal->setShowCharFlag(placeHolder.toUnsignedInt());

      if( !jump(8) ) { return false; }

      return true;
      }

bool BarsParse::parseMultiMeasureRest(MeasureData* measureData, int /*length*/) {
      Block placeHolder(2);
      MultiMeasureRest* measureRest = new MultiMeasureRest();
      measureData->addMusicData(measureRest);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(measureRest) ) { return false; }

      if( !jump(6) ) { return false; }

      return true;
      }

bool BarsParse::parseHarmonyGuitarFrame(MeasureData* measureData, int length) {
      Block placeHolder;

      Harmony* harmony = new Harmony();
      measureData->addMusicData(harmony);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(harmony) ) { return false; }

      // root
      if( !readBuffer(placeHolder, 1) ) { return false; }
      harmony->setRoot(placeHolder.toUnsignedInt());

      // type
      if( !readBuffer(placeHolder, 1) ) { return false; }
      //harmony->setHarmonyType((HarmonyType)placeHolder.toUnsignedInt()); // TODO

      // bass
      if( !readBuffer(placeHolder, 1) ) { return false; }
      harmony->setBass(placeHolder.toUnsignedInt());

      int jumpAmount = ove_->getIsVersion4() ? length - 12 : length - 10;
      if( !jump(jumpAmount) ) { return false; }

      return true;
      }

void extractOctave(unsigned int Bits, OctaveShiftType& octaveShiftType, QList<OctaveShiftPosition>& positions) {
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
                  ;
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

bool BarsParse::parseOctaveShift(MeasureData* measureData, int /*length*/) {
      Block placeHolder;

      OctaveShift* octave = new OctaveShift();
      measureData->addCrossMeasureElement(octave, true);

      if( !jump(3) ) { return false; }

      // common
      if( !parseCommonBlock(octave) ) { return false; }

      // octave
      if( !readBuffer(placeHolder, 1) ) { return false; }
      unsigned int type = getLowNibble(placeHolder.toUnsignedInt());
      OctaveShiftType octaveShiftType = OctaveShiftType::OS_8;
      QList<OctaveShiftPosition> positions;
      extractOctave(type, octaveShiftType, positions);

      octave->setOctaveShiftType(octaveShiftType);

      if( !jump(1) ) { return false; }

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      octave->setYOffset(placeHolder.toInt());

      if( !jump(4) ) { return false; }

      // length
      if( !readBuffer(placeHolder, 2) ) { return false; }
      octave->setLength(placeHolder.toUnsignedInt());

      // end tick
      if( !readBuffer(placeHolder, 2) ) { return false; }
      octave->setEndTick(placeHolder.toUnsignedInt());

      // start & stop maybe appear in same measure
      for (int i=0; i<positions.size(); ++i) {
            OctaveShiftPosition position = positions[i];
            OctaveShiftEndPoint* octavePoint = new OctaveShiftEndPoint();
            measureData->addMusicData(octavePoint);

            octavePoint->copyCommonBlock(*octave);
            octavePoint->setOctaveShiftType(octaveShiftType);
            octavePoint->setOctaveShiftPosition(position);
            octavePoint->setEndTick(octave->getEndTick());

            // stop
            if( i==0 && position == OctaveShiftPosition::Stop ) {
                  octavePoint->start()->setOffset(octave->start()->getOffset()+octave->getLength());
                  }

            // end point
            if( i>0 ) {
                  octavePoint->start()->setOffset(octave->start()->getOffset()+octave->getLength());
                  octavePoint->setTick(octave->getEndTick());
                  }
            }

      return true;
      }

bool BarsParse::parseMidiController(MeasureData* measureData, int /*length*/) {
      Block placeHolder;
      MidiController* controller = new MidiController();
      measureData->addMidiData(controller);

      parseMidiCommon(controller);

      // value [0, 128)
      if( !readBuffer(placeHolder, 1) ) { return false; }
      controller->setValue(placeHolder.toUnsignedInt());

      // controller number
      if( !readBuffer(placeHolder, 1) ) { return false; }
      controller->setController(placeHolder.toUnsignedInt());

      if( ove_->getIsVersion4() ) {
            if( !jump(2) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseMidiProgramChange(MeasureData* measureData, int /*length*/) {
      Block placeHolder;
      MidiProgramChange* program = new MidiProgramChange();
      measureData->addMidiData(program);

      parseMidiCommon(program);

      if( !jump(1) ) { return false; }

      // patch
      if( !readBuffer(placeHolder, 1) ) { return false; }
      program->setPatch(placeHolder.toUnsignedInt());

      if( ove_->getIsVersion4() ) {
            if( !jump(2) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseMidiChannelPressure(MeasureData* measureData, int /*length*/) {
      Block placeHolder;
      MidiChannelPressure* pressure = new MidiChannelPressure();
      measureData->addMidiData(pressure);

      parseMidiCommon(pressure);

      if( !jump(1) ) { return false; }

      // pressure
      if( !readBuffer(placeHolder, 1) ) { return false; }
      pressure->setPressure(placeHolder.toUnsignedInt());

      if( ove_->getIsVersion4() )
            {
            if( !jump(2) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseMidiPitchWheel(MeasureData* measureData, int /*length*/) {
      Block placeHolder;
      MidiPitchWheel* wheel = new MidiPitchWheel();
      measureData->addMidiData(wheel);

      parseMidiCommon(wheel);

      // pitch wheel
      if( !readBuffer(placeHolder, 2) ) { return false; }
      int value = placeHolder.toUnsignedInt();
      wheel->setValue(value);

      if( ove_->getIsVersion4() ) {
            if( !jump(2) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseSizeBlock(int length) {
      if( !jump(length) ) { return false; }

      return true;
      }

bool BarsParse::parseMidiCommon(MidiData* ptr) {
      Block placeHolder;

      if( !jump(3) ) { return false; }

      // start position
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->setTick(placeHolder.toUnsignedInt());

      return true;
      }

bool BarsParse::parseCommonBlock(MusicData* ptr) {
      Block placeHolder;

      // start tick
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->setTick(placeHolder.toInt());

      // start unit
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->start()->setOffset(placeHolder.toInt());

      if( ove_->getIsVersion4() ) {
            // color
            if( !readBuffer(placeHolder, 1) ) { return false; }
            ptr->setColor(placeHolder.toUnsignedInt());

            if( !jump(1) ) { return false; }
            }

      return true;
      }

bool BarsParse::parseOffsetCommonBlock(MusicData* ptr) {
      Block placeHolder;

      // offset measure
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->stop()->setMeasure(placeHolder.toUnsignedInt());

      // end unit
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->stop()->setOffset(placeHolder.toInt());

      return true;
      }

bool BarsParse::parsePairLinesBlock(PairEnds* ptr) {
      Block placeHolder;

      // left line
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->getLeftLine()->setLine(placeHolder.toInt());

      // right line
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->getRightLine()->setLine(placeHolder.toInt());

      return true;
      }

bool BarsParse::parseOffsetElement(OffsetElement* ptr) {
      Block placeHolder;

      // x offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->setXOffset(placeHolder.toInt());

      // y offset
      if( !readBuffer(placeHolder, 2) ) { return false; }
      ptr->setYOffset(placeHolder.toInt());

      return true;
      }

bool BarsParse::getCondElementType(unsigned int byteData, CondType& type) {
      if( byteData == 0x09 ) {
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

bool BarsParse::getBdatElementType(unsigned int byteData, BdatType& type) {
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

///////////////////////////////////////////////////////////////////////////////
LyricChunkParse::LyricChunkParse(OveSong* ove) :
      BasicParse(ove) {
      }

void LyricChunkParse::setLyricChunk(SizeChunk* chunk) {
      chunk_ = chunk;
      }

// only ove3 has this chunk
bool LyricChunkParse::parse() {
      unsigned int i;
      Block* dataBlock = chunk_->getDataBlock();
      unsigned int blockSize = chunk_->getSizeBlock()->toSize();
      StreamHandle handle(dataBlock->data(), blockSize);
      Block placeHolder;

      handle_ = &handle;

      if( !jump(4) ) { return false; }

      // Lyric count
      if( !readBuffer(placeHolder, 2) ) { return false; }
      unsigned int count = placeHolder.toUnsignedInt();

      for( i=0; i<count; ++i ) {
            LyricInfo info;

            if( !readBuffer(placeHolder, 2) ) { return false; }
            //unsigned int size = placeHolder.toUnsignedInt();

            // 0x0D00
            if( !jump(2) ) { return false; }

            // voice
            if( !readBuffer(placeHolder, 1) ) { return false; }
            info.voice_ = placeHolder.toUnsignedInt();

            // verse
            if( !readBuffer(placeHolder, 1) ) { return false; }
            info.verse_ = placeHolder.toUnsignedInt();

            // track
            if( !readBuffer(placeHolder, 1) ) { return false; }
            info.track_ = placeHolder.toUnsignedInt();

            if( !jump(1) ) { return false; }

            // measure
            if( !readBuffer(placeHolder, 2) ) { return false; }
            info.measure_ = placeHolder.toUnsignedInt();

            // word count
            if( !readBuffer(placeHolder, 2) ) { return false; }
            info.wordCount_ = placeHolder.toUnsignedInt();

            // lyric size
            if( !readBuffer(placeHolder, 2) ) { return false; }
            info.lyricSize_ = placeHolder.toUnsignedInt();

            if( !jump(6) ) { return false; }

            // name
            if( !readBuffer(placeHolder, 32) ) { return false; }
            info.name_ = ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray());

            if( info.lyricSize_ > 0 ) {
                  // lyric
                  if( info.lyricSize_ > 0 ) {
                        if( !readBuffer(placeHolder, info.lyricSize_) ) { return false; }
                        info.lyric_ = ove_->getCodecString(placeHolder.fixedSizeBufferToStrByteArray());
                        }

                  if( !jump(4) ) { return false; }

                  // font
                  if( !readBuffer(placeHolder, 2) ) { return false; }
                  info.font_ = placeHolder.toUnsignedInt();

                  if( !jump(1) ) { return false; }

                  // font size
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  info.fontSize_ = placeHolder.toUnsignedInt();

                  // font style
                  if( !readBuffer(placeHolder, 1) ) { return false; }
                  info.fontStyle_ = placeHolder.toUnsignedInt();

                  if( !jump(1) ) { return false; }

                  for( int j=0; j<info.wordCount_; ++j ) {
                        if( !jump(8) ) { return false; }
                        }
                  }

            processLyricInfo(info);
            }

      return true;
      }

bool isSpace(char c) {
      return c == ' ' || c == '\n';
      }

void LyricChunkParse::processLyricInfo(const LyricInfo& info) {
      int i;
      int j;
      int index = 0; //words

      int measureId = info.measure_-1;
      bool changeMeasure = true;
      MeasureData* measureData = 0;
      int trackMeasureCount = ove_->getTrackBarCount();
      QStringList words = info.lyric_.split(" ", QString::SkipEmptyParts);

      while ( index < words.size() && measureId+1 < trackMeasureCount ) {
            if( changeMeasure ) {
                  ++measureId;
                  measureData = ove_->getMeasureData(info.track_, measureId);
                  changeMeasure = false;
                  }

            if( measureData == 0 ) { return; }
            QList<NoteContainer*> containers = measureData->getNoteContainers();
            QList<MusicData*> lyrics = measureData->getMusicDatas(MusicDataType::Lyric);

            for( i=0; i<containers.size() && index<words.size(); ++i ) {
                  if( containers[i]->getIsRest() ) {
                        continue;
                        }

                  for( j=0; j<lyrics.size(); ++j ) {
                        Lyric* lyric = static_cast<Lyric*>(lyrics[j]);

                        if( containers[i]->start()->getOffset() == lyric->start()->getOffset() &&
                            (int)containers[i]->getVoice() == info.voice_ &&
                            lyric->getVerse() == info.verse_ ) {
                              if(index<words.size()) {
                                    QString l = words[index].trimmed();
                                    if(!l.isEmpty()) {
                                          lyric->setLyric(l);
                                          lyric->setVoice(info.voice_);
                                          }
                                    }

                              ++index;
                              }
                        }
                  }

            changeMeasure = true;
            }
      }

///////////////////////////////////////////////////////////////////////////////
TitleChunkParse::TitleChunkParse(OveSong* ove) :
      BasicParse(ove) {
      titleType_ = 0x00000001;
      annotateType_ = 0x00010000;
      writerType_ = 0x00020002;
      copyrightType_ = 0x00030001;
      headerType_ = 0x00040000;
      footerType_ = 0x00050002;
      }

void TitleChunkParse::setTitleChunk(SizeChunk* chunk) {
      chunk_ = chunk;
      }

QByteArray getByteArray(const Block& block) {
      QByteArray array((char*)block.data(), block.size());
      int index0 = array.indexOf('\0');
      array = array.left(index0);

      return array;
      }

bool TitleChunkParse::parse() {
      Block* dataBlockP = chunk_->getDataBlock();
      unsigned int blockSize = chunk_->getSizeBlock()->toSize();
      StreamHandle handle(dataBlockP->data(), blockSize);
      Block typeBlock;
      unsigned int titleType;

      handle_ = &handle;

      if( !readBuffer(typeBlock, 4) ) { return false; }

      titleType = typeBlock.toUnsignedInt();

      if( titleType == titleType_ || titleType == annotateType_ || titleType == writerType_ || titleType == copyrightType_ ) {
            Block offsetBlock;

            if( !readBuffer(offsetBlock, 4) ) { return false; }

            const unsigned int itemCount = 4;
            unsigned int i;

            for( i=0; i<itemCount; ++i ) {
                  if( i>0 ) {
                        //0x 00 AB 00 0C 00 00
                        if( !jump(6) ) { return false; }
                        }

                  Block countBlock;
                  if( !readBuffer(countBlock, 2) ) { return false; }
                  unsigned int titleSize = countBlock.toUnsignedInt();

                  Block dataBlock;
                  if( !readBuffer(dataBlock, titleSize) ) { return false; }

                  QByteArray array = getByteArray(dataBlock);
                  if(!array.isEmpty()) {
                        addToOve(ove_->getCodecString(array), titleType);
                        }
                  }

            return true;
            }

      if( titleType == headerType_ || titleType == footerType_ ) {
            if( !jump(10) ) { return false; }

            Block countBlock;
            if( !readBuffer(countBlock, 2) ) { return false; }
            unsigned int titleSize = countBlock.toUnsignedInt();

            Block dataBlock;
            if( !readBuffer(dataBlock, titleSize) ) { return false; }

            QByteArray array = getByteArray(dataBlock);
            addToOve(ove_->getCodecString(array), titleType);

            //0x 00 AB 00 0C 00 00
            if( !jump(6) ) { return false; }

            return true;
            }

      return false;
      }

void TitleChunkParse::addToOve(const QString& str, unsigned int titleType) {
      if( str.isEmpty() ) { return; }

      if (titleType == titleType_) {
            ove_->addTitle(str);
            }

      if (titleType == annotateType_) {
            ove_->addAnnotate(str);
            }

      if (titleType == writerType_) {
            ove_->addWriter(str);
            }

      if (titleType == copyrightType_) {
            ove_->addCopyright(str);
            }

      if (titleType == headerType_) {
            ove_->addHeader(str);
            }

      if (titleType == footerType_) {
            ove_->addFooter(str);
            }
      }

// OveOrganize.cpp
OveOrganizer::OveOrganizer(OveSong* ove) {
      ove_ = ove;
      }

void OveOrganizer::organize() {
      if(ove_ == NULL) {
            return;
            }

      organizeTracks();
      organizeAttributes();
      organizeMeasures();
      }

void OveOrganizer::organizeAttributes() {
      int i;
      int j;
      int k;

      // key
      if(ove_->getLineCount() > 0) {
            Line* line = ove_->getLine(0);
            int partBarCount = ove_->getPartBarCount();
            int lastKey = 0;

            if(line != 0){
                  for(i=0; i<line->getStaffCount(); ++i) {
                        QPair<int, int> partStaff = ove_->trackToPartStaff(i);
                        Staff* staff = line->getStaff(i);
                        lastKey = staff->getKeyType();

                        for(j=0; j<partBarCount; ++j) {
                              MeasureData* measureData = ove_->getMeasureData(partStaff.first, partStaff.second, j);

                              if(measureData != 0) {
                                    Key* key = measureData->getKey();

                                    if( j==0 ) {
                                          key->setKey(lastKey);
                                          key->setPreviousKey(lastKey);
                                          }

                                    if( !key->getSetKey() ) {
                                          key->setKey(lastKey);
                                          key->setPreviousKey(lastKey);
                                          }
                                    else {
                                          if( key->getKey() != lastKey ) {
                                                lastKey = key->getKey();
                                                }
                                          }
                                    }
                              }
                        }
                  }
            }

      // clef
      if( ove_->getLineCount() > 0 ) {
            Line* line = ove_->getLine(0);
            int partBarCount = ove_->getPartBarCount();
            ClefType lastClefType = ClefType::Treble;

            if(line != 0){
                  for( i=0; i<line->getStaffCount(); ++i ) {
                        QPair<int, int> partStaff = ove_->trackToPartStaff(i);
                        Staff* staff = line->getStaff(i);
                        lastClefType = staff->getClefType();

                        for( j=0; j<partBarCount; ++j ) {
                              MeasureData* measureData = ove_->getMeasureData(partStaff.first, partStaff.second, j);

                              if(measureData != 0) {
                                    Clef* clefPtr = measureData->getClef();
                                    clefPtr->setClefType((int)lastClefType);

                                    const QList<MusicData*>& clefs = measureData->getMusicDatas(MusicDataType::Clef);

                                    for( k=0; k<clefs.size(); ++k ) {
                                          Clef* clef = static_cast<Clef*>(clefs[k]);
                                          lastClefType = clef->getClefType();
                                          }
                                    }
                              }
                        }
                  }
            }
      }

Staff* getStaff(OveSong* ove, int track) {
      if (ove->getLineCount() > 0) {
            Line* line = ove->getLine(0);
            if(line != 0 && line->getStaffCount() > 0) {
                  Staff* staff = line->getStaff(track);
                  return staff;
                  }
            }

      return 0;
      }

void OveOrganizer::organizeTracks() {
      int i;
      //QList<QPair<ClefType, int> > trackChannels;
      QList<Track*> tracks = ove_->getTracks();
      QList<bool> comboStaveStarts;

      for( i=0; i<tracks.size(); ++i ) {
            comboStaveStarts.push_back(false);
            }

      for( i=0; i<tracks.size(); ++i ) {
            Staff* staff = getStaff(ove_, i);
            if(staff != 0) {
                  if(staff->getGroupType() == GroupType::Brace && staff->getGroupStaffCount() == 1 ) {
                        comboStaveStarts[i] = true;
                        }
                  }

            /*if( i < tracks.size() - 1 ) {
         if( tracks[i]->getStartClef() == ClefType::Treble &&
            tracks[i+1]->getStartClef() == ClefType::Bass &&
            tracks[i]->getChannel() == tracks[i+1]->get_channel() ) {
         }
      }*/
            }

      int trackId = 0;
      QList<int> partStaffCounts;

      while( trackId < (int)tracks.size() ) {
            int partTrackCount = 1;

            if( comboStaveStarts[trackId] ) {
                  partTrackCount = 2;
                  }

            partStaffCounts.push_back(partTrackCount);
            trackId += partTrackCount;
            }

      ove_->setPartStaffCounts(partStaffCounts);
      }

void OveOrganizer::organizeMeasures() {
      int trackBarCount = ove_->getTrackBarCount();

      for( int i=0; i<ove_->getPartCount(); ++i ) {
            int partStaffCount = ove_->getStaffCount(i);

            for( int j=0; j<partStaffCount; ++j ) {
                  for( int k=0; k<trackBarCount; ++k ) {
                        Measure* measure = ove_->getMeasure(k);
                        MeasureData* measureData = ove_->getMeasureData(i, j, k);

                        organizeMeasure(i, j, measure, measureData);
                        }
                  }
            }
      }

void OveOrganizer::organizeMeasure(int part, int track, Measure* measure, MeasureData* measureData) {
      // note containers
      organizeContainers(part, track, measure, measureData);

      // single end data
      organizeMusicDatas(part, track, measure, measureData);

      // cross measure elements
      organizeCrossMeasureElements(part, track, measure, measureData);
      }

void addToList(QList<int>& list, int number) {
      for(int i=0; i<list.size(); ++i){
            if(list[i] == number){
                  return;
                  }
            }

      list.push_back(number);
      }

void OveOrganizer::organizeContainers(int /*part*/, int /*track*/,
                                      Measure* measure, MeasureData* measureData) {
      int i;
      QList<NoteContainer*> containers = measureData->getNoteContainers();
      int barUnits = measure->getTime()->getUnits();
      QList<int> voices;

      for(i=0; i<containers.size(); ++i){
            int endUnit = barUnits;
            if( i < containers.size() - 1 ) {
                  endUnit = containers[i+1]->start()->getOffset();
                  }

            containers[i]->stop()->setOffset(endUnit);
            addToList(voices, containers[i]->getVoice());
            }

      // shift voices
      qSort(voices.begin(), voices.end());

      for (i = 0; i < voices.size(); ++i) {
            int voice = voices[i];
            // voice -> i
            for(int j=0; j<(int)containers.size(); ++j) {
                  int avoice = containers[j]->getVoice();
                  if ( avoice == voice && avoice != i ) {
                        containers[j]->setVoice(i);
                        }
                  }
            }
      }

void OveOrganizer::organizeMusicDatas(int /*part*/, int /*track*/, Measure* measure, MeasureData* measureData) {
      int i;
      int barIndex = measure->getBarNumber()->getIndex();
      QList<MusicData*> datas = measureData->getMusicDatas(MusicDataType::None);

      for(i=0; i<datas.size(); ++i) {
            datas[i]->start()->setMeasure(barIndex);
            }
      }

void OveOrganizer::organizeCrossMeasureElements(int part, int track, Measure* measure, MeasureData* measureData) {
      int i;
      QList<MusicData*> pairs = measureData->getCrossMeasureElements(MusicDataType::None, MeasureData::PairType::Start);

      for(i=0; i<pairs.size(); ++i) {
            MusicData* pair = pairs[i];

            switch ( pair->getMusicDataType() ) {
                  case MusicDataType::Beam :
                  case MusicDataType::Glissando :
                  case MusicDataType::Slur :
                  case MusicDataType::Tie :
                  case MusicDataType::Tuplet :
                  case MusicDataType::Pedal :
                  case MusicDataType::Numeric_Ending :
                        //case MusicDataType::OctaveShift_EndPoint :
                  case MusicDataType::Measure_Repeat : {
                        organizePairElement(pair, part, track, measure, measureData);
                        break;
                        }
                  case MusicDataType::OctaveShift : {
                        OctaveShift* octave = static_cast<OctaveShift*>(pair);
                        organizeOctaveShift(octave, measure, measureData);
                        break;
                        }
                  case MusicDataType::Wedge : {
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
            MeasureData* measureData) {
      int bar1Index = measure->getBarNumber()->getIndex();
      int bar2Index = bar1Index + data->stop()->getMeasure();
      MeasureData* measureData2 = ove_->getMeasureData(part, track, bar2Index);

      data->start()->setMeasure(bar1Index);

      if(measureData2 != 0 && measureData != measureData2) {
            measureData2->addCrossMeasureElement(data, false);
            }

      if( data->getMusicDataType() == MusicDataType::Tuplet ){
            Tuplet* tuplet = static_cast<Tuplet*>(data);
            const QList<NoteContainer*> containers = measureData->getNoteContainers();

            for(int i=0; i<containers.size(); ++i){
                  if(containers[i]->getTick() > tuplet->getTick()){
                        break;
                        }

                  if(containers[i]->getTick() == tuplet->getTick()){
                        tuplet->setNoteType(containers[i]->getNoteType());
                        }
                  }

            int tupletTick = NoteTypeToTick(tuplet->getNoteType(), ove_->getQuarter())*tuplet->getSpace();
            if( tuplet->getTick() % tupletTick != 0 ) {
                  int newStartTick = (tuplet->getTick() / tupletTick) * tupletTick;

                  for(int i=0; i<containers.size(); ++i){
                        if( containers[i]->getTick() == newStartTick &&
                            containers[i]->getTuplet() == tuplet->getTuplet()) {
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
            MeasureData* measureData) {
      // octave shift
      int i;
      const QList<NoteContainer*> containers = measureData->getNoteContainers();
      int barIndex = measure->getBarNumber()->getIndex();

      octave->start()->setMeasure(barIndex);

      for(i=0; i<containers.size(); ++i) {
            int noteShift = octave->getNoteShift();
            int containerTick = containers[i]->getTick();

            if( octave->getTick() <= containerTick && octave->getEndTick() > containerTick ) {
                  containers[i]->setNoteShift(noteShift);
                  }
            }
      }

bool getMiddleUnit(
            OveSong* ove, int /*part*/, int /*track*/,
            Measure* measure1, Measure* measure2, int unit1, int /*unit2*/,
            Measure* middleMeasure, int& middleUnit) {
      Q_UNUSED(middleMeasure); // avoid (bogus?) warning to show up
      QList<int> barUnits;
      int i;
      int bar1Index = measure1->getBarNumber()->getIndex();
      int bar2Index = measure2->getBarNumber()->getIndex();
      int sumUnit = 0;

      for( int j=bar1Index; j<=bar2Index; ++j ) {
            Measure* measure = ove->getMeasure(j);
            barUnits.push_back(measure->getTime()->getUnits());
            sumUnit += measure->getTime()->getUnits();
            }

      int currentSumUnit = 0;
      for( i=0; i<barUnits.size(); ++i ) {
            int barUnit = barUnits[i];

            if( i==0 ) {
                  barUnit = barUnits[i] - unit1;
                  }

            if( currentSumUnit + barUnit < sumUnit/2 ) {
                  currentSumUnit += barUnit;
                  }
            else {
                  break;
                  }
            }

      if( i < barUnits.size() ) {
            int barMiddleIndex = bar1Index + i;
            middleMeasure = ove->getMeasure(barMiddleIndex);
            middleUnit = sumUnit/2 - currentSumUnit;

            return true;
            }

      return false;
      }

void OveOrganizer::organizeWedge(Wedge* wedge, int part, int track, Measure* measure, MeasureData* measureData) {
      int bar1Index = measure->getBarNumber()->getIndex();
      int bar2Index = bar1Index + wedge->stop()->getMeasure();
      MeasureData* measureData2 = ove_->getMeasureData(part, track, bar2Index);
      WedgeType wedgeType = wedge->getWedgeType();

      if( wedge->getWedgeType() == WedgeType::Double_Line ) {
            wedgeType = WedgeType::Cres_Line;
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

      if(measureData2 != 0) {
            measureData2->addMusicData(stopPoint);
            }

      if( wedge->getWedgeType() == WedgeType::Double_Line ) {
            Measure* middleMeasure = NULL;
            int middleUnit = 0;

            getMiddleUnit(
                              ove_, part, track,
                              measure, ove_->getMeasure(bar2Index),
                              wedge->start()->getOffset(), wedge->stop()->getOffset(),
                              middleMeasure, middleUnit);

            if( middleUnit != 0 ) {
                  WedgeEndPoint* midStopPoint = new WedgeEndPoint();
                  measureData->addMusicData(midStopPoint);

                  midStopPoint->setTick(wedge->getTick());
                  midStopPoint->start()->setOffset(middleUnit);
                  midStopPoint->setWedgeStart(false);
                  midStopPoint->setWedgeType(WedgeType::Cres_Line);
                  midStopPoint->setHeight(wedge->getHeight());

                  WedgeEndPoint* midStartPoint = new WedgeEndPoint();
                  measureData->addMusicData(midStartPoint);

                  midStartPoint->setTick(wedge->getTick());
                  midStartPoint->start()->setOffset(middleUnit);
                  midStartPoint->setWedgeStart(true);
                  midStartPoint->setWedgeType(WedgeType::Decresc_Line);
                  midStartPoint->setHeight(wedge->getHeight());
                  }
            }
      }


// OveSerialize.cpp
enum class ChunkType : char {
      OVSC = 00 ,
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

ChunkType nameToChunkType(const NameBlock& name) {
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

int chunkTypeToMaxTimes(ChunkType type) {
      int maxTimes = -1; // no limit

      switch (type) {
            case ChunkType::OVSC: {
                  maxTimes = 1;
                  break;
                  }
            case ChunkType::TRKL: {// case ChunkType::TRAK :
                  maxTimes = 1;
                  break;
                  }
            case ChunkType::PAGL: {// case ChunkType::PAGE :
                  maxTimes = 1;
                  break;
                  }
                  // case ChunkType::LINE :
                  // case ChunkType::STAF :
            case ChunkType::LINL: {
                  maxTimes = 1;
                  break;
                  }
                  // case ChunkType::MEAS :
                  // case ChunkType::COND :
                  // case ChunkType::BDAT :
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
                  // case ChunkType::NONE :
            default:
                  break;
            }

      return maxTimes;
      }

///////////////////////////////////////////////////////////////////////////////////////////

OveSerialize::OveSerialize() :
      ove_(0),
      streamHandle_(0),
      notify_(0) {
      }

OveSerialize::~OveSerialize() {
      if(streamHandle_ != 0) {
            delete streamHandle_;
            streamHandle_ = 0;
            }
      }

void OveSerialize::setOve(OveSong* ove) {
      ove_ = ove;
      }

void OveSerialize::setFileStream(unsigned char* buffer, unsigned int size) {
      streamHandle_ = new StreamHandle(buffer, size);
      }

void OveSerialize::setNotify(IOveNotify* notify) {
      notify_ = notify;
      }

void OveSerialize::messageOutError() {
      if (notify_ != NULL) {
            notify_->loadError();
            }
      }

void OveSerialize::messageOut(const QString& str) {
      if (notify_ != NULL) {
            notify_->loadInfo(str);
            }
      }

bool OveSerialize::load(void) {
      if(streamHandle_ == 0)
            return false;

      if( !readHeader() ) {
            messageOutError();
            return false;
            }

      unsigned int i;
      QMap<ChunkType, int> chunkTimes;
      //bool firstEnter = true;

      for( i=(int)ChunkType::OVSC; i<(int)ChunkType::NONE; ++i ) {
            chunkTimes[(ChunkType)i] = 0;
            }

      ChunkType chunkType = ChunkType::NONE;

      do {
            NameBlock nameBlock;
            SizeChunk sizeChunk;

            if( !readNameBlock(nameBlock) ) { return false; }

            chunkType = nameToChunkType(nameBlock);
            ++chunkTimes[chunkType];
            int maxTime = chunkTypeToMaxTimes(chunkType);

            if( maxTime > 0 && chunkTimes[chunkType] > maxTime ) {
                  messageOut("format not support, chunk appear more than accept.\n");
                  return false;
                  }

            switch (chunkType) {
                  /*case ChunkType::OVSC :
       {
       if( !readHeadData(&sizeChunk) )
       {
       messageOut_error();
       return false;
       }

       break;
       }*/
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

                        LyricChunkParse parse(ove_);

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

                        TitleChunkParse titleChunkParse(ove_);

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
                        /*if( firstEnter )
          {
          QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.\n";
          messageOut(info);
          messageOutError();

          return false;
          }*/

                        break;
                  }

            //firstEnter = false;
            }
      while ( chunkType != ChunkType::NONE );

      // if( !readOveEnd() ) { return false; }

      // organize OveData
      OVE::OveOrganizer organizer(ove_);
      organizer.organize();

      return true;
      }

void OveSerialize::release() {
      delete this;
      }

bool OveSerialize::readHeader() {
      ChunkType chunkType = ChunkType::NONE;
      NameBlock nameBlock;
      SizeChunk sizeChunk;

      if (!readNameBlock(nameBlock)) {
            return false;
            }

      chunkType = nameToChunkType(nameBlock);
      //int maxTime = chunkTypeToMaxTimes(chunkType);

      if (chunkType == ChunkType::OVSC) {
            if (readHeadData(&sizeChunk)) {
                  return true;
                  }
            }

      QString info = "Not compatible file, try to load and save with newer version, Overture 4 is recommended.\n";
      messageOut(info);

      return false;
      }

bool OveSerialize::readHeadData(SizeChunk* ovscChunk) {
      if (!readSizeChunk(ovscChunk))
            return false;

      OvscParse ovscParse(ove_);

      ovscParse.setNotify(notify_);
      ovscParse.setOvsc(ovscChunk);

      return ovscParse.parse();
      }

bool OveSerialize::readTracksData() {
      GroupChunk trackGroupChunk;

      if (!readGroupChunk(&trackGroupChunk))
            return false;

      unsigned int i;
      unsigned short trackCount = trackGroupChunk.getCountBlock()->toCount();

      for (i = 0; i < trackCount; ++i) {
            SizeChunk* trackChunk = new SizeChunk();

            if (ove_->getIsVersion4()) {
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

            TrackParse trackParse(ove_);

            trackParse.setTrack(trackChunk);
            trackParse.parse();
            }

      return true;
      }

bool OveSerialize::readPagesData() {
      GroupChunk pageGroupChunk;

      if (!readGroupChunk(&pageGroupChunk))
            return false;

      unsigned short pageCount = pageGroupChunk.getCountBlock()->toCount();
      unsigned int i;
      PageGroupParse parse(ove_);

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

bool OveSerialize::readLinesData() {
      GroupChunk lineGroupChunk;
      if (!readGroupChunk(&lineGroupChunk))
            return false;

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

            StaffCountGetter getter(ove_);
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

      LineGroupParse parse(ove_);

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

bool OveSerialize::readBarsData() {
      GroupChunk barGroupChunk;
      if (!readGroupChunk(&barGroupChunk))
            return false;

      unsigned short measCount = barGroupChunk.getCountBlock()->toCount();
      int i;

      QList<SizeChunk*> measureChunks;
      QList<SizeChunk*> conductChunks;
      QList<SizeChunk*> bdatChunks;

      ove_->setTrackBarCount(measCount);

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

            if (!readChunkName(conductChunkPtr, Chunk::ConductName))
                  return false;

            if (!readSizeChunk(conductChunkPtr))
                  return false;

            conductChunks.push_back(conductChunkPtr);
            }

      int bdatCount = ove_->getTrackCount() * measCount;
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
      BarsParse barsParse(ove_);

      for (i = 0; i < (int) measureChunks.size(); ++i) {
            barsParse.addMeasure(measureChunks[i]);
            }

      for (i = 0; i < (int) conductChunks.size(); ++i) {
            barsParse.addConduct(conductChunks[i]);
            }

      for (i = 0; i < (int) bdatChunks.size(); ++i) {
            barsParse.addBdat(bdatChunks[i]);
            }

      barsParse.setNotify(notify_);
      if (!barsParse.parse()) {
            return false;
            }

      return true;
      }

bool OveSerialize::readOveEnd() {
      if (streamHandle_ == 0)
            return false;

      const unsigned int END_OVE1 = 0xffffffff;
      const unsigned int END_OVE2 = 0x00000000;
      unsigned int buffer;

      if (!streamHandle_->read((char*) &buffer, sizeof(unsigned int)))
            return false;

      if (buffer != END_OVE1)
            return false;

      if (!streamHandle_->read((char*) &buffer, sizeof(unsigned int)))
            return false;

      if (buffer != END_OVE2)
            return false;

      return true;
      }

/////////////////////////////////////////////////////////////////////////////////////////
bool OveSerialize::readNameBlock(NameBlock& nameBlock) {
      if (streamHandle_ == 0)
            return false;

      if (!streamHandle_->read((char*) nameBlock.data(), nameBlock.size()))
            return false;

      return true;
      }

bool OveSerialize::readChunkName(Chunk* /*chunk*/, const QString& name) {
      if (streamHandle_ == 0)
            return false;

      NameBlock nameBlock;

      if (!streamHandle_->read((char*) nameBlock.data(), nameBlock.size()))
            return false;

      if (!(nameBlock.toStrByteArray() == name))
            return false;

      return true;
      }

bool OveSerialize::readSizeChunk(SizeChunk* sizeChunk) {
      if (streamHandle_ == 0)
            return false;

      SizeBlock* sizeBlock = sizeChunk->getSizeBlock();

      if (!streamHandle_->read((char*) sizeBlock->data(), sizeBlock->size()))
            return false;

      unsigned int blockSize = sizeBlock->toSize();

      sizeChunk->getDataBlock()->resize(blockSize);

      Block* dataBlock = sizeChunk->getDataBlock();

      if (!streamHandle_->read((char*) dataBlock->data(), blockSize))
            return false;

      return true;
      }

bool OveSerialize::readDataChunk(Block* block, unsigned int size) {
      if (streamHandle_ == 0)
            return false;

      block->resize(size);

      if (!streamHandle_->read((char*) block->data(), size))
            return false;

      return true;
      }

bool OveSerialize::readGroupChunk(GroupChunk* groupChunk) {
      if (streamHandle_ == 0)
            return false;

      CountBlock* countBlock = groupChunk->getCountBlock();

      if (!streamHandle_->read((char*) countBlock->data(), countBlock->size()))
            return false;

      return true;
      }

IOVEStreamLoader* createOveStreamLoader() {
      return new OveSerialize;
      }

}

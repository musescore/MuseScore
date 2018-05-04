//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: importove.cpp 3763 2010-12-15 15:50:09Z vanferry $
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

#include "globals.h"
//#include "musescore.h"
#include "libmscore/sig.h"
#include "libmscore/tempo.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/glissando.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/accidental.h"
#include "libmscore/ottava.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/pitchspelling.h"
#include "preferences.h"
#include "libmscore/repeat.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/staff.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/tuplet.h"
#include "libmscore/tremolo.h"
#include "libmscore/volta.h"
#include "libmscore/chordlist.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "libmscore/sym.h"
#include "libmscore/bracketItem.h"

using namespace Ms;

class MeasureToTick {
public:
      MeasureToTick();
      ~MeasureToTick();

public:
      void build(OVE::OveSong* ove, int quarter);

      int getTick(int measure, int tick_pos);
      static int unitToTick(int unit, int quarter);

      struct TimeTick	{
            int numerator_;
            int denominator_;
            int measure_;
            int tick_;
            bool isSymbol_;

            TimeTick():numerator_(4), denominator_(4), measure_(0), tick_(0), isSymbol_(false){}
            };
      QList<TimeTick> getTimeTicks() const;

private:
      int quarter_;
      OVE::OveSong* ove_;
      QList<TimeTick> tts_;
      };

int getMeasureTick(int quarter, int num, int den){
      return quarter * 4 * num / den;
      }

MeasureToTick::MeasureToTick(){
      quarter_ = 480;
      ove_ = 0;
      }

MeasureToTick::~MeasureToTick(){
      }

void MeasureToTick::build(OVE::OveSong* ove, int quarter){
      unsigned int i;
      int currentTick = 0;
      unsigned int measureCount = ove->getMeasureCount();

      quarter_ = quarter;
      ove_ = ove;
      tts_.clear();

      for(i=0; i<measureCount; ++i)	{
            OVE::Measure* measure = ove_->getMeasure(i);
            OVE::TimeSignature* time = measure->getTime();
            TimeTick tt;
            bool change = false;

            tt.tick_ = currentTick;
            tt.numerator_ = time->getNumerator();
            tt.denominator_ = time->getDenominator();
            tt.measure_ = i;
            tt.isSymbol_ = time->getIsSymbol();

            if( i == 0 ){
                  change = true;
                  } else {
                  OVE::TimeSignature* previousTime = ove_->getMeasure(i-1)->getTime();

                  if( time->getNumerator() != previousTime->getNumerator() ||
                      time->getDenominator() != previousTime->getDenominator() ){
                        change = true;
                        } else if(time->getIsSymbol() != previousTime->getIsSymbol()){
                        change = true;
                        }
                  }

            if( change ){
                  tts_.push_back(tt);
                  }

            currentTick += getMeasureTick(quarter_, tt.numerator_, tt.denominator_);
            }
      }

int MeasureToTick::getTick(int measure, int tick_pos){
      TimeTick tt;

      for(int i=0; i<tts_.size(); ++i) {
            if( measure >= tts_[i].measure_ && ( i==tts_.size()-1 || measure < tts_[i+1].measure_ ) ) {
                  int measuresTick = (measure - tts_[i].measure_) * getMeasureTick(quarter_, tts_[i].numerator_, tts_[i].denominator_);
                  return tts_[i].tick_ + measuresTick + tick_pos;
                  }
            }

      return 0;
      }

int MeasureToTick::unitToTick(int unit, int quarter) {
      // 0x100 correspond to quarter tick
      float ratio = (float)unit / (float)256.0;
      int tick = ratio * quarter;
      return tick;
      }

QList<MeasureToTick::TimeTick> MeasureToTick::getTimeTicks() const {
      return tts_;
      }

///////////////////////////////////////////////////////////////////
class OveToMScore {
public:
      OveToMScore();
      ~OveToMScore();

public:
      void convert(OVE::OveSong* oveData, Score* score);

private:
      void createStructure();
      void convertHeader();
      void convertGroups();
      void convertTrackHeader(OVE::Track* track, Part* part);
      void convertTrackElements(int track);
      void convertLineBreak();
      void convertSignatures();
      void convertMeasures();
      void convertMeasure(Measure* measure);
      void convertMeasureMisc(Measure* measure, int part, int staff, int track);
      void convertNotes(Measure* measure, int part, int staff, int track);
      void convertArticulation(Measure* measure, ChordRest* cr, int track, int absTick, OVE::Articulation* art);
      void convertLyrics(Measure* measure, int part, int staff, int track);
      void convertHarmonys(Measure* measure, int part, int staff, int track);
      void convertRepeats(Measure* measure, int part, int staff, int track);
      void convertDynamics(Measure* measure, int part, int staff, int track);
      void convertExpressions(Measure* measure, int part, int staff, int track);
      void convertLines(Measure* measure);
      void convertSlurs(Measure* measure, int part, int staff, int track);
      void convertGlissandos(Measure* measure, int part, int staff, int track);
      void convertWedges(Measure* measure, int part, int staff, int track);
      void convertOctaveShifts(Measure* measure, int part, int staff, int track);

      OVE::NoteContainer* getContainerByPos(int part, int staff, const OVE::MeasurePos& pos);
      //OVE::MusicData* getMusicDataByUnit(int part, int staff, int measure, int unit, OVE::MusicDataType type);
      OVE::MusicData* getCrossMeasureElementByPos(int part, int staff, const OVE::MeasurePos& pos, int voice, OVE::MusicDataType type);
      ChordRest* findChordRestByPos(int absTick, int track);

      void clearUp();

private:
      OVE::OveSong* ove_;
      Score* score_;
      MeasureToTick* mtt_;

      Pedal* pedal_;
      };

OveToMScore::OveToMScore() {
      ove_ = 0;
      mtt_ = new MeasureToTick();
      pedal_ = 0;
      }

OveToMScore::~OveToMScore() {
      delete mtt_;
      }

void OveToMScore::convert(OVE::OveSong* ove, Score* score) {
      ove_ = ove;
      score_ = score;
      mtt_->build(ove_, ove_->getQuarter());

      convertHeader();
      createStructure();
      convertGroups();
      convertSignatures();
      //convertLineBreak();

      int staffCount = 0;
      for(int i=0; i<ove_->getPartCount(); ++i ){
            int partStaffCount = ove_->getStaffCount(i) ;
            Part* part = score_->parts().at(i);

            for(int j=0; j<partStaffCount; ++j){
                  OVE::Track* track = ove_->getTrack(i, j);

                  convertTrackHeader(track, part);
                  }

            staffCount += partStaffCount;
            }

      convertMeasures();

      // convert elements by ove track sequence
      staffCount = 0;
      for(int i=0; i<ove_->getPartCount(); ++i ){
            int partStaffCount = ove_->getStaffCount(i) ;

            for(int j=0; j<partStaffCount; ++j){
                  int trackIndex = staffCount + j;

                  convertTrackElements(trackIndex);
                  }

            staffCount += partStaffCount;
            }

      clearUp();
      }

void OveToMScore::createStructure() {
      int i;
      for(i=0; i<ove_->getPartCount(); ++i ){
            int partStaffCount = ove_->getStaffCount(i) ;
            Part* part = new Part(score_);

            for(int j=0; j<partStaffCount; ++j){
                  //OVE::Track* track = ove_->getTrack(i, j);
                  Staff* staff = new Staff(score_);
                  staff->setPart(part);

                  part->staves()->push_back(staff);
                  score_->staves().push_back(staff);
                  }

            score_->appendPart(part);
            part->setStaves(partStaffCount);
            }

      for(i = 0; i <ove_->getMeasureCount(); ++i) {
            Measure* measure  = new Measure(score_);
            int tick = mtt_->getTick(i, 0);
            measure->setTick(tick);
            measure->setNo(i);
            score_->measures()->add(measure);
            }
      }

void OveToMScore::clearUp() {
      if(pedal_ != NULL) {
            delete pedal_;
            pedal_ = 0;
            }
      }

OVE::Staff* getStaff(const OVE::OveSong* ove, int track) {
      if (ove->getLineCount() > 0) {
            OVE::Line* line = ove->getLine(0);
            if(line != 0 && line->getStaffCount() > 0) {
                  OVE::Staff* staff = line->getStaff(track);
                  return staff;
                  }
            }

      return 0;
      }

void addText(VBox* & vbox, Score* s, QString strTxt, SubStyle stl) {
      if (!strTxt.isEmpty()) {
            Text* text = new Text(s);
            text->initSubStyle(stl);
            text->setPlainText(strTxt);
            if(vbox == 0) {
                  vbox = new VBox(s);
                  }
            vbox->add(text);
            }
      }

void OveToMScore::convertHeader() {
      VBox* vbox = 0;
      QList<QString> titles = ove_->getTitles();
      if( !titles.empty() && !titles[0].isEmpty() ) {
            QString title = titles[0];
            score_->setMetaTag("movementTitle", title);
            addText(vbox, score_, title, SubStyle::TITLE);
            }

      QList<QString> copyrights = ove_->getCopyrights();
      if( !copyrights.empty() && !copyrights[0].isEmpty() ) {
            QString copyright = copyrights[0];
            score_->setMetaTag("copyright", copyright);
            }

      QList<QString> annotates = ove_->getAnnotates();
      if( !annotates.empty() && !annotates[0].isEmpty() ) {
            QString annotate = annotates[0];
            addText(vbox, score_, annotate, SubStyle::POET);
            }

      QList<QString> writers = ove_->getWriters();
      if(!writers.empty()) {
            QString composer = writers[0];
            score_->setMetaTag("composer", composer);
            addText(vbox, score_, composer, SubStyle::COMPOSER);
            }

      if(writers.size() > 1) {
            QString lyricist = writers[1];
            addText(vbox, score_, lyricist, SubStyle::POET);
            }

      if (vbox) {
            vbox->setTick(0);
            score_->measures()->add(vbox);
            }
      }

void OveToMScore::convertGroups() {
      int i;
      int staffCount = 0;
      const QList<Part*>& parts = score_->parts();
      for(i=0; i<ove_->getPartCount(); ++i ){
            int partStaffCount = ove_->getStaffCount(i);
            //if(parts == 0)
            //	continue;
            Part* part = parts.at(i);
            if(part == 0)
                  continue;

            for(int j=0; j<partStaffCount; ++j){
                  int staffIndex = staffCount + j;
                  Staff* staff = score_->staff(staffIndex);
                  if(staff == 0)
                        continue;

                  // brace
                  if( j == 0 && partStaffCount == 2 ) {
                        staff->setBracketType(0, BracketType::BRACE);
                        staff->setBracketSpan(0, 2);
                        staff->setBarLineSpan(2);
                        }

                  // bracket
                  OVE::Staff* staffPtr = getStaff(ove_, staffIndex);
                  if(staffPtr != 0 && staffPtr->getGroupType() == OVE::GroupType::Bracket) {
                        int span = staffPtr->getGroupStaffCount() + 1;
                        int endStaff = staffIndex + span;
                        if(span > 0 && endStaff >= staffIndex && endStaff <= ove_->getTrackCount()) {
                              staff->addBracket(new BracketItem(staff->score(), BracketType::NORMAL, span));
                              staff->setBarLineSpan(span);
                              }
                        }
                  }

            staffCount += partStaffCount;
            }
      }

ClefType OveClefToClef(OVE::ClefType type){
      ClefType clef = ClefType::G;
      switch(type){
            case OVE::ClefType::Treble:{
                  clef = ClefType::G;
                  break;
                  }
            case OVE::ClefType::Bass:{
                  clef = ClefType::F;
                  break;
                  }
            case OVE::ClefType::Alto:{
                  clef = ClefType::C3;
                  break;
                  }
            case OVE::ClefType::UpAlto:{
                  clef = ClefType::C4;
                  break;
                  }
            case OVE::ClefType::DownDownAlto:{
                  clef = ClefType::C1;
                  break;
                  }
            case OVE::ClefType::DownAlto:{
                  clef = ClefType::C2;
                  break;
                  }
            case OVE::ClefType::UpUpAlto:{
                  clef = ClefType::C5;
                  break;
                  }
            case OVE::ClefType::Treble8va:{
                  clef = ClefType::G8_VA;
                  break;
                  }
            case OVE::ClefType::Bass8va:{
                  clef = ClefType::F_8VA;
                  break;
                  }
            case OVE::ClefType::Treble8vb:{
                  clef = ClefType::G8_VB;
                  break;
                  }
            case OVE::ClefType::Bass8vb:{
                  clef = ClefType::F8_VB;
                  break;
                  }
            case OVE::ClefType::Percussion1:{
                  clef = ClefType::PERC;
                  break;
                  }
            case OVE::ClefType::Percussion2:{
                  clef = ClefType::PERC2;
                  break;
                  }
            case OVE::ClefType::TAB:{
                  clef = ClefType::TAB;
                  break;
                  }
            default:
                  break;
            }
      return clef;
      }

NoteHead::Group getHeadGroup(OVE::NoteHeadType type) {
      NoteHead::Group headGroup = NoteHead::Group::HEAD_NORMAL;
      switch (type) {
            case OVE::NoteHeadType::Standard: {
                  headGroup = NoteHead::Group::HEAD_NORMAL;
                  break;
                  }
            case OVE::NoteHeadType::Invisible: {
                  break;
                  }
            case OVE::NoteHeadType::Rhythmic_Slash: {
                  headGroup = NoteHead::Group::HEAD_SLASH;
                  break;
                  }
            case OVE::NoteHeadType::Percussion: {
                  headGroup = NoteHead::Group::HEAD_XCIRCLE;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Rhythm: {
                  headGroup = NoteHead::Group::HEAD_CROSS;
                  break;
                  }
            case OVE::NoteHeadType::Open_Rhythm: {
                  headGroup = NoteHead::Group::HEAD_CROSS;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Slash: {
                  headGroup = NoteHead::Group::HEAD_SLASH;
                  break;
                  }
            case OVE::NoteHeadType::Open_Slash: {
                  headGroup = NoteHead::Group::HEAD_SLASH;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Do: {
                  headGroup = NoteHead::Group::HEAD_DO;
                  break;
                  }
            case OVE::NoteHeadType::Open_Do: {
                  headGroup = NoteHead::Group::HEAD_DO;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Re: {
                  headGroup = NoteHead::Group::HEAD_RE;
                  break;
                  }
            case OVE::NoteHeadType::Open_Re: {
                  headGroup = NoteHead::Group::HEAD_RE;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Mi: {
                  headGroup = NoteHead::Group::HEAD_MI;
                  break;
                  }
            case OVE::NoteHeadType::Open_Mi: {
                  headGroup = NoteHead::Group::HEAD_MI;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Fa: {
                  headGroup = NoteHead::Group::HEAD_FA;
                  break;
                  }
            case OVE::NoteHeadType::Open_Fa: {
                  headGroup = NoteHead::Group::HEAD_FA;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Sol: {
                  break;
                  }
            case OVE::NoteHeadType::Open_Sol: {
                  break;
                  }
            case OVE::NoteHeadType::Closed_La: {
                  headGroup = NoteHead::Group::HEAD_LA;
                  break;
                  }
            case OVE::NoteHeadType::Open_La: {
                  headGroup = NoteHead::Group::HEAD_LA;
                  break;
                  }
            case OVE::NoteHeadType::Closed_Ti: {
                  headGroup = NoteHead::Group::HEAD_TI;
                  break;
                  }
            case OVE::NoteHeadType::Open_Ti: {
                  headGroup = NoteHead::Group::HEAD_TI;
                  break;
                  }
            default: {
                  break;
                  }
            }

      return headGroup;
      }

void OveToMScore::convertTrackHeader(OVE::Track* track, Part* part){
      if(track == 0 || part == 0)
            return;

      QString longName = track->getName();
      if (longName != QString() && track->getShowName()){
            part->setPlainLongName(longName);
            }

      QString shortName = track->getBriefName();
      if (shortName != QString() && track->getShowBriefName()) {
            part->setPlainShortName(shortName);
            }

      part->setMidiProgram(track->getPatch());

      if (ove_->getShowTransposeTrack() && track->getTranspose() != 0 ) {
            Ms::Interval interval = part->instrument()->transpose();
            interval.diatonic = -track->getTranspose();
            part->instrument()->setTranspose(interval);
            }

      // DrumSet
      if(track->getStartClef()==OVE::ClefType::Percussion1 || track->getStartClef()==OVE::ClefType::Percussion2) {
            //use overture drumset
            Drumset* drumset = new Drumset();
            for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
                  drumset->drum(i).name     = smDrumset->drum(i).name;
                  drumset->drum(i).notehead = smDrumset->drum(i).notehead;
                  drumset->drum(i).line     = smDrumset->drum(i).line;
                  drumset->drum(i).stemDirection = smDrumset->drum(i).stemDirection;
                  drumset->drum(i).voice     = smDrumset->drum(i).voice;
                  drumset->drum(i).shortcut = 0;
                  }
            QList<OVE::Track::DrumNode> nodes = track->getDrumKit();
            for (int i=0; i<nodes.size(); ++i) {
                  int pitch = nodes[i].pitch_;
                  OVE::Track::DrumNode node = nodes[i];
                  if (pitch < DRUM_INSTRUMENTS) {
                        drumset->drum(pitch).line = node.line_;
                        drumset->drum(pitch).notehead = getHeadGroup(OVE::NoteHeadType(node.headType_));
                        drumset->drum(pitch).voice = node.voice_;
                        }
                  }

            part->instrument()->channel(0)->bank = 128;
            part->setMidiProgram(0);
            part->instrument()->setDrumset(smDrumset);
            part->instrument()->setDrumset(drumset);
            }
      }

static OttavaType OctaveShiftTypeToInt(OVE::OctaveShiftType type) {
      OttavaType subtype = OttavaType::OTTAVA_8VA;
      switch (type) {
            case OVE::OctaveShiftType::OS_8: {
                  subtype = OttavaType::OTTAVA_8VA;
                  break;
                  }
            case OVE::OctaveShiftType::OS_15: {
                  subtype = OttavaType::OTTAVA_15MA;
                  break;
                  }
            case OVE::OctaveShiftType::OS_Minus_8: {
                  subtype = OttavaType::OTTAVA_8VB;
                  break;
                  }
            case OVE::OctaveShiftType::OS_Minus_15: {
                  subtype = OttavaType::OTTAVA_15MB;
                  break;
                  }
            default:
                  break;
            }

      return subtype;
      }

void OveToMScore::convertTrackElements(int track) {
      Ottava* ottava = 0;

      for(int i=0; i<ove_->getTrackBarCount(); ++i) {
            OVE::MeasureData* measureData = ove_->getMeasureData(track, i);
            if(measureData == 0)
                  continue;

            // octave shift
            QList<OVE::MusicData*> octaves = measureData->getMusicDatas(OVE::MusicDataType::OctaveShift_EndPoint);
            for(int j=0; j<octaves.size(); ++j) {
                  OVE::OctaveShiftEndPoint* octave = static_cast<OVE::OctaveShiftEndPoint*>(octaves[j]);
                  int absTick = mtt_->getTick(i, octave->getTick());

                  if(octave->getOctaveShiftPosition() == OVE::OctaveShiftPosition::Start) {
                        if(ottava == 0) {
                              ottava = new Ottava(score_);
                              ottava->setTrack(track * VOICES);
                              ottava->setOttavaType(OctaveShiftTypeToInt(octave->getOctaveShiftType()));

                              int y_off = 0;
                              switch (octave->getOctaveShiftType()) {
                                    case OVE::OctaveShiftType::OS_8:
                                    case OVE::OctaveShiftType::OS_15: {
                                          y_off = -3;
                                          break;
                                          }
                                    case OVE::OctaveShiftType::OS_Minus_8:
                                    case OVE::OctaveShiftType::OS_Minus_15: {
                                          y_off = 8;
                                          break;
                                          }
                                    default:{
                                          break;
                                          }
                                    }

                              if(y_off != 0) {
                                    ottava->setUserOff(QPointF(0, y_off * score_->spatium()));
                                    }

                              ottava->setTick(absTick);

                              } else {
                              qDebug("overlapping octave-shift not supported");
                              delete ottava;
                              ottava = 0;
                              }
                        } else if (octave->getOctaveShiftPosition() == OVE::OctaveShiftPosition::Stop) {
                        if(ottava != 0) {
                              int absTick = mtt_->getTick(i, octave->getEndTick());

                              ottava->setTick2(absTick);
                              score_->addSpanner(ottava);
                              ottava->staff()->updateOttava();
                              ottava = 0;
                              } else {
                              qDebug("octave-shift stop without start");
                              }
                        }
                  }
            }
      }

void OveToMScore::convertLineBreak(){
      for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*> (mb);

            for (int i = 0; i < ove_->getLineCount(); ++i) {
                  OVE::Line* line = ove_->getLine(i);
                  if (measure->no() > 0) {
                        if ((int)line->getBeginBar() + (int)line->getBarCount()-1 == measure->no()) {
                              LayoutBreak* lb = new LayoutBreak(score_);
                              lb->setTrack(0);
                              lb->setLayoutBreakType(LayoutBreak::Type::LINE);
                              measure->add(lb);
                              }
                        }
                  }
            }
      }

void OveToMScore::convertSignatures(){
      int i;
      int j;
      int k;

      // Time
      const QList<MeasureToTick::TimeTick> tts = mtt_->getTimeTicks() ;
      for( i=0; i<(int)tts.size(); ++i ){
            MeasureToTick::TimeTick tt = tts[i];
            Fraction f(tt.numerator_, tt.denominator_);

            TimeSigMap* sigmap = score_->sigmap();
            sigmap->add(tt.tick_, f);

            Measure* measure  = score_->tick2measure(tt.tick_);
            if(measure){
                  for(int staffIdx = 0; staffIdx < score_->nstaves(); ++staffIdx) {
                        TimeSigType subtype = TimeSigType::NORMAL;
                        if(tt.numerator_ == 4 && tt.denominator_ == 4 && tt.isSymbol_ ){
                              subtype = TimeSigType::FOUR_FOUR;
                              } else if(tt.numerator_ == 2 && tt.denominator_ == 2 && tt.isSymbol_ ){
                              subtype = TimeSigType::ALLA_BREVE;
                              }

                        TimeSig* ts = new TimeSig(score_);
                        ts->setTrack(staffIdx * VOICES);
                        ts->setSig(Fraction(tt.numerator_, tt.denominator_), subtype);

                        Segment* seg = measure->getSegment(SegmentType::TimeSig, tt.tick_);
                        seg->add(ts);
                        }
                  }
            }

      // Key
      int staffCount = 0;
      bool createKey = false ;
      for(i=0; i<ove_->getPartCount(); ++i ){
            int partStaffCount = ove_->getStaffCount(i) ;

            for(j=0; j<partStaffCount; ++j){
                  for(k=0; k<ove_->getMeasureCount(); ++k){
                        OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k) ;

                        if( measureData != 0 ){
                              OVE::Key* keyPtr = measureData->getKey() ;

                              if( k == 0 || keyPtr->getKey() != keyPtr->getPreviousKey() )	{
                                    int tick = mtt_->getTick(k, 0);
                                    int keyValue = keyPtr->getKey();
                                    Measure* measure = score_->tick2measure(tick);
                                    if(measure){
                                          KeySigEvent ke;
                                          ke.setKey(Key(keyValue));
                                          score_->staff(staffCount+j)->setKey(tick, ke);

                                          KeySig* keysig = new KeySig(score_);
                                          keysig->setTrack((staffCount+j) * VOICES);
                                          keysig->setKeySigEvent(ke);

                                          Segment* s = measure->getSegment(SegmentType::KeySig, tick);
                                          s->add(keysig);

                                          createKey = true;
                                          }
                                    }
                              }
                        }
                  }

            staffCount += partStaffCount;
            }

      if( !createKey ){
            staffCount = 0;
            for(i=0; i<ove_->getPartCount(); ++i ){
                  int partStaffCount = ove_->getStaffCount(i) ;

                  for(j=0; j<partStaffCount; ++j){
                        Measure* measure = score_->tick2measure(mtt_->getTick(0, 0));
                        if(measure){
                              KeySig* keysig = new KeySig(score_);
                              keysig->setTrack((staffCount+j) * VOICES);
                              keysig->setKeySigEvent(KeySigEvent());

                              Segment* s = measure->getSegment(SegmentType::KeySig, 0);
                              s->add(keysig);
                              }
                        }
                  staffCount += partStaffCount;
                  }
            }

      // Clef
      staffCount = 0;
      for(i=0; i<ove_->getPartCount(); ++i){
            int partStaffCount = ove_->getStaffCount(i) ;
            for(j=0; j<partStaffCount; ++j){
                  // start clef
                  Staff* staff = score_->staff(staffCount+j);
                  if(staff){
                        OVE::Track* track = ove_->getTrack(i, j);
                        ClefType clefType = OveClefToClef(track->getStartClef());
                        Measure* measure = score_->tick2measure(0);
                        //				staff->setClef(0, clefType);

                        // note: also generate symbol for tick 0
                        // was not necessary before 0.9.6
                        Clef* clef = new Clef(score_);
                        clef->setClefType(clefType);
                        clef->setTrack((staffCount+j)*VOICES);

                        Segment* s = measure->getSegment(SegmentType::HeaderClef, 0);
                        s->add(clef);
                        }

                  // clef in measure
                  for(k=0; k<ove_->getMeasureCount(); ++k){
                        OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k);
                        QList<OVE::MusicData*> clefs = measureData->getMusicDatas(OVE::MusicDataType::Clef);
                        Measure* measure = score_->tick2measure(mtt_->getTick(k, 0));

                        for( int l=0; l<clefs.size(); ++l){
                              if(measure != 0){
                                    OVE::Clef* clefPtr = static_cast<OVE::Clef*>(clefs[l]);
                                    int absTick = mtt_->getTick(k, clefPtr->getTick());
                                    ClefType clefType = OveClefToClef(clefPtr->getClefType());

                                    Clef* clef = new Clef(score_);
                                    clef->setClefType(clefType);
                                    clef->setTrack((staffCount+j)*VOICES);

                                    Segment* s = measure->getSegment(SegmentType::Clef, absTick);
                                    s->add(clef);
                                    }
                              }
                        }
                  }

            staffCount += partStaffCount;
            }

      // Tempo
      std::map<int, double> tempos;
      for(i=0; i<ove_->getPartCount(); ++i){
            int partStaffCount = ove_->getStaffCount(i);

            for(j=0; j<partStaffCount; ++j){
                  for(k=0; k<ove_->getMeasureCount(); ++k){
                        OVE::Measure* measure = ove_->getMeasure(k);
                        OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k);
                        QList<OVE::MusicData*> tempoPtrs = measureData->getMusicDatas(OVE::MusicDataType::Tempo);

                        if(k==0 || ( k>0 && qAbs(measure->getTypeTempo()-ove_->getMeasure(k-1)->getTypeTempo())>0.01 )){
                              int tick = mtt_->getTick(k, 0);
                              tempos[tick] = measure->getTypeTempo();
                              }

                        for(int l=0; l<tempoPtrs.size(); ++l) {
                              OVE::Tempo* ptr = static_cast<OVE::Tempo*>(tempoPtrs[l]);
                              int tick = mtt_->getTick(measure->getBarNumber()->getIndex(), ptr->getTick());
                              double tempo = ptr->getQuarterTempo()>0 ? ptr->getQuarterTempo() : 1.0;

                              tempos[tick] = tempo;
                              }
                        }
                  }
            }

      std::map<int, double>::iterator it;
      int lastTempo = 0;
      for(it=tempos.begin(); it!=tempos.end(); ++it) {
            if( it==tempos.begin() || (*it).second != lastTempo ) {
                  double tpo = ((*it).second) / 60.0;
                  score_->setTempo((*it).first, tpo);
                  }

            lastTempo = (*it).second;
            }
      }

int ContainerToTick(OVE::NoteContainer* container, int quarter){
      int tick = OVE::NoteTypeToTick(container->getNoteType(), quarter);

      int dotLength = tick;
      for( int i=0; i<container->getDot(); ++i ) {
            dotLength /= 2;
            }
      dotLength = tick - dotLength;

      if(container->getTuplet() > 0) {
            tick = tick * container->getSpace() / container->getTuplet();
            }

      tick += dotLength;

      return tick;
      }

const OVE::Tuplet* getTuplet(const QList<OVE::MusicData*>& tuplets, int unit){
      for(int i=0; i<tuplets.size(); ++i){
            const OVE::MusicData* data = tuplets[i];
            if(unit >= data->start()->getOffset() && unit <= data->stop()->getOffset()){
                  const OVE::Tuplet* tuplet = static_cast<OVE::Tuplet*>(tuplets[i]);
                  return tuplet;
                  }
            }
      return 0;
      }

TDuration OveNoteType_To_Duration(OVE::NoteType noteType){
      TDuration d;
      switch(noteType){
            case OVE::NoteType::Note_DoubleWhole: {
                  d.setType(TDuration::DurationType::V_BREVE);
                  break;
                  }
            case OVE::NoteType::Note_Whole: {
                  d.setType(TDuration::DurationType::V_WHOLE);
                  break;
                  }
            case OVE::NoteType::Note_Half: {
                  d.setType(TDuration::DurationType::V_HALF);
                  break;
                  }
            case OVE::NoteType::Note_Quarter: {
                  d.setType(TDuration::DurationType::V_QUARTER);
                  break;
                  }
            case OVE::NoteType::Note_Eight: {
                  d.setType(TDuration::DurationType::V_EIGHTH);
                  break;
                  }
            case OVE::NoteType::Note_Sixteen: {
                  d.setType(TDuration::DurationType::V_16TH);
                  break;
                  }
            case OVE::NoteType::Note_32: {
                  d.setType(TDuration::DurationType::V_32ND);
                  break;
                  }
            case OVE::NoteType::Note_64: {
                  d.setType(TDuration::DurationType::V_64TH);
                  break;
                  }
            case OVE::NoteType::Note_128: {
                  d.setType(TDuration::DurationType::V_128TH);
                  break;
                  }
            case OVE::NoteType::Note_256: {
                  d.setType(TDuration::DurationType::V_256TH);
                  break;
                  }
            default:
                  d.setType(TDuration::DurationType::V_QUARTER);
                  break;
            }

      return d;
      }

int accidentalToAlter(OVE::AccidentalType type) {
      int alter = 0;

      switch( type ) {
            case OVE::AccidentalType::Normal:
            case OVE::AccidentalType::Natural:
            case OVE::AccidentalType::Natural_Caution: {
                  alter = 0;
                  break;
                  }
            case OVE::AccidentalType::Sharp:
            case OVE::AccidentalType::Sharp_Caution: {
                  alter = 1;
                  break;
                  }
            case OVE::AccidentalType::Flat:
            case OVE::AccidentalType::Flat_Caution: {
                  alter = -1;
                  break;
                  }
            case OVE::AccidentalType::DoubleSharp:
            case OVE::AccidentalType::DoubleSharp_Caution: {
                  alter = 2;
                  break;
                  }
            case OVE::AccidentalType::DoubleFlat:
            case OVE::AccidentalType::DoubleFlat_Caution: {
                  alter = -2;
                  break;
                  }
            default:
                  break;
            }

      return alter;
      }

void getMiddleToneOctave(OVE::ClefType clef, OVE::ToneType& tone, int& octave) {
      tone = OVE::ToneType::B;
      octave = 4;

      switch ( clef ) {
            case OVE::ClefType::Treble: {
                  tone = OVE::ToneType::B;
                  octave = 4;
                  break;
                  }
            case OVE::ClefType::Treble8va: {
                  tone = OVE::ToneType::B;
                  octave = 5;
                  break;
                  }
            case OVE::ClefType::Treble8vb: {
                  tone = OVE::ToneType::B;
                  octave = 3;
                  break;
                  }
            case OVE::ClefType::Bass: {
                  tone = OVE::ToneType::D;
                  octave = 3;
                  break;
                  }
            case OVE::ClefType::Bass8va: {
                  tone = OVE::ToneType::D;
                  octave = 4;
                  break;
                  }
            case OVE::ClefType::Bass8vb: {
                  tone = OVE::ToneType::D;
                  octave = 2;
                  break;
                  }
            case OVE::ClefType::Alto: {
                  tone = OVE::ToneType::C;
                  octave = 4;
                  break;
                  }
            case OVE::ClefType::UpAlto: {
                  tone = OVE::ToneType::A;
                  octave = 3;
                  break;
                  }
            case OVE::ClefType::DownDownAlto: {
                  tone = OVE::ToneType::G;
                  octave = 4;
                  break;
                  }
            case OVE::ClefType::DownAlto: {
                  tone = OVE::ToneType::E;
                  octave = 4;
                  break;
                  }
            case OVE::ClefType::UpUpAlto: {
                  tone = OVE::ToneType::F;
                  octave = 3;
                  break;
                  }
            case OVE::ClefType::Percussion1: {
                  tone = OVE::ToneType::A;
                  octave = 3;
                  break;
                  }
            case OVE::ClefType::Percussion2: {
                  tone = OVE::ToneType::A;
                  octave = 3;
                  break;
                  }
            case OVE::ClefType::TAB: {
                  break;
                  }
            default:
                  break;
            }
      }

OVE::ClefType getClefType(OVE::MeasureData* measure, int tick) {
      OVE::ClefType type = measure->getClef()->getClefType();
      QList<OVE::MusicData*> clefs = measure->getMusicDatas(OVE::MusicDataType::Clef);

      for(int i=0; i<clefs.size(); ++i){
            if(tick < clefs[i]->getTick()){
                  break;
                  }
            if(tick >= clefs[i]->getTick()){
                  OVE::Clef* clef = static_cast<OVE::Clef*>(clefs[i]);
                  type = clef->getClefType();
                  }
            }

      return type;
      }

void OveToMScore::convertMeasures() {
      for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*>(mb);
            int tick = measure->tick();
            measure->setLen(score_->sigmap()->timesig(tick).timesig());
            measure->setTimesig(score_->sigmap()->timesig(tick).timesig()); //?
            convertMeasure(measure);
            }

      //  convert based on notes
      for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            Measure* measure = static_cast<Measure*>(mb);

            convertLines(measure);
            }
      }

void OveToMScore::convertMeasure(Measure* measure){
      int staffCount = 0;
      int measureCount = ove_->getMeasureCount();

      for (int i=0; i < ove_->getPartCount(); ++i) {
            int partStaffCount = ove_->getStaffCount(i);

            for (int j=0; j < partStaffCount; ++j) {
                  int measureID = measure->no();

                  if (measureID >= 0 && measureID < measureCount) {
                        int trackIndex = (staffCount + j) * VOICES;

                        convertMeasureMisc(measure, i, j, trackIndex);
                        convertNotes(measure, i, j, trackIndex);
                        convertLyrics(measure, i, j, trackIndex);
                        convertHarmonys(measure, i, j, trackIndex);
                        convertRepeats(measure, i, j, trackIndex);
                        convertDynamics(measure, i, j, trackIndex);
                        convertExpressions(measure, i, j, trackIndex);
                        }
                  }

            staffCount += partStaffCount;
            }
      }

void OveToMScore::convertLines(Measure* measure){
      int staffCount = 0;
      int measureCount = ove_->getMeasureCount();

      for( int i=0; i<ove_->getPartCount(); ++i ){
            int partStaffCount = ove_->getStaffCount(i);

            for( int j=0; j<partStaffCount; ++j ){
                  int measureID = measure->no();

                  if (measureID >= 0 && measureID < measureCount) {
                        int trackIndex = (staffCount + j) * VOICES;

                        convertSlurs(measure, i, j, trackIndex);
                        convertGlissandos(measure, i, j, trackIndex);
                        convertWedges(measure, i, j, trackIndex);
                        }
                  }

            staffCount += partStaffCount;
            }
      }

void OveToMScore::convertMeasureMisc(Measure* measure, int part, int staff, int track){
      OVE::Measure* measurePtr = ove_->getMeasure(measure->no());
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measurePtr == 0 || measureData == 0)
            return;

      // pickup
      if(measurePtr->getIsPickup()){
            measure->setIrregular(true);
            }

      // multiple measure rest
      if(measurePtr->getIsMultiMeasureRest()){
            measure->setBreakMultiMeasureRest(true);
            }

      // barline
      BarLineType bartype = BarLineType::NORMAL;

      switch(measurePtr->getRightBarline()) {
            case OVE::BarLineType::Default:{
                  bartype = BarLineType::NORMAL;
                  break;
                  }
            case OVE::BarLineType::Double:{
                  bartype = BarLineType::DOUBLE;
                  break;
                  }
            case OVE::BarLineType::Final:{
                  bartype = BarLineType::END;
                  break;
                  }
            case OVE::BarLineType::Null:{
                  bartype = BarLineType::NORMAL;
                  break;
                  }
            case OVE::BarLineType::RepeatLeft:{
                  bartype = BarLineType::START_REPEAT;
                  measure->setRepeatStart(true);
                  break;
                  }
            case OVE::BarLineType::RepeatRight:{
                  bartype = BarLineType::END_REPEAT;
                  measure->setRepeatEnd(true);
                  break;
                  }
            case OVE::BarLineType::Dashed:{
                  bartype = BarLineType::BROKEN;
                  break;
                  }
            default:
                  break;
            }

//TODO      if (bartype != BarLineType::NORMAL && bartype != BarLineType::END_REPEAT && bartype != BarLineType::START_REPEAT && bartype != BarLineType::END_START_REPEAT && bartype != BarLineType::END)
      if (bartype != BarLineType::NORMAL && bartype != BarLineType::END_REPEAT && bartype != BarLineType::START_REPEAT && bartype != BarLineType::END)
            measure->setEndBarLineType(bartype, 0);

      if (bartype == BarLineType::END_REPEAT)
            measure->setRepeatEnd(true);

      if(measurePtr->getLeftBarline() == OVE::BarLineType::RepeatLeft){
            //bartype = BarLineType::START_REPEAT;
            measure->setRepeatStart(true);
            }

      // rehearsal
      int i;
      QList<OVE::MusicData*> texts = measureData->getMusicDatas(OVE::MusicDataType::Text);
      for(i=0; i<texts.size(); ++i){
            OVE::Text* textPtr = static_cast<OVE::Text*>(texts[i]);
            if(textPtr->getTextType() == OVE::Text::Type::Rehearsal){
                  RehearsalMark* text = new RehearsalMark(score_);
                  text->setPlainText(textPtr->getText());
//TODO:ws                  text->setAbove(true);
                  text->setTrack(track);

                  Segment* s = measure->getSegment(SegmentType::ChordRest, mtt_->getTick(measure->no(), 0));
                  s->add(text);
                  }
            }

      // tempo
      QList<OVE::MusicData*> tempos = measureData->getMusicDatas(OVE::MusicDataType::Tempo);
      for(i=0; i<tempos.size(); ++i){
            OVE::Tempo* tempoPtr = static_cast<OVE::Tempo*>(tempos[i]);
            TempoText* t = new TempoText(score_);
            int absTick = mtt_->getTick(measure->no(), tempoPtr->getTick());
            double tpo = (tempoPtr->getQuarterTempo())/60.0;

            score_->setTempo(absTick, tpo);

            t->setTempo(tpo);
            QString durationTempoL;
            QString durationTempoR;
            if ((int)(tempoPtr->getLeftNoteType()))
                  durationTempoL = TempoText::duration2tempoTextString(OveNoteType_To_Duration(tempoPtr->getLeftNoteType()));
            if ((int)(tempoPtr->getRightNoteType()))
                  durationTempoR = TempoText::duration2tempoTextString(OveNoteType_To_Duration(tempoPtr->getRightNoteType()));
            QString textTempo;
            if (tempoPtr->getShowBeforeText())
                  textTempo += (tempoPtr->getLeftText()).toHtmlEscaped();
            if (tempoPtr->getShowMark()) {
                  if (!textTempo.isEmpty())
                        textTempo += " ";
                  if (tempoPtr->getShowParenthesis())
                        textTempo += "(";
                  textTempo += durationTempoL;
                  if (tempoPtr->getLeftNoteDot())
                        textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                  textTempo += " = ";
                  switch (tempoPtr->getRightSideType()) {
                        case 1:
                              textTempo += durationTempoR;
                              if (tempoPtr->getRightNoteDot())
                                    textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                              break;
                        case 2:
                              textTempo += (tempoPtr->getRightText()).toHtmlEscaped();
                              break;
                        case 3:
                              textTempo += QString::number(qFloor(tempoPtr->getTypeTempo()));
                              break;
                        case 0:
                        default:
                              textTempo += QString::number(tempoPtr->getTypeTempo());
                              break;
                        }
                  if (tempoPtr->getShowParenthesis())
                        textTempo += ")";
                  }
            if (textTempo.isEmpty()) {
                  textTempo = durationTempoL;
                  if (tempoPtr->getLeftNoteDot())
                        textTempo += "<sym>space</sym><sym>metAugmentationDot</sym>";
                  textTempo += " = " + QString::number(tempoPtr->getTypeTempo());
                  t->setVisible(false);
                  }
            t->setXmlText(textTempo);
//TODO:ws            t->setAbove(true);
            t->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest, absTick);
            s->add(t);
            }
      }

// beam in grace
int getGraceLevel(const QList<OVE::NoteContainer*>& containers, int tick, int unit){
      int graceCount = 0;
      int level = 0; // normal chord rest

      for(int i=0; i<containers.size(); ++i){
            OVE::NoteContainer* container = containers[i];
            if(container->getTick() > tick)
                  break;

            if(container->getIsGrace() && container->getTick() == tick){
                  ++graceCount;

                  if(unit <= container->start()->getOffset()){
                        ++level;
                        }
                  }
            }

      return level;
      }

bool isRestDefaultLine(OVE::Note* rest, OVE::NoteType noteType) {
      bool isDefault = true;
      switch (noteType) {
            case OVE::NoteType::Note_DoubleWhole:
            case OVE::NoteType::Note_Whole:
            case OVE::NoteType::Note_Half:
            case OVE::NoteType::Note_Quarter: {
                  if(rest->getLine() != 0)
                        isDefault = false;
                  break;
                  }
            case OVE::NoteType::Note_Eight: {
                  if(rest->getLine() != 1)
                        isDefault = false;
                  break;
                  }
            case OVE::NoteType::Note_Sixteen:
            case OVE::NoteType::Note_32: {
                  if(rest->getLine() != -1)
                        isDefault = false;
                  break;
                  }
            case OVE::NoteType::Note_64: {
                  if(rest->getLine() != -3)
                        isDefault = false;
                  break;
                  }
            case OVE::NoteType::Note_128: {
                  if(rest->getLine() != -4)
                        isDefault = false;
                  break;
                  }
            default: {
                  break;
                  }
            }

      return isDefault;
      }

Drumset* getDrumset(Score* score, int part) {
      Part* p = score->parts().at(part);
      return const_cast<Drumset*>(p->instrument()->drumset());   //TODO: remove cast
      }

void OveToMScore::convertNotes(Measure* measure, int part, int staff, int track){
      int j;
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      QList<OVE::NoteContainer*> containers = measureData->getNoteContainers();
      QList<OVE::MusicData*> tuplets = measureData->getCrossMeasureElements(OVE::MusicDataType::Tuplet, OVE::MeasureData::PairType::Start);
      QList<OVE::MusicData*> beams = measureData->getCrossMeasureElements(OVE::MusicDataType::Beam, OVE::MeasureData::PairType::Start);
      Tuplet* tuplet = 0;
      ChordRest* cr = 0;
      int partStaffCount = ove_->getStaffCount(part);

      if(containers.empty()){
            int absTick = mtt_->getTick(measure->no(), 0);

            cr = new Rest(score_);
            cr->setDuration(measure->len());
            cr->setDurationType(TDuration::DurationType::V_MEASURE);
            cr->setTrack(track);
            Segment* s = measure->getSegment(SegmentType::ChordRest, absTick);
            s->add(cr);
            }
      QList<Ms::Chord*> graceNotes;
      for (int i = 0; i < containers.size(); ++i) {
            OVE::NoteContainer* container = containers[i];
            int tick = mtt_->getTick(measure->no(), container->getTick());
            int noteTrack = track + container->getVoice();

            if (container->getIsRest()) {
                  TDuration duration = OveNoteType_To_Duration(container->getNoteType());
                  duration.setDots(container->getDot());

                  cr = new Rest(score_);
                  cr->setDuration(duration.fraction());
                  cr->setDurationType(duration);
                  cr->setTrack(noteTrack);
                  cr->setVisible(container->getShow());
                  Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
                  s->add(cr);

                  QList<OVE::Note*> notes = container->getNotesRests();
                  for (j = 0; j < notes.size(); ++j) {
                        OVE::Note* notePtr = notes[j];
                        if(!isRestDefaultLine(notePtr, container->getNoteType()) && notePtr->getLine() != 0) {
                              double yOffset = -(double)(notePtr->getLine());
                              int stepOffset = cr->staff()->staffType(cr->tick())->stepOffset();
                              int lineOffset = static_cast<Ms::Rest*>(cr)->computeLineOffset(5);
                              yOffset -= qreal(lineOffset + stepOffset);
                              yOffset *= score_->spatium()/2.0;
                              cr->setUserYoffset(yOffset);
                              cr->setAutoplace(false);
                              }
                        }
                  }
            else {
                  QList<OVE::Note*> notes = container->getNotesRests();

                  cr = measure->findChord(tick, noteTrack);
                  if (cr == 0) {
                        cr = new Ms::Chord(score_);
                        cr->setTrack(noteTrack);

                        // grace
                        if (container->getIsGrace()) {
                              TDuration duration = OveNoteType_To_Duration(container->getGraceNoteType());
                              duration.setDots(container->getDot());
                              ((Ms::Chord*) cr)->setNoteType(NoteType::APPOGGIATURA);

                              if (duration.type() == TDuration::DurationType::V_QUARTER) {
                                    ((Ms::Chord*) cr)->setNoteType(NoteType::GRACE4);
                                    cr->setDurationType(TDuration::DurationType::V_QUARTER);
                                    } else if (duration.type() == TDuration::DurationType::V_16TH) {
                                    ((Ms::Chord*) cr)->setNoteType(NoteType::GRACE16);
                                    cr->setDurationType(TDuration::DurationType::V_16TH);
                                    } else if (duration.type() == TDuration::DurationType::V_32ND) {
                                    ((Ms::Chord*) cr)->setNoteType(NoteType::GRACE32);
                                    cr->setDurationType(TDuration::DurationType::V_32ND);
                                    } else {
                                    cr->setDurationType(TDuration::DurationType::V_EIGHTH);
                                    }

                              // st = SegmentType::Grace;
                              }
                        else {
                              TDuration duration = OveNoteType_To_Duration(container->getNoteType());
                              duration.setDots(container->getDot());

                              if (duration.type() == TDuration::DurationType::V_INVALID)
                                    duration.setType(TDuration::DurationType::V_QUARTER);
                              cr->setDurationType(duration);
                              // append grace notes before
                              int ii = -1;
                              for (ii = graceNotes.size() - 1; ii >= 0; ii--) {
                                    Ms::Chord* gc = graceNotes[ii];
                                    if(gc->voice() == cr->voice()){
                                          cr->add(gc);
                                          }
                                    }
                              graceNotes.clear();
                              }
                        cr->setDuration(cr->durationType().fraction());

                        if(!container->getIsGrace()) {
                              Segment* s = measure->getSegment(SegmentType::ChordRest, tick);
                              s->add(cr);
                              }
                        else {
                              graceNotes.append(static_cast<Ms::Chord*>(cr));
                              }
                        }

                  cr->setVisible(container->getShow());
                  cr->setSmall(container->getIsCue());
                  for (j = 0; j < notes.size(); ++j) {
                        OVE::Note* oveNote = notes[j];
                        Note* note = new Note(score_);
                        int pitch = oveNote->getNote();

                        //note->setTrack(noteTrack);
                        note->setVeloType(Note::ValueType::USER_VAL);
                        note->setVeloOffset(oveNote->getOnVelocity());
                        note->setPitch(pitch);

                        // tpc
                        bool setDirection = false;
                        OVE::ClefType clefType = getClefType(measureData, container->getTick());
                        if(clefType == OVE::ClefType::Percussion1 || clefType == OVE::ClefType::Percussion2) {
                              Drumset* drumset = getDrumset(score_, part);
                              if(drumset != 0) {
                                    if (!drumset->isValid(pitch) || pitch == -1) {
                                          qDebug("unmapped drum note 0x%02x %d", note->pitch(), note->pitch());
                                          }
                                    else {
                                          note->setHeadGroup(drumset->noteHead(pitch));
                                          int line = drumset->line(pitch);
                                          note->setLine(line);
                                          note->setTpcFromPitch();
                                          ((Ms::Chord*) cr)->setStemDirection(drumset->stemDirection(pitch));
                                          setDirection = true;
                                          }
                                    }
                              else {
                              	// no drumset, we don't allow mid staff percussion
                                    note->setTpc(14);
                              	}
                              }
                        else {
                              const int OCTAVE = 7;
                              OVE::ToneType clefMiddleTone;
                              int clefMiddleOctave;
                              getMiddleToneOctave(clefType, clefMiddleTone, clefMiddleOctave);
                              int absLine = (int) clefMiddleTone + clefMiddleOctave * OCTAVE + oveNote->getLine();
                              if ((partStaffCount == 2) && oveNote->getOffsetStaff())
                                    absLine += 2 * (oveNote->getOffsetStaff());
                              int tone = absLine % OCTAVE;
                              int alter = accidentalToAlter(oveNote->getAccidental());
                              NoteVal nv(pitch);
                              note->setTrack(cr->track());
                              note->setNval(nv, tick);
                              // note->setTpcFromPitch();
                              note->setTpc(step2tpc(tone, AccidentalVal(alter)));
                              if (oveNote->getShowAccidental()) {
                                    Ms::Accidental* a = new Accidental(score_);
                                    bool bracket = (int)(oveNote->getAccidental()) & 0x8;
                                    AccidentalType at = Ms::AccidentalType::NONE;
                                    switch(alter) {
                                          case 0: at = Ms::AccidentalType::NATURAL; break;
                                          case 1: at = Ms::AccidentalType::SHARP; break;
                                          case -1: at = Ms::AccidentalType::FLAT; break;
                                          case 2: at = Ms::AccidentalType::SHARP2; break;
                                          case -2: at = Ms::AccidentalType::FLAT2; break;
                                          }
                                    a->setAccidentalType(at);
                                    a->setBracket(AccidentalBracket(bracket));
                                    a->setRole(Ms::AccidentalRole::USER);
                                    note->add(a);
                                    }
                              note->setHeadGroup(getHeadGroup(oveNote->getHeadType()));
                              }
                        if ((oveNote->getHeadType() == OVE::NoteHeadType::Invisible) || !(oveNote->getShow()))
                              note->setVisible(false);
                        // tie
                        if ((int(oveNote->getTiePos()) & int(OVE::TiePos::LeftEnd)) == int(OVE::TiePos::LeftEnd)) {
                              Tie* tie = new Tie(score_);
                              note->setTieFor(tie);
                              tie->setStartNote(note);
                              tie->setTrack(noteTrack);
                              }

                        // pitch must be set before adding note to chord as note
                        // is inserted into pitch sorted list (ws)
                        cr->add(note);

                        //cr->setVisible(oveNote->getShow());
                        ((Ms::Chord*) cr)->setNoStem(int(container->getNoteType()) <= int(OVE::NoteType::Note_Whole));
                        if(!setDirection)
                              ((Ms::Chord*) cr)->setStemDirection(container->getStemUp() ? Direction::UP : Direction::DOWN);

                        // cross staff
                        int staffMove = 0;
                        if(partStaffCount == 2) { /*treble-bass*/
                              staffMove = oveNote->getOffsetStaff();
                              }
                        cr->setStaffMove(staffMove);
                        }
                  }

            // beam
            //Beam::Mode bm = container->getIsRest() ? Beam::Mode::NONE : Beam::Mode::AUTO;
            Beam::Mode bm = Beam::Mode::NONE;
            if(container->getInBeam()){
                  OVE::MeasurePos pos = container->start()->shiftMeasure(0);
                  OVE::MusicData* data = getCrossMeasureElementByPos(part, staff, pos, container->getVoice(), OVE::MusicDataType::Beam);

                  if(data != 0){
                        OVE::Beam* beam = static_cast<OVE::Beam*>(data);
                        OVE::MeasurePos startPos = beam->start()->shiftMeasure(0);
                        OVE::MeasurePos stopPos = beam->stop()->shiftMeasure(beam->start()->getMeasure());

                        if(startPos == pos){
                              bm = Beam::Mode::BEGIN;
                              }
                        else if(stopPos == pos){
                              bm = Beam::Mode::END;
                              }
                        else{
                              bm = Beam::Mode::MID;
                              }
                        }
                  }
            cr->setBeamMode(bm);

            // tuplet
            if (container->getTuplet() > 0) {
                  if (tuplet == 0) {
                        bool create = true;

                        // check tuplet start
                        if(container->getNoteType() < OVE::NoteType::Note_Eight) {
                              const OVE::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->getOffset());
                              if(oveTuplet == 0) {
                                    create = false;
                                    }
                              }

                        if(create) {
                              tuplet = new Tuplet(score_);
                              tuplet->setTrack(noteTrack);
                              tuplet->setRatio(Fraction(container->getTuplet(), container->getSpace()));
                              TDuration duration = OveNoteType_To_Duration(container->getNoteType());
                              tuplet->setBaseLen(duration);
                              tuplet->setTick(tick);
                              tuplet->setParent(measure);
                              //measure->add(tuplet);
                              }
                        }

                  if (tuplet != 0) {
                        cr->setTuplet(tuplet);
                        tuplet->add(cr);
                        }

                  if (tuplet != 0) {
                        // check tuplet end
                        const OVE::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->getOffset());
                        if (oveTuplet != 0) {
                              //set direction
                              tuplet->setDirection(oveTuplet->getLeftShoulder()->getYOffset() < 0 ? Direction::UP : Direction::DOWN);

                              if(container->start()->getOffset() == oveTuplet->stop()->getOffset()){
                                    tuplet = 0;
                                    }
                              }
                        }
                  }

            // articulation
            QList<OVE::Articulation*> articulations = container->getArticulations();
            for (j = 0; j < articulations.size(); ++j) {
                  convertArticulation(measure, cr, noteTrack, tick, articulations[j]);
                  }
            }
      }

void OveToMScore::convertArticulation(
            Measure* measure, ChordRest* cr,
            int track, int absTick, OVE::Articulation* art){

      switch ( art->getArtType() ) {
            case OVE::ArticulationType::Major_Trill :
            case OVE::ArticulationType::Minor_Trill :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::ornamentTrill);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Trill_Section :{
                  break;
                  }
            case OVE::ArticulationType::Inverted_Short_Mordent :
            case OVE::ArticulationType::Inverted_Long_Mordent :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::ornamentMordent);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Short_Mordent :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::ornamentMordentInverted);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Turn :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::ornamentTurn);
                  cr->add(a);
                  break;
                  }
                  //	case OVE::ArticulationType::Flat_Accidental_For_Trill :
                  //	case OVE::ArticulationType::Sharp_Accidental_For_Trill :
                  //	case OVE::ArticulationType::Natural_Accidental_For_Trill :
            case OVE::ArticulationType::Tremolo_Eighth :{
                  Tremolo* t = new Tremolo(score_);
                  t->setTremoloType(TremoloType::R8);
                  cr->add(t);
                  break;
                  }
            case OVE::ArticulationType::Tremolo_Sixteenth :{
                  Tremolo* t = new Tremolo(score_);
                  t->setTremoloType(TremoloType::R16);
                  cr->add(t);
                  break;
                  }
            case OVE::ArticulationType::Tremolo_Thirty_Second :{
                  Tremolo* t = new Tremolo(score_);
                  t->setTremoloType(TremoloType::R32);
                  cr->add(t);
                  break;
                  }
            case OVE::ArticulationType::Tremolo_Sixty_Fourth :{
                  Tremolo* t = new Tremolo(score_);
                  t->setTremoloType(TremoloType::R64);
                  cr->add(t);
                  break;
                  }
            case OVE::ArticulationType::Marcato :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::articAccentAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Marcato_Dot :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::articAccentAbove);
                  cr->add(a);

                  a = new Articulation(score_);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Heavy_Attack :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::articAccentAbove);
                  cr->add(a);

                  a = new Articulation(score_);
                  a->setSymId(SymId::articTenutoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::SForzando :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(true);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::SForzando_Inverted :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(false);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::SForzando_Dot :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(true);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);

                  a = new Articulation(score_);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::SForzando_Dot_Inverted :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(false);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);

                  a = new Articulation(score_);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Heavier_Attack :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(true);
                  a->setSymId(SymId::articMarcatoAbove);
                  cr->add(a);

                  a = new Articulation(score_);
                  a->setSymId(SymId::articTenutoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Staccatissimo :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(true);
                  a->setSymId(SymId::articStaccatissimoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Staccato :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::articStaccatoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Tenuto :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::articTenutoAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Pause :{
                  Breath* b = new Breath(score_);
                  b->setTrack(track);
                  Segment* seg = measure->getSegment(SegmentType::Breath, absTick + (cr ? cr->actualTicks() : 0));
                  seg->add(b);
                  break;
                  }
            case OVE::ArticulationType::Grand_Pause :{
                  // TODO?
                  break;
                  }
            case OVE::ArticulationType::Up_Bow :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::stringsUpBow);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Down_Bow :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::stringsDownBow);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Up_Bow_Inverted :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::stringsUpBow);
                  a->setUserYoffset(5.3);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Down_Bow_Inverted :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::stringsDownBow);
                  a->setUserYoffset(5.3);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Natural_Harmonic :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::stringsHarmonic);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Artificial_Harmonic :{
                  break;
                  }
            case OVE::ArticulationType::Finger_1 :
            case OVE::ArticulationType::Finger_2 :
            case OVE::ArticulationType::Finger_3 :
            case OVE::ArticulationType::Finger_4 :
            case OVE::ArticulationType::Finger_5 :{
                  break;
                  }
            case OVE::ArticulationType::Plus_Sign :{
                  Articulation* a = new Articulation(score_);
                  a->setSymId(SymId::brassMuteClosed);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Arpeggio :{
                  // there can be only one
                  if (!(static_cast<Ms::Chord*>(cr))->arpeggio()) {
                        Arpeggio* a = new Arpeggio(score_);
                        a->setArpeggioType(ArpeggioType::NORMAL);
                        /*      	if (art->getPlacementAbove()){
            a->setSubtype(ArpeggioType::UP);
         }else {
            a->setSubtype(ARpeggioType::DOWN);
         }*/
                        cr->add(a);
                        }

                  break;
                  }
            case OVE::ArticulationType::Fermata :{
                  Articulation* a = new Articulation(score_);
                  a->setUp(true);
                  a->setSymId(SymId::fermataAbove);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Fermata_Inverted :{
                  Articulation* a = new Articulation(score_);
                  a->setDirection(Direction::DOWN);
                  a->setSymId(SymId::fermataBelow);
                  cr->add(a);
                  break;
                  }
            case OVE::ArticulationType::Pedal_Down :{
                  if (pedal_) {
                        delete pedal_;
                        pedal_ = 0;
                        }
                  else {
                        pedal_ = new Pedal(score_);
                        pedal_->setTrack(track);
                        Segment* seg = measure->getSegment(SegmentType::ChordRest, absTick);
                        pedal_->setTick(seg->tick());
                        score_->addSpanner(pedal_);
                        }
                  break;
                  }
            case OVE::ArticulationType::Pedal_Up :{
                  if(pedal_){
                        Segment* seg = measure->getSegment(SegmentType::ChordRest, absTick);
                        pedal_->setTick2(seg->tick());
                        pedal_ = 0;
                        }
                  break;
                  }
                  //	case OVE::ArticulationType::Toe_Pedal :
                  //	case OVE::ArticulationType::Heel_Pedal :
                  //	case OVE::ArticulationType::Toe_To_Heel_Pedal :
                  //	case OVE::ArticulationType::Heel_To_Toe_Pedal :
                  //	case OVE::ArticulationType::Open_String :
            default:
                  break;
            }
      }

void OveToMScore::convertLyrics(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> lyrics = measureData->getMusicDatas(OVE::MusicDataType::Lyric);

      for(int i=0; i<lyrics.size(); ++i){
            OVE::Lyric* oveLyric = static_cast<OVE::Lyric*>(lyrics[i]);
            int tick = mtt_->getTick(measure->no(), oveLyric->getTick());

            Lyrics* lyric = new Lyrics(score_);
            lyric->setNo(oveLyric->getVerse());
            lyric->setPlainText(oveLyric->getLyric());
            lyric->setTrack(track);
            Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
            if (segment->element(track))
                  static_cast<ChordRest*>(segment->element(track))->add(lyric);
            }
      }

void OveToMScore::convertHarmonys(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> harmonys = measureData->getMusicDatas(OVE::MusicDataType::Harmony);

      for(int i=0; i<harmonys.size(); ++i){
            OVE::Harmony* harmonyPtr = static_cast<OVE::Harmony*>(harmonys[i]);
            int absTick = mtt_->getTick(measure->no(), harmonyPtr->getTick());

            Harmony* harmony = new Harmony(score_);

            // TODO - does this need to be key-aware?
            harmony->setTrack(track);
            harmony->setRootTpc(step2tpc(harmonyPtr->getRoot(), AccidentalVal(harmonyPtr->getAlterRoot())));
            if(harmonyPtr->getBass() != OVE::INVALID_NOTE && (harmonyPtr->getBass() != harmonyPtr->getRoot() || (harmonyPtr->getBass() == harmonyPtr->getRoot() && harmonyPtr->getAlterBass() != harmonyPtr->getAlterRoot()))){
                  harmony->setBaseTpc(step2tpc(harmonyPtr->getBass(), AccidentalVal(harmonyPtr->getAlterBass())));
                  }
            const ChordDescription* d = harmony->fromXml(harmonyPtr->getHarmonyType());
            if(d != 0){
                  harmony->setId(d->id);
                  harmony->setTextName(d->names.front());
                  }
            else {
                  harmony->setId(-1);
                  harmony->setTextName(harmonyPtr->getHarmonyType());
                  }
            harmony->render();

            Segment* s = measure->getSegment(SegmentType::ChordRest, absTick);
            s->add(harmony);
            }
      }

/*OVE::MusicData* OveToMScore::getMusicDataByUnit(int part, int staff, int measure, int unit, OVE::MusicDataType type){
   OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure);
   if(measureData != 0) {
      const QList<OVE::MusicData*>& datas = measureData->getMusicDatas(type);
      for(unsigned int i=0; i<datas.size(); ++i){
         if(datas[i]->getTick() == unit){//different measurement
            return datas[i];
         }
      }
   }

   return 0;
}*/

OVE::MusicData* OveToMScore::getCrossMeasureElementByPos(int part, int staff, const OVE::MeasurePos& pos, int voice, OVE::MusicDataType type){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, pos.getMeasure());
      if(measureData != 0) {
            const QList<OVE::MusicData*>& datas = measureData->getCrossMeasureElements(type, OVE::MeasureData::PairType::All);
            for(int i=0; i<datas.size(); ++i){
                  OVE::MeasurePos dataStart = datas[i]->start()->shiftMeasure(0);
                  OVE::MeasurePos dataStop = datas[i]->stop()->shiftMeasure(datas[i]->start()->getMeasure());

                  if(dataStart <= pos && dataStop >= pos && (int)datas[i]->getVoice() == voice){
                        return datas[i];
                        }
                  }
            }

      return 0;
      }

OVE::NoteContainer* OveToMScore::getContainerByPos(int part, int staff, const OVE::MeasurePos& pos){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, pos.getMeasure());
      if(measureData != 0) {
            const QList<OVE::NoteContainer*>& containers = measureData->getNoteContainers();
            for(int i=0; i<containers.size(); ++i){
                  if(pos == containers[i]->start()->shiftMeasure(0)){
                        return containers[i];
                        }
                  }
            }

      return 0;
      }

void OveToMScore::convertRepeats(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      int i;
      QList<OVE::MusicData*> repeats = measureData->getMusicDatas(OVE::MusicDataType::Repeat);

      for(i=0; i<repeats.size(); ++i){
            OVE::RepeatSymbol* repeatPtr = static_cast<OVE::RepeatSymbol*>(repeats[i]);
            OVE::RepeatType type = repeatPtr->getRepeatType();
            Element* e = 0;

            switch(type) {
                  case OVE::RepeatType::Segno:{
                        Marker* marker = new Marker(score_);
                        marker->setMarkerType(Marker::Type::SEGNO);
                        e = marker;
                        break;
                        }
                  case OVE::RepeatType::Coda:{
                        Marker* marker = new Marker(score_);
                        marker->setMarkerType(Marker::Type::CODA);
                        e = marker;
                        break;
                        }
                  case OVE::RepeatType::DSAlCoda:{
                        Jump* jp = new Jump(score_);
                        jp->setJumpType(Jump::Type::DS_AL_CODA);
                        e = jp;
                        break;
                        }
                  case OVE::RepeatType::DSAlFine:{
                        Jump* jp = new Jump(score_);
                        jp->setJumpType(Jump::Type::DS_AL_FINE);
                        e = jp;
                        break;
                        }
                  case OVE::RepeatType::DCAlCoda:{
                        Jump* jp = new Jump(score_);
                        jp->setJumpType(Jump::Type::DC_AL_CODA);
                        e = jp;
                        break;
                        }
                  case OVE::RepeatType::DCAlFine:{
                        Jump* jp = new Jump(score_);
                        jp->setJumpType(Jump::Type::DC_AL_FINE);
                        e = jp;
                        break;
                        }
                  case OVE::RepeatType::ToCoda:{
                        Marker* m = new Marker(score_);
                        m->setMarkerType(Marker::Type::TOCODA);
                        e = m;
                        break;
                        }
                  case OVE::RepeatType::Fine:{
                        Marker* m = new Marker(score_);
                        m->setMarkerType(Marker::Type::FINE);
                        e = m;
                        break;
                        }
                  default:
                        break;
                  }

            if(e != 0){
                  e->setTrack(track);
                  measure->add(e);
                  }
            }

      QList<OVE::MusicData*> endings = measureData->getCrossMeasureElements(
                        OVE::MusicDataType::Numeric_Ending,
                        OVE::MeasureData::PairType::Start);

      for(i=0; i<endings.size(); ++i){
            OVE::NumericEnding* ending = static_cast<OVE::NumericEnding*>(endings[i]);
            int absTick1 = mtt_->getTick(measure->no(), 0);
            int absTick2 = mtt_->getTick(measure->no() + ending->stop()->getMeasure(), 0);

            if (absTick1 < absTick2) {
                  Volta* volta = new Volta(score_);
                  volta->setTrack(track);
                  volta->setTick(absTick1);
                  volta->setTick2(absTick2);
                  score_->addElement(volta);
                  volta->setVoltaType(Volta::Type::CLOSED);
                  volta->setText(ending->getText());

                  volta->endings().clear();
                  QList<int> numbers = ending->getNumbers();
                  for (int j = 0; j < numbers.size(); ++j) {
                        volta->endings().append(numbers[j]);
                        }
                  }
            }
      }

void OveToMScore::convertSlurs(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> slurs = measureData->getCrossMeasureElements(OVE::MusicDataType::Slur, OVE::MeasureData::PairType::Start);

      for(int i=0; i<slurs.size(); ++i){
            OVE::Slur* slurPtr = static_cast<OVE::Slur*>(slurs[i]);

            OVE::NoteContainer* startContainer = getContainerByPos(part, staff, slurPtr->start()->shiftMeasure(0));
            OVE::NoteContainer* endContainer = getContainerByPos(
                              part,
                              staff,
                              slurPtr->stop()->shiftMeasure(slurPtr->start()->getMeasure()));

            if(startContainer != 0 && endContainer != 0){
                  int absStartTick = mtt_->getTick(slurPtr->start()->getMeasure(), startContainer->getTick());
                  int absEndTick = mtt_->getTick(slurPtr->start()->getMeasure()+slurPtr->stop()->getMeasure(), endContainer->getTick());

                  Slur* slur = new Slur(score_);
                  slur->setSlurDirection(slurPtr->getShowOnTop()? Direction::UP : Direction::DOWN);
                  slur->setTick(absStartTick);
                  slur->setTick2(absEndTick);
                  slur->setTrack(track);
                  slur->setTrack2(track+endContainer->getOffsetStaff());

                  score_->addSpanner(slur);
                  }
            }
      }

QString OveDynamics_To_Dynamics(OVE::DynamicsType type){
      QString dynamic = "other-dynamics";

      switch( type ){
            case OVE::DynamicsType::PPPP:
                  dynamic = "pppp";
                  break;
            case OVE::DynamicsType::PPP:
                  dynamic = "ppp";
                  break;
            case OVE::DynamicsType::PP:
                  dynamic = "pp";
                  break;
            case OVE::DynamicsType::P:
                  dynamic = "p";
                  break;
            case OVE::DynamicsType::MP:
                  dynamic = "mp";
                  break;
            case OVE::DynamicsType::MF:
                  dynamic = "mf";
                  break;
            case OVE::DynamicsType::F:
                  dynamic = "f";
                  break;
            case OVE::DynamicsType::FF:
                  dynamic = "ff";
                  break;
            case OVE::DynamicsType::FFF:
                  dynamic = "fff";
                  break;
            case OVE::DynamicsType::FFFF:
                  dynamic = "ffff";
                  break;
            case OVE::DynamicsType::SF:
                  dynamic = "sf";
                  break;
            case OVE::DynamicsType::FZ:
                  dynamic = "fz";
                  break;
            case OVE::DynamicsType::SFZ:
                  dynamic = "sfz";
                  break;
            case OVE::DynamicsType::SFFZ:
                  dynamic = "sffz";
                  break;
            case OVE::DynamicsType::FP:
                  dynamic = "fp";
                  break;
            case OVE::DynamicsType::SFP:
                  dynamic = "sfp";
                  break;
            default:
                  break;
            }

      return dynamic;
      }

void OveToMScore::convertDynamics(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> dynamics = measureData->getMusicDatas(OVE::MusicDataType::Dynamics);

      for(int i=0; i<dynamics.size(); ++i){
            OVE::Dynamics* dynamicPtr = static_cast<OVE::Dynamics*>(dynamics[i]);
            int absTick = mtt_->getTick(measure->no(), dynamicPtr->getTick());
            Dynamic* dynamic = new Dynamic(score_);

            dynamic->setDynamicType(OveDynamics_To_Dynamics(dynamicPtr->getDynamicsType()));
            dynamic->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest, absTick);
            s->add(dynamic);
            }
      }

void OveToMScore::convertExpressions(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> expressions = measureData->getMusicDatas(OVE::MusicDataType::Expressions);

      for(int i=0; i<expressions.size(); ++i){
            OVE::Expressions* expressionPtr = static_cast<OVE::Expressions*>(expressions[i]);
            int absTick = mtt_->getTick(measure->no(), expressionPtr->getTick());
            Text* t = new Text(SubStyle::EXPRESSION, score_);

            t->setPlainText(expressionPtr->getText());
            t->setTrack(track);

            Segment* s = measure->getSegment(SegmentType::ChordRest, absTick);
            s->add(t);
            }
      }

void OveToMScore::convertGlissandos(Measure* measure, int part, int staff, int track){
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> glissandos = measureData->getCrossMeasureElements(OVE::MusicDataType::Glissando, OVE::MeasureData::PairType::All);

      for(int i=0; i<glissandos.size(); ++i){
            OVE::Glissando* glissandoPtr = static_cast<OVE::Glissando*>(glissandos[i]);
            OVE::NoteContainer* startContainer = getContainerByPos(part, staff, glissandoPtr->start()->shiftMeasure(0));
            OVE::NoteContainer* endContainer = getContainerByPos(
                              part,
                              staff,
                              glissandoPtr->stop()->shiftMeasure(glissandoPtr->start()->getMeasure()));

            if(startContainer != 0 && endContainer != 0){
                  int absTick = mtt_->getTick(measure->no(), glissandoPtr->getTick());
                  ChordRest* cr = measure->findChordRest(absTick, track);
                  if(cr != 0){
                        Glissando* g = new Glissando(score_);
                        g->setGlissandoType(GlissandoType::WAVY);
                        cr->add(g);
                        }
                  }
            }
      }

static HairpinType OveWedgeType_To_Type(OVE::WedgeType type) {
      HairpinType subtype = HairpinType::CRESC_HAIRPIN;
      switch(type) {
            case OVE::WedgeType::Cres_Line: {
                  subtype = HairpinType::CRESC_HAIRPIN;
                  break;
                  }
            case OVE::WedgeType::Double_Line: {
                  subtype = HairpinType::CRESC_HAIRPIN;
                  break;
                  }
            case OVE::WedgeType::Decresc_Line: {
                  subtype = HairpinType::DECRESC_HAIRPIN;
                  break;
                  }
            case OVE::WedgeType::Cres: {
                  subtype = HairpinType::CRESC_HAIRPIN;
                  break;
                  }
            case OVE::WedgeType::Decresc: {
                  subtype = HairpinType::DECRESC_HAIRPIN;
                  break;
                  }
            default:
                  break;
            }

      return subtype;
      }

void OveToMScore::convertWedges(Measure* measure, int part, int staff, int track) {
      OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
      if(measureData == 0)
            return;

      QList<OVE::MusicData*> wedges = measureData->getCrossMeasureElements(OVE::MusicDataType::Wedge, OVE::MeasureData::PairType::All);

      for(int i=0; i<wedges.size(); ++i){
            OVE::Wedge* wedgePtr = static_cast<OVE::Wedge*>(wedges[i]);
            int absTick = mtt_->getTick(
                              measure->no(),
                              MeasureToTick::unitToTick(wedgePtr->start()->getOffset(), ove_->getQuarter()));
            int absTick2 = mtt_->getTick(
                              measure->no()+wedgePtr->stop()->getMeasure(),
                              MeasureToTick::unitToTick(wedgePtr->stop()->getOffset(), ove_->getQuarter()));

            if (absTick2 > absTick) {
                  Hairpin* hp = new Hairpin(score_);

                  hp->setHairpinType(OveWedgeType_To_Type(wedgePtr->getWedgeType()));
                  //hp->setYoff(wedgePtr->getYOffset());
                  hp->setTrack(track);

                  hp->setTick(absTick);
                  hp->setTick2(absTick2);
                  hp->setAnchor(Spanner::Anchor::SEGMENT);
                  score_->addSpanner(hp);
                  score_->updateHairpin(hp);
                  }
            }
      }

//////////////////////////////////////////////////////////////////////////

Score::FileError importOve(MasterScore* score, const QString& name) {
      OVE::IOVEStreamLoader* oveLoader = OVE::createOveStreamLoader();
      OVE::OveSong oveSong;

      QFile oveFile(name);
      if(!oveFile.exists())
            return Score::FileError::FILE_NOT_FOUND;
      if (!oveFile.open(QFile::ReadOnly)) {
            //messageOutString(QString("can't read file!"));
            return Score::FileError::FILE_OPEN_ERROR;
            }

      QByteArray buffer = oveFile.readAll();

      oveFile.close();

      oveSong.setTextCodecName(preferences.getString(PREF_IMPORT_OVERTURE_CHARSET));
      oveLoader->setOve(&oveSong);
      oveLoader->setFileStream((unsigned char*) buffer.data(), buffer.size());
      bool result = oveLoader->load();
      oveLoader->release();

      if(result){
            OveToMScore otm;
            otm.convert(&oveSong, score);

            //		score->connectSlurs();
            }

      return result ? Score::FileError::FILE_NO_ERROR : Score::FileError::FILE_ERROR;
      }


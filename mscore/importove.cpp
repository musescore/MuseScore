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
#include "musescore.h"
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
			Staff* staff = new Staff(score_, part, j);

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

void addText(VBox* & vbox, Score* s, QString strTxt, int stl) {
	if (!strTxt.isEmpty()) {
		Text* text = new Text(s);
		text->setTextStyleType(stl);
		text->setText(strTxt);
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
		addText(vbox, score_, title, TEXT_STYLE_TITLE);
	}

	QList<QString> writers = ove_->getWriters();
	if(!writers.empty()) {
		QString composer = writers[0];
		addText(vbox, score_, composer, TEXT_STYLE_COMPOSER);
	}

	if(writers.size() > 1) {
		QString lyricist = writers[1];
		addText(vbox, score_, lyricist, TEXT_STYLE_POET);
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
	        	staff->setBracket(0, BRACKET_BRACE);
	        	staff->setBracketSpan(0, 2);
	        	staff->setBarLineSpan(2);
	        }

	        // bracket
	        OVE::Staff* staffPtr = getStaff(ove_, staffIndex);
	        if(staffPtr != 0 && staffPtr->getGroupType() == OVE::Group_Bracket) {
	        	int span = staffPtr->getGroupStaffCount() + 1;
	        	int endStaff = staffIndex + span;
	        	if(span > 0 && endStaff >= staffIndex && endStaff <= ove_->getTrackCount()) {
	        		staff->addBracket(BracketItem(BRACKET_NORMAL, span));
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
	case OVE::Clef_Treble:{
		clef = ClefType::G;
		break;
	}
	case OVE::Clef_Bass:{
		clef = ClefType::F;
		break;
	}
	case OVE::Clef_Alto:{
		clef = ClefType::C3;
		break;
	}
	case OVE::Clef_UpAlto:{
		clef = ClefType::C4;
		break;
	}
	case OVE::Clef_DownDownAlto:{
		clef = ClefType::C1;
		break;
	}
	case OVE::Clef_DownAlto:{
		clef = ClefType::C2;
		break;
	}
	case OVE::Clef_UpUpAlto:{
		clef = ClefType::C5;
		break;
	}
	case OVE::Clef_Treble8va:{
		clef = ClefType::G1;
		break;
	}
	case OVE::Clef_Bass8va:{
		clef = ClefType::F_8VA;
		break;
	}
	case OVE::Clef_Treble8vb:{
		clef = ClefType::G3;
		break;
	}
	case OVE::Clef_Bass8vb:{
		clef = ClefType::F8;
		break;
	}
	case OVE::Clef_Percussion1:{
		clef = ClefType::PERC;
		break;
	}
	case OVE::Clef_Percussion2:{
		clef = ClefType::PERC2;
		break;
	}
	case OVE::Clef_TAB:{
		clef = ClefType::TAB;
		break;
	}
	default:
		break;
	}
	return clef;
}

Note::NoteHeadGroup getHeadGroup(OVE::NoteHeadType type) {
    Note::NoteHeadGroup headGroup = Note::HEAD_NORMAL;
	switch (type) {
	case OVE::NoteHead_Standard: {
		headGroup = Note::HEAD_NORMAL;
		break;
	}
	case OVE::NoteHead_Invisible: {
		break;
	}
	case OVE::NoteHead_Rhythmic_Slash: {
		headGroup = Note::HEAD_SLASH;
		break;
	}
	case OVE::NoteHead_Percussion: {
		headGroup = Note::HEAD_XCIRCLE;
		break;
	}
	case OVE::NoteHead_Closed_Rhythm: {
		headGroup = Note::HEAD_CROSS;
		break;
	}
	case OVE::NoteHead_Open_Rhythm: {
		headGroup = Note::HEAD_CROSS;
		break;
	}
	case OVE::NoteHead_Closed_Slash: {
		headGroup = Note::HEAD_SLASH;
		break;
	}
	case OVE::NoteHead_Open_Slash: {
		headGroup = Note::HEAD_SLASH;
		break;
	}
	case OVE::NoteHead_Closed_Do: {
		headGroup = Note::HEAD_DO;
		break;
	}
	case OVE::NoteHead_Open_Do: {
		headGroup = Note::HEAD_DO;
		break;
	}
	case OVE::NoteHead_Closed_Re: {
		headGroup = Note::HEAD_RE;
		break;
	}
	case OVE::NoteHead_Open_Re: {
		headGroup = Note::HEAD_RE;
		break;
	}
	case OVE::NoteHead_Closed_Mi: {
		headGroup = Note::HEAD_MI;
		break;
	}
	case OVE::NoteHead_Open_Mi: {
		headGroup = Note::HEAD_MI;
		break;
	}
	case OVE::NoteHead_Closed_Fa: {
		headGroup = Note::HEAD_FA;
		break;
	}
	case OVE::NoteHead_Open_Fa: {
		headGroup = Note::HEAD_FA;
		break;
	}
	case OVE::NoteHead_Closed_Sol: {
		break;
	}
	case OVE::NoteHead_Open_Sol: {
		break;
	}
	case OVE::NoteHead_Closed_La: {
		headGroup = Note::HEAD_LA;
		break;
	}
	case OVE::NoteHead_Open_La: {
		headGroup = Note::HEAD_LA;
		break;
	}
	case OVE::NoteHead_Closed_Ti: {
		headGroup = Note::HEAD_TI;
		break;
	}
	case OVE::NoteHead_Open_Ti: {
		headGroup = Note::HEAD_TI;
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
		part->setLongName(longName);
	}

	QString shortName = track->getBriefName();
	if (shortName != QString() && track->getShowBriefName()) {
		part->setShortName(shortName);
	}

	part->setMidiProgram(track->getPatch());

	if (ove_->getShowTransposeTrack() && track->getTranspose() != 0 ) {
        Ms::Interval interval = part->instr()->transpose();
        interval.diatonic = -track->getTranspose();
        part->instr()->setTranspose(interval);
	}

	// DrumSet
	if(track->getStartClef()==OVE::Clef_Percussion1 || track->getStartClef()==OVE::Clef_Percussion2) {
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

		part->instr()->channel(0).bank = 128;
		part->setMidiProgram(0);
		part->instr()->setUseDrumset(true);
        part->instr()->setDrumset(drumset);
	}
}

static OttavaType OctaveShiftTypeToInt(OVE::OctaveShiftType type) {
	OttavaType subtype = OttavaType::OTTAVA_8VA;
	switch (type) {
	case OVE::OctaveShift_8: {
		subtype = OttavaType::OTTAVA_8VA;
		break;
	}
	case OVE::OctaveShift_15: {
		subtype = OttavaType::OTTAVA_15MA;
		break;
	}
	case OVE::OctaveShift_Minus_8: {
		subtype = OttavaType::OTTAVA_8VB;
		break;
	}
	case OVE::OctaveShift_Minus_15: {
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
		QList<OVE::MusicData*> octaves = measureData->getMusicDatas(OVE::MusicData_OctaveShift_EndPoint);
		for(int j=0; j<octaves.size(); ++j) {
			OVE::OctaveShiftEndPoint* octave = static_cast<OVE::OctaveShiftEndPoint*>(octaves[j]);
			int absTick = mtt_->getTick(i, octave->getTick());

			if(octave->getOctaveShiftPosition() == OVE::OctavePosition_Start) {
				if(ottava == 0) {
					ottava = new Ottava(score_);
                    ottava->setTrack(track * VOICES);
                    ottava->setOttavaType(OctaveShiftTypeToInt(octave->getOctaveShiftType()));

                    int y_off = 0;
                	switch (octave->getOctaveShiftType()) {
                	case OVE::OctaveShift_8:
                	case OVE::OctaveShift_15: {
                		y_off = -3;
                		break;
                	}
                	case OVE::OctaveShift_Minus_8:
                	case OVE::OctaveShift_Minus_15: {
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
					qDebug("overlapping octave-shift not supported\n");
					delete ottava;
					ottava = 0;
				}
			} else if (octave->getOctaveShiftPosition() == OVE::OctavePosition_Stop) {
				if(ottava != 0) {
					int absTick = mtt_->getTick(i, octave->getEndTick());

                    ottava->setTick2(absTick);
                    ottava->staff()->updateOttava(ottava);
                    score_->addSpanner(ottava);
                    ottava = 0;
				} else {
                    qDebug("octave-shift stop without start\n");
				}
			}
		}
	}
}

void OveToMScore::convertLineBreak(){
    for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
		if (mb->type() != Element::MEASURE)
			continue;
		Measure* measure = static_cast<Measure*> (mb);

		for (int i = 0; i < ove_->getLineCount(); ++i) {
			OVE::Line* line = ove_->getLine(i);
			if (measure->no() > 0) {
				if ((int)line->getBeginBar() + (int)line->getBarCount()-1 == measure->no()) {
					LayoutBreak* lb = new LayoutBreak(score_);
					lb->setTrack(0);
					lb->setLayoutBreakType(LayoutBreak::LINE);
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
				TimeSigType subtype = TSIG_NORMAL;
				if(tt.numerator_ == 4 && tt.denominator_ == 4 && tt.isSymbol_ ){
					subtype = TSIG_FOUR_FOUR;
				} else if(tt.numerator_ == 2 && tt.denominator_ == 2 && tt.isSymbol_ ){
					subtype = TSIG_ALLA_BREVE;
				}

					TimeSig* ts = new TimeSig(score_);
					ts->setTrack(staffIdx * VOICES);
					ts->setSig(Fraction(tt.numerator_, tt.denominator_), subtype);

					Segment* seg = measure->getSegment(ts, tt.tick_);
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
		                	KeySigEvent key;
		                	key.setAccidentalType(keyValue);
		                	(*score_->staff(staffCount+j)->keymap())[tick] = key;

		                    KeySig* keysig = new KeySig(score_);
		                    keysig->setTrack((staffCount+j) * VOICES);
		                    keysig->setKeySigEvent(key);

		                	Segment* s = measure->getSegment(keysig, tick);
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

					Segment* s = measure->getSegment(keysig, 0);
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

	            Segment* s = measure->getSegment(clef, 0);
	            s->add(clef);
			}

			// clef in measure
			for(k=0; k<ove_->getMeasureCount(); ++k){
				OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k);
				QList<OVE::MusicData*> clefs = measureData->getMusicDatas(OVE::MusicData_Clef);
				Measure* measure = score_->tick2measure(mtt_->getTick(k, 0));

				for( int l=0; l<clefs.size(); ++l){
					if(measure != 0){
						OVE::Clef* clefPtr = static_cast<OVE::Clef*>(clefs[l]);
						int absTick = mtt_->getTick(k, clefPtr->getTick());
						ClefType clefType = OveClefToClef(clefPtr->getClefType());

			            Clef* clef = new Clef(score_);
			            clef->setClefType(clefType);
			            clef->setTrack((staffCount+j)*VOICES);

			            Segment* s = measure->getSegment(clef, absTick);
			            s->add(clef);

//						if(staff){
//                                          staff->setClef(absTick, clefType);
//						}
					}
				}
			}
		}

		staffCount += partStaffCount;
	}

	// Tempo
	std::map<int, int> tempos;
	for(i=0; i<ove_->getPartCount(); ++i){
		int partStaffCount = ove_->getStaffCount(i);

		for(j=0; j<partStaffCount; ++j){
			for(k=0; k<ove_->getMeasureCount(); ++k){
				OVE::Measure* measure = ove_->getMeasure(k);
				OVE::MeasureData* measureData = ove_->getMeasureData(i, j, k);
				QList<OVE::MusicData*> tempoPtrs = measureData->getMusicDatas(OVE::MusicData_Tempo);

				if(k==0 || ( k>0 && qAbs(measure->getTypeTempo()-ove_->getMeasure(k-1)->getTypeTempo())>0.01 )){
					int tick = mtt_->getTick(k, 0);
					tempos[tick] = (int)measure->getTypeTempo();
				}

				for(int l=0; l<tempoPtrs.size(); ++l) {
					OVE::Tempo* ptr = static_cast<OVE::Tempo*>(tempoPtrs[l]);
					int tick = mtt_->getTick(measure->getBarNumber()->getIndex(), ptr->getTick());
					int tempo = ptr->getQuarterTempo()>0 ? ptr->getQuarterTempo() : 1;

					tempos[tick] = tempo;
				}
			}
		}
	}

	std::map<int, int>::iterator it;
	int lastTempo = 0;
	for(it=tempos.begin(); it!=tempos.end(); ++it) {
		if( it==tempos.begin() || (*it).second != lastTempo ) {
	        double tpo = ((double)(*it).second) / 60.0;
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
	case OVE::Note_DoubleWhole: {
		d.setType(TDuration::V_BREVE);
		break;
	}
	case OVE::Note_Whole: {
		d.setType(TDuration::V_WHOLE);
		break;
	}
	case OVE::Note_Half: {
		d.setType(TDuration::V_HALF);
		break;
	}
	case OVE::Note_Quarter: {
		d.setType(TDuration::V_QUARTER);
		break;
	}
	case OVE::Note_Eight: {
		d.setType(TDuration::V_EIGHT);
		break;
	}
	case OVE::Note_Sixteen: {
		d.setType(TDuration::V_16TH);
		break;
	}
	case OVE::Note_32: {
		d.setType(TDuration::V_32ND);
		break;
	}
	case OVE::Note_64: {
		d.setType(TDuration::V_64TH);
		break;
	}
	case OVE::Note_128: {
		d.setType(TDuration::V_128TH);
		break;
	}
	case OVE::Note_256: {
		d.setType(TDuration::V_256TH);
		break;
	}
	default:
		d.setType(TDuration::V_QUARTER);
		break;
	}

	return d;
}

int accidentalToAlter(OVE::AccidentalType type) {
	int alter = 0;

	switch( type ) {
	case OVE::Accidental_Normal:
	case OVE::Accidental_Natural:
	case OVE::Accidental_Natural_Caution: {
			alter = 0;
			break;
		}
	case OVE::Accidental_Sharp:
	case OVE::Accidental_Sharp_Caution: {
			alter = 1;
			break;
		}
	case OVE::Accidental_Flat:
	case OVE::Accidental_Flat_Caution: {
			alter = -1;
			break;
		}
	case OVE::Accidental_DoubleSharp:
	case OVE::Accidental_DoubleSharp_Caution: {
			alter = 2;
			break;
		}
	case OVE::Accidental_DoubleFlat:
	case OVE::Accidental_DoubleFlat_Caution: {
			alter = -2;
			break;
		}
	default:
		break;
	}

	return alter;
}

void getMiddleToneOctave(OVE::ClefType clef, OVE::ToneType& tone, int& octave) {
	tone = OVE::Tone_B;
	octave = 4;

	switch ( clef ) {
	case OVE::Clef_Treble: {
			tone = OVE::Tone_B;
			octave = 4;
			break;
		}
	case OVE::Clef_Treble8va: {
			tone = OVE::Tone_B;
			octave = 5;
			break;
		}
	case OVE::Clef_Treble8vb: {
			tone = OVE::Tone_B;
			octave = 3;
			break;
		}
	case OVE::Clef_Bass: {
			tone = OVE::Tone_D;
			octave = 3;
			break;
		}
	case OVE::Clef_Bass8va: {
			tone = OVE::Tone_D;
			octave = 4;
			break;
		}
	case OVE::Clef_Bass8vb: {
			tone = OVE::Tone_D;
			octave = 2;
			break;
		}
	case OVE::Clef_Alto: {
			tone = OVE::Tone_C;
			octave = 4;
			break;
		}
	case OVE::Clef_UpAlto: {
			tone = OVE::Tone_A;
			octave = 3;
			break;
		}
	case OVE::Clef_DownDownAlto: {
			tone = OVE::Tone_G;
			octave = 4;
			break;
		}
	case OVE::Clef_DownAlto: {
			tone = OVE::Tone_E;
			octave = 4;
			break;
		}
	case OVE::Clef_UpUpAlto: {
			tone = OVE::Tone_F;
			octave = 3;
			break;
		}
	case OVE::Clef_Percussion1: {
			tone = OVE::Tone_A;
			octave = 3;
			break;
		}
	case OVE::Clef_Percussion2: {
			tone = OVE::Tone_A;
			octave = 3;
			break;
		}
	case OVE::Clef_TAB: {
			break;
		}
	default:
		break;
	}
}

OVE::ClefType getClefType(OVE::MeasureData* measure, int tick) {
	OVE::ClefType type = measure->getClef()->getClefType();
	QList<OVE::MusicData*> clefs = measure->getMusicDatas(OVE::MusicData_Clef);

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
          if (mb->type() != Element::MEASURE)
                continue;
          Measure* measure = static_cast<Measure*>(mb);

          convertMeasure(measure);
    }

    //  convert based on notes
    for (MeasureBase* mb = score_->measures()->first(); mb; mb = mb->next()) {
          if (mb->type() != Element::MEASURE)
                continue;
          Measure* measure = static_cast<Measure*>(mb);

          convertLines(measure);
    }
}

void OveToMScore::convertMeasure(Measure* measure){
	int staffCount = 0;
	int measureCount = ove_->getMeasureCount();

	for( int i=0; i<ove_->getPartCount(); ++i ){
		int partStaffCount = ove_->getStaffCount(i);

		for( int j=0; j<partStaffCount; ++j ){
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
	BarLineType bartype = NORMAL_BAR;

	switch(measurePtr->getRightBarline()) {
	case OVE::Barline_Default:{
			bartype = NORMAL_BAR;
			break;
		}
	case OVE::Barline_Double:{
			bartype = DOUBLE_BAR;
			break;
		}
	case OVE::Barline_Final:{
			bartype = END_BAR;
			break;
		}
	case OVE::Barline_Null:{
			bartype = NORMAL_BAR;
			break;
		}
	case OVE::Barline_RepeatLeft:{
			bartype = START_REPEAT;
			measure->setRepeatFlags(RepeatStart);
			break;
		}
	case OVE::Barline_RepeatRight:{
			bartype = END_REPEAT;
			measure->setRepeatFlags(RepeatEnd);
			break;
		}
	case OVE::Barline_Dashed:{
			bartype = BROKEN_BAR;
			break;
		}
	default:
		break;
	}

	if(measure->no() == ove_->getMeasureCount()-1){
		bartype = END_BAR;
	}

	measure->setEndBarLineType(bartype, false);

	if(measurePtr->getLeftBarline() == OVE::Barline_RepeatLeft){
		//bartype = START_REPEAT;
		measure->setRepeatFlags(measure->repeatFlags()|RepeatStart);
	}

	// rehearsal
	int i;
	QList<OVE::MusicData*> texts = measureData->getMusicDatas(OVE::MusicData_Text);
	for(i=0; i<texts.size(); ++i){
		OVE::Text* textPtr = static_cast<OVE::Text*>(texts[i]);
		if(textPtr->getTextType() == OVE::Text::Text_Rehearsal){
			Text* text = new RehearsalMark(score_);
            text->setText(textPtr->getText());
            text->setAbove(true);
            text->setTrack(track);

            Segment* s = measure->getSegment(Segment::SegChordRest, mtt_->getTick(measure->no(), 0));
            s->add(text);
		}
	}

	// tempo
	QList<OVE::MusicData*> tempos = measureData->getMusicDatas(OVE::MusicData_Tempo);
	for(i=0; i<tempos.size(); ++i){
		OVE::Tempo* tempoPtr = static_cast<OVE::Tempo*>(tempos[i]);
		TempoText* t = new TempoText(score_);
		int absTick = mtt_->getTick(measure->no(), tempoPtr->getTick());
		double tpo = ((double)tempoPtr->getQuarterTempo())/60.0;

        score_->setTempo(absTick, tpo);

        t->setTempo(tpo);
        t->setText(tempoPtr->getRightText());
        t->setAbove(true);
        t->setTrack(track);

        Segment* s = measure->getSegment(Segment::SegChordRest, absTick);
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
	case OVE::Note_DoubleWhole:
	case OVE::Note_Whole:
	case OVE::Note_Half:
	case OVE::Note_Quarter: {
		if(rest->getLine() != 0)
			isDefault = false;
		break;
	}
	case OVE::Note_Eight: {
		if(rest->getLine() != 1)
			isDefault = false;
		break;
	}
	case OVE::Note_Sixteen:
	case OVE::Note_32: {
		if(rest->getLine() != -1)
			isDefault = false;
		break;
	}
	case OVE::Note_64: {
		if(rest->getLine() != -3)
			isDefault = false;
		break;
	}
	case OVE::Note_128: {
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
	return p->instr()->drumset();
}

void OveToMScore::convertNotes(Measure* measure, int part, int staff, int track){
	int j;
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	QList<OVE::NoteContainer*> containers = measureData->getNoteContainers();
	QList<OVE::MusicData*> tuplets = measureData->getCrossMeasureElements(OVE::MusicData_Tuplet, OVE::MeasureData::PairType_Start);
	QList<OVE::MusicData*> beams = measureData->getCrossMeasureElements(OVE::MusicData_Beam, OVE::MeasureData::PairType_Start);
	Tuplet* tuplet = 0;
	ChordRest* cr = 0;
	int partStaffCount = ove_->getStaffCount(part);

	if(containers.empty()){
		TDuration duration(TDuration::V_MEASURE);
		int absTick = mtt_->getTick(measure->no(), 0);

		cr = new Rest(score_, duration);
            cr->setDuration(measure->len());
		cr->setTrack(track);
		Segment* s = measure->getSegment(cr, absTick);
		s->add(cr);
	}

	for (int i = 0; i < containers.size(); ++i) {
		OVE::NoteContainer* container = containers[i];
		int tick = mtt_->getTick(measure->no(), container->getTick());
		int noteTrack = track + container->getVoice();

		if (container->getIsRest()) {
			TDuration duration = OveNoteType_To_Duration(container->getNoteType());
			duration.setDots(container->getDot());

			cr = new Rest(score_, duration);
                  cr->setDuration(duration.fraction());
			cr->setTrack(noteTrack);
			cr->setVisible(container->getShow());

			QList<OVE::Note*> notes = container->getNotesRests();
			for (j = 0; j < notes.size(); ++j) {
				OVE::Note* notePtr = notes[j];
				if(!isRestDefaultLine(notePtr, container->getNoteType()) && notePtr->getLine() != 0) {
					double yOffset = -((double)notePtr->getLine()/2.0 * score_->spatium());
					cr->setUserYoffset(yOffset);
				}
			}

			Segment* s = measure->getSegment(cr, tick);
			s->add(cr);
		} else {
			QList<OVE::Note*> notes = container->getNotesRests();
			int graceLevel = getGraceLevel(containers, container->getTick(), container->start()->getOffset());
			// TODO-S          cr = measure->findChord(tick, noteTrack, graceLevel);
			cr = measure->findChord(tick, noteTrack);
			if (cr == 0) {
				// Segment::SegmentType st = Segment::SegChordRest;

				cr = new Ms::Chord(score_);
				cr->setTrack(noteTrack);

				// grace
				if (container->getIsGrace()) {
					TDuration duration = OveNoteType_To_Duration(container->getGraceNoteType());
					duration.setDots(container->getDot());
					((Ms::Chord*) cr)->setNoteType(NOTE_APPOGGIATURA);

					if (duration.type() == TDuration::V_QUARTER) {
						((Ms::Chord*) cr)->setNoteType(NOTE_GRACE4);
						cr->setDurationType(TDuration::V_QUARTER);
					} else if (duration.type() == TDuration::V_16TH) {
						((Ms::Chord*) cr)->setNoteType(NOTE_GRACE16);
						cr->setDurationType(TDuration::V_16TH);
					} else if (duration.type() == TDuration::V_32ND) {
						((Ms::Chord*) cr)->setNoteType(NOTE_GRACE32);
						cr->setDurationType(TDuration::V_32ND);
					} else {
						cr->setDurationType(TDuration::V_EIGHT);
					}

					// st = Segment::SegGrace;
				} else {
					TDuration duration = OveNoteType_To_Duration(container->getNoteType());
					duration.setDots(container->getDot());

					if (duration.type() == TDuration::V_INVALID)
						duration.setType(TDuration::V_QUARTER);
					cr->setDurationType(duration);
				}
				cr->setDuration(cr->durationType().fraction());

//TODO-S	Deal with grace notes
//        Segment* s = measure->getGraceSegment(tick, graceLevel);
//				s->add(cr);
          if(graceLevel == 0) {
               Segment* s = measure->getSegment(cr, tick);
               s->add(cr);
               }
			}

			for (j = 0; j < notes.size(); ++j) {
				OVE::Note* oveNote = notes[j];
				Note* note = new Note(score_);
				int pitch = oveNote->getNote();

				//note->setTrack(noteTrack);
                note->setVeloType(MScore::USER_VAL);
				note->setVeloOffset(oveNote->getOnVelocity());
				//note->setUserAccidental(OveAccidental_to_Accidental(notePtr->getAccidental()));
				note->setPitch(pitch);

				// tpc
				bool setDirection = false;
				OVE::ClefType clefType = getClefType(measureData, container->getTick());
				if(clefType == OVE::Clef_Percussion1 || clefType == OVE::Clef_Percussion2) {
					Drumset* drumset = getDrumset(score_, part);
					if(drumset != 0) {
						if (!drumset->isValid(pitch) || pitch == -1) {
							qDebug("unmapped drum note 0x%02x %d\n", note->pitch(), note->pitch());
						} else {
							note->setHeadGroup(drumset->noteHead(pitch));
							int line = drumset->line(pitch);
							note->setLine(line);
							note->setTpcFromPitch();
							((Ms::Chord*) cr)->setStemDirection(drumset->stemDirection(pitch));
							setDirection = true;
						}
					}
				} else {
					const int OCTAVE = 7;
					OVE::ToneType clefMiddleTone;
					int clefMiddleOctave;
					getMiddleToneOctave(clefType, clefMiddleTone, clefMiddleOctave);
					int absLine = (int) clefMiddleTone + clefMiddleOctave * OCTAVE + oveNote->getLine();
					int tone = absLine % OCTAVE;
					int alter = accidentalToAlter(oveNote->getAccidental());
					note->setTpc(step2tpc(tone, AccidentalVal(alter)));

					note->setHeadGroup(getHeadGroup(oveNote->getHeadType()));
				}

				// tie
				if ((oveNote->getTiePos() & OVE::Tie_LeftEnd) == OVE::Tie_LeftEnd) {
					Tie* tie = new Tie(score_);
					note->setTieFor(tie);
					tie->setStartNote(note);
					tie->setTrack(noteTrack);
				}

				// pitch must be set before adding note to chord as note
				// is inserted into pitch sorted list (ws)
				cr->add(note);

				cr->setVisible(oveNote->getShow());
				((Ms::Chord*) cr)->setNoStem((int) container->getNoteType() <= OVE::Note_Whole);
				if(!setDirection)
					((Ms::Chord*) cr)->setStemDirection(container->getStemUp() ? MScore::UP : MScore::DOWN);

				// cross staff
				int staffMove = 0;
				if(partStaffCount == 2){/*treble-bass*/
					staffMove = oveNote->getOffsetStaff();
				}
				cr->setStaffMove(staffMove);
			}
		}

		// beam
		BeamMode bm = container->getIsRest() ? BeamMode::NONE : BeamMode::AUTO;
		if(container->getInBeam()){
			OVE::MeasurePos pos = container->start()->shiftMeasure(0);
			OVE::MusicData* data = getCrossMeasureElementByPos(part, staff, pos, container->getVoice(), OVE::MusicData_Beam);

			if(data != 0){
				OVE::Beam* beam = static_cast<OVE::Beam*>(data);
				OVE::MeasurePos startPos = beam->start()->shiftMeasure(0);
				OVE::MeasurePos stopPos = beam->stop()->shiftMeasure(beam->start()->getMeasure());

				if(startPos == pos){
					bm = BeamMode::BEGIN;
				}
				else if(stopPos == pos){
					bm = BeamMode::END;
				}
				else{
					bm = BeamMode::MID;
				}
			}
		}
		cr->setBeamMode(bm);

		// tuplet
		if (container->getTuplet() > 0) {
			if (tuplet == 0) {
				bool create = true;

				// check tuplet start
				if(container->getNoteType() < OVE::Note_Eight) {
					const OVE::Tuplet* oveTuplet = getTuplet(tuplets, container->start()->getOffset());
					if(oveTuplet == 0) {
						create = false;
					}
				}

				if(create) {
					tuplet = new Tuplet(score_);
					tuplet->setTrack(noteTrack);
					tuplet->setRatio(Fraction(container->getTuplet(), container->getSpace()));
					tuplet->setTick(tick);
					measure->add(tuplet);
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
					tuplet->setDirection(oveTuplet->getLeftShoulder()->getYOffset() < 0 ? MScore::UP : MScore::DOWN);

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
	case OVE::Articulation_Major_Trill :
	case OVE::Articulation_Minor_Trill :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Trill);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Trill_Section :{
		break;
	}
	case OVE::Articulation_Inverted_Short_Mordent :
	case OVE::Articulation_Inverted_Long_Mordent :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Prall);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Short_Mordent :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Mordent);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Turn :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Turn);
		cr->add(a);
		break;
	}
//	case OVE::Articulation_Flat_Accidental_For_Trill :
//	case OVE::Articulation_Sharp_Accidental_For_Trill :
//	case OVE::Articulation_Natural_Accidental_For_Trill :
	case OVE::Articulation_Tremolo_Eighth :{
		Tremolo* t = new Tremolo(score_);
		t->setTremoloType(TREMOLO_R8);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Tremolo_Sixteenth :{
		Tremolo* t = new Tremolo(score_);
		t->setTremoloType(TREMOLO_R16);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Tremolo_Thirty_Second :{
		Tremolo* t = new Tremolo(score_);
		t->setTremoloType(TREMOLO_R32);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Tremolo_Sixty_Fourth :{
		Tremolo* t = new Tremolo(score_);
		t->setTremoloType(TREMOLO_R64);
		cr->add(t);
		break;
	}
	case OVE::Articulation_Marcato :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Sforzatoaccent);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Marcato_Dot :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Sforzatoaccent);
		cr->add(a);

		a = new Articulation(score_);
		a->setArticulationType(Articulation_Staccato);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Heavy_Attack :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Sforzatoaccent);
		cr->add(a);

		a = new Articulation(score_);
		a->setArticulationType(Articulation_Tenuto);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando :{
		Articulation* a = new Articulation(score_);
            a->setUp(true);
		a->setArticulationType(Articulation_Marcato);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando_Inverted :{
		Articulation* a = new Articulation(score_);
            a->setUp(false);
		a->setArticulationType(Articulation_Marcato);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando_Dot :{
		Articulation* a = new Articulation(score_);
            a->setUp(true);
		a->setArticulationType(Articulation_Marcato);
		cr->add(a);

		a = new Articulation(score_);
		a->setArticulationType(Articulation_Staccato);
		cr->add(a);
		break;
	}
	case OVE::Articulation_SForzando_Dot_Inverted :{
		Articulation* a = new Articulation(score_);
            a->setUp(false);
		a->setArticulationType(Articulation_Marcato);
		cr->add(a);

		a = new Articulation(score_);
		a->setArticulationType(Articulation_Staccato);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Heavier_Attack :{
		Articulation* a = new Articulation(score_);
            a->setUp(true);
		a->setArticulationType(Articulation_Marcato);
		cr->add(a);

		a = new Articulation(score_);
		a->setArticulationType(Articulation_Tenuto);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Staccatissimo :{
		Articulation* a = new Articulation(score_);
            a->setUp(true);
		a->setArticulationType(Articulation_Staccatissimo);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Staccato :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Staccato);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Tenuto :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Tenuto);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Pause :{
        Breath* b = new Breath(score_);
        b->setTrack(track);
        Segment* seg = measure->getSegment(Segment::SegBreath, absTick);
        seg->add(b);
		break;
	}
	case OVE::Articulation_Grand_Pause :{
		break;
	}
	case OVE::Articulation_Up_Bow :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Upbow);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Down_Bow :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Downbow);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Up_Bow_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Upbow);
		a->setUserYoffset(5.3);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Down_Bow_Inverted :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Downbow);
		a->setUserYoffset(5.3);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Natural_Harmonic :{
		break;
	}
	case OVE::Articulation_Artificial_Harmonic :{
		break;
	}
	case OVE::Articulation_Finger_1 :
	case OVE::Articulation_Finger_2 :
	case OVE::Articulation_Finger_3 :
	case OVE::Articulation_Finger_4 :
	case OVE::Articulation_Finger_5 :{
		break;
	}
	case OVE::Articulation_Plus_Sign :{
		Articulation* a = new Articulation(score_);
		a->setArticulationType(Articulation_Plusstop);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Arpeggio :{
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
	case OVE::Articulation_Fermata :{
		Articulation* a = new Articulation(score_);
            a->setUp(true);
		a->setArticulationType(Articulation_Fermata);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Fermata_Inverted :{
		Articulation* a = new Articulation(score_);
            a->setUp(false);
		a->setArticulationType(Articulation_Fermata);
		cr->add(a);
		break;
	}
	case OVE::Articulation_Pedal_Down :{
		if (pedal_) {
			delete pedal_;
			pedal_ = 0;
		}
		else {
			pedal_ = new Pedal(score_);
			pedal_->setTrack(track);
            Segment* seg = measure->getSegment(Segment::SegChordRest, absTick);
            pedal_->setTick(seg->tick());
            seg->add(pedal_);
		}
		break;
	}
	case OVE::Articulation_Pedal_Up :{
		if(pedal_){
              Segment* seg = measure->getSegment(Segment::SegChordRest, absTick);
              pedal_->setTick2(seg->tick());
		  pedal_ = 0;
		}
		break;
	}
//	case OVE::Articulation_Toe_Pedal :
//	case OVE::Articulation_Heel_Pedal :
//	case OVE::Articulation_Toe_To_Heel_Pedal :
//	case OVE::Articulation_Heel_To_Toe_Pedal :
//	case OVE::Articulation_Open_String :
	default:
		break;
	}
}

void OveToMScore::convertLyrics(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	QList<OVE::MusicData*> lyrics = measureData->getMusicDatas(OVE::MusicData_Lyric);

	for(int i=0; i<lyrics.size(); ++i){
		OVE::Lyric* oveLyric = static_cast<OVE::Lyric*>(lyrics[i]);
		int tick = mtt_->getTick(measure->no(), oveLyric->getTick());

		Lyrics* lyric = new Lyrics(score_);
		lyric->setNo(oveLyric->getVerse());
		lyric->setText(oveLyric->getLyric());
		lyric->setTrack(track);
	    Segment* segment = measure->getSegment(Segment::SegChordRest, tick);
          if (segment->element(track))
                  static_cast<ChordRest*>(segment->element(track))->add(lyric);
	}
}

QString OveHarmony_To_String(OVE::HarmonyType type){
	static std::map<unsigned int, QString> harmony_map;

	harmony_map[OVE::Harmony_maj] = "major";
	harmony_map[OVE::Harmony_min] = "minor";
	harmony_map[OVE::Harmony_aug] = "augmented";
	harmony_map[OVE::Harmony_dim] = "diminished";
	harmony_map[OVE::Harmony_dim7] = "diminished-seventh";
	harmony_map[OVE::Harmony_sus2] = "suspended-second";
	harmony_map[OVE::Harmony_sus4] = "suspended-fourth";
	harmony_map[OVE::Harmony_sus24] = "suspended-second";
	harmony_map[OVE::Harmony_add2] = "major";
	harmony_map[OVE::Harmony_add9] = "dominant-ninth";
	//harmony_map[OVE::Harmony_omit3] = "";
	//harmony_map[OVE::Harmony_omit5] = "";
	harmony_map[OVE::Harmony_2] = "2";
	harmony_map[OVE::Harmony_5] = "power";
	harmony_map[OVE::Harmony_6] = "major-sixth";
	harmony_map[OVE::Harmony_69] = "major-sixth";
	harmony_map[OVE::Harmony_7] = "dominant";
	harmony_map[OVE::Harmony_7b5] = "dominant";
	harmony_map[OVE::Harmony_7b9] = "dominant";
	harmony_map[OVE::Harmony_7s9] = "dominant";
	harmony_map[OVE::Harmony_7s11] = "dominant";
	harmony_map[OVE::Harmony_7b5s9] = "dominant";
	harmony_map[OVE::Harmony_7b5b9] = "dominant";
	harmony_map[OVE::Harmony_7b9s9] = "dominant";
	harmony_map[OVE::Harmony_7b9s11] = "dominant";
	harmony_map[OVE::Harmony_7sus4] = "suspended-fourth";
	harmony_map[OVE::Harmony_9] = "dominant-ninth";
	harmony_map[OVE::Harmony_9b5] = "dominant-ninth";
	harmony_map[OVE::Harmony_9s11] = "dominant-ninth";
	harmony_map[OVE::Harmony_9sus4] = "dominant-ninth";
	harmony_map[OVE::Harmony_11] = "dominant-11th";
	harmony_map[OVE::Harmony_13] = "dominant-13th";
	harmony_map[OVE::Harmony_13b5] = "dominant-13th";
	harmony_map[OVE::Harmony_13b9] = "dominant-13th";
	harmony_map[OVE::Harmony_13s9] = "dominant-13th";
	harmony_map[OVE::Harmony_13s11] = "dominant-13th";
	harmony_map[OVE::Harmony_13sus4] = "dominant-13th";
	harmony_map[OVE::Harmony_min_add2] = "minor";
	harmony_map[OVE::Harmony_min_add9] = "minor";
	harmony_map[OVE::Harmony_min_maj7] = "minor-major";
	harmony_map[OVE::Harmony_min6] = "minor-sixth";
	harmony_map[OVE::Harmony_min6_add9] = "minor-sixth";
	harmony_map[OVE::Harmony_min7] = "minor-seventh";
	harmony_map[OVE::Harmony_min7b5] = "half-diminished";
	harmony_map[OVE::Harmony_min7_add4] = "minor-seventh";
	harmony_map[OVE::Harmony_min7_add11] = "minor-seventh";
	harmony_map[OVE::Harmony_min9] = "minor-ninth";
	harmony_map[OVE::Harmony_min9_b5] = "minor-ninth";
	harmony_map[OVE::Harmony_min9_maj7] = "major-minor";
	harmony_map[OVE::Harmony_min11] = "minor-11th";
	harmony_map[OVE::Harmony_min13] = "minor-13th";
	harmony_map[OVE::Harmony_maj7] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_b5] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_s5] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_69] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_add9] = "major-seventh";
	harmony_map[OVE::Harmony_maj7_s11] = "major-seventh";
	harmony_map[OVE::Harmony_maj9] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_sus4] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_b5] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_s5] = "major-ninth";
	harmony_map[OVE::Harmony_maj9_s11] = "major-ninth";
	harmony_map[OVE::Harmony_maj13] = "major-13th";
	harmony_map[OVE::Harmony_maj13_b5] = "major-13th";
	harmony_map[OVE::Harmony_maj13_b9] = "major-13th";
	harmony_map[OVE::Harmony_maj13_b9b5] = "major-13th";
	harmony_map[OVE::Harmony_maj13_s11] = "major-13th";
	harmony_map[OVE::Harmony_aug7] = "augmented-seventh";
	harmony_map[OVE::Harmony_aug7_b9] = "augmented-seventh";
	harmony_map[OVE::Harmony_aug7_s9] = "augmented-seventh";

	return harmony_map[type];
}

void OveToMScore::convertHarmonys(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	QList<OVE::MusicData*> harmonys = measureData->getMusicDatas(OVE::MusicData_Harmony);

	for(int i=0; i<harmonys.size(); ++i){
		OVE::Harmony* harmonyPtr = static_cast<OVE::Harmony*>(harmonys[i]);
		int absTick = mtt_->getTick(measure->no(), harmonyPtr->getTick());

		Harmony* harmony = new Harmony(score_);

		// TODO - does this need to be key-aware?
		harmony->setTrack(track);
		harmony->setRootTpc(pitch2tpc(harmonyPtr->getRoot(), KEY_C, PREFER_NEAREST));
		if(harmonyPtr->getBass() != OVE::INVALID_NOTE && harmonyPtr->getBass() != harmonyPtr->getRoot()){
			harmony->setBaseTpc(pitch2tpc(harmonyPtr->getBass(), KEY_C, PREFER_NEAREST));
		}
		const ChordDescription* d = harmony->fromXml(OveHarmony_To_String(harmonyPtr->getHarmonyType()));
		if(d != 0){
			harmony->setId(d->id);
			harmony->render();
		}

		Segment* s = measure->getSegment(Segment::SegChordRest, absTick);
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
		const QList<OVE::MusicData*>& datas = measureData->getCrossMeasureElements(type, OVE::MeasureData::PairType_All);
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
	QList<OVE::MusicData*> repeats = measureData->getMusicDatas(OVE::MusicData_Repeat);

	for(i=0; i<repeats.size(); ++i){
		OVE::RepeatSymbol* repeatPtr = static_cast<OVE::RepeatSymbol*>(repeats[i]);
		OVE::RepeatType type = repeatPtr->getRepeatType();
		int absTick = mtt_->getTick(measure->no(), repeatPtr->getTick());
		Element* e = 0;

		switch(type) {
		case OVE::Repeat_Segno:{
			Marker* marker = new Marker(score_);
		    marker->setMarkerType(MarkerType::SEGNO);
		    e = marker;
			break;
		}
		case OVE::Repeat_Coda:{
			Marker* marker = new Marker(score_);
		    marker->setMarkerType(MarkerType::CODA);
		    e = marker;
			break;
		}
		case OVE::Repeat_DSAlCoda:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JumpType::DS_AL_CODA);
            e = jp;
			break;
		}
		case OVE::Repeat_DSAlFine:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JumpType::DS_AL_FINE);
            e = jp;
			break;
		}
		case OVE::Repeat_DCAlCoda:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JumpType::DC_AL_CODA);
            e = jp;
			break;
		}
		case OVE::Repeat_DCAlFine:{
            Jump* jp = new Jump(score_);
            jp->setJumpType(JumpType::DC_AL_FINE);
            e = jp;
			break;
		}
		case OVE::Repeat_ToCoda:{
			Marker* m = new Marker(score_);
			m->setMarkerType(MarkerType::TOCODA);
			e = m;
			break;
		}
		case OVE::Repeat_Fine:{
			Marker* m = new Marker(score_);
			m->setMarkerType(MarkerType::FINE);
			e = m;
			break;
		}
		default:
			break;
		}

		if(e != 0){
			e->setTrack(track);
			Segment* s = measure->getSegment(e, absTick);
			s->add(e);
			//measure->add(e);
		}
	}

	QList<OVE::MusicData*> endings = measureData->getCrossMeasureElements(
															OVE::MusicData_Numeric_Ending,
															OVE::MeasureData::PairType_Start);

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
			volta->setVoltaType(VoltaType::CLOSED);
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

	QList<OVE::MusicData*> slurs = measureData->getCrossMeasureElements(OVE::MusicData_Slur, OVE::MeasureData::PairType_Start);

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
	        slur->setSlurDirection(slurPtr->getShowOnTop()? MScore::UP : MScore::DOWN);
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
	case OVE::Dynamics_pppp:{
			dynamic = "pppp";
			break;
		}
	case OVE::Dynamics_ppp:{
			dynamic = "ppp";
			break;
		}
	case OVE::Dynamics_pp:{
			dynamic = "pp";
			break;
		}
	case OVE::Dynamics_p:{
			dynamic = "p";
			break;
		}
	case OVE::Dynamics_mp:{
			dynamic = "mp";
			break;
		}
	case OVE::Dynamics_mf:{
			dynamic = "mf";
			break;
		}
	case OVE::Dynamics_f:{
			dynamic = "f";
			break;
		}
	case OVE::Dynamics_ff:{
			dynamic = "ff";
			break;
		}
	case OVE::Dynamics_fff:{
			dynamic = "fff";
			break;
		}
	case OVE::Dynamics_ffff:{
			dynamic = "ffff";
			break;
		}
	case OVE::Dynamics_sf:{
			dynamic = "sf";
			break;
		}
	case OVE::Dynamics_fz:{
			dynamic = "fz";
			break;
		}
	case OVE::Dynamics_sfz:{
			dynamic = "sfz";
			break;
		}
	case OVE::Dynamics_sffz:{
			dynamic = "sffz";
			break;
		}
	case OVE::Dynamics_fp:{
			dynamic = "fp";
			break;
		}
	case OVE::Dynamics_sfp:{
			dynamic = "sfp";
			break;
		}
	default:
	    break;
	}

	return dynamic;
}

void OveToMScore::convertDynamics(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	QList<OVE::MusicData*> dynamics = measureData->getMusicDatas(OVE::MusicData_Dynamics);

	for(int i=0; i<dynamics.size(); ++i){
		OVE::Dynamics* dynamicPtr = static_cast<OVE::Dynamics*>(dynamics[i]);
		int absTick = mtt_->getTick(measure->no(), dynamicPtr->getTick());
		Dynamic* dynamic = new Dynamic(score_);

		dynamic->setDynamicType(OveDynamics_To_Dynamics(dynamicPtr->getDynamicsType()));
		dynamic->setTrack(track);

		Segment* s = measure->getSegment(Segment::SegChordRest, absTick);
        s->add(dynamic);
	}
}

void OveToMScore::convertExpressions(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	QList<OVE::MusicData*> expressions = measureData->getMusicDatas(OVE::MusicData_Expressions);

	for(int i=0; i<expressions.size(); ++i){
		OVE::Expressions* expressionPtr = static_cast<OVE::Expressions*>(expressions[i]);
		int absTick = mtt_->getTick(measure->no(), expressionPtr->getTick());
		Text* t = new Text(score_);

		t->setTextStyleType(TEXT_STYLE_TECHNIK);
		t->setText(expressionPtr->getText());
		t->setTrack(track);

        Segment* s = measure->getSegment(Segment::SegChordRest, absTick);
        s->add(t);
	}
}

void OveToMScore::convertGlissandos(Measure* measure, int part, int staff, int track){
	OVE::MeasureData* measureData = ove_->getMeasureData(part, staff, measure->no());
	if(measureData == 0)
		return;

	QList<OVE::MusicData*> glissandos = measureData->getCrossMeasureElements(OVE::MusicData_Glissando, OVE::MeasureData::PairType_All);

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

static Hairpin::HairpinType OveWedgeType_To_Type(OVE::WedgeType type) {
	Hairpin::HairpinType subtype = Hairpin::CRESCENDO;
	switch(type) {
	case OVE::Wedge_Cres_Line: {
		subtype = Hairpin::CRESCENDO;
		break;
	}
	case OVE::Wedge_Double_Line: {
		subtype = Hairpin::CRESCENDO;
		break;
	}
	case OVE::Wedge_Decresc_Line: {
		subtype = Hairpin::DECRESCENDO;
		break;
	}
	case OVE::Wedge_Cres: {
		subtype = Hairpin::CRESCENDO;
		break;
	}
	case OVE::Wedge_Decresc: {
		subtype = Hairpin::DECRESCENDO;
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

	QList<OVE::MusicData*> wedges = measureData->getCrossMeasureElements(OVE::MusicData_Wedge, OVE::MeasureData::PairType_All);

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
                hp->setAnchor(Spanner::ANCHOR_SEGMENT);
                score_->addSpanner(hp);
		    score_->updateHairpin(hp);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
Score::FileError importOve(Score* score, const QString& name) {
	OVE::IOVEStreamLoader* oveLoader = OVE::createOveStreamLoader();
	OVE::OveSong oveSong;

	QFile oveFile(name);
	if(!oveFile.exists())
		return Score::FILE_NOT_FOUND;
	if (!oveFile.open(QFile::ReadOnly)) {
		//messageOutString(QString("can't read file!"));
		return Score::FILE_OPEN_ERROR;
	}

	QByteArray buffer = oveFile.readAll();

	oveFile.close();

	oveSong.setTextCodecName(preferences.importCharsetOve);
	oveLoader->setOve(&oveSong);
	oveLoader->setFileStream((unsigned char*) buffer.data(), buffer.size());
	bool result = oveLoader->load();
	oveLoader->release();

	if(result){
		OveToMScore otm;
		otm.convert(&oveSong, score);

//		score->connectSlurs();
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  int tick = m->tick();
                  m->setLen(score->sigmap()->timesig(tick).timesig());
                  m->setTimesig(score->sigmap()->timesig(tick).timesig()); //?
                  }
	}

      return result ? Score::FILE_NO_ERROR : Score::FILE_ERROR;
}

//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportly.cpp 5510 2012-03-30 14:51:09Z wschweer $
//
//  Copyright (C) 2007 Werner Schweer and others
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
//
// Lilypond export.
// For HISTORY, NEWS and TODOS: see end of file (Olav).
//
// Some revisions by olagunde@start.no As I have no prior knowledge of
// C or C++, I have to resort to techniques well known in standard
// Pascal as it was known in 1983, when I got my very short
// programming education in the "Programming for Poets" course.

// I have written a primitive program that takes a full score output
// from exportly and adds separate parts to it:
// http://home.online.no/~olagu2/lyparts.cpp
// It does not work with other lilypond files.

// Olav.


#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "config.h"
#include "libmscore/dynamic.h"
#include "libmscore/element.h"
#include <fstream>
#include "libmscore/glissando.h"
#include "globals.h"
#include <iostream>
using std::cout;
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/repeat.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "libmscore/style.h"
#include "libmscore/sym.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "musescore.h"

namespace Ms {

static  const int MAX_SLURS = 8;
static  const int BRACKSTAVES=64;
static  const int MAXPARTGROUPS = 8;
static const int VERSES = 8;

//---------------------------------------------------------
//   ExportLy
//---------------------------------------------------------

class ExportLy {
  Score* score;
  QFile f;
  QTextStream os;
  int level;        // indent level
  int curTicks;
  MScore::Direction stemDirection;
  int indx;
  bool partial; //length of pickupbar

  int  timedenom, z1, z2, z3, z4; //timesignatures
  int barlen, wholemeasurerest;
  QString wholemeasuretext;
  bool pickup;
  bool rehearsalnumbers;
  bool donefirst; //to prevent doing things in first ordinary bar which are already done in pickupbar
  bool graceswitch, gracebeam;
  int gracecount;
  int prevpitch, staffpitch, chordpitch;
  int measurenumber, lastind, taktnr, staffInd;
  bool repeatactive;
  bool firstalt,secondalt;
  enum voltatype {startending, endending, startrepeat, endrepeat, bothrepeat, doublebar, brokenbar, endbar, none};
  struct  voltareg { voltatype voltart; int barno; };
  struct voltareg  voltarray[255];
  int tupletcount;
  bool pianostaff;
  bool slur;
  const Slur* slurre[MAX_SLURS];
  bool started[MAX_SLURS];
  int phraseslur;
  int slurstack;
  int findSlur(const Slur* s) const;
  const char *relativ, *staffrelativ;
  bool voiceActive[VOICES];
  int prevElTick;
  bool ottvaswitch, jumpswitch;
  char privateRehearsalMark;

  struct lybrackets
  {
    bool piano;
    bool bracestart,brakstart, braceend, brakend;
    int braceno, brakno;
  };

  struct lybrackets lybracks[BRACKSTAVES];
  void bracktest();

  struct staffnameinfo
  {
    QString voicename[VOICES];
    QString  staffid, partname, partshort;
    bool simultaneousvoices;
    int numberofvoices;
  };

  struct staffnameinfo staffname[32];

  QString cleannote, prevnote;

  struct InstructionAnchor
// Even if it is exactly the same thing as "direction" of music-xml,
// the word "instruction" is used in this file, so as not to cause
// confusion with "direction" of the exportxml-file.
  {
    Element* instruct;  // the element containing the instruction
    Element* anchor;    // the element it is attached to
    bool     start;     // whether it is attached to start or end
    int      tick;      // the timestamp
  };

  int nextAnchor;
  struct InstructionAnchor anker;
  struct InstructionAnchor anchors[1024];

  struct glisstablelem
  {
    Chord* chord;
    int tick;
    QString glisstext;
    int type;
  };
  int glisscount;
  struct glisstablelem glisstable[99];

  QString voicebuffer;
  QTextStream out;
  QString scorebuffer;
  QTextStream scorout;
  //  int numberofverses;

  bool nochord;
  int chordcount;
  void chordName(struct InstructionAnchor chordanchor);

  struct chordData
  {
    QString chrName;
    QString extName;
    int alt;
    QString bsnName;
    int bsnAlt;
    int ticklen;
    int tickpos;
  };

  struct chordData thisHarmony;
  struct chordData prevHarmony;
  void resetChordData(struct chordData&);
  QString chord2Name(int ch);

  struct chordPost //element of a list to store hamonies.
  {
    struct chordData cd;
    struct chordPost * next;
    struct chordPost * prev;
  };
  struct chordPost cp;
  struct chordPost * chordHead;
  struct chordPost * chordThis;


  // one lyricsRecord for each staff. Each record have room for VERSES
  // no. of verses.
  struct lyricsData
  {
    QString verselyrics[VERSES];
    QString voicename[VERSES];
    QString staffname;
    int tick[VERSES];
    int segmentnumber[VERSES];
  };

  struct lyricsRecord
  {
    int numberofverses;
    struct lyricsData lyrdat;
    struct lyricsRecord * next;
    struct lyricsRecord *prev;
  };

  //  struct lyricsRecord * lyrrec;
  struct lyricsRecord * thisLyrics;
  struct lyricsRecord * headOfLyrics;
  struct lyricsRecord * tailOfLyrics;


  void storeChord(struct InstructionAnchor chAnk);
  void chordInsertList(chordPost *);
  void printChordList();
  void cleanupChordList();
  void writeFingering (int&, QString fingering[5]);
  void findLyrics();
  void newLyricsRecord();
  void cleanupLyrics();
  void writeLyrics();
  void connectLyricsToStaff();
  void findGraceNotes(Note*,bool&, int);
  void setOctave(int&, int&, int (&foo)[12]);
  bool arpeggioTest(Chord* chord);
  bool glissandotest(Chord*);
  bool findNoteSymbol(Note*, QString &);
  void buildGlissandoList(int strack, int etrack);
  void writeStringInstruction(int &, QString stringarr[10]);
  void findFingerAndStringno(Note* note, int&, int&, QString (&finger)[5], QString (&strng)[10]);
  struct jumpOrMarkerLM
  {
    Marker* marker;
    int measurenum;
    bool start;
  };

  int lastJumpOrMarker;
  struct jumpOrMarkerLM  jumpOrMarkerList[100];

  void writeLilyHeader();
  void writeLilyMacros();
  void writePageFormat();
  void writeScoreTitles();
  void initJumpOrMarkerLMs();
  void resetJumpOrMarkerLM(struct jumpOrMarkerLM &mlm);
  void removeJumpOrMarkerLM(int);
  void preserveJumpOrMarker(Element *, int, bool);
  void printJumpOrMarker(int mnum, bool start);

  void anchortest();
  void voltatest();
  void jumptest();
  void storeAnchor(struct InstructionAnchor);
  void initAnchors();
  void removeAnchor(int);
  void resetAnchor(struct InstructionAnchor &ank);
  bool findMatchInMeasure(int, Staff*, Measure*, int, int, bool);
  bool findMatchInPart(int, Staff*, Score*, int, int, bool);

  void jumpAtMeasureStop(Measure*);
  void findMarkerAtMeasureStart(Measure*);
  void writeMeasuRestNum();
  void writeTremolo(Chord *);

  void writeSymbol(QString);
  void tempoText(TempoText *);
  void words(Text *);
  void hairpin(Hairpin* hp, int tick);
  void ottava(Ottava* ot, int tick);
  void pedal(Pedal* pd, int tick);
  void dynamic(Dynamic*, int);
  void textLine(Element*, int, bool);
  void findTextProperties(Text* , QString&, int &);
  bool textspannerdown;
  // to avoid writing barlinecheck in the middle of a textspanner.
  bool textspanswitch;
  //from exportxml's class directionhandler:
  void buildInstructionListPart(int strack, int etrack);
  void buildInstructionList(Measure* m, int strack, int etrack);
  void handleElement(Element* el);
  void handlePreInstruction(Element * el);
  void instructionJump(Jump*);
  void instructionMarker(Marker*);
  QString primitiveJump(Jump* );
  QString primitiveMarker(Marker*);
  int checkJumpOrMarker(int, bool, Element*&);
  void writeCombinedMarker(int, Element* );
  QString flatInInstrName(QString);

  void indent(); //buffer-string
  void indentF(); //file
  int getLen(int ticks, int* dots);
  void writeLen(int);
  void writeChordLen(int ticks);
  QString tpc2name(int tpc);
  QString tpc2purename(int tpc);

  void writeScore();
  void stemDir(Chord *);
  void writeVoiceMeasure(MeasureBase*, Staff*, int, int);
  void writeKeySig(int);
  void writeTimeSig(TimeSig*);
  void writeClef(int);
  void writeChord(Chord*, bool);
  void writeRest(int, int);
  void findVolta();
  void findStartRepNoBarline(int &i, Measure*);
  void writeBarline(Measure *);
  int  voltaCheckBar(Measure *, int);
  void writeVolta(int, int);
  void findTuplets(ChordRest*);
  void writeArticulation(ChordRest*);
  void writeScoreBlock();
  void checkSlur(Chord*, bool);
  void doSlurStart(Chord*, bool);
  void doSlurStop(Chord*);
  void initBrackets();
  void brackRegister(int, int, int, bool, bool);
  void findBrackets();

public:
  ExportLy(Score* s)
  {
    score  = s;
    level  = 0;
    curTicks = MScore::division;
    slur   = false;
    stemDirection = MScore::AUTO;
  }
  bool write(const QString& name);
};


//---------------------------------------------------------
// abs num value
//---------------------------------------------------------
int numval(int num)
{  if (num <0) return -num;
  return num;
}


//---------------------------------------------------------
// initBrackets -- init array of brackets and braces info
//---------------------------------------------------------

void ExportLy::initBrackets()
{
  for (int i = 0; i < BRACKSTAVES; ++i)     //init bracket-array
    {
      lybracks[i].piano=false;
      lybracks[i].bracestart=false;
      lybracks[i].brakstart=false;
      lybracks[i].braceend=false;
      lybracks[i].brakend=false;
      lybracks[i].braceno=0;
      lybracks[i].brakno=0;
    }
}



//----------------------------------------------------------------
//   brackRegister register where partGroup Start, and whether brace,
//   bracket or pianostaff.
//----------------------------------------------------------------

void ExportLy::brackRegister(int brnumber, int bratype, int staffnr, bool start, bool end)

{
  QString br = "";
  switch(bratype)
    {
    case BRACKET_NORMAL:
      if (start) lybracks[staffnr].brakstart=true;
      if (end) lybracks[staffnr].brakend=true;
      lybracks[staffnr].brakno=brnumber;
      break;
    case BRACKET_BRACE:
      if (start) lybracks[staffnr].bracestart=true;
      if (end) lybracks[staffnr].braceend=true;
      lybracks[staffnr].braceno=brnumber;
      break;
    case -1: //piano-staff: lilypond makes rigid distance between
	     //staffs to allow cross-staff beaming.
      lybracks[staffnr].piano=true;
      if (start) lybracks[staffnr].bracestart=true;
      if (end) lybracks[staffnr].braceend=true;
      lybracks[staffnr].braceno=brnumber;
      break;
    default:
      qDebug("bracket subtype %d not understood\n", bratype);
    }
}


//-------------------------------------------------------------
// findBrackets
// run thru parts and staffs to find start and end of braces and brackets
//---------------------------------------------------------------

void ExportLy::findBrackets()
{
  initBrackets();
  char groupnumber;
  groupnumber=1;
  const QList<Part*>& il = score->parts();  //list of parts

  for (int partnumber = 0; partnumber < il.size(); ++partnumber)  //run thru list of parts
    {
      Part* part = il.at(partnumber);
      if (part->nstaves() == 2) pianostaff=true;
      for (int stavno = 0; stavno < part->nstaves(); stavno++) //run thru list of staves in part.
	{
	  if (pianostaff)
	    {
	      if (stavno==0)
		{
		  brackRegister(groupnumber, -1, partnumber+stavno, true, false);
		}
	      if (stavno==1)
		{
		  brackRegister(groupnumber, -1, partnumber+stavno, false, true);
		  pianostaff=false;
		}
	    }
	  else //not pianostaff
	    {
	      Staff* st = part->staff(stavno);
	      if (st)
		{
		  for (int braclev= 0; braclev < st->bracketLevels(); braclev++) //run thru bracketlevels of staff
		    {
		      if (st->bracket(braclev) != NO_BRACKET) //if bracket
			{
			  groupnumber++;
			  if (groupnumber < MAXPARTGROUPS)
			    { //brackRegister(bracketnumber, brackettype, staffnr, start, end)
			      brackRegister(groupnumber, st->bracket(braclev), partnumber, true, false);
			      brackRegister(groupnumber,st->bracket(braclev), partnumber-1+st->bracketSpan(braclev), false, true);
			    }
			}//end of if bracket
		    }//end of bracket-levels of staff
		}//end if staff
	    } // end of else:not pianostaff
	}//end of stafflist
    }//end of parts-list
}//end of findBrackets;



void ExportLy::bracktest()
      {
      for (int i = 0; i < 10; i++) {
            qDebug("stavnr: %d braceno: %d brackno %d\n", i, lybracks[i].braceno, lybracks[i].brakno);
            }
      }


//-------------------------------------------------------
// instructionJump
//--------------------------------------------------------

void ExportLy::instructionJump(Jump* jp)
{
  JumpType jtp = jp->jumpType();
  QString words = "\n    \\once\\override Score.RehearsalMark #'self-alignment-X = #RIGHT \n      ";

  if (jtp == JumpType::DC)
    words += "\\mark \"Da capo\" ";
  else if (jtp == JumpType::DC_AL_FINE)
    words += "\\DCalfine ";
  else if (jtp == JumpType::DC_AL_CODA)
    words += "\\DCalcoda";
  else if (jtp == JumpType::DS_AL_CODA)
    words += "\\DSalcoda";
  else if (jtp == JumpType::DS_AL_FINE)
    words += "\\DSalfine";
  else if (jtp == JumpType::DS)
    words += "\\mark \\markup{Dal segno \\raise #2 \\halign#-1 \\musicglyph #\"scripts.segno\"}";
  else
    qDebug("jump type=%d not implemented\n", jtp);
  out <<  words << " ";
}



//---------------------------------------------------------
//   instructionMarker -- write marker
//---------------------------------------------------------

void ExportLy::instructionMarker(Marker* m)
{
  MarkerType mtp = m->markerType();
  QString words = "";
  if (mtp == MarkerType::CODA)
      words = "\\theCoda ";
  else if (mtp == MarkerType::CODETTA)
     	words = "\\codetta";
  else if (mtp == MarkerType::SEGNO)
      words = "\\thesegno";
  else if (mtp == MarkerType::FINE)
	words = "\\fine";
  else if (mtp == MarkerType::TOCODA)
	words = "\\gotocoda ";
  else if (mtp == MarkerType::VARCODA)
	words = "\\varcodasign ";
  else if (mtp == MarkerType::USER)
	qDebug("unknown user marker\n");
  else
    qDebug("marker type=%d not implemented\n", mtp);

  out <<  words << " ";

}



//---------------------------------------------------------------------
// primitiveJump -- write jumpsign without macros: to be combined with
// rehearsalmark
//---------------------------------------------------------------------

QString ExportLy::primitiveJump(Jump* jp)
{
  JumpType jtp = jp->jumpType();
  QString words = "";

  cout << "primitivejump\n";

  if (jtp == JumpType::DC)
     	words = "Da capo";
  else if (jtp == JumpType::DC_AL_FINE)
      words = "D.C. al fine";
  else if (jtp == JumpType::DC_AL_CODA)
	words = "D.C. al coda";
  else if (jtp == JumpType::DS_AL_CODA)
	words = "D.S. al coda";
  else if (jtp == JumpType::DS_AL_FINE)
	words = "D.S. al fine";
  else if (jtp == JumpType::DS)
      words = "Dal segnoX \\musicglyph #\"scripts.segno\"";
  else
    qDebug("jump type=%d not implemented\n", jtp);
  return  words;
}

//------------------------------------------------------------------
//   primitiveMarker -- write marker without macros: to be combined
//   with rehearsalmark.
//------------------------------------------------------------------

QString ExportLy::primitiveMarker(Marker* m)
{
  MarkerType mtp = m->markerType();
  QString words = "";
  if (mtp == MarkerType::CODA) //the coda
    //    words = "\\line{\\halign #-0.75\\noBreak \\codaspace \\resumeStaff \\showClefKey \musicglyph #\"scripts.coda\" \\musicglyph #\"scripts.coda\"}";
    words = "\\line{\\halign #-0.75 \\musicglyph #\"scripts.coda\" \\musicglyph #\"scripts.coda\"}";
  else if (mtp == MarkerType::CODETTA)
     	words = "\\line {\\musicglyph #\"scripts.coda\" \\hspace #-1.3 \\musicglyph #\"scripts.coda\"} } \n";
  else if (mtp == MarkerType::SEGNO)
      words = "\\musicglyph #\"scripts.segno\"";
  else if (mtp == MarkerType::FINE)
	words =  "{\"Fine\"} \\mark \\markup {\\musicglyph #\"scripts.ufermata\" } \\bar \"\bar \"||\" } \n";
  else if (mtp == MarkerType::TOCODA)
	words = "\\musicglyph #\"scripts.coda\"";
  else if (mtp == MarkerType::VARCODA)
    words = "\\musicglyph#\"scripts.varcoda\"";
  else if (mtp == MarkerType::USER)
	qDebug("unknown user marker\n");
  else
    qDebug("marker type=%d not implemented\n", mtp);
  return words;
}



//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

void ExportLy::writeSymbol(QString name)
{
 //  QString name = symbols[sym->sym()].name();
  //this needs rewriting probably because of some rewriting of sym.cpp/sym.h
  //  cout << "symbolname: " << name.toLatin1().data() << "\n";

  if (wholemeasurerest > 0) writeMeasuRestNum();

  if (name == "clef eight")
      out << "^\\markup \\tiny\\roman 8 ";
  else if (name == "pedal ped")
    out << " \\sustainOn ";
  else if (name == "pedalasterisk")
    out << " \\sustainOff ";
  else if (name == "scripts.trill")
    out << "\\trill ";
  else if (name == "scripts.flageolet")
    out << "\\flageolet ";
  else if (name == "rcomma")
    out << "\\mark \\markup {\\musicglyph #\"scripts.rcomma\"} ";
  else if (name == "lcomma")
    out << "\\mark \\markup {\\musicglyph #\"scripts.lcomma\"} ";
  else
    out << "^\\markup{\\musicglyph #\"" << name << "\"} ";
  // else if (name == "acc discant")
  //   out << "^\\markup{\\musicglyph #\"accordion.accDiscant\"} ";
  // else if (name == "acc dot")
  //   //we need to place the dot on the correct place within the discant
  //   //and other base symbols. Is this possible in mscore? The entire
  //   //example in lily manual "2.2.3 Accordion" must be input as a
  //   //macro?
  //   out << "^\\markup{\\musicglyph #\"accordion.accDot\"} ";
  // else if (name == "acc freebase")
  //   out << "^\\markup{\\musicglyph #\"accordion.accFreebase\"} ";
  // else if (name == "acc stdbase")
  //   out << "^\\markup{\\musicglyph #\"accordion.accStdbase\"} ";
  // else if (name == "acc bayanbase")
  //   out << "^\\markup{\\musicglyph #\"accordion.accBayanbase\"} ";
  // else if (name == "acc old ee")
  //   out << "^\\markup{\\musicglyph #\"accordion.accOldEE\"} ";
  // else
  //   {
  //   qDebug("ExportLy::symbol(): %s not supported\n", name.toLatin1().data());
  //   return;
  //   }
}


void ExportLy::resetChordData(struct chordData &CD)
{
  CD.chrName="";
  CD.extName="";
  CD.alt=0;
  CD.bsnName="";
  CD.tickpos=0;
  CD.bsnAlt=0;
  CD.ticklen=0;
}


void ExportLy::cleanupChordList()
{
  chordPost * next;
  chordThis = chordHead;
  if (chordThis == 0)
        return;
  next = chordThis->next;

  while (next !=NULL)
    {
      next->prev = NULL;
      delete chordThis;
      chordThis = next;
      next = next->next;
    }
  delete chordThis;
}


void ExportLy::writeChordLen(int ticks)
{
  int dots = 0;
  int len = getLen(ticks, &dots);

  switch (len)
    {
    case -5:
      os << "1*5/4"; // 5/4
      break;
    case -4:
      os << "2*5 "; // 5/2 ??
      break;
    case -3:
      os << "1.*2 ";
      break;
    case -2://longa 16/4
      os << "1*4 ";
      break;
    case -1: //brevis 8/4
      os << "1*2";
      break;
    default:
      os << len;
      for (int i = 0; i < dots; ++i)
	os << ".";
      break;
    }
}

void ExportLy::printChordList()
{
  chordThis = chordHead;
  if (chordThis == 0)   // ws
      return;

  struct chordPost * next;
  next = chordThis->next;
  int i=0;
  int dots=0;
  int lilylen=0;

  while (next != NULL)
    {
      i++;

      //insert spacer rests before first chord:
      if ((i==1) and (chordThis->cd.tickpos > 0))
	{
	  int factor=1;
	  // works at least if denominator is 4:
	  if (timedenom == 2) factor=2; else factor = 1;
	  int measnum = chordThis->cd.tickpos / (z1 * MScore::division * factor);
	  qDebug("Measnum chord: \n");
	  int surplus = chordThis->cd.tickpos % MScore::division;
	  if (measnum == 0) surplus = chordThis->cd.tickpos;
	  level++;
	  indentF();
	  if (surplus > 0)
	    {
	      lilylen= getLen(surplus, &dots);
	      level++;
	      indentF();
	      os << "s" << lilylen;
	      while (dots>0)
		{
		  os<< ".";
		  dots--;
		}
	      os << " ";
	    }
	  if (measnum > 0 ) os << "s1*" << measnum<< " \n";
	}// end if firstone is not on tick 0 print spacer rest.
      else
	{
	  //compute ticklen for the rest of the chords:
	  chordThis->cd.ticklen =  next->cd.tickpos - chordThis->cd.tickpos;
	  chordThis=next;
	  next=next->next;
	}
    }//while not end of list.

  if (next == NULL)
     chordThis->cd.ticklen= 480;


  chordThis = chordHead;
  next = chordThis;
  //  i=0;

  indentF();


  while (next != NULL)
    {
      next=next->next;
      dots=0;
      lilylen=0;
      i++;
      //      lilylen = getLen(chordThis->cd.ticklen, &dots);
      os << chordThis->cd.chrName;
      curTicks=0;

      writeChordLen(chordThis->cd.ticklen); //<< lilylen;

      while (dots > 0)
	{
	  os << ".";
	  dots--;
	}

      if (chordThis->cd.extName !="")
	os << ":" << chordThis->cd.extName;

      if (chordThis->cd.bsnName !="")
	os << "/" << chordThis->cd.bsnName;
      if (chordThis->cd.bsnAlt > 0)
	os << chordThis->cd.bsnAlt;
      os << " ";
      chordThis=next;
    }//end of while chordthis...
  os << "}%%end of chordlist \n\n";
}//end of printChordList



//-----------------------------------------------------------
// chord2Name
//-----------------------------------------------------------
QString ExportLy::chord2Name(int ch)
      {
      const char names[] = "fcgdaeb";
      return QString(names[(ch + 1) % 7]);
      }


//----------------------------------------------------------
// chordInsertList
//----------------------------------------------------------
void ExportLy::chordInsertList(chordPost * newchord)
{

  if (chordHead == NULL) //first element: make head of list.
    {
      chordcount++;
      chordHead = newchord;
      newchord->prev = NULL;
      newchord->next = NULL;
    }
  else //at least one previous existent element
    {
      chordcount++;
      chordThis = chordHead;
      while ((newchord->cd.tickpos >= chordThis->cd.tickpos) && (chordThis->next != NULL))
	{
	  chordThis = chordThis->next;
	}
      if ((chordThis->next == NULL) && (chordThis->cd.tickpos <= newchord->cd.tickpos)) //we have reached end of list
	{
	  //insert new element as tail
	  chordThis->next = newchord;
	  newchord->prev = chordThis;
	}
      else
	//insert somewhere in the middle
	{
	  newchord->next = chordThis;
	  newchord->prev = chordThis->prev;
	  if (chordHead != chordThis)
	    {
	      chordThis = chordThis->prev;
	      chordThis->next = newchord;
	    }
	  else // the middle is immediately after head and before the tail.
	    {
	      chordThis->prev = newchord;
	      chordHead = newchord;
	    }
	}//middle
    }//at least one previous
}//end of chordInsertList

//-----------------------------------------------------------------
// storeChord
//-----------------------------------------------------------------
void ExportLy::storeChord(struct InstructionAnchor chordanchor)
{
  cout << "chords!!!\n";
  //first create new element
  chordPost * aux;
  aux = new chordPost();
  resetChordData(aux->cd);
  aux->next = NULL;
  aux->prev = NULL;

  //then fill it
  Harmony* harmelm = (Harmony*) chordanchor.instruct;
  int  chordroot = harmelm->rootTpc();
  QString n, app;

  if (chordroot != INVALID_TPC)
    {
      if (nochord == true) nochord = false;
      aux->cd.chrName = chord2Name(chordroot);
      n=thisHarmony.chrName;

      aux->cd.tickpos = harmelm->parent()->type() == Element::SEGMENT
         ? static_cast<Segment*>(harmelm->parent())->tick() : 0;

      if (!harmelm->xmlKind().isEmpty())
	{
	  aux->cd.extName = harmelm->extensionName();
	  aux->cd.extName = aux->cd.extName.toLower();
	}

      int alter = tpc2alter(chordroot);
      if (alter==1) app = "is";
      else
	{
	  if (alter == -1)
	    {
	      if (n == "e") app = "s";
	      else app = "es";
	    }
	}
      aux->cd.chrName = aux->cd.chrName + app;

      int  bassnote = harmelm->baseTpc();
      if (bassnote != INVALID_TPC)
	{
	  aux->cd.bsnName = chord2Name(bassnote);
	  int alter = tpc2alter(bassnote);
	  n=aux->cd.bsnName;

	  if (alter==1) app = "is";
	  else if (alter == -1)
	  {
	    if (n=="e")  app =  "s"; else app = "es";
	  }

	  aux->cd.bsnName = n + app;
	  aux->cd.bsnAlt=alter;
	} //end if bassnote
      //and at last insert it in list:
      chordInsertList(aux);
    }//end if chordroot
  else
    storeAnchor(anker);
}


//---------------------------------------------------------
//   tempoText
//---------------------------------------------------------

void ExportLy::tempoText(TempoText* text)
      {
	QString temptekst = text->text();
	double met = text->tempo();
	int metronome;
	metronome = (int) (met * 60);
	out << "\\tempo \""  << text->text() << "\" " <<  timedenom << " = " << metronome << "  ";
      }



//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportLy::words(Text* text)
     {
       QString style;
       int size;
       findTextProperties(text,style,size);
       //todo: find exact mscore-position of text and not only anchorpoint, and position accordingly in lily.
//TODO     if ((text->subtypeName() != "RehearsalMark"))
       // if (text->text() != "")
       out << "^\\markup {" << style<< " \"" << text->text() << "\"} ";
     //     qDebug("tekst %s\n", tekst.toLatin1().data());
      }



//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportLy::hairpin(Hairpin* hp, int tick)
{ // print hairpin from anchorlist
  // todo: find exact mscore-position of
  // hairpin start and end and not only anchorpoint, and position
  // accordingly in lily.
	int art=2;
	art=hp->hairpinType();
	if (hp->tick() == tick)
	  {
	    if (art == 0) //diminuendo
	      out << "\\< ";
	    if (art == 1) //crescendo
	      out << "\\> ";
	    if (art > 1 ) out << "\\!x ";
	  }
//TODO-WS       if (hp->tick2() == tick) out << "\\! "; //end of hairpin
      }

//---------------------------------------------------------
//  start ottava
//---------------------------------------------------------

void ExportLy::ottava(Ottava* ot, int tick)
{
  int st = ot->ottavaType();
  if (ot->tick() == tick)
    {
      switch(st) {
      case 0:
	out << "\\ottva ";
	break;
      case 1:
	out << "\\ottva \\once\\override TextSpanner #'(bound-details left text) = \"15va\" \n";
	indent();
	break;
      case 2:
	out << "\\ottvabassa ";
	break;
      case 3:
	out << "\\ottvabassa \\once \\override TextSpanner #'(bound-details left text) = \"15vb\"  \n";
	indent();
	break;
      default:
	qDebug("ottava subtype %d not understood\n", st);
      }
    }
  else {
     	  out << "\\ottvaend ";
        }
}


//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportLy::pedal(Pedal* pd, int tick)
      {
      if (pd->tick() == tick)
	out << "\\sustainOn ";
      else
	out << "\\sustainOff ";
      }



//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------
void ExportLy::dynamic(Dynamic* dyn, int nop)
{
  QString t = dyn->text();
  if (t == "p" || t == "pp" || t == "ppp" || t == "pppp" || t == "ppppp" || t == "pppppp"
      || t == "f" ||
      t == "ff" || t == "fff" || t == "ffff" || t == "fffff" || t == "ffffff"
      || t == "mp" || t == "mf" || t == "sf" || t == "sfp" || t == "sfpp" || t == "fp"
      || t == "rf" || t == "rfz" || t == "sfz" || t == "sffz" || t == "fz" || t == "sff")
    {
	switch(nop)
	    {
	    case 0:
		out << "\\" << t << " ";
		break;
	    case 1:
		out << "_\\markup\{\\dynamic " << t.toLatin1().data() << " \\halign #-2 ";
		break;
	    case 2:
		out <<  " \\dynamic " << t.toLatin1().data() << " } ";
		break;
	    default:
		out << "\\" << t.toLatin1().data() << " ";
		break;
	    }

    }
  else if (t == "m" || t == "z")
    {
      out << "\\"<< t.toLatin1().data() << " ";
    }
    else
      out << "_\\markup{\""<< t.toLatin1().data() << "\"} ";
}//end dynamic


//-----------------------------------------------------------------------------------
// findTextProperties
//-----------------------------------------------------------------------------------
void ExportLy::findTextProperties(Text* tekst, QString &tekststyle, int &fontsize)
{
  QFont fontprops=tekst->font();
  fontsize= fontprops.pointSizeF();
  switch (fontprops.style())
    {
    case QFont::StyleNormal :
      tekststyle = "\\upright ";
      break;
    case QFont::StyleItalic :
    case QFont::StyleOblique:
      tekststyle = "\\italic";
      break;
    default :
      tekststyle = "\\upright ";
      break;
    }
  switch (fontprops.weight())
    {
    case QFont::Light:
    case QFont::Normal:
      break;
    case QFont::DemiBold:
    case QFont::Bold:
    case QFont::Black:
      tekststyle += "\\bold ";
      break;
    default:
      break;
    }
}//end findTextProperties

//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportLy::textLine(Element* instruction, int tick, bool pre)
{
  qDebug("textline\n");
  QString rest;
  QPointF point;
  QString lineEnd = "none";
  QString type;
  //  int lineoffset;
  QString lineType;
//  SLine* sl = (SLine*) instruction;
  int fontsize=0;
  TextLine* tekstlinje = (TextLine *) instruction;
  bool post = false;
  if (pre == false) post = true;

  //start of line:
  if (tekstlinje->tick() == tick)
    {
      if (pre)
	{
	  switch (tekstlinje->lineStyle())
	    {
	    case Qt::DashDotLine:
	    case Qt::DashDotDotLine:
	    case Qt::DashLine:
	      out << " \\once\\override TextSpanner  #'style = #'dashed-line \n";
	      indent();
	      break;
	    case Qt::DotLine:
	      out << " \\once\\override TextSpanner  #'style = #'dotted-line \n";
	      indent();
	      break;
	    default:
	      break;
	    }
	  if (tekstlinje->endHook())
	    {
	      double h = tekstlinje->endHookHeight().val();
	      if (h < 0.0)
		{
		  out << "\\once\\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . 1) }\n";
		  indent();
		}
	      else
		{
		  out << "\\once\\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . -1) }\n";
		  indent();
		}
	    }
	  if (tekstlinje->beginText())
	    {
	      QString linetext = tekstlinje->beginText()->text();
	      Text* tekst = (Text*) tekstlinje->beginText();
	      QString tekststyle = "";
	      findTextProperties(tekst, tekststyle, fontsize);
	      out << "\\once\\override TextSpanner #'(bound-details left text) = \\markup{";
	      out << tekststyle<< "\"" << linetext <<"\"} \n";
	      indent();
	    }
	  point = tekstlinje->frontSegment()->userOff();
	  if (point.y() > 0.0) //below
	    {
	      out <<"\\textSpannerDown ";
	      textspannerdown=true;
	    }
	  else if (textspannerdown)
	    {
	      out << "\\textSpannerNeutral ";
	      textspannerdown = false;
	    }
	}// end if pre

      else if (post) //after note, start of line.
	{
	  out << "\\startTextSpan ";
	  textspanswitch=true;
	}// end if post: after note, start of textline
    }//end of start-of-textline
#if 0 // TODO-WS
  else if  (sl->tick2() == tick)  //at end of textline.
    {
      if (pre)
	{
	  out << "\\stopTextSpan ";
	  textspanswitch=false;
	   // from exportxml.cpp: output of user offset from anchor:
	  // userOff2 is relative to userOff in MuseScore
	  //            point = tekstlinje->lineSegments().last()->userOff2() + tekstlinje->lineSegments().first()->userOff();
	  //            lineoffset = tekstlinje->mxmlOff2();
	}
      else if (post)
	{
	  //just relax for the moment.
	}
    }// end if tick2()
#endif
}// end of textLine()


//---------------------------------------------------------------------
// anchortest
//---------------------------------------------------------------------
void ExportLy::anchortest()
{
      int i;
      for (i=0; i<nextAnchor ; i++)
	{
	  Element * instruction = anchors[i].instruct;
	  Element::ElementType instructiontype = instruction ->type();
//	  Text* text = (Text*) instruction;
	  qDebug("anker nr: %d ", i);
	  switch(instructiontype)
	    {
	    case Element::STAFF_TEXT:
	      qDebug("STAFF_TEXT ");
//TODO	      if (text->subtypeName()== "RehearsalMark") qDebug(" rehearsal STAFF ");
	      qDebug("\n");
	      break;
	    case Element::TEXT:
	      qDebug("TEXT ");
//	      if (text->subtypeName()== "RehearsalMark") qDebug(" rehearsal MEASURE");
	      qDebug("\n");
	      break;
	    case Element::MARKER:
	      qDebug("MARKER\n");
	      break;
	    case Element::JUMP:
	      qDebug("JUMP\n");
	      break;
	    case Element::SYMBOL:
	      qDebug("SYMBOL\n");
	      break;
	    case Element::TEMPO_TEXT:
	      qDebug("TEMPOTEXT MEASURE\n");
	      break;
	    case Element::DYNAMIC:
	      qDebug("Dynamic\n");
	      break;
	    case Element::HARMONY:
	      qDebug("akkordnavn. \n");
	      break;
	    case Element::HAIRPIN:
	      qDebug("hairpin \n");
	      break;
	    case Element::PEDAL:
	      qDebug("pedal\n");
	      break;
	    case Element::TEXTLINE:
	      qDebug("textline\n");
	      break;
	    case Element::OTTAVA:
	      qDebug("ottava\n");
	      break;
	    default: break;
	    }
	}
      qDebug("Anchortest finished\n");
}//end anchortest





//---------------------------------------------------------------------
// jumptest
//---------------------------------------------------------------------
void ExportLy::jumptest()
{
  qDebug("at jumptest A lastjump %d\n", lastJumpOrMarker);
      int i;
      for (i=0; i<lastJumpOrMarker; i++)
	{
	  qDebug("jumptest 1\n");
	  Element * merke = jumpOrMarkerList[i].marker;
	  qDebug("jumptest 2\n");
	  Element::ElementType instructiontype = merke->type();
	  qDebug("jumptest 3\n");
//	  Text* text = (Text*) merke;
	  qDebug("jumptest 4\n");
	  qDebug("marker nr: %d ", i);
	  switch(instructiontype)
	    {
	    case Element::STAFF_TEXT:
	      qDebug("STAFF_TEXT ");
//	      if (text->subtypeName()== "RehearsalMark") qDebug(" rehearsal ");
	      qDebug("\n");
	      break;
	    case Element::TEXT:
	      qDebug("TEXT ");
//	      if (text->subtypeName()== "RehearsalMark") qDebug(" rehearsal ");
	      qDebug("\n");
	      break;
	    case Element::MARKER:
	      qDebug("MARKER\n");
	      break;
	    case Element::JUMP:
	      qDebug("JUMP\n");
	      break;
	    case Element::SYMBOL:
	      qDebug("SYMBOL\n");
	      break;
	    case Element::TEMPO_TEXT:
	      qDebug("TEMPOTEXT MEASURE\n");
	      break;
	    case Element::DYNAMIC:
	      qDebug("Dynamic\n");
	      break;
	    case Element::HARMONY:
	      qDebug("akkordnavn. \n");
	      break;
	    case Element::HAIRPIN:
	      qDebug("hairpin \n");
	      break;
	    case Element::PEDAL:
	      qDebug("pedal\n");
	      break;
	    case Element::TEXTLINE:
	      qDebug("textline\n");
	      break;
	    case Element::OTTAVA:
	      qDebug("ottava\n");
	      break;
	    default:
	      break;
	    }
	}
}//end jumptest



//--------------------------------------------------------
//  initAnchors
//--------------------------------------------------------
void ExportLy::initAnchors()
{
  int i;
  for (i=0; i<1024; i++)
    resetAnchor(anchors[i]);
}



//--------------------------------------------------------
//   resetAnchor
//--------------------------------------------------------
void ExportLy::resetAnchor(struct InstructionAnchor &ank)
{
  ank.instruct=0;
  ank.anchor=0;
  ank.start=false;
  ank.tick=0;
}

//---------------------------------------------------------
//   deleteAnchor
//---------------------------------------------------------
void ExportLy::removeAnchor(int ankind)
{
  int i;
  resetAnchor(anchors[ankind]);
  for (i=ankind; i<=nextAnchor; i++)
    anchors[i]=anchors[i+1];
  resetAnchor(anchors[nextAnchor]);
  nextAnchor=nextAnchor-1;
}

//---------------------------------------------------------
//   storeAnchor
//---------------------------------------------------------

void ExportLy::storeAnchor(struct InstructionAnchor a)
      {
	if (nextAnchor < 1024)
	  {
	    anchors[nextAnchor++] = a;
	  }
	else
	  qDebug("InstructionHandler: too many instructions\n");
	resetAnchor(anker);
      }

void ExportLy::writeCombinedMarker(int foundJoM, Element* elm)
{
  out << "\\mark\\markup\\column \{ \n";
  level++;
  indent();
  if (foundJoM == Element::MARKER)
    {
      QString primark = primitiveMarker((Marker*)elm);
      out << primark << "\n";
    }
  if (foundJoM == Element::JUMP)
    out << primitiveJump((Jump*) elm) << "\n";
  indent();
  out << "\\box\\bold \"";
  if (rehearsalnumbers)
    out << (int)privateRehearsalMark; //+n
  else
    out << privateRehearsalMark << "\" \n";
  indent();
  level--;
  indent();
  out << "} \n";
  indent();
  out << "\\set Score.rehearsalMark = #" << (int)privateRehearsalMark-63 << "\n";
  indent();
}




//-----------------------------------------------------------------
// handlePreInstruction -- handle the instructions attached to one
// specific element and which are to be exported BEFORE the element
// itself.
// -----------------------------------------------------------------

void ExportLy::handlePreInstruction(Element * el)
{
  int i = 0;
  int foundJoM = 0;
  Text* tekst;
  for (i = 0; i <= nextAnchor; i++) //run thru anchorlist
    {
      if  ((anchors[i].anchor != 0) && (anchors[i].anchor == el))
	{
	  Element * instruction = anchors[i].instruct;
	  Element::ElementType instructiontype = instruction->type();

	  switch(instructiontype)
	    {
	    case Element::STAFF_TEXT:
	    case Element::REHEARSAL_MARK:
	      {
		    tekst = (Text*) instruction;
		    if (wholemeasurerest >=1) writeMeasuRestNum();
		    bool ok = false;
		    // int dec=0;
		    QString c;
		    c=tekst->text();
		    // dec = c.toInt(&ok, 10);
		    if (ok) rehearsalnumbers=true;
		    Element* elm = 0;
    		    foundJoM = checkJumpOrMarker(measurenumber, true, elm); //true means at the start of measure.
		      if (foundJoM)
			writeCombinedMarker(foundJoM,elm);
		      else
		        out << "\\mark\\default ";//xxx
		      privateRehearsalMark++;
		      removeAnchor(i); //to use this caused trouble at another place. Maybe remove?
		break;
	      }
	    case Element::OTTAVA:
	      if (wholemeasurerest >=1) writeMeasuRestNum();
	      ottvaswitch=true;
	      ottava((Ottava*) instruction, anchors[i].tick);
	      removeAnchor(i);
	      break;
	    case Element::TEMPO_TEXT:
	      tempoText((TempoText*) instruction);
	      removeAnchor(i);
	      break;
	    case Element::TEXTLINE:
	      textLine(instruction, anchors[i].tick, true);
	      break;
	    default: break;
	    }//end switch
	}//end if anchors
    }//end for (i...)
}//End of handlePreInstructiion



//---------------------------------------------------------
//   handleElement -- handle all instructions attached to one specific
//   element and which are to be exported AFTER the element itself.
//---------------------------------------------------------

void ExportLy::handleElement(Element* el)
{
  int i = 0;
  Symbol * sym;
  QString name;
  for (i = 0; i<=nextAnchor; i++)//run thru filled part of list
    {
	if (anchors[i].anchor != 0 and anchors[i].anchor==el) // if anchored to this element
	    {
		Element* instruction = anchors[i].instruct;
		Element::ElementType instructiontype = instruction->type();

		switch(instructiontype)
		    {
		    case Element::MARKER:
			qDebug("MARKER\n");
			instructionMarker((Marker*) instruction);
			break;
		    case Element::JUMP:
			qDebug("JUMP\n");
			instructionJump((Jump*) instruction);
			break;
		    case Element::SYMBOL:
			{
			    cout << "symbol in anchorlist tick: " << anchors[i].tick << "  \n";
			    sym = (Symbol*) instruction;
			    name = Sym::id2name(sym->sym());
			    writeSymbol(name);
			    break;
			}
		    case Element::TEMPO_TEXT:
			//   qDebug("TEMPOTEXT MEASURE\n");
			//   tempoText((TempoText*) instruction);
			break;
		    case Element::STAFF_TEXT:
		    case Element::TEXT:
			cout << "anchored text \n";
			if (wholemeasurerest)
			    {
				Text* wmtx = (Text*) instruction;
				wholemeasuretext = wmtx->text();
			    }
			else
			    words((Text*) instruction);
			break;
		    case Element::DYNAMIC:
			{
			    int nextorprev=0;

			    if ((anchors[i+1].anchor != 0) and (anchors[i+1].anchor==el))
				{

				    Element* nextinstruct = anchors[i+1].instruct;
				    Element::ElementType nextinstrtype = nextinstruct->type();
				    if (nextinstrtype == Element::DYNAMIC)
				    nextorprev = 1;
				}
			    else if ((anchors[i-1].anchor != 0) and (anchors[i-1].anchor==el))
				{
				    Element* previnstruct = anchors[i-1].instruct;
				    Element::ElementType previnstrtype = previnstruct->type();
				    if (previnstrtype == Element::DYNAMIC)
					nextorprev=2;
				}
			    dynamic((Dynamic*) instruction, nextorprev);
			    break;
			}
		    case Element::HAIRPIN:
			hairpin((Hairpin*) instruction, anchors[i].tick);
			break;
		    case Element::HARMONY:
			words((Text*) instruction);
			break;
		    case Element::PEDAL:
			pedal((Pedal*) instruction, anchors[i].tick);
			break;
		    case Element::TEXTLINE:
			textLine(instruction, anchors[i].tick, false);
			break;
		    case Element::OTTAVA:
			break;
		    default:
			qDebug("post-InstructionHandler::handleElement: direction type %s at tick %d not implemented\n",
			       Element::name(instruction->type()), anchors[i].tick);
			break;
		    }
		//	  removeAnchor(i);
	    }
    } //foreach position i anchor-array.
}




//--------------------------------------------------------
//   resetMarkerLM
//--------------------------------------------------------
void ExportLy::resetJumpOrMarkerLM(struct jumpOrMarkerLM &mlm)
{
  mlm.marker=0;
  mlm.measurenum=0;
  mlm.start=false;
}

//--------------------------------------------------------
//  initMarkerLMs
//--------------------------------------------------------
void ExportLy::initJumpOrMarkerLMs()
{
  int i;
  for (i=0; i<100; i++)
    resetJumpOrMarkerLM(jumpOrMarkerList[i]);
}

//---------------------------------------------------------
//   removeMarkerLM -- not used ?!
//---------------------------------------------------------
void ExportLy::removeJumpOrMarkerLM(int markerind)
{
  int i;
  resetJumpOrMarkerLM(jumpOrMarkerList[markerind]);
  for (i=markerind; i<=nextAnchor; i++)
    jumpOrMarkerList[i]=jumpOrMarkerList[i+1];
  resetJumpOrMarkerLM(jumpOrMarkerList[lastJumpOrMarker]);
  lastJumpOrMarker=lastJumpOrMarker-1;
}


//---------------------------------------------------------------------
// preserveJumpOrMark
//---------------------------------------------------------------------

void ExportLy::preserveJumpOrMarker(Element* dir, int mnum, bool start)
{
  jumpswitch=true;
  jumpOrMarkerLM mlm;
  Marker* ma = (Marker*) dir;
  mlm.marker = ma;
  mlm.measurenum = mnum;
  mlm.start = start;
  if (lastJumpOrMarker < 100)
    {
      lastJumpOrMarker++;
      jumpOrMarkerList[lastJumpOrMarker] = mlm;
    }
  else
    qDebug("PreserveMarker: Too many marksorjumps\n");
}



//--------------------------------------------------------------------
// checkJumpOrMarker
//---------------------------------------------------------------------
int ExportLy::checkJumpOrMarker(int mnum, bool start, Element* &moj)
{
  cout << "checkjumpormarker\n";

  int tp=0;
  int i=0;

  if (start) mnum--; //we place these things at the end of the previous measure
  cout << "mnum: " << mnum << "\n";

  while (jumpOrMarkerList[i].measurenum < mnum)
    {
      ++i;
      if (jumpOrMarkerList[i].measurenum ==0 )
	goto endofcheck;
    }

  while ((jumpOrMarkerList[i].measurenum == mnum) and (i < 100))
    {
      cout << "found measure  " << jumpOrMarkerList[i].start << "\n";
      if (jumpOrMarkerList[i].start == true)
	{
	  moj = jumpOrMarkerList[i].marker;
	  tp = moj->type();
	  cout << "moj->type: " << tp << "\n";
    	}
      i++;
      cout << i << "\n";
      // if (i >= 100)
      // 	break;
    }
 endofcheck:
  cout << "checkjumpormarker, type: " << tp << "\n";
  return tp;
}


//--------------------------------------------------------------------
// printJumpOrMarker
//---------------------------------------------------------------------
void ExportLy::printJumpOrMarker(int mnum, bool start)
{
  cout << "printjumpormarker 1\n";

  int i=0;
  while (jumpOrMarkerList[i].measurenum < mnum)
    i++;

  cout << "test 2\n";

  while ((jumpOrMarkerList[i].measurenum == mnum) and (i < 100))
    {
      cout << "test 3\n";

      if (jumpOrMarkerList[i].start == start)
	{
	  cout << "test 4\n";

	  Element* moj = jumpOrMarkerList[i].marker;
	  int tp = moj->type();
	  if (tp == Element::MARKER)
	    {
	      cout << "test 5\n";
	      Marker* ma = (Marker*) moj;
	      instructionMarker(ma);
	    }
	  else if (tp ==Element::JUMP)
	    {
	      cout << "test 6\n";
	      Jump* jp = (Jump*) moj;
	      instructionJump(jp);
    	    }
    	  cout << "test 7\n";
    	}
      i++;
      cout << i << "\n";
      // if (i >= 100)
      // 	break;
    }
  cout << "test 8\n";
}



//---------------------------------------------------------------------
// findMarkerAtMeasureStart
//---------------------------------------------------------------------


void ExportLy::findMarkerAtMeasureStart(Measure* m)
{
   for (auto ci = m->el()->begin(); ci != m->el()->end(); ++ci)
     {
       Element* dir = *ci;
       int tp = dir->type();
       if (tp == Element::MARKER)
  	 { //only markers, not jumps, are used at measure start.
	   Marker* ma = (Marker*) dir;
	   MarkerType mtp = ma->markerType();
	   //discard markers which belong at measure end:
	   if (!(mtp == MarkerType::FINE || mtp == MarkerType::TOCODA))
	     {
	       cout << "marker found at measure: " << measurenumber << "\n";
	       //	       instructionMarker(ma);
	       preserveJumpOrMarker(dir, measurenumber, true); //true means start of measure
	     }
  	 }
     }
}

//---------------------------------------------------------
//  jumpAtMeasureStop -- write jumps at end of measure
//---------------------------------------------------------

void ExportLy::jumpAtMeasureStop(Measure* m)
      {
	// loop over all measure relative elements in this measure
	// looking for JUMPS and MARKERS
	for (auto ci = m->el()->begin(); ci != m->el()->end(); ++ci)
	  {
	    Element* dir = *ci;
	    int tp = dir->type();
	    bool end; // start;
	    // start=true;
	    end=false;

	    if (tp == Element::JUMP)
	      {
		// all jumps are handled at measure end
		Jump* jp = (Jump*) dir;
		//writing the jump-mark in part one of the score:
		instructionJump(jp);
		// in mscore jumps and markers are found only in the
		// first staff. If it shall be possible to extract
		// parts from the exported lilypond-score, jumps and
		// markers must be inserted in each and every part. We
		// will hence have to preserve those elements in a list
		// to be used when we write the parts other than the
		// first in our lilypond-score:
	      	preserveJumpOrMarker(dir, measurenumber, end);
	      }
	    else if (tp == Element::MARKER)
	      {
		Marker* ma = (Marker*) dir;
		MarkerType mtp = ma->markerType();
		//only print markers which belong at measure end:
		if (mtp == MarkerType::FINE || mtp == MarkerType::TOCODA)
		  {
		    //print the marker in part one
		    instructionMarker(ma);
		    //preserve the marker for later use in other parts:
		    preserveJumpOrMarker(dir, measurenumber, end);
		  }
	      }
	  }
      }



//---------------------------------------------------------
//   findMatchInMeasure -- find chord or rest in measure
//     starting or ending at tick
//---------------------------------------------------------
bool ExportLy::findMatchInMeasure(int tick, Staff* stf, Measure* m, int strack, int etrack, bool rehearsalmark)
{
  int iter=0;
  bool  found = false;

  for (int st = strack; st < etrack; ++st)
    {
      for (Segment* seg = m->first(); seg; seg = seg->next())
	{
	  iter ++;
	  Element* el = seg->element(st);
	  if (!el) continue;

	  if ((el->isChordRest()) and ((el->staff() == stf) or (rehearsalmark==true)) && ((seg->tick() >= tick)))
	    {
	      if (seg->tick() > tick) tick=prevElTick;
	      anker.anchor=el;
	      found=true;
	      anker.tick=tick;
	      anker.start=true;
	      goto fertig;
	    }
	    prevElTick = seg->tick();
	 }
    }
 fertig:
 return found;
}


//---------------------------------------------------------
//   findMatchInPart -- find chord or rest in part
//     starting or ending at tick
//---------------------------------------------------------

bool ExportLy::findMatchInPart(int tick, Staff* stav, Score* sc, int starttrack, int endtrack, bool rehearsalmark)
{

  bool found=false;
  for (MeasureBase* mb = sc->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != Element::MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      found = findMatchInMeasure(tick, stav, m, starttrack, endtrack, rehearsalmark);
      if (found) break;
     }
return found;
}

//---------------------------------------------------------
//     buildInstructionList -- associate instruction (measure relative elements)
//     with elements in segments to enable writing at the correct position
//     in the output stream. Called once for every part to handle all part-level elements.
//---------------------------------------------------------

void ExportLy::buildInstructionListPart(int strack, int etrack)
{

  // part-level elements stored in the score layout: at the global level
  prevElTick=0;
#if 0 // TODO-WS implementation changed
  foreach(Element* instruction, *(score->gel()))
    {
      bool found=false;
      bool rehearsalm=false;
      switch(instruction->type())
	{
	case Element::JUMP:
//TODO-WS	   qDebug("score JUMP found at tick: %d\n", instruction->tick());
            break;
	case Element::MARKER:
/*TODO-WS	    qDebug("score MARKER found at tick: %d\n", instruction->tick()); */ break;
	case Element::HAIRPIN:
	case Element::HARMONY:
	case Element::OTTAVA:
	case Element::PEDAL:
	case Element::DYNAMIC:
	case Element::TEXT:
	case Element::TEXTLINE:
	  {
	    SLine* sl = (SLine*) instruction;
	    Text* tekst = (Text*) instruction;
	    //	    if (tekst->subtypeName() == "System") qDebug("Systemtekst in part\n");
	    //      if (tekst->subtypeName() == "Staff")  qDebug("Stafftest in part\n");
	    if (tekst->subtypeName() == "RehearsalMark")
	      {
		rehearsalm=true;
		qDebug("found rehearsalmark in part\n");
	      }
	    //start of instruction:
	    found=findMatchInPart(sl->tick(), sl->staff(), score, strack, etrack, rehearsalm);
	    if (found)
	      {
		anker.instruct=instruction;
		storeAnchor(anker);
	      }
	    //end of instruction:
//TODO-WS	    found=findMatchInPart(sl->tick2(), sl->staff(), score, strack, etrack, rehearsalm);
	    if (found)
	      {
		anker.instruct=instruction;
		storeAnchor(anker);
	      }
	    break;
	  } //end textline
	default:
	  // all others ignored
	  // qDebug(" instruction type %s not implemented\n", Element::name(instruction->type()));
	  break;
	}
    }// end foreach element....
#endif

  // part-level elements stored in measures:
  for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != Element::MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      buildInstructionList(m, strack, etrack);
    }
}//end: buildInstructionList


//---------------------------------------------------------
//   buildInstructionList -- associate instruction (measure relative elements)
//     with elements in segments to enable writing at the correct position
//     in the output stream. Called once for every measure to handle either
//     part-level or measure-level elements.
//---------------------------------------------------------

void ExportLy::buildInstructionList(Measure* m, int strack, int etrack)
{

  // loop over all measure relative elements in this measure
  for (auto ci = m->el()->begin(); ci != m->el()->end(); ++ci)
    {
      bool found=false;
//      bool rehearsal=false;

      Element* instruction = *ci;
      switch(instruction->type())
	{
	case Element::DYNAMIC:
	case Element::SYMBOL:
	case Element::TEMPO_TEXT:
	case Element::TEXT:
	case Element::HAIRPIN:
	  //case Element::HARMONY:
	case Element::OTTAVA:
	case Element::PEDAL:
	case Element::STAFF_TEXT:
#if 0 // TODO-WS
	  { 	    //	    if (instruction->subtypeName() == "Staff") qDebug("stafftekst i measure\n");
	    //   if (instruction->subtypeName() == "System") qDebug("systemtekst i measure\n");
	    if (instruction->subtypeName() == "RehearsalMark") rehearsal=true;
	    found = findMatchInMeasure(instruction->tick(), instruction->staff(), m, strack, etrack, rehearsal);
	  if (found)
	    {
	      anker.instruct=instruction;
	      storeAnchor(anker);
	    }
       }
#endif
	  break;
	case Element::HARMONY:
	  {
          Harmony* h = static_cast<Harmony*>(instruction);
          int tick = h->parent()->type() == Element::SEGMENT
             ? static_cast<Segment*>(h->parent())->tick() : 0;
	    found = findMatchInMeasure(tick, instruction->staff(), m, strack, etrack, false);
	    if ((found) && (staffInd == 0)) //only save chords in first staff.
	      {
		anker.instruct=instruction;
		storeChord(anker);
		resetAnchor(anker);
	      }
	    break;
	  }
	 default:
	   break;
	}
    }
}// end buildinstructionlist(measure)


void ExportLy::buildGlissandoList(int strack, int etrack)
{
  //seems to be overkill to go thru entire score first to find
  //glissandos. Alternative would be to back up to the previous chord
  //in writeChordMeasure(). But I don't know how to do that. So I steal the
  //buildinstructionlist-functions to make a parallell
  //buildglissandolist-function. (og)
  for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != Element::MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      for (int st = strack; st < etrack; ++st)
       	{
	  for (Segment* seg = m->first(); seg; seg = seg->next())
	    {
	      Element* el = seg->element(st);//(st);
	      if (!el) continue;

	      if (el->type() == Element::CHORD)
		{
		 Chord* cd = (Chord*)el;
		  if (cd->glissando())
		    {
		      glisscount++;
		      //this may cause trouble in multistaff-scores (??):
		      Element* prevel = seg->prev()->element(st); //(st);
		      Chord* prevchord = (Chord*)prevel;
		      glisstable[glisscount].chord = prevchord;
		      glisstable[glisscount].type = int(cd->glissando()->glissandoType());
		      glisstable[glisscount].glisstext = cd->glissando()->text();
		      glisstable[glisscount].tick = prevchord->tick();
		    }
		}
	    }
	 }
    }
}



//---------------------------------------------------------
//   indent  -- scorebuffer
//---------------------------------------------------------

void ExportLy::indent()
{
  for (int i = 0; i < level; ++i)
    out << "    ";
}


//---------------------------------------------------------
//   indent  -- outputfile
//---------------------------------------------------------

void ExportLy::indentF()
{
      for (int i = 0; i < level; ++i)
	    os << "    ";
}


//-------------------------------------
// Find tuplets Note
//-------------------------------------

void ExportLy::findTuplets(ChordRest* cr)
{
      Tuplet* t = cr->tuplet();

      if (t) {
            if (tupletcount == 0) {
                  int actNotes   = t->ratio().numerator();
                  int nrmNotes   = t->ratio().denominator();
                  int baselength = t->duration().ticks() / nrmNotes;
                  int thislength = cr->duration().ticks();
		  tupletcount    = nrmNotes * baselength - thislength;
                  out << "\\times " <<  nrmNotes << "/" << actNotes << "{" ;
                  }
            else if (tupletcount > 1) {
                  int thislength = cr->duration().ticks();
                  tupletcount    = tupletcount - thislength;
                  if (tupletcount == 0)
                        tupletcount = -1;
                  }
            }
      }

//-----------------------------------------------------
//  voltaCheckBar
//
// supplements findVolta and called from there: check barlinetypes in
// addition to endings
//------------------------------------------------------
int ExportLy::voltaCheckBar(Measure* meas, int i)
{

  int barlinetype = meas->endBarLineType();

  switch(barlinetype)
    {
    case START_REPEAT:
      i++;
      voltarray[i].voltart=startrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_REPEAT:
      i++;
      voltarray[i].voltart=endrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_START_REPEAT:
      i++;
      voltarray[i].voltart=bothrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_BAR:
      i++;
      voltarray[i].voltart=endbar;
      voltarray[i].barno=taktnr;
      break;
    case DOUBLE_BAR:
      i++;
      voltarray[i].voltart=doublebar;
      voltarray[i].barno=taktnr;
      break;
    case BROKEN_BAR:
    case DOTTED_BAR:
      i++;
      voltarray[i].voltart=brokenbar;
      voltarray[i].barno=taktnr;
      break;
    default:
      break;
    }//switch

  // find startrepeat which does not exist as endbarline: If
  // startrepeat is at beginning of line, and endrepeatbar ends this
  // first measure of the line, repeatFlag is not set to RepeatStart,
  // then this does not help, and I need "findStartRepNoBarline"
  if (meas->repeatFlags() == RepeatStart)
    {
      // we have to exclude startrepeats found as endbarlines in previous measure
      if ((voltarray[i].barno != taktnr-1) and (voltarray[i].voltart != startrepeat) and ( voltarray[i].voltart != bothrepeat ))
	{
	  i++;
	  voltarray[i].voltart=startrepeat;
	  voltarray[i].barno=taktnr-1; //set as last element in previous measure.
	}
    }

  return i;
}//end voltacheckbarline

//------------------------------------------------------------------------
// findStartRepNoBarline
// helper routine for findVolta.
//------------------------------------------------------------------------

void ExportLy::findStartRepNoBarline(int &i, Measure* m)
{
 // loop over all measure relative segments in this measure
  for (Segment* seg = m->first(); seg; seg = seg->next())
    {
      if (seg->segmentType() == Segment::SegStartRepeatBarLine)
	{
	  i++; // insert at next slot of voltarray
	  voltarray[i].voltart = startrepeat;
	  voltarray[i].barno = taktnr-1;
	  break;
	}
    }
}



//------------------------------------------------------------------
//   findVolta -- find and register volta and repeats in entire piece,
//   register them in voltarray for later use in writeVolta.
//------------------------------------------------------------------

void  ExportLy::findVolta()
{
  taktnr=0;
  lastind=0;
  int i=0;

  for (i=0; i<255; i++)
    {
      voltarray[i].voltart=none;
      voltarray[i].barno=0;
    }

  i=0;

  for (MeasureBase * m=score->first(); m; m=m->next())
    {// for all measures
      if (m->type() != Element::MEASURE )
	continue;

      ++taktnr; //should really not be incremented in case of pickupbars.

      //needed because of problems with repeatflag and because there
      //are no readymade functions for finding startbarlines, and
      //because startbarlines are not at the global level:
      Measure* meas = (Measure*)m;
      findStartRepNoBarline(i,meas);

#if 0 // TODO-WS implementation changed
      foreach(Element* el, *(m->score()->gel()))
	//for each element at the global level relevant for this measure
	{
	  if (el->type() == Element::VOLTA)
	    {
	      Volta* v = (Volta*) el;

	      if (v->tick() == m->tick()) //If we are at the beginning of the measure
		{
		  i++;
		  //  if (v->subtype() == Volta::VOLTA_CLOSED)
		  // 		    {
		  //                 Lilypond has second volta closed for all kinds of thin-tick or tick-thin double bars
		  //                 with or without repeat dots. But not for thin-thin double bar or single barline.
		  //                 The only way I know of to make volta closed for thin-thin double bar
		  //                 and single bar is to put the following lines in the source code, the file
		  //                 volta-bracket.cc, at approx line 133, and recompile Lilypond
		  //                	&& str != "||"
		  //                        && str != "|"
		  //                 But then closing becomes hardcoded and we have no choice.
		  //                 There must be some \override or \set which fixes this stubbornness of the
		  //                 Lilypond developers?? (olagunde@start.no)
		  // 		    }
		  // 		  else if (v->subtype() == Volta::VOLTA_OPEN)
		  // 		    {
		  // 		    }
		  voltarray[i].voltart = startending;
		  voltarray[i].barno=taktnr-1; //register as last element i previous measure
		}
#if 0 // TODO-WS
	      if (v->tick2() == m->tick() + m->ticks()) // if it is at the end of measure
		{
		  i++;
		  voltarray[i].voltart = endending;
		  voltarray[i].barno=taktnr;//last element of this measure
		  // 		  if (v->subtype() == Volta::VOLTA_CLOSED)
		  // 		    {// see comment above.
		  // 		    }
		  // 		  else if (v->subtype() == Volta::VOLTA_OPEN)
		  // 		    {// see comment above.
		  // 		    }
		}
#endif
	    }//if volta
	}// for all global elements
#endif
      i=voltaCheckBar((Measure *) m, i);
    }//for all measures
  lastind=i;

}// end findvolta

void ExportLy::voltatest()
{
  int i=0;
  for (i=0; i<lastind; i++)
    {
      qDebug("iter: %d\n", i);
      switch(voltarray[i].voltart)
	{
	case startrepeat:
	  qDebug("startrepeat, bar %d\n", voltarray[i].barno);
	  break;
	case endrepeat:
	  qDebug("endrepeat, bar %d\n", voltarray[i].barno);
	  break;
	case bothrepeat:
	  qDebug("bothrepeat, bar %d\n", voltarray[i].barno);
	  break;
	case endbar:
	  qDebug("endbar, bar %d\n", voltarray[i].barno);
	  break;
	case doublebar:
	  qDebug("doublebar, bar %d\n", voltarray[i].barno);
	  break;
	case startending:
	  qDebug("startending, bar %d\n", voltarray[i].barno);
	  break;
	case endending:
	  qDebug("endending, bar %d\n", voltarray[i].barno);
	  break;
	default:
	  break;
	}

    }
}


//---------------------------------------------------------
//   exportLilypond
//---------------------------------------------------------

bool MuseScore::saveLilypond(Score* score, const QString& name)
{
  ExportLy em(score);
  return em.write(name);
}


//---------------------------------------------------------
//   writeClef
//---------------------------------------------------------

void ExportLy::writeClef(int clef)
{
  out << "\\clef ";
  switch(clef) {
  case ClefType::G:      out << "treble\n";         break;
  case ClefType::F:      out << "bass\n";           break;
  case ClefType::G1:     out << "\"treble^8\"\n";   break;
  case ClefType::G2:     out << "\"treble^15\"\n";  break;
  case ClefType::G3:     out << "\"treble_8\"\n";   break;
  case ClefType::F8:     out << "\"bass_8\"\n";     break;
  case ClefType::F15:    out << "\"bass_15\"\n";    break;
  case ClefType::F_B:    out << "bass\n";           break;
  case ClefType::F_C:    out << "bass\n";           break;
  case ClefType::C1:     out <<  "soprano\n";       break;
  case ClefType::C2:     out <<  "mezzo-soprano\n"; break;
  case ClefType::C3:     out <<  "alto\n";          break;
  case ClefType::C4:     out <<  "tenor\n";         break;
  case ClefType::TAB2:
  case ClefType::TAB:    out <<  "tab\n";           break;
  case ClefType::PERC:   out <<  "percussion\n";    break;
  }

}

//---------------------------------------------------------
//   writeTimeSig
//---------------------------------------------------------

void ExportLy::writeTimeSig(TimeSig* sig)
{
  int st     = sig->timeSigType();
  Fraction f = sig->sig();
  timedenom  = f.denominator();
  z1         = f.numerator();

  //lilypond writes 4/4 as C by default, so only check for cut.
  if (st == TSIG_ALLA_BREVE)
    {
      z1=2;
      timedenom=2;
      // 2/2 automatically written as alla breve by lily.
    }
  indent();
  out << "\\time " << z1 << "/" << timedenom << " ";
}

//---------------------------------------------------------
//   writeKeySig
//---------------------------------------------------------

void ExportLy::writeKeySig(int st)
{
  st = char(st & 0xff);
  out << "\\key ";
  switch(st) {
  case 7:  out << "cis"; break;
  case 6:  out << "fis"; break;
  case 5:  out << "b";   break;
  case 4:  out << "e";   break;
  case 3:  out << "a";   break;
  case 2:  out << "d";   break;
  case 1:  out << "g";   break;
  case 0:  out << "c";   break;
  case -7: out << "ces"; break;
  case -6: out << "ges"; break;
  case -5: out << "des"; break;
  case -4: out << "as";  break;
  case -3: out << "es";  break;
  case -2: out << "bes"; break;
  case -1: out << "f";   break;
  default:
    qDebug("illegal key %d\n", st);
    break;
  }
  out << " \\major \n";
}

//---------------------------------------------------------
//   tpc2name
//---------------------------------------------------------

QString ExportLy::tpc2name(int tpc)
{
  const char names[] = "fcgdaeb";
  int acc   = ((tpc+1) / 7) - 2;
  QString s(names[(tpc + 1) % 7]);
  switch(acc) {
  case -2: s += "eses"; break;
  case -1: s += "es";  break;
  case  1: s += "is";  break;
  case  2: s += "isis"; break;
  case  0: break;
  default: s += "??"; break;
  }
  return s;
}


//---------------------------------------------------------
//   tpc2purename
//---------------------------------------------------------

QString ExportLy::tpc2purename(int tpc)
{
  const char names[] = "fcgdaeb";
  QString s(names[(tpc + 1) % 7]);
  return s;
}


//--------------------------------------------------------
//  Slur functions, stolen from exportxml.cpp. I really
//  don't understand these functions, but they seem to work
//  (olav)
//
//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int ExportLy::findSlur(const Slur* s) const
{
  for (int i = 0; i < 8; ++i)
    if (slurre[i] == s) return i;
  return -1;
}

//---------------------------------------------------------
//   doSlurStart. Find start of slur connecte to chord.
//---------------------------------------------------------

void ExportLy::doSlurStart(Chord* chord, bool nextisrest)
{
#if 0 // TODO-S
  int slurcount=0;
  for(const Spanner* sp = chord->spannerFor(); sp; sp = sp->next())
    {
      if (sp->type() != Element::SLUR)
            continue;
      const Slur* s = static_cast<const Slur*>(sp);

      slurcount++;

      int i = findSlur(s);

      if (i >= 0)
	{
	  slurstack++;
	  slurre[i] = 0;
	  started[i] = false;
	  if (s->slurDirection() == MScore::UP) out << "^";
	  if (s->slurDirection() == MScore::DOWN) out << "_";
	  if (slurcount==2)
	    {
	      phraseslur=slurstack;
	      out <<"\\";
	    }
	  if (nextisrest)
	    {
	      out << "\\laissezVibrer " ;
	    }
	    else
	      out << "(";

	}
      else
	{
	  i = findSlur(0);
	  if (i >= 0)
	    {
	      slurstack++;
	      slurre[i] = s;
	      started[i] = true;
	      if (s->slurDirection() == MScore::UP) out << "^";
	      if (s->slurDirection() == MScore::DOWN) out << "_";
	      if (slurcount==2)
		{
		  phraseslur=slurstack;
		  out <<"\\";
		}

	      if (nextisrest)
	     {
	       out << "\\laissezVibrer " ;
	     }
	     else
	      out << "(";
	    }
	  else
	    qDebug("no free slur slot");
	}
    }
#endif

}


//---------------------------------------------------------
//   doSlurStop
//   From exportxml.cpp:
//-------------------------------------------
void ExportLy::doSlurStop(Chord* chord)
{
#if 0 // TODO-S
  for(const Spanner* sp = chord->spannerBack(); sp; sp = sp->next())
    {
    if (sp->type() != Element::SLUR)
          continue;
    const Slur* s = static_cast<const Slur*>(sp);

      // check if on slur list
      int i = findSlur(s);
      if (i < 0)
	{
	  // if not, find free slot to store it
	  i = findSlur(0);
	  if (i >= 0)
	    {
	      slurre[i] = s;
	      started[i] = false;
	      if (slurstack == phraseslur)
		{
		  phraseslur=0;
		  out << "\\";
		}
	      slurstack--;
	      out << ")";  //why do we always end here??
	    }
	  else
	    qDebug("no free slur slot");
	}
    }
#endif
      for (int i = 0; i < 8; ++i) {
            if (slurre[i]) {
#if 0 // TODO-S
                  if  (slurre[i]->endElement() == chord) {
                        if (started[i]) {
                              slurre[i] = 0;
                              started[i] = false;
                              if (phraseslur == slurstack) {
                                    out << "\\";
     	                              phraseslur = 0;
     	                              }
                              slurstack--;
                              out << ")"; //why do we never end here?!
                              }
                        }
#endif
	            }
            }
      }

//-------------------------
// checkSlur
//-------------------------
void ExportLy::checkSlur(Chord* chord, bool nextisrest)
{
  //init array:
  for (int i = 0; i < 8; ++i)
    {
      slurre[i] = 0;
      started[i] = false;
     }
  doSlurStop(chord);
  doSlurStart(chord, nextisrest);
}


//-----------------------------------
// helper routine for writeScore
// -- called from there
//-----------------------------------

void ExportLy::writeArticulation(ChordRest* c)
{
  foreach(Articulation* a, c->articulations())
    {
      switch(a->articulationType())
	{
	case Articulation_Fermata:
        if (a->up())
	      out << "\\fermata ";
        else
	      out << "_\\fermata ";
	  break;
	case Articulation_Thumb:
	  out << "\\thumb ";
	  break;
	case Articulation_Sforzatoaccent:
	  out << "-> ";
	  break;
	case Articulation_Espressivo:
	  out << "\\espressivo ";
	  break;
	case Articulation_Staccato:
	  out << "-. ";
	  break;
	case Articulation_Staccatissimo:
        if (a->up())
	      out << "-| ";
        else
	      out << "_| ";
	  break;
	case Articulation_Tenuto:
	  out << "-- ";
	  break;
//TODO:	case Articulation_Flageolet:
//	  out << "\\flageolet ";
	case Articulation_Portato:
        if (a->up())
	      out << "-_ ";
        else
	      out << "__ ";
	  break;
	case Articulation_Marcato:
        if (a->up())
	      out << "-^ ";
        else
	      out << "_^ ";
	  break;
	case Articulation_Ouvert:
	  out << "\\open ";
	  break;
	case Articulation_Plusstop:
	  out << "-+ ";
	  break;
	case Articulation_Upbow:
	  out << "\\upbow ";
	  break;
	case Articulation_Downbow:
	  out << "\\downbow ";
	  break;
	case Articulation_Reverseturn:
	  out << "\\reverseturn ";
	  break;
	case Articulation_Turn:
	  out << "\\turn ";
	  break;
	case Articulation_Trill:
	  out << "\\trill ";
	  break;
	case Articulation_Prall:
	  out << "\\prall ";
	  break;
	case Articulation_Mordent:
	  out << "\\mordent ";
	  break;
	case Articulation_PrallPrall:
	  out << "\\prallprall ";
	  break;
	case Articulation_PrallMordent:
	  out << "\\prallmordent ";
	  break;
	case Articulation_UpPrall:
	  out << "\\prallup ";
	  break;
	case Articulation_DownPrall:
	  out << "\\pralldown ";
	  break;
	case Articulation_UpMordent:
	  out << "\\upmordent ";
	  break;
	case Articulation_DownMordent:
	  out << "\\downmordent ";
	  break;
	default:
	  qDebug("unsupported note attribute %d\n", int(a->articulationType()));
	  break;
	}// end switch
    }// end foreach
}// end writeArticulation();


//------------------------------------------
// write Tremolo. stolen from exportxml.cpp
//------------------------------------------

void ExportLy::writeTremolo(Chord * chord)
{
  if (chord->tremolo())
    {
      Tremolo * tr = chord->tremolo();
      int st = tr->tremoloType();
      switch (st)
	{
	case TREMOLO_R8:
	  out << ":8 ";
	  break;
	case TREMOLO_R16:
	  out << ":16 ";
	  break;
	case TREMOLO_R32:
	  out << ":32 ";
	  break;
	case TREMOLO_R64:
	  out << ":64 ";
	  break;
	default:
	  qDebug("unknown tremolo %d\n", st);
	  break;
	}
    }
}


//-------------------------------------------------------------------------------------------
//  findFingerAndStringno
//------------------------------------------------------------------------------------------

void ExportLy::findFingerAndStringno(Note* note, int &fingix, int &stringix, QString (&fingarray)[5], QString (&stringarray)[10])
      {
      foreach (const Element* e, note->el()) {
            if (e->type() == Element::FINGERING) {
                  const Text* text = static_cast<const Text*>(e);
                  if (text->textStyleType() == TEXT_STYLE_FINGERING) {
	                  fingix++;
      	            Text* f = (Text*)e;
	                  fingarray[fingix] = f->text();
	                  }
                  else if (text->textStyleType() == TEXT_STYLE_STRING_NUMBER) {
                        stringix++;
                        Text * s = (Text*)e;
                        stringarray[stringix] = s->text();
                        }
                  }
            }
      }//end findfingerandstringno


void ExportLy::writeStringInstruction(int &strgix, QString stringarr[10])
{
  if (strgix > 0)
    {  //there should be only one stringinstruction, so this is possibly redundant.
      for (int i=0; i < strgix; i++)
	out << "\\" << stringarr[strgix];
    }
  strgix = 0;
}


//---------------------------------------------------------
//  writeFingering
//---------------------------------------------------------
void ExportLy::writeFingering (int &fingr,   QString fingering[5])
{
  if (fingr > 0)
	{
	  if (fingr == 1) out << "-" << fingering[1] << " ";
	  else if (fingr >1)
	    {
	      out << "^\\markup {\\finger \"";
	      out << fingering[1] << " - " << fingering[2] << "\"} ";
	    }
	}
  fingr=0;
}

//----------------------------------------------------------------
// stemDirection
//----------------------------------------------------------------

void ExportLy::stemDir(Chord * chord)
{
  // For now, we only export stem directions for gracenotes.
  if (chord->beam() == 0 || chord->beam()->elements().front() == chord)
    {
      MScore::Direction d = chord->stemDirection();
      if (d != stemDirection)
	{
	  stemDirection = d;
	  if ((d == MScore::UP) and (graceswitch == true))
	    out << "\\stemUp ";
	  else if ((d == MScore::DOWN)  and (graceswitch == true))
	    out << "\\stemDown ";
	  //   else if (d == MScore::AUTO)
	  // 	    {
	  // 	      if (graceswitch == true)
	  // 		{
	  // 		  out << "\\stemNeutral "; // we set this at the end of graces anyway.
	  // 		}
	  // 	    }
	}
    }
}//end stemDirection

//-------------------------------------------------------------
// findGraceNotes
//--------------------------------------------------------------
void ExportLy::findGraceNotes(Note *note, bool &chordstart, int streng)
{
  NoteType gracen;
  gracen = note->noteType();
  switch(gracen)
    {
    case NOTE_INVALID:
    case NOTE_NORMAL:
      if (graceswitch==true)
	{
	  graceswitch=false;
	  gracebeam=false;
	  if (gracecount > 1) out << " ] "; //single graces are not beamed
	  out << " } \\stemNeutral "; //end of grace
	  gracecount=0;
	}
      if ((chordstart) or (streng > 0))
	{
	  out << "<";
	  chordstart=false;
	}
      break;
    case NOTE_ACCIACCATURA:
    case NOTE_APPOGGIATURA:
    case NOTE_GRACE4:
    case NOTE_GRACE16:
    case NOTE_GRACE32:
      if (graceswitch==false)
	{
	  out << "\\grace{\\stemUp "; //as long as general stemdirecton is unsolved: graces always stemUp.
	  graceswitch=true;
	  gracebeam=false;
	  gracecount=0;
	}
      gracecount++;
      break;
    } //end of switch(gracen)
}//end findGraceNotes

//---------------------------------------------------------------------------
//   setOctave
//---------------------------------------------------------------------------
void ExportLy::setOctave(int &purepitch, int &pitchidx, int (&pitchlist)[12])
{
  int oktavdiff=prevpitch - purepitch;
  int oktreit=numval(oktavdiff);
  while (oktreit > 0)
    {
      if ((oktavdiff < -6) or ((prevnote=="b") and (oktavdiff < -5)))
	{ //up
	  out << "'";
	  oktavdiff=oktavdiff+12;
	}
      else if ((oktavdiff > 6)  or ((prevnote=="f") and (oktavdiff > 5)))
	{//down
	  out << ",";
	  oktavdiff=oktavdiff-12;
	}
      oktreit=oktreit-12;
    }
  prevpitch=purepitch;
  pitchlist[pitchidx]=purepitch;
  pitchidx++;
}//end setOctave


bool ExportLy::arpeggioTest(Chord* chord)
{
  bool arp=false;
  if (chord->arpeggio())
    {
      arp=true;
      int subtype = int(chord->arpeggio()->arpeggioType());
      switch (subtype)
	{
	case 0:
	  out << "\\arpeggioNormal ";
	  break;
	case 1:
	  out << "\\arpeggioArrowUp ";
	  break;
	case 2:
	  out << "\\arpeggioArrowDown ";
	  break;
	default:
	  qDebug("unknown arpeggio subtype %d\n", subtype);
	  break;
	}
    }
  return arp;
}


bool ExportLy::glissandotest(Chord* chord)
{
  bool gliss=false;
  int i=0;
  for (i=0; i < glisscount; i++)
    {
      if (glisstable[i].chord == chord)
	{
	  if (glisstable[i].type == 1)
	    {
	      out << "\\once\\override Glissando #'style = #'trill \n";
	      indent();
	    }
	  gliss=true;
	}
    }
  return gliss;
}


//------------------------------------------------------------
// findNoteSymbol
// Find symbols attached to note.
//------------------------------------------------------------

bool ExportLy::findNoteSymbol(Note* n, QString& symbolname)
      {
      symbolname = "";

      foreach(const Element* symbol, n->el()) {
            if (symbol->type() == Element::SYMBOL) {
                  const Symbol* symb = static_cast<const Symbol*>(symbol);
                  symbolname = Sym::id2name(symb->sym());
                  return true; // what about more symbols connected to one note? Return array of names?
                  }
            }
      return false;
      }//end findNoteSymbol

//---------------------------------------------------------
//   writeChord
//---------------------------------------------------------

void ExportLy::writeChord(Chord* c, bool nextisrest)
{
  int  purepitch;
  QString purename, chordnote;
  int pitchlist[12];
  QString fingering[5];
  QString stringno[10];
  bool tie=false;
  bool symb=false;
  QList<Note*> nl = c->notes();
  bool chordstart=false;
  int fing=0;
  int streng=0;
  bool gliss=false;
  QString glisstext;
  QString symbolname;

  int j=0;
  for (j=0; j<12; j++) pitchlist[j]=0;

  stemDir(c);

  if (nl.size() > 1) chordstart = true;

  int  pitchidx=0;
  bool arpeggioswitch=false;
  arpeggioswitch=arpeggioTest(c);

  gliss = glissandotest(c);
  int iter=0;
  for (QList<Note*>::iterator notesinchord = nl.begin();;)
    {
	  iter++;
      Note* n = *notesinchord;
      //if fingering found on _previous_ chordnote, now is the time for writing it:
      if (fing>0)  writeFingering(fing,fingering);
      if (streng>0) writeStringInstruction(streng,stringno);

      //find diverse elements and attributes connected to the note
      findFingerAndStringno(n, fing, streng, fingering, stringno);

      if (iter == 1) findTuplets(n->chord());

      findGraceNotes(n, chordstart, streng);//also writes start of chord symbol "<" if necessary

      symb = findNoteSymbol(n, symbolname);

      if (n->tieFor()) tie=true;

      if (gracecount==2) out << " [ ";


      out << tpc2name(n->tpc()).toUtf8().data();  //Output of The Notename Itself

      if ((chordstart) and (symb))
	{
	  cout << "symbol in chord\n";
	  writeSymbol(symbolname);
	}

      purepitch = n->pitch();
      purename = tpc2name(n->tpc());  //with -es or -is
      prevnote=cleannote;             //without -es or -is
      cleannote=tpc2purename(n->tpc());//without -es or -is

      if (purename.contains("eses")==1)  purepitch=purepitch+2;
      else if (purename.contains("es")==1)  purepitch=purepitch+1;
      else if (purename.contains("isis")==1) purepitch=purepitch-2;
      else if (purename.contains("is")==1) purepitch=purepitch-1;

      setOctave(purepitch, pitchidx, pitchlist);

      if (notesinchord == nl.begin())
	{
	  chordpitch=prevpitch;
	  chordnote=cleannote;
	}

      ++notesinchord; //number of notes in chord, we progress to next chordnote
      if (notesinchord == nl.end())
	break;
      out << " ";
    } //end of notelist = end of chord

  if ((nl.size() > 1) or (streng > 0))
    {
      //if fingering found on previous chordnote, now is the time for writing it:
      if (fing   > 0) writeFingering(fing, fingering);
      if (streng > 0) writeStringInstruction(streng,stringno);
      out << ">"; //endofchord sign
      cleannote=chordnote;
      //if this is a chord, use first note of chord as previous note
      //instead of actual previous note.
    }

  int ix=0;
  prevpitch=pitchlist[0];
   while (pitchlist[ix] !=0)
     {
       if (pitchlist[ix]<prevpitch) prevpitch=pitchlist[ix];
       ix++;
     }

  writeLen(c->actualTicks());

  if ((symb) and (nl.size() == 1))
    writeSymbol(symbolname);

  if (arpeggioswitch)
    {
      out << "\\arpeggio ";
      arpeggioswitch=false;
    }


  //if fingering found on a single note, now is the time for writing it:
  if (nl.size() == 1)
    writeFingering(fing, fingering);

  writeTremolo(c);

  if (gliss)
    {
      out << "\\glissando ";
      if (glisstable[glisscount].glisstext !="")
	out << "^\\markup{" << glisstable[glisscount].glisstext << "} ";
      //todo: make glisstext follow glissline
    }

  if (tie)
    {
      out << "~";
      tie=false;
    }

  writeArticulation(c);
  checkSlur(c, nextisrest);

  out << " ";

}// end of writechord


//---------------------------------------------------------
//   getLen
//---------------------------------------------------------

int ExportLy::getLen(int l, int* dots)
{
  int len  = 4;

  if (l == 16 * MScore::division) //longa, whole measure of 4/2-time
    len=-2;
  else if (l == 12 * MScore::division) // "6/2" "dotted brevis" used for whole-measure rest in 6/2 time.
    len=-3;
  else if (l == 10 * MScore::division) // "5/2"- time, used for whole-measure rest.
    len=-4;
  else if (l == 8 * MScore::division) //brevis
    len = -1;
  else if (l == 7 * MScore::division) //doubledotted whole
    {
      len = 1;
      *dots = 2;
    }
  else if (l == 6 * MScore::division) //dotted whole
    {
      len  = 1;
      *dots = 1;
    }
  else if (l == 5 * MScore::division) // whole measure of 5/4-time
      len = -5;
  else if (l == 4 * MScore::division) //whole
    len = 1;
  else if (l == 3 * MScore::division) // dotted half
    {
      len = 2;
      *dots = 1;
    }
  else if (l == ((MScore::division/2)*7)) // double-dotted half: 7/8 used for \partial bar.
    {
      len = 2;
      *dots=2;
    }
  else if (l == 2 * MScore::division)
    len = 2;
  else if (l == MScore::division)         //quarter
    len = 4;
  else if (l == MScore::division *3 /2)   //dotted quarter
    {
      len=4;
      *dots=1;
    }
  else if (l == ((MScore::division/4)*7)) // double-dotted quarter
    {
      len = 4;
      *dots=2;
    }
  else if (l == MScore::division / 2)     //8th
    len = 8;
  else if (l == MScore::division*3 /4) //dotted 8th
    {
      len = 8;
      *dots=1;
    }
  else if (l == ((MScore::division/8)*7)) // double-dotted 8th
    {
      len = 8;
      *dots=2;
    }
  else if (l == MScore::division / 4)
    len = 16;
  else if (l == MScore::division / 8)
    len = 32;
  else if (l == MScore::division * 3 /8) //dotted 16th.
    {
      len = 16;
      *dots = 1;
    }
  else if (l == ((MScore::division/16)*7)) // double-dotted 16th.
    {
      len = 16;
      *dots=2;
    }
  else if (l == MScore::division / 16)
    len = 64;
  else if (l == MScore::division /32)
    len = 128;
  //triplets, lily uses nominal value surrounded by \times 2/3 {  }
  //so we set len equal to nominal value
  else if (l == ((MScore::division  * 8)/3))
     len = 1;
  else if (l == MScore::division * 4 /3)
     len = 2;
  else if (l == (MScore::division * 2)/3)
    len = 4;
  else if (l == MScore::division /3)
    len = 8;
  else if (l == MScore::division /(3*2))
    len = 16;
  else if (l == MScore::division /3*4)
    len = 32;
  else if (l == MScore::division/3*8)
    len = 64;
  else if (l == 0)
    len = 1;
  else qDebug("measure: %d, unsupported len %d (%d,%d)\n", measurenumber, l, l/MScore::division, l % MScore::division);
  return len;
}

//---------------------------------------------------------
//   writeLen
//---------------------------------------------------------

void ExportLy::writeLen(int ticks)
{
  int dots = 0;
  int len = getLen(ticks, &dots);

  if (ticks != curTicks)
    {
      switch (len)
	{
	case -5:
	  out << "1*5/4";
	  break;
	case -4:
	  out << "2*5 ";
	  break;
	case -3:
	  out << "1.*2 ";
	  break;
	case -2://longa
	  out << "\\longa ";
	    break;
	case -1: //brevis
	  out << "\\breve";
	  break;
	default:
	  out << len;
	  for (int i = 0; i < dots; ++i)
	    out << ".";
	  break;
	}
      curTicks = ticks;
      if (dots>0)
	curTicks = -1; //first note after dotted: always explicit length
    }
}

//---------------------------------------------------------
//   writeRest
//    type = 0    normal rest
//    type = 1    whole measure rest
//    type = 2    spacer rest
//---------------------------------------------------------

void ExportLy::writeRest(int l, int type)
{
  if (type == 1) //whole measure rest
    {
      out << "R";
      curTicks = -1; //whole measure rest always requires explicit length
      writeLen(l);
      wholemeasurerest=1;
     }
  else if (type == 2) //invisible rest
    {
      curTicks = -1;
      out << "s";
      writeLen(l);
    }
  else //normal rest
    {
      out << "r";
     writeLen(l);
    }
  out << " ";
}

//--------------------------------------------------------------
//   write number of whole measure rests
//-------------------------------------------------------------
void ExportLy::writeMeasuRestNum()
{
  if (wholemeasurerest >1) out << "*" << wholemeasurerest << " ";
  if (wholemeasuretext != "")
    {
      out << "^\\markup{" << wholemeasuretext << "} \n";
      indent();
    }
  out << " | % \n";
  indent();
  wholemeasurerest=0;
  wholemeasuretext= "";
  curTicks = -9;
}

//--------------------------------------------------------
//   writeVolta
//--------------------------------------------------------
void ExportLy::writeVolta(int measurenumber, int lastind)
{
  bool utgang=false;
  int i=0;

  if (pickup)
    measurenumber--;
  while ((voltarray[i].barno < measurenumber) and (i<=lastind))
    {
      //find the present measure
      i++;
    }

  if (measurenumber==voltarray[i].barno)
    {
      while (utgang==false)
	{
	  switch(voltarray[i].voltart)
	    {
	    case startrepeat:
	      if (wholemeasurerest > 0) writeMeasuRestNum();
	      indent();
	      out << "\\repeat volta 2 { %startrep \n";
	      firstalt=false;
	      secondalt=false;
	      repeatactive=true;
	      curTicks=-1;
	      break;
	    case endrepeat:
	      if ((repeatactive==true) and (secondalt==false))
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} % end of repeatactive\n";
		  curTicks=-1;
		  // repeatactive=false;
		}
	      indent();
	      break;
	    case bothrepeat:
	      if (firstalt==false)
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} % end of repeat (both)\n";
		  indent();
		  out << "\\repeat volta 2 { % bothrep \n";
		  firstalt=false;
		  secondalt=false;
		  repeatactive=true;
		  curTicks=-1;
		}
	      break;
	    case doublebar:
	      if (wholemeasurerest > 0) writeMeasuRestNum();
	      out << "\n";
	      indent();
	      out << "\\bar \"||\"";
	      curTicks=-1;
	      break;
	    case startending:
	      if (firstalt==false)
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} % end of repeat except alternate endings\n";
		  indent();
		  out << "\\alternative{ {  ";
		  firstalt=true;
		  curTicks=-1;
		}
	      else
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();//should not happen?
		  out << "{ ";
		  indent();
		  firstalt=false;
		  secondalt=true;
		  curTicks=-1;
		}
	      break;
	    case endending:
	      if (firstalt)
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} %close alt1\n";
		  secondalt=true;
		  repeatactive=true;
		  curTicks=-1;
		}
	      else
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} } %close alternatives\n";
		  secondalt=false;
		  firstalt=true;
		  repeatactive=false;
		  curTicks=-1;
		}
	      break;
	    case endbar:
	      if (wholemeasurerest > 0) writeMeasuRestNum();
	      out << "\\bar \"|.\"";
	      curTicks=-1;
	      break;
          default:
	    // case none: qDebug("strange voltarraycontents?\n");
	    break;
	    }//end switch

	  if (voltarray[i+1].barno==measurenumber)
	    {
	      i++;
	    }
	  else utgang=true;
	}// end of while utgang false;
    }// if barno=measurenumber
}// end writevolta



//-----------------------------------------------------------------------
//    checkifnextisrest
//-----------------------------------------------------------------------
static void checkIfNextIsRest(MeasureBase* mb, Segment* s, bool &nextisrest, int track)
{
  nextisrest = false;
  Segment* nextseg = s->next();
  Element*  nextelem;
  nextelem= nextseg->element(track);

  while (!(nextseg->segmentType() == Segment::SegEndBarLine))//  and !(nextseg->segmentType() == Segment::SegEndBarLine)))
    {
      //go to next segment, check if it is chord or end of measure.
      if (nextseg->isChordRest())	break;
      nextseg = nextseg->next();
      nextelem = nextseg->element(track); //check if it is on this track
    }

  //if it is not on this track, continue until end we find segment
  //containing element of this track, or end of measure
  while ((nextelem==0) and (!(nextseg->segmentType() == Segment::SegEndBarLine)))
    {
      nextseg = nextseg->next();
      nextelem = nextseg->element(track);
    }

  // if next segment contains element of this track, check for end of
  // measure and chordorrest.
  if ((nextseg->segmentType() != Segment::SegEndBarLine) &&  (nextseg->isChordRest()))
    {
      // probably superfluous as we have previously checked for
      // element on this track (!=0)
      if ((!(nextelem == 0 || nextelem->generated())))
	{
	  if (nextelem->type() == Element::REST)
	    {
	      nextisrest=true;
	    }
	}
    }
  else // if we have reached end of measure
    {
      // go to next measure:
      if (mb->next()) //if it is not the last one of the piece.
	{
	  mb = mb->next();
	  if (mb->type() == Element::MEASURE)
	    {
	      Measure* meas = (Measure*) mb;
	      for(Segment* s = meas->first(); s; s = s->next())
		{
		  if (s->isChordRest())
		    {
		      Element* elem = s->element(track);
		      if (!(elem == 0 ||  elem->generated()))
			{
			  if (elem->type() == Element::REST)
			    {
			      nextisrest=true;
			    }
			  else if (elem->type() == Element::CHORD)
			    {
			      //relax
			    }
			}
		      break; //do not check more segments.
		    }
		}
	    }
	}
      else nextisrest=false;
    }
}




void ExportLy::newLyricsRecord()
{
  lyricsRecord* lyrrec;
  lyrrec = new lyricsRecord();

  for (int i = 0; i < VERSES; i++)
    {
      lyrrec->lyrdat.tick[i]=0;
      lyrrec->lyrdat.verselyrics[i] = "";
      lyrrec->lyrdat.segmentnumber[i] = 0;
    }
  lyrrec->lyrdat.staffname = staffname[staffInd].staffid;
  lyrrec->numberofverses=-1;
  lyrrec->next = NULL;
  lyrrec->prev = NULL;

  if (tailOfLyrics != NULL)
    {
      lyrrec->prev = tailOfLyrics;
      tailOfLyrics->next = lyrrec;

    }

  tailOfLyrics = lyrrec;
  thisLyrics = lyrrec;

  if (headOfLyrics == NULL)  headOfLyrics = lyrrec;
}

//--------------------------------------------------------------------
// findLyrics
//--------------------------------------------------------------------
void ExportLy::findLyrics()
{
  int verse = 0;
  int track = 0;
  int vox = 0;
  int prevverse =  0;

  for (int staffno=0; staffno < staffInd; staffno++)
    {
      newLyricsRecord();//one record for each staff. Contains multiple voices and verses.

      for (MeasureBase* mb = score->first(); mb; mb = mb->next())
	{
	  if (mb->type() != Element::MEASURE)
	    continue;
	  Measure* meas = (Measure*)mb;

        Segment::SegmentTypes st = Segment::SegChordRest | Segment::SegGrace;
	  for(Segment* seg = meas->first(st); seg; seg = seg->next(st))
	    {
        const QList<Lyrics*>* lyrlist = seg->lyricsList(staffno*VOICES);
            if (!lyrlist)
                  continue;

	      foreach(const Lyrics* lix, *lyrlist)
		{
		  if (lix)
		    {
		      verse = (lix)->no();
		      if ((verse - prevverse) > 1)
			{
			  thisLyrics->lyrdat.verselyrics[verse-1] += "__ _ ";
			}
		      track = (lix)->track();
		      vox = track - (staffno*VOICES);

		      thisLyrics->lyrdat.segmentnumber[verse]++;
		      thisLyrics->lyrdat.tick[verse] = (lix)->segment()->tick();

		      if (verse > thisLyrics->numberofverses)
			{
			  thisLyrics->numberofverses = verse;
			  if (verse > 0)
			    {
			      int segdiff = (thisLyrics->lyrdat.segmentnumber[verse-1] -  thisLyrics->lyrdat.segmentnumber[verse]);
			      if (segdiff > 0)
				{
				  for (int i = 0; i < segdiff; i++)
				    thisLyrics->lyrdat.verselyrics[verse] += " _ ";
				  thisLyrics->lyrdat.segmentnumber[verse] += segdiff;
				}
			    }
			}

		      QString lyriks = (lix)->text();

          //  escape '"' character
          if (lyriks.contains('"'))
                 lyriks = "\"" + lyriks.replace("\"","\\\"") + "\"";

		      lyriks = lyriks.replace(" ", "_"); //bolton: if two words on one note.
		      thisLyrics->lyrdat.verselyrics[verse] += lyriks;

		      thisLyrics->lyrdat.staffname =  staffname[staffno].staffid;
		      thisLyrics->lyrdat.voicename[verse] = staffname[staffno].voicename[vox];

		      thisLyrics->lyrdat.tick[verse] = (lix)->segment()->tick();

		      int syl   = (lix)->syllabic();
		      switch(syl)
			{
			case Lyrics::SINGLE:
			  thisLyrics ->lyrdat.verselyrics[verse] += " ";
			  break;
			case Lyrics::BEGIN:
			  thisLyrics->lyrdat.verselyrics[verse] +=  " -- ";
			  break;
			case Lyrics::END:
			  thisLyrics->lyrdat.verselyrics[verse] += "  ";
			  break;
			case Lyrics::MIDDLE:
			  thisLyrics->lyrdat.verselyrics[verse] += " -- ";
			  break;
			default:
			  qDebug("unknown syllabic %d\n", syl);
			}//switch syllable
		      cout << " lyrics endtick: " << (lix)->endTick() << "\n";
		      if ((lix)->ticks() > 0) //more than one note on this syllable
			{
			  cout << " _ ";
			  thisLyrics->lyrdat.verselyrics[verse] += " _ ";
			}
		    } //if lyrics
		  prevverse = verse;
		} // for each member of lyricslist
		if (verse < thisLyrics->numberofverses)
		  thisLyrics->lyrdat.verselyrics[thisLyrics->numberofverses] += "__ _ ";
	    } // for each segment
	} //for each staff
    } //for measurebase first to last
}// end of findlyrics

//-------------------------------------------------------------
// writeLyrics
//-------------------------------------------------------------
void ExportLy::writeLyrics()
{

  thisLyrics = headOfLyrics;
  tailOfLyrics->next = NULL;//???
  int staffi=0;
  int stanza=0;

  while (thisLyrics != NULL)
    {
      staffi=0;
      while (staffname[staffi].staffid != "laststaff")
	{
	  for (int j=0; j< staffname[staffi].numberofvoices; j++)
	    {
		  stanza=0;
	      for (int ix = 0; ix < thisLyrics->numberofverses+1; ix++)//thisLyrics->numberofverses; ix++)
		{
		  if ((thisLyrics->lyrdat.staffname == staffname[staffi].staffid)
		      and (thisLyrics->lyrdat.voicename[ix] == staffname[staffi].voicename[j]))
		    {
		      indentF();
		      stanza++;
		      char verseno = (ix + 65);
		      os << "  " << thisLyrics->lyrdat.staffname;
		      os << "verse" << verseno << " = \\lyricmode { \\set stanza = \" " << stanza << ". \" ";
		      os << thisLyrics->lyrdat.verselyrics[ix] << "}\n";
		    }
		}
	    }
	  staffi++;
	}
       //if (thisLyrics->next != NULL)
      thisLyrics = thisLyrics->next;
    }
  thisLyrics = headOfLyrics;
}



//--------------------------------------------------------------
// connectLyricsToStaff
//--------------------------------------------------------------

void ExportLy::connectLyricsToStaff()
{
  /*      if (lyrics attached to one of the voices in this staff)*/
  thisLyrics =headOfLyrics;
  while (thisLyrics != NULL)
    {
      for (int j=0; j< staffname[indx].numberofvoices; j++)
	{
	  for (int ix = 0; ix <= thisLyrics->numberofverses; ix++)//;
	    {
	      if (thisLyrics->lyrdat.staffname == staffname[indx].staffid)
		{
		  if (thisLyrics->lyrdat.voicename[ix] == staffname[indx].voicename[j])
		    {
		      indentF();
		      char verseno = ix + 65;
		      os << " \\context Lyrics = " << staffname[indx].staffid;
		      os << "verse"<< verseno;
		      os <<  "\\lyricsto ";
		      os << thisLyrics->lyrdat.voicename[ix] << "  \\";
		      os << thisLyrics->lyrdat.staffname << "verse" << verseno << "\n";;
		    }
		}
	    }
	}

      //if (thisLyrics->next != NULL)
      thisLyrics = thisLyrics->next;
    }
  os << "\n";
}//end connectlyricstostaff

//--------------------------------------------------------------------
// cleanupLyrics
//--------------------------------------------------------------------
void ExportLy::cleanupLyrics()
{
  thisLyrics=headOfLyrics;
  while (thisLyrics !=NULL)
    {
      headOfLyrics=headOfLyrics->next;
      delete thisLyrics;
      thisLyrics=headOfLyrics;
    }
}



//-----------------------------------------------------------------------
// flatInInstrName
//-----------------------------------------------------------------------
QString ExportLy::flatInInstrName(QString name)
{
  //(unecessarily?) big deal for handling the flat-sign in instrumentnames.
  int pt = 0;
  QChar kar;
  int unum;
  bool flat=false;
  QString newname="";
  for (pt = 0; pt < name.size(); pt++)
    {
      kar=name.at(pt);
      unum = kar.unicode();
      if (unum < 256)
	{
	  newname.append(kar);
	}
      else if (unum == 57613)
	// 57613 is the decimal value of ==hex e10d, the
	// unicode code point for "flat" in mscore's and
	// lilypond's fonts. How do I convert the QChar
	// directly to hex?
	{
	  newname.append("\\smaller \\flat ");
	  flat=true;
	}
    }
  if (flat)
    {
      newname.prepend("\\markup{");
      newname.append("}");
    }
  else newname = "";
  return newname;
}


//---------------------------------------------------------
//   writeVoiceMeasure
//---------------------------------------------------------

void ExportLy::writeVoiceMeasure(MeasureBase* mb, Staff* staff, int staffInd, int voice)

{
  int i=0;
  char cvoicenum, cstaffnum;
  bool  barempty=true;
  bool nextisrest=false;
  Measure* m = (Measure*) mb;

  //print barchecksign and barnumber for previous measure:
  if ((m->no() > 0) and (wholemeasurerest==0) and (textspanswitch==false))
    {
      indent();
      out << " | % " << m->no() << "\n" ;
    }
  measurenumber=m->no()+1;

   // if (m->irregular())
   //   {
   // 	       qDebug("irregular measure, number: %d\n", measurenumber);
   //   }


   if ((measurenumber==1) and (donefirst==false))
     // ^^^^if clause: to prevent doing these things for both pickup and first full measure
    {
      donefirst=true;
      level=0;
      indent();
      cvoicenum=voice+65;
      cstaffnum= staffInd+65;
      //there must be more elegant ways to do this, but whatever...
      staffname[staffInd].voicename[voice] = staffname[staffInd].partshort;
      staffname[staffInd].voicename[voice].append("voice");
      staffname[staffInd].voicename[voice].append(cstaffnum);
      staffname[staffInd].voicename[voice].append(cvoicenum);
      staffname[staffInd].voicename[voice].prepend("A");
      staffname[staffInd].voicename[voice].remove(QRegExp("[0-9]"));
      staffname[staffInd].voicename[voice].remove(QChar('.'));
      staffname[staffInd].voicename[voice].remove(QChar(' '));

      out << staffname[staffInd].voicename[voice];
      out << " = \\relative c" << relativ;
      indent();
      out << "{\n";
      level++;
      indent();
      if (voice==0)
	{
	  QString flatpartn="";
	  QString flatshortn="";

	  cout << "X" << staffname[staffInd].partname.toUtf8().data() << "x\n";

	  flatpartn = flatInInstrName(staffname[staffInd].partname);
	  flatshortn = flatInInstrName(staffname[staffInd].partshort);

	  out <<"\\set Staff.instrumentName = ";

	  cout << "F" << flatpartn.toUtf8().data() << "f\n";

	  if (flatpartn == "")
	    out<< "#\"" << staffname[staffInd].partname << "\"";
	  else
	    out << flatpartn;
	  out << "\n";

	  indent();
	  out << "\\set Staff.shortInstrumentName = ";
	  if (flatshortn =="")
	    out << "#\"" << staffname[staffInd].partshort << "\"";
	  else
	    out << flatshortn;
	  out << "\n";

	  indent();
	  writeClef(staff->clef(0));
	  indent();
	  out << "%staffkeysig\n";
	  indent();
	  //done in first measure anyway: ??
	  writeKeySig(staff->keymap()->key(0).accidentalType());
// 	  score->sigmap->timesig(0, z1, timedenom);
// 	  out << "\\time " << z1<< "/" << timedenom << " \n";
	}

      cout << "pianostaff: " << pianostaff << "\n";

      if (pianostaff==false)
	//voice settings does not work very well with pianostaffs. Use
	//\stemUp \stemNeutral \stemDown instead
	{
	  switch(voice)
	    {
	    case 0: break;
	      // we don't want voiceOne-specific behaviour if there is only one
	      // voice, so if there are more voices, we append "\voiceOne" later
	    case 1:
	      out <<"\\voiceTwo" <<"\n\n";
	      break;
	    case 2:
	      out <<"\\voiceThree" <<"\n\n";
	      break;
	    case 3:
	      out <<"\\voiceFour" <<"\n\n";
	      break;
	    }
	}

      //check for implicit startrepeat before first measure: (could
      //this be done in findvolta()?)
      i=0;
      while ((voltarray[i].voltart != startrepeat) and (voltarray[i].voltart != endrepeat)
	     and (voltarray[i].voltart !=bothrepeat) and (i<=lastind))
	{
	  i++;
	}

      if (i<=lastind)
	{
	  if ((voltarray[i].voltart==endrepeat) or (voltarray[i].voltart==bothrepeat))
	    {
	      indent();
	      out << "\\repeat volta 2 { \n";
	      repeatactive=true;
	    }
	}
    }// END if start of first measure

   if (wholemeasurerest < 1) indent();
   int tick = m->tick();
   int measuretick=0;
   Element* e;

   for(Segment* s = m->first(); s; s = s->next())
     {
       // for each segment in measure. Get element:
       int track = staffInd * VOICES + voice;
       e = s->element(track);

       if (!(e == 0 || e->generated()))
	 {
	   voiceActive[voice] = true;
	   barempty = false;
	 }
       else
         continue;

       handlePreInstruction(e); // Handle instructions which are to be printed before the element itself
       barlen=m->ticks();
       //handle element:
       switch(e->type())
	 {
	 case Element::CLEF:
	   if (wholemeasurerest >=1) writeMeasuRestNum();
	   writeClef(static_cast<Clef*>(e)->clefType());
	   indent();
	   break;
	 case Element::TIMESIG:
	   {
		 if (wholemeasurerest >=1)
		       writeMeasuRestNum();
		 out << "%bartimesig: \n";
		 writeTimeSig((TimeSig*)e);
		 out << "\n";

		 int nombarlen=z1*MScore::division;

		 if (timedenom==8) nombarlen=nombarlen/2;
		 if (timedenom == 2) nombarlen = 2*nombarlen;

		 if ((barlen<nombarlen) and (measurenumber==1) and (voice == 0))
		       {
			     pickup = true;
           partial = true;
			     indent();
			     const SigEvent ev(m->score()->sigmap()->timesig(m->tick()));
      	   out << "\\partial " << ev.timesig().denominator() << "*" << ev.timesig().numerator() << "\n";
		       }
		 curTicks=-1; //we always need explicit length after timesig.
		 indent();
		 break;
	   }
	 case Element::KEYSIG:
	     {
		 if (wholemeasurerest >=1) writeMeasuRestNum();

		 out << "%barkeysig: \n";
		 //this simple line did the job before mid-december
		 // 2009:

		 //writeKeySig(e->subtype());

		 //but then, some changes must have been made to
		 // keysig.cpp and .h I then stole (as usual) the code
		 // below from exportxml.cpp. It was, however marked
		 // with a "todo". The check for not end of keylist
		 // prevents some keychanges in the middle of the
		 // piece from being written, so I had to comment it
		 // out. (olav.)

		 KeySig* ksig= (KeySig*) e;
		 int keytick = ksig->tick();
		 cout << "at tick: " << keytick << "\n";
		 KeyList* kl = score->staff(staffInd)-> keymap();
		 KeySigEvent key = kl->key(keytick);
//		 auto ci = kl->find(keytick);
		 //
		 //		 if (ci != kl->end())
		 //     {
			 cout << "barkeysig: " << key.accidentalType() << " measureno: " << measurenumber << "\n";
			 indent();
			 writeKeySig(key.accidentalType());
		 //    }

		 indent();
		 curTicks=-1; //feels safe to force explicit length after keysig
		 break;
	     }
	 case Element::CHORD:
	     {
		 if (wholemeasurerest >=1) writeMeasuRestNum();
		 int ntick = static_cast<Chord*>(e)->tick() - tick;
		 if (ntick > 0)
		     {
			 writeRest(ntick, 2);//invisible rest: s
			 curTicks=-1;
		     }
		 tick += ntick;
		 measuretick=measuretick+ntick;
		 checkIfNextIsRest(mb, s, nextisrest, track);
		 writeChord((Chord*)e, nextisrest);
		 tick += ((Chord*)e)->actualTicks();
		 measuretick=measuretick+((Chord*)e)->actualTicks();
		 break;
	     }
	 case Element::REST:
	   {
	     bool articul=false;
	     findTuplets((ChordRest *) e);

	     QList<Articulation*> a;
	     ChordRest * CR = (ChordRest*) e;

	     a = CR->articulations();

	     if (!(a.isEmpty()) ) articul = true;

	     int l = ((Rest*)e)->actualTicks();
	     int mlen=((Rest*)e)->segment()->measure()->ticks();

	     int nombarl=z1*MScore::division;

	     if (((l==mlen) || (l==0)) and (mlen ==nombarl))  //l == 0 ??
	       {
		 if (wholemeasurerest > 0)
		   {
		     if (articul)
		       {
			 writeMeasuRestNum();
			 writeRest(l,0);
		         writeArticulation((ChordRest*) e);
		       }
		     else
		     wholemeasurerest++;
		   }
		 else
		   {
		     //wholemeasurerest: on fermata, output of * and start of new count.
		     l = ((Rest*)e)->segment()->measure()->ticks();
		     if (articul)
		       {
			 writeRest(l,0);
			 writeArticulation((ChordRest*) e);
		       }
		     else
		       writeRest(l, 1); //wholemeasure rest: R
		   }
	       }
	     else
	       {
		 if (wholemeasurerest >=1)
		   writeMeasuRestNum();
		 writeRest(l, 0);//ordinary rest: r
		 if (articul) writeArticulation((ChordRest*) e);
	       }
	     tick += l;
	     measuretick=measuretick+l;
	  } //end REST
	  break;
	case Element::MARKER:
	  qDebug("ordinary elements: Marker found\n");
	  break;
	case Element::BREATH:
	  out << "\\breathe ";
	  break;
	default:
	  //qDebug("Export Lilypond: unsupported element <%s>\n", e->name());
	  break;
	} // end switch elementtype

       handleElement(e); //check for instructions anchored to element e.

      if (tupletcount==-1)
	{
	  out << " } ";
	  tupletcount=0;
	}
    } //end for all segments

   barlen=m->ticks();
   if (barempty == true)
   // no stuff in this bar in this voice: fill empty bar with silent rest
    {
      if ((pickup) and (measurenumber==1) and (voice == 0))
	{

    const SigEvent ev(m->score()->sigmap()->timesig(m->tick()));
    out << "\\partial " << ev.timesig().denominator() << "*" << ev.timesig().numerator() << "\n";
	  indent();
	  writeRest(barlen,2);
	  out << "\n";
	}//end if pickup
      else //if not pickupbar: full measure silent bar
	{
	  writeRest(barlen, 2);
	  curTicks=-1;
	}
    }//end bar empty
   else // voice bar not empty
     {
       //we have to fill with spacer rests before and after nonsilent material
       if ((measuretick < barlen) and (measurenumber>0))
	 {
	   //fill rest of measure with silent rest
	   int negative=barlen-measuretick;
	   curTicks=-1;
	   writeRest(negative, 2);
	   curTicks=-1;
	 }
     }
   int mno;
   if (!partial)
     mno = measurenumber +1;
   else
     mno = measurenumber;
   writeVolta(mno, lastind);
} //end write VoiceMeasure



//---------------------------------------------------------
//   writeScore
//---------------------------------------------------------

void ExportLy::writeScore()
{
  // init of some fundamental variables
  firstalt=false;
  secondalt=false;
  tupletcount=0;
  char  cpartnum;
  chordpitch=41;
  repeatactive=false;
  staffInd = 0;
  graceswitch=false;
  int voice=0;
  cleannote="c";
  prevnote="c";
  gracecount=0;
  donefirst=false;
  lastJumpOrMarker = 0;
  initJumpOrMarkerLMs();
  wholemeasuretext = "";
  glisscount = 0;
  textspanswitch = false;
  textspannerdown=false;
  headOfLyrics = NULL;
  tailOfLyrics = NULL;
  privateRehearsalMark='A';


  foreach(Part* part, score->parts())
    {
      nextAnchor=0;
      initAnchors();
      resetAnchor(anker);

      int n = part->staves()->size();
      staffname[staffInd].partname  = part->longName().toPlainText();
      staffname[staffInd].partshort = part->shortName().toPlainText();
      curTicks=-1;
      pickup=false;

      if (part->nstaves()==2)
	pianostaff = true;
      else
	pianostaff = false;

      int strack = score->staffIdx(part) * VOICES;
      int etrack = strack + n* VOICES;

      buildInstructionListPart(strack, etrack);
      buildGlissandoList(strack,etrack);


      //ANCHORTEST: print instructionlist
      //      qDebug("anchortest\n");
      //   anchortest();
      //      qDebug("jumptest\n");
      //      jumptest(); segfaults!?!?

      foreach(Staff* staff, *part->staves())
	{

	  out << "\n";
    relativ="";
	  switch(staff->clef(0))
	    {
	    case ClefType::G:
	      relativ="'";
	      staffpitch=12*5;
	      break;
	    case ClefType::TAB:
	    case ClefType::PERC:
	    case ClefType::PERC2:
	    case ClefType::G3:
	    case ClefType::F:
	      relativ="";
	      staffpitch=12*4;
	      break;
	    case ClefType::G1:
	    case ClefType::G2:
	      relativ="''";
	      staffpitch=12*6;
	      break;
	    case ClefType::F_B:
	    case ClefType::F_C:
	    case ClefType::F8:
	      relativ=",";
	      staffpitch=12*3;
	      break;
	    case ClefType::F15:
	      relativ=",,";
	      staffpitch=12*2;
	      break;
	    case ClefType::C1:
	    case ClefType::C2:
	    case ClefType::C3:
	    case ClefType::C4:
	      relativ="'";
	      staffpitch=12*5;
	      break;
          default:      //??
            break;
	    }

	  staffrelativ=relativ;

	  cpartnum = staffInd + 65;
	  staffname[staffInd].staffid = staffname[staffInd].partshort;
	  staffname[staffInd].staffid.append("part");
	  staffname[staffInd].staffid.append(cpartnum);
	  staffname[staffInd].staffid.prepend("A");
	  staffname[staffInd].staffid.remove(QRegExp("[0-9]"));
	  staffname[staffInd].staffid.remove(QChar('.'));
	  staffname[staffInd].staffid.remove(QChar(' '));

	  findVolta();
	  //qDebug("voltatest\n");
	  //	  voltatest();

	  for (voice = 0; voice < VOICES; ++voice)  voiceActive[voice] = false;

	  for (voice = 0; voice < VOICES; ++voice)
	    {
	      prevpitch=staffpitch;
	      relativ=staffrelativ;
	      donefirst=false;
	      partial=0;

	      //for all measures in this voice:
	      for (MeasureBase* m = score->first(); m; m = m->next())
		{
		  if (m->type() != Element::MEASURE)
		    continue;

		  if (staffInd == 0)
		    findMarkerAtMeasureStart((Measure*) m );
		  //xxx		  else
		  //xxx		    printJumpOrMarker(measurenumber, true);

		  writeVoiceMeasure(m, staff, staffInd, voice); //really write the measure contents

		  if (staffInd == 0)
		    jumpAtMeasureStop( (Measure*) m);
		  //xxx else
		  //xxx printJumpOrMarker(measurenumber, false);
		}
	      level--;
	      indent();
	      out << "\\bar \"|.\" \n"; //thin-thick barline as last.
	      level=0;
	      indent();
	      out << "}% end of last bar in partorvoice\n\n";
	      if (voiceActive[voice])
		{
		  scorout<< voicebuffer;
		}
	      voicebuffer = " \n";
	    } // for voice 0 to VOICES

	  int voiceno=0;

	  for (voice = 0; voice < VOICES; ++voice)
	    if (voiceActive[voice]) voiceno++;

	  if (voiceno == 1)
	    staffname[staffInd].simultaneousvoices=false;

	  if (voiceno>1) //if more than one voice must be combined into one staff.
	    {
	      level=0;
	      indent();
	      out << staffname[staffInd].staffid << " =  << \n";
	      staffname[staffInd].simultaneousvoices=true;
	      level++;
	      indent();
	      out << "\\mergeDifferentlyHeadedOn\n";
	      indent();
              out << "\\mergeDifferentlyDottedOn \n";
	      ++level;

	      for (voice = 0; voice < voiceno; voice++)
		{
		  if (voiceActive[voice])
		    {
		      //have to go back to explicitly  naming the voices, so that
		      //it will be possible to attach lyrics to them.
		      indent();
		      out << "\\context Voice = " << staffname[staffInd].voicename[voice] ;
		      if ((voice == 0) and (pianostaff ==false))
			out << "{\\voiceOne ";
		      out << "\\" << staffname[staffInd].voicename[voice];
		      if ((voice == 0) and (pianostaff == false))
			out << "}";
		      if (voice < voiceno-1) out << "\\\\ \n";
		      else out <<"\n";
		    }
		}

	      indent();
	      out << ">> \n\n";
	      level=0;
	      indent();
	      scorout<< voicebuffer;
	      voicebuffer = " \n";
	    }
	  staffname[staffInd].numberofvoices=voiceno;
	  ++staffInd;
	}// end of foreach staff

      staffname[staffInd].staffid="laststaff";
      if (n > 1)
	{
	  --level;
	  indent();
	}
    }// end for each part
}// end of writeScore


//-------------------------------------------------------------------
// write score-block: combining parts and voices, drawing brackets and
// braces, at end of lilypond file
// -------------------------------------------------------------------
void ExportLy::writeScoreBlock()
{
  thisLyrics = headOfLyrics;

  if (nochord==false) // output the chords as a separate staff before the score-block
    {
      os << "theChords = \\chordmode { \n";
      printChordList();
      cleanupChordList();
      level--;
    }

  //  bracktest();

  level=0;
  os << "\n\\score { \n";
  level++;
  indentF();
  os << "<< \n";

  indx=0;
  while (staffname[indx].staffid!="laststaff")
    {
      if (lybracks[indx].brakstart)
	{
	  ++level;
	  indentF();
	  os << "\\context StaffGroup = " << (char)(lybracks[indx].brakno + 64) << "<< \n";
	}

      if (lybracks[indx].bracestart)
	{
	  ++level;
	  indentF();
	  if (lybracks[indx].piano)
	    {
	      os << "\\context PianoStaff <<\n";
	      indentF();
	      os << "\\set PianoStaff.instrumentName=\"Piano\" \n";
	      pianostaff=true;
	    }
	  else
	    os << "\\context GrandStaff = " << (char)(lybracks[indx].braceno + 64) << "<< \n";
	}

      if ((nochord == false) && (indx==0)) //insert chords as the first staff.
	{
	  indentF();
	  os << "\\new ChordNames { \\theChords } \n";
	}


      ++level;
      indentF();
      os << "\\context Staff = " << staffname[indx].staffid << " << \n";
      ++level;
      indentF();
      os << "\\";
      if (staffname[indx].simultaneousvoices)
	os << staffname[indx].staffid << "\n";
      else
	{
	  // have to reintroduce explicit naming of voices because of "\lyricsto"
	  os << "context Voice = "  << staffname[indx].voicename[0] << " \\";
	  os << staffname[indx].voicename[0] << "\n"; //voices are counted from 0.
	}

      if (lybracks[indx].piano)
	{
	  indentF();
	  os << "\\set Staff.instrumentName = #\"\"\n";
	  indentF();
	  os << "\\set Staff.shortInstrumentName = #\"\"\n";
	}

      --level;
      indentF();
      os << ">>\n\n"; // end of this staff

      connectLyricsToStaff();

      if (((lybracks[indx].brakstart) and (lybracks[indx].brakend)) or ((lybracks[indx].bracestart) and (lybracks[indx].braceend)))
	{
	  //if bracket or brace starts and ends on same staff: one-staff brace/bracket.
	  indentF();
	  os << "\\override StaffGroup.SystemStartBracket #'collapse-height = #1 \n";
	  indentF();
	  os << "\\override Score.SystemStartBar #'collapse-height = #1 \n";
	}

      if (lybracks[indx].brakend)
	{  --level;
	  indentF();
	  os << ">> %end of StaffGroup" << (char)(lybracks[indx].brakno + 64) << "\n\n";
	}
      if (lybracks[indx].braceend)
	{
	  --level;
	  indentF();
	  if (lybracks[indx].piano)
	    os << ">> %end of PianoStaff" << (char)(lybracks[indx].braceno + 64) << "\n";
	  else
	    os << ">> %end of GrandStaff" << (char)(lybracks[indx].braceno + 64) << "\n";
	}


      --level;
      ++indx;

    }//while still more staves

  cleanupLyrics();

  os << "\n";

  os << "\n"
  "      \\set Score.skipBars = ##t\n"
  "      %%\\set Score.melismaBusyProperties = #'()\n"
  "      \\override Score.BarNumber #'break-visibility = #end-of-line-invisible %%every bar is numbered.!!!\n"
  "      %% remove previous line to get barnumbers only at beginning of system.\n"
  "       #(set-accidental-style 'modern-cautionary)\n";
  if (rehearsalnumbers) os <<  "      \\set Score.markFormatter = #format-mark-box-numbers %%boxed rehearsal-numbers \n";
  else  os <<  "      \\set Score.markFormatter = #format-mark-box-letters %%boxed rehearsal-marks\n";
  if ((timedenom == 2) and (z1 == 2))
    {os << "%% "; }
  os << "       \\override Score.TimeSignature #'style = #'() %%makes timesigs always numerical\n"
  "      %% remove previous line to get cut-time/alla breve or common time \n";

os <<
  "      \\set Score.pedalSustainStyle = #'mixed \n"
  "       %% make spanners comprise the note it end on, so that there is no doubt that this note is included.\n"
  "       \\override Score.TrillSpanner #'(bound-details right padding) = #-2\n"
  "      \\override Score.TextSpanner #'(bound-details right padding) = #-1\n"
  "      %% Lilypond's normal textspanners are too weak:  \n"
  "      \\override Score.TextSpanner #'dash-period = #1\n"
  "      \\override Score.TextSpanner #'dash-fraction = #0.5\n"
  "      %% lilypond chordname font, like mscore jazzfont, is both far too big and extremely ugly (olagunde@start.no):\n"
  "      \\override Score.ChordName #'font-family = #'roman \n"
  "      \\override Score.ChordName #'font-size =#0 \n"
  "      %% In my experience the normal thing in printed scores is maj7 and not the triangle. (olagunde):\n"
  "      \\set Score.majorSevenSymbol = \\markup {maj7}\n"
  "  >>\n\n"
  "  %% Boosey and Hawkes, and Peters, have barlines spanning all staff-groups in a score,\n"
  "  %% Eulenburg and Philharmonia, like Lilypond, have no barlines between staffgroups.\n"
  "  %% If you want the Eulenburg/Lilypond style, comment out the following line:\n"
  "  \\layout {\\context {\\Score \\consists Span_bar_engraver}}\n"
  "}%% end of score-block \n\n";

  if (((pianostaff) and (indx==2)) or (indx < 2))
    os << "#(set-global-staff-size 20)\n";
  else if (indx > 2)
    os << "#(set-global-staff-size 14)\n";
}// end scoreblock



//-------------------------------------------------------------------------
//    writeLilyMacros
//-------------------------------------------------------------------------

void ExportLy::writeLilyMacros()
{
  if ((jumpswitch) || (ottvaswitch))
    {
      os<< " %%---------------MSCORE'S LILYPOND MACROS: -------------------------\n\n";
    }

  if (ottvaswitch)
    {
      os << " %%-----------------replacement for the \\ottava command--------------------\n\n";

      //The lilypond \ottava command moves the visual notes one octave
      //down, so that they will sound at their correct pitch when we
      //take account of the 8va instruction. Mscore adds the
      //8va-instruction and leave the notes in place on the staff. In
      //order to make the lilypond code exported from mscore reflect
      //mscore behavior, it was necessary to construct the macros \okt
      //and \oktend as substitutes for \ottava. A more elegant
      //solution would be to prevent lilypond's \ottava from temporarily
      //resetting the middleCPosition, but I did not understand how to
      //do that. (olav)

      os << "ottva =\n  "
	"{  %% for explanation, see mscore source file exportly.cpp \n"
	"   \\once\\override TextSpanner #'(bound-details left text) = \"8va\" \n"
	"   \\once\\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . -1) }\n"
	"   #(ly:export (make-event-chord (list (make-span-event 'TextSpanEvent START)))) \n"
	"}\n"
	"\n"

	"ottvaend ={ #(ly:export (make-event-chord (list (make-span-event 'TextSpanEvent STOP)))) \n"
	"   \\textSpannerNeutral} \n"

	"ottvabassa = \n"
	"{   \n"
	"   \\once \\override TextSpanner #'(bound-details left text) = \"8vb\"  \n"
	"   \\textSpannerDown \n"
        "   \\once \\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . 1) } \n"
	"   #(ly:export (make-event-chord (list (make-span-event 'TextSpanEvent START)))) \n"
	"}\n"
	"\n"

	"%%------------------end ottava macros ---------------------\n\n";
 }// end of if ottva


  if (jumpswitch)
    {
      os << "   %%------------------coda---segno---macros--------------------\n"

	"   %%                 modified from lsr-snippets. Work in progress:       \n"

	"   %% These macros presupposes a difference between the use of the       \n"
	"   %% Coda-sign telling us to jump to the coda (\\gotocoda), and the   \n"
	"   %% Coda-sign telling us that this is actually the Coda (\\theCoda).  \n"
	"   %% This goes well if you use the mscore text: \"To Coda\" as a mark of \n"
	"   %% of where to jump from, and the codawheel as the mark of where to jump to\n"
	"   %% Otherwise (using codawheel for both) you have to edit the lilypond-file by hand.\n"

	"   gotocoda     = \\mark \\markup {\\musicglyph #\"scripts.coda\"}               \n"
	"   thecodasign  = \\mark \\markup {\\musicglyph #\"scripts.coda\" \"Coda\"}     \n"
	"   thesegno     = \\mark \\markup {\\musicglyph #\"scripts.segno\"}              \n"
	"   varcodasign  = \\mark \\markup {\\musicglyph #\"scripts.varcoda\"}            \n"
	"   Radjust      =  \\once \\override Score.RehearsalMark #'self-alignment-X = #RIGHT \n"
	"   blankClefKey = {\\once \\override Staff.KeySignature #'break-visibility = #all-invisible \n"
	"		    \\once \\override Staff.Clef #'break-visibility = #all-invisible   \n"
	"                 } \n"
	"   codetta     = {\\mark \\markup \\line {\\musicglyph #\"scripts.coda\" \\hspace #-1.3 \\musicglyph #\"scripts.coda\"} } \n"
	"   fine        = {\\Radjust \\mark \\markup {\"Fine\"} \\mark \\markup {\\musicglyph #\"scripts.ufermata\" } \n"
	"		  \\bar \"||\" } \n"
	"   DCalfine    = {\\Radjust \\mark \\markup {\"D.C. al fine\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn } \n"
	"   DCalcoda    = {\\Radjust \\mark \\markup {\"D.C. al coda\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn }  \n"
	"   DSalfine    = {\\Radjust \\mark \\markup {\"D.S. al fine\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn } \n"
	"   DSalcoda    = {\\Radjust \\mark \\markup {\"D.S. al coda\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn } \n"
	"   showClefKey = {\\once \\override Staff.KeySignature #'break-visibility = #all-visible \n"
	"               \\once \\override Staff.Clef #'break-visibility = #all-visible \n"
	"		 } \n"
	"   resumeStaff = {\\cadenzaOff \\startStaff % Resume bar count and show staff lines again \n"
	"		  \\partial 32 s32 % Add a whee bit of staff before the clef! \n"
	"		  \\bar \"\" \n"
	"		 } \n"
	"   %%   whitespace between D.S./D.C. and the Coda: \n"
	"   codaspace = {\\repeat unfold 2 {s4 s4 s4 s4 \\noBreak \\bar \"\" }}  \n"
	"   theCoda   = {\\noBreak \\codaspace \\resumeStaff \\showClefKey \\thecodasign} \n"

	" %% -------------------end-of-coda-segno-macros------------------  \n\n ";
    }

 if ((jumpswitch) || ottvaswitch)
   {
     os << "%% --------------END MSCORE LILYPOND-MACROS------------------------\n\n\n\n\n";
   }
} //end of writelilymacros



//-------------------------------------------------------------
//   writeLilyHeader
//-------------------------------------------------------------
void ExportLy::writeLilyHeader()
{
  os << "%=============================================\n"
    "%   created by MuseScore Version: " << VERSION << "\n"
    "%          " << QDate::currentDate().toString(Qt::SystemLocaleLongDate);
  os << "\n";
  os <<"%=============================================\n"
    "\n"
    "\\version \"2.12.0\"\n\n";     // target lilypond version

  os << "\n\n";
}

  //---------------------------------------------------
  //    Page format
  //---------------------------------------------------
void ExportLy::writePageFormat()
{
  const PageFormat* pf = score->pageFormat();
  os << "#(set-default-paper-size ";
      os << "\"" << QString(pf->paperSize()->name).toLower() << "\"";

  if (pf->size().width() > pf->size().height()) os << " 'landscape";

  os << ")\n\n";

  // TODO/O.G.: choose between standard formats and specified paper
  // dimensions. We normally don't need both.

  double lw = pf->printableWidth();
  os << "\\paper {\n";
  os <<  "  line-width    = " << lw * INCH << "\\mm\n";
  os <<  "  left-margin   = " << pf->evenLeftMargin() * INCH << "\\mm\n";
  os <<  "  top-margin    = " << pf->evenTopMargin() * INCH << "\\mm\n";
  os <<  "  bottom-margin = " << pf->evenBottomMargin() * INCH << "\\mm\n";
  os <<  "  %%indent = 0 \\mm \n";
  os <<  "  %%set to ##t if your score is less than one page: \n";
  os <<  "  ragged-last-bottom = ##t \n";
  os <<  "  ragged-bottom = ##f  \n";
  os <<  "  %% in orchestral scores you probably want the two bold slashes \n";
  os <<  "  %% separating the systems: so uncomment the following line: \n";
  os <<  "  %% system-separator-markup = \\slashSeparator \n";
  os <<  "  }\n\n";
}//end writepageformat



//---------------------------------------------------
//    writeScoreTitles
//---------------------------------------------------
void ExportLy::writeScoreTitles()
{
#if 0 //TODOws

  os << "\\header {\n";

  ++level;
  const MeasureBase* m = score->first();
  foreach(const Element* e, *m->el()) {
    if (e->type() != TEXT)
      continue;
    QString s = ((Text*)e)->text();
    indentF();
    switch(e->subtype()) {
    case TEXT_TITLE:
      os << "title = ";
      break;
    case TEXT_SUBTITLE:
      os << "subtitle = ";
      break;
    case TEXT_COMPOSER:
      os << "composer = ";
      break;
    case TEXT_POET:
      os << "poet = ";
      break;
    default:
      qDebug("text-type %d not supported\n", e->subtype());
      os << "subtitle = ";
      break;
    }
    os << "\"" << s << "\"\n";
  }

  if (!score->metaTag("Copyright").isEmpty())
    {
      indentF();
      os << "copyright = \"" << score->metaTag("Copyright") << "\"\n";
    }

  indentF();
  os << "}\n";
#endif
}// end writeScoreTitles



//---------------------------------------------------------
//---------------------------------------------------------
//  WRITE  =  the main function of exportly
//---------------------------------------------------------
//---------------------------------------------------------

bool ExportLy::write(const QString& name)
{
  //init of some fundamental variables.
  pianostaff=false;
  rehearsalnumbers=false;
  wholemeasurerest=0;
  f.setFileName(name);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  os.setDevice(&f);
  os.setCodec("utf8");
  out.setCodec("utf8");
  out.setString(&voicebuffer);
  voicebuffer = "";
  scorout.setCodec("utf8");
  scorout.setString(&scorebuffer);
  scorebuffer = "";
  chordHead=NULL;
  chordcount = 0;
  slurstack=0;
  phraseslur=0;
  ottvaswitch = false;
  jumpswitch = false;
  nochord = true;

  writeLilyHeader();

  writeScore();

  findLyrics();

  writeLilyMacros();

  writePageFormat();

  writeScoreTitles();

  findBrackets();

  os << scorebuffer;
  scorebuffer = "";

  writeLyrics();

  writeScoreBlock();

  f.close();
  return f.error() == QFile::NoError;
}// end of function "write"





/*----------------------- NEWS and HISTORY:--------------------  */

/*

  02.feb 2010. If \voiceOne etc. is used in pianostaff, articulation
  signs are either placed on only above or only below staff, and not
  in any reasonable connection with the notehead. So \voiceOne etc. is
  no longer used in pianostaff.

  27.dec.09 Two (and not more than two) dynamic signs on the same
  note.

  26.dec.09 Fixed bug in triplets of chords. Tried to update outdated
  code on keychanges.

  23,dec.09 Flat symbol in instrumentnames are now handled. Some
  progress on reconciling rehearsalmarks and segno/coda-symbols.

  17. dec.09 Dynamics and text can now be connected to the same note.

   09.dec.09 Fermatas on rests (wholemeasure and others). Fixed bugs
  in repeats/doblebars and in wholemeasurerests caused by pickupbar

  20.nov.2009  Tried to repair reported crash on file Cronicas.mscz

  7.nov. 2009: Lyrics. Works reasonably well on demo adeste.

   1.nov. lefthandposition (roman numbers with line: violin, guitar),
   trill, pedal and general lines with/out text.

   30.oct. Unterminated slurs: \laissezVibrer. Whole notes as part of
   triplets. Flageolets as symbol connected to the note.

   28.oct. Arpeggios and glissandos. Fixed issue of 6.may in the issue
   tracker: incorrect export of polyphony.

   25.oct. Implemented fingering and guitar string-number

   24.oct Support for metronome marks.

   22.oct  conditional output of exportly's lilypond macros (\okt and
     \segno). bugfix for repeats.

   13.oct fixed grace-note-troubles on demos: golliwogg, and troubles
   with wholemeasure rests in the shifting timesignatures in
   promenade. Started on lilypond \chordmode

   08.okt.  (olav) Tremolo. Segno and Coda. Correct insertion of s-rests
          in demo: adeste.

   01.oct. 2009 (Olav) Improved export of whole measure rests.

   29.sep.2009 (Olav) Rudiments of new 8va. Bugfix for repeats. Some
   support for Segno/Coda.

   12.sep.2009 (Olav) Improved export of rehearsalmarks.

   17.aug.2009 (db) add quotes around unparsed markup (since it can
   contain special characters), commented out the indent=0, fix spelling
   mistake for octave markings ("set-octaviation"), fix type of ottava

   mar. 2009 always explicit end-bar -> no need to declare last bar as
   incomplete, but writes two end-bars when last bar is complete. This
   doesn't show in print.

   12.feb.2009. - Removed bug: double instrument definitions in pieces
   with pickup-measure (prev'ly defined both in measure 0 and measure
   1). - Removed bug: nonrecognition of startrepeats. -Improved
   recognition of whole-measure rests.

   NEW 5.feb.2009: separated grandstaff (variable distance between staffs) from
   pianostaff: constant distance between staffs to prepare for cross-staff
   beaming (not implemented). Brackets/braces for single staffs.

   NEW 25.jan.2009: system brackets and braces for simple scores.
   Unsolved complications for multistaff instruments (piano, organ,
   harp), and for bracketing single staffs.

   NEW 22.jan. 2009
   -- fixed a problem with beams on grace-notes, and
   some faults produced by the previous revision of exportly.

   NEW 18. jan. 2009
   -- Now avoids export of empty voices.

   DELETED HISTORY PRE 2009.

*/


/*----------------------TODOS------------------------------------


      -- Coda/Segno symbols collides with rehearsalmarks, which
      accordingly are not printed. Lilypond has automatic
      incrementation of rehearsalmarks. It is easy to input values to
      the variable in which the mark is stored. But I have not
      succeeded in finding an easy way to extract this
      value. Lilyponds \mark\default, which write the automatically
      incremented rehearsalmark is not reconcilable with segnos and
      coda, except thru very complex macros, which will make the
      exported lilypond code very ugly and not very perspicuous. If it
      was easy to extract the value of the reharsalmark variable from
      Lilypond, these macros would not be necessary. Because they
      clutter the lilypond-code I do not want to use them. I
      can then increment the rehearsalmarks in exportly.cpp, or I can
      extract the values of the rehearsalmarks in mscore. As mscore's
      manual insertion of rehearsalmarks is less elegant than than
      automatic incrementation, I chose to make exportly.cpp increment
      the rehearsalmarks. This gives the cleanest lilypond-code.

      -- all kinds of symbols at the notelevel. More symbols on the
     measurelevel

      -- odd noteheads and percussion staffs.  See output from noteedit.

      -- breaks and spacers

      -- accordion symbols.

      -- dotted rests in timesignatures which do not subdivide in 3
         (like 6/8, 12/8) are plain and simply wrong, and must be made
         impossible: translate to two separate rests.

   -- become clear on the difference between system text and staff
      text.

   -- octave-trouble in golliwogg.

   -- provide for more than one pianostaff in a score.

   -- Determine whether text goes above or below staff.

   -- correct export of chordsymbols: many faults.

   -- cross-staff beaming in pianostaff cross-voice slurs!?!?!? seems
      _very_ complex to implement (example demos:promenade, bar 6)
      Will \partcombine do it?


   -- difficult problem with hairpins: Beginning of hairpin and
   -- end of hairpin are anchored to different notes. This is done
   -- automatically when you drop an hairpin in the appropriate place
   -- in the score. exportly find these anchors and insert \< and \!
   -- after these notes. But the start of the hairpin protrudes to the
   -- left of the anchor. And often the end of the hairpin is anchored
   -- to a note which is too far to the right. The placement of the
   -- lily-symbols must take regard for the placement on the canvas
   -- and not to the anchorpoints alone. Check the procedure in the
   -- main program to see how the anchorpoints and the canvas-position
   -- is made and compensate for this when exporting the
   -- lily-symbols. -- check \set Score.hairpinToBarline = ##t

   -- close second volta: doesn't seem possible for single and
      thin-thin double barlines

   --Collisions in crowded multi-voice staffs
     (e.g. cello-suite). check \override RestCollision
     #'positioning-done = #'merge-rests-on-positioning. Use
     \partcombine instead of ord. polyphony

   -- General tuplets massive failure on demos: prelude_sr.mscz

   -- Many of the demos have voice 2 as the upper one =>
      trouble. Exportly.cpp must be made to identify the uppermost
      voice as lilypond voice 1, whatever number it has in
      mscore. will \partcombine resolve this?

  -- Markups belonging to a multimeasure rest should be
     left-adjusted to the left barline, and not centered over the
     rest. No good solutions found.

  -- Bug in lilypond. See notation reference 1.2.6 for 2.12.1:
     gracenotes. At the end: issues and warnings: "Grace note
     synchronization can also lead to surprises. Staff notation, such
     as key signatures, bar lines, etc., are also synchronized. Take
     care when you mix staves with grace notes and staves without, for
     example, ....This can be remedied by inserting grace skips of the
     corresponding durations in the other staves." In earlier editions
     of the manual, this is rightly described as a bug. I am awaiting
     the correction of this in lilypond, which, given the promotion
     from bug to "issue", probably will be never, and I will not correct
     for it here. (olav)
 */
}


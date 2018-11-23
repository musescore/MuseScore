//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of class Score (partial).
*/

#include <assert.h>
#include "score.h"
#include "key.h"
#include "sig.h"
#include "clef.h"
#include "tempo.h"
#include "measure.h"
#include "page.h"
#include "undo.h"
#include "system.h"
#include "select.h"
#include "segment.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "slur.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "tuplet.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "line.h"
#include "volta.h"
#include "repeat.h"
#include "ottava.h"
#include "barline.h"
#include "box.h"
#include "utils.h"
#include "excerpt.h"
#include "stafftext.h"
#include "repeatlist.h"
#include "keysig.h"
#include "beam.h"
#include "stafftype.h"
#include "tempotext.h"
#include "articulation.h"
#include "revisions.h"
#include "tiemap.h"
#include "layoutbreak.h"
#include "harmony.h"
#include "mscore.h"
#ifdef OMR
#include "omr/omr.h"
#endif
#include "bracket.h"
#include "audio.h"
#include "instrtemplate.h"
#include "cursor.h"
#include "sym.h"
#include "rehearsalmark.h"
#include "breath.h"
#include "instrchange.h"

namespace Ms {

Score* gscore;                 ///< system score, used for palettes etc.

bool scriptDebug     = false;
bool noSeq           = false;
bool noMidi          = false;
bool midiInputTrace  = false;
bool midiOutputTrace = false;
bool showRubberBand  = true;

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

MeasureBaseList::MeasureBaseList()
      {
      _first = 0;
      _last  = 0;
      _size  = 0;
      };

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void MeasureBaseList::push_back(MeasureBase* e)
      {
      ++_size;
      if (_last) {
            _last->setNext(e);
            e->setPrev(_last);
            e->setNext(0);
            }
      else {
            _first = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _last = e;
      }

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void MeasureBaseList::push_front(MeasureBase* e)
      {
      ++_size;
      if (_first) {
            _first->setPrev(e);
            e->setNext(_first);
            e->setPrev(0);
            }
      else {
            _last = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _first = e;
      }

//---------------------------------------------------------
//   add
//    insert e before e->next()
//---------------------------------------------------------

void MeasureBaseList::add(MeasureBase* e)
      {
      MeasureBase* el = e->next();
      if (el == 0) {
            push_back(e);
            return;
            }
      if (el == _first) {
            push_front(e);
            return;
            }
      ++_size;
      e->setPrev(el->prev());
      el->prev()->setNext(e);
      el->setPrev(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* el)
      {
      --_size;
      if (el->prev())
            el->prev()->setNext(el->next());
      else
            _first = el->next();
      if (el->next())
            el->next()->setPrev(el->prev());
      else
            _last = el->prev();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void MeasureBaseList::insert(MeasureBase* fm, MeasureBase* lm)
      {
      ++_size;
      for (MeasureBase* m = fm; m != lm; m = m->next())
            ++_size;
      MeasureBase* pm = fm->prev();
      if (pm)
            pm->setNext(fm);
      else
            _first = fm;
      MeasureBase* nm = lm->next();
      if (nm)
            nm->setPrev(lm);
      else
            _last = lm;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* fm, MeasureBase* lm)
      {
      --_size;
      for (MeasureBase* m = fm; m != lm; m = m->next())
            --_size;
      MeasureBase* pm = fm->prev();
      MeasureBase* nm = lm->next();
      if (pm)
            pm->setNext(nm);
      else
            _first = nm;
      if (nm)
            nm->setPrev(pm);
      else
            _last = pm;
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void MeasureBaseList::change(MeasureBase* ob, MeasureBase* nb)
      {
      nb->setPrev(ob->prev());
      nb->setNext(ob->next());
      if (ob->prev())
            ob->prev()->setNext(nb);
      if (ob->next())
            ob->next()->setPrev(nb);
      if (ob == _last)
            _last = nb;
      if (ob == _first)
            _first = nb;
      if (nb->type() == Element::Type::HBOX || nb->type() == Element::Type::VBOX
         || nb->type() == Element::Type::TBOX || nb->type() == Element::Type::FBOX)
            nb->setSystem(ob->system());
      foreach(Element* e, nb->el())
            e->setParent(nb);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Score::init()
      {
      _linkId         = 0;
      _currentLayer   = 0;
      _playMode       = PlayMode::SYNTHESIZER;
      Layer l;
      l.name          = "default";
      l.tags          = 1;
      _layer.append(l);
      _layerTags[0]   = "default";

      if (!_parentScore) {
#if defined(Q_OS_WIN)
            _metaTags.insert("platform", "Microsoft Windows");
#elif defined(Q_OS_MAC)
            _metaTags.insert("platform", "Apple Macintosh");
#elif defined(Q_OS_LINUX)
            _metaTags.insert("platform", "Linux");
#else
            _metaTags.insert("platform", "Unknown");
#endif
            _metaTags.insert("movementNumber", "");
            _metaTags.insert("movementTitle", "");
            _metaTags.insert("workNumber", "");
            _metaTags.insert("workTitle", "");
            _metaTags.insert("arranger", "");
            _metaTags.insert("composer", "");
            _metaTags.insert("lyricist", "");
            _metaTags.insert("poet", "");
            _metaTags.insert("translator", "");
            _metaTags.insert("source", "");
            _metaTags.insert("copyright", "");
            _metaTags.insert("creationDate", QDate::currentDate().toString(Qt::ISODate));
            _undo       = new UndoStack();
            _repeatList = new RepeatList(this);
            }
      else {
            _undo = 0;
            _repeatList = 0;
            }

      _revisions = new Revisions;
      _scoreFont = ScoreFont::fontFactory("emmentaler");

      _pageNumberOffset = 0;

      _mscVersion             = MSCVERSION;
      _created                = false;

      _updateAll              = true;
      _layoutAll              = true;
      layoutFlags             = 0;
      _undoRedo               = false;
      _playNote               = false;
      _playChord              = false;
      _excerptsChanged        = false;
      _instrumentsChanged     = false;
      _selectionChanged       = false;

      keyState                = 0;
      _showInvisible          = true;
      _showUnprintable        = true;
      _showFrames             = true;
      _showPageborders        = false;
      _showInstrumentNames    = true;
      _showVBox               = true;

      _printing               = false;
      _playlistDirty          = true;
      _autosaveDirty          = true;
      _saved                  = false;
      _pos[int(POS::CURRENT)] = 0;
      _pos[int(POS::LEFT)]    = 0;
      _pos[int(POS::RIGHT)]   = 0;
      _fileDivision           = MScore::division;
      _defaultsRead           = false;
      _omr                    = 0;
      _audio                  = 0;
      _showOmr                = false;
      _sigmap                 = 0;
      _tempomap               = 0;
      _layoutMode             = LayoutMode::PAGE;
      _noteHeadWidth          = 0.0;      // set in doLayout()
      _midiPortCount          = 0;
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
   : QObject(0), ScoreElement(this), _is(this), _selection(this), _selectionFilter(this)
      {
      _parentScore = 0;
      init();
      _tempomap = new TempoMap;
      _sigmap   = new TimeSigMap();
      _style    = *(MScore::defaultStyle());
      accInfo = tr("No selection");
      }

Score::Score(const MStyle* s)
   : ScoreElement(this), _is(this), _selection(this), _selectionFilter(this)
      {
      _parentScore = 0;
      init();
      _tempomap = new TempoMap;
      _sigmap   = new TimeSigMap();
      _style    = *s;
      accInfo = tr("No selection");
      }

//
//  a linked score shares some properties with parentScore():
//    _undo
//    _sigmap
//    _tempomap
//    _repeatList
//    _links
//    _staffTypes
//    _metaTags
//

Score::Score(Score* parent)
   : ScoreElement(this), _is(this), _selection(this), _selectionFilter(this)
      {
      _parentScore = parent;
      init();

      if (MScore::defaultStyleForParts())
            _style = *MScore::defaultStyleForParts();
      else {
            // inherit most style settings from parent
            _style = *parent->style();
            // but borrow defaultStyle page layout settings
            const PageFormat* pf = MScore::defaultStyle()->pageFormat();
            qreal sp = MScore::defaultStyle()->spatium();
            _style.setPageFormat(*pf);
            _style.setSpatium(sp);

            //concert pitch is off for parts
            _style.set(StyleIdx::concertPitch, false);
            }

      _synthesizerState = parent->_synthesizerState;
      accInfo = tr("No selection");
      }

Score::Score(Score* parent, const MStyle* s)
   : ScoreElement(this), _is(this), _selection(this), _selectionFilter(this)
      {
      _parentScore = parent;
      init();
      _style    = *s;
      accInfo = tr("No selection");
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
      _midiPortCount = 0;
      foreach(MuseScoreView* v, viewer)
            v->removeScore();
      // deselectAll();
      for (MeasureBase* m = _measures.first(); m;) {
            MeasureBase* nm = m->next();
            delete m;
            m = nm;
            }
      foreach(Part* p, _parts)
            delete p;
      foreach(Staff* staff, _staves)
            delete staff;
      foreach(System* s, _systems)
            delete s;
      foreach(Page* page, _pages)
            delete page;
      foreach(Excerpt* e, _excerpts)
            delete e;
      delete _revisions;
      delete _undo;           // this also removes _undoStack from Mscore::_undoGroup
      delete _tempomap;
      delete _sigmap;
      delete _repeatList;
      }

//---------------------------------------------------------
//   elementAdjustReadPos
//---------------------------------------------------------

static void elementAdjustReadPos(void*, Element* e)
      {
      if (e->isMovable())
            e->adjustReadPos();
      }

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

void Score::addMeasure(MeasureBase* m, MeasureBase* pos)
      {
      m->setNext(pos);
      _measures.add(m);
      }

//---------------------------------------------------------
//    fixTicks
//    update:
//      - measure ticks
//      - tempo map
//      - time signature map
//---------------------------------------------------------

/**
 This is needed after
      - inserting or removing a measure
      - changing the sigmap
      - after inserting/deleting time (changes the sigmap)
*/

void Score::fixTicks()
      {
      int tick = 0;
      Measure* fm = firstMeasure();
      if (fm == 0)
            return;

      for (Staff* staff : _staves)
            staff->clearTimeSig();

      TimeSigMap* smap = sigmap();
      Fraction sig(fm->len());
      Fraction nsig(fm->timesig());

      if (!parentScore()) {
            tempomap()->clear();
            smap->clear();
            smap->add(0, SigEvent(sig,  nsig, 0));
            }

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != Element::Type::MEASURE) {
                  mb->setTick(tick);
                  continue;
                  }
            Measure* m       = static_cast<Measure*>(mb);
            int mtick        = m->tick();
            int diff         = tick - mtick;
            int measureTicks = m->ticks();
            m->moveTicks(diff);
            if (m->mmRest())
                  m->mmRest()->moveTicks(diff);

            // There is a single tempo map in root score, setPause only in rootScore
            //
            //  implement section break rest
            //
            if (!parentScore() && m->sectionBreak() && m->pause() != 0.0)
                  setPause(m->tick() + m->ticks(), m->pause());

            //
            // implement fermata as a tempo change
            //

            for (Segment* s = m->first(); s; s = s->next()) {
                  if (!parentScore() && s->segmentType() == Segment::Type::Breath) {
                        qreal length = 0.0;
                        int tick = s->tick();
                        // find longest pause
                        for (int i = 0, n = ntracks(); i < n; ++i) {
                              Element* e = s->element(i);
                              if (e && e->type() == Element::Type::BREATH) {
                                    Breath* b = static_cast<Breath*>(e);
                                    length = qMax(length, b->pause());
                                    }
                              }
                        if (length != 0.0)
                              setPause(tick, length);
                        }
                  else if (s->segmentType() == Segment::Type::TimeSig) {
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              TimeSig* ts = static_cast<TimeSig*>(s->element(staffIdx * VOICES));
                              if (ts)
                                    staff(staffIdx)->addTimeSig(ts);
                              }
                        }
                  else if (!parentScore() && (s->segmentType() == Segment::Type::ChordRest)) {
                        foreach (Element* e, s->annotations()) {
                              if (e->type() == Element::Type::TEMPO_TEXT) {
                                    const TempoText* tt = static_cast<const TempoText*>(e);
                                    setTempo(tt->segment(), tt->tempo());
                                    }
                              }
                        qreal stretch = 0.0;
                        for (int i = 0; i < s->elist().size(); ++i) {
                              Element* e = s->elist().at(i);
                              if (!e)
                                    continue;
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              int nn = cr->articulations().size();
                              for (int ii = 0; ii < nn; ++ii)
                                    stretch = qMax(cr->articulations().at(ii)->timeStretch(), stretch);
                              if (stretch != 0.0 && stretch != 1.0) {
                                    qreal otempo = tempomap()->tempo(cr->tick());
                                    qreal ntempo = otempo / stretch;
                                    setTempo(cr->tick(), ntempo);
                                    int etick = cr->tick() + cr->actualTicks() - 1;
                                    auto e = tempomap()->find(etick);
                                    if (e == tempomap()->end())
                                          setTempo(etick, otempo);
                                    break;
                                    }
                              }
                        }
                  }

            // update time signature map
            // create event if measure len and time signature are different
            // even if they are equivalent 4/4 vs 2/2
            if (!parentScore() && (!sig.identical(m->len()) || !nsig.identical(m->timesig()))) {
                  sig = m->len();
                  nsig = m->timesig();
                  smap->add(tick, SigEvent(sig, nsig,  m->no()));
                  }

            tick += measureTicks;
            }
      if (tempomap()->empty())
            tempomap()->setTempo(0, 2.0);
      }

//---------------------------------------------------------
//   validSegment
//---------------------------------------------------------

static bool validSegment(Segment* s, int startTrack, int endTrack)
      {
      for (int track = startTrack; track < endTrack; ++track) {
            if (s->element(track))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   pos2measure
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.
*/

MeasureBase* Score::pos2measure(const QPointF& p, int* rst, int* pitch,
   Segment** seg, QPointF* offset) const
      {
      Measure* m = searchMeasure(p);
      if (m == 0)
            return 0;

      System* s = m->system();
      qreal y   = p.y() - s->canvasPos().y();

      int i;
      for (i = 0; i < nstaves();) {
            SysStaff* stff = s->staff(i);
            if (!stff->show() || !staff(i)->show()) {
                  ++i;
                  continue;
                  }
            int ni = i;
            for (;;) {
                  ++ni;
                  if (ni == nstaves() || (s->staff(ni)->show() && staff(ni)->show()))
                        break;
                  }

            qreal sy2;
            if (ni != nstaves()) {
                  SysStaff* nstaff = s->staff(ni);
                  qreal s1y2 = stff->bbox().y() + stff->bbox().height();
                  sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                  }
            else
                  sy2 = s->page()->height() - s->pos().y();   // s->height();
            if (y > sy2) {
                  i   = ni;
                  continue;
                  }
            break;
            }

      // search for segment + offset
      QPointF pppp = p - m->canvasPos();
      int strack = i * VOICES;
      if (!staff(i))
            return 0;
      int etrack = staff(i)->part()->nstaves() * VOICES + strack;

      SysStaff* sstaff = m->system()->staff(i);
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* segment = m->first(st); segment; segment = segment->next(st)) {
            if (!validSegment(segment, strack, etrack))
                  continue;
            Segment* ns = segment->next(st);
            for (; ns; ns = ns->next(st)) {
                  if (validSegment(ns, strack, etrack))
                        break;
                  }
            if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                  *rst = i;
                  if (pitch) {
                        Staff* s = _staves[i];
                        ClefType clef = s->clef(segment->tick());
                        *pitch = y2pitch(pppp.y() - sstaff->bbox().y(), clef, s->spatium());
                        }
                  if (offset)
                        *offset = pppp - QPointF(segment->x(), sstaff->bbox().y());
                  if (seg)
                        *seg = segment;
                  return m;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   staffIdx
//
///  Return index for the first staff of \a part.
//---------------------------------------------------------

int Score::staffIdx(const Part* part) const
      {
      int idx = 0;
      foreach(Part* p, _parts) {
            if (p == part)
                  break;
            idx += p->nstaves();
            }
      return idx;
      }

//---------------------------------------------------------
//   setShowInvisible
//---------------------------------------------------------

void Score::setShowInvisible(bool v)
      {
      _showInvisible = v;
      rebuildBspTree();
      _updateAll = true;
      end();
      }

//---------------------------------------------------------
//   setShowUnprintable
//---------------------------------------------------------

void Score::setShowUnprintable(bool v)
      {
      _showUnprintable = v;
      _updateAll      = true;
      end();
      }

//---------------------------------------------------------
//   setShowFrames
//---------------------------------------------------------

void Score::setShowFrames(bool v)
      {
      _showFrames = v;
      _updateAll  = true;
      end();
      }

//---------------------------------------------------------
//   setShowPageborders
//---------------------------------------------------------

void Score::setShowPageborders(bool v)
      {
      _showPageborders = v;
      _updateAll = true;
      end();
      }

//---------------------------------------------------------
//   dirty
//---------------------------------------------------------

bool Score::dirty() const
      {
      return !undo()->isClean();
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell()
      {
      for (int i = 0; i < nstaves(); ++i) {
            QList<Note*> notes;
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  int strack = i * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e && e->type() == Element::Type::CHORD)
                              notes.append(static_cast<Chord*>(e)->notes());
                        }
                  }
            spellNotelist(notes);
            }
      }

void Score::spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment)
      {
      for (int i = startStaff; i < endStaff; ++i) {
            QList<Note*> notes;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next()) {
                  int strack = i * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e && e->type() == Element::Type::CHORD)
                              notes.append(static_cast<Chord*>(e)->notes());
                        }
                  }
            spellNotelist(notes);
            }
      }

//---------------------------------------------------------
//   prevNote
//---------------------------------------------------------

Note* prevNote(Note* n)
      {
      Chord* chord = n->chord();
      Segment* seg = chord->segment();
      const QList<Note*> nl = chord->notes();
      int i = nl.indexOf(n);
      if (i)
            return nl[i-1];
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() - 1;
      int endTrack   = 0;
      while (seg) {
            if (seg->segmentType() == Segment::Type::ChordRest) {
                  for (int track = startTrack; track >= endTrack; --track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == Element::Type::CHORD)
                              return static_cast<Chord*>(e)->upNote();
                        }
                  }
            seg = seg->prev1();
            startTrack = staff * VOICES + VOICES - 1;
            }
      return n;
      }

//---------------------------------------------------------
//   nextNote
//---------------------------------------------------------

Note* nextNote(Note* n)
      {
      Chord* chord = n->chord();
      const QList<Note*> nl = chord->notes();
      int i = nl.indexOf(n);
      ++i;
      if (i < nl.size())
            return nl[i];
      Segment* seg   = chord->segment();
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() + 1;
      int endTrack   = staff * VOICES + VOICES;
      while (seg) {
            if (seg->segmentType() == Segment::Type::ChordRest) {
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == Element::Type::CHORD) {
                              return ((Chord*)e)->downNote();
                              }
                        }
                  }
            seg = seg->next1();
            startTrack = staff * VOICES;
            }
      return n;
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell(Note* note)
      {
      QList<Note*> notes;

      notes.append(note);
      Note* nn = nextNote(note);
      notes.append(nn);
      nn = nextNote(nn);
      notes.append(nn);
      nn = nextNote(nn);
      notes.append(nn);

      nn = prevNote(note);
      notes.prepend(nn);
      nn = prevNote(nn);
      notes.prepend(nn);
      nn = prevNote(nn);
      notes.prepend(nn);

      int opt = Ms::computeWindow(notes, 0, 7);
      note->setTpc(Ms::tpc(3, note->pitch(), opt));
      }

//---------------------------------------------------------
//   isSavable
//---------------------------------------------------------

bool Score::isSavable() const
      {
      // TODO: check if file can be created if it does not exist
      return info.isWritable() || !info.exists();
      }

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void Score::appendPart(Part* p)
      {
      _parts.append(p);
      }

//---------------------------------------------------------
//   rebuildMidiMapping
//---------------------------------------------------------

void Score::rebuildMidiMapping()
      {
      _midiMapping.clear();
      int port        = 0;
      int drumPort    = 0;
      int midiChannel = 0;
      int idx         = 0;
      int maxport     = 0;
      for (Part* part : _parts) {
            const InstrumentList* il = part->instruments();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  const Instrument* instr = i->second;
                  bool drum         = instr->useDrumset();

                  for (Channel* channel : instr->channel()) {
                        MidiMapping mm;
                        if (port > maxport)
                              maxport = port;
                        if (drumPort > maxport)
                              maxport = drumPort;
                        if (drum) {
                              mm.port    = drumPort;
                              mm.channel = 9;
                              drumPort++;
                              }
                        else {
                              mm.port    = port;
                              mm.channel = midiChannel;
                              if (midiChannel == 15) {
                                    midiChannel = 0;
                                    ++port;
                                    }
                              else {
                                    ++midiChannel;
                                    if (midiChannel == 9)   // skip drum channel
                                          ++midiChannel;
                                    }
                              }
                        mm.part         = part;
                        mm.articulation = channel;
                        _midiMapping.append(mm);
                        channel->channel = idx;
                        ++idx;
                        }
                  }
            }
      setMidiPortCount(maxport);
      }

//---------------------------------------------------------
//   midiPortCount
//---------------------------------------------------------

int Score::midiPortCount() const {
      const Score* root = rootScore();
      if (this == root)
            return _midiPortCount;
      else
            return root->midiPortCount();
      }

//---------------------------------------------------------
//   setMidiPortCount
//---------------------------------------------------------

void Score::setMidiPortCount(int maxport) {
      Score* root = rootScore();
      if (this == root)
            _midiPortCount = maxport;
      else
            root->setMidiPortCount(maxport);
      }

//---------------------------------------------------------
//   midiPort
//---------------------------------------------------------

int Score::midiPort(int idx) const
      {
      return _midiMapping[idx].port;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Score::midiChannel(int idx) const
      {
      return _midiMapping[idx].channel;
      }

//---------------------------------------------------------
//   searchPage
//    p is in canvas coordinates
//---------------------------------------------------------

Page* Score::searchPage(const QPointF& p) const
      {
      for (Page* page : pages()) {
            if (page->bbox().translated(page->pos()).contains(p))
                  return page;
            }
      return 0;
      }

//---------------------------------------------------------
//   searchSystem
//    return list of systems as there may be more than
//    one system in a row
//    p is in canvas coordinates
//---------------------------------------------------------

QList<System*> Score::searchSystem(const QPointF& pos) const
      {
      QList<System*> systems;
      Page* page = searchPage(pos);
      if (page == 0)
            return systems;
      qreal y = pos.y() - page->pos().y();  // transform to page relative
      const QList<System*>* sl = page->systems();
      qreal y2;
      int n = sl->size();
      for (int i = 0; i < n; ++i) {
            System* s = sl->at(i);
            System* ns = 0;               // next system row
            int ii = i + 1;
            for (; ii < n; ++ii) {
                  ns = sl->at(ii);
                  if (ns->y() != s->y())
                        break;
                  }
            if ((ii == n) || (ns == 0))
                  y2 = page->height();
            else {
                  qreal sy2 = s->y() + s->bbox().height();
                  y2         = sy2 + (ns->y() - sy2) * .5;
                  }
            if (y < y2) {
                  systems.append(s);
                  for (int ii = i+1; ii < n; ++ii) {
                        if (sl->at(ii)->y() != s->y())
                              break;
                        systems.append(sl->at(ii));
                        }
                  return systems;
                  }
            }
      return systems;
      }

//---------------------------------------------------------
//   searchMeasure
//    p is in canvas coordinates
//---------------------------------------------------------

Measure* Score::searchMeasure(const QPointF& p) const
      {
      QList<System*> systems = searchSystem(p);
      if (systems.isEmpty())
            return 0;

      foreach(System* system, systems) {
            qreal x = p.x() - system->canvasPos().x();
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() != Element::Type::MEASURE)
                        continue;
                  if (x < (mb->x() + mb->bbox().width()))
                        return static_cast<Measure*>(mb);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//    getNextValidInputSegment
//    - s is of type Segment::Type::ChordRest
//---------------------------------------------------------

static Segment* getNextValidInputSegment(Segment* s, int track, int voice)
      {
      if (s == 0)
            return 0;
      Q_ASSERT(s->segmentType() == Segment::Type::ChordRest);
      // Segment* s1 = s;
      ChordRest* cr1;
      for (Segment* s1 = s; s1; s1 = s1->prev(Segment::Type::ChordRest)) {
            cr1 = static_cast<ChordRest*>(s1->element(track + voice));
            if (cr1)
                  break;
            }
      int nextTick = (cr1 == 0) ? s->measure()->tick() : cr1->tick() + cr1->actualTicks();

      static const Segment::Type st { Segment::Type::ChordRest };
      while (s) {
            if (s->element(track + voice))
                  break;
            if (voice && s->tick() == nextTick)
                  return s;
#if 0
            int v;
            for (v = 0; v < VOICES; ++v) {
                  if (s->element(track + v))
                        break;
                  }
            if ((v != VOICES) && voice) {
                  int ntick;
                  bool skipChord = false;
                  bool ns        = false;
                  for (Segment* s1 = s->measure()->first(st); s1; s1 = s1->next(st)) {
                        ChordRest* cr = static_cast<ChordRest*>(s1->element(track + voice));
                        if (cr) {
                              if (ns)
                                    return s1;
                              ntick = s1->tick() + cr->actualTicks();
                              skipChord = true;
                              }
                        if (s1 == s)
                              ns = true;
                        if (skipChord) {
                              if (s->tick() >= ntick)
                                    skipChord = false;
                              }
                        if (!skipChord && ns)
                              return s1;
                        }
                  if (!skipChord)
                        return s;
                  }
#endif
            s = s->next(st);
            }
      return s;
      }

//---------------------------------------------------------
//   getPosition
//    return true if valid position found
//---------------------------------------------------------

bool Score::getPosition(Position* pos, const QPointF& p, int voice) const
      {
      Measure* measure = searchMeasure(p);
      if (measure == 0)
            return false;

      pos->fret = FRET_NONE;
      //
      //    search staff
      //
      pos->staffIdx      = 0;
      SysStaff* sstaff   = 0;
      System* system     = measure->system();
      qreal y           = p.y() - system->pagePos().y();
      for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
            Staff* st = staff(pos->staffIdx);
            if (!st->part()->show())
                  continue;
            qreal sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            if (!ss->show())
                  continue;
            SysStaff* nstaff = 0;

            // find next visible staff
            for (int i = pos->staffIdx + 1; i < nstaves(); ++i) {
                  Staff* st = staff(i);
                  if (!st->part()->show())
                        continue;
                  nstaff = system->staff(i);
                  if (!nstaff->show()) {
                        nstaff = 0;
                        continue;
                        }
                  break;
                  }

            if (nstaff) {
                  qreal s1y2 = ss->bbox().bottom();
                  sy2        = system->page()->canvasPos().y() + s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = system->page()->canvasPos().y() + system->page()->height() - system->pagePos().y();   // system->height();
            if (y < sy2) {
                  sstaff = ss;
                  break;
                  }
            }
      if (sstaff == 0)
            return false;

      //
      //    search segment
      //
      QPointF pppp(p - measure->canvasPos());
      qreal x         = pppp.x();
      Segment* segment = 0;
      pos->segment     = 0;

      // int track = pos->staffIdx * VOICES + voice;
      int track = pos->staffIdx * VOICES;

      for (segment = measure->first(Segment::Type::ChordRest); segment;) {
            segment = getNextValidInputSegment(segment, track, voice);
            if (segment == 0)
                  break;
            Segment* ns = getNextValidInputSegment(segment->next(Segment::Type::ChordRest), track, voice);

            qreal x1 = segment->x();
            qreal x2;
            qreal d;
            if (ns) {
                  x2    = ns->x();
                  d     = x2 - x1;
                  }
            else {
                  x2    = measure->bbox().width();
                  d     = (x2 - x1) * 2.0;
                  x     = x1;
                  pos->segment = segment;
                  break;
                  }

            if (x < (x1 + d * .5)) {
                  x = x1;
                  pos->segment = segment;
                  break;
                  }
            segment = ns;
            }
      if (segment == 0)
            return false;
      //
      // TODO: restrict to reasonable values (pitch 0-127)
      //
      Staff* s    = staff(pos->staffIdx);
      qreal mag   = s->mag();
      // in TABs, step from one string to another; in other staves, step on and between lines
      qreal lineDist = s->staffType()->lineDistance().val() * (s->isTabStaff() ? 1 : .5) * mag * spatium();

      pos->line  = lrint((pppp.y() - sstaff->bbox().y()) / lineDist);
      if (s->isTabStaff()) {
            if (pos->line < -1 || pos->line > s->lines()+1)
                  return false;
            if (pos->line < 0)
                  pos->line = 0;
            else if (pos->line >= s->lines())
                  pos->line = s->lines() - 1;
            }
      else {
            int minLine   = absStep(0);
            ClefType clef = s->clef(pos->segment->tick());
            minLine       = relStep(minLine, clef);
            int maxLine   = absStep(127);
            maxLine       = relStep(maxLine, clef);

            if (pos->line > minLine || pos->line < maxLine)
                  return false;
            }

      y         = sstaff->y() + pos->line * lineDist;
      pos->pos  = QPointF(x, y) + measure->canvasPos();
      return true;
      }

//---------------------------------------------------------
//   checkHasMeasures
//---------------------------------------------------------

bool Score::checkHasMeasures() const
      {
      Page* page = pages().front();
      const QList<System*>* sl = page->systems();
      if (sl == 0 || sl->empty() || sl->front()->measures().empty()) {
            qDebug("first create measure, then repeat operation");
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   moveBracket
//    columns are counted from right to left
//---------------------------------------------------------

void Score::moveBracket(int staffIdx, int srcCol, int dstCol)
      {
      foreach(System* system, *systems()) {
            if (system->isVbox())
                  continue;
            foreach(Bracket* b, system->brackets()) {
                  if (b->staffIdx() == staffIdx && b->level() == srcCol)
                        b->setLevel(dstCol);
                  }
            }
      }

//---------------------------------------------------------
//   spatiumHasChanged
//---------------------------------------------------------

static void spatiumHasChanged(void* data, Element* e)
      {
      qreal* val = (qreal*)data;
      e->spatiumChanged(val[0], val[1]);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Score::spatiumChanged(qreal oldValue, qreal newValue)
      {
      qreal data[2];
      data[0] = oldValue;
      data[1] = newValue;
      scanElements(data, spatiumHasChanged, true);
      foreach (Staff* staff, _staves)
            staff->spatiumChanged(oldValue, newValue);
      _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, newValue / SPATIUM20);
      }

void Score::setSpatium(qreal v)
      {
      style()->setSpatium(v);
      }

//---------------------------------------------------------
//   getCreateMeasure
//    - return Measure for tick
//    - create new Measure(s) if there is no measure for
//      this tick
//---------------------------------------------------------

Measure* Score::getCreateMeasure(int tick)
      {
      Measure* last = lastMeasure();
      if (last == 0 || ((last->tick() + last->ticks()) <= tick)) {
            int lastTick  = last ? (last->tick()+last->ticks()) : 0;
            while (tick >= lastTick) {
                  Measure* m = new Measure(this);
                  Fraction ts = _sigmap->timesig(lastTick).timesig();
// qDebug("getCreateMeasure %d  %d/%d", tick, ts.numerator(), ts.denominator());
                  m->setTick(lastTick);
                  m->setTimesig(ts);
                  m->setLen(ts);
                  measures()->add(static_cast<MeasureBase*>(m));
                  lastTick += ts.ticks();
                  }
            }
      return tick2measure(tick);
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 Add \a element to its parent.

 Several elements (clef, keysig, timesig) need special handling, as they may cause
 changes throughout the score.
*/

void Score::addElement(Element* element)
      {
      if (MScore::debugMode) {
            qDebug("   Score(%p)::addElement %p(%s) parent %p(%s)",
               this, element, element->name(), element->parent(),
               element->parent() ? element->parent()->name() : "");
            }

      if (element->parent() && element->parent()->type() == Element::Type::SEGMENT)
            static_cast<Segment*>(element->parent())->measure()->setDirty();

      Element::Type et = element->type();
      if (et == Element::Type::TREMOLO)
            setLayoutAll(true);
      else if (et == Element::Type::MEASURE
         || (et == Element::Type::HBOX && element->parent()->type() != Element::Type::VBOX)
         || et == Element::Type::VBOX
         || et == Element::Type::TBOX
         || et == Element::Type::FBOX
         ) {
            setLayoutAll(true);
            measures()->add(static_cast<MeasureBase*>(element));
            addLayoutFlags(LayoutFlag::FIX_TICKS);
            return;
            }

      if (element->parent())
            element->parent()->add(element);

      switch(et) {
            case Element::Type::BEAM:
                  {
                  Beam* b = static_cast<Beam*>(element);
                  int n = b->elements().size();
                  for (int i = 0; i < n; ++i)
                        b->elements().at(i)->setBeam(b);
                  }
                  break;

            case Element::Type::SLUR:
                  addLayoutFlags(LayoutFlag::PLAY_EVENTS);
                  // fall through

            case Element::Type::VOLTA:
            case Element::Type::TRILL:
            case Element::Type::PEDAL:
            case Element::Type::TEXTLINE:
            case Element::Type::HAIRPIN:
                  {
                  Spanner* spanner = static_cast<Spanner*>(element);
                  if (et == Element::Type::TEXTLINE && spanner->anchor() == Spanner::Anchor::NOTE)
                        break;
                  addSpanner(spanner);
                  for (SpannerSegment* ss : spanner->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;

            case Element::Type::OTTAVA:
                  {
                  Ottava* o = static_cast<Ottava*>(element);
                  addSpanner(o);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  o->staff()->updateOttava();
                  _playlistDirty = true;
                  }
                  break;

            case Element::Type::DYNAMIC:
                  layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case Element::Type::TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(element);
                  setTempo(tt->segment(), tt->tempo());
                  }
                  break;

            case Element::Type::INSTRUMENT_CHANGE: {
                  InstrumentChange* ic = static_cast<InstrumentChange*>(element);
                  ic->part()->setInstrument(ic->instrument(), ic->segment()->tick());
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  }
                  break;
            case Element::Type::CHORD:
                  setPlaylistDirty();
                  // create playlist does not work here bc. tremolos may not be complete
                  // createPlayEvents(static_cast<Chord*>(element));
                  break;

            case Element::Type::NOTE:
            case Element::Type::TREMOLO:
            case Element::Type::ARTICULATION:
            case Element::Type::ARPEGGIO:
                  {
                  Element* cr = element->parent();
                  if (cr->type() == Element::Type::CHORD)
                        createPlayEvents(static_cast<Chord*>(cr));
                  }
                  break;
            case Element::Type::BREATH:
                  addLayoutFlags(LayoutFlag::FIX_TICKS);
                  break;
            case Element::Type::LAYOUT_BREAK:
                  if (static_cast<LayoutBreak*>(element)->layoutBreakType() == LayoutBreak::Type::SECTION)
                        addLayoutFlags(LayoutFlag::FIX_TICKS);
                  break;
            default:
                  break;
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   removeElement
///   Remove \a element from its parent.
///   Several elements (clef, keysig, timesig) need special handling, as they may cause
///   changes throughout the score.
//---------------------------------------------------------

void Score::removeElement(Element* element)
      {
      Element* parent = element->parent();

      if (MScore::debugMode) {
            qDebug("   Score(%p)::removeElement %p(%s) parent %p(%s)",
               this, element, element->name(), parent, parent ? parent->name() : "");
            }

      if (parent && parent->type() == Element::Type::SEGMENT)
            static_cast<Segment*>(parent)->measure()->setDirty();

      // special for MEASURE, HBOX, VBOX
      // their parent is not static

      Element::Type et = element->type();
      if (et == Element::Type::TREMOLO)
            setLayoutAll(true);

      else if (et == Element::Type::MEASURE
         || (et == Element::Type::HBOX && parent->type() != Element::Type::VBOX)
         || et == Element::Type::VBOX
         || et == Element::Type::TBOX
         || et == Element::Type::FBOX
            ) {
            measures()->remove(static_cast<MeasureBase*>(element));
            addLayoutFlags(LayoutFlag::FIX_TICKS);
            setLayoutAll(true);
            return;
            }

      if (et == Element::Type::BEAM)          // beam parent does not survive layout
            element->setParent(0);

      if (parent)
            parent->remove(element);

      switch(et) {
            case Element::Type::BEAM:
                  {
                  Beam* b = static_cast<Beam*>(element);
                  foreach(ChordRest* cr, b->elements())
                        cr->setBeam(0);
                  }
                  break;

            case Element::Type::SLUR:
                  addLayoutFlags(LayoutFlag::PLAY_EVENTS);
                  // fall through

            case Element::Type::VOLTA:
            case Element::Type::TRILL:
            case Element::Type::PEDAL:
            case Element::Type::TEXTLINE:
            case Element::Type::HAIRPIN:
//            case Element::Type::LYRICSLINE:
                  {
                  Spanner* spanner = static_cast<Spanner*>(element);
                  if (et == Element::Type::TEXTLINE && spanner->anchor() == Spanner::Anchor::NOTE)
                        break;
                  removeSpanner(spanner);
                  for (SpannerSegment* ss : spanner->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
                  break;

            case Element::Type::OTTAVA:
                  {
                  Ottava* o = static_cast<Ottava*>(element);
                  removeSpanner(o);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  o->staff()->updateOttava();
                  layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  _playlistDirty = true;
                  }
                  break;

            case Element::Type::DYNAMIC:
                  layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case Element::Type::CHORD:
            case Element::Type::REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(element);
                  if (cr->beam())
                        cr->beam()->remove(cr);
                  for (Lyrics* lyr : cr->lyricsList())
                        if (lyr)                // lyrics list may be sparse
                              lyr->removeFromScore();
                  // TODO: check for tuplet?
                  }
                  break;
            case Element::Type::TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(element);
                  int tick = tt->segment()->tick();
                  tempomap()->delTempo(tick);
                  }
                  break;
            case Element::Type::INSTRUMENT_CHANGE: {
                  InstrumentChange* ic = static_cast<InstrumentChange*>(element);
                  ic->part()->removeInstrument(ic->segment()->tick());
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  }
                  break;

            case Element::Type::TREMOLO:
            case Element::Type::ARTICULATION:
            case Element::Type::ARPEGGIO:
                  {
                  Element* cr = element->parent();
                  if (cr->type() == Element::Type::CHORD)
                        createPlayEvents(static_cast<Chord*>(cr));
                  }
                  break;

            default:
                  break;
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* Score::firstMeasure() const
      {
      MeasureBase* mb = _measures.first();
      while (mb && mb->type() != Element::Type::MEASURE)
            mb = mb->next();

      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   firstMeasureMM
//---------------------------------------------------------

Measure* Score::firstMeasureMM() const
      {
      MeasureBase* mb = _measures.first();
      while (mb && mb->type() != Element::Type::MEASURE)
            mb = mb->next();
      Measure* m = static_cast<Measure*>(mb);
      if (m && styleB(StyleIdx::createMultiMeasureRests) && m->hasMMRest())
            return m->mmRest();
      return m;
      }

//---------------------------------------------------------
//   firstMM
//---------------------------------------------------------

MeasureBase* Score::firstMM() const
      {
      MeasureBase* m = _measures.first();
      if (m
         && m->type() == Element::Type::MEASURE
         && styleB(StyleIdx::createMultiMeasureRests)
         && static_cast<Measure*>(m)->hasMMRest()) {
            return static_cast<Measure*>(m)->mmRest();
            }
      return m;
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

MeasureBase* Score::measure(int idx) const
      {
      MeasureBase* mb = _measures.first();
      for (int i = 0; i < idx; ++i) {
            mb = mb->next();
            if (mb == 0)
                  return 0;
            }
      return mb;
      }

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* Score::lastMeasure() const
      {
      MeasureBase* mb = _measures.last();
      while (mb && mb->type() != Element::Type::MEASURE)
            mb = mb->prev();
      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   lastMeasureMM
//---------------------------------------------------------

Measure* Score::lastMeasureMM() const
      {
      Measure* m = lastMeasure();
      if (m && styleB(StyleIdx::createMultiMeasureRests)) {
            Measure* m1 = const_cast<Measure*>(m->mmRest1());
            if (m1)
                  return m1;
            }
      return m;
      }

//---------------------------------------------------------
//   firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(Segment::Type segType) const
      {
      Segment* seg;
      Measure* m = firstMeasure();
      if (!m)
            seg = 0;
      else {
            seg = m->first();
            if (seg && !(seg->segmentType() & segType))
                  seg = seg->next1(segType);
            }

#ifdef SCRIPT_INTERFACE
      // if called from QML/JS, tell QML engine not to garbage collect this object
      if (seg)
            QQmlEngine::setObjectOwnership(seg, QQmlEngine::CppOwnership);
#endif
      return seg;
      }

//---------------------------------------------------------
//   firstSegment Q_INVOKABLE wrapper
//---------------------------------------------------------

Segment* Score::firstSegment(int segType) const
      {
      return firstSegment(static_cast<Segment::Type>(segType));
      }

//---------------------------------------------------------
//   firstSegmentMM
//---------------------------------------------------------

Segment* Score::firstSegmentMM(Segment::Type segType) const
      {
      Measure* m = firstMeasureMM();
      return m ? m->first(segType) : 0;
      }

//---------------------------------------------------------
//   lastSegment
//---------------------------------------------------------

Segment* Score::lastSegment() const
      {
      Measure* m = lastMeasure();
      return m ? m->last() : 0;
      }

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

qreal Score::utick2utime(int tick) const
      {
      return repeatList()->utick2utime(tick);
      }

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int Score::utime2utick(qreal utime) const
      {
      return repeatList()->utime2utick(utime);
      }

//---------------------------------------------------------
//   inputPos
//---------------------------------------------------------

int Score::inputPos() const
      {
      return _is.tick();
      }

//---------------------------------------------------------
//   scanElements
//    scan all elements
//---------------------------------------------------------

void Score::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            mb->scanElements(data, func, all);
            if (mb->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  Measure* mmr = m->mmRest();
                  if (mmr)
                        mmr->scanElements(data, func, all);
                  }
            }
      for (Page* page : pages())
            page->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   scanElementsInRange
//---------------------------------------------------------

void Score::scanElementsInRange(void* data, void (*func)(void*, Element*), bool all)
      {
      Segment* startSeg = _selection.startSegment();
      for (Segment* s = startSeg; s && s !=_selection.endSegment(); s = s->next1()) {
            s->scanElements(data, func, all);
            Measure* m = s->measure();
            if (m && s == m->first()) {
                  Measure* mmr = m->mmRest();
                  if (mmr)
                        mmr->scanElements(data, func, all);
                  }
            }
      for (Element* e : _selection.elements()) {
            if (e->isSpanner()) {
                  Spanner* spanner = static_cast<Spanner*>(e);
                  for (SpannerSegment* ss : spanner->spannerSegments()) {
                        ss->scanElements(data, func, all);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setSelection
//---------------------------------------------------------

void Score::setSelection(const Selection& s)
      {
      deselectAll();
      _selection = s;

      foreach(Element* e, _selection.elements())
            e->setSelected(true);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

Text* Score::getText(TextStyleType subtype)
      {
      MeasureBase* m = first();
      if (m && m->type() == Element::Type::VBOX) {
            foreach(Element* e, m->el()) {
                  if (e->type() == Element::Type::TEXT && static_cast<Text*>(e)->textStyleType() == subtype)
                        return static_cast<Text*>(e);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   rootScore
//---------------------------------------------------------

Score* Score::rootScore()
      {
      Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score;
      }

const Score* Score::rootScore() const
      {
      const Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score;
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

UndoStack* Score::undo() const
      {
      return rootScore()->_undo;
      }

//---------------------------------------------------------
//   repeatList
//---------------------------------------------------------

RepeatList* Score::repeatList()  const
      {
      return rootScore()->_repeatList;
      }

//---------------------------------------------------------
//   links
//---------------------------------------------------------

QMap<int, LinkedElements*>& Score::links()
      {
      return rootScore()->_elinks;
      }

//---------------------------------------------------------
//   setTempomap
//---------------------------------------------------------

void Score::setTempomap(TempoMap* tm)
      {
      delete _tempomap;
      _tempomap = tm;
      }

//---------------------------------------------------------
//   tempomap
//---------------------------------------------------------

TempoMap* Score::tempomap() const
      {
      return rootScore()->_tempomap;
      }

//---------------------------------------------------------
//   sigmap
//---------------------------------------------------------

TimeSigMap* Score::sigmap() const
      {
      return rootScore()->_sigmap;
      }

//---------------------------------------------------------
//   metaTag
//---------------------------------------------------------

QString Score::metaTag(const QString& s) const
      {
      if (_metaTags.contains(s))
            return _metaTags.value(s);
      return rootScore()->_metaTags.value(s);
      }

//---------------------------------------------------------
//   setMetaTag
//---------------------------------------------------------

void Score::setMetaTag(const QString& tag, const QString& val)
      {
      _metaTags.insert(tag, val);
      }

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void Score::addExcerpt(Score* score)
      {
      Excerpt* ex = new Excerpt(this);
      ex->setPartScore(score);
      excerpts().append(ex);
      ex->setTitle(score->name());
      for (Staff* s : score->staves()) {
            LinkedStaves* ls = s->linkedStaves();
            if (ls == 0)
                  continue;
            foreach(Staff* ps, ls->staves()) {
                  if (ps->score() == this) {
                        ex->parts().append(ps->part());
                        break;
                        }
                  }
            }
      setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   removeExcerpt
//---------------------------------------------------------

void Score::removeExcerpt(Score* score)
      {
      for (Excerpt* ex : excerpts()) {
            if (ex->partScore() == score) {
                  if (excerpts().removeOne(ex)) {
                        delete ex;
                        return;
                        }
                  else
                        qDebug("removeExcerpt:: ex not found");
                  }
            }
      qDebug("Score::removeExcerpt: excerpt not found");
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

Score* Score::clone()
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();

      xml.stag("museScore version=\"" MSC_VERSION "\"");
      write(xml, false);
      xml.etag();

      buffer.close();

      XmlReader r(buffer.buffer());
      Score* score = new Score(style());
      score->read1(r, true);

      score->addLayoutFlags(LayoutFlag::FIX_TICKS | LayoutFlag::FIX_PITCH_VELO);
      score->doLayout();
      score->scanElements(0, elementAdjustReadPos);  //??
      return score;
      }

//---------------------------------------------------------
//   setSynthesizerState
//---------------------------------------------------------

void Score::setSynthesizerState(const SynthesizerState& s)
      {
      // TODO: make undoable
      _synthesizerState = s;
      }

//---------------------------------------------------------
//   setLayoutAll
//---------------------------------------------------------

void Score::setLayoutAll(bool val)
      {
      foreach(Score* score, scoreList())
            score->_layoutAll = val;
      }

//---------------------------------------------------------
//   removeOmr
//---------------------------------------------------------

void Score::removeOmr()
      {
      _showOmr = false;
#ifdef OMR
      delete _omr;
      _omr = 0;
#endif
      }

//---------------------------------------------------------
//   removeAudio
//---------------------------------------------------------

void Score::removeAudio()
      {
      delete _audio;
      _audio = 0;
      }

//---------------------------------------------------------
//   appendScore
//---------------------------------------------------------

bool Score::appendScore(Score* score, bool addPageBreak, bool addSectionBreak)
      {
      if (parts().size() < score->parts().size() || staves().size() < score->staves().size()) {
            qDebug("Score to append has %d parts and %d staves, but this score only has %d parts and %d staves.", score->parts().size(), score->staves().size(), parts().size(), staves().size());
            return false;
            }

      if (!last()) {
            qDebug("This score doesn't have any MeasureBase objects.");
            return false;
            }

      TieMap tieMap;
      int tickOfAppend = last()->endTick();

      // apply Page/Section Breaks if desired
      if (addPageBreak) {
            if (!last()->pageBreak()) {
                  last()->undoSetBreak(false, LayoutBreak::Type::LINE); // remove line break if exists
                  last()->undoSetBreak(true, LayoutBreak::Type::PAGE);  // apply page break
                  }
            }
      else if (!last()->lineBreak() && !last()->pageBreak())
            last()->undoSetBreak(true, LayoutBreak::Type::LINE);
      if (addSectionBreak && !last()->sectionBreak())
            last()->undoSetBreak(true, LayoutBreak::Type::SECTION);

      // match concert pitch states
      if (styleB(StyleIdx::concertPitch) != score->styleB(StyleIdx::concertPitch))
            score->cmdConcertPitchChanged(styleB(StyleIdx::concertPitch), true);

      // convert any "generated" initial clefs into real "non-generated" clefs if clef type changes
      if (score->firstMeasure()) {
            Segment* initialClefSegment = score->firstMeasure()->findSegment(Segment::Type::Clef, 0);      // find clefs at first tick of first measure
            if (initialClefSegment) {
                  for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                        int track    = staffIdx * VOICES;
                        Staff* staff = score->staff(staffIdx);
                        Clef* initialClef  = static_cast<Clef*>(initialClefSegment->element(track));

                        // if the first clef of score to append is generated and
                        // if the first clef of score to append is of different type than clef at final tick of first score
                        if (initialClef && initialClef->generated() && initialClef->clefType() != this->staff(staffIdx)->clef(tickOfAppend)) {

                              // then convert that generated clef into a real non-generated clef so that its different type will be copied to joined score
                              score->undoChangeClef(staff, initialClefSegment, initialClef->clefType());
                              }
                        }
                  }
            }

      // clone the measures
      MeasureBaseList* ml = &score->_measures;
      for (MeasureBase* mb = ml->first(); mb; mb = mb->next()) {
            MeasureBase* nmb;
            if (mb->type() == Element::Type::MEASURE)
                  nmb = static_cast<Measure*>(mb)->cloneMeasure(this, &tieMap);
            else
                  nmb = mb->clone();
            nmb->setNext(0);
            nmb->setPrev(0);
            nmb->setScore(this);
            _measures.add(nmb);
            }
      fixTicks();
      Measure* firstAppendedMeasure = tick2measure(tickOfAppend);

      // if the appended score has less staves,
      // make sure the measures have full measure rest
      for (Measure* m = firstAppendedMeasure; m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Fraction f;
                  for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next(Segment::Type::ChordRest)) {
                        for (int v = 0; v < VOICES; ++v) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(staffIdx * VOICES + v));
                              if (cr == 0)
                                    continue;
                              f += cr->actualFraction();
                              }
                        }
                  if (f.isZero())
                        addRest(m->tick(), staffIdx*VOICES, TDuration(TDuration::DurationType::V_MEASURE), 0);
                  }
            }

      // adjust key signatures
      if (firstAppendedMeasure) {
            Segment* seg = firstAppendedMeasure->getSegment(Segment::Type::KeySig, tickOfAppend);
            for (Staff* st : score->staves()) {
                  int staffIdx = score->staffIdx(st);
                  Staff* joinedStaff = staff(staffIdx);
                  // special case for initial "C" key signature - these have no explicit element
                  if (!seg->element(staffIdx * VOICES)) {
                        // no need to create new initial "C" key sig
                        // if staff already ends in that key
                        if (joinedStaff->key(tickOfAppend - 1) == Key::C)
                              continue;
                        Key key = Key::C;
                        KeySig* ks = new KeySig(this);
                        ks->setTrack(staffIdx * VOICES);
                        ks->setKey(key);
                        ks->setParent(seg);
                        addElement(ks);
                        }
                  // other key signatures (initial other than "C", non-initial)
                  for (auto k : *(st->keyList())) {
                        int tick = k.first;
                        KeySigEvent key = k.second;
                        joinedStaff->setKey(tick + tickOfAppend, key);
                        }
                  }
            }

      // clone the spanners
      for (auto sp : score->spanner()) {
            Spanner* spanner = sp.second;
            Spanner* ns = static_cast<Spanner*>(spanner->clone());
            ns->setScore(this);
            ns->setParent(0);
            ns->setTick(spanner->tick() + tickOfAppend);
            ns->setTick2(spanner->tick2() + tickOfAppend);
            if (ns->type() == Element::Type::SLUR) {
                  // set start/end element for slur
                  ns->setStartElement(0);
                  ns->setEndElement(0);
                  Measure* sm = tick2measure(ns->tick());
                  if (sm)
                        ns->setStartElement(sm->findChordRest(ns->tick(), ns->track()));
                  Measure * em = tick2measure(ns->tick2());
                  if (em)
                        ns->setEndElement(em->findChordRest(ns->tick2(), ns->track2()));
                  if (!ns->startElement())
                        qDebug("clone Slur: no start element");
                  if (!ns->endElement())
                        qDebug("clone Slur: no end element");
                  }
            addElement(ns);
            }
      setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   splitStaff
//---------------------------------------------------------

void Score::splitStaff(int staffIdx, int splitPoint)
      {
      qDebug("split staff %d point %d", staffIdx, splitPoint);

      //
      // create second staff
      //
      Staff* s  = staff(staffIdx);
      Part*  p  = s->part();
      Staff* ns = new Staff(this);
      ns->setPart(p);
      // convert staffIdx from score-relative to part-relative
      int staffIdxPart = staffIdx - p->staff(0)->idx();
      undoInsertStaff(ns, staffIdxPart + 1, false);

      Clef* clef = new Clef(this);
      clef->setClefType(ClefType::F);
      clef->setTrack((staffIdx+1) * VOICES);
      Segment* seg = firstMeasure()->getSegment(Segment::Type::Clef, 0);
      clef->setParent(seg);
      undoAddElement(clef);

      undoChangeKeySig(ns, 0, s->keySigEvent(0));

      rebuildMidiMapping();
      _instrumentsChanged = true;
      doLayout();

      //
      // move notes
      //
      select(0, SelectType::SINGLE, 0);
      int strack = staffIdx * VOICES;
      int dtrack = (staffIdx + 1) * VOICES;

      for (Segment* s = firstSegment(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  Element* el = s->element(strack + voice);
                  if (el == 0 || el->type() != Element::Type::CHORD)
                        continue;
                  Chord* c = static_cast<Chord*>(el);
                  QList<Note*> removeNotes;
                  foreach(Note* note, c->notes()) {
                        if (note->pitch() >= splitPoint)
                              continue;
                        Element* el = s->element(dtrack + voice);
                        Q_ASSERT(!el || (el->type() == Element::Type::CHORD));
                        Chord* chord = static_cast<Chord*>(el);
                        if (chord == 0) {
                              chord = new Chord(*c);
                              qDeleteAll(chord->notes());
                              chord->notes().clear();
                              chord->setTrack(dtrack + voice);
                              undoAddElement(chord);
                              }
                        Note* nnote = new Note(*note);
                        nnote->setTrack(dtrack + voice);
                        chord->add(nnote);
                        nnote->updateLine();
                        removeNotes.append(note);
                        }
                  c->sortNotes();
                  for (Note* note : removeNotes) {
                        undoRemoveElement(note);
                        Chord* chord = note->chord();
                        if (chord->notes().isEmpty()) {
                              for (auto sp : spanner()) {
                                    Spanner* spanner = sp.second;
                                    if (spanner->type() != Element::Type::SLUR)
                                          continue;
                                    Slur* slur = static_cast<Slur*>(spanner);
                                    if (slur->startCR() == chord) {
                                          slur->undoChangeProperty(P_ID::TRACK, slur->track()+VOICES);
                                          for (ScoreElement* ee : slur->linkList()) {
                                                Slur* lslur = static_cast<Slur*>(ee);
                                                lslur->setStartElement(0);
                                                }
                                          }
                                    if (slur->endCR() == chord) {
                                          slur->undoChangeProperty(P_ID::SPANNER_TRACK2, slur->track2()+VOICES);
                                          for (ScoreElement* ee : slur->linkList()) {
                                                Slur* lslur = static_cast<Slur*>(ee);
                                                lslur->setEndElement(0);
                                                }
                                          }
                                    }
                              undoRemoveElement(chord);
                              }
                        }
                  }
            }
      //
      // make sure that the timeline for dtrack
      // has no gaps
      //
      int ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(dtrack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                        // insert Rest
                        Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              qDebug("no segment at %d", ctick);
                              continue;
                              }
                        setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->actualTicks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      //
      // same for strack
      //
      ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(Segment::Type::ChordRest); s; s = s->next1(Segment::Type::ChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(strack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                        // insert Rest
                        Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              qDebug("no segment at %d", ctick);
                              continue;
                              }
                        setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->actualTicks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Score::cmdRemovePart(Part* part)
      {
      int sidx   = staffIdx(part);
      int n      = part->nstaves();

      for (int i = 0; i < n; ++i)
            cmdRemoveStaff(sidx);

      undoRemovePart(part, sidx);
      }

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, int idx)
      {
      int staff = 0;
      for (QList<Part*>::iterator i = _parts.begin(); i != _parts.end(); ++i) {
            if (staff >= idx) {
                  _parts.insert(i, part);
                  return;
                  }
            staff += (*i)->nstaves();
            }
      _parts.push_back(part);
      }

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
      {
      _parts.removeAt(_parts.indexOf(part));
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Score::insertStaff(Staff* staff, int ridx)
      {
      staff->part()->insertStaff(staff, ridx);

      int idx = staffIdx(staff->part()) + ridx;
      _staves.insert(idx, staff);

      for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
            Spanner* s = i->second;
            if (s->systemFlag())
                  continue;
            if (s->staffIdx() >= idx) {
                  int t = s->track() + VOICES;
                  s->setTrack(t < ntracks() ? t : ntracks() - 1);
                  if (s->track2() != -1) {
                        t = s->track2() + VOICES;
                        s->setTrack2(t < ntracks() ? t : s->track());
                        }
                  }
            }
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
      {
      int idx = staffIdx(staff);
      for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
            Spanner* s = i->second;
            if (s->staffIdx() > idx) {
                  int t = s->track() - VOICES;
                  s->setTrack(t >=0 ? t : 0);
                  if (s->track2() != -1) {
                        t = s->track2() - VOICES;
                        s->setTrack2(t >=0 ? t : s->track());
                        }
                  }
            }
      _staves.removeAll(staff);
      staff->part()->removeStaff(staff);
      }

//---------------------------------------------------------
//   adjustBracketsDel
//---------------------------------------------------------

void Score::adjustBracketsDel(int sidx, int eidx)
      {
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            for (int i = 0; i < staff->bracketLevels(); ++i) {
                  int span = staff->bracketSpan(i);
                  if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx))
                        continue;
                  if ((sidx >= staffIdx) && (eidx <= (staffIdx + span)))
                        undoChangeBracketSpan(staff, i, span - (eidx-sidx));
                  }
            int span = staff->barLineSpan();
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
                  int newSpan = span - (eidx-sidx) + 1;
                  int lastSpannedStaffIdx = staffIdx + newSpan - 1;
                  undoChangeBarLineSpan(staff, newSpan, 0, (_staves[lastSpannedStaffIdx]->lines()-1)*2);
                  }
            }
      }

//---------------------------------------------------------
//   adjustBracketsIns
//---------------------------------------------------------

void Score::adjustBracketsIns(int sidx, int eidx)
      {
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            int bl = staff->bracketLevels();
            for (int i = 0; i < bl; ++i) {
                  int span = staff->bracketSpan(i);
                  if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx))
                        continue;
                  if ((sidx >= staffIdx) && (eidx < (staffIdx + span)))
                        undoChangeBracketSpan(staff, i, span + (eidx-sidx));
                  }
            int span = staff->barLineSpan();
            if ((sidx >= staffIdx) && (eidx < (staffIdx + span))) {
                  int idx = staffIdx + span - 1;
                  if (idx >= _staves.size())
                        idx = _staves.size() - 1;
                  undoChangeBarLineSpan(staff, span, 0, (_staves[idx]->lines()-1)*2);
                  }
            }
      }

//---------------------------------------------------------
//   adjustKeySigs
//---------------------------------------------------------

void Score::adjustKeySigs(int sidx, int eidx, KeyList km)
      {
      for (int staffIdx = sidx; staffIdx < eidx; ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            if (staff->isDrumStaff())
                  continue;
            for (auto i = km.begin(); i != km.end(); ++i) {
                  int tick = i->first;
                  Measure* measure = tick2measure(tick);
                  if (!measure)
                        continue;
                  KeySigEvent oKey = i->second;
                  KeySigEvent nKey = oKey;
                  int diff = -staff->part()->instrument(tick)->transpose().chromatic;
                  if (diff != 0 && !styleB(StyleIdx::concertPitch) && !oKey.custom() && !oKey.isAtonal())
                        nKey.setKey(transposeKey(nKey.key(), diff));
                  staff->setKey(tick, nKey);
                  KeySig* keysig = new KeySig(this);
                  keysig->setTrack(staffIdx * VOICES);
                  keysig->setKeySigEvent(nKey);
                  Segment* s = measure->getSegment(keysig, tick);
                  s->add(keysig);
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemoveStaff
//---------------------------------------------------------

void Score::cmdRemoveStaff(int staffIdx)
      {
      Staff* s = staff(staffIdx);
      adjustBracketsDel(staffIdx, staffIdx+1);

      QList<Spanner*> sl;
      for (auto i = _spanner.cbegin(); i != _spanner.cend(); ++i) {
            Spanner* s = i->second;
            if (s->staffIdx() == staffIdx && (staffIdx != 0 || !s->systemFlag()))
                  sl.append(s);
            }
      for (auto i : sl) {
            i->undoUnlink();
            undo(new RemoveElement(i));
            }

      undoRemoveStaff(s);

      // remove linked staff and measures in linked staves in excerpts
      // unlink staff in the same score
      if (s->linkedStaves()) {
            Staff* sameScoreLinkedStaff = nullptr;
            auto staves = s->linkedStaves()->staves();
            for (Staff* staff : staves) {
                  if (staff == s)
                        continue;
                  Score* lscore = staff->score();
                  if (lscore != this) {
                        lscore->undoRemoveStaff(staff);
                        s->score()->undo(new UnlinkStaff(s, staff));
                        if (staff->part()->nstaves() == 0) {
                              int pIndex    = lscore->staffIdx(staff->part());
                              lscore->undoRemovePart(staff->part(), pIndex);
                              }
                        }
                  else // linked staff in the same score
                       sameScoreLinkedStaff = staff;
                  }
            if (sameScoreLinkedStaff)
                  s->score()->undo(new UnlinkStaff(sameScoreLinkedStaff, s)); // once should be enough
            }
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Score::sortStaves(QList<int>& dst)
      {
      systems()->clear();  //??
      _parts.clear();
      Part* curPart = 0;
      QList<Staff*> dl;
      foreach (int idx, dst) {
            Staff* staff = _staves[idx];
            if (staff->part() != curPart) {
                  curPart = staff->part();
                  curPart->staves()->clear();
                  _parts.push_back(curPart);
                  }
            curPart->staves()->push_back(staff);
            dl.push_back(staff);
            }
      _staves = dl;

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->sortStaves(dst);
            if (m->hasMMRest())
                  m->mmRest()->sortStaves(dst);
            }
      for (auto i : _spanner.map()) {
            Spanner* sp = i.second;
            if (sp->systemFlag())
                  continue;
            int voice    = sp->voice();
            int staffIdx = sp->staffIdx();
            int idx = dst.indexOf(staffIdx);
            if (idx >=0) {
                  sp->setTrack(idx * VOICES + voice);
                  if (sp->track2() != -1)
                        sp->setTrack2(idx * VOICES +(sp->track2() % VOICES)); // at least keep the voice...
                  }
            }
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag, bool /*useDoubleSharpsFlats*/)
      {
      undo(new ChangeConcertPitch(this, flag));       // change style flag

      for (Staff* staff : _staves) {
            if (staff->staffType()->group() == StaffGroup::PERCUSSION)
                  continue;
            // if this staff has no transposition, and no instrument changes, we can skip it
            Interval interval = staff->part()->instrument()->transpose();
            if (interval.isZero() && staff->part()->instruments()->size() == 1)
                  continue;
            if (!flag)
                  interval.flip();

            int staffIdx   = staff->idx();
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;

            transposeKeys(staffIdx, staffIdx + 1, 0, lastSegment()->tick(), interval, true, !flag);

            for (Segment* segment = firstSegment(Segment::Type::ChordRest); segment; segment = segment->next1(Segment::Type::ChordRest)) {
                  interval = staff->part()->instrument(segment->tick())->transpose();
                  if (!flag)
                        interval.flip();
                  for (Element* e : segment->annotations()) {
                        if ((e->type() != Element::Type::HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                              continue;
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                        for (ScoreElement* e : h->linkList()) {
                              // don't transpose all links
                              // just ones resulting from mmrests
                              Harmony* he = static_cast<Harmony*>(e);
                              if (he->staff() == h->staff())
                                    undoTransposeHarmony(he, rootTpc, baseTpc);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   addAudioTrack
//---------------------------------------------------------

void Score::addAudioTrack()
      {
      // TODO
      }

//---------------------------------------------------------
//   padToggle
//---------------------------------------------------------

void Score::padToggle(Pad n)
      {
      int oldDots = _is.duration().dots();
      switch (n) {
            case Pad::NOTE00:
                  _is.setDuration(TDuration::DurationType::V_LONG);
                  break;
            case Pad::NOTE0:
                  _is.setDuration(TDuration::DurationType::V_BREVE);
                  break;
            case Pad::NOTE1:
                  _is.setDuration(TDuration::DurationType::V_WHOLE);
                  break;
            case Pad::NOTE2:
                  _is.setDuration(TDuration::DurationType::V_HALF);
                  break;
            case Pad::NOTE4:
                  _is.setDuration(TDuration::DurationType::V_QUARTER);
                  break;
            case Pad::NOTE8:
                  _is.setDuration(TDuration::DurationType::V_EIGHTH);
                  break;
            case Pad::NOTE16:
                  _is.setDuration(TDuration::DurationType::V_16TH);
                  break;
            case Pad::NOTE32:
                  _is.setDuration(TDuration::DurationType::V_32ND);
                  break;
            case Pad::NOTE64:
                  _is.setDuration(TDuration::DurationType::V_64TH);
                  break;
            case Pad::NOTE128:
                  _is.setDuration(TDuration::DurationType::V_128TH);
                  break;
            case Pad::REST:
                  _is.setRest(!_is.rest());
                  break;
            case Pad::DOT:
                  if ((_is.duration().dots() == 1) || (_is.duration() == TDuration::DurationType::V_128TH))
                        _is.setDots(0);
                  else
                        _is.setDots(1);
                  break;
            case Pad::DOTDOT:
                  if ((_is.duration().dots() == 2) || (_is.duration() == TDuration::DurationType::V_64TH) || (_is.duration() == TDuration::DurationType::V_128TH))
                        _is.setDots(0);
                  else
                        _is.setDots(2);
                  break;
            case Pad::DOT3:
                  if ((_is.duration().dots() == 3)
                     || (_is.duration() == TDuration::DurationType::V_32ND)
                     || (_is.duration() == TDuration::DurationType::V_64TH)
                     || (_is.duration() == TDuration::DurationType::V_128TH))
                        _is.setDots(0);
                  else
                        _is.setDots(3);
                  break;
            }
      if (n >= Pad::NOTE00 && n <= Pad::NOTE128) {
            _is.setDots(0);
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode()) {
                  if (usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
                        switch (oldDots) {
                              case 1:
                                    padToggle(Pad::DOT);
                                    break;
                              case 2:
                                    padToggle(Pad::DOTDOT);
                                    break;
                              }
                        NoteVal nval;
                        if (_is.rest()) {
                              // Enter a rest
                              nval = NoteVal();
                              }
                        else {
                              // Enter a note on the middle staff line
                              Staff* s = staff(_is.track() / VOICES);
                              int tick = _is.tick();
                              ClefType clef = s->clef(tick);
                              Key key = s->key(tick);
                              nval = NoteVal(line2pitch(4, clef, key));
                              }
                        setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction());
                        _is.moveToNextInputPos();
                        }
                  else
                        _is.setRest(false);
                  }
            }

      if (noteEntryMode() || !selection().isSingle()) {
            return;
            }

      //do not allow to add a dot on a full measure rest
      Element* e = selection().element();
      if (e && e->type() == Element::Type::REST) {
            Rest* r = static_cast<Rest*>(e);
            TDuration d = r->durationType();
            if (d.type() == TDuration::DurationType::V_MEASURE) {
                  _is.setDots(0);
                  }
            }

      // on measure rest, select the first actual rest
      ChordRest* cr = selection().cr();
      if (cr && cr->isRest() && cr->measure()->isMMRest()) {
            Measure* m = cr->measure()->mmRestFirst();
            if (m)
                  cr = m->findChordRest(0, 0);
            }

      if (!cr || cr->type() == Element::Type::REPEAT_MEASURE)
            return;

      if (cr->type() == Element::Type::CHORD && (static_cast<Chord*>(cr)->noteType() != NoteType::NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            undoChangeChordRestLen(cr, _is.duration());
            }
      else
            changeCRlen(cr, _is.duration());

      }

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(Element* el)
      {
      refresh |= el->abbox();
      _selection.remove(el);
      setSelectionChanged(true);
      }

//---------------------------------------------------------
//   select
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* e, SelectType type, int staffIdx)
      {
      if (e && (e->type() == Element::Type::NOTE || e->type() == Element::Type::REST)) {
            Element* ee = e;
            if (ee->type() == Element::Type::NOTE)
                  ee = ee->parent();
            int tick = static_cast<ChordRest*>(ee)->segment()->tick();
            if (playPos() != tick)
                  setPlayPos(tick);
            }
      if (MScore::debugMode)
            qDebug("select element <%s> type %d(state %d) staff %d",
               e ? e->name() : "", int(type), int(selection().state()), e ? e->staffIdx() : -1);

      switch (type) {
            case SelectType::SINGLE:     return selectSingle(e, staffIdx);
            case SelectType::ADD:        return selectAdd(e);
            case SelectType::RANGE:      return selectRange(e, staffIdx);
            }
      }

//---------------------------------------------------------
//   selectSingle
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::selectSingle(Element* e, int staffIdx)
      {
      SelState selState = _selection.state();
      deselectAll();
      if (e == 0) {
            selState = SelState::NONE;
            _updateAll = true;
            }
      else {
            if (e->type() == Element::Type::MEASURE) {
                  select(e, SelectType::RANGE, staffIdx);
                  return;
                  }
            refresh |= e->abbox();
            _selection.add(e);
            _is.setTrack(e->track());
            selState = SelState::LIST;
            if (e->type() == Element::Type::NOTE) {
                  e = e->parent();
                  }
            if (e->isChordRest()) {
                  _is.setLastSegment(_is.segment());
                  _is.setSegment(static_cast<ChordRest*>(e)->segment());
                  }
            }
      _selection.setActiveSegment(0);
      _selection.setActiveTrack(0);

      _selection.setState(selState);
      }

//---------------------------------------------------------
//   selectAdd
//---------------------------------------------------------

void Score::selectAdd(Element* e)
      {
      SelState selState = _selection.state();

      if (_selection.isRange()) {
            select(0, SelectType::SINGLE, 0);
            return;
            }

      if (e->type() == Element::Type::MEASURE) {
            Measure* m = static_cast<Measure*>(e);
            int tick  = m->tick();
            if (_selection.isNone()) {
                  _selection.setRange(m->tick2segment(tick),
                                      m == lastMeasure() ? 0 : m->last(),
                                      0,
                                      nstaves());
                  }
            else {
                  select(0, SelectType::SINGLE, 0);
                  return;
                  }
            _updateAll = true;
            selState = SelState::RANGE;
            _selection.updateSelectedElements();
            }
      else { // None or List
            refresh |= e->abbox();
            if (_selection.elements().contains(e))
                  _selection.remove(e);
            else {
                  _selection.add(e);
                  selState = SelState::LIST;
                  }
            }
      _selection.setState(selState);
      }

//---------------------------------------------------------
//   selectRange
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::selectRange(Element* e, int staffIdx)
      {
      int activeTrack = e->track();
      // current selection is range extending to end of score?
      bool endRangeSelected = selection().isRange() && selection().endSegment() == nullptr;
      if (e->type() == Element::Type::MEASURE) {
            Measure* m = static_cast<Measure*>(e);
            int tick  = m->tick();
            int etick = tick + m->ticks();
            activeTrack = staffIdx * VOICES;
            if (_selection.isNone()
               || (_selection.isList() && !_selection.isSingle())) {
                        if (_selection.isList())
                              deselectAll();
                  _selection.setRange(m->tick2segment(tick),
                                      m == lastMeasure() ? 0 : m->last(),
                                      staffIdx,
                                      staffIdx + 1);
                  }
            else if (_selection.isRange()) {
                  _selection.extendRangeSelection(m->tick2segment(tick),
                                                  m == lastMeasure() ? 0 : m->last(),
                                                  staffIdx,
                                                  tick,
                                                  etick);
                  }
            else if (_selection.isSingle()) {
                  Element* oe = selection().element();
                  if (oe->type() == Element::Element::Type::NOTE || oe->isChordRest()) {
                        if (oe->type() == Element::Element::Type::NOTE)
                              oe = oe->parent();

                        ChordRest* cr = static_cast<ChordRest*>(oe);
                        int oetick = cr->segment()->tick();
                        Segment* startSegment = cr->segment();
                        Segment* endSegment = m->last();
                        if (tick < oetick) {
                              startSegment = m->tick2segment(tick);
                              if (etick <= oetick)
                                    endSegment = cr->nextSegmentAfterCR(Segment::Type::ChordRest
                                                                                    | Segment::Type::EndBarLine
                                                                                    | Segment::Type::Clef);

                              }
                        int staffStart = staffIdx;
                        int endStaff = staffIdx + 1;
                        if (staffStart > cr->staffIdx())
                              staffStart = cr->staffIdx();
                        else if (cr->staffIdx() >= endStaff)
                              endStaff = cr->staffIdx() + 1;
                        _selection.setRange(startSegment, endSegment, staffStart, endStaff);
                        }
                  else {
                        deselectAll();
                        _selection.setRange(m->tick2segment(tick),
                                            m == lastMeasure() ? 0 : m->last(),
                                            staffIdx,
                                            staffIdx + 1);
                        }
                  }
            else {
                  qDebug("SELECT_RANGE: measure: sel state %d", int(_selection.state()));
                  return;
                  }
            }
      else if (e->type() == Element::Type::NOTE || e->isChordRest()) {
            if (e->type() == Element::Type::NOTE)
                  e = e->parent();
            ChordRest* cr = static_cast<ChordRest*>(e);

            if (_selection.isNone()
                || (_selection.isList() && !_selection.isSingle())) {
                  if (_selection.isList())
                        deselectAll();
                  _selection.setRange(cr->segment(),
                                      cr->nextSegmentAfterCR(Segment::Type::ChordRest
                                                             | Segment::Type::EndBarLine
                                                             | Segment::Type::Clef),
                                      e->staffIdx(),
                                      e->staffIdx() + 1);
                  activeTrack = cr->track();
                  }
            else if (_selection.isSingle()) {
                  Element* oe = _selection.element();
                  if (oe && (oe->type() == Element::Type::NOTE || oe->type() == Element::Type::REST)) {
                        if (oe->type() == Element::Type::NOTE)
                              oe = oe->parent();
                        ChordRest* ocr = static_cast<ChordRest*>(oe);

                        Segment* endSeg = tick2segmentMM(ocr->segment()->tick() + ocr->actualTicks());
                        if (!endSeg)
                              endSeg = ocr->segment()->next();

                        _selection.setRange(ocr->segment(),
                                            endSeg,
                                            oe->staffIdx(),
                                            oe->staffIdx() + 1);
                        _selection.extendRangeSelection(cr);

                        }
                  else {
                        select(e, SelectType::SINGLE, 0);
                        return;
                        }
                  }
            else if (_selection.isRange()) {
                  _selection.extendRangeSelection(cr);
                  }
            else {
                  qDebug("sel state %d", int(_selection.state()));
                  return;
                  }
            if (!endRangeSelected && !_selection.endSegment())
                  _selection.setEndSegment(cr->segment()->nextCR());
            if (!_selection.startSegment())
                  _selection.setStartSegment(cr->segment());
            }
      else {
            select(e, SelectType::SINGLE, staffIdx);
            return;
            }

      _selection.setActiveTrack(activeTrack);

      // doing this in note entry mode can clear selection
      if (_selection.startSegment() && !noteEntryMode())
            setPlayPos(_selection.startSegment()->tick());

      _selection.updateSelectedElements();
      }

//---------------------------------------------------------
//   collectMatch
//---------------------------------------------------------

void Score::collectMatch(void* data, Element* e)
      {
      ElementPattern* p = static_cast<ElementPattern*>(data);
      if (p->type != int(e->type()))
            return;

      if (p->type == int(Element::Type::NOTE)) {
            if (p->subtype < 0) {
                  if (!(static_cast<Note*>(e)->chord()->isGrace()))
                        return;
                  }
            else if ((static_cast<Note*>(e)->chord()->isGrace()) || (p->subtype != e->subtype()))
                  return;
            }
      else if (p->subtypeValid && p->subtype != e->subtype())
            return;

      if ((p->staffStart != -1)
         && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx())))
            return;

      if (p->voice != -1 && p->voice != e->voice())
            return;

      if (p->system) {
            Element* ee = e;
            do {
                  if (ee->type() == Element::Type::SYSTEM) {
                        if (p->system != ee)
                              return;
                        break;
                        }
                  ee = ee->parent();
                  } while (ee);
            }
      p->el.append(e);
      }

//---------------------------------------------------------
//   collectNoteMatch
//---------------------------------------------------------

void Score::collectNoteMatch(void* data, Element* e)
      {
      NotePattern* p = static_cast<NotePattern*>(data);
      if (e->type() != Element::Type::NOTE)
            return;
      Note* n = static_cast<Note*>(e);
      if (p->type != NoteType::INVALID && p->type != n->noteType())
            return;
      if (p->pitch != -1 && p->pitch != n->pitch())
            return;
      if (p->string != STRING_NONE && p->string != n->string())
            return;
      if (p->tpc != Tpc::TPC_INVALID && p->tpc != n->tpc())
            return;
      if (p->notehead != NoteHead::Group::HEAD_INVALID && p->notehead != n->headGroup())
            return;
      if (p->duration.type() != TDuration::DurationType::V_INVALID && p->duration != n->chord()->actualDurationType())
            return;
      if ((p->staffStart != -1)
         && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx())))
            return;
      if (p->voice != -1 && p->voice != e->voice())
            return;
      if (p->system && (p->system != n->chord()->segment()->system()))
            return;
      p->el.append(n);
      }


//---------------------------------------------------------
//   selectSimilar
//---------------------------------------------------------

void Score::selectSimilar(Element* e, bool sameStaff)
      {
      Element::Type type = e->type();
      Score* score = e->score();

      ElementPattern pattern;
      pattern.type = int(type);
      pattern.subtype = 0;
      pattern.subtypeValid = false;
      if (type == Element::Type::NOTE) {
            if (static_cast<Note*>(e)->chord()->isGrace())
                  pattern.subtype = -1; // hack
            else
                  pattern.subtype = e->subtype();
            }
      else if (type == Element::Type::SLUR_SEGMENT) {
            pattern.subtype = static_cast<int>(static_cast<SlurSegment*>(e)->spanner()->type());
            pattern.subtypeValid = true;
            }
      pattern.staffStart = sameStaff ? e->staffIdx() : -1;
      pattern.staffEnd = sameStaff ? e->staffIdx() + 1 : -1;
      pattern.voice   = -1;
      pattern.system  = 0;

      score->scanElements(&pattern, collectMatch);

      score->select(0, SelectType::SINGLE, 0);
      for (Element* e : pattern.el) {
            score->select(e, SelectType::ADD, 0);
            }
      }

//---------------------------------------------------------
//   selectSimilarInRange
//---------------------------------------------------------

void Score::selectSimilarInRange(Element* e)
      {
      Element::Type type = e->type();
      Score* score = e->score();

      ElementPattern pattern;
      pattern.type    = int(type);
      pattern.subtype = 0;
      pattern.subtypeValid = false;
      if (type == Element::Type::NOTE) {
            if (static_cast<Note*>(e)->chord()->isGrace())
                  pattern.subtype = -1; //hack
            else
                  pattern.subtype = e->subtype();
            pattern.subtypeValid = true;
            }
      else if (type == Element::Type::SLUR_SEGMENT) {
            pattern.subtype = static_cast<int>(static_cast<SlurSegment*>(e)->spanner()->type());
            pattern.subtypeValid = true;
            }
      pattern.staffStart = selection().staffStart();
      pattern.staffEnd = selection().staffEnd();
      pattern.voice   = -1;
      pattern.system  = 0;

      score->scanElementsInRange(&pattern, collectMatch);

      score->select(0, SelectType::SINGLE, 0);
      for (Element* e : pattern.el)
            score->select(e, SelectType::ADD, 0);
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, SelectType::SINGLE, 0);
      QRectF fr(bbox.normalized());
      foreach(Page* page, _pages) {
            QRectF pr(page->bbox());
            QRectF frr(fr.translated(-page->pos()));
            if (pr.right() < frr.left())
                  continue;
            if (pr.left() > frr.right())
                  break;

            QList<Element*> el = page->items(frr);
            for (int i = 0; i < el.size(); ++i) {
                  Element* e = el.at(i);
                  if (frr.contains(e->abbox())) {
                        if (e->type() != Element::Type::MEASURE && e->selectable())
                              select(e, SelectType::ADD, 0);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   lassoSelectEnd
//---------------------------------------------------------

void Score::lassoSelectEnd()
      {
      int noteRestCount     = 0;
      Segment* startSegment = 0;
      Segment* endSegment   = 0;
      int startStaff        = 0x7fffffff;
      int endStaff          = 0;
      const ChordRest* endCR = 0;

      if (_selection.elements().isEmpty()) {
            _selection.setState(SelState::NONE);
            _updateAll = true;
            return;
            }
      _selection.setState(SelState::LIST);

      foreach(const Element* e, _selection.elements()) {
            if (e->type() != Element::Type::NOTE && e->type() != Element::Type::REST)
                  continue;
            ++noteRestCount;
            if (e->type() == Element::Type::NOTE)
                  e = e->parent();
            Segment* seg = static_cast<const ChordRest*>(e)->segment();
            if ((startSegment == 0) || (*seg < *startSegment))
                  startSegment = seg;
            if ((endSegment == 0) || (*seg > *endSegment)) {
                  endSegment = seg;
                  endCR = static_cast<const ChordRest*>(e);
                  }
            int idx = e->staffIdx();
            if (idx < startStaff)
                  startStaff = idx;
            if (idx > endStaff)
                  endStaff = idx;
            }
      if (noteRestCount > 0) {
            endSegment = endCR->nextSegmentAfterCR(Segment::Type::ChordRest
               | Segment::Type::EndBarLine
               | Segment::Type::Clef);
            _selection.setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (!_selection.isRange())
                  _selection.setState(SelState::RANGE);
            _selection.updateSelectedElements();
            }
      _updateAll = true;
      }

//---------------------------------------------------------
//   addLyrics
//---------------------------------------------------------

void Score::addLyrics(int tick, int staffIdx, const QString& txt)
      {
      if (txt.trimmed().isEmpty())
            return;
      Measure* measure = tick2measure(tick);
      Segment* seg     = measure->findSegment(Segment::Type::ChordRest, tick);
      if (seg == 0) {
            qDebug("no segment found for lyrics<%s> at tick %d",
               qPrintable(txt), tick);
            return;
            }

      bool lyricsAdded = false;
      for (int voice = 0; voice < VOICES; ++voice) {
            int track = staffIdx * VOICES + voice;
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (cr) {
                  Lyrics* l = new Lyrics(this);
                  l->setXmlText(txt);
                  l->setTrack(track);
                  cr->add(l);
                  lyricsAdded = true;
                  break;
                  }
            }
      if (!lyricsAdded) {
            qDebug("no chord/rest for lyrics<%s> at tick %d, staff %d",
               qPrintable(txt), tick, staffIdx);
            }
      }

//---------------------------------------------------------
//   setTempo
//    convenience function to access TempoMap
//---------------------------------------------------------

void Score::setTempo(Segment* segment, qreal tempo)
      {
      setTempo(segment->tick(), tempo);
      }

void Score::setTempo(int tick, qreal tempo)
      {
      tempomap()->setTempo(tick, tempo);
      _playlistDirty = true;
      }

//---------------------------------------------------------
//   removeTempo
//---------------------------------------------------------

void Score::removeTempo(int tick)
      {
      tempomap()->delTempo(tick);
      _playlistDirty = true;
      }

//---------------------------------------------------------
//   setPause
//---------------------------------------------------------

void Score::setPause(int tick, qreal seconds)
      {
      tempomap()->setPause(tick, seconds);
      _playlistDirty = true;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Score::tempo(int tick) const
      {
      return tempomap()->tempo(tick);
      }

//---------------------------------------------------------
//   loWidth
//---------------------------------------------------------

qreal Score::loWidth() const
      {
      return pageFormat()->size().width() * DPI;
      }

//---------------------------------------------------------
//   loHeight
//---------------------------------------------------------

qreal Score::loHeight() const
      {
      return pageFormat()->size().height() * DPI;
      }

//---------------------------------------------------------
//   cmdSelectAll
//---------------------------------------------------------

void Score::cmdSelectAll()
      {
      if (_measures.size() == 0)
            return;
      deselectAll();
      Measure* first = firstMeasureMM();
      if (!first)
            return;
      Measure* last = lastMeasureMM();
      selectRange(first, 0);
      selectRange(last, nstaves() - 1);
      setUpdateAll(true);
      end();
      }

//---------------------------------------------------------
//   cmdSelectSection
//---------------------------------------------------------

void Score::cmdSelectSection()
      {
      Segment* s = _selection.startSegment();
      if (s == 0)
            return;
      MeasureBase* sm = s->measure();
      MeasureBase* em = sm;
      while (sm->prev()) {
            if (sm->prev()->sectionBreak())
                  break;
            sm = sm->prev();
            }
      while (em->next()) {
            if (em->sectionBreak())
                  break;
            em = em->next();
            }
      while (sm && sm->type() != Element::Type::MEASURE)
            sm = sm->next();
      while (em && em->type() != Element::Type::MEASURE)
            em = em->next();
      if (sm == 0 || em == 0)
            return;

      _selection.setRange(static_cast<Measure*>(sm)->first(),
         static_cast<Measure*>(em)->last(), 0, nstaves());
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void Score::undo(UndoCommand* cmd) const
      {
      undo()->push(cmd);
      }

//---------------------------------------------------------
//   linkId
//---------------------------------------------------------

int Score::linkId()
      {
      return (rootScore()->_linkId)++;
      }

// val is a used link id
void Score::linkId(int val)
      {
      Score* s = rootScore();
      if (val >= s->_linkId)
            s->_linkId = val + 1;   // update unused link id
      }

//---------------------------------------------------------
//   scoreList
//    return a list of scores containing the root score
//    and all part scores (if there are any)
//---------------------------------------------------------

QList<Score*> Score::scoreList()
      {
      QList<Score*> scores;
      Score* root = rootScore();
      scores.append(root);
      for (const Excerpt* ex : root->excerpts()) {
            if (ex->partScore())
                  scores.append(ex->partScore());
            }
      return scores;
      }

//---------------------------------------------------------
//   switchLayer
//---------------------------------------------------------

bool Score::switchLayer(const QString& s)
      {
      int layerIdx = 0;
      for (const Layer& l : layer()) {
            if (s == l.name) {
                  setCurrentLayer(layerIdx);
                  return true;
                  }
            ++layerIdx;
            }
      return false;
      }

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void Score::appendPart(const QString& name)
      {
      static InstrumentTemplate defaultInstrument;
      InstrumentTemplate* t;

      t = searchTemplate(name);
      if (t == 0) {
            qDebug("appendPart: <%s> not found", qPrintable(name));
            t = &defaultInstrument;
            }

      if (t->channel.isEmpty()) {
            Channel a;
            a.chorus = 0;
            a.reverb = 0;
            a.name   = "normal";
            a.bank   = 0;
            a.volume = 100;
            a.pan    = 64; // actually 63.5 for center
            t->channel.append(a);
            }
      Part* part = new Part(this);
      part->initFromInstrTemplate(t);
      int n = nstaves();
      for (int i = 0; i < t->nstaves(); ++i) {
            Staff* staff = new Staff(this);
            staff->setPart(part);
            staff->setLines(t->staffLines[i]);
            staff->setSmall(t->smallStaff[i]);
            if (i == 0) {
                  staff->setBracket(0, t->bracket[0]);
                  staff->setBracketSpan(0, t->nstaves());
                  }
            undoInsertStaff(staff, i);
            }

      part->staves()->front()->setBarLineSpan(part->nstaves());
      undoInsertPart(part, n);
      fixTicks();
      rebuildMidiMapping();
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n)
      {
      for (int i = 0; i < n; ++i)
            insertMeasure(Element::Type::MEASURE, 0, false);
      }

#ifdef SCRIPT_INTERFACE
//---------------------------------------------------------
//   addText
//---------------------------------------------------------

void Score::addText(const QString& type, const QString& txt)
      {
      MeasureBase* measure = first();
      if (measure == 0 || measure->type() != Element::Type::VBOX) {
            insertMeasure(Element::Type::VBOX, measure);
            measure = first();
            }
      Text* text = new Text(this);
      if (type == "title")
            text->setTextStyleType(TextStyleType::TITLE);
      else if (type == "subtitle")
            text->setTextStyleType(TextStyleType::SUBTITLE);
      text->setParent(measure);
      text->setXmlText(txt);
      undoAddElement(text);
      }

//---------------------------------------------------------
//   newCursor
//---------------------------------------------------------

Cursor* Score::newCursor()
      {
      return new Cursor(this);
      }
#endif

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Score::addSpanner(Spanner* s)
      {
      _spanner.addSpanner(s);
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeSpanner(Spanner* s)
      {
      _spanner.removeSpanner(s);
      }

//---------------------------------------------------------
//   isSpannerStartEnd
//    does is spanner start or end at tick position tick
//    for track ?
//---------------------------------------------------------

bool Score::isSpannerStartEnd(int tick, int track) const
      {
      for (auto i : _spanner.map()) {
            if (i.second->track() != track)
                  continue;
            if (i.second->tick() == tick || i.second->tick2() == tick)
                  return true;
            }
      return false;
      }

void Score::insertTime(int tick, int len)
      {
      for (Staff* staff : staves())
            staff->insertTime(tick, len);
      for (Part* part : parts())
            part->insertTime(tick, len);
      }

//---------------------------------------------------------
//   addUnmanagedSpanner
//---------------------------------------------------------

void Score::addUnmanagedSpanner(Spanner* s)
      {
      _unmanagedSpanner.insert(s);
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Score::removeUnmanagedSpanner(Spanner* s)
      {
      _unmanagedSpanner.erase(s);
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Score::setPos(POS pos, int tick)
      {
      if (tick < 0)
            tick = 0;
      if (tick != _pos[int(pos)])
            _pos[int(pos)] = tick;
      // even though tick position might not have changed, layout might have
      // so we should update cursor here
      // however, we must be careful not to call setPos() again while handling posChanged, or recursion results
      emit posChanged(pos, unsigned(tick));
      }

//---------------------------------------------------------
//   uniqueStaves
//---------------------------------------------------------

QList<int> Score::uniqueStaves() const
      {
      QList<int> sl;

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* s = staff(staffIdx);
            if (s->linkedStaves()) {
                  bool alreadyInList = false;
                  for (int idx : sl) {
                        if (s->linkedStaves()->staves().contains(staff(idx))) {
                              alreadyInList = true;
                              break;
                              }
                        }
                  if (alreadyInList)
                        continue;
                  }
            sl.append(staffIdx);
            }
      return sl;
      }

//---------------------------------------------------------
//   findCR
//    find chord/rest <= tick in track
//---------------------------------------------------------

ChordRest* Score::findCR(int tick, int track) const
      {
      Measure* m = tick2measureMM(tick);
      if (!m) {
            qDebug("findCR: no measure for tick %d", tick);
            return nullptr;
            }
      // attach to first rest all spanner when mmRest
      if (m->isMMRest())
            tick = m->tick();
      Segment* s = m->first(Segment::Type::ChordRest);
      for (Segment* ns = s; ; ns = ns->next(Segment::Type::ChordRest)) {
            if (ns == 0 || ns->tick() > tick)
                  break;
            if (ns->element(track))
                  s = ns;
            }
      if (s)
            return static_cast<ChordRest*>(s->element(track));
      return nullptr;
      }

//---------------------------------------------------------
//   findCRinStaff
//    find last chord/rest on staff that ends before tick
//---------------------------------------------------------

ChordRest* Score::findCRinStaff(int tick, int staffIdx) const
      {
      int ptick = tick - 1;
      Measure* m = tick2measureMM(ptick);
      if (!m) {
            qDebug("findCRinStaff: no measure for tick %d", ptick);
            return nullptr;
            }
      // attach to first rest all spanner when mmRest
      if (m->isMMRest())
            ptick = m->tick();
      Segment* s = m->first(Segment::Type::ChordRest);
      int strack = staffIdx * VOICES;
      int etrack = strack + VOICES;
      int actualTrack = strack;

      int lastTick = -1;
      for (Segment* ns = s; ; ns = ns->next(Segment::Type::ChordRest)) {
            if (ns == 0 || ns->tick() > ptick)
                  break;
            // found a segment; now find longest cr on this staff that does not overlap tick
            for (int t = strack; t < etrack; ++t) {
                  ChordRest* cr = static_cast<ChordRest*>(ns->element(t));
                  if (cr) {
                        int endTick = cr->tick() + cr->actualTicks();
                        // allow fudge factor for tuplets
                        // TODO: replace with fraction-based calculation
                        int fudge = cr->tuplet() ? 5 : 0;
                        if (endTick + fudge >= lastTick && endTick - fudge <= tick) {
                              s = ns;
                              actualTrack = t;
                              lastTick = endTick;
                              }
                        }
                  }
            }
      if (s)
            return static_cast<ChordRest*>(s->element(actualTrack));
      return nullptr;
      }

//---------------------------------------------------------
//   setSoloMute
//   called once at opening file, adds soloMute marks
//---------------------------------------------------------

void Score::setSoloMute()
      {
      for (int i = 0; i < _midiMapping.size(); i++) {
            Channel* b = _midiMapping[i].articulation;
            if (b->solo) {
                  b->soloMute = false;
                  for (int j = 0; j < _midiMapping.size(); j++) {
                        Channel* a = _midiMapping[j].articulation;
                        a->soloMute = (i != j && !a->solo);
                        a->solo     = (i == j || a->solo);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setName
//---------------------------------------------------------

void Score::setName(QString s)
      {
      s.replace('/', '_');    // for sanity
      if (!(s.endsWith(".mscz") || s.endsWith(".mscx")))
            s += ".mscz";
      info.setFile(s);
      }

//---------------------------------------------------------
//   setImportedFilePath
//---------------------------------------------------------

void Score::setImportedFilePath(const QString& filePath)
      {
      _importedFilePath = filePath;
      }

//---------------------------------------------------------
//   title
//---------------------------------------------------------

QString Score::title()
      {
      QString fn;
      Text* t = getText(TextStyleType::TITLE);
      if (t)
            fn = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&").replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");

      if (fn.isEmpty())
            fn = metaTag("workTitle");

      if (fn.isEmpty())
            fn = fileInfo()->completeBaseName();

      if (fn.isEmpty())
            fn = "Untitled";

      return fn.simplified();
      }

//---------------------------------------------------------
//   subtitle
//---------------------------------------------------------

QString Score::subtitle()
      {
      QString fn;
      Text* t = getText(TextStyleType::SUBTITLE);
      if (t)
            fn = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&").replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");

      return fn.simplified();
      }

//---------------------------------------------------------
//   composer
//---------------------------------------------------------

QString Score::composer()
      {
      QString fn;
      Text* t = getText(TextStyleType::COMPOSER);
      if (t)
            fn = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&").replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");

      if (fn.isEmpty())
            fn = metaTag("composer");

      return fn.simplified();
      }

//---------------------------------------------------------
//   poet
//---------------------------------------------------------

QString Score::poet()
      {
      QString fn;
      Text* t = getText(TextStyleType::POET);
      if (t)
            fn = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&").replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");

      if (fn.isEmpty())
            fn = metaTag("lyricist");

      if (fn.isEmpty())
            fn = "";

      return fn.simplified();
      }

//---------------------------------------------------------
//   nmeasure
//---------------------------------------------------------

int Score::nmeasures()
      {
      int n = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            n++;
      return n;
      }

//---------------------------------------------------------
//   hasLyrics
//---------------------------------------------------------

bool Score::hasLyrics()
      {
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (int i = 0; i < ntracks(); ++i) {
                  if (seg->lyricsList(i) && seg->lyricsList(i)->size() > 0)
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   hasHarmonies
//---------------------------------------------------------

bool Score::hasHarmonies()
      {
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (Element* e : seg->annotations()) {
                  if (e->type() == Element::Type::HARMONY)
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   lyricCount
//---------------------------------------------------------

int Score::lyricCount()
      {
      int count = 0;
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (int i = 0; i < ntracks(); ++i) {
                  if (seg->lyricsList(i))
                        count += seg->lyricsList(i)->size();
                  }
            }
      return count;
      }

//---------------------------------------------------------
//   harmonyCount
//---------------------------------------------------------

int Score::harmonyCount()
      {
      int count = 0;
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (Element* e : seg->annotations()) {
                  if (e->type() == Element::Type::HARMONY)
                        count++;
                  }
            }
      return count;
      }

QString Score::extractLyrics()
      {
      QString result;
      updateRepeatList(true);
      setPlaylistDirty();
      Segment::Type st = Segment::Type::ChordRest;
      for (int track = 0; track < ntracks(); track += VOICES) {
            bool found = false;
            int maxLyrics = 1;
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                  m->setPlaybackCount(0);
                  }
            // follow the repeat segments
            for (const RepeatSegment* rs : *repeatList()) {
                  int startTick  = rs->tick;
                  int endTick    = startTick + rs->len();
                  for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                        int playCount = m->playbackCount();
                        for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                              // consider voice 1 only
                              if (seg->lyricsList(track) == nullptr || seg->lyricsList(track)->size() == 0)
                                    continue;
                              if (seg->lyricsList(track)->size() > maxLyrics)
                                    maxLyrics = seg->lyricsList(track)->size();
                              if (playCount >= seg->lyricsList(track)->size())
                                    continue;
                              Lyrics* l = seg->lyricsList(track)->at(playCount);
                              if (!l)
                                    continue;
                              found = true;
                              QString lyric = l->plainText().trimmed();
                              if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END)
                                    result += lyric + " ";
                              else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics::Syllabic::MIDDLE)
                                    result += lyric;
                              }
                        m->setPlaybackCount(m->playbackCount() + 1);
                        if (m->tick() + m->ticks() >= endTick)
                              break;
                        }
                  }
            // consider remaning lyrics
            for (int lyricsNumber = 0; lyricsNumber < maxLyrics; lyricsNumber++) {
                  for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                        int playCount = m->playbackCount();
                        if (lyricsNumber >= playCount) {
                              for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                                    // consider voice 1 only
                                    if (seg->lyricsList(track) == nullptr || seg->lyricsList(track)->size() == 0)
                                          continue;
                                    if (seg->lyricsList(track)->size() > maxLyrics)
                                          maxLyrics = seg->lyricsList(track)->size();
                                    if (lyricsNumber >= seg->lyricsList(track)->size())
                                          continue;
                                    Lyrics* l = seg->lyricsList(track)->at(lyricsNumber);
                                    if (!l)
                                          continue;
                                    found = true;
                                    QString lyric = l->plainText().trimmed();
                                    if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END)
                                          result += lyric + " ";
                                    else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics:: Syllabic::MIDDLE)
                                          result += lyric;
                                    }
                              }
                        }
                  }
            if (found)
                  result += "\n\n";
            }
      return result.trimmed();
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

int Score::keysig()
      {
      Key result = Key::C;
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* st = staff(staffIdx);
            Key key = st->key(0);
            if (st->staffType()->group() == StaffGroup::PERCUSSION || st->keySigEvent(0).custom() || st->keySigEvent(0).isAtonal())       // ignore percussion and custom / atonal key
                  continue;
            result = key;
            int diff = st->part()->instrument()->transpose().chromatic;
            if (!styleB(StyleIdx::concertPitch) && diff)
                  result = transposeKey(key, diff);
            break;
            }
      return int(result);
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

int Score::duration()
      {
      updateRepeatList(true);
      if (repeatList()->isEmpty())
            return 0;
      RepeatSegment* rs = repeatList()->last();
      return lrint(utick2utime(rs->utick + rs->len()));
      }

//---------------------------------------------------------
//   createRehearsalMarkText
//---------------------------------------------------------

QString Score::createRehearsalMarkText(RehearsalMark* current) const
      {
      int tick = current->segment()->tick();
      RehearsalMark* before = 0;
      RehearsalMark* after = 0;
      for (Segment* s = firstSegment(); s; s = s->next1()) {
            for (Element* e : s->annotations()) {
                  if (e && e->type() == Element::Type::REHEARSAL_MARK) {
                        if (s->tick() < tick)
                              before = static_cast<RehearsalMark*>(e);
                        else if (s->tick() > tick) {
                              after = static_cast<RehearsalMark*>(e);
                              break;
                              }
                        }
                  }
            if (after)
                  break;
            }
      QString s = "A";
      QString s1 = before ? before->xmlText() : "";
      QString s2 = after ? after->xmlText()  : "";
      if (s1.isEmpty())
            return s;
      s = nextRehearsalMarkText(before, current);     // try to sequence
      if (s == current->xmlText()) {
            // no sequence detected (or current happens to be correct)
            return s;
            }
      else if (s == s2) {
            // next in sequence already present
            if (s1[0].isLetter()) {
                  if (s1.size() == 2)
                        s = s1[0] + QChar::fromLatin1(s1[1].toLatin1() + 1);  // BB, BC, CC
                  else
                        s = s1 + QChar::fromLatin1('1');                      // B, B1, C
                  }
            else {
                  s = s1 + QChar::fromLatin1('A');                            // 2, 2A, 3
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   nextRehearsalMarkText
//    finds next rehearsal in sequence established by previous
//     Alphabetic sequences:
//      A, B, ..., Y, Z, AA, BB, ..., YY, ZZ
//      a, b, ..., y, z, aa, bb, ..., yy, zz
//     Numeric sequences:
//      1, 2, 3, ...
//      If number of previous rehearsal mark matches measure number, assume use of measure numbers throughout
//---------------------------------------------------------

QString Score::nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const
      {
      QString previousText = previous->xmlText();
      QString fallback = current ? current->xmlText() : previousText + "'";

      if (previousText.length() == 1 && previousText[0].isLetter()) {
            // single letter sequence
            if (previousText == "Z")
                  return "AA";
            else if (previousText == "z")
                  return "aa";
            else
                  return QChar::fromLatin1(previousText[0].toLatin1() + 1);
            }
      else if (previousText.length() == 2 && previousText[0].isLetter() && previousText[1].isLetter()) {
            // double letter sequence
            if (previousText[0] == previousText[1]) {
                  // repeated letter sequence
                  if (previousText.toUpper() != "ZZ") {
                        QString c = QChar::fromLatin1(previousText[0].toLatin1() + 1);
                        return c + c;
                        }
                  else {
                        return fallback;
                        }
                  }
            else {
                  return fallback;
                  }
            }
      else {
            // try to interpret as number
            bool ok;
            int n = previousText.toInt(&ok);
            if (!ok) {
                  return fallback;
                  }
            else if (current && n == previous->segment()->measure()->no() + 1) {
                  // use measure number
                  n = current->segment()->measure()->no() + 1;
                  return QString("%1").arg(n);
                  }
            else {
                  // use number sequence
                  n = previousText.toInt() + 1;
                  return QString("%1").arg(n);
                  }
            }
      }

//---------------------------------------------------------
//   changeVoice
//    moves selected notes into specified voice if possible
//---------------------------------------------------------

void Score::changeVoice(int voice)
      {
      startCmd();
      QList<Element*> el;
      QList<Element*> oel = selection().elements();     // make copy
      for (Element* e : oel) {
            if (e->type() == Element::Type::NOTE) {
                  Note* note   = static_cast<Note*>(e);
                  Chord* chord = note->chord();

                  // TODO - handle ties; for now we skip tied notes
                  if (note->tieFor() || note->tieBack())
                        continue;

                  // move grace notes with main chord only
                  if (chord->isGrace())
                        continue;

                  if (chord->voice() != voice) {
                        Segment* s       = chord->segment();
                        Measure* m       = s->measure();
                        int notes        = chord->notes().size();
                        int dstTrack     = chord->staffIdx() * VOICES + voice;
                        ChordRest* dstCR = static_cast<ChordRest*>(s->element(dstTrack));
                        Chord* dstChord  = nullptr;

                        // set up destination chord

                        if (dstCR && dstCR->type() == Element::Type::CHORD && dstCR->globalDuration() == chord->globalDuration()) {
                              // existing chord in destination with correct duration;
                              //   can simply move note in
                              dstChord = static_cast<Chord*>(dstCR);
                              }

                        else if (dstCR && dstCR->type() == Element::Type::REST && dstCR->globalDuration() == chord->globalDuration()) {
                              // existing rest in destination with correct duration;
                              //   replace with chord, then move note in
                              //   this case allows for tuplets, unlike the more general case below
                              dstChord = new Chord(this);
                              dstChord->setTrack(dstTrack);
                              dstChord->setDurationType(chord->durationType());
                              dstChord->setDuration(chord->duration());
                              dstChord->setTuplet(dstCR->tuplet());
                              dstChord->setParent(s);
                              undoRemoveElement(dstCR);
                              }

                        else if (!chord->tuplet()) {
                              // rests or gap in destination
                              //   insert new chord if the rests / gap are long enough
                              //   then move note in
                              ChordRest* pcr = nullptr;
                              ChordRest* ncr = nullptr;
                              for (Segment* s2 = m->first(Segment::Type::ChordRest); s2; s2 = s2->next()) {
                                    if (s2->segmentType() != Segment::Type::ChordRest)
                                          continue;
                                    ChordRest* cr2 = static_cast<ChordRest*>(s2->element(dstTrack));
                                    if (!cr2 || cr2->type() == Element::Type::REST)
                                          continue;
                                    if (s2->tick() < s->tick()) {
                                          pcr = cr2;
                                          continue;
                                          }
                                    else if (s2->tick() >= s->tick()) {
                                          ncr = cr2;
                                          break;
                                          }
                                    }
                              int gapStart = pcr ? pcr->tick() + pcr->actualTicks() : m->tick();
                              int gapEnd = ncr ? ncr->tick() : m->tick() + m->ticks();
                              if (gapStart <= s->tick() && gapEnd >= s->tick() + chord->actualTicks()) {
                                    // big enough gap found
                                    dstChord = new Chord(this);
                                    dstChord->setTrack(dstTrack);
                                    dstChord->setDurationType(chord->durationType());
                                    dstChord->setDuration(chord->duration());
                                    dstChord->setParent(s);
                                    // makeGapVoice will not back-fill an empty voice
                                    if (voice && !dstCR)
                                          expandVoice(s, /*m->first(Segment::Type::ChordRest,*/ dstTrack);
                                    makeGapVoice(s, dstTrack, chord->actualFraction(), s->tick());
                                    }
                              }

                        // move note to destination chord
                        if (dstChord) {
                              // create & add new note
                              Note* newNote = new Note(*note);
                              newNote->setSelected(false);
                              newNote->setParent(dstChord);
                              undoAddElement(newNote);
                              el.append(newNote);
                              // add new chord if one was created
                              if (dstChord != dstCR)
                                    undoAddCR(dstChord, m, s->tick());
                              // remove original note
                              if (notes > 1) {
                                    undoRemoveElement(note);
                                    }
                              else if (notes == 1) {
                                    // create rest to leave behind
                                    Rest* r = new Rest(this);
                                    r->setTrack(chord->track());
                                    r->setDurationType(chord->durationType());
                                    r->setDuration(chord->duration());
                                    r->setTuplet(chord->tuplet());
                                    r->setParent(s);
                                    // if there were grace notes, move them
                                    for (Chord* gc : chord->graceNotes()) {
                                          Chord* ngc = new Chord(*gc);
                                          undoRemoveElement(gc);
                                          ngc->setParent(dstChord);
                                          ngc->setTrack(dstChord->track());
                                          undoAddElement(ngc);
                                          }
                                    // remove chord, replace with rest
                                    undoRemoveElement(chord);
                                    undoAddCR(r, m, s->tick());
                                    }
                              }
                        }
                  }
            }

      if (!el.isEmpty())
            selection().clear();
      for (Element* e : el)
            select(e, SelectType::ADD, -1);
      setLayoutAll(true);
      endCmd();
      }

//---------------------------------------------------------
//   cropPage - crop a single page score to the content
///    margins will be applied on the 4 sides
//---------------------------------------------------------

void Score::cropPage(qreal margins)
      {
      if (npages() == 1) {
            Page* page = pages()[0];
            if (page) {
                  QRectF ttbox = page->tbbox();

                  PageFormat* curFormat = pageFormat();
                  PageFormat f;
                  f.copy(*curFormat);

                  qreal margin = margins / INCH;
                  f.setSize(QSizeF((ttbox.width() / DPI) + 2 * margin, (ttbox.height()/ DPI) + 2 * margin));

                  qreal offset = curFormat->oddLeftMargin() - ttbox.x() / DPI;
                  if (offset < 0)
                        offset = 0.0;
                  f.setOddLeftMargin(margin + offset);
                  f.setEvenLeftMargin(margin + offset);
                  f.setOddBottomMargin(margin);
                  f.setOddTopMargin(margin);
                  f.setEvenBottomMargin(margin);
                  f.setEvenTopMargin(margin);

                  undoChangePageFormat(&f, spatium(), pageNumberOffset());
                  }
            }
      }

//---------------------------------------------------------
//   switchToPageMode
//    switch to layout mode PAGE
//---------------------------------------------------------

void Score::switchToPageMode()
      {
      if (layoutMode() != LayoutMode::PAGE) {
            startCmd();
            ScoreElement::undoChangeProperty(P_ID::LAYOUT_MODE, int(LayoutMode::PAGE));
            doLayout();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Score::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::LAYOUT_MODE:
                  return QVariant(static_cast<int>(_layoutMode));
            default:
                  qDebug("Score::getProperty: unhandled id");
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Score::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::LAYOUT_MODE:
                  setLayoutMode(LayoutMode(v.toInt()));
                  break;
            default:
                  qDebug("Score::setProperty: unhandled id");
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Score::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LAYOUT_MODE:
                  return static_cast<int>(LayoutMode::PAGE);
            default:
                  return QVariant();
            }
      }

}


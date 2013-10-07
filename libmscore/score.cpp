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

namespace Ms {

Score* gscore;                 ///< system score, used for palettes etc.
QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

bool layoutDebug     = false;
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
//   erase
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
      if (nb->type() == Element::HBOX || nb->type() == Element::VBOX
         || nb->type() == Element::TBOX || nb->type() == Element::FBOX)
            nb->setSystem(ob->system());
      foreach(Element* e, *nb->el())
            e->setParent(nb);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Score::init()
      {
      _linkId         = 0;
      _currentLayer   = 0;
      _playMode       = PLAYMODE_SYNTHESIZER;
      Layer l;
      l.name          = "default";
      l.tags          = 1;
      _layer.append(l);
      _layerTags[0]   = "default";

      if (!_parentScore) {
#ifdef Q_OS_WIN
            _metaTags.insert("platform", "WIN");
#endif
#ifdef Q_OS_MAC
            _metaTags.insert("platform", "MAC");
#endif
#ifdef Q_OS_LINUX
            _metaTags.insert("platform", "X11");
#endif
            _metaTags.insert("movementNumber", "");
            _metaTags.insert("movementTitle", "");
            _metaTags.insert("workNumber", "");
            _metaTags.insert("workTitle", "");
            /* TODO enable following block of code
             * This adds arranger, composer, lyricist, poet and translator fields
             * to the File / Info dialog.
             * As these fields are saved in every .msc[xz] file, it requires updating
             * all regression test files in the mtest directory.
            _metaTags.insert("arranger", "");
            _metaTags.insert("composer", "");
            _metaTags.insert("lyricist", "");
            _metaTags.insert("poet", "");
            _metaTags.insert("translator", "");
             */
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

      _revisions      = new Revisions;
      _symIdx         = 0;
      _pageNumberOffset = 0;
      int numOfPresets = StaffType::numOfPresets();
      for (int idx = 0; idx < numOfPresets; idx++) {
            StaffType * st = StaffType::preset(idx)->clone();
            addStaffType(st);
            }

      _mscVersion     = MSCVERSION;
      _created        = false;

      _updateAll      = true;
      _layoutAll      = true;
      layoutFlags     = 0;
      _undoRedo       = false;
      _playNote       = false;
      _excerptsChanged = false;
      _instrumentsChanged = false;
      _selectionChanged   = false;

      keyState             = 0;
      _showInvisible       = true;
      _showUnprintable     = true;
      _showFrames          = true;
      _showPageborders     = false;
      _showInstrumentNames = true;
      _showVBox            = true;

      _printing       = false;
      _playlistDirty  = false;
      _autosaveDirty  = false;
      _dirty          = false;
      _saved          = false;
      _playPos        = 0;
      _loopInTick     = -1;
      _loopOutTick    = -1;
      _fileDivision   = MScore::division;
      _defaultsRead   = false;
      _omr            = 0;
      _audio          = 0;
      _showOmr        = false;
      _sigmap         = 0;
      _tempomap       = 0;
      _layoutMode     = LayoutPage;
      _noteHeadWidth  = symbols[_symIdx][quartheadSym].width(spatium() / (MScore::DPI * SPATIUM20));
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
   : QObject(0), _selection(this)
      {
      _parentScore = 0;
      init();
      _tempomap = new TempoMap;
      _sigmap   = new TimeSigMap();
      _style    = *(MScore::defaultStyle());
      }

Score::Score(const MStyle* s)
   : _selection(this)
      {
      _parentScore = 0;
      init();
      _tempomap = new TempoMap;
      _sigmap   = new TimeSigMap();
      _style    = *s;
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
   : _selection(this)
      {
      _parentScore = parent;
      init();

      _style = *parent->style();
      if (!MScore::partStyle.isEmpty()) {
            QFile f(MScore::partStyle);
            if (f.open(QIODevice::ReadOnly))
                  _style.load(&f);
            }
      _synthesizerState = parent->_synthesizerState;
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
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
      foreach(StaffType** st, _staffTypes) {
            if (!(*st)->builtin())
                  delete *st;
            delete st;
            }

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

      TimeSigMap* smap = sigmap();
      Fraction sig(fm->len());
      Fraction nsig(fm->timesig());
      if (!parentScore()) {
            tempomap()->clear();
            smap->clear();
            smap->add(0, SigEvent(sig,  nsig, 0));
            }

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != Element::MEASURE) {
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

            if (!parentScore()) {
                  //
                  //  implement section break rest
                  //
                  if (m->sectionBreak())
                        setPause(m->tick() + m->ticks(), m->pause());

                  //
                  // implement fermata as a tempo change
                  //
                  Segment::SegmentTypes st = Segment::SegChordRest | Segment::SegBreath;

                  for (Segment* s = m->first(st); s; s = s->next(st)) {
                        if (s->segmentType() == Segment::SegBreath) {
                              setPause(s->tick(), .1);
                              }
                        else {
                              foreach(Element* e, s->annotations()) {
                                    if (e->type() == Element::TEMPO_TEXT) {
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
                  }

            //
            // update time signature map
            //
            if (!parentScore() && (m->len() != sig)) {
                  sig = m->len();
                  smap->add(tick, SigEvent(sig, m->timesig(),  m->no()));
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
//      qreal sy1 = 0;
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
//                  sy1 = sy2;
                  i   = ni;
                  continue;
                  }
            break;
            }

      // search for segment + offset
      QPointF pppp = p - m->canvasPos();
      int strack = i * VOICES;
      int etrack = staff(i)->part()->nstaves() * VOICES + strack;

      SysStaff* sstaff = m->system()->staff(i);
      Segment::SegmentTypes st = Segment::SegChordRest;
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
      _updateAll     = true;
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
//   setDirty
//---------------------------------------------------------

void Score::setDirty(bool val)
      {
      if (_dirty != val) {
            _dirty         = val;
            _playlistDirty = true;
            }
      if (_dirty) {
            _playlistDirty = true;
            _autosaveDirty = true;
            }
      }

//---------------------------------------------------------
//   playlistDirty
//---------------------------------------------------------

bool Score::playlistDirty()
      {
      bool val = _playlistDirty;
      _playlistDirty = false;
      return val;
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
                        if (e && e->type() == Element::CHORD)
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
                        if (e && e->type() == Element::CHORD)
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
            if (seg->segmentType() == Segment::SegChordRest) {
                  for (int track = startTrack; track >= endTrack; --track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == Element::CHORD)
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
            if (seg->segmentType() == Segment::SegChordRest) {
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == Element::CHORD) {
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
//   setInputState
//---------------------------------------------------------

void Score::setInputState(const InputState& st)
      {
      _is = st;
      }

//---------------------------------------------------------
//   setInputTrack
//---------------------------------------------------------

void Score::setInputTrack(int v)
      {
      if (v < 0) {
            qDebug("setInputTrack: bad value: %d", v);
            return;
            }
      _is.setTrack(v);
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
      int port    = 0;
      int channel = 0;
      int idx     = 0;
      foreach(Part* part, _parts) {
            InstrumentList* il = part->instrList();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  bool drum = i->second.useDrumset();
                  for (int k = 0; k < i->second.channel().size(); ++k) {
                        Channel* a = &(i->second.channel(k));
                        MidiMapping mm;
                        if (drum) {
                              mm.port    = port;
                              mm.channel = 9;
                              }
                        else {
                              mm.port    = port;
                              mm.channel = channel;
                              if (channel == 15) {
                                    channel = 0;
                                    ++port;
                                    }
                              else {
                                    ++channel;
                                    if (channel == 9)
                                          ++channel;
                                    }
                              }
                        mm.part         = part;
                        mm.articulation = a;
                        _midiMapping.append(mm);
                        a->channel = idx;
                        ++idx;
                        }
                  }
            }
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
      foreach(Page* page, pages()) {
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
            else  {
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
                  if (mb->type() != Element::MEASURE)
                        continue;
                  if (x < (mb->x() + mb->bbox().width()))
                        return static_cast<Measure*>(mb);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//    getNextValidInputSegment
//    - s is of type Segment::SegChordRest
//---------------------------------------------------------

static Segment* getNextValidInputSegment(Segment* s, int track, int voice)
      {
      if (s == 0)
            return 0;
      assert(s->segmentType() == Segment::SegChordRest);
      // Segment* s1 = s;
      ChordRest* cr1;
      for (Segment* s1 = s; s1; s1 = s1->prev(Segment::SegChordRest)) {
            cr1 = static_cast<ChordRest*>(s1->element(track + voice));
            if (cr1)
                  break;
            }
      int nextTick = (cr1 == 0) ? s->measure()->tick() : cr1->tick() + cr1->actualTicks();

      static const Segment::SegmentTypes st = Segment::SegChordRest;
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
            qreal sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            if ((pos->staffIdx+1) != nstaves()) {
                  SysStaff* nstaff = system->staff(pos->staffIdx+1);
                  qreal s1y2 = ss->bbox().y() + ss->bbox().height();
                  sy2         = s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = system->page()->height() - system->pos().y();   // system->height();
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

      for (segment = measure->first(Segment::SegChordRest); segment;) {
            segment = getNextValidInputSegment(segment, track, voice);
            if (segment == 0)
                  break;
            Segment* ns = getNextValidInputSegment(segment->next(Segment::SegChordRest), track, voice);

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
      _noteHeadWidth = symbols[_symIdx][quartheadSym].width(newValue / (MScore::DPI * SPATIUM20));
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
                  add(m);
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
#if 0
      if (MScore::debugMode) {
            qDebug("   Score(%p)::addElement %p(%s) parent %p(%s)",
               this, element, element->name(), element->parent(),
               element->parent() ? element->parent()->name() : "");
            }
#endif

      if (element->parent() && element->parent()->type() == Element::SEGMENT)
            static_cast<Segment*>(element->parent())->measure()->setDirty();

      Element::ElementType et = element->type();
      if (et == Element::TREMOLO)
            setLayoutAll(true);
      else if (et == Element::MEASURE
         || (et == Element::HBOX && element->parent()->type() != Element::VBOX)
         || et == Element::VBOX
         || et == Element::TBOX
         || et == Element::FBOX
         ) {
            add(element);
            addLayoutFlags(LAYOUT_FIX_TICKS);
            return;
            }

      if (element->parent() == 0)
            add(element);
      else
            element->parent()->add(element);

      switch(et) {
            case Element::SLUR:
                  addLayoutFlags(LAYOUT_PLAY_EVENTS);
                  // fall through

            case Element::VOLTA:
            case Element::TRILL:
            case Element::PEDAL:
            case Element::TEXTLINE:
            case Element::HAIRPIN:
                  {
                  Spanner* spanner = static_cast<Spanner*>(element);
                  if (et == Element::TEXTLINE && spanner->anchor() == Spanner::ANCHOR_NOTE)
                        break;
                  foreach(SpannerSegment* ss, spanner->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;

            case Element::OTTAVA:
                  {
                  Ottava* o = static_cast<Ottava*>(element);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  o->staff()->updateOttava(o);
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  }
                  break;

            case Element::DYNAMIC:
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case Element::CLEF:
                  {
                  Clef* clef = static_cast<Clef*>(element);
                  if (!clef->generated())
                        updateNoteLines(clef->segment(), clef->track());
                  }
                  break;

            case Element::KEYSIG:
                  {
                  KeySig* ks = static_cast<KeySig*>(element);
                  Staff*  staff = element->staff();
                  KeySigEvent keySigEvent = ks->keySigEvent();
                  if (!ks->generated()) {
                        staff->setKey(ks->segment()->tick(), keySigEvent);
                        ks->insertIntoKeySigChain();
                        }
                  }
                  break;

            case Element::TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(element);
                  setTempo(tt->segment(), tt->tempo());
                  }
                  break;

            case Element::INSTRUMENT_CHANGE:
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  break;

            case Element::CHORD:
                  createPlayEvents(static_cast<Chord*>(element));
                  break;

            case Element::NOTE: {
                  Note* note = static_cast<Note*>(element);
                  updateAccidentals(note->chord()->segment()->measure(), element->staffIdx());
                  }
                  // fall through

            case Element::TREMOLO:
            case Element::ARTICULATION:
            case Element::ARPEGGIO:
                  {
                  Element* cr = element->parent();
                  if (cr->type() == Element::CHORD)
                         createPlayEvents(static_cast<Chord*>(cr));
                  }
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

#if 0
      if (MScore::debugMode) {
            qDebug("   Score(%p)::removeElement %p(%s) parent %p(%s)",
               this, element, element->name(), parent, parent ? parent->name() : "");
            }
#endif
      if (element->parent() && element->parent()->type() == Element::SEGMENT)
            static_cast<Segment*>(element->parent())->measure()->setDirty();

      // special for MEASURE, HBOX, VBOX
      // their parent is not static

      Element::ElementType et = element->type();
      if (et == Element::TREMOLO)
            setLayoutAll(true);

      else if (et == Element::MEASURE
         || (et == Element::HBOX && parent->type() != Element::VBOX)
         || et == Element::VBOX
         || et == Element::TBOX
         || et == Element::FBOX
            ) {
            remove(element);
            addLayoutFlags(LAYOUT_FIX_TICKS);
            setLayoutAll(true);
            return;
            }

      if (et == Element::BEAM)          // beam parent does not survive layout
            element->setParent(0);

      if (parent)
            parent->remove(element);
      else
            remove(element);

      switch(et) {
            case Element::SLUR:
                  addLayoutFlags(LAYOUT_PLAY_EVENTS);
                  // fall through

            case Element::VOLTA:
            case Element::TRILL:
            case Element::PEDAL:
            case Element::TEXTLINE:
            case Element::HAIRPIN:
                  {
                  Spanner* spanner = static_cast<Spanner*>(element);
                  foreach(SpannerSegment* ss, spanner->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
                  break;

            case Element::OTTAVA:
                  {
                  Ottava* o = static_cast<Ottava*>(element);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  Staff* s = o->staff();
                  s->pitchOffsets().remove(o->tick());
                  s->pitchOffsets().remove(o->tick2());
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  }
                  break;

            case Element::DYNAMIC:
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case Element::CHORD:
            case Element::REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(element);
                  if (cr->beam())
                        cr->beam()->remove(cr);
                  }
                  break;
            case Element::CLEF:
                  {
                  Clef* clef = static_cast<Clef*>(element);
                  if (!clef->generated())
                        updateNoteLines(clef->segment(), clef->track());
                  }
                  break;
            case Element::KEYSIG:
                  {
                  KeySig* ks    = static_cast<KeySig*>(element);
                  Staff*  staff = element->staff();
                  if (!ks->generated()) {
                        ks->removeFromKeySigChain();
                        staff->removeKey(ks->segment()->tick());
                        }
                  }
                  break;
            case Element::TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(element);
                  int tick = tt->segment()->tick();
                  tempomap()->delTempo(tick);
                  }
                  break;
            case Element::INSTRUMENT_CHANGE:
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  break;

            case Element::TREMOLO:
            case Element::ARTICULATION:
            case Element::ARPEGGIO:
                  {
                  Element* cr = element->parent();
                  if (cr->type() == Element::CHORD)
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
      while (mb && mb->type() != Element::MEASURE)
            mb = mb->next();

      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   firstMeasureMM
//---------------------------------------------------------

Measure* Score::firstMeasureMM() const
      {
      MeasureBase* mb = _measures.first();
      while (mb && mb->type() != Element::MEASURE)
            mb = mb->next();
      Measure* m = static_cast<Measure*>(mb);
      if (m && styleB(ST_createMultiMeasureRests) && m->hasMMRest())
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
         && m->type() == Element::MEASURE
         && styleB(ST_createMultiMeasureRests)
         && static_cast<Measure*>(m)->hasMMRest()) {
            return static_cast<Measure*>(m)->mmRest();
            }
      return m;
      }

//---------------------------------------------------------
//   measureIdx
//---------------------------------------------------------

int Score::measureIdx(MeasureBase* m) const
      {
      int idx = 0;
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb == m)
                  return idx;
            ++idx;
            }
      return -1;
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
      while (mb && mb->type() != Element::MEASURE)
            mb = mb->prev();
      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   lastMeasureMM
//---------------------------------------------------------

Measure* Score::lastMeasureMM() const
      {
      MeasureBase* mb = _measures.last();
      for (; mb; mb = mb->prev()) {
            if (mb->type() != Element::MEASURE)
                  continue;
            if (!styleB(ST_createMultiMeasureRests))
                  break;
            Measure* m = static_cast<Measure*>(mb);
            if (m->mmRestCount() < 0)
                  continue;
            if (m->hasMMRest())
                  mb = m->mmRest();
            break;
            }
      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(Segment::SegmentTypes segType) const
      {
      Measure* m = firstMeasure();
      return m ? m->first(segType) : 0;
      }

//---------------------------------------------------------
//   firstSegmentMM
//---------------------------------------------------------

Segment* Score::firstSegmentMM(Segment::SegmentTypes segType) const
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
      for(MeasureBase* m = first(); m; m = m->next())
            m->scanElements(data, func, all);
      foreach(Page* page, pages())
            page->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   customKeySigIdx
//    try to find custom key signature in table,
//    return index or -1 if not found
//---------------------------------------------------------

int Score::customKeySigIdx(KeySig* ks) const
      {
      int idx = 0;
      foreach(KeySig* k, customKeysigs) {
            if (*k == *ks)
                  return idx;
            ++idx;
            }
      qDebug("  not found");
      return -1;
      }

//---------------------------------------------------------
//   addCustomKeySig
//---------------------------------------------------------

int Score::addCustomKeySig(KeySig* ks)
      {
      customKeysigs.append(ks);
      int idx = customKeysigs.size() - 1;
      KeySigEvent k = ks->keySigEvent();
      k.setCustomType(idx);
      ks->setKeySigEvent(k);
      ks->setScore(this);
      return idx;
      }

//---------------------------------------------------------
//   customKeySig
//---------------------------------------------------------

KeySig* Score::customKeySig(int idx) const
      {
      return customKeysigs.value(idx);
      }

//---------------------------------------------------------
//   keySigFactory
//---------------------------------------------------------

KeySig* Score::keySigFactory(const KeySigEvent& e)
      {
      KeySig* ks;
      if (!e.isValid())
            return 0;
      if (e.custom()) {
            KeySig* cks = customKeysigs.value(e.customType());
            if (cks)
                  ks = new KeySig(*cks);
            else {
                  qDebug("Score::keySigFactory: invalid custom key index");
                  return 0;
                  }
            }
      else {
            ks = new KeySig(this);
            ks->setKeySigEvent(e);
            }
      return ks;
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

Text* Score::getText(int subtype)
      {
      MeasureBase* m = first();
      if (m && m->type() == Element::VBOX) {
            foreach(Element* e, *m->el()) {
                  if (e->type() == Element::TEXT && static_cast<Text*>(e)->textStyleType() == subtype)
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
//   metaTags
//---------------------------------------------------------

const QMap<QString, QString>& Score::metaTags() const
      {
      return _metaTags;
      }

QMap<QString, QString>& Score::metaTags()
      {
      return _metaTags;
      }

//---------------------------------------------------------
//   metaTag
//---------------------------------------------------------

Q_INVOKABLE QString Score::metaTag(const QString& s) const
      {
      if (_metaTags.contains(s))
            return _metaTags.value(s);
      return rootScore()->_metaTags.value(s);
      }

//---------------------------------------------------------
//   setMetaTag
//---------------------------------------------------------

Q_INVOKABLE void Score::setMetaTag(const QString& tag, const QString& val)
      {
      _metaTags.insert(tag, val);
      }

//---------------------------------------------------------
//   addStaffType
//    ownership of st move to score except if the buildin
//    flag is set
//---------------------------------------------------------

void Score::addStaffType(StaffType* st)
      {
      addStaffType(-1, st);
      }

void Score::addStaffType(int idx, StaffType* st)
      {
      // if the modified staff type is NOT replacing an existing type
      if (idx < 0 || idx >= _staffTypes.size()) {
            // store new pointer to pointer to type data
            StaffType** stp = new StaffType*;
            *stp = st;
            _staffTypes.append(stp);
            }
      // if the modified staff type IS replacing an existing type
      else {
            StaffType* oldStaffType = *(_staffTypes[idx]);
            // update the type of each score staff which uses the old type
            for(int staffIdx = 0; staffIdx < staves().size(); staffIdx++)
                  if(staff(staffIdx)->staffType() == oldStaffType)
                        staff(staffIdx)->setStaffType(st);
            // store the updated staff type
            *(_staffTypes[idx]) = st;
            // delete old staff type if not built-in
            if (!oldStaffType->builtin())
                  delete oldStaffType;
            }
      }

//---------------------------------------------------------
//   staffTypeIdx
//---------------------------------------------------------

int Score::staffTypeIdx(StaffType* st) const
      {
      for (int i = 0; i < _staffTypes.size(); ++i) {
            if ((*_staffTypes[i]) == st)
                  return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

StaffType* Score::staffType(int idx)
      {
      if (idx < 0 || idx >= _staffTypes.size())
            return 0;
      return *(_staffTypes[idx]);
      }

//---------------------------------------------------------
//   replaceStaffTypes
//---------------------------------------------------------

void Score::replaceStaffTypes(const QList<StaffType*>& tl)
      {
      Q_ASSERT(this == rootScore());
      for (int idx = 0; idx < tl.size(); idx++)
            addStaffType(idx, tl[idx]->clone());
      }

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void Score::addExcerpt(Score* score)
      {
      score->setParentScore(this);
      Excerpt* ex = new Excerpt(score);
      excerpts().append(ex);
      ex->setTitle(score->name());
      foreach(Staff* s, score->staves()) {
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
      foreach (Excerpt* ex, excerpts()) {
            if (ex->score() == score) {
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
//   updateNotes
///   calculate note lines and accidental
//---------------------------------------------------------

void Score::updateNotes()
      {
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  AccidentalState tversatz;      // state of already set accidentals for this measure
                  tversatz.init(staff(staffIdx)->keymap()->key(m->tick()));

                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        if (!(segment->segmentType() & (Segment::SegChordRest)))
                              continue;
                        m->layoutChords10(segment, staffIdx * VOICES, &tversatz);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdUpdateNotes
///   calculate note lines and accidental
//---------------------------------------------------------

void Score::cmdUpdateNotes()
      {
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
                  updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   cmdUpdateAccidentals
///   update accidentals upto next keySig change
//---------------------------------------------------------

void Score::cmdUpdateAccidentals(Measure* beginMeasure, int staffIdx)
      {
//      qDebug("cmdUpdateAccidentals m=%d for staff=%d",
//            beginMeasure->no(), staffIdx);
      Staff* st = staff(staffIdx);
      for (Measure* m = beginMeasure; m; m = m->nextMeasureMM()) {
            AccidentalState as;
            as.init(st->keymap()->key(m->tick()));

            for (Segment* s = m->first(); s; s = s->next()) {
                  if ((m != beginMeasure) &&
                        (s->segmentType() & (Segment::SegKeySig))) {
                        KeySig* ks = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                        if (ks && (!ks->generated())) {
                              // found new key signature
                              qDebug("leaving cmdUpdateAccidentals at m=%d",
                                    m->no());
                              return;
                              }
                        }
                  if (s->segmentType() & (Segment::SegChordRest))
                        m->updateAccidentals(s, staffIdx, &as);
                  }
            }
//      qDebug("leaving cmdUpdateAccidentals at end of score");
      }

//---------------------------------------------------------
//   updateAccidentals
//---------------------------------------------------------

void Score::updateAccidentals(Measure* m, int staffIdx)
      {
// qDebug("updateAccidentals measure %d staff %d", m->no(), staffIdx);
      Staff* st = staff(staffIdx);
      AccidentalState as;      // list of already set accidentals for this measure
      as.init(st->keymap()->key(m->tick()));

      for (Segment* segment = m->first(); segment; segment = segment->next()) {
            if (segment->segmentType() & (Segment::SegChordRest))
                  m->updateAccidentals(segment, staffIdx, &as);
            }
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

      int staffIdx = 0;
      foreach(Staff* st, score->staves()) {
            if (st->updateKeymap())
                  st->keymap()->clear();
            int track = staffIdx * VOICES;
            KeySig* key1 = 0;
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->generated())
                              continue;
                        if ((s->segmentType() == Segment::SegKeySig) && st->updateKeymap()) {
                              KeySig* ks = static_cast<KeySig*>(e);
                              int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                              ks->setOldSig(naturals);
                              st->setKey(s->tick(), ks->keySigEvent());
                              key1 = ks;
                              }
                        }
                  if (m->sectionBreak())
                        key1 = 0;
                  }
            st->setUpdateKeymap(false);
            ++staffIdx;
            }
      score->updateNotes();
      score->addLayoutFlags(LAYOUT_FIX_TICKS | LAYOUT_FIX_PITCH_VELO);
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
      _dirty = true;
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

bool Score::appendScore(Score* score)
      {
      TieMap  tieMap;

      MeasureBaseList* ml = &score->_measures;
      for (MeasureBase* mb = ml->first(); mb; mb = mb->next()) {
            MeasureBase* nmb;
            if (mb->type() == Element::MEASURE)
                  nmb = static_cast<Measure*>(mb)->cloneMeasure(this, &tieMap);
            else
                  nmb = mb->clone();
            nmb->setNext(0);
            nmb->setPrev(0);
            nmb->setScore(this);
            _measures.add(nmb);
            }
      fixTicks();
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
      int rstaff = s->rstaff();
      Staff* ns = new Staff(this, p, rstaff + 1);
      ns->setRstaff(rstaff + 1);

      undoInsertStaff(ns, staffIdx+1);

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->cmdAddStaves(staffIdx+1, staffIdx+2, false);
            if (m->hasMMRest())
                  m->mmRest()->cmdAddStaves(staffIdx+1, staffIdx+2, false);
            }

      Clef* clef = new Clef(this);
      clef->setClefType(ClefType::F);
      clef->setTrack((staffIdx+1) * VOICES);
      Segment* seg = firstMeasure()->getSegment(Segment::SegClef, 0);
      clef->setParent(seg);
      undoAddElement(clef);

      undoChangeKeySig(ns, 0, s->key(0));

      rebuildMidiMapping();
      _instrumentsChanged = true;
      doLayout();

      //
      // move notes
      //
      select(0, SELECT_SINGLE, 0);
      int strack = staffIdx * VOICES;
      int dtrack = (staffIdx + 1) * VOICES;

      for (Segment* s = firstSegment(Segment::SegChordRest); s; s = s->next1(Segment::SegChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(strack + voice));
                  if (cr == 0 || cr->type() == Element::REST)
                        continue;
                  Chord* c = static_cast<Chord*>(cr);
                  QList<Note*> removeNotes;
                  foreach(Note* note, c->notes()) {
                        if (note->pitch() >= splitPoint)
                              continue;
                        Chord* chord = static_cast<Chord*>(s->element(dtrack + voice));
                        if (chord && (chord->type() != Element::CHORD))
                              abort();
                        if (chord == 0) {
                              chord = new Chord(*c);
                              foreach(Note* note, chord->notes())
                                    delete note;
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
                  foreach(Note* note, removeNotes) {
                        undoRemoveElement(note);
                        if (note->chord()->notes().isEmpty())
                              undoRemoveElement(note->chord());
                        }
                  }
            }
      //
      // make sure that the timeline for dtrack
      // has no gaps
      //
      int ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(Segment::SegChordRest); s; s = s->next1(Segment::SegChordRest)) {
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
            for (Segment* s = m->first(Segment::SegChordRest); s; s = s->next1(Segment::SegChordRest)) {
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
//   cmdInsertPart
//    insert before staffIdx
//---------------------------------------------------------

void Score::cmdInsertPart(Part* part, int staffIdx)
      {
      undoInsertPart(part, staffIdx);

      int sidx = this->staffIdx(part);
      int eidx = sidx + part->nstaves();
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->cmdAddStaves(sidx, eidx, true);
            if (m->hasMMRest())
                  m->mmRest()->cmdAddStaves(sidx, eidx, true);
            }

      adjustBracketsIns(sidx, eidx);
      }

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Score::cmdRemovePart(Part* part)
      {
      int sidx   = staffIdx(part);
      int n      = part->nstaves();
      int eidx   = sidx + n;

      //
      //    adjust measures
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->cmdRemoveStaves(sidx, eidx);
            if (m->hasMMRest())
                  m->mmRest()->cmdRemoveStaves(sidx, eidx);
            }

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

void Score::insertStaff(Staff* staff, int idx)
      {
      _staves.insert(idx, staff);
      staff->part()->insertStaff(staff);
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
            if ((sidx >= staffIdx) && (eidx < (staffIdx + span)))
                  undoChangeBarLineSpan(staff, span, 0, (_staves[staffIdx + span -1]->lines()-1)*2);
            }
      }

//---------------------------------------------------------
//   adjustKeySigs
//---------------------------------------------------------

void Score::adjustKeySigs(int sidx, int eidx, KeyList km)
      {
      for (int staffIdx = sidx; staffIdx < eidx; ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            if(!staff->isDrumStaff()) {
                  for (auto i = km.begin(); i != km.end(); ++i) {
                        int tick = i->first;
                        Measure* measure = tick2measure(tick);
                        KeySigEvent oKey = i->second;
                        KeySigEvent nKey = oKey;
                        int diff = -staff->part()->instr()->transpose().chromatic;
                        if (diff != 0 && !styleB(ST_concertPitch))
                              nKey.setAccidentalType(transposeKey(nKey.accidentalType(), diff));
                        (*(staff->keymap()))[tick] = nKey;
                        KeySig* keysig = new KeySig(this);
                        keysig->setTrack(staffIdx * VOICES);
                        keysig->setKeySigEvent(nKey);
                        Segment* s = measure->getSegment(keysig, tick);
                        s->add(keysig);
                        }
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
            if (s->staffIdx() == staffIdx)
                  sl.append(s);
            }
      for (auto i : sl)
            undoRemoveElement(i);

      undoRemoveStaff(s, staffIdx);

      // remove linked staff and measures in linked staves in excerps
      // should be done earlier for the main staff
      if (s->linkedStaves()) {
            for(Staff* staff : s->linkedStaves()->staves()) {
                 Score* lscore = staff->score();
                 if(lscore != this) {
                       int lstaffIdx = lscore->staffIdx(staff);
                       int pIndex = lscore->staffIdx(staff->part());
                       //adjustBracketsDel(lstaffIdx, lstaffIdx+1);
                       for (Measure* m = lscore->firstMeasure(); m; m = m->nextMeasure()) {
                              m->cmdRemoveStaves(lstaffIdx, lstaffIdx + 1);
                              if (m->hasMMRest())
                                    m->mmRest()->cmdRemoveStaves(lstaffIdx, lstaffIdx + 1);
                              }
                        undoRemoveStaff(staff, lstaffIdx);
                        if (staff->part()->nstaves() == 0)
                              undoRemovePart(staff->part(), pIndex);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
      {
      _staves.removeAll(staff);
      staff->part()->removeStaff(staff);
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
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag, bool useDoubleSharpsFlats)
      {
      undo(new ChangeConcertPitch(this, flag));

      foreach(Staff* staff, _staves) {
            if (staff->staffType()->group() == PERCUSSION_STAFF_GROUP)
                  continue;
            Instrument* instr = staff->part()->instr();
            Interval interval = instr->transpose();
            if (interval.isZero())
                  continue;
            if (!flag)
                  interval.flip();
            cmdTransposeStaff(staff->idx(), interval, useDoubleSharpsFlats);
            }
      for (Segment* s = firstMeasure()->first(Segment::SegClef); s; s = s->next1(Segment::SegClef)) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Clef* clef = static_cast<Clef*>(s->element(staffIdx * VOICES));
                  if (!clef)
                        continue;
                  clef->setClefType(flag ? clef->concertClef() : clef->transposingClef());
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

void Score::padToggle(int n)
      {
      switch (n) {
            case PAD_NOTE00:
                  _is.setDuration(TDuration::V_LONG);
                  break;
            case PAD_NOTE0:
                  _is.setDuration(TDuration::V_BREVE);
                  break;
            case PAD_NOTE1:
                  _is.setDuration(TDuration::V_WHOLE);
                  break;
            case PAD_NOTE2:
                  _is.setDuration(TDuration::V_HALF);
                  break;
            case PAD_NOTE4:
                  _is.setDuration(TDuration::V_QUARTER);
                  break;
            case PAD_NOTE8:
                  _is.setDuration(TDuration::V_EIGHT);
                  break;
            case PAD_NOTE16:
                  _is.setDuration(TDuration::V_16TH);
                  break;
            case PAD_NOTE32:
                  _is.setDuration(TDuration::V_32ND);
                  break;
            case PAD_NOTE64:
                  _is.setDuration(TDuration::V_64TH);
                  break;
            case PAD_NOTE128:
                  _is.setDuration(TDuration::V_128TH);
                  break;
            case PAD_REST:
                  _is.rest = !_is.rest;
                  break;
            case PAD_DOT:
                  if (_is.duration().dots() == 1)
                        _is.setDots(0);
                  else
                        _is.setDots(1);
                  break;
            case PAD_DOTDOT:
                  if (_is.duration().dots() == 2)
                        _is.setDots(0);
                  else
                        _is.setDots(2);
                  break;
            }
      if (n >= PAD_NOTE00 && n <= PAD_NOTE128) {
            _is.setDots(0);
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode())
                  _is.rest = false;
            }

      if (noteEntryMode() || !selection().isSingle()) {
            return;
            }

      //do not allow to add a dot on a full measure rest
      Element* e = selection().element();
      if (e && e->type() == Element::REST) {
            Rest* r = static_cast<Rest*>(e);
            TDuration d = r->durationType();
            if (d.type() == TDuration::V_MEASURE) {
                  _is.setDots(0);
                  // return;
                  }
            }

      Element* el = selection().element();
      if (el->type() == Element::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      if (cr->type() == Element::CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            undo(new ChangeDurationType(cr, _is.duration()));
            undo(new ChangeDuration(cr, _is.duration().fraction()));
            }
      else
            changeCRlen(cr, _is.duration());
      }

//---------------------------------------------------------
//   setInputState
//---------------------------------------------------------

void Score::setInputState(Element* e)
      {
// qDebug("setInputState %s", e ? e->name() : "--");

      if (e == 0)
            return;
      if (e && e->type() == Element::CHORD)
            e = static_cast<Chord*>(e)->upNote();

      _is.setDrumNote(-1);
//      _is.setDrumset(0);
      if (e->type() == Element::NOTE) {
            Note* note    = static_cast<Note*>(e);
            Chord* chord  = note->chord();
            _is.setDuration(chord->durationType());
            _is.rest      = false;
            _is.setTrack(note->track());
            _is.noteType  = note->noteType();
            _is.beamMode  = chord->beamMode();
            }
      else if (e->type() == Element::REST) {
            Rest* rest   = static_cast<Rest*>(e);
            if (rest->durationType().type() == TDuration::V_MEASURE)
                  _is.setDuration(TDuration::V_QUARTER);
            else
                  _is.setDuration(rest->durationType());
            _is.rest     = true;
            _is.setTrack(rest->track());
            _is.beamMode = rest->beamMode();
            _is.noteType = NOTE_NORMAL;
            }
      else {
/*            _is.rest     = false;
            _is.setDots(0);
            _is.setDuration(TDuration::V_INVALID);
            _is.noteType = NOTE_INVALID;
            _is.beamMode = BEAM_INVALID;
            _is.noteType = NOTE_NORMAL;
*/
            }
      if (e->type() == Element::NOTE || e->type() == Element::REST) {
            const Instrument* instr   = e->staff()->part()->instr();
            if (instr->useDrumset()) {
                  if (e->type() == Element::NOTE)
                        _is.setDrumNote(static_cast<Note*>(e)->pitch());
                  else
                        _is.setDrumNote(-1);
//                  _is.setDrumset(instr->drumset());
                  }
            }
//      mscore->updateInputState(this);
      }

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(Element* el)
      {
      refresh |= el->abbox();
      _selection.remove(el);
      }

//---------------------------------------------------------
//   select
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* e, SelectType type, int staffIdx)
      {
      if (e && (e->type() == Element::NOTE || e->type() == Element::REST)) {
            Element* ee = e;
            if (ee->type() == Element::NOTE)
                  ee = ee->parent();
            setPlayPos(static_cast<ChordRest*>(ee)->segment()->tick());
            }
      if (MScore::debugMode)
            qDebug("select element <%s> type %d(state %d) staff %d",
               e ? e->name() : "", type, selection().state(), e ? e->staffIdx() : -1);

      SelState selState = _selection.state();

      if (type == SELECT_SINGLE) {
            deselectAll();
            if (e == 0) {
                  selState = SEL_NONE;
                  _updateAll = true;
                  }
            else {
                  if (e->type() == Element::MEASURE) {
                        select(e, SELECT_RANGE, staffIdx);
                        return;
                        }
                  refresh |= e->abbox();
                  _selection.add(e);
                  _is.setTrack(e->track());
                  selState = SEL_LIST;
                  if (e->type() == Element::NOTE)
                        e = e->parent();
                  if (e->type() == Element::REST || e->type() == Element::CHORD)
                        _is.setSegment(static_cast<ChordRest*>(e)->segment());
                  }
            _selection.setActiveSegment(0);
            _selection.setActiveTrack(0);
            }
      else if (type == SELECT_ADD) {
            if (e->type() == Element::MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  // int etick = tick + m->ticks();
                  if (_selection.state() == SEL_NONE) {
                        _selection.setStartSegment(m->tick2segment(tick));
                        // _selection.setEndSegment(m == lastMeasure() ? 0 : tick2segment(etick));
                        _selection.setEndSegment(m == lastMeasure() ? 0 : m->last());
                        }
                  else {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  _updateAll = true;
                  selState = SEL_RANGE;
                  _selection.setStaffStart(0);
                  _selection.setStaffEnd(nstaves());
                  _selection.updateSelectedElements();
                  }
            else {
                  if (_selection.state() == SEL_RANGE) {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  else {
                        refresh |= e->abbox();
                        if (_selection.elements().contains(e))
                              _selection.remove(e);
                        else {
                            _selection.add(e);
                            selState = SEL_LIST;
                            }
                        }
                  }
            }
      else if (type == SELECT_RANGE) {
            bool activeIsFirst = false;
            int activeTrack = e->track();
            if (e->type() == Element::MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->ticks();
                  activeTrack = staffIdx * VOICES;
                  if (_selection.state() == SEL_NONE) {
                        _selection.setStaffStart(staffIdx);
                        _selection.setStaffEnd(staffIdx + 1);
                        _selection.setStartSegment(m->tick2segment(tick));
                        // _selection.setEndSegment(m == lastMeasure() ? 0 : tick2segment(etick));
                        _selection.setEndSegment(m == lastMeasure() ? 0 : m->last());
                        }
                  else if (_selection.state() == SEL_RANGE) {
                        if (staffIdx < _selection.staffStart())
                              _selection.setStaffStart(staffIdx);
                        else if (staffIdx >= _selection.staffEnd())
                              _selection.setStaffEnd(staffIdx + 1);
                        if (tick < _selection.tickStart()) {
                              _selection.setStartSegment(m->tick2segment(tick));
                              activeIsFirst = true;
                              }
                        else if (etick >= _selection.tickEnd())
                              //_selection.setEndSegment(m == lastMeasure() ? 0 : tick2segment(etick));
                              _selection.setEndSegment(m == lastMeasure() ? 0 : m->last());
                        else {
                              if (_selection.activeSegment() == _selection.startSegment()) {
                                    _selection.setStartSegment(m->tick2segment(tick));
                                    activeIsFirst = true;
                                    }
                              else
                                    //_selection.setEndSegment(m == lastMeasure() ? 0 : tick2segment(etick));
                                    _selection.setEndSegment(m == lastMeasure() ? 0 : m->last());
                              }
                        }
                  else if (_selection.isSingle()) {
                        Segment* seg = 0;
                        Element* oe  = _selection.element();
                        bool reverse = false;
                        int ticks    = 0;
                        if (oe->isChordRest())
                              ticks = static_cast<ChordRest*>(oe)->actualTicks();
                        int oetick = 0;
                        if (oe->parent()->type() == Element::SEGMENT)
                              oetick = static_cast<Segment*>(oe->parent())->tick();
                        if (tick < oetick)
                              seg = m->first();
                        else if (etick >= oetick + ticks) {
                              seg = m->last();
                              reverse = true;
                              }
                        int track = staffIdx * VOICES;
                        Element* el = 0;
                        // find first or last chord/rest in measure
                        for (;;) {
                              el = seg->element(track);
                              if (el && el->isChordRest())
                                    break;
                              if (reverse)
                                    seg = seg->prev1MM();
                              else
                                    seg = seg->next1MM();
                              if (!seg)
                                    break;
                              }
                        if (el)
                              select(el, SELECT_RANGE, staffIdx);
                        return;
                        }
                  else {
                        qDebug("SELECT_RANGE: measure: sel state %d", _selection.state());
                        return;
                        }
                  }
            else if (e->type() == Element::NOTE || e->isChordRest()) {
                  if (e->type() == Element::NOTE)
                        e = e->parent();
                  ChordRest* cr = static_cast<ChordRest*>(e);

                  if (_selection.state() == SEL_NONE) {
                        _selection.setStaffStart(e->staffIdx());
                        _selection.setStaffEnd(_selection.staffStart() + 1);
                        _selection.setStartSegment(cr->segment());
                        activeTrack = cr->track();
                        _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                        }
                  else if (_selection.isSingle()) {
                        Element* oe = _selection.element();
                        if (oe && (oe->type() == Element::NOTE || oe->type() == Element::REST)) {
                              if (oe->type() == Element::NOTE)
                                    oe = oe->parent();
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              _selection.setStaffStart(oe->staffIdx());
                              _selection.setStaffEnd(_selection.staffStart() + 1);
                              _selection.setStartSegment(ocr->segment());
                              _selection.setEndSegment(tick2segment(ocr->tick() + ocr->actualTicks()));
                              if (!_selection.endSegment())
                                    _selection.setEndSegment(ocr->segment()->next());

                              staffIdx = cr->staffIdx();
                              int tick = cr->tick();
                              if (staffIdx < _selection.staffStart())
                                    _selection.setStaffStart(staffIdx);
                              else if (staffIdx >= _selection.staffEnd())
                                    _selection.setStaffEnd(staffIdx + 1);
                              if (tick < _selection.tickStart()) {
                                    _selection.setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else if (tick >= _selection.tickEnd())
                                    _selection.setEndSegment(tick2segment(cr->tick() + cr->actualTicks()));
                              else {
                                    if (_selection.activeSegment() == _selection.startSegment()) {
                                          _selection.setStartSegment(cr->segment());
                                          activeIsFirst = true;
                                          }
                                    else
                                          _selection.setEndSegment(tick2segment(cr->tick() + cr->actualTicks()));
                                    }
                              }
                        else {
                              select(e, SELECT_SINGLE, 0);
                              return;
                              }
                        }
                  else if (_selection.state() == SEL_RANGE) {
                        staffIdx = cr->staffIdx();
                        int tick = cr->tick();
                        if (staffIdx < _selection.staffStart())
                              _selection.setStaffStart(staffIdx);
                        else if (staffIdx >= _selection.staffEnd())
                              _selection.setStaffEnd(staffIdx + 1);
                        if (tick < _selection.tickStart()) {
                              if (_selection.activeSegment() == _selection.endSegment())
                                    _selection.setEndSegment(_selection.startSegment());
                              _selection.setStartSegment(cr->segment());
                              activeIsFirst = true;
                              }
                        else if (_selection.endSegment() && tick >= _selection.tickEnd()) {
                              if (_selection.activeSegment() == _selection.startSegment())
                                    _selection.setStartSegment(_selection.endSegment());
                              Segment* s = cr->segment()->nextCR(cr->track());
                              _selection.setEndSegment(s);
                              }
                        else {
                              if (_selection.activeSegment() == _selection.startSegment()) {
                                    _selection.setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else {
                                    _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                                    }
                              }
                        }
                  else {
                        qDebug("sel state %d", _selection.state());
                        }
                  selState = SEL_RANGE;
                  if (!_selection.endSegment())
                        _selection.setEndSegment(cr->segment()->nextCR());
                  if (!_selection.startSegment())
                        _selection.setStartSegment(cr->segment());
                  }
            else {
                  select(e, SELECT_SINGLE, staffIdx);
                  return;
                  }

            if (activeIsFirst)
                  _selection.setActiveSegment(_selection.startSegment());
            else
                  _selection.setActiveSegment(_selection.endSegment());

            _selection.setActiveTrack(activeTrack);

            selState = SEL_RANGE;
            _selection.updateSelectedElements();
            }
      _selection.setState(selState);
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, SELECT_SINGLE, 0);
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
                  e->itemDiscovered = 0;
                  if (frr.contains(e->abbox())) {
                        if (e->type() != Element::MEASURE && e->selectable())
                              select(e, SELECT_ADD, 0);
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

      if (_selection.elements().isEmpty()) {
            _selection.setState(SEL_NONE);
            _updateAll = true;
            return;
            }
      _selection.setState(SEL_LIST);

      foreach(const Element* e, _selection.elements()) {
            if (e->type() != Element::NOTE && e->type() != Element::REST)
                  continue;
            ++noteRestCount;
            if (e->type() == Element::NOTE)
                  e = e->parent();
            Segment* seg = static_cast<const ChordRest*>(e)->segment();
            if ((startSegment == 0) || (*seg < *startSegment))
                  startSegment = seg;
            if ((endSegment == 0) || (*seg > *endSegment)) {
                  endSegment = seg;
                  }
            int idx = e->staffIdx();
            if (idx < startStaff)
                  startStaff = idx;
            if (idx > endStaff)
                  endStaff = idx;
            }
      if (noteRestCount > 0) {
            endSegment = endSegment->nextCR(endStaff * VOICES);
            _selection.setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (_selection.state() != SEL_RANGE)
                  _selection.setState(SEL_RANGE);
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
      Segment* seg     = measure->findSegment(Segment::SegChordRest, tick);
      if (seg == 0) {
            qDebug("no segment found for lyrics<%s> at tick %d",
               qPrintable(txt), tick);
            return;
            }
      int track = staffIdx * VOICES;
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr) {
            Lyrics* l = new Lyrics(this);
            l->setText(txt);
            l->setTrack(track);
            cr->add(l);
            }
      else {
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
      return pageFormat()->size().width() * MScore::DPI;
      }

//---------------------------------------------------------
//   loHeight
//---------------------------------------------------------

qreal Score::loHeight() const
      {
      return pageFormat()->size().height() * MScore::DPI;
      }

//---------------------------------------------------------
//   cmdSelectAll
//---------------------------------------------------------

void Score::cmdSelectAll()
      {
      if (_measures.size() == 0)
            return;
      _selection.setState(SEL_RANGE);
      Segment* s1 = firstMeasureMM()->first();
      Segment* s2 = lastMeasureMM()->last();
      _selection.setRange(s1, s2, 0, nstaves());
      _selection.updateSelectedElements();
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
      while (sm && sm->type() != Element::MEASURE)
            sm = sm->next();
      while (em && em->type() != Element::MEASURE)
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
//   setLayoutMode
//---------------------------------------------------------

void Score::setLayoutMode(LayoutMode lm)
      {
      _layoutMode = lm;
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
      foreach(const Excerpt* ex, root->excerpts())
            scores.append(ex->score());
      return scores;
      }

//---------------------------------------------------------
//   switchLayer
//---------------------------------------------------------

bool Score::switchLayer(const QString& s)
      {
      int layerIdx = 0;
      foreach(const Layer& l, layer()) {
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
            a.pan    = 60;
            t->channel.append(a);
            }
      Part* part = new Part(this);
      part->initFromInstrTemplate(t);
      int n = nstaves();
      for (int i = 0; i < t->nstaves(); ++i) {
            Staff* staff = new Staff(this, part, i);
            staff->setLines(t->staffLines[i]);
            staff->setSmall(t->smallStaff[i]);
            if (i == 0) {
                  staff->setBracket(0, t->bracket[0]);
                  staff->setBracketSpan(0, t->nstaves());
                  }
            undoInsertStaff(staff, n + i);
            }

      part->staves()->front()->setBarLineSpan(part->nstaves());
      cmdInsertPart(part, n);
      fixTicks();
      rebuildMidiMapping();
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n)
      {
      for (int i = 0; i < n; ++i)
            insertMeasure(Element::MEASURE, 0, false);
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

void Score::addText(const QString& type, const QString& txt)
      {
      MeasureBase* measure = first();
      if (measure == 0 || measure->type() != Element::VBOX) {
            insertMeasure(Element::VBOX, measure);
            measure = first();
            }
      Text* text = new Text(this);
      if (type == "title")
            text->setTextStyleType(TEXT_STYLE_TITLE);
      else if (type == "subtitle")
            text->setTextStyleType(TEXT_STYLE_SUBTITLE);
      text->setParent(measure);
      text->setText(txt);
      undoAddElement(text);
      }

//---------------------------------------------------------
//   newCursor
//---------------------------------------------------------

Cursor* Score::newCursor()
      {
      return new Cursor(this);
      }

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
//   findSpanner
//---------------------------------------------------------

Spanner* Score::findSpanner(int id) const
      {
      for (auto i = _spanner.crbegin(); i != _spanner.crend(); ++i) {
            if (i->second->id() == id)
                  return i->second;
            }
      return 0;
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

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Score::insertTime(int tick, int len)
      {
      if (len == 0)
            return;

      for (auto i : _spanner.map()) {
            Spanner* s = i.second;
            if (s->tick2() < tick)
                  continue;
            if (len > 0) {
                  if (tick > s->tick() && tick < s->tick2()) {
                        //
                        //  case a:
                        //  +----spanner--------+
                        //    +---add---
                        //
                        undoChangeProperty(s, P_SPANNER_TICK2, s->tick2() + len);
                       }
                  else if (tick <= s->tick()) {
                        //
                        //  case b:
                        //       +----spanner--------
                        //  +---add---
                        // and
                        //            +----spanner--------
                        //  +---add---+
                        undoChangeProperty(s, P_SPANNER_TICK, s->tick() + len);
                        undoChangeProperty(s, P_SPANNER_TICK2, s->tick2() + len);
                        }
                  }
            else {
                  int tick2 = tick - len;
                  if (s->tick() >= tick2) {
                        //
                        //  case A:
                        //  +----remove---+ +---spanner---+
                        //
                        int t = s->tick() + len;
                        if (t < 0)
                              t = 0;
                        undoChangeProperty(s, P_SPANNER_TICK, t);
                        undoChangeProperty(s, P_SPANNER_TICK2, s->tick2() + len);
                        }
                  else if ((s->tick() < tick) && (s->tick2() > tick2)) {
                        //
                        //  case B:
                        //  +----spanner--------+
                        //    +---remove---+
                        //
                        int t2 = s->tick2() + len;
                        if (t2 > s->tick())
                              undoChangeProperty(s, P_SPANNER_TICK2, t2);
                        }
                  else if (s->tick() >= tick && s->tick2() < tick2) {
                        //
                        //  case C:
                        //    +---spanner---+
                        //  +----remove--------+
                        //
                        undoRemoveElement(s);
                        }
                  else if (s->tick() > tick && s->tick2() > tick2) {
                        //
                        //  case D:
                        //       +----spanner--------+
                        //  +---remove---+
                        //
                        int d1 = s->tick() - tick;
                        int d2 = tick2 - s->tick();
                        int len = s->tickLen() - d2;
                        if (len == 0)
                             undoRemoveElement(s);
                        else {
                              undoChangeProperty(s, P_SPANNER_TICK, s->tick() - d1);
                              undoChangeProperty(s, P_SPANNER_TICK2, s->tick2() - (tick2-tick));
                              }
                        }
                  }
            }
      }


//---------------------------------------------------------
//   set Loop In position
//---------------------------------------------------------

void Score::setLoopInTick(int tick)
      {
//      qDebug ("setLoopInTick : tick = %d", tick);
      if(!lastMeasure())
            return;
      if ((tick < 0) || (tick > lastMeasure()->endTick()-1))
            tick = 0;
      _loopInTick = tick;
      }

//---------------------------------------------------------
//   set Loop Out position
//---------------------------------------------------------

void Score::setLoopOutTick(int tick)
      {
//      qDebug ("setLoopOutTick : tick = %d", tick);
      if(!lastMeasure())
            return;
      int lastTick = lastMeasure()->endTick()-1;
      if ((tick > lastTick) || (tick < 0))
            _loopOutTick = lastTick;
      else
            _loopOutTick = tick;
      }

}


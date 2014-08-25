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

namespace Ms {

Score* gscore;                 ///< system score, used for palettes etc.
QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

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
      if (nb->type() == Element::Type::HBOX || nb->type() == Element::Type::VBOX
         || nb->type() == Element::Type::TBOX || nb->type() == Element::Type::FBOX)
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
      _playlistDirty          = false;
      _autosaveDirty          = false;
      _dirty                  = false;
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
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
   : QObject(0), _is(this), _selection(this), _selectionFilter(this)
      {
      _parentScore = 0;
      init();
      _tempomap = new TempoMap;
      _sigmap   = new TimeSigMap();
      _style    = *(MScore::defaultStyle());
      accInfo = tr("No selection");
      }

Score::Score(const MStyle* s)
   : _is(this), _selection(this), _selectionFilter(this)
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
//    _dirty
//

Score::Score(Score* parent)
   : _is(this), _selection(this), _selectionFilter(this)
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
   : _is(this), _selection(this), _selectionFilter(this)
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

            if (!parentScore()) {
                  //
                  //  implement section break rest
                  //
                  if (m->sectionBreak())
                        setPause(m->tick() + m->ticks(), m->pause());

                  //
                  // implement fermata as a tempo change
                  //

                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (s->segmentType() == Segment::Type::Breath)
                              setPause(s->tick(), .1);
                        else if (s->segmentType() == Segment::Type::TimeSig) {
                              for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                                    TimeSig* ts = static_cast<TimeSig*>(s->element(staffIdx * VOICES));
                                    if (ts)
                                          staff(staffIdx)->addTimeSig(ts);
                                    }
                              }
                        else if (s->segmentType() == Segment::Type::ChordRest) {
                              foreach(Element* e, s->annotations()) {
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
//   setDirty
//---------------------------------------------------------

void Score::setDirty(bool val)
      {
      Score* s = rootScore();
      if (s->dirty() != val) {
            s->_dirty = val;
            s->_playlistDirty = true;
            }
      if (s->dirty()) {
            s->_playlistDirty = true;
            s->_autosaveDirty = true;
            }
      }

//---------------------------------------------------------
//   dirty
//---------------------------------------------------------

bool Score::dirty() const
      {
      return rootScore()->_dirty;
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
      int port    = 0;
      int channel = 0;
      int idx     = 0;
      foreach(Part* part, _parts) {
            InstrumentList* il = part->instrList();
            for (auto i = il->begin(); i != il->end(); ++i) {
                  DrumsetKind drum = i->second.useDrumset();
                  for (int k = 0; k < i->second.channel().size(); ++k) {
                        Channel* a = &(i->second.channel(k));
                        MidiMapping mm;
                        if (drum != DrumsetKind::NONE) {
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
            if (st->invisible() || !st->part()->show())
                  continue;
            qreal sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            SysStaff* nstaff = 0;

            // find next visible staff
            for (int i = pos->staffIdx + 1; i < nstaves(); ++i) {
                  Staff* st = staff(i);
                  if (st->invisible() || !st->part()->show())
                        continue;
                  nstaff = system->staff(i);
                  break;
                  }

            if (nstaff) {
                  qreal s1y2 = ss->bbox().y() + ss->bbox().height();
                  sy2        = s1y2 + (nstaff->bbox().y() - s1y2) * .5;
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
      _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, newValue / (MScore::DPI * SPATIUM20));
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
#if 0
      if (MScore::debugMode) {
            qDebug("   Score(%p)::addElement %p(%s) parent %p(%s)",
               this, element, element->name(), element->parent(),
               element->parent() ? element->parent()->name() : "");
            }
#endif

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

            case Element::Type::INSTRUMENT_CHANGE:
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  break;

            case Element::Type::CHORD:
                  createPlayEvents(static_cast<Chord*>(element));
                  break;

            case Element::Type::NOTE: {
                  Note* note = static_cast<Note*>(element);
                  note->chord()->segment()->measure()->cmdUpdateNotes(element->staffIdx());
                  }
                  // fall through

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
            case Element::Type::INSTRUMENT_CHANGE:
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
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

#if 0
//---------------------------------------------------------
//   measureIdx
//---------------------------------------------------------

int Score::measureIdx(MeasureBase* m) const
      {
      int idx = 0;
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->nextMeasureMM()) {
            if (mb == m)
                  return idx;
            ++idx;
            }
      return -1;
      }
#endif

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
      Measure* m1 = m->mmRest1();
      if (m1)
           return m1;
      return m;
      }

//---------------------------------------------------------
//   firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(Segment::Type segType) const
      {
      Measure* m = firstMeasure();
      Segment* seg = m ? m->first(segType) : 0;
#ifdef SCRIPT_INTERFACE
      // if called from QML/JS, tell QML engine not to garbage collect this object
      if (seg)
            QQmlEngine::setObjectOwnership(seg, QQmlEngine::CppOwnership);
#endif
      return seg;
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
      for (MeasureBase* m = first(); m; m = m->next())
            m->scanElements(data, func, all);
      for (Page* page : pages())
            page->scanElements(data, func, all);
      }

void Score::scanElementsInRange(void* data, void (*func)(void*, Element*), bool all)
      {
      Segment* startSeg = _selection.startSegment();
      for (Segment* s = startSeg; s && s!=_selection.endSegment(); s = s->next1MM()) {
            s->scanElements(data,func,all);
            }
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

Text* Score::getText(TextStyleType subtype)
      {
      MeasureBase* m = first();
      if (m && m->type() == Element::Type::VBOX) {
            foreach(Element* e, *m->el()) {
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
///   recompute note lines and accidental
///   not undoable add/remove
//---------------------------------------------------------

void Score::updateNotes()
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
                  m->updateNotes(staffIdx);
            }
      }

//---------------------------------------------------------
//   cmdUpdateNotes
///   recompute note lines and accidental
///   undoable add/remove
//---------------------------------------------------------

void Score::cmdUpdateNotes()
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
                  m->cmdUpdateNotes(staffIdx);
            }
      }

//---------------------------------------------------------
//   cmdUpdateAccidentals
///   update accidentals upto next keySig change
//---------------------------------------------------------

void Score::cmdUpdateAccidentals(Measure* beginMeasure, int staffIdx)
      {
      for (Measure* m = beginMeasure; m; m = m->nextMeasureMM()) {
            m->cmdUpdateNotes(staffIdx);
            if (m == beginMeasure)
                  continue;
            for (Segment* s = m->first(Segment::Type::KeySig); s; s = s->next(Segment::Type::KeySig)) {
                  KeySig* ks = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                  if (ks && (!ks->generated()))
                        return;
                  }
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

      score->updateNotes();
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
      undoInsertStaff(ns, staffIdx+1, false);

      Clef* clef = new Clef(this);
      clef->setClefType(ClefType::F);
      clef->setTrack((staffIdx+1) * VOICES);
      Segment* seg = firstMeasure()->getSegment(Segment::Type::Clef, 0);
      clef->setParent(seg);
      undoAddElement(clef);

      undoChangeKeySig(ns, 0, s->key(0));

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
                  Chord* c = static_cast<Chord*>(s->element(strack + voice));
                  if (c == 0 || c->type() != Element::Type::CHORD)
                        continue;
                  QList<Note*> removeNotes;
                  foreach(Note* note, c->notes()) {
                        if (note->pitch() >= splitPoint)
                              continue;
                        Chord* chord = static_cast<Chord*>(s->element(dtrack + voice));
                        Q_ASSERT(!chord || (chord->type() == Element::Type::CHORD));
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
                  Key oKey = i->second;
                  Key nKey = oKey;
                  int diff = -staff->part()->instr()->transpose().chromatic;
                  if (diff != 0 && !styleB(StyleIdx::concertPitch))
                        nKey = transposeKey(nKey, diff);
                  staff->setKey(tick, nKey);
                  KeySig* keysig = new KeySig(this);
                  keysig->setTrack(staffIdx * VOICES);
                  keysig->setKey(nKey);
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
            if (s->staffIdx() == staffIdx && (staffIdx != 0 || s->type() != Element::Type::VOLTA))
                  sl.append(s);
            }
      for (auto i : sl) {
            i->undoUnlink();
            undo(new RemoveElement(i));
            }

      undoRemoveStaff(s);

      // remove linked staff and measures in linked staves in excerps
      // should be done earlier for the main staff
      Staff* s2 = 0;
      if (s->linkedStaves()) {
            for (Staff* staff : s->linkedStaves()->staves()) {
                  if (staff != s)
                        s2 = staff;
                 Score* lscore = staff->score();
                 if (lscore != this) {
                        undoRemoveStaff(staff);
                        if (staff->part()->nstaves() == 0) {
                              int pIndex    = lscore->staffIdx(staff->part());
                              undoRemovePart(staff->part(), pIndex);
                              }
                        }
                  }
            s->score()->undo(new UnlinkStaff(s2, s));
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
            Interval interval = staff->part()->instr()->transpose();
            if (interval.isZero())
                  continue;
            if (!flag)
                  interval.flip();

            int staffIdx   = staff->idx();
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;

            transposeKeys(staffIdx, staffIdx+1, 0, lastSegment()->tick(), interval);

            for (Segment* segment = firstSegment(Segment::Type::ChordRest); segment; segment = segment->next1(Segment::Type::ChordRest)) {
                  for (Element* e : segment->annotations()) {
                        if ((e->type() != Element::Type::HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                              continue;
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  }
            }
      cmdUpdateNotes();
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
                  if (_is.duration().dots() == 1)
                        _is.setDots(0);
                  else
                        _is.setDots(1);
                  break;
            case Pad::DOTDOT:
                  if (_is.duration().dots() == 2)
                        _is.setDots(0);
                  else
                        _is.setDots(2);
                  break;
            }
      if (n >= Pad::NOTE00 && n <= Pad::NOTE128) {
            _is.setDots(0);
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode())
                  _is.setRest(false);
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
                  // return;
                  }
            }

      Element* el = selection().element();
      if (el->type() == Element::Type::NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      if (cr->type() == Element::Type::CHORD && (static_cast<Chord*>(cr)->noteType() != NoteType::NORMAL)) {
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
      if (e && (e->type() == Element::Type::NOTE || e->type() == Element::Type::REST)) {
            Element* ee = e;
            if (ee->type() == Element::Type::NOTE)
                  ee = ee->parent();
            setPlayPos(static_cast<ChordRest*>(ee)->segment()->tick());
            }
      if (MScore::debugMode)
            qDebug("select element <%s> type %hhd(state %hhd) staff %d",
               e ? e->name() : "", type, selection().state(), e ? e->staffIdx() : -1);

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
            if (e->type() == Element::Type::REST || e->type() == Element::Type::CHORD) {
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
                        if (tick < oetick) {
                              _selection.setStartSegment(m->tick2segment(tick));
                              if (etick >= oetick)
                                    _selection.setEndSegment(m->last());
                              else
                                    _selection.setEndSegment(cr->nextSegmentAfterCR(Segment::Type::ChordRest
                                                                                    | Segment::Type::EndBarLine
                                                                                    | Segment::Type::Clef));

                              }
                        else {
                              _selection.setStartSegment(cr->segment());
                              _selection.setEndSegment(m->last());
                              }

                        _selection.setStaffStart(staffIdx);
                        _selection.setStaffEnd(staffIdx + 1);
                        if (_selection.staffStart() > cr->staffIdx())
                              _selection.setStaffStart(cr->staffIdx());
                        else if (cr->staffIdx() >= _selection.staffEnd())
                              _selection.setStaffEnd(cr->staffIdx() + 1);
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
                  qDebug("SELECT_RANGE: measure: sel state %hhd", _selection.state());
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

                        Segment* endSeg = tick2segment(ocr->segment()->tick() + ocr->actualTicks());
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
                  qDebug("sel state %hhd", _selection.state());
                  return;
                  }
            if (!_selection.endSegment())
                  _selection.setEndSegment(cr->segment()->nextCR());
            if (!_selection.startSegment())
                  _selection.setStartSegment(cr->segment());
            }
      else {
            select(e, SelectType::SINGLE, staffIdx);
            return;
            }

      _selection.setActiveTrack(activeTrack);

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
      if (p->subtypeValid && p->subtype != e->subtype())
            return;
      if ((p->staffStart != -1)
          && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx())))
            return;
      if (e->type() == Element::Type::CHORD || e->type() == Element::Type::REST || e->type() == Element::Type::NOTE || e->type() == Element::Type::LYRICS || e->type() == Element::Type::STEM) {
            if (p->voice != -1 && p->voice != e->voice())
                  return;
            }
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
//   selectSimilar
//---------------------------------------------------------

void Score::selectSimilar(Element* e, bool sameStaff)
      {
      Element::Type type = e->type();
      Score* score = e->score();

      ElementPattern pattern;
      pattern.type    = int(type);
      pattern.subtype = 0;
      pattern.subtypeValid = false;
      pattern.staffStart = sameStaff ? e->staffIdx() : -1;
      pattern.staffEnd = sameStaff ? e->staffIdx()+1 : -1;
      pattern.voice   = -1;
      pattern.system  = 0;

      score->scanElements(&pattern, collectMatch);

      score->select(0, SelectType::SINGLE, 0);
      foreach (Element* e, pattern.el) {
            score->select(e, SelectType::ADD, 0);
            }
      }

//---------------------------------------------------------
//   selectSimilarInRange
//---------------------------------------------------------

void Score::selectSimilarInRange(Element* e)
      {
      Element::Type type = e->type();
      ElementPattern pattern;

      Score* score = e->score();
      pattern.type    = int(type);
      pattern.subtype = 0;
      pattern.staffStart = selection().staffStart();
      pattern.staffEnd = selection().staffEnd();
      pattern.voice   = -1;
      pattern.system  = 0;
      pattern.subtypeValid = false;

      score->scanElementsInRange(&pattern, collectMatch);

      score->select(0, SelectType::SINGLE, 0);
      foreach (Element* e, pattern.el) {
                  score->select(e, SelectType::ADD, 0);
            }
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
      const ChordRest* endCR;

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
      deselectAll();
      selectRange(firstMeasureMM(), 0);
      selectRange(lastMeasureMM(), nstaves() - 1);
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
      for (const Excerpt* ex : root->excerpts())
            scores.append(ex->score());
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
            a.pan    = 60;
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
            undoInsertStaff(staff, n + i);
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
      for (Score* score : scoreList()) {
            for (Staff* staff : score->staves())
                  staff->insertTime(tick, len);
            }
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Score::setPos(POS pos, int tick)
      {
      if (tick < 0)
            tick = 0;
      if (tick != _pos[int(pos)]) {
            _pos[int(pos)] = tick;
            emit posChanged(pos, unsigned(tick));
            }
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
//    find chord/rest <= tick in staff
//---------------------------------------------------------

ChordRest* Score::findCRinStaff(int tick, int track) const
      {
      Measure* m = tick2measureMM(tick);
      if (!m) {
            qDebug("findCRinStaff: no measure for tick %d", tick);
            return nullptr;
            }
      // attach to first rest all spanner when mmRest
      if (m->isMMRest())
            tick = m->tick();
      Segment* s = m->first(Segment::Type::ChordRest);
      int strack = (track / VOICES) * VOICES;
      int etrack = strack + VOICES;
      int actualTrack = strack;

      for (Segment* ns = s; ; ns = ns->next(Segment::Type::ChordRest)) {
            if (ns == 0 || ns->tick() > tick)
                  break;
            for (int t = strack; t < etrack; ++t) {
                  if (ns->element(t)) {
                        s = ns;
                        actualTrack = t;
                        break;
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

}


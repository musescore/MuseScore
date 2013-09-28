//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: debugger.cpp 5656 2012-05-21 15:36:47Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "debugger.h"
#include "musescore.h"
#include "icons.h"
#include "textstyle.h"
#include "globals.h"
#include "libmscore/element.h"
#include "libmscore/page.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/text.h"
#include "libmscore/hairpin.h"
#include "libmscore/beam.h"
#include "libmscore/tuplet.h"
#include "libmscore/clef.h"
#include "libmscore/barline.h"
#include "libmscore/hook.h"
#include "libmscore/dynamic.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/lyrics.h"
#include "libmscore/volta.h"
#include "libmscore/line.h"
#include "libmscore/textline.h"
#include "libmscore/system.h"
#include "libmscore/arpeggio.h"
#include "libmscore/glissando.h"
#include "libmscore/tremolo.h"
#include "libmscore/articulation.h"
#include "libmscore/ottava.h"
#include "libmscore/bend.h"
#include "libmscore/stem.h"
#include "libmscore/iname.h"
#include "libmscore/accidental.h"
#include "libmscore/keysig.h"
#include "libmscore/sig.h"
#include "libmscore/notedot.h"
#include "libmscore/spacer.h"
#include "libmscore/box.h"
#include "libmscore/fret.h"
#include "libmscore/harmony.h"
#include "libmscore/stemslash.h"
#include "libmscore/ledgerline.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/chordlist.h"
#include "libmscore/bracket.h"
#include "libmscore/trill.h"

namespace Ms {

extern bool useFactorySettings;

//---------------------------------------------------------
//   ElementItem
//---------------------------------------------------------

class ElementItem : public QTreeWidgetItem
      {
      Element* el;

   public:
      ElementItem(QTreeWidget* lv, Element* e);
      ElementItem(QTreeWidgetItem* ei, Element* e);
      Element* element() const { return el; }
      void init();
      };

ElementItem::ElementItem(QTreeWidget* lv, Element* e)
   : QTreeWidgetItem(lv, e->type())
      {
      el = e;
      init();
      }

ElementItem::ElementItem(QTreeWidgetItem* ei, Element* e)
   : QTreeWidgetItem(ei, e->type())
      {
      el = e;
      init();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void ElementItem::init()
      {
      QString s;
      switch(el->type()) {
            case Element::PAGE:
                  {
                  QString no;
                  no.setNum(((Page*)el)->no()+1);
                  s = "Page-" + no;
                  }
                  break;
            case Element::MEASURE:
                  {
                  QString no;
                  no.setNum(((Measure*)el)->no()+1);
                  s = "Measure-" + no;
                  }
                  break;
            default:
                  s = el->name();
                  break;
            }
      setText(0, s);
      }

//---------------------------------------------------------
//   Debugger
//---------------------------------------------------------

Debugger::Debugger(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      for (int i = 0; i < Element::MAXTYPE; ++i)
            elementViews[i] = 0;
      curElement   = 0;

//      connect(tupletView, SIGNAL(scoreChanged()), SLOT(layoutScore()));
//      connect(notePanel,  SIGNAL(scoreChanged()), SLOT(layoutScore()));

      connect(list, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(itemClicked(QTreeWidgetItem*,int)));
      connect(list, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      connect(list, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));

      list->resizeColumnToContents(0);
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("Debugger");
            split->restoreState(settings.value("splitter").toByteArray());
            resize(settings.value("size", QSize(1000, 500)).toSize());
            move(settings.value("pos", QPoint(10, 10)).toPoint());
            settings.endGroup();
            }
      back->setEnabled(false);
      forward->setEnabled(false);
      connect(back,    SIGNAL(clicked()), SLOT(backClicked()));
      connect(forward, SIGNAL(clicked()), SLOT(forwardClicked()));
      connect(reload,  SIGNAL(clicked()), SLOT(reloadClicked()));
      connect(selectButton, SIGNAL(clicked()), SLOT(selectElement()));
      connect(resetButton,  SIGNAL(clicked()), SLOT(resetElement()));
      connect(layoutButton, SIGNAL(clicked()), SLOT(layout()));
      }

//---------------------------------------------------------
//   selectElement
//---------------------------------------------------------

void Debugger::selectElement()
      {
      curElement->score()->select(curElement);
      }

//---------------------------------------------------------
//   resetElement
//---------------------------------------------------------

void Debugger::resetElement()
      {
      curElement->reset();
      layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Debugger::layout()
      {
      curElement->score()->doLayout();
      curElement->score()->end();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Debugger::writeSettings()
      {
      QSettings settings;
      settings.beginGroup("Debugger");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.setValue("splitter", split->saveState());
      settings.endGroup();
      }

//---------------------------------------------------------
//   layoutScore
//---------------------------------------------------------

void Debugger::layoutScore()
      {
//      cs->setLayoutAll(true);
//      cs->end();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

static void addSymbol(ElementItem* parent, BSymbol* bs)
      {
      const QList<Element*>el = bs->leafs();
      ElementItem* i = new ElementItem(parent, bs);
      if (!el.isEmpty()) {
            foreach(Element* g, el)
                  addSymbol(i, static_cast<BSymbol*>(g));
            }
      }

//---------------------------------------------------------
//   addMeasureBaseToList
//---------------------------------------------------------

static void addMeasureBaseToList(ElementItem* mi, MeasureBase* mb)
      {
      foreach(Element* e, *mb->el()) {
            ElementItem* mmi = new ElementItem(mi, e);
            if (e->type() == Element::HBOX || e->type() == Element::VBOX)
                  addMeasureBaseToList(mmi, static_cast<MeasureBase*>(e));
            }
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void Debugger::showEvent(QShowEvent*)
      {
      updateList(cs);
      }

//---------------------------------------------------------
//   addBSymbol
//---------------------------------------------------------

static void addBSymbol(ElementItem* item, BSymbol* e)
      {
      ElementItem* si = new ElementItem(item, e);
      foreach(Element* ee, e->leafs())
            addBSymbol(si, static_cast<BSymbol*>(ee));
      }

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

static void addChord(ElementItem* sei, Chord* chord)
      {
      if (chord->hook())
            new ElementItem(sei, chord->hook());
      if (chord->stem())
            new ElementItem(sei, chord->stem());
      if (chord->arpeggio())
            new ElementItem(sei, chord->arpeggio());
      if (chord->tremolo() && (chord->tremolo()->chord1() == chord))
            new ElementItem(sei, chord->tremolo());
      if (chord->glissando())
            new ElementItem(sei, chord->glissando());

      foreach(Articulation* a, chord->articulations())
            new ElementItem(sei, a);
      for(LedgerLine* h = chord->ledgerLines(); h; h = h->next())
            new ElementItem(sei, h);
      foreach(Note* note, chord->notes()) {
            ElementItem* ni = new ElementItem(sei, note);
            if (note->accidental()) {
                  new ElementItem(ni, note->accidental());
                  }
            foreach (Element* f, note->el()) {
                  if (f->type() == Element::SYMBOL || f->type() == Element::IMAGE) {
                        BSymbol* bs = static_cast<BSymbol*>(f);
                        addSymbol(ni, bs);
                        }
                  else
                        new ElementItem(ni, f);
                  }
            for (int i = 0; i < 3; ++i) {
                  if (note->dot(i))
                        new ElementItem(ni, note->dot(i));
                  }

            if (note->tieFor()) {
                  Tie* tie = note->tieFor();
                  ElementItem* ti = new ElementItem(ni, tie);
                  foreach (Element* el1, tie->spannerSegments())
                        new ElementItem(ti, el1);
                  }
            foreach (Spanner* s, note->spannerFor()) {
                  ElementItem* si = new ElementItem(ni, s);
                  foreach(Element* ls, s->spannerSegments())
                        new ElementItem(si, ls);
                  }
            }
      foreach(Element* e, chord->el()) {
            ElementItem* ei = new ElementItem(sei, e);
            if (e->type() == Element::SLUR) {
                  Slur* gs = static_cast<Slur*>(e);
                  foreach (SpannerSegment* sp, gs->spannerSegments())
                        new ElementItem(ei, sp);
                  }
            }
      foreach (Chord* c, chord->graceNotes()) {
            ElementItem* ssei = new ElementItem(sei, c);
            addChord(ssei, c);
            }

      if (chord->beam() && chord->beam()->elements().front() == chord)
            new ElementItem(sei, chord->beam());
      foreach(Lyrics* lyrics, chord->lyricsList()) {
            if (lyrics)
                  new ElementItem(sei, lyrics);
            }
      DurationElement* de = chord;
      while (de->tuplet() && de->tuplet()->elements().front() == de) {
            new ElementItem(sei, de->tuplet());
            de = de->tuplet();
            }
      }

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

void Debugger::addMeasure(ElementItem* mi, Measure* measure)
      {
      int staves = cs->nstaves();
      int tracks = staves * VOICES;
      foreach (MStaff* ms, *measure->staffList()) {
            if (ms->_vspacerUp)
                  new ElementItem(mi, ms->_vspacerUp);
            if (ms->_vspacerDown)
                  new ElementItem(mi, ms->_vspacerDown);
            }
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            ElementItem* segItem = new ElementItem(mi, segment);
            for (int track = 0; track < tracks; ++track) {
                  Element* e = segment->element(track);
                  if (!e)
                        continue;
                  ElementItem* sei = new ElementItem(segItem, e);
                  if (e->type() == Element::CHORD)
                        addChord(sei, static_cast<Chord*>(e));
                  else if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr->beam() && cr->beam()->elements().front() == cr)
                              new ElementItem(sei, cr->beam());
                        foreach(Lyrics* lyrics, cr->lyricsList()) {
                              if (lyrics)
                                    new ElementItem(sei, lyrics);
                              }
                        DurationElement* de = cr;
                        while (de->tuplet() && de->tuplet()->elements().front() == de) {
                              new ElementItem(sei, de->tuplet());
                              de = de->tuplet();
                              }
                        }
                  }

            foreach(Element* s, segment->annotations()) {
                  if (s->type() == Element::SYMBOL || s->type() == Element::IMAGE)
                        addBSymbol(segItem, static_cast<BSymbol*>(s));
                  else if (s->type() == Element::FRET_DIAGRAM) {
                        ElementItem* fdi = new ElementItem(segItem, s);
                        FretDiagram* fd = static_cast<FretDiagram*>(s);
                        if (fd->harmony())
                              new ElementItem(fdi, fd->harmony());
                        }
                  else
                        new ElementItem(segItem, s);
                  }
            }
      }

//---------------------------------------------------------
//   updateList
//---------------------------------------------------------

void Debugger::updateList(Score* s)
      {
      if (cs != s) {
            backStack.clear();
            forwardStack.clear();
            back->setEnabled(false);
            forward->setEnabled(false);
            cs = s;
            }
      curElement = 0;
      list->clear();
      if (!isVisible())
            return;

      QTreeWidgetItem* li = new QTreeWidgetItem(list, Element::INVALID);
      li->setText(0, "Global");
      for (auto i : s->spanner()) {
            ElementItem* it = new ElementItem(li, i.second);
            if (i.second->type() == Element::TRILL) {
                  Trill* trill = static_cast<Trill*>(i.second);
                  if (trill->accidental())
                        new ElementItem(it, trill->accidental());
                  }
            }

      foreach (Page* page, cs->pages()) {
            ElementItem* pi = new ElementItem(list, page);

            foreach (System* system, *page->systems()) {
                  ElementItem* si = new ElementItem(pi, system);
                  if (system->barLine())
                        new ElementItem(si, system->barLine());
                  for (Bracket* b : system->brackets())
                        new ElementItem(si, b);
                  for (SpannerSegment* ss : system->spannerSegments())
                        new ElementItem(si, ss);
                  foreach(SysStaff* ss, *system->staves()) {
                        foreach(InstrumentName* in, ss->instrumentNames)
                              new ElementItem(si, in);
                        }

                  for (MeasureBase* mb : system->measures()) {
                        ElementItem* mi = new ElementItem(si, mb);
                        addMeasureBaseToList(mi, mb);

                        if (mb->type() != Element::MEASURE)
                              continue;
                        Measure* measure = (Measure*) mb;
                        if (measure->mmRest()) {
                              ElementItem* mmi = new ElementItem(mi, measure->mmRest());
                              addMeasure(mmi, measure->mmRest());
                              }
                        addMeasure(mi, measure);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   searchElement
//---------------------------------------------------------

bool Debugger::searchElement(QTreeWidgetItem* pi, Element* el)
      {
      for (int i = 0;; ++i) {
            QTreeWidgetItem* item = pi->child(i);
            if (item == 0)
                  break;
            ElementItem* ei = (ElementItem*)item;
            if (ei->element() == el) {
                  QTreeWidget* tw = pi->treeWidget();
                  tw->setItemExpanded(item, true);
                  tw->setCurrentItem(item);
                  tw->scrollToItem(item);
                  return true;
                  }
            if (searchElement(item, el)) {
                  pi->treeWidget()->setItemExpanded(item, true);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Debugger::setElement(Element* el)
      {
      if (curElement) {
            backStack.push(curElement);
            back->setEnabled(true);
            forwardStack.clear();
            forward->setEnabled(false);
            }
      updateElement(el);
      }

//---------------------------------------------------------
//   itemExpanded
//---------------------------------------------------------

void Debugger::itemExpanded(QTreeWidgetItem*)
      {
      list->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   itemClicked
//---------------------------------------------------------

void Debugger::itemClicked(QTreeWidgetItem* i, int)
      {
      if (i == 0)
            return;
      if (i->type() == Element::INVALID)
            return;
      Element* el = static_cast<ElementItem*>(i)->element();
      if (curElement) {
            backStack.push(curElement);
            back->setEnabled(true);
            forwardStack.clear();
            forward->setEnabled(false);
            }
      updateElement(el);
      }

//---------------------------------------------------------
//   updateElement
//---------------------------------------------------------

void Debugger::updateElement(Element* el)
      {
      if (el == 0 || !isVisible())
            return;
      for (int i = 0;; ++i) {
            QTreeWidgetItem* item = list->topLevelItem(i);
            if (item == 0) {
                  qDebug("Debugger::Element not found %s %p", el->name(), el);
                  break;
                  }
            ElementItem* ei = (ElementItem*)item;
            if (ei->element() == el) {
                  list->setItemExpanded(item, true);
                  list->setCurrentItem(item);
                  list->scrollToItem(item);
                  break;
                  }
            if (searchElement(item, el)) {
                  list->setItemExpanded(item, true);
                  break;
                  }
            }
      setWindowTitle(QString("MuseScore: Debugger: ") + el->name());

      ShowElementBase* ew = elementViews[el->type()];
      if (ew == 0) {
            switch (el->type()) {
                  case Element::PAGE:             ew = new ShowPageWidget;      break;
                  case Element::SYSTEM:           ew = new SystemView;          break;
                  case Element::MEASURE:          ew = new MeasureView;         break;
                  case Element::CHORD:            ew = new ChordDebug;          break;
                  case Element::NOTE:             ew = new ShowNoteWidget;      break;
                  case Element::REST:             ew = new RestView;            break;
                  case Element::CLEF:             ew = new ClefView;            break;
                  case Element::TIMESIG:          ew = new ShowTimesigWidget;   break;
                  case Element::KEYSIG:           ew = new KeySigView;          break;
                  case Element::SEGMENT:          ew = new SegmentView;         break;
                  case Element::HAIRPIN:          ew = new HairpinView;         break;
                  case Element::BAR_LINE:         ew = new BarLineView;         break;
                  case Element::DYNAMIC:          ew = new DynamicView;         break;
                  case Element::TUPLET:           ew = new TupletView;          break;
                  case Element::SLUR:             ew = new SlurTieView;         break;
                  case Element::TIE:              ew = new TieView;             break;
                  case Element::VOLTA:            ew = new VoltaView;           break;
                  case Element::VOLTA_SEGMENT:    ew = new VoltaSegmentView;    break;
                  case Element::PEDAL:
                  case Element::TEXTLINE:         ew = new TextLineView;        break;
                  case Element::PEDAL_SEGMENT:
                  case Element::TEXTLINE_SEGMENT: ew = new TextLineSegmentView; break;
                  case Element::LYRICS:           ew = new LyricsView;          break;
                  case Element::BEAM:             ew = new BeamView;            break;
                  case Element::TREMOLO:          ew = new TremoloView;         break;
                  case Element::OTTAVA:           ew = new OttavaView;          break;
                  case Element::OTTAVA_SEGMENT:   ew = new TextLineSegmentView; break;
                  case Element::SLUR_SEGMENT:     ew = new SlurSegmentView;     break;
                  case Element::ACCIDENTAL:       ew = new AccidentalView;      break;
                  case Element::ARTICULATION:     ew = new ArticulationView;    break;
                  case Element::STEM:             ew = new StemView;            break;
                  case Element::VBOX:
                  case Element::HBOX:
                  case Element::FBOX:
                  case Element::TBOX:             ew = new BoxView;             break;

                  case Element::FINGERING:
                  case Element::MARKER:
                  case Element::JUMP:
                  case Element::TEXT:
                  case Element::STAFF_TEXT:
                        ew = new TextView;
                        break;
                  case Element::HARMONY:
                        ew = new HarmonyView;
                        break;
                  case Element::TRILL_SEGMENT:
                  case Element::HAIRPIN_SEGMENT:
                        ew = new LineSegmentView; break;
                        break;
                  default:
                        ew = new ElementView;
                        break;
                  }
            stack->addWidget(ew);
            connect(ew,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
            elementViews[el->type()] = ew;
            }
      curElement = el;
      ew->setElement(el);
      stack->setCurrentWidget(ew);
      }

//-----------------------------------------
//   ElementListWidgetItem
//-----------------------------------------

class ElementListWidgetItem : public QListWidgetItem {
      Element* e;

   public:
      ElementListWidgetItem(Element* el) : QListWidgetItem () {
            e = el;
            setText(e->name());
            }
      Element* element() const { return e; }
      };

//---------------------------------------------------------
//   ShowPageWidget
//---------------------------------------------------------

ShowPageWidget::ShowPageWidget()
   : ShowElementBase()
      {
      pb.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowPageWidget::setElement(Element* e)
      {
      Page* p = (Page*)e;
      ShowElementBase::setElement(e);
      pb.pageNo->setValue(p->no());
      }

//---------------------------------------------------------
//   itemClicked
//---------------------------------------------------------

void ShowPageWidget::itemClicked(QListWidgetItem* i)
      {
      ElementListWidgetItem* item = (ElementListWidgetItem*)i;
      emit elementChanged(item->element());
      }

//---------------------------------------------------------
//   MeasureView
//---------------------------------------------------------

MeasureView::MeasureView()
   : ShowElementBase()
      {
      mb.setupUi(addWidget());
      connect(mb.sel, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      connect(mb.nextButton, SIGNAL(clicked()), SLOT(nextClicked()));
      connect(mb.prevButton, SIGNAL(clicked()), SLOT(prevClicked()));
      connect(mb.mmRest, SIGNAL(clicked()), SLOT(mmRestClicked()));
      }

//---------------------------------------------------------
//   nextClicked
//---------------------------------------------------------

void MeasureView::nextClicked()
      {
      emit elementChanged(((MeasureBase*)element())->next());
      }

//---------------------------------------------------------
//   prevClicked
//---------------------------------------------------------

void MeasureView::prevClicked()
      {
      emit elementChanged(((MeasureBase*)element())->prev());
      }

//---------------------------------------------------------
//   mmRestClicked
//---------------------------------------------------------

void MeasureView::mmRestClicked()
      {
      emit elementChanged(((Measure*)element())->mmRest());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void MeasureView::setElement(Element* e)
      {
      Measure* m = (Measure*)e;
      ShowElementBase::setElement(e);

      mb.segments->setValue(m->size());
      mb.staves->setValue(m->staffList()->size());
      mb.measureNo->setValue(m->no());
      mb.noOffset->setValue(m->noOffset());
      mb.stretch->setValue(m->userStretch());
      mb.lineBreak->setChecked(m->lineBreak());
      mb.pageBreak->setChecked(m->pageBreak());
      mb.sectionBreak->setChecked(m->sectionBreak());
      mb.irregular->setChecked(m->irregular());
      mb.endRepeat->setValue(m->repeatCount());
      mb.repeatFlags->setText(QString("0x%1").arg(m->repeatFlags(), 6, 16, QChar('0')));
      mb.breakMultiMeasureRest->setChecked(m->getBreakMultiMeasureRest());
      mb.breakMMRest->setChecked(m->breakMMRest());
      mb.endBarLineType->setValue(m->endBarLineType());
      mb.endBarLineGenerated->setChecked(m->endBarLineGenerated());
      mb.endBarLineVisible->setChecked(m->endBarLineVisible());
      mb.mmRestCount->setValue(m->mmRestCount());
      mb.timesig->setText(m->timesig().print());
      mb.len->setText(m->len().print());
      mb.tick->setValue(m->tick());
      mb.sel->clear();
      foreach(const Element* e, *m->el()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
//            item->setText(1, QString("%1").arg(e->subtype()));
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            mb.sel->addTopLevelItem(item);
            }
      mb.prevButton->setEnabled(m->prev());
      mb.nextButton->setEnabled(m->next());
      mb.mmRest->setEnabled(m->mmRest() != 0);
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void MeasureView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   SegmentView
//---------------------------------------------------------

SegmentView::SegmentView()
   : ShowElementBase()
      {
      sb.setupUi(addWidget());
      sb.segmentType->clear();
      static std::vector<Segment::SegmentType> segmentTypes = {
            Segment::SegClef, Segment::SegKeySig, Segment::SegTimeSig, Segment::SegStartRepeatBarLine,
            Segment::SegBarLine, Segment::SegChordRest, Segment::SegBreath, Segment::SegEndBarLine,
            Segment::SegTimeSigAnnounce, Segment::SegKeySigAnnounce
            };
      for (Segment::SegmentType t : segmentTypes)
            sb.segmentType->addItem(Segment::subTypeName(t), int(t));
      connect(sb.lyrics, SIGNAL(itemClicked(QListWidgetItem*)),      SLOT(gotoElement(QListWidgetItem*)));
      connect(sb.spannerFor, SIGNAL(itemClicked(QListWidgetItem*)),  SLOT(gotoElement(QListWidgetItem*)));
      connect(sb.spannerBack, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(sb.annotations, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SegmentView::setElement(Element* e)
      {
      ShowElementBase::setElement(e);

      Segment* s = (Segment*)e;
      ShowElementBase::setElement(e);
      int st = s->segmentType();
      int idx;
      for (idx = 0; idx < 11; ++idx) {
            if ((1 << idx) == st)
                  break;
            }
      int tick = s->tick();
      TimeSigMap* sm = s->score()->sigmap();

      int bar, beat, ticks;
      sm->tickValues(tick, &bar, &beat, &ticks);
      sb.bar->setValue(bar);
      sb.beat->setValue(beat);
      sb.ticks->setValue(ticks);
      sb.tick->setValue(s->tick());
      sb.rtick->setValue(s->rtick());
      sb.segmentType->setCurrentIndex(idx);
      sb.lyrics->clear();

//      Score* cs = e->score();
#if 0 // TODO
      for (int i = 0; i < cs->nstaves(); ++i) {
            const LyricsList* ll = s->lyricsList(i);
            if (ll) {
                  foreach(Lyrics* l, *ll) {
                        QString s;
                        s.setNum(long(l), 16);
                        QListWidgetItem* item = new QListWidgetItem(s, 0, long(l));
                        sb.lyrics->addItem(item);
                        }
                  }
            }
#endif
      sb.spannerFor->clear();
      sb.spannerBack->clear();
      sb.annotations->clear();
      foreach(Element* sp, s->annotations()) {
            QString s;
            s.setNum(long(sp), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)sp));
            sb.annotations->addItem(item);
            }
      }

//---------------------------------------------------------
//   ChordDebug
//---------------------------------------------------------

ChordDebug::ChordDebug()
   : ShowElementBase()
      {
      // chord rest
      crb.setupUi(addWidget());
      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.upFlag,   SIGNAL(toggled(bool)), SLOT(upChanged(bool)));
      connect(crb.beamMode, SIGNAL(activated(int)), SLOT(beamModeChanged(int)));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.lyrics, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      // chord
      cb.setupUi(addWidget());
      connect(cb.hookButton, SIGNAL(clicked()), SLOT(hookClicked()));
      connect(cb.stemButton, SIGNAL(clicked()), SLOT(stemClicked()));
      connect(cb.stemSlashButton, SIGNAL(clicked()), SLOT(stemSlashClicked()));
      connect(cb.arpeggioButton, SIGNAL(clicked()), SLOT(arpeggioClicked()));
      connect(cb.tremoloButton, SIGNAL(clicked()), SLOT(tremoloClicked()));
      connect(cb.glissandoButton, SIGNAL(clicked()), SLOT(glissandoClicked()));

      connect(cb.stemDirection, SIGNAL(activated(int)), SLOT(directionChanged(int)));
      connect(cb.helplineList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(cb.notes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(cb.graceChords1, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(cb.graceChords2, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(cb.elements,     SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      crb.beamMode->addItem("auto");
      crb.beamMode->addItem("beam begin");
      crb.beamMode->addItem("beam mid");
      crb.beamMode->addItem("beam end");
      crb.beamMode->addItem("no beam");
      crb.beamMode->addItem("begin 1/32");
      crb.beamMode->addItem("begin 1/64");

      cb.stemDirection->addItem("Auto", 0);
      cb.stemDirection->addItem("Up",   1);
      cb.stemDirection->addItem("Down", 2);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ChordDebug::setElement(Element* e)
      {
      Chord* chord = (Chord*)e;
      ShowElementBase::setElement(e);

      crb.tick->setValue(chord->tick());
      crb.beamButton->setEnabled(chord->beam());
      crb.tupletButton->setEnabled(chord->tuplet());
      crb.upFlag->setChecked(chord->up());
      crb.beamMode->setCurrentIndex(int(chord->beamMode()));
      crb.dots->setValue(chord->dots());
      crb.ticks->setValue(chord->actualTicks());
      crb.durationType->setText(chord->durationType().name());
      crb.duration->setText(chord->duration().print());
      crb.move->setValue(chord->staffMove());
      crb.spaceL->setValue(chord->space().lw());
      crb.spaceR->setValue(chord->space().rw());

      cb.hookButton->setEnabled(chord->hook());
      cb.stemButton->setEnabled(chord->stem());
      cb.graceNote->setChecked(chord->noteType() != NOTE_NORMAL);
      cb.stemDirection->setCurrentIndex(int(chord->stemDirection()));

      cb.stemSlashButton->setEnabled(chord->stemSlash());
      cb.arpeggioButton->setEnabled(chord->arpeggio());
      cb.tremoloButton->setEnabled(chord->tremolo());
      cb.glissandoButton->setEnabled(chord->glissando());

      crb.attributes->clear();
      foreach(Articulation* a, chord->articulations()) {
            QString s;
            s.setNum(long(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)a));
            crb.attributes->addItem(item);
            }
      crb.lyrics->clear();
      foreach(Lyrics* lyrics, chord->lyricsList()) {
            QString s;
            s.setNum(long(lyrics), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)lyrics));
            crb.lyrics->addItem(item);
            }
      cb.helplineList->clear();
      for (LedgerLine* h = chord->ledgerLines(); h; h = h->next()) {
            QString s;
            s.setNum(long(h), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)h));
            cb.helplineList->addItem(item);
            }
      cb.notes->clear();
      foreach(Element* n, chord->notes()) {
            QString s;
            s.setNum(long(n), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)n));
            cb.notes->addItem(item);
            }
      cb.graceChords1->clear();
      for (Element* c : chord->graceNotes()) {
            QString s;
            s.setNum(long(c), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)c));
            cb.graceChords1->addItem(item);
            }
      cb.elements->clear();
      for (Element* c : chord->el()) {
            QString s;
            s.setNum(long(c), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)c));
            cb.elements->addItem(item);
            }
      cb.userPlayEvents->setChecked(chord->userPlayEvents());
      }

//---------------------------------------------------------
//   hookClicked
//---------------------------------------------------------

void ChordDebug::hookClicked()
      {
      emit elementChanged(((Chord*)element())->hook());
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void ChordDebug::stemClicked()
      {
      emit elementChanged(((Chord*)element())->stem());
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void ChordDebug::beamClicked()
      {
      emit elementChanged(((Chord*)element())->beam());
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void ChordDebug::tupletClicked()
      {
      emit elementChanged(((Chord*)element())->tuplet());
      }

void ChordDebug::stemSlashClicked()
      {
      emit elementChanged(((Chord*)element())->stemSlash());
      }

void ChordDebug::arpeggioClicked()
      {
      emit elementChanged(((Chord*)element())->arpeggio());
      }

void ChordDebug::tremoloClicked()
      {
      emit elementChanged(((Chord*)element())->tremolo());
      }

void ChordDebug::glissandoClicked()
      {
      emit elementChanged(((Chord*)element())->glissando());
      }

//---------------------------------------------------------
//   upChanged
//---------------------------------------------------------

void ChordDebug::upChanged(bool val)
      {
      ((Chord*)element())->setUp(val);
      }

//---------------------------------------------------------
//   beamModeChanged
//---------------------------------------------------------

void ChordDebug::beamModeChanged(int n)
      {
      ((Chord*)element())->setBeamMode(BeamMode(n));
      element()->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   directionChanged
//---------------------------------------------------------

void ChordDebug::directionChanged(int val)
      {
      ((Chord*)element())->setStemDirection(MScore::Direction(val));
      }

//---------------------------------------------------------
//   ShowNoteWidget
//---------------------------------------------------------

ShowNoteWidget::ShowNoteWidget()
   : ShowElementBase()
      {
      nb.setupUi(addWidget());

      connect(nb.tieFor,     SIGNAL(clicked()), SLOT(tieForClicked()));
      connect(nb.tieBack,    SIGNAL(clicked()), SLOT(tieBackClicked()));
      connect(nb.accidental, SIGNAL(clicked()), SLOT(accidentalClicked()));
      connect(nb.fingering,  SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(nb.tpc,        SIGNAL(valueChanged(int)), SLOT(tpcChanged(int)));
      connect(nb.dot1,       SIGNAL(clicked()), SLOT(dot1Clicked()));
      connect(nb.dot2,       SIGNAL(clicked()), SLOT(dot2Clicked()));
      connect(nb.dot3,       SIGNAL(clicked()), SLOT(dot3Clicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowNoteWidget::setElement(Element* e)
      {
      Note* note = (Note*)e;
      ShowElementBase::setElement(e);

      nb.pitch->setValue(note->pitch());
      nb.tuning->setValue(note->tuning());
      nb.line->setValue(note->line());
      nb.string->setValue(note->string());
      nb.fret->setValue(note->fret());
      nb.mirror->setChecked(note->mirror());
      nb.tpc->setValue(note->tpc());
      nb.headGroup->setValue(note->headGroup());
      nb.hidden->setChecked(note->hidden());
      nb.subchannel->setValue(note->subchannel());

      nb.tieFor->setEnabled(note->tieFor());
      nb.tieBack->setEnabled(note->tieBack());
      nb.accidental->setEnabled(note->accidental());
      nb.dot1->setEnabled(note->dot(0));
      nb.dot2->setEnabled(note->dot(1));
      nb.dot3->setEnabled(note->dot(2));

      note->el().clear();     // ??
      foreach(Element* text, note->el()) {
            QString s;
            s.setNum(long(text), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)text));
            nb.fingering->addItem(item);
            }
      note->playEvents().clear();
      foreach(const NoteEvent& e, note->playEvents()) {
            QString s = QString("%1 %2 %3").arg(e.pitch()).arg(e.ontime()).arg(e.len());
            QListWidgetItem* item = new QListWidgetItem(s);
            nb.noteEvents->addItem(item);
            }
      }

//---------------------------------------------------------
//   dot1Clicked
//---------------------------------------------------------

void ShowNoteWidget::dot1Clicked()
      {
      emit elementChanged(((Note*)element())->dot(0));
      }

//---------------------------------------------------------
//   dot2Clicked
//---------------------------------------------------------

void ShowNoteWidget::dot2Clicked()
      {
      emit elementChanged(((Note*)element())->dot(1));
      }

//---------------------------------------------------------
//   dot3Clicked
//---------------------------------------------------------

void ShowNoteWidget::dot3Clicked()
      {
      emit elementChanged(((Note*)element())->dot(2));
      }

//---------------------------------------------------------
//   tpcChanged
//---------------------------------------------------------

void ShowNoteWidget::tpcChanged(int val)
      {
      ((Note*)element())->setTpc(val);
      emit scoreChanged();
      }

//---------------------------------------------------------
//   tieForClicked
//---------------------------------------------------------

void ShowNoteWidget::tieForClicked()
      {
      emit elementChanged(((Note*)element())->tieFor());
      }

//---------------------------------------------------------
//   tieBackClicked
//---------------------------------------------------------

void ShowNoteWidget::tieBackClicked()
      {
      emit elementChanged(((Note*)element())->tieBack());
      }

//---------------------------------------------------------
//   accidentalClicked
//---------------------------------------------------------

void ShowNoteWidget::accidentalClicked()
      {
      emit elementChanged(((Note*)element())->accidental());
      }

//---------------------------------------------------------
//   RestView
//---------------------------------------------------------

RestView::RestView()
   : ShowElementBase()
      {
      // chort rest
      crb.setupUi(addWidget());
      crb.beamMode->addItem("auto");
      crb.beamMode->addItem("beam begin");
      crb.beamMode->addItem("beam mid");
      crb.beamMode->addItem("beam end");
      crb.beamMode->addItem("no beam");
      crb.beamMode->addItem("begin 1/32");

      rb.setupUi(addWidget());

      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.lyrics, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void RestView::setElement(Element* e)
      {
      Rest* rest = (Rest*)e;
      ShowElementBase::setElement(e);

      crb.tick->setValue(rest->tick());
      crb.beamButton->setEnabled(rest->beam());
      crb.tupletButton->setEnabled(rest->tuplet());
      crb.upFlag->setChecked(rest->up());
      crb.beamMode->setCurrentIndex(int(rest->beamMode()));
      crb.attributes->clear();
      crb.dots->setValue(rest->dots());
      crb.ticks->setValue(rest->actualTicks());
      crb.durationType->setText(rest->durationType().name());
      crb.duration->setText(rest->duration().print());
      crb.move->setValue(rest->staffMove());
      crb.spaceL->setValue(rest->space().lw());
      crb.spaceR->setValue(rest->space().rw());

      crb.attributes->clear();
      foreach(Articulation* a, rest->articulations()) {
            QString s;
            s.setNum(long(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)a));
            crb.attributes->addItem(item);
            }
      crb.lyrics->clear();
      foreach(Lyrics* lyrics, rest->lyricsList()) {
            QString s;
            s.setNum(long(lyrics), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)lyrics));
            crb.lyrics->addItem(item);
            }

      Measure* m = rest->measure();
      int seg = 0;
      int tracks = 0; // TODO cs->nstaves() * VOICES;
      for (Segment* s = m->first(); s; s = s->next(), ++seg) {
            int track;
            for (track = 0; track < tracks; ++track) {
                  Element* e = s->element(track);
                  if (e == rest)
                        break;
                  }
            if (track < tracks)
                  break;
            }
      rb.sym->setValue(rest->sym());
      rb.dotline->setValue(rest->getDotline());
      rb.mmWidth->setValue(rest->mmWidth());
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void RestView::beamClicked()
      {
      emit elementChanged(static_cast<Rest*>(element())->beam());
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void RestView::tupletClicked()
      {
      emit elementChanged(static_cast<Rest*>(element())->tuplet());
      }

//---------------------------------------------------------
//   ShowTimesigWidget
//---------------------------------------------------------

ShowTimesigWidget::ShowTimesigWidget()
   : ShowElementBase()
      {
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowTimesigWidget::setElement(Element* e)
      {
//      TimeSig* tsig = (TimeSig*)e;
      ShowElementBase::setElement(e);
      }

//---------------------------------------------------------
//   ElementView
//---------------------------------------------------------

ElementView::ElementView()
   : ShowElementBase()
      {
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   TextView
//---------------------------------------------------------

TextView::TextView()
   : ShowElementBase()
      {
      tb.setupUi(addWidget());
      connect(tb.text, SIGNAL(textChanged()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void TextView::textChanged()
      {
      emit scoreChanged();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TextView::setElement(Element* e)
      {
      Text* te = (Text*)e;

      tb.textStyle->clear();
      for (int i = 0; i < TEXT_STYLES; ++i)
            tb.textStyle->addItem(e->score()->textStyle(i).name());

      ShowElementBase::setElement(e);
      tb.text->setPlainText(te->text());
      tb.xoffset->setValue(te->xoff());
      tb.yoffset->setValue(te->yoff());
      tb.rxoffset->setValue(te->reloff().x());
      tb.ryoffset->setValue(te->reloff().y());
      tb.offsetType->setCurrentIndex(int(te->offsetType()));
      tb.textStyle->setCurrentIndex(te->textStyleType());
      tb.styled->setChecked(te->styled());
      tb.layoutToParentWidth->setChecked(te->layoutToParentWidth());
      }

//---------------------------------------------------------
//   HarmonyView
//---------------------------------------------------------

HarmonyView::HarmonyView()
   : ShowElementBase()
      {
      tb.setupUi(addWidget());
      hb.setupUi(addWidget());

      connect(hb.leftParen, SIGNAL(clicked(bool)), SLOT(on_leftParen_clicked(bool)));
      connect(hb.rightParen, SIGNAL(clicked(bool)), SLOT(on_rightParen_clicked(bool)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void HarmonyView::setElement(Element* e)
      {
      Harmony* harmony = (Harmony*)e;

      tb.textStyle->clear();
      for (int i = 0; i < TEXT_STYLES; ++i)
            tb.textStyle->addItem(e->score()->textStyle(i).name());

      ShowElementBase::setElement(e);
      tb.text->setPlainText(harmony->text());
      tb.xoffset->setValue(harmony->xoff());
      tb.yoffset->setValue(harmony->yoff());
      tb.rxoffset->setValue(harmony->reloff().x());
      tb.ryoffset->setValue(harmony->reloff().y());
      tb.offsetType->setCurrentIndex(int(harmony->offsetType()));
      tb.textStyle->setCurrentIndex(harmony->textStyleType());
      tb.styled->setChecked(harmony->styled());
      tb.layoutToParentWidth->setChecked(harmony->layoutToParentWidth());

      hb.tbboxx->setValue(harmony->bboxtight().x());
      hb.tbboxy->setValue(harmony->bboxtight().y());
      hb.tbboxw->setValue(harmony->bboxtight().width());
      hb.tbboxh->setValue(harmony->bboxtight().height());
      hb.leftParen->setChecked(harmony->leftParen());
      hb.rightParen->setChecked(harmony->rightParen());
      hb.rootTpc->setValue(harmony->rootTpc());
      if (harmony->rootTpc() == INVALID_TPC)
            hb.rootName->setText("");
      else
            hb.rootName->setText(harmony->rootName());
      hb.bassTpc->setValue(harmony->baseTpc());
      if (harmony->baseTpc() == INVALID_TPC)
            hb.bassName->setText("");
      else
            hb.bassName->setText(harmony->baseName());
      hb.chordId->setValue(harmony->id());
      hb.chordName->setText(harmony->hTextName());
      hb.userName->setText(harmony->hUserName());

      // need to set header row
      hb.degreeTab->setColumnWidth(0,hb.degreeTab->width()/3);
      hb.degreeTab->setColumnWidth(1,hb.degreeTab->width()/3);
      hb.degreeTab->setRowCount(harmony->numberOfDegrees());
      for (int i = 0, n = harmony->numberOfDegrees(); i < n; ++i) {
            const HDegree& d = harmony->degree(i);
            QString s;
            switch (d.type()) {
                  case ADD:      s = "add";      break;
                  case ALTER:    s = "alter";    break;
                  case SUBTRACT: s = "subtract"; break;
                  default:       s = "";         break;
                  }
            hb.degreeTab->setItem(i, 0, new QTableWidgetItem(s));
            hb.degreeTab->setItem(i, 1, new QTableWidgetItem(QVariant(d.value()).toString()));
            hb.degreeTab->setItem(i, 2, new QTableWidgetItem(QVariant(d.alter()).toString()));
            }
      }

void HarmonyView::on_leftParen_clicked(bool checked)
      {
      hb.leftParen->setChecked(!checked); // simulate read-only checkbox
      }

void HarmonyView::on_rightParen_clicked(bool checked)
      {
      hb.rightParen->setChecked(!checked); // simulate read-only checkbox
      }

//---------------------------------------------------------
//   SpannerView
//---------------------------------------------------------

SpannerView::SpannerView()
   : ShowElementBase()
      {
      sp.setupUi(addWidget());
      connect(sp.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(gotoElement(QTreeWidgetItem*)));
      connect(sp.startElement, SIGNAL(clicked()), SLOT(startClicked()));
      connect(sp.endElement, SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SpannerView::setElement(Element* e)
      {
      Spanner* spanner = static_cast<Spanner*>(e);
      ShowElementBase::setElement(e);
      sp.tick->setValue(spanner->tick());
      sp.tick2->setValue(spanner->tick2());
      sp.anchor->setCurrentIndex(int(spanner->anchor()));

      sp.segments->clear();
      foreach(const Element* e, spanner->spannerSegments()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            sp.segments->addTopLevelItem(item);
            }
      sp.startElement->setEnabled(spanner->startElement() != 0);
      sp.endElement->setEnabled(spanner->endElement() != 0);
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void SpannerView::startClicked()
      {
      Spanner* spanner = static_cast<Spanner*>(element());
      emit elementChanged(spanner->startElement());
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void SpannerView::endClicked()
      {
      Spanner* spanner = static_cast<Spanner*>(element());
      emit elementChanged(spanner->endElement());
      }

//---------------------------------------------------------
//   HairpinView
//---------------------------------------------------------

HairpinView::HairpinView()
   : SpannerView()
      {
      sl.setupUi(addWidget());
      hp.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void HairpinView::setElement(Element* e)
      {
      SpannerView::setElement(e);
      Hairpin* hairpin = (Hairpin*)e;
      ShowElementBase::setElement(e);
      sl.diagonal->setChecked(hairpin->diagonal());
      hp.veloChange->setValue(hairpin->veloChange());
      }

//---------------------------------------------------------
//   BarLineView
//---------------------------------------------------------

BarLineView::BarLineView()
   : ShowElementBase()
      {
      bl.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void BarLineView::setElement(Element* e)
      {
      BarLine* barline = (BarLine*)e;
      ShowElementBase::setElement(e);
      bl.subType->setValue(barline->barLineType());
      bl.span->setValue(barline->span());
      bl.spanFrom->setValue(barline->spanFrom());
      bl.spanTo->setValue(barline->spanTo());
      }

//---------------------------------------------------------
//   DynamicView
//---------------------------------------------------------

DynamicView::DynamicView()
   : ShowElementBase()
      {
      tb.setupUi(addWidget());
      bl.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void DynamicView::setElement(Element* e)
      {
      Dynamic* dynamic = (Dynamic*)e;

      tb.textStyle->clear();
      for (int i = 0; i < TEXT_STYLES; ++i)
            tb.textStyle->addItem(e->score()->textStyle(i).name());

      tb.text->setPlainText(dynamic->text());
      tb.xoffset->setValue(dynamic->xoff());
      tb.yoffset->setValue(dynamic->yoff());
      tb.rxoffset->setValue(dynamic->reloff().x());
      tb.ryoffset->setValue(dynamic->reloff().y());
      tb.offsetType->setCurrentIndex(int(dynamic->offsetType()));
      tb.textStyle->setCurrentIndex(dynamic->textStyleType());
      tb.styled->setChecked(dynamic->styled());
      tb.layoutToParentWidth->setChecked(dynamic->layoutToParentWidth());

      ShowElementBase::setElement(e);
      bl.subType->setValue(dynamic->dynamicType());
      }

//---------------------------------------------------------
//   TupletView
//---------------------------------------------------------

TupletView::TupletView()
   : ShowElementBase()
      {
      tb.setupUi(addWidget());

      tb.direction->addItem("Auto", 0);
      tb.direction->addItem("Up",   1);
      tb.direction->addItem("Down", 2);

      connect(tb.number, SIGNAL(clicked()), SLOT(numberClicked()));
      connect(tb.elements, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   numberClicked
//---------------------------------------------------------

void TupletView::numberClicked()
      {
      emit elementChanged(((Tuplet*)element())->number());
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void TupletView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TupletView::setElement(Element* e)
      {
      ShowElementBase::setElement(e);
      Tuplet* tuplet = static_cast<Tuplet*>(e);
      tb.baseLen->setText(tuplet->baseLen().name());
      tb.ratioZ->setValue(tuplet->ratio().numerator());
      tb.ratioN->setValue(tuplet->ratio().denominator());
      tb.number->setEnabled(tuplet->number());
      tb.elements->clear();
      foreach(DurationElement* e, tuplet->elements()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            item->setText(1, QString("%1").arg(e->tick()));
            item->setText(2, QString("%1").arg(e->actualTicks()));
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            tb.elements->addTopLevelItem(item);
            }
      tb.isUp->setChecked(tuplet->isUp());
      tb.direction->setCurrentIndex(int(tuplet->direction()));
      }

//---------------------------------------------------------
//   DoubleLabel
//---------------------------------------------------------

DoubleLabel::DoubleLabel(QWidget* parent)
   : QLabel(parent)
      {
//      setFrameStyle(QFrame::LineEditPanel | QFrame::Sunken);
  //    setPaletteBackgroundColor(palette().active().brightText());
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void DoubleLabel::setValue(double val)
      {
      QString s;
      setText(s.setNum(val, 'g', 3));
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize DoubleLabel::sizeHint() const
      {
      QFontMetrics fm = fontMetrics();
      int h           = fm.height() + 4;
      int n           = 3 + 3;
      int w = fm.width(QString("-0.")) + fm.width('0') * n + 6;
      return QSize(w, h);
      }

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

ShowElementBase::ShowElementBase()
   : QWidget()
      {
      layout = new QVBoxLayout;
      setLayout(layout);
      layout->addStretch(2000);

      eb.setupUi(addWidget());

      connect(eb.parentButton,   SIGNAL(clicked()), SLOT(parentClicked()));
      connect(eb.offsetx,        SIGNAL(valueChanged(double)), SLOT(offsetxChanged(double)));
      connect(eb.offsety,        SIGNAL(valueChanged(double)), SLOT(offsetyChanged(double)));
      connect(eb.selected,       SIGNAL(clicked(bool)), SLOT(selectedClicked(bool)));
      connect(eb.visible,        SIGNAL(clicked(bool)), SLOT(visibleClicked(bool)));
      connect(eb.link1,          SIGNAL(clicked()), SLOT(linkClicked()));
      }

//---------------------------------------------------------
//   addWidget
//---------------------------------------------------------

QWidget* ShowElementBase::addWidget()
      {
      QWidget* w = new QWidget;
      layout->insertWidget(layout->count()-1, w);
      return w;
      }

//---------------------------------------------------------
//   gotoElement
//---------------------------------------------------------

void ShowElementBase::gotoElement(QListWidgetItem* item)
      {
      Element* e = (Element*)item->data(Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   gotoElement
//---------------------------------------------------------

void ShowElementBase::gotoElement(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

void ShowElementBase::setElement(Element* e)
      {
      el = e;

      eb.address->setText(QString("%1").arg((unsigned long)e, 0, 16));
      eb.score->setText(QString("%1").arg((unsigned long)(e->score()), 0, 16));

      eb.selected->setChecked(e->selected());
      eb.selectable->setChecked(e->selectable());
      eb.droptarget->setChecked(e->dropTarget());
      eb.generated->setChecked(e->generated());
      eb.visible->setChecked(e->visible());
      eb.track->setValue(e->track());
      eb.z->setValue(e->z());
      eb.posx->setValue(e->ipos().x());
      eb.posy->setValue(e->ipos().y());
      eb.cposx->setValue(e->pagePos().x());
      eb.cposy->setValue(e->pagePos().y());
      eb.offsetx->setValue(e->userOff().x());
      eb.offsety->setValue(e->userOff().y());
      eb.readPosX->setValue(e->readPos().x());
      eb.readPosY->setValue(e->readPos().y());
      eb.placement->setCurrentIndex(int(e->placement()));

#if 0
      Align a = e->align();
      QString s;
      s += a & ALIGN_LEFT ? "L" : "-";
      s += a & ALIGN_HCENTER ? "C" : "-";
      s += a & ALIGN_RIGHT ? "R" : "-";
      s += " ";
      s += a & ALIGN_TOP ? "T" : "-";
      s += a & ALIGN_VCENTER ? "C" : "-";
      s += a & ALIGN_BOTTOM ? "B" : "-";
      s += a & ALIGN_BASELINE ? "L" : "-";
      eb.alignment->setText(s);
#endif

      eb.bboxx->setValue(e->bbox().x());
      eb.bboxy->setValue(e->bbox().y());
      eb.bboxw->setValue(e->bbox().width());
      eb.bboxh->setValue(e->bbox().height());
      eb.color->setColor(e->color());
      eb.parentButton->setEnabled(e->parent());
      eb.link1->setEnabled(e->links());
      eb.mag->setValue(e->mag());
      eb.systemFlag->setChecked(e->systemFlag());
      }

//---------------------------------------------------------
//   selectedClicked
//---------------------------------------------------------

void ShowElementBase::selectedClicked(bool val)
      {
      QRectF r(el->abbox());
      if (val)
            el->score()->select(el, SELECT_ADD, 0);
      else
            el->score()->deselect(el);
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   visibleClicked
//---------------------------------------------------------

void ShowElementBase::visibleClicked(bool val)
      {
      QRectF r(el->abbox());
      el->setVisible(val);
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   parentClicked
//---------------------------------------------------------

void ShowElementBase::parentClicked()
      {
      emit elementChanged(el->parent());
      }

//---------------------------------------------------------
//   linkClicked
//---------------------------------------------------------

void ShowElementBase::linkClicked()
      {
      qDebug("linkClicked");
      foreach(Element* e, *el->links()) {
            qDebug("  element <%p> <%p>", e->score(), e);
            if (e != el) {
                  emit elementChanged(e);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   offsetxChanged
//---------------------------------------------------------

void ShowElementBase::offsetxChanged(double val)
      {
      QRectF r(el->abbox());
      el->setUserXoffset(val);
//      Element* e = el;
//TODO      while ((e = e->parent()))
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   offsetyChanged
//---------------------------------------------------------

void ShowElementBase::offsetyChanged(double val)
      {
      QRectF r(el->abbox());
      el->setUserYoffset(val);
      el->score()->addRefresh(r | el->abbox());
      }

//---------------------------------------------------------
//   SlurTieView
//---------------------------------------------------------

SlurTieView::SlurTieView()
   : SpannerView()
      {
      st.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SlurTieView::setElement(Element* e)
      {
      Slur* slur = (Slur*)e;
      SpannerView::setElement(e);

      st.upFlag->setChecked(slur->up());
      st.direction->setCurrentIndex(slur->slurDirection());
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void SlurTieView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   TieView
//---------------------------------------------------------

TieView::TieView()
   : SlurTieView()
      {
      tb.setupUi(addWidget());
      connect(tb.startNote, SIGNAL(clicked()), SLOT(startClicked()));
      connect(tb.endNote, SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TieView::setElement(Element* e)
      {
      SlurTieView::setElement(e);
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void TieView::startClicked()
      {
      emit elementChanged(static_cast<Spanner*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void TieView::endClicked()
      {
      emit elementChanged(static_cast<Spanner*>(element())->endElement());
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void VoltaView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   beginTextClicked
//---------------------------------------------------------

void VoltaView::beginTextClicked()
      {
      emit elementChanged(static_cast<Volta*>(element())->beginText());
      }

//---------------------------------------------------------
//   continueTextClicked
//---------------------------------------------------------

void VoltaView::continueTextClicked()
      {
      emit elementChanged(static_cast<Volta*>(element())->continueText());
      }

//---------------------------------------------------------
//   VoltaView
//---------------------------------------------------------

VoltaView::VoltaView()
   : ShowElementBase()
      {
      sp.setupUi(addWidget());

      // SLineBase
      lb.setupUi(addWidget());

      // TextLineBase
      tlb.setupUi(addWidget());

      connect(tlb.beginText,    SIGNAL(clicked()), SLOT(beginTextClicked()));
      connect(tlb.continueText, SIGNAL(clicked()), SLOT(continueTextClicked()));
      connect(sp.segments,      SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaView::setElement(Element* e)
      {
      Volta* volta = (Volta*)e;
      ShowElementBase::setElement(e);

      tlb.lineWidth->setValue(volta->lineWidth().val());
//      lb.anchor->setCurrentIndex(int(volta->anchor()));
      lb.diagonal->setChecked(volta->diagonal());
//      lb.leftElement->setText(QString("%1").arg((unsigned long)volta->startElement(), 8, 16));
//      lb.rightElement->setText(QString("%1").arg((unsigned long)volta->endElement(), 8, 16));

      sp.segments->clear();
      const QList<SpannerSegment*>& el = volta->spannerSegments();
      foreach(const SpannerSegment* e, el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            sp.segments->addTopLevelItem(item);
            }

//      sp.startElement->setEnabled(volta->startElement() != 0);
//      sp.endElement->setEnabled(volta->endElement() != 0);
      sp.anchor->setCurrentIndex(int(volta->anchor()));

      tlb.beginText->setEnabled(volta->beginText());
      tlb.continueText->setEnabled(volta->continueText());
      }

//---------------------------------------------------------
//   leftElementClicked
//---------------------------------------------------------

void VoltaView::leftElementClicked()
      {
//      emit elementChanged(static_cast<Volta*>(element())->startElement());
      }

//---------------------------------------------------------
//   rightElementClicked
//---------------------------------------------------------

void VoltaView::rightElementClicked()
      {
//      emit elementChanged(static_cast<Volta*>(element())->endElement());
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void VoltaView::startClicked()
      {
//      emit elementChanged(static_cast<Spanner*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void VoltaView::endClicked()
      {
//      emit elementChanged(static_cast<Spanner*>(element())->endElement());
      }


//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

VoltaSegmentView::VoltaSegmentView()
   : ShowElementBase()
      {
      lb.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaSegmentView::setElement(Element* e)
      {
      VoltaSegment* vs = (VoltaSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(vs->spannerSegmentType());
      lb.pos2x->setValue(vs->pos2().x());
      lb.pos2y->setValue(vs->pos2().y());
      lb.offset2x->setValue(vs->userOff2().x());
      lb.offset2y->setValue(vs->userOff2().y());
      }

//---------------------------------------------------------
//   LineSegmentView
//---------------------------------------------------------

LineSegmentView::LineSegmentView()
   : ShowElementBase()
      {
      lb.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void LineSegmentView::setElement(Element* e)
      {
      LineSegment* vs = (LineSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(vs->spannerSegmentType());
      lb.pos2x->setValue(vs->pos2().x());
      lb.pos2y->setValue(vs->pos2().y());
      lb.offset2x->setValue(vs->userOff2().x());
      lb.offset2y->setValue(vs->userOff2().y());
      }

//---------------------------------------------------------
//   LyricsView
//---------------------------------------------------------

LyricsView::LyricsView()
   : ShowElementBase()
      {
      lb.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void LyricsView::setElement(Element* e)
      {
      Lyrics* l = (Lyrics*)e;
      ShowElementBase::setElement(e);

      lb.row->setValue(l->no());
      lb.endTick->setValue(l->endTick());
      lb.syllabic->setCurrentIndex(l->syllabic());
      }

//---------------------------------------------------------
//   backClicked
//---------------------------------------------------------

void Debugger::backClicked()
      {
      if (backStack.isEmpty())
            return;
      forwardStack.push(curElement);
      forward->setEnabled(true);
      updateElement(backStack.pop());
      back->setEnabled(!backStack.isEmpty());
      }

//---------------------------------------------------------
//   forwardClicked
//---------------------------------------------------------

void Debugger::forwardClicked()
      {
      if (forwardStack.isEmpty())
            return;
      backStack.push(curElement);
      back->setEnabled(true);
      updateElement(forwardStack.pop());
      forward->setEnabled(!forwardStack.isEmpty());
      }

//---------------------------------------------------------
//   reloadClicked
//---------------------------------------------------------

void Debugger::reloadClicked()
      {
      Element* e = curElement;
	updateList(cs);
	if (e)
	      updateElement(e);
      }

//---------------------------------------------------------
//   BeamView
//---------------------------------------------------------

BeamView::BeamView()
   : ShowElementBase()
      {
      bb.setupUi(addWidget());
      connect(bb.elements, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void BeamView::setElement(Element* e)
      {
      Beam* b = (Beam*)e;
      ShowElementBase::setElement(e);

      bb.up->setValue(b->up());
      bb.elements->clear();
      foreach (ChordRest* cr, b->elements()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)cr, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)cr));
            item->setText(1, cr->name());
            item->setText(2, QString("%1").arg(cr->segment()->tick()));
            bb.elements->addTopLevelItem(item);
            }
      bb.grow1->setValue(b->growLeft());
      bb.grow2->setValue(b->growRight());
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void BeamView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   TremoloView
//---------------------------------------------------------

TremoloView::TremoloView()
   : ShowElementBase()
      {
      tb.setupUi(addWidget());
      connect(tb.firstChord, SIGNAL(clicked()), SLOT(chord1Clicked()));
      connect(tb.secondChord, SIGNAL(clicked()), SLOT(chord2Clicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TremoloView::setElement(Element* e)
      {
      Tremolo* b = static_cast<Tremolo*>(e);
      ShowElementBase::setElement(e);

      tb.firstChord->setEnabled(b->chord1());
      tb.secondChord->setEnabled(b->chord2());
      }

//---------------------------------------------------------
//   chord1Clicked
//---------------------------------------------------------

void TremoloView::chord1Clicked()
      {
      emit elementChanged(static_cast<Tremolo*>(element())->chord1());
      }

//---------------------------------------------------------
//   chord2Clicked
//---------------------------------------------------------

void TremoloView::chord2Clicked()
      {
      emit elementChanged(static_cast<Tremolo*>(element())->chord2());
      }

//---------------------------------------------------------
//   OttavaView
//---------------------------------------------------------

OttavaView::OttavaView()
   : TextLineView()
      {
//      ob.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void OttavaView::setElement(Element* e)
      {
//      Ottava* o = static_cast<Ottava*>(e);
      TextLineView::setElement(e);
      }

//---------------------------------------------------------
//   SlurSegmentView
//---------------------------------------------------------

SlurSegmentView::SlurSegmentView()
   : ShowElementBase()
      {
      ss.setupUi(addWidget());
      connect(ss.slurTie, SIGNAL(clicked()), SLOT(slurTieClicked()));
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void SlurSegmentView::slurTieClicked()
      {
      emit elementChanged(static_cast<SlurSegment*>(element())->slurTie());
      }

//---------------------------------------------------------
//   SlurSegmentView
//---------------------------------------------------------

void SlurSegmentView::setElement(Element* e)
      {
      SlurSegment* s = static_cast<SlurSegment*>(e);
      ShowElementBase::setElement(e);
      ss.up1px->setValue(s->getUps(GRIP_START)->p.x());
      ss.up1py->setValue(s->getUps(GRIP_START)->p.y());
      ss.up1ox->setValue(s->getUps(GRIP_START)->off.x());
      ss.up1oy->setValue(s->getUps(GRIP_START)->off.y());

      ss.up2px->setValue(s->getUps(GRIP_BEZIER1)->p.x());
      ss.up2py->setValue(s->getUps(GRIP_BEZIER1)->p.y());
      ss.up2ox->setValue(s->getUps(GRIP_BEZIER1)->off.x());
      ss.up2oy->setValue(s->getUps(GRIP_BEZIER1)->off.y());

      ss.up3px->setValue(s->getUps(GRIP_BEZIER2)->p.x());
      ss.up3py->setValue(s->getUps(GRIP_BEZIER2)->p.y());
      ss.up3ox->setValue(s->getUps(GRIP_BEZIER2)->off.x());
      ss.up3oy->setValue(s->getUps(GRIP_BEZIER2)->off.y());

      ss.up4px->setValue(s->getUps(GRIP_END)->p.x());
      ss.up4py->setValue(s->getUps(GRIP_END)->p.y());
      ss.up4ox->setValue(s->getUps(GRIP_END)->off.x());
      ss.up4oy->setValue(s->getUps(GRIP_END)->off.y());

      }

//---------------------------------------------------------
//   AccidentalView
//---------------------------------------------------------

AccidentalView::AccidentalView()
   : ShowElementBase()
      {
      acc.setupUi(addWidget());
      }

//---------------------------------------------------------
//   AccidentalView
//---------------------------------------------------------

void AccidentalView::setElement(Element* e)
      {
      Accidental* s = static_cast<Accidental*>(e);
      ShowElementBase::setElement(e);

      acc.hasBracket->setChecked(s->hasBracket());
      acc.accAuto->setChecked(s->role() == Accidental::ACC_AUTO);
      acc.accUser->setChecked(s->role() == Accidental::ACC_USER);
      acc.small->setChecked(s->small());
      }

//---------------------------------------------------------
//   ClefView
//---------------------------------------------------------

ClefView::ClefView()
   : ShowElementBase()
      {
      clef.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ClefView::setElement(Element* e)
      {
      Clef* c = static_cast<Clef*>(e);
      ShowElementBase::setElement(e);

      clef.clefType->setValue(int(c->clefType()));
      clef.showCourtesy->setChecked(c->showCourtesy());
      clef.small->setChecked(c->small());

      clef.concertClef->setValue(int(c->concertClef()));
      clef.transposingClef->setValue(int(c->transposingClef()));
      }

//---------------------------------------------------------
//   ArticulationView
//---------------------------------------------------------

ArticulationView::ArticulationView()
   : ShowElementBase()
      {
      articulation.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ArticulationView::setElement(Element* e)
      {
      Articulation* a = static_cast<Articulation*>(e);
      ShowElementBase::setElement(e);

      articulation.direction->setCurrentIndex(int(a->direction()));
      articulation.up->setChecked(a->up());
      articulation.anchor->setCurrentIndex(int(a->anchor()));
      articulation.channelName->setText(a->channelName());
      }

//---------------------------------------------------------
//   KeySigView
//---------------------------------------------------------

KeySigView::KeySigView()
   : ShowElementBase()
      {
      keysig.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void KeySigView::setElement(Element* e)
      {
      KeySig* ks = static_cast<KeySig*>(e);
      ShowElementBase::setElement(e);

      keysig.showCourtesySig->setChecked(ks->showCourtesy());
      keysig.showNaturals->setChecked(ks->showNaturals());
      keysig.accidentalType->setValue(ks->keySigEvent().accidentalType());
      keysig.naturalType->setValue(ks->keySigEvent().naturalType());
      keysig.customType->setValue(ks->keySigEvent().customType());
      keysig.custom->setChecked(ks->keySigEvent().custom());
      keysig.invalid->setChecked(ks->keySigEvent().invalid());
      }

//---------------------------------------------------------
//   StemView
//---------------------------------------------------------

StemView::StemView()
   : ShowElementBase()
      {
      stem.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void StemView::setElement(Element* e)
      {
      Stem* s = static_cast<Stem*>(e);
      ShowElementBase::setElement(e);

      stem.len->setValue(s->len());
      stem.userLen->setValue(s->userLen());
      }

//---------------------------------------------------------
//   BoxView
//---------------------------------------------------------

BoxView::BoxView()
   : ShowElementBase()
      {
      box.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void BoxView::setElement(Element* e)
      {
      Box* b = static_cast<Box*>(e);
      ShowElementBase::setElement(e);

      box.width->setValue(b->boxWidth().val());
      box.height->setValue(b->boxHeight().val());
      box.topGap->setValue(b->topGap());
      box.bottomGap->setValue(b->bottomGap());
      box.topMargin->setValue(b->topMargin());
      box.bottomMargin->setValue(b->bottomMargin());
      box.leftMargin->setValue(b->leftMargin());
      box.rightMargin->setValue(b->rightMargin());
      }

//---------------------------------------------------------
//   TextLineView
//---------------------------------------------------------

TextLineView::TextLineView()
   : SpannerView()
      {
      // SLineBase
      lb.setupUi(addWidget());

      // TextLineBase
      tlb.setupUi(addWidget());

      connect(tlb.beginText,    SIGNAL(clicked()), SLOT(beginTextClicked()));
      connect(tlb.continueText, SIGNAL(clicked()), SLOT(continueTextClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TextLineView::setElement(Element* e)
      {
      Volta* volta = (Volta*)e;
      SpannerView::setElement(e);

      tlb.lineWidth->setValue(volta->lineWidth().val());
      lb.diagonal->setChecked(volta->diagonal());

      tlb.beginText->setEnabled(volta->beginText());
      tlb.continueText->setEnabled(volta->continueText());
      }

//---------------------------------------------------------
//   beginTextClicked
//---------------------------------------------------------

void TextLineView::beginTextClicked()
      {
      Volta* volta = (Volta*)element();
      emit elementChanged(volta->beginText());
      }

//---------------------------------------------------------
//   continueTextClicked
//---------------------------------------------------------

void TextLineView::continueTextClicked()
      {
      Volta* volta = (Volta*)element();
      emit elementChanged(volta->continueText());
      }


//---------------------------------------------------------
//   TextLineSegmentView
//---------------------------------------------------------

TextLineSegmentView::TextLineSegmentView()
   : ShowElementBase()
      {
      lb.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TextLineSegmentView::setElement(Element* e)
      {
      VoltaSegment* vs = (VoltaSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(vs->spannerSegmentType());
      lb.pos2x->setValue(vs->pos2().x());
      lb.pos2y->setValue(vs->pos2().y());
      lb.offset2x->setValue(vs->userOff2().x());
      lb.offset2y->setValue(vs->userOff2().y());
      connect(lb.line, SIGNAL(clicked()), SLOT(lineClicked()));
      }

//---------------------------------------------------------
//   lineClicked
//---------------------------------------------------------

void TextLineSegmentView::lineClicked()
      {
      Spanner* sp = static_cast<SpannerSegment*>(element())->spanner();
      emit elementChanged(sp);
      }

//---------------------------------------------------------
//   SystemView
//---------------------------------------------------------

SystemView::SystemView()
   : ShowElementBase()
      {
      mb.setupUi(addWidget());
      connect(mb.spanner, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      connect(mb.measureList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(measureClicked(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SystemView::setElement(Element* e)
      {
      System* vs = (System*)e;
      ShowElementBase::setElement(e);
      mb.spanner->clear();
      foreach(const Element* e, vs->spannerSegments()) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, e->name());
            void* p = (void*) e;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(p));
            mb.spanner->addTopLevelItem(item);
            }
      mb.measureList->clear();
      for (MeasureBase* m : vs->measures()) {
            ElementListWidgetItem* item = new ElementListWidgetItem(m);
            mb.measureList->addItem(item);
            }
      }

//---------------------------------------------------------
//   elementClicked
//---------------------------------------------------------

void SystemView::elementClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   measureClicked
//---------------------------------------------------------

void SystemView::measureClicked(QListWidgetItem* i)
      {
      ElementListWidgetItem* item = (ElementListWidgetItem*)i;
      emit elementChanged(item->element());
      }

}


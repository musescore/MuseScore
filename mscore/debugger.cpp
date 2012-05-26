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
            case PAGE:
                  {
                  QString no;
                  no.setNum(((Page*)el)->no()+1);
                  s = "Page-" + no;
                  }
                  break;
            case MEASURE:
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
//      setWindowTitle(tr("MuseScore: Debugger"));

      for (int i = 0; i < MAXTYPE; ++i)
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
      curElement->toDefault();
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

void Debugger::addSymbol(ElementItem* parent, BSymbol* bs)
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
            if (e->type() == HBOX || e->type() == VBOX)
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

      QTreeWidgetItem* li = new QTreeWidgetItem(list, INVALID);
      li->setText(0, "Global");

      int staves = cs->nstaves();
      int tracks = staves * VOICES;
      foreach(Page* page, cs->pages()) {
            ElementItem* pi = new ElementItem(list, page);

            foreach(System* system, *page->systems()) {
                  ElementItem* si = new ElementItem(pi, system);

                  if (system->getBarLine())
                        new ElementItem(si, system->getBarLine());


                  foreach(SysStaff* ss, *system->staves()) {
                        foreach(InstrumentName* in, ss->instrumentNames)
                              new ElementItem(si, in);
                        }

                  foreach (MeasureBase* mb, system->measures()) {
                        ElementItem* mi = new ElementItem(si, mb);

                        addMeasureBaseToList(mi, mb);

                        if (mb->type() != MEASURE)
                              continue;
                        Measure* measure = (Measure*) mb;
                        foreach (MStaff* ms, *measure->staffList()) {
                              if (ms->_vspacerUp)
                                    new ElementItem(si, ms->_vspacerUp);
                              if (ms->_vspacerDown)
                                    new ElementItem(si, ms->_vspacerDown);
                              }

                        foreach(Spanner* s, measure->spannerFor()) {
                              SLine* sl = (SLine*)s;
                              ElementItem* si = new ElementItem(mi, s);
                              foreach(Element* ls, sl->spannerSegments())
                                    new ElementItem(si, ls);
                              }
                        if (measure->noText())
                              new ElementItem(mi, measure->noText());
                        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                              ElementItem* segItem = new ElementItem(mi, segment);
                              for (int track = 0; track < tracks; ++track) {
                                    Element* e = segment->element(track);
                                    if (!e)
                                          continue;
                                    ElementItem* sei = new ElementItem(segItem, e);
                                    if (e->type() == CHORD) {
                                          Chord* chord = (Chord*)e;
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

                                          foreach(Articulation* a, *chord->getArticulations())
                                                new ElementItem(sei, a);
                                          foreach(LedgerLine* h, *chord->ledgerLines())
                                                new ElementItem(sei, h);
                                          foreach(Note* note, chord->notes()) {
                                                ElementItem* ni = new ElementItem(sei, note);
                                                if (note->accidental()) {
                                                      new ElementItem(ni, note->accidental());
                                                      }
                                                foreach(Element* f, note->el()) {
                                                      if (f->type() == SYMBOL || f->type() == IMAGE) {
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
                                                      foreach(Element* el1, tie->spannerSegments())
                                                            new ElementItem(ti, el1);
                                                      }
                                                }
                                          foreach(Element* e, chord->el())
                                                new ElementItem(sei, e);
                                          }
                                    if (e->isChordRest()) {
                                          ChordRest* cr = static_cast<ChordRest*>(e);
                                          if (cr->beam() && cr->beam()->elements().front() == cr)
                                                new ElementItem(sei, cr->beam());
                                          foreach(Spanner* slur, cr->spannerFor()) {
                                                ElementItem* sli = new ElementItem(sei, slur);
                                                foreach(SpannerSegment* ss, slur->spannerSegments()) {
                                                      new ElementItem(sli, ss);
                                                      }
                                                }
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
                              foreach(Spanner* s, segment->spannerFor()) {
                                    SLine* sl = (SLine*)s;
                                    ElementItem* si = new ElementItem(segItem, s);
                                    foreach(Element* ls, sl->spannerSegments())
                                          new ElementItem(si, ls);
                                    }
                              foreach(Element* s, segment->annotations()) {
                                    if (s->type() == SYMBOL || s->type() == IMAGE)
                                          addBSymbol(segItem, static_cast<BSymbol*>(s));
                                    else
                                          new ElementItem(segItem, s);
                                    }
#if 0 // TODO
                              for (int i = 0; i < staves; ++i) {
                                    foreach(Lyrics* l, *(segment->lyricsList(i))) {
                                          if (l)
                                                new ElementItem(segItem, l);
                                          }
                                    }
#endif
                              }
#if 0 // TODOxxx
                        foreach(Tuplet* tuplet, *measure->tuplets()) {
					ElementItem* item = new ElementItem(mi, tuplet);
                              if (tuplet->number())
                                    new ElementItem(item, tuplet->number());
                              }
#endif
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
      if (i->type() == INVALID)
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
                  qDebug("Debugger::Element not found %s %p\n", el->name(), el);
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
                  case PAGE:          ew = new ShowPageWidget;    break;
                  case SYSTEM:        ew = new ShowSystemWidget;  break;
                  case MEASURE:       ew = new MeasureView;       break;
                  case CHORD:         ew = new ShowChordWidget;   break;
                  case NOTE:          ew = new ShowNoteWidget;    break;
                  case REST:          ew = new RestView;          break;
                  case CLEF:          ew = new ClefView;          break;
                  case TIMESIG:       ew = new ShowTimesigWidget; break;
                  case KEYSIG:        ew = new KeySigView;        break;
                  case SEGMENT:       ew = new SegmentView;       break;
                  case HAIRPIN:       ew = new HairpinView;       break;
                  case BAR_LINE:      ew = new BarLineView;       break;
                  case DYNAMIC:       ew = new DynamicView;       break;
                  case TUPLET:        ew = new TupletView;        break;
                  case SLUR:          ew = new SlurView;          break;
                  case TIE:           ew = new TieView;           break;
                  case VOLTA:         ew = new VoltaView;         break;
                  case VOLTA_SEGMENT: ew = new VoltaSegmentView;  break;
                  case LYRICS:        ew = new LyricsView;        break;
                  case BEAM:          ew = new BeamView;          break;
                  case TREMOLO:       ew = new TremoloView;       break;
                  case OTTAVA:        ew = new OttavaView;        break;
                  case SLUR_SEGMENT:  ew = new SlurSegmentView;   break;
                  case ACCIDENTAL:    ew = new AccidentalView;    break;
                  case ARTICULATION:  ew = new ArticulationView;  break;
                  case STEM:          ew = new StemView;          break;
                  case FINGERING:
                  case MARKER:
                  case JUMP:
                  case TEXT:
                  case STAFF_TEXT:
                        ew = new TextView;
                        break;
                  case TRILL_SEGMENT:
                  case HAIRPIN_SEGMENT:
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
      QWidget* page = new QWidget;
      pb.setupUi(page);
      layout->addWidget(page);
      layout->addStretch(10);
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
//   ShowSystemWidget
//---------------------------------------------------------

ShowSystemWidget::ShowSystemWidget()
   : ShowElementBase()
      {
      layout->addStretch(100);
      }

//---------------------------------------------------------
//   MeasureView
//---------------------------------------------------------

MeasureView::MeasureView()
   : ShowElementBase()
      {
      QWidget* seg = new QWidget;
      mb.setupUi(seg);
      layout->addWidget(seg);
      layout->addStretch(10);
      connect(mb.sel, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(elementClicked(QTreeWidgetItem*)));
      connect(mb.nextButton, SIGNAL(clicked()), SLOT(nextClicked()));
      connect(mb.prevButton, SIGNAL(clicked()), SLOT(prevClicked()));
      seg->show();
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
//   setElement
//---------------------------------------------------------

void MeasureView::setElement(Element* e)
      {
      Measure* m = (Measure*)e;
      ShowElementBase::setElement(e);

      mb.segments->setValue(m->size());
      mb.staves->setValue(m->staffList()->size());
//TODOxxx      mb.tuplets->setValue(m->tuplets()->size());
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
      mb.multiMeasure->setValue(m->multiMeasure());
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
      QWidget* seg = new QWidget;
      sb.setupUi(seg);
      layout->addWidget(seg);
      layout->addStretch(10);
      sb.segmentType->clear();
      sb.segmentType->addItem("SegClef",               0x1);
      sb.segmentType->addItem("SegKeySig",             0x2);
      sb.segmentType->addItem("SegTimeSig",            0x4);
      sb.segmentType->addItem("SegStartRepeatBarLine", 0x8);
      sb.segmentType->addItem("SegBarLine",            0x10);
      sb.segmentType->addItem("SegGrace",              0x20);
      sb.segmentType->addItem("SegChordRest",          0x40);
      sb.segmentType->addItem("SegBreath",             0x80);
      sb.segmentType->addItem("SegEndBarLine",         0x100);
      sb.segmentType->addItem("SegTimeSigAnnounce",    0x200);
      sb.segmentType->addItem("SegKeySigAnnounce",     0x400);
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
      int st = s->subtype();
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
      foreach(Spanner* sp, s->spannerFor()) {
            QString s;
            s.setNum(long(sp), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)sp));
            sb.spannerFor->addItem(item);
            }
      foreach(Spanner* sp, s->spannerBack()) {
            QString s;
            s.setNum(long(sp), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)sp));
            sb.spannerBack->addItem(item);
            }
      foreach(Element* sp, s->annotations()) {
            QString s;
            s.setNum(long(sp), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)sp));
            sb.annotations->addItem(item);
            }
      }

//---------------------------------------------------------
//   ShowChordWidget
//---------------------------------------------------------

ShowChordWidget::ShowChordWidget()
   : ShowElementBase()
      {
      // chord rest
      QWidget* chr = new QWidget;
      crb.setupUi(chr);
      layout->addWidget(chr);
      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.upFlag,   SIGNAL(toggled(bool)), SLOT(upChanged(bool)));
      connect(crb.beamMode, SIGNAL(activated(int)), SLOT(beamModeChanged(int)));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurFor, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurBack, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.lyrics, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      // chord
      QWidget* ch = new QWidget;
      cb.setupUi(ch);
      layout->addWidget(ch);
      layout->addStretch(100);
      connect(cb.hookButton, SIGNAL(clicked()), SLOT(hookClicked()));
      connect(cb.stemButton, SIGNAL(clicked()), SLOT(stemClicked()));
      connect(cb.stemSlashButton, SIGNAL(clicked()), SLOT(stemSlashClicked()));
      connect(cb.arpeggioButton, SIGNAL(clicked()), SLOT(arpeggioClicked()));
      connect(cb.tremoloButton, SIGNAL(clicked()), SLOT(tremoloClicked()));
      connect(cb.glissandoButton, SIGNAL(clicked()), SLOT(glissandoClicked()));

      connect(cb.stemDirection, SIGNAL(activated(int)), SLOT(directionChanged(int)));
      connect(cb.helplineList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(cb.notes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      crb.beamMode->addItem("auto");
      crb.beamMode->addItem("beam begin");
      crb.beamMode->addItem("beam mid");
      crb.beamMode->addItem("beam end");
      crb.beamMode->addItem("no beam");
      crb.beamMode->addItem("begin 1/32");

      cb.stemDirection->addItem("Auto", 0);
      cb.stemDirection->addItem("Up",   1);
      cb.stemDirection->addItem("Down", 2);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ShowChordWidget::setElement(Element* e)
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
      foreach(Articulation* a, *chord->getArticulations()) {
            QString s;
            s.setNum(long(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)a));
            crb.attributes->addItem(item);
            }
      crb.slurFor->clear();
      foreach(Spanner* slur, chord->spannerFor()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)slur));
            crb.slurFor->addItem(item);
            }
      crb.slurBack->clear();
      foreach(Spanner* slur, chord->spannerBack()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)slur));
            crb.slurBack->addItem(item);
            }
      crb.lyrics->clear();
      foreach(Lyrics* lyrics, chord->lyricsList()) {
            QString s;
            s.setNum(long(lyrics), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)lyrics));
            crb.slurBack->addItem(item);
            }

      cb.helplineList->clear();
      foreach(LedgerLine* h, *chord->ledgerLines()) {
            QString s;
            s.setNum(long(h), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)h));
            cb.helplineList->addItem(item);
            }
      cb.notes->clear();
      foreach(Note* n, chord->notes()) {
            QString s;
            s.setNum(long(n), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)n));
            cb.notes->addItem(item);
            }
      }

//---------------------------------------------------------
//   hookClicked
//---------------------------------------------------------

void ShowChordWidget::hookClicked()
      {
      emit elementChanged(((Chord*)element())->hook());
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void ShowChordWidget::stemClicked()
      {
      emit elementChanged(((Chord*)element())->stem());
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void ShowChordWidget::beamClicked()
      {
      emit elementChanged(((Chord*)element())->beam());
      }

//---------------------------------------------------------
//   tupletClicked
//---------------------------------------------------------

void ShowChordWidget::tupletClicked()
      {
      emit elementChanged(((Chord*)element())->tuplet());
      }

void ShowChordWidget::stemSlashClicked()
      {
      emit elementChanged(((Chord*)element())->stemSlash());
      }

void ShowChordWidget::arpeggioClicked()
      {
      emit elementChanged(((Chord*)element())->arpeggio());
      }

void ShowChordWidget::tremoloClicked()
      {
      emit elementChanged(((Chord*)element())->tremolo());
      }

void ShowChordWidget::glissandoClicked()
      {
      emit elementChanged(((Chord*)element())->glissando());
      }

//---------------------------------------------------------
//   upChanged
//---------------------------------------------------------

void ShowChordWidget::upChanged(bool val)
      {
      ((Chord*)element())->setUp(val);
      }

//---------------------------------------------------------
//   beamModeChanged
//---------------------------------------------------------

void ShowChordWidget::beamModeChanged(int n)
      {
      ((Chord*)element())->setBeamMode(BeamMode(n));
      element()->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   directionChanged
//---------------------------------------------------------

void ShowChordWidget::directionChanged(int val)
      {
      ((Chord*)element())->setStemDirection(Direction(val));
      }

//---------------------------------------------------------
//   ShowNoteWidget
//---------------------------------------------------------

ShowNoteWidget::ShowNoteWidget()
   : ShowElementBase()
      {
      QWidget* note = new QWidget;
      nb.setupUi(note);
      layout->addWidget(note);
      layout->addStretch(10);

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
      nb.ppitch->setValue(note->ppitch());
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

      nb.onTimeOffset->setValue(note->onTimeOffset());
      nb.offTimeOffset->setValue(note->offTimeOffset());
      nb.onTimeUserOffset->setValue(note->onTimeUserOffset());
      nb.offTimeUserOffset->setValue(note->offTimeUserOffset());

      note->el().clear();     // ??
      foreach(Element* text, note->el()) {
            QString s;
            s.setNum(long(text), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)text));
            nb.fingering->addItem(item);
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
      QWidget* chr = new QWidget;
      crb.setupUi(chr);
      layout->addWidget(chr);
      crb.beamMode->addItem("auto");
      crb.beamMode->addItem("beam begin");
      crb.beamMode->addItem("beam mid");
      crb.beamMode->addItem("beam end");
      crb.beamMode->addItem("no beam");
      crb.beamMode->addItem("begin 1/32");

      QWidget* rw = new QWidget;
      rb.setupUi(rw);
      layout->addWidget(rw);

      connect(crb.beamButton, SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.attributes, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurFor, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.slurBack, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.lyrics, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));

      layout->addStretch(100);
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

      crb.slurFor->clear();
      foreach(Spanner* slur, rest->spannerFor()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)slur));
            crb.slurFor->addItem(item);
            }
      crb.slurBack->clear();
      foreach(Spanner* slur, rest->spannerBack()) {
            QString s;
            s.setNum(long(slur), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)slur));
            crb.slurBack->addItem(item);
            }
      crb.attributes->clear();
      foreach(Articulation* a, *rest->getArticulations()) {
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
            crb.slurBack->addItem(item);
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
      QWidget* page = new QWidget;
      tb.setupUi(page);
      layout->addWidget(page);
      layout->addStretch(10);
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
      tb.text->setPlainText(te->getText());
      tb.xoffset->setValue(te->xoff());
      tb.yoffset->setValue(te->yoff());
      tb.rxoffset->setValue(te->reloff().x());
      tb.ryoffset->setValue(te->reloff().y());
      tb.offsetType->setCurrentIndex(int(te->offsetType()));
      tb.textStyle->setCurrentIndex(te->textStyleType());
      tb.styled->setChecked(te->styled());
      }

//---------------------------------------------------------
//   HairpinView
//---------------------------------------------------------

HairpinView::HairpinView()
   : ShowElementBase()
      {
      QWidget* spanner = new QWidget;
      sp.setupUi(spanner);
      layout->addWidget(spanner);

      QWidget* line = new QWidget;
      sl.setupUi(line);
      layout->addWidget(line);

      QWidget* hairpin = new QWidget;
      hp.setupUi(hairpin);
      layout->addWidget(hairpin);

      layout->addStretch(10);
      connect(sp.startElement,   SIGNAL(clicked()), SLOT(startClicked()));
      connect(sp.endElement,     SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void HairpinView::setElement(Element* e)
      {
      Hairpin* hairpin = (Hairpin*)e;
      ShowElementBase::setElement(e);
      sl.diagonal->setChecked(hairpin->diagonal());
      sp.startElement->setEnabled(hairpin->startElement() != 0);
      sp.endElement->setEnabled(hairpin->endElement() != 0);
      hp.veloChange->setValue(hairpin->veloChange());
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void HairpinView::startClicked()
      {
      emit elementChanged(static_cast<Spanner*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void HairpinView::endClicked()
      {
      emit elementChanged(static_cast<Spanner*>(element())->endElement());
      }

//---------------------------------------------------------
//   BarLineView
//---------------------------------------------------------

BarLineView::BarLineView()
   : ShowElementBase()
      {
      QWidget* barline = new QWidget;
      bl.setupUi(barline);
      layout->addWidget(barline);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void BarLineView::setElement(Element* e)
      {
      BarLine* barline = (BarLine*)e;
      ShowElementBase::setElement(e);
      bl.subType->setValue(barline->subtype());
      bl.span->setValue(barline->span());
      }

//---------------------------------------------------------
//   DynamicView
//---------------------------------------------------------

DynamicView::DynamicView()
   : ShowElementBase()
      {
      QWidget* tw = new QWidget;
      tb.setupUi(tw);
      layout->addWidget(tw);

      QWidget* dynamic = new QWidget;
      bl.setupUi(dynamic);
      layout->addWidget(dynamic);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void DynamicView::setElement(Element* e)
      {
      Dynamic* dynamic = (Dynamic*)e;

//      tb.style->clear();
//      foreach (TextStyle* s, dynamic->score()->textStyles())
//            tb.style->addItem(s->name);
      tb.text->setText(dynamic->getText());
//      tb.xoffset->setValue(dynamic->styleOffset().x());
//      tb.yoffset->setValue(dynamic->styleOffset().y());

      ShowElementBase::setElement(e);
      bl.subType->setValue(dynamic->subtype());
      }

//---------------------------------------------------------
//   TupletView
//---------------------------------------------------------

TupletView::TupletView()
   : ShowElementBase()
      {
      QWidget* tw = new QWidget;
      tb.setupUi(tw);
      layout->addWidget(tw);
      layout->addStretch(10);

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
      QWidget* elemView = new QWidget;
      eb.setupUi(elemView);
      layout = new QVBoxLayout;
      setLayout(layout);
      layout->addWidget(elemView);
      connect(eb.parentButton,   SIGNAL(clicked()), SLOT(parentClicked()));
      connect(eb.offsetx,        SIGNAL(valueChanged(double)), SLOT(offsetxChanged(double)));
      connect(eb.offsety,        SIGNAL(valueChanged(double)), SLOT(offsetyChanged(double)));
      connect(eb.selected,       SIGNAL(clicked(bool)), SLOT(selectedClicked(bool)));
      connect(eb.visible,        SIGNAL(clicked(bool)), SLOT(visibleClicked(bool)));
      connect(eb.link1,          SIGNAL(clicked()), SLOT(linkClicked()));
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
//   ShowElementBase
//---------------------------------------------------------

void ShowElementBase::setElement(Element* e)
      {
      el = e;

      eb.address->setText(QString("%1").arg((unsigned long)e, 0, 16));
      eb.score->setText(QString("%1").arg((unsigned long)(e->score()), 0, 16));

//      eb.subtype->setValue(e->subtype());
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
      qDebug("linkClicked\n");
      foreach(Element* e, *el->links()) {
            qDebug("  element <%p> <%p>\n", e->score(), e);
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
//   SlurView
//---------------------------------------------------------

SlurView::SlurView()
   : ShowElementBase()
      {
      QWidget* slurTie = new QWidget;
      st.setupUi(slurTie);
      QWidget* slur = new QWidget;
      sb.setupUi(slur);
      layout->addWidget(slurTie);
      layout->addWidget(slur);
      layout->addStretch(10);
      connect(st.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      connect(st.startElement,   SIGNAL(clicked()), SLOT(startClicked()));
      connect(st.endElement,     SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void SlurView::startClicked()
      {
      emit elementChanged(static_cast<SlurTie*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void SlurView::endClicked()
      {
      emit elementChanged(static_cast<SlurTie*>(element())->endElement());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SlurView::setElement(Element* e)
      {
      Slur* slur = (Slur*)e;
      ShowElementBase::setElement(e);

      st.segments->clear();
      const QList<SpannerSegment*>& el = slur->spannerSegments();
      foreach(const Element* e, el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            st.segments->addTopLevelItem(item);
            }
      st.upFlag->setChecked(slur->up());
      st.direction->setCurrentIndex(slur->slurDirection());
      st.startElement->setEnabled(slur->startElement());
      st.endElement->setEnabled(slur->endElement());

//TODO1      sb.tick2->setValue(slur->tick2());
      sb.staff2->setValue(slur->track2() / VOICES);
      sb.voice2->setValue(slur->track2() % VOICES);
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void SlurView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   segmentClicked
//---------------------------------------------------------

void TieView::segmentClicked(QTreeWidgetItem* item)
      {
      Element* e = (Element*)item->data(0, Qt::UserRole).value<void*>();
      emit elementChanged(e);
      }

//---------------------------------------------------------
//   TieView
//---------------------------------------------------------

TieView::TieView()
   : ShowElementBase()
      {
      QWidget* tie = new QWidget;
      st.setupUi(tie);
      layout->addWidget(tie);
      layout->addStretch(10);
      connect(st.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      connect(st.startElement,   SIGNAL(clicked()), SLOT(startClicked()));
      connect(st.endElement,     SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void TieView::startClicked()
      {
      emit elementChanged(static_cast<SlurTie*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void TieView::endClicked()
      {
      emit elementChanged(static_cast<SlurTie*>(element())->endElement());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TieView::setElement(Element* e)
      {
      Tie* tie = (Tie*)e;
      ShowElementBase::setElement(e);

      st.segments->clear();
      const QList<SpannerSegment*>& el = tie->spannerSegments();
      foreach(const Element* e, el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            st.segments->addTopLevelItem(item);
            }
      st.upFlag->setChecked(tie->up());
      st.direction->setCurrentIndex(tie->slurDirection());
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
      // SLineBase
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);

      // TextLineBase
      w = new QWidget;
      tlb.setupUi(w);
      layout->addWidget(w);

      layout->addStretch(10);
//      connect(lb.segments, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(segmentClicked(QTreeWidgetItem*)));
      connect(tlb.beginText, SIGNAL(clicked()), SLOT(beginTextClicked()));
      connect(tlb.continueText, SIGNAL(clicked()), SLOT(continueTextClicked()));
//      connect(lb.leftElement, SIGNAL(clicked()), SLOT(leftElementClicked()));
//      connect(lb.rightElement, SIGNAL(clicked()), SLOT(rightElementClicked()));
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

/*      lb.segments->clear();
      const QList<SpannerSegment*>& el = volta->spannerSegments();
      foreach(const SpannerSegment* e, el) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, QString("%1").arg((unsigned long)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            lb.segments->addTopLevelItem(item);
            }
      */

      tlb.beginText->setEnabled(volta->beginText());
      tlb.continueText->setEnabled(volta->continueText());
      }

//---------------------------------------------------------
//   leftElementClicked
//---------------------------------------------------------

void VoltaView::leftElementClicked()
      {
      emit elementChanged(static_cast<Volta*>(element())->startElement());
      }

//---------------------------------------------------------
//   rightElementClicked
//---------------------------------------------------------

void VoltaView::rightElementClicked()
      {
      emit elementChanged(static_cast<Volta*>(element())->endElement());
      }

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

VoltaSegmentView::VoltaSegmentView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaSegmentView::setElement(Element* e)
      {
      VoltaSegment* vs = (VoltaSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(vs->subtype());
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
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void LineSegmentView::setElement(Element* e)
      {
      LineSegment* vs = (LineSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(vs->subtype());
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
      QWidget* w = new QWidget;
      lb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
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
      QWidget* w = new QWidget;
      bb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
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
      QWidget* w = new QWidget;
      tb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
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
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      sb.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      connect(sb.startElement, SIGNAL(clicked()), SLOT(startElementClicked()));
      connect(sb.endElement,   SIGNAL(clicked()), SLOT(endElementClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void OttavaView::setElement(Element* e)
      {
      Ottava* o = static_cast<Ottava*>(e);
      ShowElementBase::setElement(e);

      sb.startElement->setEnabled(o->startElement());
      sb.endElement->setEnabled(o->endElement());
      sb.anchor->setCurrentIndex(int(o->anchor()));
      }

//---------------------------------------------------------
//   startElementClicked
//---------------------------------------------------------

void OttavaView::startElementClicked()
      {
      emit elementChanged(static_cast<Ottava*>(element())->startElement());
      }

//---------------------------------------------------------
//   endElementClicked
//---------------------------------------------------------

void OttavaView::endElementClicked()
      {
      emit elementChanged(static_cast<Ottava*>(element())->endElement());
      }

//---------------------------------------------------------
//   SlurSegmentView
//---------------------------------------------------------

SlurSegmentView::SlurSegmentView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      ss.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
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
      QWidget* w = new QWidget;
      acc.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   AccidentalView
//---------------------------------------------------------

void AccidentalView::setElement(Element* e)
      {
      Accidental* s = static_cast<Accidental*>(e);
      ShowElementBase::setElement(e);

      acc.hasBracket->setChecked(s->hasBracket());
      acc.accAuto->setChecked(s->role() == ACC_AUTO);
      acc.accUser->setChecked(s->role() == ACC_USER);
      }

//---------------------------------------------------------
//   ClefView
//---------------------------------------------------------

ClefView::ClefView()
   : ShowElementBase()
      {
      QWidget* w = new QWidget;
      clef.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void ClefView::setElement(Element* e)
      {
      Clef* c = static_cast<Clef*>(e);
      ShowElementBase::setElement(e);

      clef.clefType->setValue(c->clefType());
      clef.showCourtesyClef->setChecked(c->showCourtesyClef());
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
      QWidget* w = new QWidget;
      articulation.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
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
      QWidget* w = new QWidget;
      keysig.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void KeySigView::setElement(Element* e)
      {
      KeySig* ks = static_cast<KeySig*>(e);
      ShowElementBase::setElement(e);

      keysig.showCourtesySig->setChecked(ks->showCourtesySig());
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
      QWidget* w = new QWidget;
      stem.setupUi(w);
      layout->addWidget(w);
      layout->addStretch(10);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void StemView::setElement(Element* e)
      {
      Stem* s = static_cast<Stem*>(e);
      ShowElementBase::setElement(e);

      stem.len->setValue(s->stemLen() - s->userLen());
      stem.userLen->setValue(s->userLen());
      }


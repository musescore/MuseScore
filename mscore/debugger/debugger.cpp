//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: debugger.cpp 5656 2012-05-21 15:36:47Z wschweer $
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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
#include "libmscore/timesig.h"
#include "libmscore/systemdivider.h"

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
   : QTreeWidgetItem(lv, int(e->type()) + int(QTreeWidgetItem::UserType))
      {
      el = e;
      init();
      }

ElementItem::ElementItem(QTreeWidgetItem* ei, Element* e)
   : QTreeWidgetItem(ei, int(e->type()) + int(QTreeWidgetItem::UserType))
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
            case ElementType::PAGE:
                  {
                  QString no;
                  no.setNum(((Page*)el)->no()+1);
                  s = "Page-" + no;
                  }
                  break;
            case ElementType::MEASURE:
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
   : AbstractDialog(parent)
      {
      setObjectName("Debugger");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      for (int i = 0; i < int(ElementType::MAXTYPE); ++i)
            elementViews[i] = 0;
      curElement   = 0;
      cs           = 0;

      connect(list, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(itemClicked(QTreeWidgetItem*,int)));
      connect(list, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(itemClicked(QTreeWidgetItem*, int)));
      connect(list, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));
      connect(list, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(itemExpanded(QTreeWidgetItem*)));

      list->resizeColumnToContents(0);
      readSettings();
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
      if (!curElement)
            return;
      curElement->score()->select(curElement);
      }

//---------------------------------------------------------
//   resetElement
//---------------------------------------------------------

void Debugger::resetElement()
      {
      if (!curElement)
            return;
      curElement->reset();
      layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Debugger::layout()
      {
      if (!curElement)
            return;
      curElement->score()->setLayoutAll();
      curElement->score()->update();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Debugger::writeSettings()
      {
      QSettings settings;
      settings.beginGroup(objectName());
      settings.setValue("splitter", split->saveState());
      settings.endGroup();

      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void Debugger::readSettings()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup(objectName());
            split->restoreState(settings.value("splitter").toByteArray());
            settings.endGroup();
            }

      MuseScore::restoreGeometry(this);
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
      foreach(Element* e, mb->el()) {
            ElementItem* mmi = new ElementItem(mi, e);
            if (e->type() == ElementType::HBOX || e->type() == ElementType::VBOX)
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
      if (chord->stemSlash())
            new ElementItem(sei, chord->stemSlash());
      if (chord->arpeggio())
            new ElementItem(sei, chord->arpeggio());
      if (chord->tremolo() && (chord->tremolo()->chord1() == chord))
            new ElementItem(sei, chord->tremolo());
//      if (chord->glissando())
//            new ElementItem(sei, chord->glissando());

      for (Articulation* a : chord->articulations())
            new ElementItem(sei, a);
      for (LedgerLine* h = chord->ledgerLines(); h; h = h->next())
            new ElementItem(sei, h);
      for (Note* note : chord->notes()) {
            ElementItem* ni = new ElementItem(sei, note);
            if (note->accidental()) {
                  new ElementItem(ni, note->accidental());
                  }
            for (Element* f : note->el()) {
                  if (f->type() == ElementType::SYMBOL || f->type() == ElementType::IMAGE) {
                        BSymbol* bs = static_cast<BSymbol*>(f);
                        addSymbol(ni, bs);
                        }
                  else
                        new ElementItem(ni, f);
                  }
            for (NoteDot* dot : note->dots())
                  new ElementItem(ni, dot);

            if (note->tieFor()) {
                  Tie* tie = note->tieFor();
                  ElementItem* ti = new ElementItem(ni, tie);
                  for (Element* el1 : tie->spannerSegments())
                        new ElementItem(ti, el1);
                  }
            for (Spanner* s : note->spannerFor()) {
                  ElementItem* si = new ElementItem(ni, s);
                  for (Element* ls : s->spannerSegments())
                        new ElementItem(si, ls);
                  }
            }
      for (Element* e : chord->el()) {
            ElementItem* ei = new ElementItem(sei, e);
            if (e->type() == ElementType::SLUR) {
                  Slur* gs = static_cast<Slur*>(e);
                  for (SpannerSegment* sp : gs->spannerSegments())
                        new ElementItem(ei, sp);
                  }
            }
      for (Chord* c : chord->graceNotes()) {
            ElementItem* ssei = new ElementItem(sei, c);
            addChord(ssei, c);
            }

      if (chord->beam() && chord->beam()->elements().front() == chord)
            new ElementItem(sei, chord->beam());
      for (Lyrics* lyrics : chord->lyrics()) {
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
      for (int staffIdx = 0; staffIdx < measure->score()->nstaves(); ++staffIdx) {
            if (measure->vspacerUp(staffIdx))
                  new ElementItem(mi, measure->vspacerUp(staffIdx));
            if (measure->vspacerDown(staffIdx))
                  new ElementItem(mi, measure->vspacerDown(staffIdx));
            if (measure->noText(staffIdx))
                  new ElementItem(mi, measure->noText(staffIdx));
            }
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            ElementItem* segItem = new ElementItem(mi, segment);
            for (int track = 0; track < tracks; ++track) {
                  Element* e = segment->element(track);
                  if (!e)
                        continue;
                  ElementItem* sei = new ElementItem(segItem, e);
                  if (e->isChord())
                        addChord(sei, toChord(e));
                  else if (e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        if (cr->beam() && cr->beam()->elements().front() == cr)
                              new ElementItem(sei, cr->beam());
                        for (Lyrics* lyrics : cr->lyrics()) {
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
                  if (s->type() == ElementType::SYMBOL || s->type() == ElementType::IMAGE)
                        addBSymbol(segItem, static_cast<BSymbol*>(s));
                  else if (s->type() == ElementType::FRET_DIAGRAM) {
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

      if (s->masterScore()->movements()) {
            QTreeWidgetItem* mi = new QTreeWidgetItem(list, int(ElementType::INVALID));
            mi->setText(0, "Movements");
            for (MasterScore* ms : *s->masterScore()->movements()) {
                  QTreeWidgetItem* s = new QTreeWidgetItem(mi, int(ElementType::INVALID));
                  s->setText(0, ms->metaTag("movementTitle"));
                  }
            }


      QTreeWidgetItem* li = new QTreeWidgetItem(list, int(ElementType::INVALID));
      li->setText(0, "Global");
      for (auto i : s->spanner()) {
            ElementItem* it = new ElementItem(li, i.second);
            if (i.second->type() == ElementType::TRILL) {
                  Trill* trill = static_cast<Trill*>(i.second);
                  if (trill->accidental())
                        new ElementItem(it, trill->accidental());
                  }
            }

      foreach (Page* page, cs->pages()) {
            ElementItem* pi = new ElementItem(list, page);

            foreach (System* system, page->systems()) {
                  ElementItem* si = new ElementItem(pi, system);
                  for (Bracket* b : system->brackets())
                        new ElementItem(si, b);
                  if (system->systemDividerLeft())
                        new ElementItem(si, system->systemDividerLeft());
                  if (system->systemDividerRight())
                        new ElementItem(si, system->systemDividerRight());
                  for (SpannerSegment* ss : system->spannerSegments())
                        new ElementItem(si, ss);
                  for (SysStaff* ss : *system->staves()) {
                        for (InstrumentName* in : ss->instrumentNames)
                              new ElementItem(si, in);
                        }

                  for (MeasureBase* mb : system->measures()) {
                        ElementItem* mi = new ElementItem(si, mb);
                        addMeasureBaseToList(mi, mb);

                        if (mb->type() != ElementType::MEASURE)
                              continue;
                        Measure* measure = (Measure*) mb;
                        if (cs->styleB(Sid::concertPitch)) {
                              if (measure->mmRest()) {
                                    ElementItem* mmi = new ElementItem(mi, measure->mmRest());
                                    addMeasure(mmi, measure->mmRest());
                                    }
                              }
                        else {
                              if (measure->isMMRest()) {
                                    Measure* m1 = measure->mmRestFirst();
                                    Measure* m2 = measure->mmRestLast();
                                    for (;;) {
                                          ElementItem* mmi = new ElementItem(mi, m1);
                                          addMeasure(mmi, m1);
                                          if (m1 == m2)
                                                break;
                                          m1 = m1->nextMeasure();
                                          }
                                    }
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
      if (i->type() == int(ElementType::INVALID))
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

      if (cs != el->score())
            updateList(el->score());
      bool found = false;
      for (QTreeWidgetItemIterator it(list); *it; ++it) {
            QTreeWidgetItem* item = *it;
            if (item->type() == QTreeWidgetItem::Type)
                  continue;
            ElementItem* ei = static_cast<ElementItem*>(item);
            if (ei->element() == el) {
                  list->setItemExpanded(item, true);
                  list->setCurrentItem(item);
                  list->scrollToItem(item);
                  found = true;
                  break;
                  }
            }
      if (!found)
            qDebug("Debugger: element not found %s", el->name());

      setWindowTitle(QString("Debugger: ") + el->name());

      ShowElementBase* ew = elementViews[int(el->type())];
      if (ew == 0) {
            switch (el->type()) {
                  case ElementType::PAGE:             ew = new ShowPageWidget;      break;
                  case ElementType::SYSTEM:           ew = new SystemView;          break;
                  case ElementType::MEASURE:          ew = new MeasureView;         break;
                  case ElementType::CHORD:            ew = new ChordDebug;          break;
                  case ElementType::NOTE:             ew = new ShowNoteWidget;      break;
                  case ElementType::REST:             ew = new RestView;            break;
                  case ElementType::CLEF:             ew = new ClefView;            break;
                  case ElementType::TIMESIG:          ew = new TimeSigView;         break;
                  case ElementType::KEYSIG:           ew = new KeySigView;          break;
                  case ElementType::SEGMENT:          ew = new SegmentView;         break;
                  case ElementType::HAIRPIN:          ew = new HairpinView;         break;
                  case ElementType::BAR_LINE:         ew = new BarLineView;         break;
                  case ElementType::DYNAMIC:          ew = new DynamicView;         break;
                  case ElementType::TUPLET:           ew = new TupletView;          break;
                  case ElementType::SLUR:             ew = new SlurTieView;         break;
                  case ElementType::TIE:              ew = new TieView;             break;
                  case ElementType::VOLTA:            ew = new VoltaView;           break;
                  case ElementType::VOLTA_SEGMENT:    ew = new VoltaSegmentView;    break;
                  case ElementType::PEDAL:
                  case ElementType::LET_RING:
                  case ElementType::VIBRATO:
                  case ElementType::TEXTLINE:         ew = new TextLineView;        break;
                  case ElementType::PEDAL_SEGMENT:
                  case ElementType::TEXTLINE_SEGMENT: ew = new TextLineSegmentView; break;
                  case ElementType::LYRICS:           ew = new LyricsView;          break;
                  case ElementType::BEAM:             ew = new BeamView;            break;
                  case ElementType::TREMOLO:          ew = new TremoloView;         break;
                  case ElementType::OTTAVA:           ew = new OttavaView;          break;
                  case ElementType::LET_RING_SEGMENT:
                  case ElementType::OTTAVA_SEGMENT:   ew = new TextLineSegmentView; break;
                  case ElementType::SLUR_SEGMENT:     ew = new SlurSegmentView;     break;
                  case ElementType::TIE_SEGMENT:      ew = new TieSegmentView;     break;
                  case ElementType::ACCIDENTAL:       ew = new AccidentalView;      break;
                  case ElementType::ARTICULATION:     ew = new ArticulationView;    break;
                  case ElementType::STEM:             ew = new StemView;            break;
                  case ElementType::VBOX:
                  case ElementType::HBOX:
                  case ElementType::FBOX:
                  case ElementType::TBOX:             ew = new BoxView;             break;
                  case ElementType::TRILL:            ew = new SpannerView;         break;

                  case ElementType::INSTRUMENT_NAME:
                  case ElementType::FINGERING:
                  case ElementType::MARKER:
                  case ElementType::JUMP:
                  case ElementType::TEXT:
                  case ElementType::STAFF_TEXT:
                  case ElementType::REHEARSAL_MARK:
                        ew = new TextView;
                        break;
                  case ElementType::HARMONY:
                        ew = new HarmonyView;
                        break;
                  case ElementType::TRILL_SEGMENT:
                  case ElementType::VIBRATO_SEGMENT:
                  case ElementType::HAIRPIN_SEGMENT:
                        ew = new LineSegmentView; break;
                        break;
                  default:
                        ew = new ElementView;
                        break;
                  }
            stack->addWidget(ew);
            connect(ew,  SIGNAL(elementChanged(Element*)), SLOT(setElement(Element*)));
            elementViews[int(el->type())] = ew;
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
//      mb.staves->setValue(m->mstaves().size());
      mb.measureNo->setValue(m->no());
      mb.noOffset->setValue(m->noOffset());
      mb.stretch->setValue(m->userStretch());
      mb.lineBreak->setChecked(m->lineBreak());
      mb.pageBreak->setChecked(m->pageBreak());
      mb.sectionBreak->setChecked(m->sectionBreak());
      mb.irregular->setChecked(m->irregular());
      mb.repeatCount->setValue(m->repeatCount());
      mb.breakMultiMeasureRest->setChecked(m->breakMultiMeasureRest());
      mb.mmRestCount->setValue(m->mmRestCount());
      mb.timesig->setText(m->timesig().print());
      mb.len->setText(m->len().print());
      mb.tick->setValue(m->tick());
      mb.startRepeat->setChecked(m->repeatStart());
      mb.endRepeat->setChecked(m->repeatEnd());
      mb.hasSystemHeader->setChecked(m->header());
      mb.hasSystemTrailer->setChecked(m->trailer());
      mb.hasCourtesyKeySig->setChecked(m->hasCourtesyKeySig());
      mb.hasVoices->setChecked(m->hasVoices(0));
      mb.sel->clear();
      foreach(const Element* e, m->el()) {
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
      connect(sb.lyrics,      SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(sb.spannerFor,  SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
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
      int tick = s->tick();
      TimeSigMap* sm = s->score()->sigmap();

      int bar, beat, ticks;
      sm->tickValues(tick, &bar, &beat, &ticks);
      sb.bar->setValue(bar);
      sb.beat->setValue(beat);
      sb.ticks->setValue(ticks);
      sb.tick->setValue(s->tick());
      sb.rtick->setValue(s->rtick());
      sb.ticks2->setValue(s->ticks());
      sb.segmentType->setText(s->subTypeName());
      sb.lyrics->clear();

//      Score* cs = e->score();
#if 0 // TODO
      for (int i = 0; i < cs->nstaves(); ++i) {
            const LyricsList* ll = s->lyrics(i);
            if (ll) {
                  foreach(Lyrics* l, *ll) {
                        QString s;
                        s.setNum(qptrdiff(l), 16);
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
            QListWidgetItem* item = new QListWidgetItem(QString("%1 %2").arg(qptrdiff(sp), 8, 16).arg(sp->name()));
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
//      connect(cb.glissandoButton, SIGNAL(clicked()), SLOT(glissandoClicked()));

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

      cb.hookButton->setEnabled(chord->hook());
      cb.stemButton->setEnabled(chord->stem());
      cb.stemSlashButton->setEnabled(chord->stemSlash());
      cb.arpeggioButton->setEnabled(chord->arpeggio());
      cb.tremoloButton->setEnabled(chord->tremolo());

//      cb.glissandoButton->setEnabled(chord->glissando());
//      cb.glissandoButton->setEnabled(false);
      cb.graceNote->setChecked(chord->noteType() != NoteType::NORMAL);
      cb.userPlayEvents->setChecked(chord->playEventType() != PlayEventType::Auto);
      cb.endsGlissando->setChecked(chord->endsGlissando());
      cb.stemDirection->setCurrentIndex(int(chord->stemDirection()));

      crb.attributes->clear();
      foreach(Articulation* a, chord->articulations()) {
            QString s;
            s.setNum(qptrdiff(a), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)a));
            crb.attributes->addItem(item);
            }
      crb.lyrics->clear();
      for (Lyrics* lyrics : chord->lyrics()) {
            QString s;
            s.setNum(qptrdiff(lyrics), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)lyrics));
            crb.lyrics->addItem(item);
            }
      cb.notes->clear();
      foreach(Element* n, chord->notes()) {
            QString s;
            s.setNum(qptrdiff(n), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)n));
            cb.notes->addItem(item);
            }
      cb.helplineList->clear();
      for (LedgerLine* h = chord->ledgerLines(); h; h = h->next()) {
            QString s;
            s.setNum(qptrdiff(h), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)h));
            cb.helplineList->addItem(item);
            }
      cb.graceChords1->clear();
      for (Element* c : chord->graceNotes()) {
            QString s;
            s.setNum(qptrdiff(c), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)c));
            cb.graceChords1->addItem(item);
            }
      cb.elements->clear();
      for (Element* c : chord->el()) {
            QString s;
            s.setNum(qptrdiff(c), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)c));
            cb.elements->addItem(item);
            }
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
/*
void ChordDebug::glissandoClicked()
      {
      emit elementChanged(((Chord*)element())->glissando());
      }
*/
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
      ((Chord*)element())->setBeamMode(Beam::Mode(n));
      element()->score()->setLayoutAll();
      }

//---------------------------------------------------------
//   directionChanged
//---------------------------------------------------------

void ChordDebug::directionChanged(int val)
      {
      ((Chord*)element())->setStemDirection(Direction(val));
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
      nb.epitch->setValue(note->epitch());
      nb.tuning->setValue(note->tuning());
      nb.line->setValue(note->line());
      nb.string->setValue(note->string());
      nb.fret->setValue(note->fret());
      nb.mirror->setChecked(note->mirror());
      nb.tpc1->setValue(note->tpc1());
      nb.tpc2->setValue(note->tpc2());
      nb.headGroup->setValue(int(note->headGroup()));
      nb.hidden->setChecked(note->hidden());
      nb.subchannel->setValue(note->subchannel());

      nb.tieFor->setEnabled(note->tieFor());
      nb.tieBack->setEnabled(note->tieBack());
      nb.accidental->setEnabled(note->accidental());
      nb.dot1->setEnabled(note->dots().size() > 0);
      nb.dot2->setEnabled(note->dots().size() > 1);
      nb.dot3->setEnabled(note->dots().size() > 2);

      nb.fingering->clear();
      for (Element* text : note->el()) {
            QString s;
            s.setNum(qptrdiff(text), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)text));
            nb.fingering->addItem(item);
            }
      nb.noteEvents->clear();
      for (const NoteEvent& e : note->playEvents()) {
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
      // chord rest
      crb.setupUi(addWidget());
      crb.beamMode->addItem("auto");
      crb.beamMode->addItem("beam begin");
      crb.beamMode->addItem("beam mid");
      crb.beamMode->addItem("beam end");
      crb.beamMode->addItem("no beam");
      crb.beamMode->addItem("begin 1/32");

      rb.setupUi(addWidget());

      connect(crb.beamButton,   SIGNAL(clicked()), SLOT(beamClicked()));
      connect(crb.tupletButton, SIGNAL(clicked()), SLOT(tupletClicked()));
      connect(crb.attributes,   SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      connect(crb.lyrics,       SIGNAL(itemClicked(QListWidgetItem*)), SLOT(gotoElement(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void RestView::setElement(Element* e)
      {
      Rest* rest = toRest(e);
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

      crb.lyrics->clear();
      for (Lyrics* lyrics : rest->lyrics()) {
            QString s;
            s.setNum(qptrdiff(lyrics), 16);
            QListWidgetItem* item = new QListWidgetItem(s);
            item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)lyrics));
            crb.lyrics->addItem(item);
            }

      rb.sym->setValue(int(rest->sym()));
      rb.dotline->setValue(rest->getDotline());
      rb.mmWidth->setValue((rest->measure() && rest->measure()->isMMRest()) ? rest->mmWidth() : 0.0);
      rb.gap->setChecked(rest->isGap());
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
//   TimesigView
//---------------------------------------------------------

TimeSigView::TimeSigView()
   : ShowElementBase()
      {
      tb.setupUi(addWidget());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void TimeSigView::setElement(Element* e)
      {
      TimeSig* tsig = static_cast<TimeSig*>(e);
      ShowElementBase::setElement(e);
      tb.numeratorString->setText(tsig->numeratorString());
      tb.denominatorString->setText(tsig->denominatorString());
      tb.numerator->setValue(tsig->sig().numerator());
      tb.denominator->setValue(tsig->sig().denominator());
      tb.numeratorStretch->setValue(tsig->stretch().numerator());
      tb.denominatorStretch->setValue(tsig->stretch().denominator());
      tb.showCourtesySig->setChecked(tsig->showCourtesySig());
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
      Text* te = static_cast<Text*>(e);

      tb.subStyle->setText(subStyleName(te->subStyleId()));

      ShowElementBase::setElement(e);
      tb.text->setPlainText(te->xmlText());
      tb.xoffset->setValue(te->offset().x());
      tb.yoffset->setValue(te->offset().y());
      tb.offsetType->setCurrentIndex(int(te->offsetType()));
      tb.layoutToParentWidth->setChecked(te->layoutToParentWidth());
      Align a = te->align();
      const char* h;
      if (a & Align::HCENTER)
            h = "center";
      else if (a & Align::RIGHT)
            h = "right";
      else
            h = "left";
      tb.halign->setText(h);
      const char* v;
      if (a & Align::BOTTOM)
            v = "bottom";
      else if (a & Align::VCENTER)
            v = "center";
      else if (a & Align::BASELINE)
            v = "baseline";
      else
            v = "top";
      tb.valign->setText(v);
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

//      tb.textStyle->clear();
//      for (int i = 0; i < int(TextStyleType::TEXT_STYLES); ++i)
//            tb.textStyle->addItem(e->score()->textStyle(TextStyleType(i)).name());

//      const TextStyle& ts = harmony->textStyle();
      ShowElementBase::setElement(e);
      tb.text->setPlainText(harmony->xmlText());
//      tb.xoffset->setValue(ts.offset().x());
//      tb.yoffset->setValue(ts.offset().y());
//      tb.offsetType->setCurrentIndex(int(ts.offsetType()));
//TODO      tb.textStyle->setCurrentIndex(ts.textStyleType());
      tb.layoutToParentWidth->setChecked(harmony->layoutToParentWidth());

      hb.tbboxx->setValue(harmony->bboxtight().x());
      hb.tbboxy->setValue(harmony->bboxtight().y());
      hb.tbboxw->setValue(harmony->bboxtight().width());
      hb.tbboxh->setValue(harmony->bboxtight().height());
      hb.leftParen->setChecked(harmony->leftParen());
      hb.rightParen->setChecked(harmony->rightParen());
      hb.rootTpc->setValue(harmony->rootTpc());
      if (harmony->rootTpc() == Tpc::TPC_INVALID)
            hb.rootName->setText("");
      else
            hb.rootName->setText(harmony->rootName());
      hb.bassTpc->setValue(harmony->baseTpc());
      if (harmony->baseTpc() == Tpc::TPC_INVALID)
            hb.bassName->setText("");
      else
            hb.bassName->setText(harmony->baseName());
      hb.chordId->setValue(harmony->id());
      //hb.chordName->setText(harmony->parsedForm()->handle());
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
                  case HDegreeType::ADD:      s = "add";      break;
                  case HDegreeType::ALTER:    s = "alter";    break;
                  case HDegreeType::SUBTRACT: s = "subtract"; break;
                  default:                    s = "";         break;
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
      connect(sp.endElement,   SIGNAL(clicked()), SLOT(endClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void SpannerView::setElement(Element* e)
      {
      Spanner* spanner = static_cast<Spanner*>(e);
      ShowElementBase::setElement(e);
      sp.tick->setValue(spanner->tick());
      sp.ticks->setValue(spanner->ticks());
      sp.anchor->setCurrentIndex(int(spanner->anchor()));
      sp.track2->setValue(spanner->track2());

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
      bl.subType->setValue(int(barline->barLineType()));
      bl.span->setValue(barline->spanStaff());
      bl.spanFrom->setValue(barline->spanFrom());
      bl.spanTo->setValue(barline->spanTo());
//      bl.customSubtype->setChecked(barline->customSubtype());
//      bl.customSpan->setChecked(barline->customSpan());
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
      Dynamic* dynamic = toDynamic(e);

      tb.text->setPlainText(dynamic->xmlText());
      tb.xoffset->setValue(dynamic->offset().x());
      tb.yoffset->setValue(dynamic->offset().y());
      tb.offsetType->setCurrentIndex(int(dynamic->offsetType()));
      tb.layoutToParentWidth->setChecked(dynamic->layoutToParentWidth());

      ShowElementBase::setElement(e);
      bl.subType->setValue(int(dynamic->dynamicType()));
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
      connect(tb.tuplet, SIGNAL(clicked()), SLOT(tupletClicked()));
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
//   tupletClicked
//---------------------------------------------------------

void TupletView::tupletClicked()
      {
      emit elementChanged(((Tuplet*)element())->tuplet());
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
      tb.tuplet->setEnabled(tuplet->tuplet());
      tb.duration->setText(tuplet->duration().print());

      tb.elements->clear();
      for (DurationElement* e : tuplet->elements()) {
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
      el = 0;
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
      connect(eb.link2,          SIGNAL(clicked()), SLOT(link2Clicked()));
      connect(eb.link3,          SIGNAL(clicked()), SLOT(link3Clicked()));
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

      eb.address->setText(QString("%1").arg((quintptr)e, 0, 16));
      eb.score->setText(QString("%1").arg((quintptr)(e->score()), 0, 16));

      eb.selected->setChecked(e->selected());
      eb.selectable->setChecked(e->selectable());
      eb.droptarget->setChecked(e->dropTarget());
      eb.generated->setChecked(e->generated());
      eb.visible->setChecked(e->visible());
      eb.enabled->setChecked(e->enabled());
      eb.header->setChecked(e->header());
      eb.trailer->setChecked(e->trailer());
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
      eb.autoplace->setChecked(e->autoplace());
      eb.placement->setCurrentIndex(int(e->placement()));

      eb.bboxx->setValue(e->bbox().x());
      eb.bboxy->setValue(e->bbox().y());
      eb.bboxw->setValue(e->bbox().width());
      eb.bboxh->setValue(e->bbox().height());
      eb.color->setColor(e->color());
      eb.parentButton->setEnabled(e->parent());
      eb.link1->setEnabled(e->links());
      eb.link2->setEnabled(e->links() && e->links()->size() > 1);
      eb.link3->setEnabled(e->links() && e->links()->size() > 2);
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
            el->score()->select(el, SelectType::ADD, 0);
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
      emit elementChanged(static_cast<Element*>(el->links()->at(0)));
      }

//---------------------------------------------------------
//   link2Clicked
//---------------------------------------------------------

void ShowElementBase::link2Clicked()
      {
      emit elementChanged(static_cast<Element*>(el->links()->at(1)));
      }

//---------------------------------------------------------
//   link3Clicked
//---------------------------------------------------------

void ShowElementBase::link3Clicked()
      {
      emit elementChanged(static_cast<Element*>(el->links()->at(2)));
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
      st.direction->setCurrentIndex(int(slur->slurDirection()));
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
//TODO      emit elementChanged(static_cast<Volta*>(element())->beginTextElement());
      }

//---------------------------------------------------------
//   continueTextClicked
//---------------------------------------------------------

void VoltaView::continueTextClicked()
      {
//TODO      emit elementChanged(static_cast<Volta*>(element())->continueTextElement());
      }

//---------------------------------------------------------
//   endTextClicked
//---------------------------------------------------------

void VoltaView::endTextClicked()
      {
//TODO      emit elementChanged(static_cast<Volta*>(element())->endTextElement());
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
      connect(tlb.endText,      SIGNAL(clicked()), SLOT(endTextClicked()));
      connect(sp.segments,      SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(gotoElement(QTreeWidgetItem*)));
      connect(sp.startElement,  SIGNAL(clicked()), SLOT(startClicked()));
      connect(sp.endElement,    SIGNAL(clicked()), SLOT(endClicked()));
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
            item->setText(0, QString("%1").arg((quintptr)e, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)e));
            sp.segments->addTopLevelItem(item);
            }

      sp.tick->setValue(volta->tick());
      sp.ticks->setValue(volta->ticks());
      sp.track2->setValue(volta->track2());
      sp.startElement->setEnabled(volta->startElement() != 0);
      sp.endElement->setEnabled(volta->endElement() != 0);
      sp.anchor->setCurrentIndex(int(volta->anchor()));

//TODO      tlb.beginText->setEnabled(volta->beginTextElement());
//      tlb.continueText->setEnabled(volta->continueTextElement());
//      tlb.endText->setEnabled(volta->endTextElement());
      }

//---------------------------------------------------------
//   startClicked
//---------------------------------------------------------

void VoltaView::startClicked()
      {
      emit elementChanged(static_cast<Spanner*>(element())->startElement());
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void VoltaView::endClicked()
      {
      emit elementChanged(static_cast<Spanner*>(element())->endElement());
      }

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

VoltaSegmentView::VoltaSegmentView()
   : ShowElementBase()
      {
      lb.setupUi(addWidget());
      connect(lb.lineButton, SIGNAL(clicked()), SLOT(lineClicked()));
      }

//---------------------------------------------------------
//   endClicked
//---------------------------------------------------------

void VoltaSegmentView::lineClicked()
      {
      emit elementChanged(static_cast<VoltaSegment*>(element())->volta());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void VoltaSegmentView::setElement(Element* e)
      {
      VoltaSegment* vs = (VoltaSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(int(vs->spannerSegmentType()));
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
      connect(lb.lineButton, SIGNAL(clicked()), SLOT(lineClicked()));
      }

//---------------------------------------------------------
//   lineClicked
//---------------------------------------------------------

void LineSegmentView::lineClicked()
      {
      emit elementChanged(((LineSegment*)element())->spanner());
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void LineSegmentView::setElement(Element* e)
      {
      LineSegment* vs = (LineSegment*)e;
      ShowElementBase::setElement(e);

      lb.segmentType->setCurrentIndex(int(vs->spannerSegmentType()));
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
      lb.syllabic->setCurrentIndex(int(l->syllabic()));
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
            item->setText(0, QString("%1").arg((quintptr)cr, 8, 16));
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>((void*)cr));
            item->setText(1, cr->name());
            item->setText(2, QString("%1").arg(cr->segment()->tick()));
            bb.elements->addTopLevelItem(item);
            }
      bb.grow1->setValue(b->growLeft());
      bb.grow2->setValue(b->growRight());
      bb.cross->setChecked(b->cross());
      bb.isGrace->setChecked(b->isGrace());
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
//   TieSegmentView
//---------------------------------------------------------

TieSegmentView::TieSegmentView()
   : ShowElementBase()
      {
      ss.setupUi(addWidget());
      connect(ss.slurTie, SIGNAL(clicked()), SLOT(slurTieClicked()));
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void TieSegmentView::slurTieClicked()
      {
      emit elementChanged(toTieSegment(element())->slurTie());
      }

//---------------------------------------------------------
//   TieSegmentView
//---------------------------------------------------------

void TieSegmentView::setElement(Element* e)
      {
      TieSegment* s = toTieSegment(e);
      ShowElementBase::setElement(e);
      ss.up1px->setValue(s->ups(Grip::START).p.x());
      ss.up1py->setValue(s->ups(Grip::START).p.y());
      ss.up1ox->setValue(s->ups(Grip::START).off.x());
      ss.up1oy->setValue(s->ups(Grip::START).off.y());

      ss.up2px->setValue(s->ups(Grip::BEZIER1).p.x());
      ss.up2py->setValue(s->ups(Grip::BEZIER1).p.y());
      ss.up2ox->setValue(s->ups(Grip::BEZIER1).off.x());
      ss.up2oy->setValue(s->ups(Grip::BEZIER1).off.y());

      ss.up3px->setValue(s->ups(Grip::BEZIER2).p.x());
      ss.up3py->setValue(s->ups(Grip::BEZIER2).p.y());
      ss.up3ox->setValue(s->ups(Grip::BEZIER2).off.x());
      ss.up3oy->setValue(s->ups(Grip::BEZIER2).off.y());

      ss.up4px->setValue(s->ups(Grip::END).p.x());
      ss.up4py->setValue(s->ups(Grip::END).p.y());
      ss.up4ox->setValue(s->ups(Grip::END).off.x());
      ss.up4oy->setValue(s->ups(Grip::END).off.y());
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
      ss.up1px->setValue(s->ups(Grip::START).p.x());
      ss.up1py->setValue(s->ups(Grip::START).p.y());
      ss.up1ox->setValue(s->ups(Grip::START).off.x());
      ss.up1oy->setValue(s->ups(Grip::START).off.y());

      ss.up2px->setValue(s->ups(Grip::BEZIER1).p.x());
      ss.up2py->setValue(s->ups(Grip::BEZIER1).p.y());
      ss.up2ox->setValue(s->ups(Grip::BEZIER1).off.x());
      ss.up2oy->setValue(s->ups(Grip::BEZIER1).off.y());

      ss.up3px->setValue(s->ups(Grip::BEZIER2).p.x());
      ss.up3py->setValue(s->ups(Grip::BEZIER2).p.y());
      ss.up3ox->setValue(s->ups(Grip::BEZIER2).off.x());
      ss.up3oy->setValue(s->ups(Grip::BEZIER2).off.y());

      ss.up4px->setValue(s->ups(Grip::END).p.x());
      ss.up4py->setValue(s->ups(Grip::END).p.y());
      ss.up4ox->setValue(s->ups(Grip::END).off.x());
      ss.up4oy->setValue(s->ups(Grip::END).off.y());

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

//TODO      acc.hasBracket->setChecked(s->hasBracket());
      acc.accAuto->setChecked(s->role() == AccidentalRole::AUTO);
      acc.accUser->setChecked(s->role() == AccidentalRole::USER);
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
      articulation.symName->setText(Sym::id2name(a->symId()));
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

      KeySigEvent ev = ks->keySigEvent();
      keysig.showCourtesySig->setChecked(ks->showCourtesy());
      keysig.accidentalType->setValue(int(ev.key()));
      keysig.custom->setChecked(ev.custom());
      keysig.atonal->setChecked(ev.isAtonal());
      keysig.invalid->setChecked(!ev.isValid());
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

//      tlb.beginText->setEnabled(volta->beginTextElement());
//      tlb.continueText->setEnabled(volta->continueTextElement());
//      tlb.endText->setEnabled(volta->endTextElement());
      }

//---------------------------------------------------------
//   beginTextClicked
//---------------------------------------------------------

void TextLineView::beginTextClicked()
      {
//      Volta* volta = (Volta*)element();
//      emit elementChanged(volta->beginTextElement());
      }

//---------------------------------------------------------
//   continueTextClicked
//---------------------------------------------------------

void TextLineView::continueTextClicked()
      {
//      Volta* volta = (Volta*)element();
//      emit elementChanged(volta->continueTextElement());
      }

//---------------------------------------------------------
//   endTextClicked
//---------------------------------------------------------

void TextLineView::endTextClicked()
      {
//      Volta* volta = (Volta*)element();
//      emit elementChanged(volta->endTextElement());
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

      lb.segmentType->setCurrentIndex(int(vs->spannerSegmentType()));
      lb.pos2x->setValue(vs->pos2().x());
      lb.pos2y->setValue(vs->pos2().y());
      lb.offset2x->setValue(vs->userOff2().x());
      lb.offset2y->setValue(vs->userOff2().y());
      connect(lb.lineButton, SIGNAL(clicked()), SLOT(lineClicked()));
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


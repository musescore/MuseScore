//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009-2011 Werner Schweer and others
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

#include "musescore.h"
#include "harmonyedit.h"
#include "harmonycanvas.h"
#include "palette.h"
#include "libmscore/accidental.h"
#include "libmscore/score.h"
#include "icons.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/symbol.h"
#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"

extern bool useFactorySettings;

//---------------------------------------------------------
//   ChordStyleEditor
//---------------------------------------------------------

ChordStyleEditor::ChordStyleEditor(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Chord Style Editor"));
      fileButton->setIcon(*icons[fileOpen_ICON]);
      chordList = 0;
      score = 0;

      connect(fileButton, SIGNAL(clicked()), SLOT(fileButtonClicked()));
      connect(saveButton, SIGNAL(clicked()), SLOT(saveButtonClicked()));
      connect(harmonyList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
         SLOT(harmonyChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ChordStyleEditor::setScore(Score* s)
      {
      score = s;
      setChordList(s->style()->chordList());
      }

//---------------------------------------------------------
//   editChordStyle
//---------------------------------------------------------

void MuseScore::editChordStyle()
      {
      if (chordStyleEditor == 0) {
            chordStyleEditor = new ChordStyleEditor(0);
            chordStyleEditor->restore();
            }
      chordStyleEditor->setScore(cs);
      chordStyleEditor->show();
      chordStyleEditor->raise();
      }

//---------------------------------------------------------
//   fileButtonClicked
//---------------------------------------------------------

void ChordStyleEditor::fileButtonClicked()
      {
      QString fn = mscore->getChordStyleFilename(true);
      if (fn.isEmpty())
            return;
      loadChordDescriptionFile(fn);
      }

//---------------------------------------------------------
//   saveButtonClicked
//---------------------------------------------------------

void ChordStyleEditor::saveButtonClicked()
      {
      if (!chordList)
            return;
      canvas->updateChordDescription();

      QString fn = mscore->getChordStyleFilename(false);
      if (fn.isEmpty())
            return;
      chordList->write(fn);
      }

//---------------------------------------------------------
//   loadChordDescriptionFile
//---------------------------------------------------------

void ChordStyleEditor::loadChordDescriptionFile(const QString& s)
      {
      ChordList* cl = new ChordList;
      if (!cl->read("chords.xml")) {
            qDebug("cannot read <chords.xml>\n");
            return;
            }
      if (!cl->read(s)) {
            qDebug("cannot read <%s>\n", qPrintable(s));
            return;
            }
      setChordList(cl);
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void ChordStyleEditor::setChordList(ChordList* cl)
      {
      harmonyList->clear();
      foreach (ChordDescription* d, *cl) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setData(0, Qt::UserRole, QVariant::fromValue<void*>(d));
            item->setText(0, QString("%1").arg(d->id));
            if (!d->names.isEmpty())
                  item->setText(1, QString("%1").arg(d->names.front()));
            harmonyList->addTopLevelItem(item);
            }
      delete chordList;
      chordList = new ChordList(*cl);
      canvas->setChordDescription(0, 0);

      paletteTab->clear();
      foreach(const ChordFont& f, chordList->fonts) {
            // create symbol palette
            Palette* p = new Palette();
            PaletteScrollArea* accPalette = new PaletteScrollArea(p);
            QSizePolicy policy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
            accPalette->setSizePolicy(policy1);
            accPalette->setRestrictHeight(false);
            p->setGrid(50, 50);
            paletteTab->addTab(accPalette, f.family);
            QFont qf(f.family);
            qf.setStyleStrategy(QFont::NoFontMerging);
            int size = lrint(20.0 * MScore::DPI / PPI);
            qf.setPixelSize(size);

            QFontMetricsF fi(qf);
            for (int i = 0; i < 255; ++i) {
                  if (fi.inFont(QChar(i))) {
                        FSymbol* s = new FSymbol(gscore);
                        s->setFont(qf);
                        s->setCode(i);
                        p->append(s, "??");
                        }
                  }
            }
      raise();
      }

//---------------------------------------------------------
//   harmonyChanged
//---------------------------------------------------------

void ChordStyleEditor::harmonyChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
      {
      if (previous) {
            // ChordDescription* d = static_cast<ChordDescription*>(previous->data(0, Qt::UserRole).value<void*>());
            canvas->updateChordDescription();
            }
      if (current) {
            ChordDescription* d = static_cast<ChordDescription*>(current->data(0, Qt::UserRole).value<void*>());
            canvas->setChordDescription(d, chordList);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void ChordStyleEditor::save()
      {
      QSettings settings;
      settings.beginGroup("ChordStyleEditor");
      settings.setValue("splitter1", splitter1->saveState());
      settings.setValue("splitter2", splitter2->saveState());
//      settings.setValue("list", harmonyList->saveState());
      settings.setValue("col1", harmonyList->columnWidth(0));
      }

//---------------------------------------------------------
//   restore
//---------------------------------------------------------

void ChordStyleEditor::restore()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("ChordStyleEditor");
            splitter1->restoreState(settings.value("splitter1").toByteArray());
            splitter2->restoreState(settings.value("splitter2").toByteArray());
            harmonyList->setColumnWidth(0, settings.value("col1", 30).toInt());
            }
      }

//---------------------------------------------------------
//   HarmonyCanvas
//---------------------------------------------------------

HarmonyCanvas::HarmonyCanvas(QWidget* parent)
   : QFrame(parent)
      {
      setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
      setAcceptDrops(true);
      setFocusPolicy(Qt::StrongFocus);
      extraMag = 3.0;
      chordDescription = 0;
      chordList = 0;
      moveElement = 0;
      QAction* a = getAction("delete");
      addAction(a);
      connect(a, SIGNAL(triggered()), SLOT(deleteAction()));
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void HarmonyCanvas::paintEvent(QPaintEvent* event)
      {
      QFrame::paintEvent(event);
      if (!chordDescription)
            return;

      qreal spatium = gscore->spatium();
      qreal mag = PALETTE_SPATIUM * extraMag / spatium;
      spatium = SPATIUM20 * MScore::DPI;

      QPainter p(this);

      p.setRenderHint(QPainter::Antialiasing, true);
      qreal wh = double(height());
      qreal ww = double(width());

      _matrix   = QTransform(mag, 0.0, 0.0, mag, ww*.1, wh*.8);
      imatrix   = _matrix.inverted();

      p.setWorldTransform(_matrix);
      QRectF f = imatrix.mapRect(QRectF(0.0, 0.0, ww, wh));

      p.setPen(QPen(Qt::darkGray));
      p.drawLine(f.x(), 0.0, f.width(), 0.0);
      p.drawLine(0.0, f.y(), 0.0, f.height());

      foreach(const TextSegment* ts, textList) {
            p.setFont(ts->font);
            QPen pen(ts->select ? Qt::blue : palette().color(QPalette::Text));
            p.setPen(pen);
            p.drawText(ts->x, ts->y, ts->text);
            }

      if (dragElement && dragElement->type() == FSYMBOL) {
            FSymbol* sb = static_cast<FSymbol*>(dragElement);

            double _spatium = 2.0 * PALETTE_SPATIUM / extraMag;
            const TextStyle* st = &gscore->textStyle(TEXT_STYLE_HARMONY);
            QFont ff(st->fontPx(_spatium));
            ff.setFamily(sb->font().family());

            QString s;
            int code = sb->code();
            if (code & 0xffff0000) {
                  s = QChar(QChar::highSurrogate(code));
                  s += QChar(QChar::lowSurrogate(code));
                  }
            else
                  s = QChar(code);
            p.setFont(ff);
            QPen pen(Qt::yellow);
            p.setPen(pen);
            p.drawText(dragElement->pos(), s);
            }
      }

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void HarmonyCanvas::render(const QList<RenderAction>& renderList, double& x, double& y, int tpc)
      {
      QStack<QPointF> stack;
      int fontIdx = 0;
      double _spatium = 2.0 * PALETTE_SPATIUM / extraMag;
//      qreal mag  = PALETTE_SPATIUM * extraMag / _spatium;

      QList<QFont> fontList;              // temp values used in render()
      const TextStyle* st = &gscore->textStyle(TEXT_STYLE_HARMONY);

      foreach(ChordFont cf, chordList->fonts) {
            if (cf.family.isEmpty() || cf.family == "default")
                  fontList.append(st->fontPx(_spatium * cf.mag));
            else {
                  QFont ff(st->fontPx(_spatium * cf.mag));
                  ff.setFamily(cf.family);
                  fontList.append(ff);
                  }
            }
      if (fontList.isEmpty())
            fontList.append(st->fontPx(_spatium));

      foreach(const RenderAction& a, renderList) {
            if (a.type == RenderAction::RENDER_SET) {
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(a.text);
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(a.text);
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_MOVE) {
                  x += a.movex;//  * mag;
                  y += a.movey; //  * mag;
                  }
            else if (a.type == RenderAction::RENDER_PUSH)
                  stack.push(QPointF(x,y));
            else if (a.type == RenderAction::RENDER_POP) {
                  if (!stack.isEmpty()) {
                        QPointF pt = stack.pop();
                        x = pt.x();
                        y = pt.y();
                        }
                  else
                        qDebug("RenderAction::RENDER_POP: stack empty\n");
                  }
            else if (a.type == RenderAction::RENDER_NOTE) {
                  bool germanNames = gscore->styleB(ST_useGermanNoteNames);
                  QChar c;
                  int acc;
                  tpc2name(tpc, germanNames, &c, &acc);
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(QString(c));
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(QString(c));
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_ACCIDENTAL) {
                  QChar c;
                  int acc;
                  tpc2name(tpc, false, &c, &acc);
                  if (acc) {
                        TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                        QString s;
                        if (acc == -1)
                              s = "b";
                        else if (acc == 1)
                              s = "#";
                        ChordSymbol cs = chordList->symbol(s);
                        if (cs.isValid()) {
                              ts->font = fontList[cs.fontIdx];
                              ts->setText(QString(cs.code));
                              }
                        else
                              ts->setText(s);
                        textList.append(ts);
                        x += ts->width();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void HarmonyCanvas::mousePressEvent(QMouseEvent* event)
      {
      startMove = imatrix.map(QPointF(event->pos()));
      moveElement = 0;
      foreach(TextSegment* ts, textList) {
            QRectF r = ts->boundingRect().translated(ts->x, ts->y);
            ts->select = r.contains(startMove);
            if (ts->select)
                  moveElement = ts;
            }
      update();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void HarmonyCanvas::mouseMoveEvent(QMouseEvent* event)
      {
      if (moveElement == 0)
            return;
      QPointF p = imatrix.map(QPointF(event->pos()));
      QPointF delta = p - startMove;
      moveElement->x += delta.x();
      moveElement->y += delta.y();
      startMove = p;
      update();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void HarmonyCanvas::mouseReleaseEvent(QMouseEvent*)
      {
      }

//---------------------------------------------------------
//   setChordDescription
//---------------------------------------------------------

void HarmonyCanvas::setChordDescription(ChordDescription* sd, ChordList* sl)
      {
      chordDescription = sd;
      chordList = sl;

      foreach(TextSegment* s, textList)
            delete s;
      textList.clear();

      if (chordList) {
            int tpc = 14;
            double x = 0.0, y = 0.0;
            render(chordList->renderListRoot, x, y, 14);
            render(chordDescription->renderList, x, y, tpc);
            }
      moveElement = 0;
      dragElement = 0;
      update();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void HarmonyCanvas::dropEvent(QDropEvent* event)
      {
      if (dragElement && dragElement->type() == FSYMBOL) {
            FSymbol* sb = static_cast<FSymbol*>(dragElement);

            double _spatium = 2.0 * PALETTE_SPATIUM / extraMag;
            const TextStyle* st = &gscore->textStyle(TEXT_STYLE_HARMONY);
            QFont ff(st->fontPx(_spatium));
            ff.setFamily(sb->font().family());

//            qDebug("drop %s\n", dragElement->name());

            QString s;
            int code = sb->code();
            if (code & 0xffff0000) {
                  s = QChar(QChar::highSurrogate(code));
                  s += QChar(QChar::lowSurrogate(code));
                  }
            else
                  s = QChar(code);

            QPointF pt = imatrix.map(event->pos());

            TextSegment* ts = new TextSegment(s, ff, pt.x(), pt.y());
            textList.append(ts);
            delete dragElement;
            dragElement = 0;
            update();
            }
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void HarmonyCanvas::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* data = event->mimeData();
      if (data->hasFormat(mimeSymbolFormat)) {
            QByteArray a = data->data(mimeSymbolFormat);

            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(a, &err, &line, &column)) {
                  qDebug("error reading drag data at %d/%d: %s\n<%s>\n",
                     line, column, err.toLatin1().data(), a.data());
                  return;
                  }
            docName = "--";
            QDomElement e = doc.documentElement();

            QPointF dragOffset;
            Fraction duration;
            ElementType type = Element::readType(e, &dragOffset, &duration);
            if (type == FSYMBOL) {
                  event->acceptProposedAction();
                  dragElement = Element::create(type, gscore);
                  dragElement->read(e);
                  dragElement->layout();
                  }
            }
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void HarmonyCanvas::dragLeaveEvent(QDragLeaveEvent*)
      {
      delete dragElement;
      dragElement = 0;
      update();
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void HarmonyCanvas::dragMoveEvent(QDragMoveEvent* event)
      {
      event->acceptProposedAction();
      if (dragElement && dragElement->type() == FSYMBOL) {
            dragElement->setPos(imatrix.map(event->pos()));
            update();
            }
      }

//---------------------------------------------------------
//   deleteAction
//---------------------------------------------------------

void HarmonyCanvas::deleteAction()
      {
      if (moveElement) {
            textList.removeOne(moveElement);
            update();
            }
      }

//---------------------------------------------------------
//   updateHarmony
//---------------------------------------------------------

static void updateHarmony(void*, Element* e)
      {
      if (e->type() == HARMONY)
            static_cast<Harmony*>(e)->render();
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ChordStyleEditor::accept()
      {
      canvas->updateChordDescription();
      score->style()->setChordList(chordList);
      chordList = 0;

      score->scanElements(0, updateHarmony);

      QDialog::accept();
      }

//---------------------------------------------------------
//   updateCurrentChordDescription
//---------------------------------------------------------

void HarmonyCanvas::updateChordDescription()
      {
      chordDescription->renderList.clear();

      int idx = 0;
      double x  = 0, y = 0;
      foreach(const TextSegment* ts, textList) {
            ++idx;
            if (idx == 1) {     // dont save base
                  x = ts->x + ts->width();
                  y = ts->y;
                  continue;
                  }
            if (ts->x != x || ts->y != y) {
                  RenderAction ra(RenderAction::RENDER_MOVE);
                  ra.movex = ts->x - x;
                  ra.movey = ts->y - y;
                  chordDescription->renderList.append(ra);
                  }
            RenderAction ra(RenderAction::RENDER_SET);
            ra.text  = ts->text;
            chordDescription->renderList.append(ra);
            x = ts->x + ts->width();
            y = ts->y;
            }
      }

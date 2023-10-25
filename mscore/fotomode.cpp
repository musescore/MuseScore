//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreview.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/lasso.h"
#include "icons.h"
#include "libmscore/page.h"
#include "preferences.h"
#include "libmscore/image.h"
#include "libmscore/mscore.h"
#include "svggenerator.h"
#include "inspector/inspector.h"
#include "fotomode.h"

namespace Ms {

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void FotoLasso::startEdit(EditData& ed)
      {
      Lasso::startEdit(ed);
      QRectF view = ((ScoreView*)ed.view)->toLogical(QRect(0.0, 0.0, ed.view->geometry().width(), ed.view->geometry().height()));
      if (bbox().isEmpty() || !view.intersects(bbox())) {
            // rect not found - construct new rect with default size & relative position
            qreal w = view.width();
            qreal h = view.height();
            QRectF rect(w * .3, h * .3, w * .4, h * .4);
            // convert to absolute position
            setbbox(rect.translated(view.topLeft()));
            }
      setVisible(false);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void FotoLasso::endEdit(EditData&)
      {
      setVisible(false);
      }

//---------------------------------------------------------
//   drawEditMode
//---------------------------------------------------------

void FotoLasso::drawEditMode(QPainter* p, EditData& ed)
      {
      QPointF pos(canvasPos());
      p->translate(pos);
      draw(p);
      p->translate(-pos);
      Lasso::drawEditMode(p, ed);
      }

//---------------------------------------------------------
//   startFotoMode
//---------------------------------------------------------

void ScoreView::startFotomode()
      {
      if (!_foto)
            _foto = new FotoLasso(_score);
      else
            _foto->setScore(_score);
      QRectF view = toLogical(QRect(0.0, 0.0, width(), height()));
      if (_foto->bbox().isEmpty() || !view.intersects(_foto->bbox())) {
            // rect not found - construct new rect with default size & relative position
            qreal w = view.width();
            qreal h = view.height();
            QRectF r(w * .3, h * .3, w * .4, h * .4);
            // convert to absolute position
            _foto->setbbox(toPhysical(r));
            }
      _foto->setFlag(ElementFlag::MOVABLE, true);
      _foto->setVisible(true);
      _score->select(_foto);
      setEditElement(_foto);
      QAction* a = getAction("fotomode");
      a->setChecked(true);
      startEdit();
      }

//---------------------------------------------------------
//   stopFotomode
//---------------------------------------------------------

void ScoreView::stopFotomode()
      {
      QAction* a = getAction("fotomode");
      a->setChecked(false);
      _foto->setVisible(false);
      endEdit();
      update();
      }

//---------------------------------------------------------
//   startFotoDrag
//---------------------------------------------------------

void ScoreView::startFotoDrag()
      {
      _score->addRefresh(_foto->abbox());
      _score->update();
      editData.grips = 0;
      }

//---------------------------------------------------------
//   doDragFoto
//    drag canvas in foto mode
//---------------------------------------------------------

void ScoreView::doDragFoto(QMouseEvent* ev)
      {
      _foto->setOffset(QPointF(0.0, 0.0));
      QPointF p = toLogical(ev->pos());
      QPointF sm = editData.startMove;

      QRectF r;
      r.setCoords(sm.x(), sm.y(), p.x(), p.y());

      _foto->setbbox(r.normalized());

      QRectF rr(_foto->bbox());
      r = _matrix.mapRect(rr);
      //QSize sz(r.size().toSize());
      //mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);

      update();
      //mscore->showMessage("drag", 2000);
      }

//---------------------------------------------------------
//   endFotoDrag
//---------------------------------------------------------

void ScoreView::endFotoDrag()
      {
      qreal w = 8.0 / _matrix.m11();
      qreal h = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      editData.grip.resize(8);
      for (int i = 0; i < 8; ++i)
            editData.grip[i] = r;
      setEditElement(_foto);
      updateGrips();
      _score->setUpdateAll();
      _score->update();
      }

//---------------------------------------------------------
//   doFotoDragEdit
//---------------------------------------------------------

void ScoreView::doFotoDragEdit(QMouseEvent* ev)
      {
      QPointF p     = toLogical(ev->pos());
      QPointF delta = p - editData.startMove;
      score()->addRefresh(_foto->abbox());

      editData.delta   = delta;
      _foto->editDrag(editData);
      updateGrips();
      editData.startMove = p;
      _score->update();
      if (mscore->inspector())
            mscore->inspector()->update(_foto->score());
      }

//---------------------------------------------------------
//   endFotoDragEdit
//---------------------------------------------------------

void ScoreView::endFotoDragEdit()
      {
      }

//---------------------------------------------------------
//   fotoEditElementDragTransition
//---------------------------------------------------------

bool ScoreView::fotoEditElementDragTransition(QMouseEvent* ev)
      {
      editData.startMove = imatrix.map(QPointF(ev->pos()));
      int i;
      for (i = 0; i < editData.grips; ++i) {
            if (editData.grip[i].contains(editData.startMove)) {
                  editData.curGrip = Grip(i);
                  switch (int(editData.curGrip)) {
                        case 0:
                        case 2:
                              setCursor(Qt::SizeFDiagCursor);
                              break;
                        case 1:
                        case 3:
                              setCursor(Qt::SizeBDiagCursor);
                              break;
                        case 4:
                        case 6:
                              setCursor(Qt::SizeVerCursor);
                              break;
                        case 5:
                        case 7:
                              setCursor(Qt::SizeHorCursor);
                              break;
                        }
                  updateGrips();
                  score()->update();
                  break;
                  }
            }
      return i != editData.grips;
      }

//---------------------------------------------------------
//   fotoScoreViewDragTest
//---------------------------------------------------------

bool ScoreView::fotoScoreViewDragTest(QMouseEvent* me)
      {
      QPointF p(imatrix.map(QPointF(me->pos())));
      if (_foto->bbox().contains(p))
            return false;
      for (int i = 0; i < editData.grips; ++i) {
            if (editData.grip[i].contains(p))
                  return false;
            }
      editData.startMove = p;
      return true;
      }

//---------------------------------------------------------
//   fotoScoreViewDragRectTest
//---------------------------------------------------------

bool ScoreView::fotoScoreViewDragRectTest(QMouseEvent* me)
      {
      QPointF p(toLogical(me->pos()));
      if (!_foto->bbox().contains(p))
            return false;
      for (int i = 0; i < editData.grips; ++i) {
            if (editData.grip[i].contains(p))
                  return false;
            }
      editData.startMove = p;
      return true;
      }

//---------------------------------------------------------
//   doDragFotoRect
//---------------------------------------------------------

void ScoreView::doDragFotoRect(QMouseEvent* ev)
      {
      QPointF p(toLogical(ev->pos()));
      QPointF delta = p - editData.startMove;
      score()->addRefresh(_foto->abbox());
      _foto->setbbox(_foto->bbox().translated(delta));
      score()->addRefresh(_foto->abbox());
      editData.startMove = p;
      updateGrips();
      _score->update();
      if (mscore->inspector())
            mscore->inspector()->update(_foto->score());
      }

//---------------------------------------------------------
//   MenuEntry
//---------------------------------------------------------

struct MenuEntry {
      const char* text;
      const char* label;
      };

static const MenuEntry resizeEntry[4] {
      { QT_TRANSLATE_NOOP("fotomode", "Resize to A"), "resizeA" },
      { QT_TRANSLATE_NOOP("fotomode", "Resize to B"), "resizeB" },
      { QT_TRANSLATE_NOOP("fotomode", "Resize to C"), "resizeC" },
      { QT_TRANSLATE_NOOP("fotomode", "Resize to D"), "resizeD" }
      };

static const MenuEntry setSizeEntry[4] {
      { QT_TRANSLATE_NOOP("fotomode", "Set size A"), "setA" },
      { QT_TRANSLATE_NOOP("fotomode", "Set size B"), "setB" },
      { QT_TRANSLATE_NOOP("fotomode", "Set size C"), "setC" },
      { QT_TRANSLATE_NOOP("fotomode", "Set size D"), "setD" }
      };

//---------------------------------------------------------
//   fotoContextPopup
//---------------------------------------------------------

void ScoreView::fotoContextPopup(QContextMenuEvent* ev)
      {
      QPoint pos(ev->globalPos());
      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);
      QAction* a = popup->addSeparator();
      a->setText(tr("Image Capture"));

      a = getAction("copy");
      popup->addAction(a);
      a = new QAction(tr("Copy with Link to Score"), this);
      a->setData("copy-link");
      popup->addAction(a);

      popup->addSeparator();
      a = popup->addAction(tr("Resolution (%1 DPI)…").arg(preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION)));
      a->setData("set-res");
      QAction* bgAction = popup->addAction(tr("Transparent background"));
      bgAction->setCheckable(true);
      bgAction->setChecked(preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY));
      bgAction->setData("set-bg");

      popup->addSeparator();
      a = new QAction(tr("Auto-resize to page"), this);
      a->setData("resizePage");
      popup->addAction(a);
      for (int i = 0; i < 4; ++i) {
            a = new QAction(qApp->translate("fotomode", resizeEntry[i].text), this);
            a->setData(resizeEntry[i].label);
            popup->addAction(a);
            }
      QMenu* setSize = new QMenu(tr("Set Standard Size…"));
      for (int i = 0; i < 4; ++i) {
            a = new QAction(qApp->translate("fotomode", setSizeEntry[i].text), this);
            a->setData(setSizeEntry[i].label);
            setSize->addAction(a);
            }
      popup->addMenu(setSize);

      popup->addSeparator();
      a = new QAction(tr("Save As (Print Mode)…"), this);
      a->setData("print");
      popup->addAction(a);
      a = new QAction(tr("Save As (Screenshot Mode)…"), this);
      a->setData("screenshot");
      popup->addAction(a);

      a = popup->exec(pos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "print")
            saveFotoAs(true, _foto->canvasBoundingRect());
      else if (cmd == "screenshot")
            saveFotoAs(false, _foto->canvasBoundingRect());
      else if (cmd == "copy")
            ;
      else if (cmd == "copy-link")
            fotoModeCopy(true);
      else if (cmd == "set-res") {
            bool ok;
            double resolution = QInputDialog::getDouble(this,
               tr("Set Output Resolution"),
               tr("Set output resolution for PNG"),
               preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION),
               0.0, 5000.0, 0,
               &ok
               );
            if (ok) {
                  preferences.setPreference(PREF_EXPORT_PNG_RESOLUTION, resolution);
                  }
            }
      else if (cmd == "resizePage") {
            _foto->setOffset(0, 0);
            QRectF r = _foto->bbox();
            Page* page = point2page(r.center());
            if (page) {
                  r = page->tbbox().translated(page->canvasPos());
                  _foto->setbbox(r);
                  updateGrips();
                  }
            }
      else if (cmd.startsWith("resize")) {
            QString size = QSettings().value(QString("fotoSize%1").arg(cmd[6]), "50x40").toString();
            qreal w = size.split("x")[0].toDouble();
            qreal h = size.split("x")[1].toDouble();
            _foto->bbox().setSize(QSizeF(w * DPMM, h * DPMM));
            updateGrips();
            }
      else if (cmd.startsWith("set")) {
            qreal w   = _foto->bbox().width() / DPMM;
            qreal h   = _foto->bbox().height() / DPMM;
            QString val(QString("%1x%2").arg(w).arg(h));
            QSettings().setValue(QString("fotoSize%1").arg(cmd[3]), val);
            }
      if (bgAction->isChecked() != preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY)) {
            preferences.setPreference(PREF_EXPORT_PNG_USETRANSPARENCY, bgAction->isChecked());
            }
      }

//---------------------------------------------------------
//   getRectImage
//---------------------------------------------------------

QImage ScoreView::getRectImage(const QRectF& rect, double dpi, bool transparent, bool printMode)
      {
      const double mag = dpi / DPI;
      const int w = lrint(rect.width()  * mag);
      const int h = lrint(rect.height() * mag);

      QImage::Format f = QImage::Format_ARGB32_Premultiplied;
      QImage img(w, h, f);
      img.setDotsPerMeterX(lrint((dpi * 1000) / INCH));
      img.setDotsPerMeterY(lrint((dpi * 1000) / INCH));
      img.fill(transparent ? 0 : 0xffffffff);

      const auto pr = MScore::pixelRatio;
      MScore::pixelRatio = 1.0 / mag;
      QPainter p(&img);
      paintRect(printMode, p, rect, mag);
      MScore::pixelRatio = pr;

      return img;
      }

//---------------------------------------------------------
//   fotoModeCopy
//---------------------------------------------------------

void ScoreView::fotoModeCopy(bool includeLink)
      {
#if defined(Q_OS_WIN)
      // See https://bugreports.qt.io/browse/QTBUG-11463
      // while transparent copy/paste works fine inside musescore,
      // it does not paste into other programs in Windows though
      bool transparent = false; // preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY);
#else
      bool transparent = preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY);
#endif
      double convDpi   = preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION);
      QRectF r(_foto->canvasBoundingRect());

      QImage printer(getRectImage(r, convDpi, transparent, /* printMode */ true));
      QApplication::clipboard()->clear();

      if (includeLink) {
            QUrl url = QUrl::fromLocalFile(score()->masterScore()->fileInfo()->canonicalFilePath());
            QByteArray imageData;
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);
            printer.save(&buffer, "PNG");
            buffer.close();
            QString html = "<a href=\"" + url.toString() + "\"><img src=\"data:image/png," + imageData.toPercentEncoding() + "\" /></a>";
            QMimeData *mdata = new QMimeData;
            mdata->setHtml(html);
            QApplication::clipboard()->setMimeData(mdata);
            // TODO: add both, with priority to html
            //QApplication::clipboard()->setImage(printer);
            }
      else {
            QApplication::clipboard()->setImage(printer);
            }
      }

//---------------------------------------------------------
//   fotoRectHit
//---------------------------------------------------------

bool ScoreView::fotoRectHit(const QPoint& pos)
      {
      QPointF p = toLogical(pos);
      for (int i = 0; i < editData.grips; ++i) {
            if (editData.grip[i].contains(p))
                  return false;
            }
      editData.startMove = p;
      return _foto->bbox().contains(p);
      }

//---------------------------------------------------------
//   saveFotoAs
//    return true on success
//---------------------------------------------------------

bool ScoreView::saveFotoAs(bool printMode, const QRectF& r)
      {
      QStringList fl;
      fl.append(tr("PNG Bitmap Graphic") + " (*.png)");
      fl.append(tr("PDF File") + " (*.pdf)");
      fl.append(tr("Scalable Vector Graphics") + " (*.svg)");

      QString selectedFilter;
      QString filter = fl.join(";;");
      QString fn = mscore->getFotoFilename(filter, &selectedFilter);

      if (fn.isEmpty())
            return false;

      QFileInfo fi(fn);
      mscore->lastSaveDirectory = fi.absolutePath();

      QString ext;
      if (selectedFilter.isEmpty()) {
            ext = fi.suffix();
            }
      else {
            int idx = fl.indexOf(selectedFilter);
            if (idx != -1) {
                  static const char* extensions[] = {
                        "png",
                        "pdf",
                        "svg"
                        };
                  ext = extensions[idx];
                  }
            }

      if (ext.isEmpty()) {
            QMessageBox::critical(mscore, tr("Save As"), tr("Cannot determine file type"));
            return false;
            }

      ext = ext.toLower();
      if (fi.suffix().toLower() != ext)
            fn += "." + ext;

      bool transparent = preferences.getBool(PREF_EXPORT_PNG_USETRANSPARENCY);
      double convDpi   = preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION);
      double mag       = convDpi / DPI;

      if (ext == "svg")
            mag = 1; // SVG is not scaled, it's scalable.

      int w = lrint(r.width()  * mag);
      int h = lrint(r.height() * mag);

      double pr = MScore::pixelRatio;
      if (ext == "pdf") {
            QPdfWriter pdfWriter(fn);
            pdfWriter.setResolution(preferences.getInt(PREF_EXPORT_PDF_DPI));
            mag = pdfWriter.logicalDpiX() / DPI;
            QSizeF size(r.width() / DPI, r.height() / DPI);
            QPageSize ps(size, QPageSize::Inch, "", QPageSize::ExactMatch);
            pdfWriter.setPageSize(ps);
            pdfWriter.setPageMargins(QMarginsF(0.0, 0.0, 0.0, 0.0));
            pdfWriter.setCreator("MuseScore Version: " VERSION);
            pdfWriter.setTitle(fn);
            MScore::pixelRatio = DPI / pdfWriter.logicalDpiX();
            QPainter p(&pdfWriter);
            MScore::pdfPrinting = true;
            paintRect(printMode, p, r, mag);
            MScore::pdfPrinting = false;
            }
      else if (ext == "svg") {
            // note that clipping is not implemented
            // (as of 4.8)
            SvgGenerator printer;
            printer.setFileName(fn);
            printer.setTitle(_score->title());
            printer.setSize(QSize(w, h));
            printer.setViewBox(QRect(0, 0, w, h));
            MScore::pixelRatio = DPI / printer.logicalDpiX();
            QPainter p(&printer);
            MScore::pdfPrinting = true;
            paintRect(printMode, p, r, mag);
            MScore::pdfPrinting = false;
            }
      else if (ext == "png") {
            QImage printer(getRectImage(r, convDpi, transparent, printMode));
            printer.save(fn, "png");
            }
      else
            qDebug("unknown extension <%s>", qPrintable(ext));
      MScore::pixelRatio = pr;
      return true;
      }

//---------------------------------------------------------
//   paintRect
//---------------------------------------------------------

void ScoreView::paintRect(bool printMode, QPainter& p, const QRectF& r, double mag)
      {
      p.scale(mag, mag);
      p.translate(-r.topLeft());
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      score()->setPrinting(printMode);

      foreach (Page* page, _score->pages()) {
            // QRectF pr(page->abbox());
            QRectF pr(page->canvasBoundingRect());
            if (pr.right() < r.left())
                  continue;
            if (pr.left() > r.right())
                  break;
            p.translate(page->pos());
            QList<Element*> ell = page->items(r.translated(-page->pos()));
            drawElements(p, ell, nullptr);
            p.translate(-page->pos());
            }

      score()->setPrinting(false);
      p.end();
      }

//---------------------------------------------------------
//   fotoDragDrop
//---------------------------------------------------------

void ScoreView::fotoDragDrop(QMouseEvent*)
      {
      bool printMode   = true;
      QRectF r(_foto->bbox());

      QTemporaryFile tf(QDir::tempPath() + QString("/imgXXXXXX.svg"));
      tf.setAutoRemove(false);  // TODO: find out whether, where, when and how to delete it
      tf.open();
      tf.close();
      qDebug("Temp File <%s>", qPrintable(tf.fileName()));

//      QString fn = "/home/ws/mops.eps";
      QString fn = tf.fileName();

      int w = lrint(r.width());
      int h = lrint(r.height());
      SvgGenerator printer;
      printer.setFileName(fn);
      printer.setTitle(_score->title());
      printer.setSize(QSize(w, h));
      printer.setViewBox(QRect(0, 0, w, h));
      QPainter p(&printer);
      MScore::pdfPrinting = true;
      paintRect(printMode, p, r, 1);
      MScore::pdfPrinting = false;

      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;

      QUrl url = QUrl::fromLocalFile(fn);
      QList<QUrl> ul;
      ul.append(url);
      mimeData->setUrls(ul);

      drag->setMimeData(mimeData);
      drag->exec(Qt::CopyAction);
      }
}


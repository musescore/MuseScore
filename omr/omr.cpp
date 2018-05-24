//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "omr.h"
#include "omrview.h"
#include "libmscore/xml.h"
#include "libmscore/sym.h"
#include "omrpage.h"
#include "pdf.h"
#ifdef OCR
#include "ocr.h"
#endif
#include "utils.h"
#include "pattern.h"

namespace Ms {

#ifdef OMR

class ScoreView;

Pattern* Omr::quartheadPattern;
Pattern* Omr::halfheadPattern;
Pattern* Omr::sharpPattern;
Pattern* Omr::flatPattern;
Pattern* Omr::naturalPattern;
Pattern* Omr::trebleclefPattern;
Pattern* Omr::bassclefPattern;
Pattern* Omr::timesigPattern[10];

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

Omr::Omr(Score* s)
      {
      _score = s;
#ifdef OCR
      _ocr = 0;
#endif
      ActionNames = QList<QString>() << QWidget::tr("Loading PDF") << QWidget::tr("Initializing Staves") << QWidget::tr("Identifying Systems");
      initUtils();
      }

Omr::Omr(const QString& p, Score* s)
      {
      _score        = s;
      _path         = p;
      _ocr          = 0;
      ActionNames = QList<QString>()<< QWidget::tr("Loading PDF") << QWidget::tr("Initializing Staves") << QWidget::tr("Load Parameters") << QWidget::tr("Identifying Systems");
      initUtils();
      }

//---------------------------------------------------------
//   newOmrView
//---------------------------------------------------------

OmrView* Omr::newOmrView(ScoreView* sv)
      {
      OmrView* ov = new OmrView(sv);
      ov->setOmr(this);
      return ov;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Omr::write(XmlWriter& xml) const
      {
      xml.stag("Omr");
      xml.tag("path", _path);
      xml.tag("spatium", _spatium);
      xml.tag("dpmm", _dpmm);
      for(OmrPage* page : _pages) {
            page->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Omr::read(XmlReader& e)
      {
      _doc = 0;
#ifdef OCR
      if (_ocr == 0)
            _ocr = new Ocr;
      _ocr->init();
#endif
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "path")
                  _path = e.readElementText();
            else if (tag == "OmrPage") {
                  OmrPage* page = new OmrPage(this);
                  page->read(e);
                  _pages.append(page);
                  }
            else if (tag == "spatium")
                  _spatium = e.readDouble();
            else if (tag == "dpmm")
                  _dpmm = e.readDouble();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   pagesInDocument
//---------------------------------------------------------

int Omr::pagesInDocument() const
      {
      return _doc ? _doc->numPages() : 0;
      }

//---------------------------------------------------------
//   readPdf
//    return true on success
//---------------------------------------------------------

bool Omr::readPdf()
      {
      QProgressDialog *progress = new QProgressDialog(QWidget::tr("Reading PDF..."), QWidget::tr("Cancel"), 0, 100, 0, Qt::FramelessWindowHint);
      progress->setWindowModality(Qt::ApplicationModal);
      progress->show();
      progress->setRange(0, ACTION_NUM);

#ifdef OCR
      if (_ocr == 0)
            _ocr = new Ocr;
      _ocr->init();
#endif
      int ID = READ_PDF;
      int page = 0;
      bool val;
      while (ID < ACTION_NUM) {
            if(ID != INIT_PAGE && ID != SYSTEM_IDENTIFICATION) {
                  page = 0;
                  progress->setLabelText(QWidget::tr("%1 at Page %2").arg(ActionNames.at(ID+1)).arg(1));
                  val = omrActions(ID, page);
                  }
            else {
                  progress->setLabelText(QWidget::tr("%1 at Page %2").arg(ActionNames.at(ID)).arg(page+1));
                  val = omrActions(ID, page);
                  page++;
                  }

            if (!val || progress->wasCanceled()) {
                  progress->close();
                  return false;
                  }
            else {
                  if (ID < ACTION_NUM)
                        progress->setValue(ID);
                  else
                        progress->setValue(ACTION_NUM - 1);
                  qApp->processEvents();
                  }
            }
      progress->close();
      delete progress;
      return true;
      }

//---------------------------------------------------------
//   actions
//---------------------------------------------------------

bool Omr::omrActions(int &ID, int page)
      {
      if(ID == READ_PDF) {
            _doc = new Pdf();
            if (!_doc->open(_path)) {
                  delete _doc;
                  _doc = 0;
                  return false;
                  }
            int n = _doc->numPages();
            printf("readPdf: %d pages\n", n);
            for (int i = 0; i < n; ++i) {
                  OmrPage* page1 = new OmrPage(this);
                  QImage image = _doc->page(i);
                  if (image.isNull())
                        return false;
                  page1->setImage(image);
                  _pages.append(page1);
                  }

            _spatium = 15.0; //constant spatium, image will be rescaled according to this parameter
            ID++;
            return true;
            }
      else if(ID == INIT_PAGE) {
            //load one page and rescale
            _pages[page]->read();

            //do the rescaling here
            int new_w = _pages[page]->image().width() * _spatium/_pages[page]->spatium();
            int new_h = _pages[page]->image().height() * _spatium/_pages[page]->spatium();
            QImage image = _pages[page]->image().scaled(new_w,new_h, Qt::KeepAspectRatio);
            _pages[page]->setImage(image);
            _pages[page]->read();

            if(page == _pages.size()-1)
                  ID++;
            return true;
            }
      else if(ID == FINALIZE_PARMS) {
            int n = _pages.size();
            double w = 0;
            for (int i = 0; i < n; ++i) {
                  w  += _pages[i]->width();
                  }
            w       /= n;
            _dpmm    = w / 210.0;            // PaperSize A4

            quartheadPattern  = new Pattern(_score, "solid_note_head");
            halfheadPattern   = new Pattern(_score, SymId::noteheadHalf,  _spatium);
            sharpPattern      = new Pattern(_score, SymId::accidentalSharp, _spatium);
            flatPattern       = new Pattern(_score, SymId::accidentalFlat, _spatium);
            naturalPattern    = new Pattern(_score, SymId::accidentalNatural,_spatium);
            trebleclefPattern = new Pattern(_score, SymId::gClef,_spatium);
            bassclefPattern   = new Pattern(_score, SymId::fClef,_spatium);
            timesigPattern[0] = new Pattern(_score, SymId::timeSig0, _spatium);
            timesigPattern[1] = new Pattern(_score, SymId::timeSig1, _spatium);
            timesigPattern[2] = new Pattern(_score, SymId::timeSig2, _spatium);
            timesigPattern[3] = new Pattern(_score, SymId::timeSig3, _spatium);
            timesigPattern[4] = new Pattern(_score, SymId::timeSig4, _spatium);
            timesigPattern[5] = new Pattern(_score, SymId::timeSig5, _spatium);
            timesigPattern[6] = new Pattern(_score, SymId::timeSig6, _spatium);
            timesigPattern[7] = new Pattern(_score, SymId::timeSig7, _spatium);
            timesigPattern[8] = new Pattern(_score, SymId::timeSig8, _spatium);
            timesigPattern[9] = new Pattern(_score, SymId::timeSig9, _spatium);

            ID++;
            return true;

            }
      else if(ID == SYSTEM_IDENTIFICATION) {
            _pages[page]->identifySystems();
            if(page == _pages.size()-1) ID++;
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   spatiumMM
//---------------------------------------------------------

double Omr::spatiumMM() const
      {
      return _spatium / _dpmm;
      }

//---------------------------------------------------------
//   staffDistance
//---------------------------------------------------------

double Omr::staffDistance() const
      {
      return _pages[0]->staffDistance();
      }

//---------------------------------------------------------
//   systemDistance
//---------------------------------------------------------

double Omr::systemDistance() const
      {
      return _pages[0]->systemDistance();
      }
#endif
}


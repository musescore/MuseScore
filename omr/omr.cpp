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

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 27d1d6c... debug pattern match
Pattern* Omr::quartheadPattern;
Pattern* Omr::halfheadPattern;
Pattern* Omr::sharpPattern;
Pattern* Omr::flatPattern;
Pattern* Omr::naturalPattern;
Pattern* Omr::trebleclefPattern;
Pattern* Omr::bassclefPattern;
<<<<<<< HEAD
<<<<<<< HEAD
Pattern* Omr::timesigPattern[10];
=======
//Pattern* Omr::quartheadPattern;
//Pattern* Omr::halfheadPattern;
//Pattern* Omr::sharpPattern;
//Pattern* Omr::flatPattern;
//Pattern* Omr::naturalPattern;
//Pattern* Omr::trebleclefPattern;
//Pattern* Omr::bassclefPattern;
>>>>>>> 57b9dad... compile omr module
=======
>>>>>>> 27d1d6c... debug pattern match
=======
Pattern* Omr::timesigPattern[10];
>>>>>>> 40265ac... fix bugs in search clef,timesig and keys

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

Omr::Omr(Score* s)
      {
      _score = s;
#ifdef OCR
      _ocr = 0;
#endif
          ActionNames = QList<QString>()<< QWidget::tr("Loading Pdf") << QWidget::tr("Initializing Staves") << QWidget::tr("Identifying Systems");
      initUtils();
      }

Omr::Omr(const QString& p, Score* s)
      {
      _score        = s;
      _path         = p;
      _ocr          = 0;
          ActionNames = QList<QString>()<< QWidget::tr("Loading Pdf") << QWidget::tr("Initializing Staves") << QWidget::tr("Identifying Systems");
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

void Omr::write(Xml& xml) const
      {
      xml.stag("Omr");
      xml.tag("path", _path);
      xml.tag("spatium", _spatium);
      xml.tag("dpmm", _dpmm);
      foreach(OmrPage* page, _pages) {
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
<<<<<<< HEAD
<<<<<<< HEAD
          QProgressDialog progress;
=======
          
>>>>>>> d065ed4... add progress dialog to the omr process
=======
          QProgressDialog progress;
>>>>>>> 8d0232d... debug skeleton creation
          //set up progress dialog
          progress.setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
          progress.setWindowModality(Qt::ApplicationModal);
          //progress.setCancelButton(0);
          progress.setCancelButtonText(QWidget::tr("Cancel"));
          //progress.setLabelText(QWidget::tr("Importing..."));
          progress.show();
          progress.setRange(0, ACTION_NUM);
          
          
<<<<<<< HEAD
=======
          
>>>>>>> d065ed4... add progress dialog to the omr process
#ifdef OCR
      if (_ocr == 0)
            _ocr = new Ocr;
      _ocr->init();
#endif
<<<<<<< HEAD

=======
>>>>>>> d065ed4... add progress dialog to the omr process
      int ID;
          for(ID = 0; ID < ACTION_NUM; ID++){
              progress.setLabelText(ActionNames.at(ID));
              bool val = actions(ID);
              if(!val || progress.wasCanceled()){
                  progress.close();
                  return false;
              }
              else{
<<<<<<< HEAD
<<<<<<< HEAD
                  if(ID + 1 < ACTION_NUM){
                      progress.setLabelText(ActionNames.at(ID+1));
                      progress.setValue(ID+1);
                  }
                  qApp->processEvents();
              }
          }
          progress.close();
=======
                  if(ID + 1 < ACTION_NUM)
=======
                  if(ID + 1 < ACTION_NUM){
>>>>>>> 8d0232d... debug skeleton creation
                      progress.setLabelText(ActionNames.at(ID+1));
                      progress.setValue(ID+1);
                  }
                  qApp->processEvents();
              }
          }
<<<<<<< HEAD
>>>>>>> d065ed4... add progress dialog to the omr process
=======
          progress.close();
>>>>>>> 8d0232d... debug skeleton creation
          return true;
      }

//---------------------------------------------------------
//   actions
//---------------------------------------------------------
    
bool Omr::actions(int ID)
    {
        if(ID == READ_PDF){
            _doc = new Pdf();
            if (!_doc->open(_path)) {
                delete _doc;
                _doc = 0;
                return false;
            }
            int n = _doc->numPages();
            printf("readPdf: %d pages\n", n);
            for (int i = 0; i < n; ++i) {
                OmrPage* page = new OmrPage(this);
                QImage image = _doc->page(i);
                page->setImage(image);
                _pages.append(page);
            }
            return true;
        }
<<<<<<< HEAD
<<<<<<< HEAD

=======
>>>>>>> d065ed4... add progress dialog to the omr process
        else if(ID == INIT_PARMS){
            double sp = 0;
            double w  = 0;
            
            int pages = 0;
            int n = _pages.size();
<<<<<<< HEAD
            double spatium_constant = 15.0;
=======
>>>>>>> d065ed4... add progress dialog to the omr process
            
            for (int i = 0; i < n; ++i) {
                _pages[i]->read();
                //            if (_pages[i]->systems().size() > 0) {
                //                sp += _pages[i]->spatium();
                //                ++pages;
                //            }
<<<<<<< HEAD
                
                sp += _pages[i]->spatium();
                ++pages;
                
//                //do the rescaling of image here
                int new_w = _pages[i]->image().width()*spatium_constant/_pages[i]->spatium();
                int new_h = _pages[i]->image().height()*spatium_constant/_pages[i]->spatium();
                QImage image = _pages[i]->image().scaled(new_w ,new_h, Qt::KeepAspectRatio);
                _pages[i]->setImage(image);
                _pages[i]->read();
                w  += _pages[i]->width();
            }
<<<<<<< HEAD
            _spatium = spatium_constant; //sp / pages;
            w       /= n;
            _dpmm    = w / 210.0;            // PaperSize A4
            
            
            
            // printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);
            //quartheadPattern  = new Pattern(_score, SymId::noteheadBlack,  _spatium);
            quartheadPattern  = new Pattern(_score, "solid_note_head");
=======
                sp += _pages[i]->spatium();
                ++pages;
                w  += _pages[i]->width();
            }
            _spatium = sp / pages;
            w       /= n;
            _dpmm    = w / 210.0;            // PaperSize A4
            
            // printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);
            quartheadPattern  = new Pattern(_score, SymId::noteheadBlack,  _spatium);
>>>>>>> d065ed4... add progress dialog to the omr process
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
            return true;
<<<<<<< HEAD
        }
        else if(ID == SYSTEM_IDENTIFICATION){
            int n = _pages.size();
            for (int i = 0; i < n; ++i) {
                _pages[i]->identifySystems();
=======
      _spatium = sp / pages;
      w       /= n;
      _dpmm    = w / 210.0;            // PaperSize A4

// printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);

      quartheadPattern  = new Pattern(_score, SymId::noteheadBlack,  _spatium);
      halfheadPattern   = new Pattern(_score, SymId::noteheadHalf,  _spatium);
      sharpPattern      = new Pattern(_score, SymId::accidentalSharp, _spatium);
      flatPattern       = new Pattern(_score, SymId::accidentalFlat, _spatium);
      naturalPattern    = new Pattern(_score, SymId::accidentalNatural,_spatium);
      trebleclefPattern = new Pattern(_score, SymId::gClef,_spatium);
      bassclefPattern   = new Pattern(_score, SymId::fClef,_spatium);
          timesigPattern[0] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[1] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[2] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[3] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[4] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[5] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[6] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[7] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[8] = new Pattern(_score, SymId::timeSig0, _spatium);
          timesigPattern[9] = new Pattern(_score, SymId::timeSig0, _spatium);

      for (int i = 0; i < n; ++i) {
            OmrPage* page = _pages[i];
            if (!page->systems().isEmpty()) {
                  page->readBarLines(i);
                  }
>>>>>>> 57b9dad... compile omr module
            }
            
            
            
//            for (int i = 0; i < n; ++i) {
//
            
            
            OmrPage* page = _pages[i];
//                if (!page->systems().isEmpty()) {
//                    page->readBarLines(i);
//                }
//            }
            return true;
=======
        _spatium = sp / pages;
        w       /= n;
        _dpmm    = w / 210.0;            // PaperSize A4
        
        // printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);
        quartheadPattern  = new Pattern(_score, SymId::noteheadBlack,  _spatium);
        halfheadPattern   = new Pattern(_score, SymId::noteheadHalf,  _spatium);
        sharpPattern      = new Pattern(_score, SymId::accidentalSharp, _spatium);
        flatPattern       = new Pattern(_score, SymId::accidentalFlat, _spatium);
        naturalPattern    = new Pattern(_score, SymId::accidentalNatural,_spatium);
        trebleclefPattern = new Pattern(_score, SymId::gClef,_spatium);
        bassclefPattern   = new Pattern(_score, SymId::fClef,_spatium);
        timesigPattern[0] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[1] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[2] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[3] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[4] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[5] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[6] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[7] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[8] = new Pattern(_score, SymId::timeSig0, _spatium);
        timesigPattern[9] = new Pattern(_score, SymId::timeSig0, _spatium);
        
        
        for (int i = 0; i < n; ++i) {
            _pages[i]->identifySystems();
=======
>>>>>>> d065ed4... add progress dialog to the omr process
        }
        else if(ID == SYSTEM_IDENTIFICATION){
            int n = _pages.size();
            for (int i = 0; i < n; ++i) {
                _pages[i]->identifySystems();
            }
            
<<<<<<< HEAD
            for (int i = 0; i < n; ++i) {
                OmrPage* page = _pages[i];
                if (!page->systems().isEmpty()) {
                    page->readBarLines(i);
                }
            }
<<<<<<< HEAD
>>>>>>> 4615c3e... consider different number of staves in systems
=======
=======
//            for (int i = 0; i < n; ++i) {
//                OmrPage* page = _pages[i];
//                if (!page->systems().isEmpty()) {
//                    page->readBarLines(i);
//                }
//            }
>>>>>>> 21738fc... debugging omr
            return true;
>>>>>>> d065ed4... add progress dialog to the omr process
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


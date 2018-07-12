//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: newwizard.h 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __NEWWIZARD_H__
#define __NEWWIZARD_H__

#include "ui_timesigwizard.h"
#include "ui_newwizard.h"

#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/fraction.h"

namespace Ms {

class Score;
class Palette;
class StaffListItem;
class InstrumentsWidget;
class ScoreBrowser;

//---------------------------------------------------------
//   TimesigWizard
//---------------------------------------------------------

class TimesigWizard : public QWidget, private Ui::TimesigWizard {
      Q_OBJECT

   private slots:
      void commonTimeToggled(bool);
      void cutTimeToggled(bool);
      void fractionToggled(bool);

   public:
      TimesigWizard(QWidget* parent = 0);
      int measures() const;
      void setMeasures(int m) { measureCount->setValue(m); }
      Fraction timesig() const;
      bool pickup(int* z, int* n) const;
      TimeSigType type() const;
      };

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

class TitleWizard : public QWidget, public Ui::NewWizard {
      Q_OBJECT

   private:
      bool moreOptionsVisible;

   private slots:
      void setMoreOptionsVisible(bool visible = true);

   public:
      TitleWizard(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

class NewWizardPage1 : public QWizardPage {
      Q_OBJECT

      TitleWizard* w;

   public:
      NewWizardPage1(QWidget* parent = nullptr);
      QString title() const          { return w->lineEditTitle->text(); }
      QString movementTitle() const  { return w->lineEditMovementTitle->text(); }
      QString movementNumber() const { return w->lineEditMovementNumber->text(); }
      QString subtitle() const       { return w->lineEditSubtitle->text(); }
      QString composer() const       { return w->lineEditComposer->text(); }
      void setComposer(QString c) const { w->lineEditComposer->setText(c); }
      QString arranger() const       { return w->lineEditArranger->text(); }
      QString lyricist() const       { return w->lineEditLyricist->text(); }
      void setLyricist(QString l) const { w->lineEditLyricist->setText(l); }
      QString poet() const           { return w->lineEditPoet->text(); }
      QString workNumber() const     { return w->lineEditWorkNumber->text(); }
      QString translator() const     { return w->lineEditTranslator->text(); }
      QString source() const         { return w->lineEditSource->text(); }
      QString copyright() const      { return w->lineEditCopyright->text(); }
      void setCopyright(QString c) const { w->lineEditCopyright->setText(c); }
      virtual void initializePage() override;
      };

//---------------------------------------------------------
//   NewWizardPage2
//---------------------------------------------------------

class NewWizardPage2 : public QWizardPage {
      Q_OBJECT

      bool complete;
      InstrumentsWidget* w;

   public slots:
      void setComplete(bool);

   public:
      NewWizardPage2(QWidget* parent = 0);
      virtual bool isComplete() const override;
      void createInstruments(Score* s);
      virtual void initializePage() override;
      };

//---------------------------------------------------------
//   NewWizardPage3
//---------------------------------------------------------

class NewWizardPage3 : public QWizardPage {
      Q_OBJECT

      TimesigWizard* w;

   public:
      NewWizardPage3(QWidget* parent = 0);
      int measures() const                     { return w->measures();   }
      void setMeasures(int m)                  { w->setMeasures(m);      }
      Fraction timesig() const                 { return w->timesig();    }
      bool pickupMeasure(int* z, int* n) const { return w->pickup(z, n); }
      TimeSigType timesigType() const          { return w->type();       }
      };

//---------------------------------------------------------
//   NewWizardPage4
//    request template file
//---------------------------------------------------------

class NewWizardPage4 : public QWizardPage {
      Q_OBJECT

      ScoreBrowser* templateFileBrowser;
      QString path;

   private slots:
      void templateChanged(const QString&);
      void fileAccepted(const QString&);

   public:
      NewWizardPage4(QWidget* parent = 0);
      virtual bool isComplete() const override;
      QString templatePath() const;
      virtual void initializePage();
      void buildTemplatesList();
      };

//---------------------------------------------------------
//   NewWizardPage5
//---------------------------------------------------------

class NewWizardPage5 : public QWizardPage {
      Q_OBJECT

      Palette* sp;
      QDoubleSpinBox* _tempo;
      QGroupBox* tempoGroup;

   public:
      NewWizardPage5(QWidget* parent = 0);
      virtual bool isComplete() const override { return true; }
      KeySigEvent keysig() const;
      double tempo() const              { return _tempo->value(); }
      void setTempo(double t) const     { _tempo->setValue(t); }
      bool createTempo() const          { return tempoGroup->isChecked(); }
      void setCreateTempo(bool c) const { tempoGroup->setChecked(c); }
      void init();
      };

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

class NewWizard : public QWizard {
      Q_OBJECT

      NewWizardPage1* p1;
      NewWizardPage2* p2;
      NewWizardPage3* p3;
      NewWizardPage4* p4;
      NewWizardPage5* p5;

      void readSettings();
      virtual void hideEvent(QHideEvent*) override;

private slots:
      void writeSettings();
      void idChanged(int);

   public:
      NewWizard(QWidget* parent = 0);
      friend class QWizardPage;
      virtual int nextId() const;

      enum Page { Invalid = -1, Type, Instruments, Template, Keysig, Timesig};

      QString templatePath() const       { return p4->templatePath(); }
      int measures() const               { return p3->measures();    }
      Fraction timesig() const           { return p3->timesig();     }
      void createInstruments(Score* s)   { p2->createInstruments(s); }
      NewWizardPage1* metaTagsPage()        { return p1; }
      KeySigEvent keysig() const         { return p5->keysig();         }
      bool pickupMeasure(int* z, int* n) const { return p3->pickupMeasure(z, n); }
      TimeSigType timesigType() const     { return p3->timesigType(); }
      double tempo() const                { return p5->tempo();       }
      bool createTempo() const            { return p5->createTempo(); }
      bool emptyScore() const;
      void updateValues() const;
      };

} // namespace Ms
#endif


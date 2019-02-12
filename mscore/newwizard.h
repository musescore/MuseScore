//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#include <QWizard>

#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/fraction.h"

namespace Ms {

class Score;
class Palette;
class StaffListItem;
class InstrumentsWidget;
class TemplateBrowser;

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
      Fraction timesig() const;
      bool pickup(int* z, int* n) const;
      TimeSigType type() const;
      };

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

class TitleWizard : public QWidget, public Ui::NewWizard {
      Q_OBJECT

   public:
      TitleWizard(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   NewWizardInfoPage
//    Enter score information such as title and composer
//---------------------------------------------------------

class NewWizardInfoPage : public QWizardPage {
      Q_OBJECT

      TitleWizard* w;

   public:
      NewWizardInfoPage(QWidget* parent = 0);
      QString title() const              { return w->title->text();      }
      QString subtitle() const           { return w->subtitle->text();   }
      QString composer() const           { return w->composer->text();   }
      QString poet() const               { return w->poet->text();       }
      QString copyright() const          { return w->copyright->text();  }
      virtual void initializePage() override;
      };

//---------------------------------------------------------
//   NewWizardInstrumentsPage
//    Choose instruments to appear in the score
//---------------------------------------------------------

class NewWizardInstrumentsPage : public QWizardPage {
      Q_OBJECT

      bool complete;
      InstrumentsWidget* instrumentsWidget;

   public slots:
      void setComplete(bool);

   public:
      NewWizardInstrumentsPage(QWidget* parent = 0);
      virtual bool isComplete() const override;
      void createInstruments(Score* s);
      virtual void initializePage() override;
      };

//---------------------------------------------------------
//   NewWizardTimesigPage
//    Choose time signature for the score
//---------------------------------------------------------

class NewWizardTimesigPage : public QWizardPage {
      Q_OBJECT

      TimesigWizard* w;

   public:
      NewWizardTimesigPage(QWidget* parent = 0);
      int measures() const                     { return w->measures();   }
      Fraction timesig() const                 { return w->timesig();    }
      bool pickupMeasure(int* z, int* n) const { return w->pickup(z, n); }
      TimeSigType timesigType() const          { return w->type();       }
      };

//---------------------------------------------------------
//   NewWizardTemplatePage
//    Choose a template on which to base the score
//---------------------------------------------------------

class NewWizardTemplatePage : public QWizardPage {
      Q_OBJECT

      TemplateBrowser* templateFileBrowser;
      QString path;

   private slots:
      void templateChanged(const QString&);
      void fileAccepted(const QString&);

   public:
      NewWizardTemplatePage(QWidget* parent = 0);
      virtual bool isComplete() const override;
      QString templatePath() const;
      virtual void initializePage();
      void buildTemplatesList();
      };

//---------------------------------------------------------
//   NewWizardKeysigPage
//    Choose key signature for the score
//---------------------------------------------------------

class NewWizardKeysigPage : public QWizardPage {
      Q_OBJECT

      Palette* sp;
      QDoubleSpinBox* _tempo;
      QGroupBox* tempoGroup;

   public:
      NewWizardKeysigPage(QWidget* parent = 0);
      virtual bool isComplete() const override { return true; }
      KeySigEvent keysig() const;
      double tempo() const            { return _tempo->value(); }
      bool createTempo() const        { return tempoGroup->isChecked(); }
      void init();
      };

//---------------------------------------------------------
//   NewWizard
//    New Score Wizard - create a new score
//---------------------------------------------------------

class NewWizard : public QWizard {
      Q_OBJECT

      NewWizardInfoPage* infoPage;
      NewWizardInstrumentsPage* instrumentsPage;
      NewWizardTimesigPage* timesigPage;
      NewWizardTemplatePage* templatePage;
      NewWizardKeysigPage* keysigPage;

      virtual void hideEvent(QHideEvent*);

   private slots:
      void idChanged(int);

   public:
      NewWizard(QWidget* parent = 0);
      friend class QWizardPage;
      virtual int nextId() const;

      enum Page { Invalid = -1, Type, Instruments, Template, Keysig, Timesig};

      QString templatePath() const       { return templatePage->templatePath(); }
      int measures() const               { return timesigPage->measures();    }
      Fraction timesig() const           { return timesigPage->timesig();     }
      void createInstruments(Score* s)   { instrumentsPage->createInstruments(s); }
      QString title() const              { return infoPage->title();       }
      QString subtitle() const           { return infoPage->subtitle();    }
      QString composer() const           { return infoPage->composer();    }
      QString poet() const               { return infoPage->poet();        }
      QString copyright() const          { return infoPage->copyright();   }
      KeySigEvent keysig() const         { return keysigPage->keysig();      }
      bool pickupMeasure(int* z, int* n) const { return timesigPage->pickupMeasure(z, n); }
      TimeSigType timesigType() const     { return timesigPage->timesigType();       }
      double tempo() const                { return keysigPage->tempo();       }
      bool createTempo() const            { return keysigPage->createTempo(); }
      bool emptyScore() const;
      void updateValues() const;
      };


} // namespace Ms
#endif


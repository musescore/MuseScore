
//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: newwizard.cpp 5626 2012-05-13 18:33:52Z lasconic $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "newwizard.h"
#include "musescore.h"
#include "preferences.h"
#include "palette.h"
#include "instrdialog.h"
#include "scoreBrowser.h"
#include "extension.h"

#include "libmscore/instrtemplate.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/clef.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/stafftype.h"
#include "libmscore/timesig.h"
#include "libmscore/sym.h"

namespace Ms {

extern Palette* newKeySigPalette();
extern void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString(""));

//---------------------------------------------------------
//   TimesigWizard
//---------------------------------------------------------

TimesigWizard::TimesigWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(tsCommonTime, SIGNAL(toggled(bool)), SLOT(commonTimeToggled(bool)));
      connect(tsCutTime,    SIGNAL(toggled(bool)), SLOT(cutTimeToggled(bool)));
      connect(tsFraction,   SIGNAL(toggled(bool)), SLOT(fractionToggled(bool)));
      }

//---------------------------------------------------------
//   measures
//---------------------------------------------------------

int TimesigWizard::measures() const
      {
      return measureCount->value();
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

Fraction TimesigWizard::timesig() const
      {
      if (tsFraction->isChecked())
            return Fraction(timesigZ->value(), 1 << timesigN->currentIndex());
      else if (tsCommonTime->isChecked())
            return Fraction(4, 4);
      else
            return Fraction(2, 2);
      }

//---------------------------------------------------------
//   pickupMeasure
//---------------------------------------------------------

bool TimesigWizard::pickup(int* z, int* n) const
      {
      *z = pickupTimesigZ->value();
      *n = 1 << pickupTimesigN->currentIndex();
      return pickupMeasure->isChecked();
      }

//---------------------------------------------------------
//   type
//---------------------------------------------------------

TimeSigType TimesigWizard::type() const
      {
      if (tsFraction->isChecked())
            return TimeSigType::NORMAL;
      if (tsCommonTime->isChecked())
            return TimeSigType::FOUR_FOUR;
      return TimeSigType::ALLA_BREVE;
      }

//---------------------------------------------------------
//   commonTimeToggled
//---------------------------------------------------------

void TimesigWizard::commonTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(4);
            // timesigN->setValue(4);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   cutTimeToggled
//---------------------------------------------------------

void TimesigWizard::cutTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(2);
            // timesigN->setValue(2);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   fractionToggled
//---------------------------------------------------------

void TimesigWizard::fractionToggled(bool val)
      {
      if (val) {
            timesigZ->setEnabled(true);
            timesigN->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

TitleWizard::TitleWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

NewWizardPage1::NewWizardPage1(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Enter score information:"));
      setAccessibleName(QWizardPage::title());
      setAccessibleDescription(QWizardPage::subTitle());

      w = new TitleWizard;

      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage1::initializePage()
      {
      w->title->setText("");
      w->subtitle->setText("");
      }

//---------------------------------------------------------
//   NewWizardPage2
//---------------------------------------------------------

NewWizardPage2::NewWizardPage2(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose instruments on the left to add to instrument list on the right:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());
      w        = new InstrumentsWidget;
      QGridLayout* grid = new QGridLayout;
      grid->setSpacing(0);
      grid->setContentsMargins(0, 0, 0, 0);
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      connect(w, SIGNAL(completeChanged(bool)), SLOT(setComplete(bool)));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage2::initializePage()
      {
      complete = false;
      w->init();
      }

//---------------------------------------------------------
//   setComplete
//---------------------------------------------------------

void NewWizardPage2::setComplete(bool val)
      {
      complete = val;
      emit completeChanged();
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardPage2::isComplete() const
      {
      return complete;
      }

//---------------------------------------------------------
//   createInstruments
//---------------------------------------------------------

void NewWizardPage2::createInstruments(Score* s)
      {
      w->createInstruments(s);
      }

//---------------------------------------------------------
//   NewWizardPage3
//---------------------------------------------------------

NewWizardPage3::NewWizardPage3(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose time signature:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      w = new TimesigWizard;
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

NewWizardPage4::NewWizardPage4(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose template file:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      templateFileBrowser = new ScoreBrowser;
      templateFileBrowser->setStripNumbers(true);
      templateFileBrowser->setShowCustomCategory(true);
      templateFileBrowser->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
      buildTemplatesList();

      QVBoxLayout* layout = new QVBoxLayout;
      QHBoxLayout* searchLayout = new QHBoxLayout;
      QLineEdit* search = new QLineEdit;
      search->setPlaceholderText(tr("Search"));
      search->setClearButtonEnabled(true);
      search->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      searchLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
      searchLayout->addWidget(search);

      layout->addLayout(searchLayout);
      layout->addWidget(templateFileBrowser);
      setLayout(layout);

      connect(templateFileBrowser, SIGNAL(scoreSelected(const QString&)), SLOT(templateChanged(const QString&)));
      connect(templateFileBrowser, SIGNAL(scoreActivated(const QString&)), SLOT(fileAccepted(const QString&)));
      connect(search, &QLineEdit::textChanged, [this] (const QString& searchString) {
            this->templateFileBrowser->filter(searchString);
            });
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage4::initializePage()
      {
      templateFileBrowser->show();
      path.clear();
      }

//---------------------------------------------------------
//   buildTemplatesList
//---------------------------------------------------------

void NewWizardPage4::buildTemplatesList()
      {

      QDir dir(mscoreGlobalShare + "/templates");
      QFileInfoList fil = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Dirs | QDir::Files, QDir::Name);
      if(fil.isEmpty()){
          fil.append(QFileInfo(QFile(":data/Empty_Score.mscz")));
          }

      QDir myTemplatesDir(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
      fil.append(myTemplatesDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Dirs | QDir::Files, QDir::Name));

      // append templates directories from extensions
      QStringList extensionsDir = Extension::getDirectoriesByType(Extension::templatesDir);
      for (QString extDir : extensionsDir) {
            QDir extTemplateDir(extDir);
            fil.append(extTemplateDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Dirs | QDir::Files, QDir::Name));
            }
      templateFileBrowser->setScores(fil);
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardPage4::isComplete() const
      {
      return !path.isEmpty();
      }

//---------------------------------------------------------
//   fileAccepted
//---------------------------------------------------------

void NewWizardPage4::fileAccepted(const QString& s)
      {
      path = s;
      templateFileBrowser->show();
      if (wizard()->currentPage() == this)
            wizard()->next();
      }

//---------------------------------------------------------
//   templateChanged
//---------------------------------------------------------

void NewWizardPage4::templateChanged(const QString& s)
      {
      path = s;
      emit completeChanged();
      }

//---------------------------------------------------------
//   templatePath
//---------------------------------------------------------

QString NewWizardPage4::templatePath() const
      {
      return path;
      }

//---------------------------------------------------------
//   NewWizardPage5
//---------------------------------------------------------

NewWizardPage5::NewWizardPage5(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose key signature and tempo:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      QGroupBox* b1 = new QGroupBox;
      b1->setTitle(tr("Key Signature"));
      b1->setAccessibleName(title());
      sp = MuseScore::newKeySigPalette();
      sp->setMoreElements(false);
      sp->setShowContextMenu(false);
      sp->setSelectable(true);
      sp->setDisableDoubleClick(true);
      sp->setSelected(14);
      PaletteScrollArea* sa = new PaletteScrollArea(sp);
      QVBoxLayout* l1 = new QVBoxLayout;
      l1->addWidget(sa);
      b1->setLayout(l1);

      tempoGroup = new QGroupBox;
      tempoGroup->setCheckable(true);
      tempoGroup->setChecked(false);
      tempoGroup->setTitle(tr("Tempo"));
      QLabel* bpm = new QLabel;
      bpm->setText(tr("BPM:"));
      _tempo = new QDoubleSpinBox;
      _tempo->setAccessibleName(tr("Beats per minute"));
      _tempo->setRange(20.0, 400.0);
      _tempo->setValue(120.0);
      _tempo->setDecimals(1);
      QHBoxLayout* l2 = new QHBoxLayout;
      l2->addWidget(bpm);
      l2->addWidget(_tempo);
      l2->addStretch(100);
      tempoGroup->setLayout(l2);

      QVBoxLayout* l3 = new QVBoxLayout;
      l3->addWidget(b1);
      l3->addWidget(tempoGroup);
      l3->addStretch(100);
      setLayout(l3);
      setFocusPolicy(Qt::StrongFocus);
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

KeySigEvent NewWizardPage5::keysig() const
      {
      int idx    = sp->getSelectedIdx();
      Element* e = sp->element(idx);
      return static_cast<KeySig*>(e)->keySigEvent();
      }

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

NewWizard::NewWizard(QWidget* parent)
   : QWizard(parent)
      {
      setObjectName("NewWizard");
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWizardStyle(QWizard::ClassicStyle);
      setPixmap(QWizard::LogoPixmap, QPixmap(":/data/mscore.png"));
      setPixmap(QWizard::WatermarkPixmap, QPixmap());
      setWindowTitle(tr("New Score Wizard"));

      setOption(QWizard::NoCancelButton, false);
      setOption(QWizard::CancelButtonOnLeft, true);
      setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
      setOption(QWizard::HaveNextButtonOnLastPage, true);

      p1 = new NewWizardPage1;
      p2 = new NewWizardPage2;
      p3 = new NewWizardPage3;
      p4 = new NewWizardPage4;
      p5 = new NewWizardPage5;

//  enum Page { Invalid = -1, Type, Instruments, Template, Keysig, Timesig};

      setPage(Page::Type,        p1);
      setPage(Page::Instruments, p2);
      setPage(Page::Template,    p4);
      setPage(Page::Keysig,      p5);
      setPage(Page::Timesig,     p3);

      resize(QSize(840, 560)); //ensure default size if no geometry in settings
      MuseScore::restoreGeometry(this);
      connect(this, SIGNAL(currentIdChanged(int)), SLOT(idChanged(int)));
      }

//---------------------------------------------------------
//   idChanged
//---------------------------------------------------------

void NewWizard::idChanged(int /*id*/)
      {
      // printf("\n===\nWizard: id changed %d\n", id);
      }

//---------------------------------------------------------
//   nextId
//---------------------------------------------------------

int NewWizard::nextId() const
      {
      int next;
      switch (Page(currentId())) {
            case Page::Type:
                  next = Page::Template;
                  break;
            case Page::Template:
                  next = emptyScore() ? Page::Instruments : Page::Keysig;
                  break;
            case Page::Instruments:
                  next = Page::Keysig;
                  break;
            case Page::Keysig:
                  next = Page::Timesig;
                  break;
            case Page::Timesig:
            default:
                  next = Page::Invalid;
                  break;
            }
      return next;
      }

//---------------------------------------------------------
//   emptyScore
//---------------------------------------------------------

bool NewWizard::emptyScore() const
      {
      QString p = p4->templatePath();
      QFileInfo fi(p);
      bool val = fi.completeBaseName() == "00-Blank";
      return val;
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void NewWizard::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}


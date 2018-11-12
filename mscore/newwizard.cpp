
//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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
#include "templateBrowser.h"
#include "svgrenderer.h"
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
      pickupMeasure->setChecked(false); // checked in the UI file to enable screen reader on pickup duration controls
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
//   NewWizardInfoPage
//---------------------------------------------------------

NewWizardInfoPage::NewWizardInfoPage(QWidget* parent)
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

void NewWizardInfoPage::initializePage()
      {
      w->title->setText("");
      w->subtitle->setText("");
      }

//---------------------------------------------------------
//   NewWizardInstrumentsPage
//---------------------------------------------------------

NewWizardInstrumentsPage::NewWizardInstrumentsPage(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose instruments on the left to add to instrument list on the right:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());
      instrumentsWidget = new InstrumentsWidget;
      QGridLayout* grid = new QGridLayout;
      grid->setSpacing(0);
      grid->setContentsMargins(0, 0, 0, 0);
      grid->addWidget(instrumentsWidget, 0, 0);
      setLayout(grid);
      connect(instrumentsWidget, SIGNAL(completeChanged(bool)), SLOT(setComplete(bool)));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardInstrumentsPage::initializePage()
      {
      complete = false;
      instrumentsWidget->init();
      }

//---------------------------------------------------------
//   setComplete
//---------------------------------------------------------

void NewWizardInstrumentsPage::setComplete(bool val)
      {
      complete = val;
      emit completeChanged();
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardInstrumentsPage::isComplete() const
      {
      return complete;
      }

//---------------------------------------------------------
//   createInstruments
//---------------------------------------------------------

void NewWizardInstrumentsPage::createInstruments(Score* s)
      {
      instrumentsWidget->createInstruments(s);
      }

//---------------------------------------------------------
//   NewWizardTimesigPage
//---------------------------------------------------------

NewWizardTimesigPage::NewWizardTimesigPage(QWidget* parent)
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
//   NewWizardTemplatePage
//---------------------------------------------------------

NewWizardTemplatePage::NewWizardTemplatePage(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose template file:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      templateFileBrowser = new TemplateBrowser(this);
      templateFileBrowser->setStripNumbers(true);

      QVBoxLayout* layout = new QVBoxLayout;
      layout->addWidget(templateFileBrowser);
      setLayout(layout);

      connect(templateFileBrowser, SIGNAL(scoreSelected(const QString&)), SLOT(templateChanged(const QString&)));
      buildTemplatesList();
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardTemplatePage::initializePage()
      {

      }

//---------------------------------------------------------
//   buildTemplatesList
//---------------------------------------------------------

void NewWizardTemplatePage::buildTemplatesList()
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

bool NewWizardTemplatePage::isComplete() const
      {
      return !path.isEmpty();
      }

//---------------------------------------------------------
//   fileAccepted
//---------------------------------------------------------

void NewWizardTemplatePage::fileAccepted(const QString& s)
      {
      path = s;
      templateFileBrowser->show();
      if (wizard()->currentPage() == this)
            wizard()->next();
      }

//---------------------------------------------------------
//   templateChanged
//---------------------------------------------------------

void NewWizardTemplatePage::templateChanged(const QString& s)
      {
      setFinalPage(QFileInfo(s).completeBaseName() != "00-Blank");
      path = s;
      emit completeChanged();
      }

//---------------------------------------------------------
//   templatePath
//---------------------------------------------------------

QString NewWizardTemplatePage::templatePath() const
      {
      return path;
      }

//---------------------------------------------------------
//   NewWizardKeysigPage
//---------------------------------------------------------

NewWizardKeysigPage::NewWizardKeysigPage(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose key signature and tempo:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      QGroupBox* b1 = new QGroupBox;
      b1->setTitle(tr("Key Signature"));
      b1->setAccessibleName(b1->title());
      b1->setAccessibleDescription(tr("Choose a key signature"));
      sp = MuseScore::newKeySigPalette();
      sp->setMoreElements(false);
      sp->setShowContextMenu(false);
      sp->setSelectable(true);
      sp->setDisableDoubleClick(true);
      int keysigCMajorIdx = 14;
      sp->setSelected(keysigCMajorIdx);
      PaletteScrollArea* sa = new PaletteScrollArea(sp);
      // set widget name to include name of selected key signature
      // we could set the description, but some screen readers ignore it
      sa->setAccessibleName(tr("Key Signature: %1").arg(qApp->translate("Palette", sp->cellAt(keysigCMajorIdx)->name.toUtf8())));
      QAccessibleEvent event(sa, QAccessible::NameChanged);
      QAccessible::updateAccessibility(&event);
      QVBoxLayout* l1 = new QVBoxLayout;
      l1->addWidget(sa);
      b1->setLayout(l1);

      tempoGroup = new QGroupBox;
      tempoGroup->setCheckable(true);
      tempoGroup->setChecked(false);
      tempoGroup->setTitle(tr("Tempo"));
      tempoGroup->setAccessibleName(tempoGroup->title());
      tempoGroup->setAccessibleDescription(tr("Add tempo marking to score"));
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

KeySigEvent NewWizardKeysigPage::keysig() const
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
      auto wizardStyleValue = QWizard::ModernStyle; //Modern Winsows look
#ifdef Q_OS_MAC
      wizardStyleValue = QWizard::MacStyle;
      setOption(QWizard::CancelButtonOnLeft, true);
#endif
      setWizardStyle(wizardStyleValue);

      QPixmap pm = SvgRenderer(":/data/mscore.svg").hdpiPixmap(QSize(64,64));
      setPixmap(QWizard::LogoPixmap, pm);
      setPixmap(QWizard::WatermarkPixmap, QPixmap());
      setWindowTitle(tr("New Score Wizard"));

      setOption(QWizard::NoCancelButton, false);
      setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
      setOption(QWizard::HaveNextButtonOnLastPage, true);

      infoPage = new NewWizardInfoPage;
      instrumentsPage = new NewWizardInstrumentsPage;
      timesigPage = new NewWizardTimesigPage;
      templatePage = new NewWizardTemplatePage;
      keysigPage = new NewWizardKeysigPage;

//  enum Page { Invalid = -1, Type, Instruments, Template, Keysig, Timesig};

      setPage(Page::Type,        infoPage);
      setPage(Page::Instruments, instrumentsPage);
      setPage(Page::Template,    templatePage);
      setPage(Page::Keysig,      keysigPage);
      setPage(Page::Timesig,     timesigPage);

      resize(QSize(840, 560)); //ensure default size if no geometry in settings
      MuseScore::restoreGeometry(this);
      connect(this, SIGNAL(currentIdChanged(int)), SLOT(idChanged(int)));
      }

//---------------------------------------------------------
//   idChanged
//---------------------------------------------------------

void NewWizard::idChanged(int /*id*/)
      {
//printf("\n===\nWizard: id changed %d\n", id);
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
      QString p = templatePage->templatePath();
      QFileInfo fi(p);
      bool val = fi.completeBaseName() == "00-Blank";
      return val;
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void NewWizard::updateValues() const
      {
      //p2->buildInstrumentsList();
      templatePage->buildTemplatesList();
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

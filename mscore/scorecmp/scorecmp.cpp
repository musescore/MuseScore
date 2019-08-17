//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scorecmp.h"

#include "ui_scorecmp_tool.h"
#include "scorediffmodel.h"
#include "scorelistmodel.h"

#include "musescore.h"
#include "scoretab.h"
#include "scoreview.h"

#include "libmscore/scorediff.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreComparisonTool
//---------------------------------------------------------

ScoreComparisonTool::ScoreComparisonTool(QWidget* parent)
   : QDockWidget(parent), _ui(new Ui::ScoreComparisonTool)
      {
      _ui->setupUi(this);
      _mode = Mode::INTELLIGENT;
      _ui->intelligentModeRadioButton->setChecked(true);
      _ui->diffWidget->setCurrentWidget(_ui->pageIntelligentDiff);
      connect(
         _ui->score1ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
         this, &ScoreComparisonTool::selectedVersionsChanged);
      connect(
         _ui->score2ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
         this, &ScoreComparisonTool::selectedVersionsChanged);
      }

//---------------------------------------------------------
//   ~ScoreComparisonTool
//---------------------------------------------------------

ScoreComparisonTool::~ScoreComparisonTool()
      {
      delete _ui;
      delete _diff;
      }

//---------------------------------------------------------
//   ScoreComparisonTool::changeEvent
//---------------------------------------------------------

void ScoreComparisonTool::changeEvent(QEvent* e)
      {
      QDockWidget::changeEvent(e);
      switch(e->type()) {
            case QEvent::LanguageChange:
                  _ui->retranslateUi(this);
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::showEvent
//---------------------------------------------------------

void ScoreComparisonTool::showEvent(QShowEvent* e)
      {
      QDockWidget::showEvent(e);
      if (e->spontaneous())
            return;
      e->accept();

      if (!_scoreListModel) {
            ScoreTab* tab = mscore->getTab1();
            _scoreListModel = new ScoreListModel(&mscore->scores(), tab, this);
            _ui->score1ComboBox->setModel(_scoreListModel);
            _ui->score2ComboBox->setModel(_scoreListModel);

            MasterScore* s = nullptr;
            if (ScoreView* v = tab->view())
                  s = v->score()->masterScore();
            _scoreVersionsModel1 = new ScoreVersionListModel(s, this);
            _scoreVersionsModel2 = new ScoreVersionListModel(s, this);
            _ui->version1ComboBox->setModel(_scoreVersionsModel1);
            _ui->version2ComboBox->setModel(_scoreVersionsModel2);
            selectedVersionsChanged();

            _ui->score1ComboBox->setCurrentIndex(tab->currentIndex());
            connect(tab->getTab(), &QTabBar::currentChanged,
               _ui->score1ComboBox, &QComboBox::setCurrentIndex);

            slotWindowSplit(mscore->splitScreen());
            connect(mscore, &MuseScore::windowSplit,
               this, &ScoreComparisonTool::slotWindowSplit);
            }

      updateDiff();
      }

//---------------------------------------------------------
//   ScoreComparisonTool::updateDiffView
//---------------------------------------------------------

void ScoreComparisonTool::updateDiffView(Mode mode)
      {
      switch(mode) {
            case Mode::RAW:
                  if (!_rawModel && _diff) {
                        _rawModel = new RawScoreDiffModel(_diff, true, this);
                        connect(this, &ScoreComparisonTool::diffUpdated,
                           _rawModel, &RawScoreDiffModel::update);
                        _ui->rawDiffView->setModel(_rawModel);
                        }
                  break;
            case Mode::INTELLIGENT:
                  if (!_intelligentModel && _diff) {
                        _intelligentModel = new ScoreDiffModel(_diff, this);
                        connect(this, &ScoreComparisonTool::diffAboutToBeUpdated,
                           _intelligentModel, &ScoreDiffModel::diffAboutToBeUpdated);
                        connect(this, &ScoreComparisonTool::diffUpdated,
                           _intelligentModel, &ScoreDiffModel::diffUpdated);
                        _ui->intelligentDiffView->setModel(_intelligentModel);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::updateDiffTitle
//---------------------------------------------------------

void ScoreComparisonTool::updateDiffTitle()
      {
      if (_diff)
            _ui->diffWidgetBox->setTitle(
               tr("Comparison of \"%1\" and \"%2\"")
                  .arg(_diff->score1()->title())
                  .arg(_diff->score2()->title())
               );
      else
            _ui->diffWidgetBox->setTitle(tr("Comparison"));
      }

//---------------------------------------------------------
//   ScoreComparisonTool::setMode
//---------------------------------------------------------

void ScoreComparisonTool::setMode(Mode mode)
      {
      if (_mode == mode)
            return;
      _mode = mode;
      updateDiffView(mode);
      switch(mode) {
            case Mode::RAW:
                  if (!_ui->rawModeRadioButton->isChecked())
                        _ui->rawModeRadioButton->setChecked(true);
                  _ui->diffWidget->setCurrentWidget(_ui->pageRawDiff);
                  break;
            case Mode::INTELLIGENT:
                  if (!_ui->intelligentModeRadioButton->isChecked())
                        _ui->intelligentModeRadioButton->setChecked(true);
                  _ui->diffWidget->setCurrentWidget(_ui->pageIntelligentDiff);
                  break;
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::compare
//---------------------------------------------------------

void ScoreComparisonTool::compare(Score* s1, Score* s2)
      {
      invalidateDiff();
      if (!s1 || !s2)
            return;
      _diff = new ScoreDiff(s1, s2);
      connect(s1, &QObject::destroyed, this, &ScoreComparisonTool::invalidateDiff);
      connect(s2, &QObject::destroyed, this, &ScoreComparisonTool::invalidateDiff);
      updateDiffView(_mode);
      updateDiffTitle();
      mscore->setCurrentScores(s1, s2);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::invalidateDiff
//---------------------------------------------------------

void ScoreComparisonTool::invalidateDiff()
      {
      if (_rawModel) {
            _rawModel->deleteLater();
            _rawModel = nullptr;
            }
      if (_intelligentModel) {
            _intelligentModel->deleteLater();
            _intelligentModel = nullptr;
            }

      if (_diff) {
            disconnect(_diff->score1(), 0, this, 0);
            disconnect(_diff->score2(), 0, this, 0);
            delete _diff;
            _diff = nullptr;
            }

      updateDiffTitle();
      }

//---------------------------------------------------------
//   ScoreComparisonTool::updateDiff
//---------------------------------------------------------

void ScoreComparisonTool::updateDiff()
      {
      if (isVisible() && _diff && !_diff->updated()) {
            emit diffAboutToBeUpdated();
            _diff->update();
            emit diffUpdated();
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::openScoreVersion
//    Opens the proper score version if it is not opened
//    and returns the score object. Does not switch active
//    tab as the score comparison tool may need some
//    specific tabs configuration.
//---------------------------------------------------------

Score* ScoreComparisonTool::openScoreVersion(const ScoreVersion& ver)
      {
      Score* s = nullptr;
      if (ver.recent) {
            s = ver.score;
            if (!s)
                  s = mscore->openScore(ver.fileInfo.absoluteFilePath(), /* switchTab */ false);
            }
      else {
            QString baseName;
            if (ver.score)
                  baseName = ver.score->masterScore()->fileInfo()->completeBaseName();
            else
                  baseName = ver.fileInfo.completeBaseName();

            QTemporaryFile* tmp = mscore->getTemporaryScoreFileCopy(
               ver.fileInfo,
               QString("[%1] %2.XXXXXX").arg(ver.name).arg(baseName)
               );
            if (tmp) {
                  s = mscore->openScore(tmp->fileName(), /* switchTab */ false);
                  if (s)
                        s->masterScore()->setReadOnly(true);
                  delete tmp;
                  // let the score know about the temporary file deletion
                  s->masterScore()->fileInfo()->refresh();
                  updateScoreVersions(s);
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_compareButton_clicked
//---------------------------------------------------------

void ScoreComparisonTool::on_compareButton_clicked()
      {
      const int idx1 = _ui->version1ComboBox->currentIndex();
      const int idx2 = _ui->version2ComboBox->currentIndex();
      if (idx1 < 0 || idx2 < 0) // no scores to compare
            return;
      const ScoreVersion& ver1 = _scoreVersionsModel1->getScoreVersion(idx1);
      const ScoreVersion& ver2 = _scoreVersionsModel2->getScoreVersion(idx2);
      Score* s1 = openScoreVersion(ver1);
      Score* s2 = openScoreVersion(ver2);
      compare(s1, s2);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::selectedVersionsChanged
//    Disables Compare button if there are no versions
//    selected for comparison.
//---------------------------------------------------------

void ScoreComparisonTool::selectedVersionsChanged()
      {
      const int idx1 = _ui->version1ComboBox->currentIndex();
      const int idx2 = _ui->version2ComboBox->currentIndex();
      _ui->compareButton->setEnabled(idx1 >= 0 && idx2 >= 0);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::updateScoreVersions
//---------------------------------------------------------

void ScoreComparisonTool::updateScoreVersions(const Score* s)
      {
      if (_scoreVersionsModel1 && _scoreVersionsModel1->score() == s) {
            _scoreVersionsModel1->update();
            resetVersion1ComboBox();
            }
      if (_scoreVersionsModel2 && _scoreVersionsModel2->score() == s) {
            _scoreVersionsModel2->update();
            resetVersion2ComboBox();
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_browseFileButton_clicked
//---------------------------------------------------------

void ScoreComparisonTool::on_browseFileButton_clicked()
      {
      Score* prevScore = mscore->currentScore();
      mscore->loadFiles(/* switchTab */ true, /* singleFile */ true);
      Score* loadedScore = mscore->currentScore();
      if (loadedScore == prevScore) // we didn't load anything?
            return;
      // ensure that we display this score in split-screen mode
      mscore->setCurrentScores(loadedScore, loadedScore);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::resetVersion1ComboBox
//---------------------------------------------------------

void ScoreComparisonTool::resetVersion1ComboBox()
      {
      if (_scoreVersionsModel1) {
            _ui->version1ComboBox->setCurrentIndex(
               _scoreVersionsModel1->getPosition(ScoreVersion::INDEX_CURRENT));
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_score1ComboBox_currentIndexChanged
//---------------------------------------------------------

void ScoreComparisonTool::on_score1ComboBox_currentIndexChanged(int idx)
      {
      if (_scoreVersionsModel1) {
            Score* s = _scoreListModel->getScore(idx);
            _scoreVersionsModel1->setScore(s ? s->masterScore() : nullptr);
            resetVersion1ComboBox();
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::resetVersion2ComboBox
//---------------------------------------------------------

void ScoreComparisonTool::resetVersion2ComboBox()
      {
      if (_scoreVersionsModel2) {
            int index = -1;
            if (!mscore->splitScreen())
                  index = _scoreVersionsModel2->getPosition(ScoreVersion::INDEX_SESSION_START);
            if (index < 0)
                  index = _scoreVersionsModel2->getPosition(ScoreVersion::INDEX_CURRENT);
            _ui->version2ComboBox->setCurrentIndex(index);
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_score2ComboBox_currentIndexChanged
//---------------------------------------------------------

void ScoreComparisonTool::on_score2ComboBox_currentIndexChanged(int idx)
      {
      if (_scoreVersionsModel2) {
            Score* s = _scoreListModel->getScore(idx);
            _scoreVersionsModel2->setScore(s ? s->masterScore() : nullptr);
            resetVersion2ComboBox();
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_rawModeRadioButton_toggled
//---------------------------------------------------------

void ScoreComparisonTool::on_rawModeRadioButton_toggled(bool checked)
      {
      if (checked)
            setMode(Mode::RAW);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_intelligentModeRadioButton_toggled
//---------------------------------------------------------

void ScoreComparisonTool::on_intelligentModeRadioButton_toggled(bool checked)
      {
      if (checked)
            setMode(Mode::INTELLIGENT);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::on_intelligentDiffView_activated
//---------------------------------------------------------

void ScoreComparisonTool::on_intelligentDiffView_activated(const QModelIndex& index)
      {
      const BaseDiff* diff = _intelligentModel->diffItem(index);

      const ScoreElement* el[2];
      bool select[2] { true, true };
      switch(diff->itemType()) {
            case ItemType::ELEMENT:
                  {
                  const ElementDiff* elDiff = static_cast<const ElementDiff*>(diff);
                  for (int i = 0; i < 2; ++i) {
                        if (elDiff->el[i])
                              el[i] = elDiff->el[i];
                        else {
                              if (elDiff->ctx[i]->isElement() || !elDiff->before[i])
                                    el[i] = elDiff->ctx[i];
                              else
                                    el[i] = elDiff->before[i];
                              select[i] = false;
                              }
                        }
                  }
                  break;
            default:
                  std::copy(diff->ctx, diff->ctx + 2, el);
                  break;
            }

      showElement(el[0], select[0]);
      showElement(el[1], select[1]);
      }

//---------------------------------------------------------
//   ScoreComparisonTool::showElement
//---------------------------------------------------------

void ScoreComparisonTool::showElement(const ScoreElement* se, bool select)
      {
      if (!se)
            return;

      ScoreView* sv1 = mscore->getTab1()->view();
      Score* score1 = (sv1 && sv1->isVisible()) ? sv1->score() : nullptr;
      ScoreView* sv2 = mscore->getTab2() ? mscore->getTab2()->view() : nullptr;
      Score* score2 = (sv2 && sv2->isVisible()) ? sv2->score() : nullptr;
      const Element* e = se->isElement() ? toElement(se) : nullptr;
      Score* score = se->score();

      score->deselectAll();
      if (select && e && (score1 == score || score2 == score)) {
            Element* el = const_cast<Element*>(e);
            if (el->isChordRest())
                  score->select(el, SelectType::RANGE);
            else
                  score->select(el);
            }

      if (score1 == score) {
            bool move = sv1->moveWhenInactive(true);
            if (e) {
                  sv1->adjustCanvasPosition(e, false);
                  sv1->update();
                  }
            sv1->moveWhenInactive(move);
            }
      if (score2 == score) {
            bool move = sv2->moveWhenInactive(true);
            if (e) {
                  sv2->adjustCanvasPosition(e, false);
                  sv2->update();
                  }
            sv2->moveWhenInactive(move);
            }
      }

//---------------------------------------------------------
//   ScoreComparisonTool::slotWindowSplit
//---------------------------------------------------------

void ScoreComparisonTool::slotWindowSplit(bool split)
      {
      ScoreTab* tab1 = mscore->getTab1();

      if (ScoreTab* tab2 = mscore->getTab2()) {
            if (split) {
                  disconnect(tab1->getTab(), &QTabBar::currentChanged,
                     _ui->score2ComboBox, &QComboBox::setCurrentIndex);
                  _ui->score2ComboBox->setCurrentIndex(tab2->currentIndex());
                  connect(tab2->getTab(), &QTabBar::currentChanged,
                     _ui->score2ComboBox, &QComboBox::setCurrentIndex,
                     Qt::UniqueConnection
                     );
                  }
            else
                  disconnect(tab2->getTab(), &QTabBar::currentChanged,
                     _ui->score2ComboBox, &QComboBox::setCurrentIndex);
            }

      if (!split) {
            _ui->score2ComboBox->setCurrentIndex(tab1->currentIndex());
            connect(tab1->getTab(), &QTabBar::currentChanged,
               _ui->score2ComboBox, &QComboBox::setCurrentIndex,
               Qt::UniqueConnection
               );
            }
      }
}

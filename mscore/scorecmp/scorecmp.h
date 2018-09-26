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

#ifndef __SCORECMP_H__
#define __SCORECMP_H__

namespace Ui {
      class ScoreComparisonTool;
      }

namespace Ms {

class Score;
class ScoreElement;
class ScoreDiff;
class RawScoreDiffModel;
class ScoreDiffModel;
class ScoreListModel;
struct ScoreVersion;
class ScoreVersionListModel;

//---------------------------------------------------------
//   ScoreComparisonTool
//---------------------------------------------------------

class ScoreComparisonTool : public QDockWidget {
      Q_OBJECT

   public:
      enum class Mode { RAW, INTELLIGENT };

   private:
      Ui::ScoreComparisonTool* _ui;
      Mode _mode;

      ScoreListModel* _scoreListModel = nullptr;
      ScoreVersionListModel* _scoreVersionsModel1 = nullptr;
      ScoreVersionListModel* _scoreVersionsModel2 = nullptr;

      ScoreDiff* _diff = nullptr;
      RawScoreDiffModel* _rawModel = nullptr;
      ScoreDiffModel* _intelligentModel = nullptr;

      void updateDiffView(Mode mode);
      void updateDiffTitle();
      void resetVersion1ComboBox();
      void resetVersion2ComboBox();
      Score* openScoreVersion(const ScoreVersion& ver);
      void showElement(const ScoreElement* se, bool select);

   private slots:
      void on_compareButton_clicked();
      void on_browseFileButton_clicked();
      void on_score1ComboBox_currentIndexChanged(int);
      void on_score2ComboBox_currentIndexChanged(int);
      void on_rawModeRadioButton_toggled(bool);
      void on_intelligentModeRadioButton_toggled(bool);
      void on_intelligentDiffView_activated(const QModelIndex&);
      void selectedVersionsChanged();

   protected:
      void changeEvent(QEvent* e) override;
      void showEvent(QShowEvent* e) override;

   signals:
       void diffAboutToBeUpdated();
       void diffUpdated();

   public:
      explicit ScoreComparisonTool(QWidget* parent = nullptr);
      ~ScoreComparisonTool();

      void setMode(Mode mode);
      Mode mode() const { return _mode; }

      void compare(Score* s1, Score* s2);

   public slots:
      void slotWindowSplit(bool);
      void invalidateDiff();
      void updateDiff();
      void updateScoreVersions(const Score*);
      };

}     // namespace Ms
#endif

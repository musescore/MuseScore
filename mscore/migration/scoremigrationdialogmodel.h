#ifndef SCOREMIGRATIONDIALOGMODEL_H
#define SCOREMIGRATIONDIALOGMODEL_H

#include <QObject>

#include "scoremigrator_3_6.h"

class ScoreMigrationDialogModel : public QObject
      {
      Q_OBJECT

      Q_PROPERTY(bool areStylingImprovementsAllowed READ areStylingImprovementsAllowed WRITE setAreStylingImprovementsAllowed NOTIFY areStylingImprovementsAllowedChanged)
      Q_PROPERTY(bool areInstrumentsImprovementsAllowed READ areInstrumentsImprovementsAllowed WRITE setAreInstrumentsImprovementsAllowed NOTIFY areInstrumentsImprovementsAllowedChanged)
      Q_PROPERTY(bool shouldNeverAskForThisScoreAgain READ shouldNeverAskForThisScoreAgain WRITE setShouldNeverAskForThisScoreAgain NOTIFY shouldNeverAskForThisScoreAgainChanged)
      Q_PROPERTY(bool shouldNeverAskAgain READ shouldNeverAskAgain WRITE setShouldNeverAskAgain NOTIFY shouldNeverAskAgainChanged)

      Q_PROPERTY(bool isApplyingAvailable READ isApplyingAvailable NOTIFY isApplyingAvailableChanged)

   public:
      explicit ScoreMigrationDialogModel(Ms::Score* score, QObject* parent = nullptr);
      ~ScoreMigrationDialogModel();

      Q_INVOKABLE void apply();
      Q_INVOKABLE void ignore();

      Q_INVOKABLE void showMoreDetails();

      bool areStylingImprovementsAllowed() const;
      bool areInstrumentsImprovementsAllowed() const;

      bool isApplyingAvailable() const;

      bool shouldNeverAskForThisScoreAgain() const;
      bool shouldNeverAskAgain() const;

   public slots:

      void setAreStylingImprovementsAllowed(bool areStylingImprovementsAllowed);
      void setAreInstrumentsImprovementsAllowed(bool areInstrumentsImprovementsAllowed);

      void setShouldNeverAskForThisScoreAgain(bool shouldNeverAskForThisScoreAgain);
      void setShouldNeverAskAgain(bool shouldNeverAskAgain);

   signals:
      void areStylingImprovementsAllowedChanged(bool areStylingImprovementsAllowed);
      void areInstrumentsImprovementsAllowedChanged(bool areInstrumentsImprovementsAllowed);

      void isApplyingAvailableChanged(bool isApplyingAvailable);

      void shouldNeverAskForThisScoreAgainChanged(bool shouldNeverAskForThisScoreAgain);
      void shouldNeverAskAgainChanged(bool shouldNeverAskAgain);

      void closeRequested();

   private:

      void setUpMigrationPolicy();

      bool m_areStylingImprovementsAllowed = false;
      bool m_areInstrumentsImprovementsAllowed = false;
      bool m_isApplyingAvailable = false;
      bool m_shouldNeverAskForThisScoreAgain = false;
      bool m_shouldNeverAskAgain = false;

      Ms::Score* m_score = nullptr;
      ScoreMigrator_3_6* m_migrator = nullptr;
      };

#endif // SCOREMIGRATIONDIALOGMODEL_H

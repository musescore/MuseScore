#include "scoremigrationdialogmodel.h"

#include <QDesktopServices>

#include "handlers/instrumentorderinghandler.h"
#include "handlers/staffverticaljustificationhandler.h"
#include "handlers/resetallelementspositionshandler.h"
#include "handlers/firstsystemindentationhandler.h"
#include "libmscore/cmd.cpp"
#include "mscore/preferences.h"

ScoreMigrationDialogModel::ScoreMigrationDialogModel(Ms::Score* score, QObject* parent)
    : QObject(parent), m_score(score), m_migrator(new ScoreMigrator_3_6())
      {
      }

ScoreMigrationDialogModel::~ScoreMigrationDialogModel()
      {
      delete m_migrator;
      }

void ScoreMigrationDialogModel::apply()
      {
      setUpMigrationPolicy();

      m_score->setStyleValue(Ms::Sid::qualityUpgradeAllowed, !m_shouldNeverAskForThisScoreAgain);
      Ms::preferences.setPreference(PREF_IMPORT_SCORE_MIGRATION_ENABLED, !m_shouldNeverAskAgain);

      if (!m_shouldNeverAskAgain && !m_shouldNeverAskForThisScoreAgain)
            m_migrator->migrateScore(m_score);

      emit closeRequested();
      }

void ScoreMigrationDialogModel::ignore()
      {
      emit closeRequested();
      }

void ScoreMigrationDialogModel::showMoreDetails()
      {
      QDesktopServices::openUrl(QUrl("https://musescore.com")); // Here will be the link for announcement
      }

bool ScoreMigrationDialogModel::areStylingImprovementsAllowed() const
      {
      return m_areStylingImprovementsAllowed;
      }

bool ScoreMigrationDialogModel::areInstrumentsImprovementsAllowed() const
      {
      return m_areInstrumentsImprovementsAllowed;
      }

bool ScoreMigrationDialogModel::isApplyingAvailable() const
      {
      return m_areInstrumentsImprovementsAllowed
             || m_areStylingImprovementsAllowed
             || m_shouldNeverAskForThisScoreAgain
             || m_shouldNeverAskAgain;
      }

bool ScoreMigrationDialogModel::shouldNeverAskForThisScoreAgain() const
      {
      return m_shouldNeverAskForThisScoreAgain;
      }

bool ScoreMigrationDialogModel::shouldNeverAskAgain() const
      {
      return m_shouldNeverAskAgain;
      }

void ScoreMigrationDialogModel::setAreStylingImprovementsAllowed(bool areStylingImprovementsAllowed)
      {
      if (m_areStylingImprovementsAllowed == areStylingImprovementsAllowed)
            return;

      m_areStylingImprovementsAllowed = areStylingImprovementsAllowed;
      emit areStylingImprovementsAllowedChanged(m_areStylingImprovementsAllowed);
      emit isApplyingAvailableChanged(isApplyingAvailable());
      }

void ScoreMigrationDialogModel::setAreInstrumentsImprovementsAllowed(bool areInstrumentsImprovementsAllowed)
      {
      if (m_areInstrumentsImprovementsAllowed == areInstrumentsImprovementsAllowed)
            return;

      m_areInstrumentsImprovementsAllowed = areInstrumentsImprovementsAllowed;
      emit areInstrumentsImprovementsAllowedChanged(m_areInstrumentsImprovementsAllowed);
      emit isApplyingAvailableChanged(isApplyingAvailable());
      }

void ScoreMigrationDialogModel::setShouldNeverAskForThisScoreAgain(bool shouldNeverAskForThisScoreAgain)
      {
      if (m_shouldNeverAskForThisScoreAgain == shouldNeverAskForThisScoreAgain)
            return;

      m_shouldNeverAskForThisScoreAgain = shouldNeverAskForThisScoreAgain;
      emit shouldNeverAskForThisScoreAgainChanged(shouldNeverAskForThisScoreAgain);
      emit isApplyingAvailableChanged(isApplyingAvailable());
      }

void ScoreMigrationDialogModel::setShouldNeverAskAgain(bool shouldNeverAskAgain)
      {
      if (m_shouldNeverAskAgain == shouldNeverAskAgain)
            return;

      m_shouldNeverAskAgain = shouldNeverAskAgain;
      emit shouldNeverAskAgainChanged(m_shouldNeverAskAgain);
      emit isApplyingAvailableChanged(isApplyingAvailable());
      }

void ScoreMigrationDialogModel::setUpMigrationPolicy()
      {
      if (m_areStylingImprovementsAllowed) {
            m_migrator->registerHandler(new ResetAllElementsPositionsHandler());
            m_migrator->registerHandler(new StaffVerticalJustificationHandler());
            m_migrator->registerHandler(new FirstSystemIndentationHandler());
            }

      if (m_areInstrumentsImprovementsAllowed) {
            m_migrator->registerHandler(new InstrumentOrderingHandler());
            }
      }

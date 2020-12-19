#include "scoremigrationdialogmodel.h"

#include <QDesktopServices>

#include "handlers/resetallelementspositionshandler.h"
#include "handlers/lelandstylehandler.h"
#include "handlers/edwinstylehandler.h"
#include "libmscore/cmd.cpp"
#include "mscore/preferences.h"

static const int MSC_V3 = 300;

ScoreMigrationDialogModel::ScoreMigrationDialogModel(Ms::Score* score, QObject* parent)
    : QObject(parent), m_score(score), m_migrator(new ScoreMigrator_3_6())
      {
      setIsAutomaticPlacementAvailable(score->mscVersion() < MSC_V3);
      setCreationAppVersion(score->mscoreVersion());
      }

ScoreMigrationDialogModel::~ScoreMigrationDialogModel()
      {
      delete m_migrator;
      }

void ScoreMigrationDialogModel::apply()
      {
      setUpMigrationPolicy();

      m_migrator->migrateScore(m_score);

      emit closeRequested();
      }

void ScoreMigrationDialogModel::ignore()
      {
      Ms::preferences.setPreference(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN, m_shouldNeverAskAgain);

      if (m_shouldNeverAskAgain) {
            Ms::preferences.setPreference(PREF_MIGRATION_APPLY_LELAND_STYLE, false);
            Ms::preferences.setPreference(PREF_MIGRATION_APPLY_EDWIN_STYLE, false);
            Ms::preferences.setPreference(PREF_MIGRATION_RESET_ELEMENT_POSITIONS, false);
            }

      emit closeRequested();
      }

void ScoreMigrationDialogModel::showMoreDetails()
      {
      QDesktopServices::openUrl(QUrl("https://musescore.com")); // Here will be the link for announcement
      }

bool ScoreMigrationDialogModel::isLelandAllowed() const
      {
      return m_isLelandAllowed;
      }

bool ScoreMigrationDialogModel::isEdwinAllowed() const
      {
      return m_isEdwinAllowed;
      }

bool ScoreMigrationDialogModel::isApplyingAvailable() const
      {
      return m_isEdwinAllowed
             || m_isLelandAllowed
             || m_isAutomaticPlacementAllowed;
      }

bool ScoreMigrationDialogModel::shouldNeverAskAgain() const
      {
      return m_shouldNeverAskAgain;
      }

bool ScoreMigrationDialogModel::isAutomaticPlacementAllowed() const
      {
      return m_isAutomaticPlacementAllowed;
      }

bool ScoreMigrationDialogModel::isAutomaticPlacementAvailable() const
      {
      return m_isAutomaticPlacementAvailable;
      }

QString ScoreMigrationDialogModel::creationAppVersion() const
      {
      return m_creationAppVersion;
      }

void ScoreMigrationDialogModel::setIsLelandAllowed(bool areStylingImprovementsAllowed)
      {
      if (m_isLelandAllowed == areStylingImprovementsAllowed)
            return;

      m_isLelandAllowed = areStylingImprovementsAllowed;
      emit isLelandAllowedChanged(m_isLelandAllowed);
      emit isApplyingAvailableChanged(isApplyingAvailable());
      }

void ScoreMigrationDialogModel::setIsEdwinAllowed(bool areInstrumentsImprovementsAllowed)
      {
      if (m_isEdwinAllowed == areInstrumentsImprovementsAllowed)
            return;

      m_isEdwinAllowed = areInstrumentsImprovementsAllowed;
      emit isEdwinAllowedChanged(m_isEdwinAllowed);
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

void ScoreMigrationDialogModel::setIsAutomaticPlacementAllowed(bool isAutomaticPlacementAllowed)
      {
      if (m_isAutomaticPlacementAllowed == isAutomaticPlacementAllowed)
            return;

      m_isAutomaticPlacementAllowed = isAutomaticPlacementAllowed;
      emit isAutomaticPlacementAllowedChanged(m_isAutomaticPlacementAllowed);
      emit isApplyingAvailableChanged(isApplyingAvailable());
      }

void ScoreMigrationDialogModel::setIsAutomaticPlacementAvailable(bool isAutomaticPlacementAvailable)
      {
      if (m_isAutomaticPlacementAvailable == isAutomaticPlacementAvailable)
            return;

      m_isAutomaticPlacementAvailable = isAutomaticPlacementAvailable;
      emit isAutomaticPlacementAvailableChanged(m_isAutomaticPlacementAvailable);
      }

void ScoreMigrationDialogModel::setCreationAppVersion(QString creationAppVersion)
      {
      if (m_creationAppVersion == creationAppVersion)
            return;

      m_creationAppVersion = creationAppVersion;
      emit creationAppVersionChanged(m_creationAppVersion);
      }

void ScoreMigrationDialogModel::setUpMigrationPolicy()
      {
      Ms::preferences.setPreference(PREF_MIGRATION_DO_NOT_ASK_ME_AGAIN, m_shouldNeverAskAgain);

      if (m_isLelandAllowed)
            m_migrator->registerHandler(new LelandStyleHandler());
      if (m_isEdwinAllowed)
            m_migrator->registerHandler(new EdwinStyleHandler());
      if (m_isAutomaticPlacementAllowed)
            m_migrator->registerHandler(new ResetAllElementsPositionsHandler());

      if (m_shouldNeverAskAgain) {
            Ms::preferences.setPreference(PREF_MIGRATION_APPLY_LELAND_STYLE, m_isLelandAllowed);
            Ms::preferences.setPreference(PREF_MIGRATION_APPLY_EDWIN_STYLE, m_isEdwinAllowed);
            Ms::preferences.setPreference(PREF_MIGRATION_RESET_ELEMENT_POSITIONS, m_isAutomaticPlacementAllowed);
            }
      }

#ifndef SCOREMIGRATIONDIALOGMODEL_H
#define SCOREMIGRATIONDIALOGMODEL_H

#include <QObject>

#include "scoremigrator_3_6.h"

class ScoreMigrationDialogModel : public QObject
      {
      Q_OBJECT

      Q_PROPERTY(bool isLelandAllowed READ isLelandAllowed WRITE setIsLelandAllowed NOTIFY isLelandAllowedChanged)
      Q_PROPERTY(bool isEdwinAllowed READ isEdwinAllowed WRITE setIsEdwinAllowed NOTIFY isEdwinAllowedChanged)
      Q_PROPERTY(bool isAutomaticPlacementAllowed READ isAutomaticPlacementAllowed WRITE setIsAutomaticPlacementAllowed NOTIFY isAutomaticPlacementAllowedChanged)
      Q_PROPERTY(bool isAutomaticPlacementAvailable READ isAutomaticPlacementAvailable WRITE setIsAutomaticPlacementAvailable NOTIFY isAutomaticPlacementAvailableChanged)

      Q_PROPERTY(bool shouldNeverAskAgain READ shouldNeverAskAgain WRITE setShouldNeverAskAgain NOTIFY shouldNeverAskAgainChanged)
      Q_PROPERTY(bool isApplyingAvailable READ isApplyingAvailable NOTIFY isApplyingAvailableChanged)
      Q_PROPERTY(QString creationAppVersion READ creationAppVersion NOTIFY creationAppVersionChanged)

   public:
      explicit ScoreMigrationDialogModel(Ms::Score* score, QObject* parent = nullptr);
      ~ScoreMigrationDialogModel();

      Q_INVOKABLE void apply();
      Q_INVOKABLE void ignore();

      Q_INVOKABLE void showMoreDetails();

      bool isLelandAllowed() const;
      bool isEdwinAllowed() const;

      bool isApplyingAvailable() const;
      bool shouldNeverAskAgain() const;

      bool isAutomaticPlacementAllowed() const;
      bool isAutomaticPlacementAvailable() const;

      QString creationAppVersion() const;

public slots:

      void setIsLelandAllowed(bool isLelandAllowed);
      void setIsEdwinAllowed(bool isEdwinAllowed);

      void setShouldNeverAskAgain(bool shouldNeverAskAgain);

      void setIsAutomaticPlacementAllowed(bool isAutomaticPlacementAllowed);
      void setIsAutomaticPlacementAvailable(bool isAutomaticPlacementAvailable);

      void setCreationAppVersion(QString creationAppVersion);

signals:
      void isLelandAllowedChanged(bool isLelandAllowed);
      void isEdwinAllowedChanged(bool isEdwinAllowed);

      void isApplyingAvailableChanged(bool isApplyingAvailable);
      void shouldNeverAskAgainChanged(bool shouldNeverAskAgain);

      void closeRequested();

      void isAutomaticPlacementAllowedChanged(bool isAutomaticPlacementAllowed);
      void isAutomaticPlacementAvailableChanged(bool isAutomaticPlacementAvailable);

      void creationAppVersionChanged(QString creationAppVersion);

private:

      void setUpMigrationPolicy();

      bool m_isLelandAllowed = false;
      bool m_isEdwinAllowed = false;
      bool m_isAutomaticPlacementAllowed = false;
      bool m_isAutomaticPlacementAvailable = false;
      bool m_isApplyingAvailable = false;
      bool m_shouldNeverAskAgain = false;

      Ms::Score* m_score = nullptr;
      ScoreMigrator_3_6* m_migrator = nullptr;
      QString m_creationAppVersion;
};

#endif // SCOREMIGRATIONDIALOGMODEL_H

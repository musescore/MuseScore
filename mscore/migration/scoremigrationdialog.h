#ifndef SCOREMIGRATIONDIALOG_H
#define SCOREMIGRATIONDIALOG_H

#include "scoremigrationdialogmodel.h"

class ScoreMigrationDialog : public QQuickWidget
      {
      Q_OBJECT

   public:
      explicit ScoreMigrationDialog(QQmlEngine* engine, Ms::Score *score);

   private:
      void showEvent(QShowEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;

      ScoreMigrationDialogModel* m_dialogModel = nullptr;
      };

#endif // SCOREMIGRATIONDIALOG_H

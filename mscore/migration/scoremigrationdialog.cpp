#include "scoremigrationdialog.h"

ScoreMigrationDialog::ScoreMigrationDialog(QQmlEngine* engine, Ms::Score* score)
      : QQuickWidget(engine, nullptr), m_dialogModel(new ScoreMigrationDialogModel(score, this))
      {
      setMinimumWidth(600);
      setMinimumHeight(m_dialogModel->isAutomaticPlacementAvailable() ? 570 : 548);

      setWindowFlags(Qt::Dialog);
      setWindowModality(Qt::ApplicationModal);

      setWindowTitle(score->title());
      setSource(QUrl(QStringLiteral("qrc:/qml/migration/ScoreMigrationDialog.qml")));

      setResizeMode(SizeRootObjectToView);

      connect(m_dialogModel, &ScoreMigrationDialogModel::closeRequested, this, &QQuickWidget::close);

      if (rootObject())
            rootObject()->setProperty("model", QVariant::fromValue(m_dialogModel));
      }

void ScoreMigrationDialog::focusInEvent(QFocusEvent* event)
      {
      QQuickWidget::focusInEvent(event);
      rootObject()->forceActiveFocus();
      }

void ScoreMigrationDialog::showEvent(QShowEvent* event)
      {
      QQuickWidget::showEvent(event);
      setFocus();
      rootObject()->forceActiveFocus();
      }

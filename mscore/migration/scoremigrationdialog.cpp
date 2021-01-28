#include "scoremigrationdialog.h"

ScoreMigrationDialog::ScoreMigrationDialog(QQmlEngine* engine, Ms::Score* score)
      : QQuickView(engine, nullptr), m_dialogModel(new ScoreMigrationDialogModel(score, this))
      {
      setMinimumWidth(600);
      setMinimumHeight(m_dialogModel->isAutomaticPlacementAvailable() ? 570 : 548);

      setFlags(Qt::Dialog);

      setTitle(score->title());
      setSource(QUrl(QStringLiteral("qrc:/qml/migration/ScoreMigrationDialog.qml")));

      setModality(Qt::ApplicationModal);
      setResizeMode(SizeRootObjectToView);

      connect(m_dialogModel, &ScoreMigrationDialogModel::closeRequested, this, &QQuickView::close);

      if (rootObject())
            rootObject()->setProperty("model", QVariant::fromValue(m_dialogModel));
      }

void ScoreMigrationDialog::focusInEvent(QFocusEvent* event)
      {
      QQuickView::focusInEvent(event);
      rootObject()->forceActiveFocus();
      }

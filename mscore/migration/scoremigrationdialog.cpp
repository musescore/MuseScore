#include "scoremigrationdialog.h"

#include <QQuickItem>

ScoreMigrationDialog::ScoreMigrationDialog(QQmlEngine* engine, Ms::Score* score)
      : QQuickView(engine, nullptr), m_dialogModel(new ScoreMigrationDialogModel(score, this))
      {
      setMinimumWidth(600);
      setMinimumHeight(600);

      setFlags(Qt::Dialog);

      setSource(QUrl(QStringLiteral("qrc:/qml/migration/ScoreMigrationDialog.qml")));

      setModality(Qt::ApplicationModal);
      setResizeMode(QQuickView::SizeRootObjectToView);

      connect(m_dialogModel, &ScoreMigrationDialogModel::closeRequested, this, &QQuickView::close);

      if (rootObject())
            rootObject()->setProperty("model", QVariant::fromValue(m_dialogModel));
      }

void ScoreMigrationDialog::focusInEvent(QFocusEvent* event)
      {
      QQuickView::focusInEvent(event);
      if (rootObject())
            rootObject()->forceActiveFocus();
      }

#include "inspectorformwidget.h"

InspectorFormWidget::InspectorFormWidget(QQmlEngine* qmlEngine, QWidget* parent) : QQuickWidget(qmlEngine, parent)
{
    setMinimumWidth(320);
    setMinimumHeight(parent->height());

    if (parent) {
        m_inspector = qobject_cast<Ms::Inspector*>(parent);
    }

    QUrl url = QUrl(QStringLiteral("qrc:/view/qml/InspectorForm.qml"));

    setSource(url);

    setResizeMode(QQuickWidget::SizeRootObjectToView);

    m_inspectorListModel = new InspectorListModel(this);
    connect(m_inspectorListModel, &InspectorListModel::elementsModified, this, &InspectorFormWidget::layoutUpdateRequested);

    setFocusPolicy(Qt::StrongFocus);

    if (rootObject()) {
        rootObject()->setProperty("inspectorListModel", QVariant::fromValue(m_inspectorListModel));
        rootObject()->setWidth(minimumWidth());
        rootObject()->setHeight(minimumHeight());
    }
}

void InspectorFormWidget::onSelectionChanged(const QList<Ms::Element*>& elementList)
{
    m_inspectorListModel->setElementList(elementList);
}

void InspectorFormWidget::focusInEvent(QFocusEvent *event)
{
    QQuickWidget::focusInEvent(event);

    if (rootObject()) {
        rootObject()->setFocus(true);
    }
}

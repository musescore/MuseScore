#ifndef INSPECTORELEMENT_H
#define INSPECTORELEMENT_H

#include "inspectordockwidget.h"
#include <QQuickWidget>

class InspectorDockWidget;
class InspectorListModel;

class InspectorFormWidget : public QQuickWidget
{
    Q_OBJECT

public:
    explicit InspectorFormWidget(QQmlEngine* qmlEngine, QWidget* parent = nullptr);

public slots:
    void onSelectionChanged(const QList<Ms::Element*>& elementList);

signals:
    void layoutUpdateRequested();

private:
    InspectorDockWidget* m_inspector = nullptr;

    InspectorListModel* m_inspectorListModel = nullptr;

protected:
    void focusInEvent(QFocusEvent* event) override;
};

#endif // INSPECTORELEMENT_H

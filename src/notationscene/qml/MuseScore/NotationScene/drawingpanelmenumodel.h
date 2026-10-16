// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "actions/actionable.h"
#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

namespace mu::notation {
class DrawingPanelMenuModel : public muse::uicomponents::AbstractMenuModel, public muse::actions::Actionable
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool keepVisible READ keepVisible NOTIFY keepVisibleChanged)

public:
    explicit DrawingPanelMenuModel(QObject* parent = nullptr);
    Q_INVOKABLE void load() override;
    bool keepVisible() const;

signals:
    void keepVisibleChanged();

private:
    bool m_loaded = false;
};
}

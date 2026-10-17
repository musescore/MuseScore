// SPDX-License-Identifier: GPL-3.0-only
#include "drawingpanelmenumodel.h"

#include "settings.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::uicomponents;

static const Settings::Key KEEP_VISIBLE("notation", "ui/drawing/keepPanelVisible");
static const actions::ActionCode TOGGLE_KEEP_VISIBLE("drawing-toggle-keep-panel-visible");

DrawingPanelMenuModel::DrawingPanelMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
    settings()->setDefaultValue(KEEP_VISIBLE, Val(false));
}

bool DrawingPanelMenuModel::keepVisible() const
{
    return settings()->value(KEEP_VISIBLE).toBool();
}

void DrawingPanelMenuModel::load()
{
    if (m_loaded) {
        return;
    }
    m_loaded = true;
    auto item = new MenuItem(this);
    item->setId(QString::fromStdString(TOGGLE_KEEP_VISIBLE));
    UiAction action;
    action.code = TOGGLE_KEEP_VISIBLE;
    action.title = TranslatableString("drawing-tools", "Keep visible");
    action.checkable = Checkable::Yes;
    item->setAction(action);
    UiActionState state;
    state.enabled = true;
    state.checked = keepVisible();
    item->setState(state);
    setItems({ item });
    settings()->valueChanged(KEEP_VISIBLE).onReceive(this, [this, item](const Val&) {
        auto state = item->state();
        state.checked = keepVisible();
        item->setState(state);
        emit itemChanged(item);
        emit keepVisibleChanged();
    });
    dispatcher()->reg(this, TOGGLE_KEEP_VISIBLE, [this]() {
        settings()->setSharedValue(KEEP_VISIBLE, Val(!keepVisible()));
    });
}

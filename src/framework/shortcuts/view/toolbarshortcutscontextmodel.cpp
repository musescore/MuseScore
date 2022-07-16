#include <QJsonDocument>
#include "toolbarshortcutscontextmodel.h"
#include "log.h"

using namespace mu::shortcuts;

ToolbarShortcutsContextModel::ToolbarShortcutsContextModel(QObject* parent)
    : QObject{parent}
{
}

void ToolbarShortcutsContextModel::addShortcut(QString action)
{
    LOGD() << "Adding shortcut in toolbarshortcutsmodel: " << action;

    QString uri = QString("musescore://shortcuts/editshortcut?sync=true&actionCode=%1")
                  .arg(action);

    interactive()->open(uri.toStdString());
}

void ToolbarShortcutsContextModel::removeShortcut(QString action)
{
    LOGD() << "Removing in toolbarshortcutsmodel: " << action;
    shortcutsRegister()->removeShortcutForAction(action);
}

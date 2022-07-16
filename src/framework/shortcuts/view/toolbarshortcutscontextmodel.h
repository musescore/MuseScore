#ifndef TOOLBARSHORTCUTSCONTEXTMODEL_H
#define TOOLBARSHORTCUTSCONTEXTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "ishortcutsregister.h"
#include "iinteractive.h"
#include "ui/iuiactionsregister.h"

namespace mu::shortcuts {
class ToolbarShortcutsContextModel : public QObject
{
    Q_OBJECT

    INJECT(shortcuts, IShortcutsRegister, shortcutsRegister)
    INJECT(shortcuts, framework::IInteractive, interactive)
    INJECT(shortcuts, ui::IUiActionsRegister, uiactionsRegister)

public:
    explicit ToolbarShortcutsContextModel(QObject* parent = nullptr);

    Q_INVOKABLE void addShortcut(QString action);
    Q_INVOKABLE void removeShortcut(QString action);

//signals:
};
}

#endif // TOOLBARSHORTCUTSCONTEXTMODEL_H

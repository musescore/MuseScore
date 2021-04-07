//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_NOTEINPUTBARMODEL_H
#define MU_NOTATION_NOTEINPUTBARMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "ui/iuiactionsregister.h"
#include "actions/iactionsdispatcher.h"
#include "playback/iplaybackcontroller.h"
#include "workspace/iworkspacemanager.h"
#include "shortcuts/ishortcutsregister.h"
#include "ui/view/abstractmenumodel.h"

namespace mu::notation {
class NoteInputBarModel : public QAbstractListModel, public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, workspace::IWorkspaceManager, workspaceManager)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit NoteInputBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& action, int actionIndex = -1);

    Q_INVOKABLE QVariantMap get(int index);

signals:
    void countChanged(int count);

private:
    enum Roles {
        CodeRole = Qt::UserRole + 1,
        IconRole,
        SectionRole,
        CheckedRole,
        HintRole,
        SubitemsRole,
        ShowSubitemsByPressAndHoldRole,
        OrderRole
    };

    void onActionsStateChanges(const actions::ActionCodeList& codes) override;

    INotationPtr notation() const;

    void onNotationChanged();

    void toggleNoteInput();
    void toggleNoteInputMethod(const actions::ActionCode& actionCode);

    void updateState();
    void updateNoteInputState();
    void updateNoteInputModeState();
    void updateNoteDotState();
    void updateNoteDurationState();
    void updateNoteAccidentalState();
    void updateTieState();
    void updateSlurState();
    void updateVoicesState();
    void updateArticulationsState();
    void updateRestState();

    bool isNoteInputModeAction(const actions::ActionCode& actionCode) const;
    bool isTupletChooseAction(const actions::ActionCode& actionCode) const;

    ui::UiAction currentNoteInputModeAction() const;

    int itemIndex(const actions::ActionCode& actionCode) const;

    ui::MenuItem makeActionItem(const ui::UiAction& action, const QString& section);
    ui::MenuItem makeAddItem(const QString& section);

    QVariantList subitems(const actions::ActionCode& actionCode) const;
    ui::MenuItemList noteInputMethodItems() const;
    ui::MenuItemList tupletItems() const;
    ui::MenuItemList addItems() const;
    ui::MenuItemList notesItems() const;
    ui::MenuItemList intervalsItems() const;
    ui::MenuItemList measuresItems() const;
    ui::MenuItemList framesItems() const;
    ui::MenuItemList textItems() const;
    ui::MenuItemList linesItems() const;

    bool isNeedShowSubitemsByPressAndHold(const actions::ActionCode& actionCode) const;

    void notifyAboutTupletItemChanged();
    void notifyAboutAddItemChanged();

    std::vector<std::string> currentWorkspaceActions() const;

    ui::MenuItem& item(const actions::ActionCode& actionCode);
    int findNoteInputModeItemIndex() const;

    INotationNoteInputPtr noteInput() const;
    INotationInteractionPtr interaction() const;
    INotationSelectionPtr selection() const;
    INotationUndoStackPtr undoStack() const;

    int resolveCurrentVoiceIndex() const;
    std::set<SymbolId> resolveCurrentArticulations() const;
    bool resolveRestSelected() const;
    DurationType resolveCurrentDurationType() const;
    bool resolveSlurSelected() const;

    bool isNoteInputMode() const;
    NoteInputState noteInputState() const;

    const ChordRest* elementToChordRest(const Element* element) const;

    QList<ui::MenuItem> m_items;
};
}

#endif // MU_NOTATION_NOTEINPUTBARMODEL_H

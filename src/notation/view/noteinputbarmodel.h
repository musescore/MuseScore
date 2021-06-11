/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_NOTATION_NOTEINPUTBARMODEL_H
#define MU_NOTATION_NOTEINPUTBARMODEL_H

#include "ui/view/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "playback/iplaybackcontroller.h"
#include "workspace/iworkspacemanager.h"

namespace mu::notation {
class NoteInputBarModel : public ui::AbstractMenuModel
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, workspace::IWorkspaceManager, workspaceManager)

public:
    explicit NoteInputBarModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load() override;

private:
    enum NoteInputRoles {
        IsMenuSecondaryRole = AbstractMenuModel::Roles::UserRole + 1,
        OrderRole
    };

    void onActionsStateChanges(const actions::ActionCodeList& codes) override;

    INotationPtr notation() const;

    void onNotationChanged();

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

    bool isMenuSecondary(const actions::ActionCode& actionCode) const;

    void notifyAboutTupletItemChanged();
    void notifyAboutAddItemChanged();

    std::vector<std::string> currentWorkspaceActions() const;

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
};
}

#endif // MU_NOTATION_NOTEINPUTBARMODEL_H

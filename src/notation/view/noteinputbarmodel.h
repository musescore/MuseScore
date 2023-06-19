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

#include "uicomponents/view/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "playback/iplaybackcontroller.h"
#include "ui/iuiconfiguration.h"

namespace mu::notation {
class NoteInputBarModel : public uicomponents::AbstractMenuModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, context)
    INJECT(playback::IPlaybackController, playbackController)
    INJECT(ui::IUiConfiguration, uiConfiguration)

    Q_PROPERTY(bool isInputAllowed READ isInputAllowed NOTIFY isInputAllowedChanged)

public:
    explicit NoteInputBarModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load() override;

    bool isInputAllowed() const;

signals:
    void isInputAllowedChanged();

private:
    enum NoteInputRoles {
        IsMenuSecondaryRole = AbstractMenuModel::Roles::UserRole + 1,
        OrderRole,
        SectionRole
    };

    INotationPtr notation() const;
    IMasterNotationPtr masterNotation() const;

    void onNotationChanged();

    void updateItemStateChecked(uicomponents::MenuItem* item, bool checked);

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
    void updateAddState();

    bool isNoteInputModeAction(const actions::ActionCode& actionCode) const;

    ui::UiAction currentNoteInputModeAction() const;

    uicomponents::MenuItem* makeActionItem(const ui::UiAction& action, const QString& section,
                                           const uicomponents::MenuItemList& subitems = {});
    uicomponents::MenuItem* makeAddItem(const QString& section);

    uicomponents::MenuItemList makeSubitems(const actions::ActionCode& actionCode);
    uicomponents::MenuItemList makeNoteInputMethodItems();
    uicomponents::MenuItemList makeCrossStaffBeamingItems();
    uicomponents::MenuItemList makeTupletItems();
    uicomponents::MenuItemList makeAddItems();
    uicomponents::MenuItemList makeNotesItems();
    uicomponents::MenuItemList makeIntervalsItems();
    uicomponents::MenuItemList makeMeasuresItems();
    uicomponents::MenuItemList makeFramesItems();
    uicomponents::MenuItemList makeTextItems();
    uicomponents::MenuItemList makeLinesItems();

    bool isMenuSecondary(const actions::ActionCode& actionCode) const;

    int findNoteInputModeItemIndex() const;

    INotationNoteInputPtr noteInput() const;
    INotationInteractionPtr interaction() const;
    INotationSelectionPtr selection() const;
    INotationUndoStackPtr undoStack() const;

    int resolveCurrentVoiceIndex() const;
    std::set<SymbolId> resolveCurrentArticulations() const;
    bool resolveRestSelected() const;
    DurationType resolveCurrentDurationType() const;

    bool isNoteInputMode() const;
    NoteInputState noteInputState() const;

    const ChordRest* elementToChordRest(const EngravingItem* element) const;
};
}

#endif // MU_NOTATION_NOTEINPUTBARMODEL_H

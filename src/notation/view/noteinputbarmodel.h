/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
class NoteInputBarModel : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT

    Q_PROPERTY(bool isInputAllowed READ isInputAllowed NOTIFY isInputAllowedChanged)

    muse::Inject<context::IGlobalContext> context = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };

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

    void updateItemStateChecked(muse::uicomponents::MenuItem* item, bool checked);

    void updateState();
    void updateNoteInputState();
    void updateNoteInputModeState();
    void updateNoteDotState();
    void updateNoteDurationState();
    void updateNoteAccidentalState();
    void updateTieState();
    void updateLvState();
    void updateSlurState();
    void updateVoicesState();
    void updateArticulationsState();
    void updateRestState();
    void updateAddState();

    bool isNoteInputModeAction(const muse::actions::ActionCode& actionCode) const;

    muse::ui::UiAction currentNoteInputModeAction() const;

    muse::uicomponents::MenuItem* makeActionItem(const muse::ui::UiAction& action, const QString& section,
                                                 const muse::uicomponents::MenuItemList& subitems = {});
    muse::uicomponents::MenuItem* makeAddItem(const QString& section);

    muse::uicomponents::MenuItemList makeSubitems(const muse::actions::ActionCode& actionCode);
    muse::uicomponents::MenuItemList makeNoteInputMethodItems();
    muse::uicomponents::MenuItemList makeCrossStaffBeamingItems();
    muse::uicomponents::MenuItemList makeTupletItems();
    muse::uicomponents::MenuItemList makeAddItems();
    muse::uicomponents::MenuItemList makeNotesItems();
    muse::uicomponents::MenuItemList makeIntervalsItems();
    muse::uicomponents::MenuItemList makeMeasuresItems();
    muse::uicomponents::MenuItemList makeFramesItems();
    muse::uicomponents::MenuItemList makeTextItems();
    muse::uicomponents::MenuItemList makeLinesItems();

    bool isMenuSecondary(const muse::actions::ActionCode& actionCode) const;

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

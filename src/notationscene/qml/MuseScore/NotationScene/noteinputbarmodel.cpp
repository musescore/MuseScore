/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "noteinputbarmodel.h"

#include "types/translatablestring.h"

#include "context/shortcutcontext.h"

#include "notation/inotationelements.h" // IWYU pragma: keep
#include "notation/inotationinteraction.h"
#include "notation/inotationnoteinput.h"
#include "notation/inotationselection.h"
#include "notation/inotationundostack.h"

#include "internal/notationuiactions.h"
#include "notationcommands.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;
using namespace muse::ui;
using namespace muse::uicomponents;

static const QString TOOLBAR_NAME("noteInput");

static const ActionCode ADD_ACTION_CODE("add");
static const ActionCode CROSS_STAFF_BEAMING_CODE("cross-staff-beaming");
static const ActionCode TUPLET_ACTION_CODE("tuplet");

NoteInputBarModel::NoteInputBarModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

QVariant NoteInputBarModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (!isIndexValid(row)) {
        return QVariant();
    }

    const MenuItem* item = items().at(row);
    switch (role) {
    case OrderRole: return row;
    case SectionRole: return item->section();
    }

    return AbstractMenuModel::data(index, role);
}

QHash<int, QByteArray> NoteInputBarModel::roleNames() const
{
    QHash<int, QByteArray> roles = AbstractMenuModel::roleNames();
    roles[OrderRole] = "order";
    roles[SectionRole] = "section";

    return roles;
}

void NoteInputBarModel::classBegin()
{
    init();
}

void NoteInputBarModel::init()
{
    subscribeOnChanges();

    uiState()->toolConfigChanged(TOOLBAR_NAME).onNotify(this, [this]() {
        load();
    });

    commandsController()->isNoteInputAllowedChanged().onReceive(this, [this](bool) {
        emit isInputAllowedChanged();
    });

    load();
}

void NoteInputBarModel::load()
{
    MenuItemList items;

    ToolConfig noteInputConfig = uiState()->toolConfig(TOOLBAR_NAME, NotationUiActions::defaultNoteInputBarConfig());

    int section = 0;
    for (const ToolConfig::Item& citem : noteInputConfig.items) {
        if (!citem.show) {
            continue;
        }

        if (citem.action.empty()) {
            section++;
            continue;
        }

        MenuItemList subitems;
        if (citem.action == CROSS_STAFF_BEAMING_CODE) {
            subitems = makeCrossStaffBeamingItems();
        } else if (citem.action == TUPLET_ACTION_CODE) {
            subitems = makeTupletItems();
        }

        MenuItem* item = makeActionItem(uiActionsRegister()->action(citem.action), QString::number(section), subitems);
        items << item;
    }

    items << makeAddItem(QString::number(++section));
    setItems(items);
}

bool NoteInputBarModel::isInputAllowed() const
{
    return commandsController()->isNoteInputAllowed();
}

MenuItem* NoteInputBarModel::makeActionItem(const UiAction& action, const QString& section,
                                            const muse::uicomponents::MenuItemList& subitems)
{
    MenuItem* item = new MenuItem(action, this);
    item->setSection(section);
    item->setSubitems(subitems);
    return item;
}

MenuItem* NoteInputBarModel::makeAddItem(const QString& section)
{
    static const UiAction addAction(ADD_ACTION_CODE, UiCtxAny, mu::context::CTX_ANY,
                                    TranslatableString("global", "Add"),
                                    IconCode::Code::PLUS);

    return makeActionItem(addAction, section, makeAddItems());
}

MenuItemList NoteInputBarModel::makeCrossStaffBeamingItems()
{
    MenuItemList items {
        makeMenuItem("move-up"),
        makeMenuItem("move-down")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeTupletItems()
{
    MenuItemList items {
        makeMenuItem(ADD_DUPLET_COMMAND),
        makeMenuItem(ADD_TRIPLET_COMMAND),
        makeMenuItem(ADD_QUADRUPLET_COMMAND),
        makeMenuItem(ADD_QUINTUPLET_COMMAND),
        makeMenuItem(ADD_SEXTUPLET_COMMAND),
        makeMenuItem(ADD_SEPTUPLET_COMMAND),
        makeMenuItem(ADD_OCTUPLET_COMMAND),
        makeMenuItem(ADD_NONUPLET_COMMAND),
        makeMenuItem(SHOW_TUPLET_CONFIGURE_COMMAND)
    };

    return items;
}

MenuItemList NoteInputBarModel::makeAddItems()
{
    MenuItemList items {
        makeMenu(TranslatableString("notation", "Notes"), makeNotesItems()),
        makeMenu(TranslatableString("notation", "Intervals"), makeIntervalsItems()),
        makeMenu(TranslatableString("notation", "Measures"), makeMeasuresItems()),
        makeMenu(TranslatableString("notation", "Frames"), makeFramesItems()),
        makeMenu(TranslatableString("notation", "Text"), makeTextItems()),
        makeMenu(TranslatableString("notation", "Lines"), makeLinesItems()),
        makeMenu(TranslatableString("notation", "Chords and fretboard diagrams"), makeChordAndFretboardDiagramsItems()),
    };

    return items;
}

MenuItemList NoteInputBarModel::makeNotesItems()
{
    MenuItemList items {
        makeMenuItem("command://notation/enter-note-c"),
        makeMenuItem("command://notation/enter-note-d"),
        makeMenuItem("command://notation/enter-note-e"),
        makeMenuItem("command://notation/enter-note-f"),
        makeMenuItem("command://notation/enter-note-g"),
        makeMenuItem("command://notation/enter-note-a"),
        makeMenuItem("command://notation/enter-note-b"),
        makeSeparator(),
        makeMenuItem("command://notation/add-note-c"),
        makeMenuItem("command://notation/add-note-d"),
        makeMenuItem("command://notation/add-note-e"),
        makeMenuItem("command://notation/add-note-f"),
        makeMenuItem("command://notation/add-note-g"),
        makeMenuItem("command://notation/add-note-a"),
        makeMenuItem("command://notation/add-note-b")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeIntervalsItems()
{
    MenuItemList items {
        makeMenuItem("interval1"),
        makeMenuItem("interval2"),
        makeMenuItem("interval3"),
        makeMenuItem("interval4"),
        makeMenuItem("interval5"),
        makeMenuItem("interval6"),
        makeMenuItem("interval7"),
        makeMenuItem("interval8"),
        makeMenuItem("interval9"),
        makeMenuItem("interval10"),
        makeSeparator(),
        makeMenuItem("interval-2"),
        makeMenuItem("interval-3"),
        makeMenuItem("interval-4"),
        makeMenuItem("interval-5"),
        makeMenuItem("interval-6"),
        makeMenuItem("interval-7"),
        makeMenuItem("interval-8"),
        makeMenuItem("interval-9"),
        makeMenuItem("interval-10")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem("insert-measure"),
        makeMenuItem("append-measure"),
        makeSeparator(),
        makeMenuItem("insert-measures"),
        makeMenuItem("insert-measures-after-selection"),
        makeSeparator(),
        makeMenuItem("insert-measures-at-start-of-score"),
        makeMenuItem("append-measures")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeFramesItems()
{
    MenuItemList items {
        makeMenuItem("insert-hbox"),
        makeMenuItem("insert-vbox"),
        makeMenuItem("insert-textframe"),
        makeMenuItem("insert-fretframe"),
        makeSeparator(),
        makeMenu(TranslatableString("notation", "Insert at end of score"), makeFramesAppendItems())
    };

    return items;
}

MenuItemList NoteInputBarModel::makeFramesAppendItems()
{
    MenuItemList items {
        makeMenuItem("append-hbox"),
        makeMenuItem("append-vbox"),
        makeMenuItem("append-textframe"),
        makeMenuItem("append-fretframe")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeTextItems()
{
    MenuItemList items {
        makeMenuItem("title-text"),
        makeMenuItem("subtitle-text"),
        makeMenuItem("composer-text"),
        makeMenuItem("poet-text"),
        makeMenuItem("part-text"),
        makeSeparator(),
        makeMenuItem("system-text"),
        makeMenuItem("staff-text"),
        makeMenuItem("add-dynamic"),
        makeMenuItem("expression-text"),
        makeMenuItem("rehearsalmark-text"),
        makeMenuItem("instrument-change-text"),
        makeMenuItem("fingering-text"),
        makeSeparator(),
        makeMenuItem("sticking-text"),
        makeMenuItem("chord-text"),
        makeMenuItem("roman-numeral-text"),
        makeMenuItem("nashville-number-text"),
        makeMenuItem("lyrics"),
        makeMenuItem("figured-bass"),
        makeMenuItem("tempo")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem("command://notation/add-slur"),
        makeMenuItem("add-hairpin"),
        makeMenuItem("add-hairpin-reverse"),
        makeMenuItem("add-8va"),
        makeMenuItem("add-8vb"),
        makeMenuItem("add-noteline")
    };

    return items;
}

MenuItemList NoteInputBarModel::makeChordAndFretboardDiagramsItems()
{
    MenuItemList items {
        makeMenuItem("chord-text"),
        makeMenuItem("add-fretboard-diagram"),
        makeSeparator(),
        makeMenuItem("insert-fretframe", TranslatableString("notation", "Fretboard diagram legend"))
    };

    return items;
}

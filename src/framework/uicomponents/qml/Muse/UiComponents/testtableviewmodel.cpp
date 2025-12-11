/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "testtableviewmodel.h"

#include "internal/tableviewheader.h"

using namespace muse::uicomponents;

TestTableViewModel::TestTableViewModel(QObject* parent)
    : AbstractTableViewModel(parent)
{
    load();
}

void TestTableViewModel::load()
{
    QVector<QVector<TableViewCell*> > table;

    QVector<TableViewCell*> row1 = {
        makeCell(Val("Text 1")),
        makeCell(Val(ValList { Val("Item 1"), Val("Item 2") }), Val("Item 1")),
        makeCell(Val("00h00m00.000 s")),
        makeCell(Val(0.0)),
        makeCell(Val("00h00m00.000 s")),
    };
    table.append(row1);

    QVector<TableViewCell*> row2 = {
        makeCell(Val("Text 2")),
        makeCell(Val(ValList { Val("Item 1"), Val("Item 2") }), Val("Item 2")),
        makeCell(Val("00h00m00.000 s")),
        makeCell(Val(0.0)),
        makeCell(Val("00h00m00.000 s")),
    };
    table.append(row2);

    QList<TableViewHeader*> hHeaders = {
        makeHorizontalHeader("Column 1", TableViewCellType::Type::String, TableViewCellEditMode::Mode::DoubleClick, 100),
        makeHorizontalHeader("Column 2", TableViewCellType::Type::List, TableViewCellEditMode::Mode::DoubleClick, 100),
        makeHorizontalHeader("Column 3", static_cast<TableViewCellType::Type>(TestTableViewCellType::Type::Custom),
                             TableViewCellEditMode::Mode::StartInEdit, 200, makeAvailableFormats()),
        makeHorizontalHeader("Column 4", TableViewCellType::Type::Double, TableViewCellEditMode::Mode::StartInEdit, 200),
        makeHorizontalHeader("Column 5", TableViewCellType::Type::String, TableViewCellEditMode::Mode::StartInEdit, 300)
    };

    setHorizontalHeaders(hHeaders);

    setTable(table);
}

MenuItemList TestTableViewModel::makeAvailableFormats()
{
    MenuItemList items;

    ui::UiActionState enabledState;
    enabledState.enabled = true;

    ui::UiAction action;

    action.code = "defaul";
    action.title = TranslatableString::untranslatable("Default");
    MenuItem* item1 = new MenuItem(action);
    item1->setState(enabledState);
    item1->setSelectable(true);
    items << item1;

    action.code = "accent";
    action.title = TranslatableString::untranslatable("Accent");
    MenuItem* item2 = new MenuItem(action);
    item2->setState(enabledState);
    item1->setSelectable(true);
    items << item2;

    return items;
}

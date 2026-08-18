/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include <gtest/gtest.h>

#include <QStandardItemModel>
#include <QString>

#include "internal/palettemodel.h"

#include "engraving/types/typesconv.h"

using namespace mu::palette;
using namespace mu::engraving;

static const QString PALETTE_NAME("Key signatures");
static const std::initializer_list<Key> KEYS = { Key::F_S, Key::B_B, Key::C };

// The name the key signature palette gives a cell, so that these tests keep
// exercising what users see if those names are ever reworded.
static QString keyName(Key key)
{
    return TConv::translatedUserName(key).toQString();
}

//---------------------------------------------------------
//   Palette_PaletteSearchTests
///   Searching a palette of key signatures, whose names contain the
///   accidental signs ♯ and ♭ that users cannot type on a keyboard.
//---------------------------------------------------------

class Palette_PaletteSearchTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        QStandardItem* palette = new QStandardItem();
        palette->setToolTip(PALETTE_NAME);

        for (Key key : KEYS) {
            QStandardItem* cell = new QStandardItem();
            cell->setToolTip(keyName(key));
            palette->appendRow(cell);
        }

        m_sourceModel.appendRow(palette);

        m_proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel.setSourceModel(&m_sourceModel);
    }

    std::vector<std::string> search(const QString& searchText)
    {
        m_proxyModel.setSearchText(searchText);

        std::vector<std::string> foundCellNames;

        for (int paletteRow = 0; paletteRow < m_proxyModel.rowCount(); ++paletteRow) {
            const QModelIndex paletteIndex = m_proxyModel.index(paletteRow, 0);
            for (int cellRow = 0; cellRow < m_proxyModel.rowCount(paletteIndex); ++cellRow) {
                const QModelIndex cellIndex = m_proxyModel.index(cellRow, 0, paletteIndex);
                foundCellNames.push_back(cellIndex.data(Qt::ToolTipRole).toString().toStdString());
            }
        }

        return foundCellNames;
    }

    static std::vector<std::string> expected(std::initializer_list<Key> keys)
    {
        std::vector<std::string> cellNames;
        for (Key key : keys) {
            cellNames.push_back(keyName(key).toStdString());
        }

        return cellNames;
    }

private:
    QStandardItemModel m_sourceModel;
    PaletteCellFilterProxyModel m_proxyModel;
};

TEST_F(Palette_PaletteSearchTests, MatchesAccidentalSignsAsTyped)
{
    EXPECT_EQ(search("f♯"), expected({ Key::F_S }));
    EXPECT_EQ(search("b♭"), expected({ Key::B_B }));
}

TEST_F(Palette_PaletteSearchTests, MatchesAccidentalSignsBySymbolShorthand)
{
    EXPECT_EQ(search("f#"), expected({ Key::F_S }));
    EXPECT_EQ(search("bb"), expected({ Key::B_B }));
}

TEST_F(Palette_PaletteSearchTests, MatchesAccidentalSignsBySpelledOutName)
{
    EXPECT_EQ(search("f sharp"), expected({ Key::F_S }));
    EXPECT_EQ(search("b flat"), expected({ Key::B_B }));
}

TEST_F(Palette_PaletteSearchTests, MatchesNamesWithoutAccidentalSigns)
{
    EXPECT_EQ(search("c major"), expected({ Key::C }));
}

TEST_F(Palette_PaletteSearchTests, MatchesAllCellsOfAPaletteByPaletteName)
{
    EXPECT_EQ(search(PALETTE_NAME), expected({ Key::F_S, Key::B_B, Key::C }));
}

TEST_F(Palette_PaletteSearchTests, MatchesNothingWhenNameIsUnknown)
{
    EXPECT_TRUE(search("h major").empty());
}

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

#include "symboldialog.h"

#include "engraving/style/style.h"
#include "engraving/types/symnames.h"
#include "engraving/infrastructure/smufl.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/symbol.h"

#include "palettewidget.h"

using namespace mu::engraving;
using namespace mu::palette;

namespace mu::engraving {
extern MasterScore* gpaletteScore;
}

//---------------------------------------------------------
//   createSymbolPalette
//---------------------------------------------------------

void SymbolDialog::createSymbolPalette()
{
    m_symbolsWidget = new PaletteWidget(this);
    createSymbols();
}

//---------------------------------------------------------
//   createSymbols
//---------------------------------------------------------

void SymbolDialog::createSymbols()
{
    int currentIndex = fontList->currentIndex();
    const IEngravingFontPtr f = engravingFonts()->fonts()[currentIndex];
    // init the font if not done yet
    engravingFonts()->fontByName(f->name());
    m_symbolsWidget->clear();
    for (auto name : Smufl::smuflRanges().at(range)) {
        SymId id = SymNames::symIdByName(name);
        if (search->text().isEmpty()
            || SymNames::translatedUserNameForSymId(id).toQString().contains(search->text(), Qt::CaseInsensitive)) {
            auto s = std::make_shared<Symbol>(gpaletteScore->dummy());
            s->setSym(SymId(id), f);
            m_symbolsWidget->appendElement(s, SymNames::translatedUserNameForSymId(SymId(id)));
        }
    }
}

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

SymbolDialog::SymbolDialog(const QString& s, QWidget* parent)
    : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
{
    setupUi(this);
    range = s;          // smufl symbol range
    int idx = 0;
    int currentIndex = 0;
    Score* score = globalContext()->currentNotation()->elements()->msScore();
    std::string styleFont = score ? score->style().styleSt(Sid::musicalSymbolFont).toStdString() : "";
    for (const IEngravingFontPtr& f : engravingFonts()->fonts()) {
        fontList->addItem(QString::fromStdString(f->name()));
        if (!styleFont.empty() && f->name() == styleFont) {
            currentIndex = idx;
            styleFont = "";
        }
        ++idx;
    }
    fontList->setCurrentIndex(currentIndex);

    QLayout* layout = new QVBoxLayout();
    frame->setLayout(layout);
    createSymbolPalette();

    QScrollArea* symbolsArea = new PaletteScrollArea(m_symbolsWidget);
    symbolsArea->setFocusProxy(m_symbolsWidget);
    symbolsArea->setFocusPolicy(Qt::TabFocus);
    layout->addWidget(symbolsArea);

    m_symbolsWidget->setAcceptDrops(false);
    m_symbolsWidget->setDrawGrid(true);
    m_symbolsWidget->setSelectable(true);

    connect(systemFlag, &QCheckBox::stateChanged, this, &SymbolDialog::systemFlagChanged);
    connect(fontList, &QComboBox::currentIndexChanged, this, &SymbolDialog::systemFontChanged);

    symbolsArea->setWidget(m_symbolsWidget);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

//---------------------------------------------------------
//   systemFlagChanged
//---------------------------------------------------------

void SymbolDialog::systemFlagChanged(int state)
{
    bool sysFlag = state == Qt::Checked;
    for (int i = 0; i < m_symbolsWidget->actualCellCount(); ++i) {
        ElementPtr e = m_symbolsWidget->elementForCellAt(i);
        if (e && e->type() == ElementType::SYMBOL) {
            std::dynamic_pointer_cast<Symbol>(e)->setSystemFlag(sysFlag);
        }
    }
}

//---------------------------------------------------------
//   systemFontChanged
//---------------------------------------------------------

void SymbolDialog::systemFontChanged(int)
{
    createSymbols();
}

void SymbolDialog::on_search_textChanged(const QString& searchPhrase)
{
    Q_UNUSED(searchPhrase);
    createSymbols();
}

void SymbolDialog::on_clearSearch_clicked()
{
    search->clear();
    createSymbols();
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void SymbolDialog::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}

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

#include "masterpalette.h"

#include "translation.h"

#include "palettewidget.h"
#include "keyedit.h"
#include "timedialog.h"
#include "symboldialog.h"

#include "internal/palettecreator.h"

#include "engraving/infrastructure/smufl.h"
#include "ui/view/widgetstatestore.h"

#include "log.h"

using namespace mu::palette;
using namespace muse::ui;
using namespace mu::engraving;

void MasterPalette::setSelectedPaletteName(const QString& name)
{
    for (int idx = 0; idx < treeWidget->topLevelItemCount(); ++idx) {
        if (treeWidget->topLevelItem(idx)->text(0) == name) {
            treeWidget->setCurrentItem(treeWidget->topLevelItem(idx));
            emit selectedPaletteNameChanged(name);
            break;
        }
    }
}

QString MasterPalette::selectedPaletteName() const
{
    return treeWidget->currentItem()->text(0);
}

//---------------------------------------------------------
//   addPalette
//---------------------------------------------------------

void MasterPalette::addPalette(PalettePtr palette)
{
    TRACEFUNC;

    PaletteWidget* widget = new PaletteWidget(this);
    widget->setReadOnly(true);
    widget->setPalette(palette);

    PaletteScrollArea* psa = new PaletteScrollArea(widget);
    psa->setRestrictHeight(false);
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(widget->name()));
    item->setData(0, Qt::UserRole, stack->count());
    item->setText(0, muse::qtrc("palette", widget->name().toUtf8().data()).replace("&&", "&"));
    stack->addWidget(psa);
    treeWidget->addTopLevelItem(item);
}

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

MasterPalette::MasterPalette(QWidget* parent)
    : TopLevelDialog(parent)
{
    TRACEFUNC;

    setObjectName("MasterPalette");
    setupUi(this);

    treeWidget->clear();

    addPalette(PaletteCreator::newClefsPalette());
    m_keyEditor = new KeyEditor;

    m_keyItem = new QTreeWidgetItem();
    m_keyItem->setData(0, Qt::UserRole, stack->count());
    stack->addWidget(m_keyEditor);
    treeWidget->addTopLevelItem(m_keyItem);

    m_timeItem = new QTreeWidgetItem();
    m_timeItem->setData(0, Qt::UserRole, stack->count());
    m_timeDialog = new TimeDialog;
    stack->addWidget(m_timeDialog);
    treeWidget->addTopLevelItem(m_timeItem);

    addPalette(PaletteCreator::newBracketsPalette());
    addPalette(PaletteCreator::newAccidentalsPalette());
    addPalette(PaletteCreator::newArticulationsPalette());
    addPalette(PaletteCreator::newOrnamentsPalette());
    addPalette(PaletteCreator::newBreathPalette());
    addPalette(PaletteCreator::newGraceNotePalette());
    addPalette(PaletteCreator::newNoteHeadsPalette());
    addPalette(PaletteCreator::newLinesPalette());
    addPalette(PaletteCreator::newBarLinePalette());
    addPalette(PaletteCreator::newArpeggioPalette());
    addPalette(PaletteCreator::newTremoloPalette());
    addPalette(PaletteCreator::newTextPalette());
    addPalette(PaletteCreator::newTempoPalette());
    addPalette(PaletteCreator::newDynamicsPalette());
    addPalette(PaletteCreator::newFingeringPalette());
    addPalette(PaletteCreator::newRepeatsPalette());
    addPalette(PaletteCreator::newFretboardDiagramPalette());
    addPalette(PaletteCreator::newAccordionPalette());
    addPalette(PaletteCreator::newBagpipeEmbellishmentPalette());
    addPalette(PaletteCreator::newLayoutPalette());
    addPalette(PaletteCreator::newBeamPalette());

    m_symbolItem = new QTreeWidgetItem();
    m_idxAllSymbols = stack->count();
    m_symbolItem->setData(0, Qt::UserRole, m_idxAllSymbols);
    m_symbolItem->setText(0, QT_TRANSLATE_NOOP("palette", "Symbols"));
    treeWidget->addTopLevelItem(m_symbolItem);
    stack->addWidget(new SymbolDialog(Smufl::SMUFL_ALL_SYMBOLS));

    // Add "All symbols" entry to be first in the list of categories
    QTreeWidgetItem* child = new QTreeWidgetItem({ Smufl::SMUFL_ALL_SYMBOLS });
    child->setData(0, Qt::UserRole, m_idxAllSymbols);
    m_symbolItem->addChild(child);

    const auto& ranges = Smufl::smuflRanges();
    size_t idx = 0;
    for (auto it = ranges.cbegin(); it != ranges.cend(); ++it) {
        const String& rangeName = it->first;
        if (rangeName == Smufl::SMUFL_ALL_SYMBOLS) {
            continue;
        }

        QTreeWidgetItem* chld = new QTreeWidgetItem({ rangeName });
        chld->setData(0, Qt::UserRole, m_idxAllSymbols + static_cast<int>(idx) + 1);

        m_symbolItem->addChild(chld);
        ++idx;
    }

    connect(treeWidget, &QTreeWidget::currentItemChanged, this, &MasterPalette::currentChanged);
    connect(treeWidget, &QTreeWidget::itemClicked, this, &MasterPalette::clicked);
    retranslate(true);
}

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void MasterPalette::retranslate(bool firstTime)
{
    m_keyItem->setText(0, muse::qtrc("palette", "Key signatures"));
    m_timeItem->setText(0, muse::qtrc("palette", "Time signatures"));
    m_symbolItem->setText(0, muse::qtrc("palette", "Symbols"));
    if (!firstTime) {
        retranslateUi(this);
    }
}

//---------------------------------------------------------
//   currentChanged
//---------------------------------------------------------

void MasterPalette::currentChanged(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    int idx = item->data(0, Qt::UserRole).toInt();

    if (idx > m_idxAllSymbols) {
        if (!m_symbolWidgets.contains(idx)) {
            String rangeName = std::next(Smufl::smuflRanges().cbegin(), idx - m_idxAllSymbols - 1)->first;
            SymbolDialog* dialog = new SymbolDialog(rangeName.toQString());
            m_symbolWidgets[idx] = dialog;
            stack->addWidget(dialog);
        }

        stack->setCurrentWidget(m_symbolWidgets[idx]);
        return;
    }

    if (idx != -1) {
        stack->setCurrentIndex(idx);
    }
}

//---------------------------------------------------------
//   clicked
//---------------------------------------------------------

void MasterPalette::clicked(QTreeWidgetItem* item, int)
{
    int idx = item->data(0, Qt::UserRole).toInt();
    if (idx == m_idxAllSymbols) {
        item->setExpanded(!item->isExpanded());
        if (idx != -1) {
            stack->setCurrentIndex(idx);
        }
    }
}

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MasterPalette::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    TopLevelDialog::showEvent(event);
}

void MasterPalette::closeEvent(QCloseEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    if (m_timeDialog->dirty()) {
        m_timeDialog->save();
    }
    if (m_keyEditor->dirty()) {
        m_keyEditor->save();
    }
    setResult(QDialog::Accepted);
    QWidget::closeEvent(event);
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void MasterPalette::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}

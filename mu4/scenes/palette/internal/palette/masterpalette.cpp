//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

//#include "musescore.h"
#include "masterpalette.h"
//#include "menus.h"
#include "symboldialog.h"
#include "palette/palette.h"
#include "keyedit.h"
#include "libmscore/score.h"
#include "libmscore/clef.h"
#include "libmscore/fingering.h"
#include "libmscore/spacer.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/dynamic.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/glissando.h"
#include "libmscore/repeat.h"
#include "libmscore/breath.h"
#include "libmscore/harmony.h"
#include "libmscore/text.h"
#include "libmscore/tempotext.h"
#include "libmscore/instrchange.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/stafftext.h"
#include "libmscore/note.h"
#include "libmscore/tremolo.h"
#include "libmscore/chordline.h"

#include "timedialog.h"

#include "palette/palettecreator.h"
#include "smuflranges.h"
#include "widgetstatestore.h"

namespace Ms {
//---------------------------------------------------------
//   createPalette
//---------------------------------------------------------

Palette* MasterPalette::createPalette(int w, int h, bool grid, double mag)
{
    Palette* sp = new Palette;
    PaletteScrollArea* psa = new PaletteScrollArea(sp);
    psa->setRestrictHeight(false);
    sp->setMag(mag);
    sp->setGrid(w, h);
    sp->setDrawGrid(grid);
    sp->setReadOnly(true);
    stack->addWidget(psa);
    return sp;
}

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void MasterPalette::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
        close();
        return;
    }
    QWidget::keyPressEvent(ev);
}

//---------------------------------------------------------
//   selectItem
//---------------------------------------------------------

void MasterPalette::selectItem(const QString& s)
{
    for (int idx = 0; idx < treeWidget->topLevelItemCount(); ++idx) {
        if (treeWidget->topLevelItem(idx)->text(0) == s) {
            treeWidget->setCurrentItem(treeWidget->topLevelItem(idx));
            break;
        }
    }
}

//---------------------------------------------------------
//   selectedItem
//---------------------------------------------------------

QString MasterPalette::selectedItem()
{
    return treeWidget->currentItem()->text(0);
}

//---------------------------------------------------------
//   addPalette
//---------------------------------------------------------

void MasterPalette::addPalette(Palette* sp)
{
    sp->setReadOnly(true);
    PaletteScrollArea* psa = new PaletteScrollArea(sp);
    psa->setRestrictHeight(false);
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList(sp->name()));
    item->setData(0, Qt::UserRole, stack->count());
    item->setText(0, qApp->translate("Palette", sp->name().toUtf8().data()).replace("&&","&"));
    stack->addWidget(psa);
    treeWidget->addTopLevelItem(item);
}

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

MasterPalette::MasterPalette(QWidget* parent)
    : QWidget(parent, Qt::Dialog)
{
    setObjectName("MasterPalette");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    treeWidget->clear();

    addPalette(PaletteCreator::newClefsPalette());
    keyEditor = new KeyEditor;

    keyItem = new QTreeWidgetItem();
    keyItem->setData(0, Qt::UserRole, stack->count());
    stack->addWidget(keyEditor);
    treeWidget->addTopLevelItem(keyItem);

    timeItem = new QTreeWidgetItem();
    timeItem->setData(0, Qt::UserRole, stack->count());
    timeDialog = new TimeDialog;
    stack->addWidget(timeDialog);
    treeWidget->addTopLevelItem(timeItem);

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
    addPalette(PaletteCreator::newBreaksPalette());
    addPalette(PaletteCreator::newFramePalette());
    addPalette(PaletteCreator::newBeamPalette());

    symbolItem = new QTreeWidgetItem();
    idxAllSymbols = stack->count();
    symbolItem->setData(0, Qt::UserRole, idxAllSymbols);
    symbolItem->setText(0, QT_TRANSLATE_NOOP("MasterPalette", "Symbols"));
    treeWidget->addTopLevelItem(symbolItem);
    stack->addWidget(new SymbolDialog(mu::SMUFL_ALL_SYMBOLS));

    // Add "All symbols" entry to be first in the list of categories
    QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(mu::SMUFL_ALL_SYMBOLS));
    child->setData(0, Qt::UserRole, idxAllSymbols);
    symbolItem->addChild(child);

    for (const QString& s : mu::smuflRanges()->keys()) {
        if (s == mu::SMUFL_ALL_SYMBOLS) {
            continue;
        }
        QTreeWidgetItem* chld = new QTreeWidgetItem(QStringList(s));
        chld->setData(0, Qt::UserRole, stack->count());
        symbolItem->addChild(chld);
        stack->addWidget(new SymbolDialog(s));
    }

    connect(treeWidget, &QTreeWidget::currentItemChanged, this, &MasterPalette::currentChanged);
    connect(treeWidget, &QTreeWidget::itemClicked, this, &MasterPalette::clicked);
    retranslate(true);

    WidgetStateStore::restoreGeometry(this);
}

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void MasterPalette::retranslate(bool firstTime)
{
    keyItem->setText(0, qApp->translate("Palette", "Key Signatures"));
    timeItem->setText(0, qApp->translate("Palette", "Time Signatures"));
    symbolItem->setText(0, qApp->translate("MasterPalette", "Symbols"));
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
    if (idx == idxAllSymbols) {
        item->setExpanded(!item->isExpanded());
        if (idx != -1) {
            stack->setCurrentIndex(idx);
        }
    }
}

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void MasterPalette::closeEvent(QCloseEvent* ev)
{
    WidgetStateStore::saveGeometry(this);
    if (timeDialog->dirty()) {
        timeDialog->save();
    }
    if (keyEditor->dirty()) {
        keyEditor->save();
    }
    emit closed(false);
    QWidget::closeEvent(ev);
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
}

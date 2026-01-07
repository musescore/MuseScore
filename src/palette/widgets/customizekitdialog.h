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
#ifndef MU_PALETTE_CUSTOMIZEKITDIALOG_H
#define MU_PALETTE_CUSTOMIZEKITDIALOG_H

#include "ui_customizekitdialog.h"

#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/rendering/isinglerenderer.h"
#include "ui/iuiconfiguration.h"

#include "engraving/dom/drumset.h"

namespace mu::palette {
//---------------------------------------------------------
//   CustomizeKitDialog
//---------------------------------------------------------

class CustomizeKitDialog : public QDialog, private Ui::CustomizeKitDialog, public muse::Injectable
{
    Q_OBJECT

public:
    inline static muse::GlobalInject<engraving::IEngravingFontsProvider> engravingFonts;
    inline static muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<engraving::rendering::ISingleRenderer> engravingRenderer = { this };

public:
    CustomizeKitDialog(QWidget* parent = nullptr);

private slots:
    void bboxClicked(QAbstractButton* button);
    void itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void nameChanged(const QString&);
    void valueChanged();
    void load();
    void save();
    void customGboxToggled(bool);
    void customQuarterChanged(int);

    void defineShortcut();

private:
    void initDrumsetAndKey();

    void apply();
    void cancel();

    QTreeWidgetItem* loadPitchesList(); // Returns a tree item to select, if applicable...

    void updateExample();

    void fillCustomNoteheadsDataFromComboboxes(int pitch);
    void setCustomNoteheadsGUIEnabled(bool enabled);

    void setEnabledPitchControls(bool enable);
    void fillNoteheadsComboboxes(bool customGroup, int pitch);

    void notifyAboutNoteInputStateChanged();

    notation::INotationPtr m_notation;
    notation::InstrumentKey m_instrumentKey;

    engraving::Drumset m_originDrumset;
    engraving::Drumset m_editedDrumset;
};
}

#endif // MU_PALETTE_CUSTOMIZEKITDIALOG_H

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

#pragma once

#include "ui_timesignaturepropertiesdialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "ui/iuiconfiguration.h"

namespace mu::engraving {
class TimeSig;
}

namespace mu::palette {
class TimeSignaturePropertiesDialog : public QDialog, public Ui::TimeSigProperties
{
    Q_OBJECT

    INJECT(mu::context::IGlobalContext, globalContext)
    INJECT(muse::ui::IUiConfiguration, uiConfiguration)

public:
    TimeSignaturePropertiesDialog(QWidget* parent = nullptr);
    ~TimeSignaturePropertiesDialog() override;

private slots:
    void accept() override;

private:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

    mu::notation::INotationPtr notation() const;

    engraving::TimeSig* m_originTimeSig = nullptr;
    engraving::TimeSig* m_editedTimeSig = nullptr;
};
}

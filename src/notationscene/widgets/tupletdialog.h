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
#pragma once

#include "ui/view/widgetdialog.h"

#include "ui_tupletdialog.h"

#include "modularity/ioc.h"
#include "project/iprojectsession.h"
#include "rcommand/icommanddispatcher.h"
#include "notation/inotationconfiguration.h"
#include "engraving/types/fraction.h"

namespace mu::engraving {
enum class TupletNumberType : unsigned char;
enum class TupletBracketType : unsigned char;
enum class TupletSelectedDuration : unsigned char;
}

namespace mu::notation {
// The choices last confirmed with OK, kept until the score closes
struct TupletDialogState {
    std::weak_ptr<IMasterNotation> masterNotation;
    engraving::Fraction ratio;
    engraving::TupletSelectedDuration selectedDurationAs {};
    engraving::TupletNumberType numberType {};
    engraving::TupletBracketType bracketType {};
};

class TupletDialog : public muse::ui::WidgetDialog, private Ui::TupletDialog
{
    Q_OBJECT

    muse::ContextInject<project::IProjectSession> projectSession = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> dispatcher = { this };
    muse::GlobalInject<INotationConfiguration> configuration;

    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

public:
    TupletDialog(QWidget* parent = nullptr);

    void componentComplete() override;

    void defaultToStyleSettings();

    engraving::TupletNumberType numberType() const;
    engraving::TupletBracketType bracketType() const;
    engraving::TupletSelectedDuration selectedDurationAs() const;

    INotationStylePtr style() const;
    INotationPtr notation() const;

    void apply();

private slots:
    void bboxClicked(QAbstractButton* button);

private:
    void setNumberType(engraving::TupletNumberType type);
    void setBracketType(engraving::TupletBracketType type);
    void setSelectedDurationAs(engraving::TupletSelectedDuration value);

    static TupletDialogState& previousState();
    void saveState();
    void restorePreviousState();
};
}

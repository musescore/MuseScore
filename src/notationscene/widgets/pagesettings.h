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
#ifndef MU_NOTATION_PAGESETTINGS_H
#define MU_NOTATION_PAGESETTINGS_H

#include "ui_pagesettings.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "framework/global/iglobalconfiguration.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class PageSettings : public QDialog, private Ui::PageSettingsBase, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<mu::context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::IGlobalConfiguration> configuration = { this };

public:
    explicit PageSettings(QWidget* parent = 0);

public slots:
    void accept();
    void reject();

private:
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    void keyPressEvent(QKeyEvent* event);

    void updateValues();
    void blockSignals(bool);
    void setMarginsMax(double);
    void applyToScore(mu::engraving::Score*);

    mu::engraving::Score* score() const;
    double styleValueDouble(StyleId styleId) const;
    bool styleValueBool(StyleId styleId) const;
    void setStyleValue(StyleId styleId, const PropertyValue& newValue) const;

    bool mmUnit = false;
    bool _changeFlag = false;

private slots:
    void mmClicked();
    void inchClicked();
    void pageFormatSelected(int);

    void applyToAllParts();
    void buttonBoxClicked(QAbstractButton*);

    void twosidedToggled(bool);
    void otmChanged(double val);
    void obmChanged(double val);
    void olmChanged(double val);
    void ormChanged(double val);
    void etmChanged(double val);
    void ebmChanged(double val);
    void elmChanged(double val);
    void ermChanged(double val);
    void spatiumChanged(double val);
    void pageHeightChanged(double);
    void pageWidthChanged(double);
    void pageOffsetChanged(int val);
    void orientationClicked();
    void on_resetPageStyleButton_clicked();
};
}
#endif // MU_NOTATION_PAGESETTINGS_H

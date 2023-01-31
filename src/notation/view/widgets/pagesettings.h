/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
class PageSettings : public QDialog, private Ui::PageSettingsBase
{
    Q_OBJECT

    INJECT(notation, mu::context::IGlobalContext, globalContext)
    INJECT(notation, mu::framework::IGlobalConfiguration, configuration)

public:
    explicit PageSettings(QWidget* parent = 0);
    PageSettings(const PageSettings&);

public slots:
    void accept();
    void reject();

private:
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);

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
    void otmChanged();
    void obmChanged();
    void olmChanged();
    void ormChanged();
    void etmChanged();
    void ebmChanged();
    void elmChanged();
    void ermChanged();
    void spatiumChanged();
    void pageHeightChanged();
    void pageWidthChanged();
    void pageOffsetChanged();
    void orientationClicked();
    void on_resetPageStyleButton_clicked();
};
}
#endif // MU_NOTATION_PAGESETTINGS_H

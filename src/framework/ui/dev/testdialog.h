/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_UI_TESTDIALOG_H
#define MU_UI_TESTDIALOG_H

#include "view/widgetdialog.h"

namespace Ui {
class TestDialog;
}

namespace mu::ui {
class TestDialog : public WidgetDialog
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    TestDialog(const TestDialog& dialog);
    explicit TestDialog(QWidget* parent = nullptr);
    ~TestDialog();

    QString title() const;

    static int static_metaTypeId();
    int metaTypeId() const override;

public slots:
    void setTitle(QString title);

signals:
    void titleChanged(QString title);

private:
    Ui::TestDialog* ui;
    QString m_title;
};
}

#endif // MU_UI_TESTDIALOG_H

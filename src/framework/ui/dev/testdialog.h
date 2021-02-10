//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

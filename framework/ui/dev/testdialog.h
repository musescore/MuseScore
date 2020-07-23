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
#ifndef MU_FRAMEWORK_TESTDIALOG_H
#define MU_FRAMEWORK_TESTDIALOG_H

#include <QDialog>

namespace Ui {
class TestDialog;
}

namespace mu {
namespace framework {
class TestDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    TestDialog(const TestDialog& dialog);
    explicit TestDialog(QWidget* parent = nullptr);
    ~TestDialog();

    static int metaTypeId();

    QString title() const;

public slots:
    void setTitle(QString title);

signals:
    void titleChanged(QString title);

private:
    Ui::TestDialog* ui;
    QString m_title;
};
}
}

Q_DECLARE_METATYPE(mu::framework::TestDialog)

#endif // MU_FRAMEWORK_TESTDIALOG_H

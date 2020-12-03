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
#include "testdialog.h"
#include "ui_testdialog.h"

using namespace mu::framework;

TestDialog::TestDialog(const TestDialog& dialog)
    : QDialog(dialog.parentWidget()),
    ui(dialog.ui)
{
}

TestDialog::TestDialog(QWidget* parent)
    : QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);
}

TestDialog::~TestDialog()
{
    delete ui;
}

int TestDialog::metaTypeId()
{
    return QMetaType::type("TestDialog");
}

QString TestDialog::title() const
{
    return m_title;
}

void TestDialog::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    ui->labelTestParam->setText(m_title);
}

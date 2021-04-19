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
#include "testdialog.h"
#include "ui_testdialog.h"

using namespace mu::ui;

TestDialog::TestDialog(const TestDialog& dialog)
    : WidgetDialog(dialog.parentWidget()),
    ui(dialog.ui)
{
}

TestDialog::TestDialog(QWidget* parent)
    : WidgetDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);
}

TestDialog::~TestDialog()
{
    delete ui;
}

QString TestDialog::title() const
{
    return m_title;
}

int TestDialog::static_metaTypeId()
{
    return qRegisterMetaType<TestDialog>("TestDialog");
}

int TestDialog::metaTypeId() const
{
    return static_metaTypeId();
}

void TestDialog::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    ui->labelTestParam->setText(m_title);
}

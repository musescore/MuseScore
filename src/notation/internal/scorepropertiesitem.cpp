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

#include "scorepropertiesitem.h"
#include <QLabel>

using namespace mu::notation;

/// <summary>
/// Create a new instance of a ScorePropertiesItem using a label and value widget from the ScoreProperty screen
/// </summary>
/// <param name="label">The label widget</param>
/// <param name="value">The value widget</param>
ScorePropertiesItem::ScorePropertiesItem(QWidget* label, QWidget* value)
{
	this->label = label;
	this->value = value;
}

/// <summary>
/// Get the string value of the label
/// </summary>
/// <returns>The name of property</returns>
QString ScorePropertiesItem::Label()
{
    // Label could be a line edit (for none standard properties) or a label widget (for standard properties)
    QLineEdit* lineEditItem = dynamic_cast<QLineEdit*>(this->label);
    if (lineEditItem)
    {
        return lineEditItem->text();
    }
    
    QLabel* labelItem = dynamic_cast<QLabel*>(this->label);
    if (labelItem)
    {
        return labelItem->text();
    }

    return "";
}

/// <summary>
/// Get the value of the property
/// </summary>
/// <returns>Value</returns>
QString ScorePropertiesItem::Value()
{
    QLineEdit* val = static_cast<QLineEdit*>(this->value);
    return val->text();
}

/// <summary>
/// Set focus to the widget
/// </summary>
void ScorePropertiesItem::SetFocus()
{
    this->label->setFocus();
}
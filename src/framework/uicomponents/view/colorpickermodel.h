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

#ifndef MUSE_UICOMPONENTS_COLORPICKER_H
#define MUSE_UICOMPONENTS_COLORPICKER_H

#include <QObject>

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"

namespace muse::uicomponents {
class ColorPickerModel : public QObject, public muse::Injectable, public async::Asyncable
{
    Q_OBJECT

    muse::Inject<IInteractive> interactive = { this };

public:
    explicit ColorPickerModel(QObject* parent = nullptr);

    Q_INVOKABLE void selectColor(const QColor& currentColor);

signals:
    void colorSelected(QColor color);
    void selectRejected();
};
}

#endif // MUSE_UICOMPONENTS_COLORPICKER_H

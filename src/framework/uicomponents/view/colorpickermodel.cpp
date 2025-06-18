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

#include "colorpickermodel.h"

#include "log.h"

using namespace muse::uicomponents;

ColorPickerModel::ColorPickerModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void ColorPickerModel::selectColor(const QColor& currentColor)
{
    auto promise = interactive()->selectColor(Color::fromQColor(currentColor));
    promise.onResolve(this, [this](const Color& c) {
        emit colorSelected(c.toQColor());
    }).onReject(this, [this](int code, const std::string& msg) {
        LOGD() << "select color rejected, err code: " << code << ", msg: " << msg;
        emit selectRejected();
    });
}

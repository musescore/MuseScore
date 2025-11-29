/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "graphicstestobject.h"

#include <QPainter>

#include "graphicsapiprovider.h"

#include "log.h"

using namespace muse::ui;

GraphicsTestObject::GraphicsTestObject()
{
    setWidth(1);
    setHeight(1);

    GraphicsApiProvider::testObjectIsAlive = true;
}

GraphicsTestObject::~GraphicsTestObject()
{
    GraphicsApiProvider::testObjectIsAlive = false;
}

void GraphicsTestObject::paint(QPainter*)
{
    LOGD() << "painted, graphics api: " << GraphicsApiProvider::graphicsApiName();

    // just for test
    // {
    //     if (GraphicsApiProvider::graphicsApi() == GraphicsApiProvider::Direct3D11) {
    //         LOGDA() << "Failed to build graphics pipeline state";
    //     }

    //     if (GraphicsApiProvider::graphicsApi() == GraphicsApiProvider::OpenGL) {
    //         LOGDA() << "Failed to build graphics pipeline state";
    //     }
    // }

    GraphicsApiProvider::testObjectHasPainted = true;
}

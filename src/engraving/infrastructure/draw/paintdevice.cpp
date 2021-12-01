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
#include "paintdevice.h"

#ifndef NO_QT_SUPPORT
#include <QPaintDevice>
#include <QPagedPaintDevice>
#endif

#include "log.h"

using namespace mu::draw;
#ifndef NO_QT_SUPPORT

PaintDevice::PaintDevice(QPaintDevice* d)
    : m_d(d)
{
}

#endif

int PaintDevice::dpi() const
{
    if (m_d) {
        return m_d->logicalDpiX();
    }
    NOT_IMPLEMENTED;
    return -1;
}

// =====================================================
// PagedPaintDevice
// =====================================================

#ifndef NO_QT_SUPPORT
PagedPaintDevice::PagedPaintDevice(QPagedPaintDevice* d)
    : PaintDevice(d), m_d(d)
{
}

#endif
void PagedPaintDevice::newPage()
{
    if (m_d) {
        m_d->newPage();
        return;
    }
    NOT_IMPLEMENTED;
}

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
#ifndef MU_DRAW_PAGEDPAINTDEVICE_H
#define MU_DRAW_PAGEDPAINTDEVICE_H

class QPaintDevice;
class QPagedPaintDevice;
namespace mu::draw {
class PaintDevice
{
public:
    PaintDevice() = default;

#ifndef NO_QT_SUPPORT
    PaintDevice(QPaintDevice* d);
#endif

    int dpi() const;

private:
    QPaintDevice* m_d = nullptr;
};

class PagedPaintDevice : public PaintDevice
{
public:
    PagedPaintDevice() = default;

#ifndef NO_QT_SUPPORT
    PagedPaintDevice(QPagedPaintDevice* d);
#endif

    void newPage();

private:
    QPagedPaintDevice* m_d = nullptr;
};
}

#endif // MU_DRAW_PAGEDPAINTDEVICE_H

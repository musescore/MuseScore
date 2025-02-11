/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef SVGGENERATOR_H
#define SVGGENERATOR_H

#include <QtGui/qpaintdevice.h>

#include <QtCore/qnamespace.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>

namespace mu::engraving {
class EngravingItem;
}

class SvgGeneratorPrivate;

//---------------------------------------------------------
//   @@ SvgGenerator
//   @P size          QSize
//   @P viewBox       QRectF
//   @P title         QString
//   @P description   QString
//   @P fileName      QString
//   @P outputDevice  QIODevice
//   @P resolution    int
//---------------------------------------------------------

class SvgGenerator : public QPaintDevice
{
    Q_DECLARE_PRIVATE(SvgGenerator)

    Q_PROPERTY(QSize size READ size WRITE setSize)
    Q_PROPERTY(QRectF viewBox READ viewBoxF WRITE setViewBox)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(QIODevice * outputDevice READ outputDevice WRITE setOutputDevice)
    Q_PROPERTY(int resolution READ resolution WRITE setResolution)
public:
    SvgGenerator();
    ~SvgGenerator();

    QString title() const;
    void setTitle(const QString& title);

    QString description() const;
    void setDescription(const QString& description);

    QSize size() const;
    void setSize(const QSize& size);

    QRect viewBox() const;
    QRectF viewBoxF() const;
    void setViewBox(const QRect& viewBox);
    void setViewBox(const QRectF& viewBox);

    QString fileName() const;
    void setFileName(const QString& fileName);

    QIODevice* outputDevice() const;
    void setOutputDevice(QIODevice* outputDevice);

    void setResolution(int dpi);
    int resolution() const;

    void setElement(const mu::engraving::EngravingItem* e);

    void setReplaceClipPathWithMask(bool v);

protected:
    QPaintEngine* paintEngine() const;
    int metric(QPaintDevice::PaintDeviceMetric metric) const;

private:
    QScopedPointer<SvgGeneratorPrivate> d_ptr;
};

#endif // QSVGGENERATOR_H

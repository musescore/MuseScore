//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "canvaspreferencesmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::appshell;
using namespace mu::notation;

CanvasPreferencesModel::CanvasPreferencesModel(QObject* parent)
    : QObject(parent)
{
}

void CanvasPreferencesModel::load()
{
    setupConnections();
}

QVariantList CanvasPreferencesModel::zoomTypes() const
{
    QVariantList types = {
        QVariantMap { { "title", qtrc("appshell", "Percentage") }, { "value", static_cast<int>(ZoomType::Percentage) } },
        QVariantMap { { "title", qtrc("appshell", "Page Width") }, { "value", static_cast<int>(ZoomType::PageWidth) } },
        QVariantMap { { "title", qtrc("appshell", "Whole Page") }, { "value", static_cast<int>(ZoomType::WholePage) } },
        QVariantMap { { "title", qtrc("appshell", "Two Pages") }, { "value", static_cast<int>(ZoomType::TwoPages) } }
    };

    return types;
}

QVariantMap CanvasPreferencesModel::defaultZoom() const
{
    QVariantMap zoom;
    ZoomType zoomType = defaultZoomType();
    zoom["type"] = static_cast<int>(zoomType);
    zoom["isPercentage"] = zoomType == ZoomType::Percentage;
    zoom["level"] = notationConfiguration()->defaultZoom();

    return zoom;
}

int CanvasPreferencesModel::mouseZoomPrecision() const
{
    return notationConfiguration()->mouseZoomPrecision();
}

int CanvasPreferencesModel::scrollPagesOrientation() const
{
    return static_cast<int>(notationConfiguration()->canvasOrientation().val);
}

bool CanvasPreferencesModel::limitScrollArea() const
{
    return notationConfiguration()->isLimitCanvasScrollArea();
}

void CanvasPreferencesModel::setDefaultZoomType(int zoomType)
{
    ZoomType type = static_cast<ZoomType>(zoomType);
    if (defaultZoomType() == type) {
        return;
    }

    notationConfiguration()->setDefaultZoomType(type);
    emit defaultZoomChanged();
}

void CanvasPreferencesModel::setDefaultZoomLevel(int zoom)
{
    if (defaultZoomLevel() == zoom) {
        return;
    }

    notationConfiguration()->setDefaultZoom(zoom);
    emit defaultZoomChanged();
}

void CanvasPreferencesModel::setMouseZoomPrecision(int precision)
{
    if (mouseZoomPrecision() == precision) {
        return;
    }

    notationConfiguration()->setMouseZoomPrecision(precision);
    emit mouseZoomPrecisionChanged();
}

void CanvasPreferencesModel::setScrollPagesOrientation(int orientation)
{
    if (orientation == scrollPagesOrientation()) {
        return;
    }

    notationConfiguration()->setCanvasOrientation(static_cast<framework::Orientation>(orientation));
}

void CanvasPreferencesModel::setLimitScrollArea(bool limit)
{
    if (limitScrollArea() == limit) {
        return;
    }

    notationConfiguration()->setIsLimitCanvasScrollArea(limit);
    emit limitScrollAreaChanged();
}

void CanvasPreferencesModel::setupConnections()
{
    notationConfiguration()->canvasOrientation().ch.onReceive(this, [this](framework::Orientation) {
        emit scrollPagesOrientationChanged();
    });
}

ZoomType CanvasPreferencesModel::defaultZoomType() const
{
    return notationConfiguration()->defaultZoomType();
}

int CanvasPreferencesModel::defaultZoomLevel() const
{
    return notationConfiguration()->defaultZoom();
}

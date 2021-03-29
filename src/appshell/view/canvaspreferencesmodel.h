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
#ifndef MU_APPSHELL_CANVASPREFERENCESMODEL_H
#define MU_APPSHELL_CANVASPREFERENCESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "notation/inotationconfiguration.h"

namespace mu::appshell {
class CanvasPreferencesModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, notation::INotationConfiguration, notationConfiguration)

    Q_PROPERTY(QVariantMap defaultZoom READ defaultZoom NOTIFY defaultZoomChanged)
    Q_PROPERTY(int mouseZoomPrecision READ mouseZoomPrecision WRITE setMouseZoomPrecision NOTIFY mouseZoomPrecisionChanged)

    Q_PROPERTY(int scrollPagesOrientation READ scrollPagesOrientation WRITE setScrollPagesOrientation NOTIFY scrollPagesOrientationChanged)
    Q_PROPERTY(bool limitScrollArea READ limitScrollArea WRITE setLimitScrollArea NOTIFY limitScrollAreaChanged)

public:
    explicit CanvasPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariantList zoomTypes() const;

    QVariantMap defaultZoom() const;
    Q_INVOKABLE void setDefaultZoomType(int zoomType);
    Q_INVOKABLE void setDefaultZoomLevel(int zoom);

    int mouseZoomPrecision() const;

    int scrollPagesOrientation() const;
    bool limitScrollArea() const;

public slots:
    void setMouseZoomPrecision(int precision);

    void setScrollPagesOrientation(int orientation);
    void setLimitScrollArea(bool limit);

signals:
    void defaultZoomChanged();
    void mouseZoomPrecisionChanged();
    void scrollPagesOrientationChanged();
    void limitScrollAreaChanged();

private:
    void setupConnections();

    notation::ZoomType defaultZoomType() const;
    int defaultZoomLevel() const;
};
}

#endif // MU_APPSHELL_CANVASPREFERENCESMODEL_H

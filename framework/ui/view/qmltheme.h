//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef MU_FRAMEWORK_QMLTHEME_H
#define MU_FRAMEWORK_QMLTHEME_H

#include <QObject>
#include <QPalette>
#include <QColor>
#include <QFont>

namespace mu {
namespace framework {
class QmlTheme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor popupBackgroundColor READ popupBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY themeChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor NOTIFY themeChanged)

    Q_PROPERTY(QColor buttonColorNormal READ buttonColorNormal NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonColorHover READ buttonColorHover NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonColorHit READ buttonColorHit NOTIFY themeChanged)

    Q_PROPERTY(QColor fontColor READ fontColor NOTIFY themeChanged)

    Q_PROPERTY(QFont font READ font CONSTANT)
public:
    enum StyleKeys {
        BACKGROUND_COLOR = 0,
        POPUP_BACKGROUND_COLOR,
        HIGHLIGHT_COLOR,
        STROKE_COLOR,
        BUTTON_COLOR_NORMAL,
        BUTTON_COLOR_HOVER,
        BUTTON_COLOR_HIT,
        FONT_COLOR
    };

    QmlTheme(QObject* parent = nullptr);

    void update();

    QColor backgroundColor() const;
    QColor popupBackgroundColor() const;
    QColor highlightColor() const;
    QColor strokeColor() const;
    QColor buttonColorNormal() const;
    QColor buttonColorHover() const;
    QColor buttonColorHit() const;
    QColor fontColor() const;
    QFont font() const;

signals:
    void themeChanged();

private:
    QFont m_font;
};
}
}

#endif // MU_FRAMEWORK_QMLTHEME_H

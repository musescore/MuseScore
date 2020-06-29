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
    Q_PROPERTY(QColor textFieldColor READ textFieldColor NOTIFY themeChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonColor READ buttonColor NOTIFY themeChanged)
    Q_PROPERTY(QColor fontColor READ fontColor NOTIFY themeChanged)

    Q_PROPERTY(qreal accentOpacityNormal READ accentOpacityNormal NOTIFY themeChanged)
    Q_PROPERTY(qreal accentOpacityHit READ accentOpacityHit NOTIFY themeChanged)
    Q_PROPERTY(qreal accentOpacityHover READ accentOpacityHover NOTIFY themeChanged)

    Q_PROPERTY(qreal buttonOpacityNormal READ buttonOpacityNormal NOTIFY themeChanged)
    Q_PROPERTY(qreal buttonOpacityHover READ buttonOpacityHover NOTIFY themeChanged)
    Q_PROPERTY(qreal buttonOpacityHit READ buttonOpacityHit NOTIFY themeChanged)

    Q_PROPERTY(QFont font READ font CONSTANT)
public:
    enum StyleKeys {
        BACKGROUND_COLOR = 0,
        POPUP_BACKGROUND_COLOR,
        TEXT_FIELD_COLOR,
        ACCENT_COLOR,
        STROKE_COLOR,
        BUTTON_COLOR,
        FONT_COLOR,

        ACCENT_OPACITY_NORMAL,
        ACCENT_OPACITY_HOVER,
        ACCENT_OPACITY_HIT,

        BUTTON_OPACITY_NORMAL,
        BUTTON_OPACITY_HOVER,
        BUTTON_OPACITY_HIT
    };

    QmlTheme(QObject* parent = nullptr);

    void update();

    QColor backgroundColor() const;
    QColor popupBackgroundColor() const;
    QColor textFieldColor() const;
    QColor accentColor() const;
    QColor strokeColor() const;
    QColor buttonColor() const;
    QColor fontColor() const;
    QFont font() const;

    qreal accentOpacityNormal() const;
    qreal accentOpacityHover() const;
    qreal accentOpacityHit() const;

    qreal buttonOpacityNormal() const;
    qreal buttonOpacityHover() const;
    qreal buttonOpacityHit() const;

signals:
    void themeChanged();

private:
    QFont m_font;
};
}
}

#endif // MU_FRAMEWORK_QMLTHEME_H

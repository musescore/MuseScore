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

#include "qmltheme.h"

#include <QVariant>

using namespace mu::framework;

static const QHash<int, QVariant> DARK_THEME {
    { QmlTheme::BACKGROUND_PRIMARY_COLOR, "#2D2D30" },
    { QmlTheme::BACKGROUND_SECONDARY_COLOR, "#363638" },
    { QmlTheme::POPUP_BACKGROUND_COLOR, "#2C2C2C" },
    { QmlTheme::TEXT_FIELD_COLOR, "#242427" },
    { QmlTheme::ACCENT_COLOR, "#FF4848" },
    { QmlTheme::STROKE_COLOR, "#1E1E1E" },
    { QmlTheme::BUTTON_COLOR, "#595959" },
    { QmlTheme::FONT_PRIMARY_COLOR, "#EBEBEB" },
    { QmlTheme::FONT_SECONDARY_COLOR, "#BABABA" },

    { QmlTheme::ACCENT_OPACITY_NORMAL, 0.8 },
    { QmlTheme::ACCENT_OPACITY_HOVER, 1.0 },
    { QmlTheme::ACCENT_OPACITY_HIT, 0.5 },

    { QmlTheme::BUTTON_OPACITY_NORMAL, 0.8 },
    { QmlTheme::BUTTON_OPACITY_HOVER, 1.0 },
    { QmlTheme::BUTTON_OPACITY_HIT, 0.5 },
};

static const QHash<int, QVariant> LIGHT_THEME {
    { QmlTheme::BACKGROUND_PRIMARY_COLOR, "#F5F5F6" },
    { QmlTheme::BACKGROUND_SECONDARY_COLOR, "#E6E9ED" },
    { QmlTheme::POPUP_BACKGROUND_COLOR, "#F5F5F6" },
    { QmlTheme::TEXT_FIELD_COLOR, "#FFFFFF" },
    { QmlTheme::ACCENT_COLOR, "#58AFFF" },
    { QmlTheme::STROKE_COLOR, "#E4E4E4" },
    { QmlTheme::BUTTON_COLOR, "#CFD5DD" },
    { QmlTheme::FONT_PRIMARY_COLOR, "#111132" },
    { QmlTheme::FONT_SECONDARY_COLOR, "#000000" },

    { QmlTheme::ACCENT_OPACITY_NORMAL, 0.3 },
    { QmlTheme::ACCENT_OPACITY_HOVER, 0.15 },
    { QmlTheme::ACCENT_OPACITY_HIT, 0.5 },

    { QmlTheme::BUTTON_OPACITY_NORMAL, 0.7 },
    { QmlTheme::BUTTON_OPACITY_HOVER, 0.5 },
    { QmlTheme::BUTTON_OPACITY_HIT, 1.0 },
};

QmlTheme::QmlTheme(QObject* parent)
    : QObject(parent)
{
    configuration()->themeTypeChanged().onReceive(this, [this](const IUiConfiguration::ThemeType) {
        update();
    });

    initFont();
    initMusicalFont();
}

void QmlTheme::update()
{
    emit themeChanged();
}

QColor QmlTheme::backgroundPrimaryColor() const
{
    return currentThemeProperites().value(BACKGROUND_PRIMARY_COLOR).toString();
}

QColor QmlTheme::backgroundSecondaryColor() const
{
    return currentThemeProperites().value(BACKGROUND_SECONDARY_COLOR).toString();
}

QColor QmlTheme::popupBackgroundColor() const
{
    return currentThemeProperites().value(POPUP_BACKGROUND_COLOR).toString();
}

QColor QmlTheme::textFieldColor() const
{
    return currentThemeProperites().value(TEXT_FIELD_COLOR).toString();
}

QColor QmlTheme::accentColor() const
{
    return currentThemeProperites().value(ACCENT_COLOR).toString();
}

QColor QmlTheme::strokeColor() const
{
    return currentThemeProperites().value(STROKE_COLOR).toString();
}

QColor QmlTheme::buttonColor() const
{
    return currentThemeProperites().value(BUTTON_COLOR).toString();
}

QColor QmlTheme::fontPrimaryColor() const
{
    return currentThemeProperites().value(FONT_PRIMARY_COLOR).toString();
}

QColor QmlTheme::fontSecondaryColor() const
{
    return currentThemeProperites().value(FONT_SECONDARY_COLOR).toString();
}

QFont QmlTheme::font() const
{
    return m_font;
}

QFont QmlTheme::musicalFont() const
{
    return m_musicalFont;
}

qreal QmlTheme::accentOpacityNormal() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_NORMAL).toReal();
}

qreal QmlTheme::accentOpacityHover() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_HOVER).toReal();
}

qreal QmlTheme::accentOpacityHit() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_HIT).toReal();
}

qreal QmlTheme::buttonOpacityNormal() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_NORMAL).toReal();
}

qreal QmlTheme::buttonOpacityHover() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_HOVER).toReal();
}

qreal QmlTheme::buttonOpacityHit() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_HIT).toReal();
}

QHash<int, QVariant> QmlTheme::currentThemeProperites() const
{
    if (configuration()->themeType() == IUiConfiguration::ThemeType::DARK_THEME) {
        return DARK_THEME;
    }

    return LIGHT_THEME;
}

void QmlTheme::initFont()
{
    m_font.setFamily(configuration()->fontFamily());
    m_font.setPointSize(configuration()->fontSize());

    configuration()->fontFamilyChanged().onReceive(this, [this](const QString& fontFamily) {
        m_font.setFamily(fontFamily);

        update();
    });

    configuration()->fontSizeChanged().onReceive(this, [this](const int fontSize) {
        m_font.setPointSize(fontSize);

        update();
    });
}

void QmlTheme::initMusicalFont()
{
    m_musicalFont.setFamily(configuration()->musicalFontFamily());
    m_musicalFont.setPointSize(configuration()->musicalFontSize());

    configuration()->musicalFontFamilyChanged().onReceive(this, [this](const QString& fontFamily) {
        m_musicalFont.setFamily(fontFamily);

        update();
    });

    configuration()->musicalFontSizeChanged().onReceive(this, [this](const int fontSize) {
        m_musicalFont.setPointSize(fontSize);

        update();
    });
}

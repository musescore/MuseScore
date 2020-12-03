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

#include "theme.h"

#include <QVariant>

using namespace mu::framework;

enum StyleKeys {
    BACKGROUND_PRIMARY_COLOR = 0,
    BACKGROUND_SECONDARY_COLOR,
    POPUP_BACKGROUND_COLOR,
    TEXT_FIELD_COLOR,
    ACCENT_COLOR,
    STROKE_COLOR,
    BUTTON_COLOR,
    FONT_PRIMARY_COLOR,
    FONT_SECONDARY_COLOR,

    ACCENT_OPACITY_NORMAL,
    ACCENT_OPACITY_HOVER,
    ACCENT_OPACITY_HIT,

    BUTTON_OPACITY_NORMAL,
    BUTTON_OPACITY_HOVER,
    BUTTON_OPACITY_HIT,

    ITEM_OPACITY_DISABLED
};

static const QHash<int, QVariant> DARK_THEME {
    { BACKGROUND_PRIMARY_COLOR, "#2D2D30" },
    { BACKGROUND_SECONDARY_COLOR, "#363638" },
    { POPUP_BACKGROUND_COLOR, "#323236" },
    { TEXT_FIELD_COLOR, "#242427" },
    { ACCENT_COLOR, "#FF4848" },
    { STROKE_COLOR, "#1E1E1E" },
    { BUTTON_COLOR, "#595959" },
    { FONT_PRIMARY_COLOR, "#EBEBEB" },
    { FONT_SECONDARY_COLOR, "#BDBDBD" },

    { ACCENT_OPACITY_NORMAL, 0.8 },
    { ACCENT_OPACITY_HOVER, 1.0 },
    { ACCENT_OPACITY_HIT, 0.5 },

    { BUTTON_OPACITY_NORMAL, 0.8 },
    { BUTTON_OPACITY_HOVER, 1.0 },
    { BUTTON_OPACITY_HIT, 0.5 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

static const QHash<int, QVariant> LIGHT_THEME {
    { BACKGROUND_PRIMARY_COLOR, "#F5F5F6" },
    { BACKGROUND_SECONDARY_COLOR, "#E6E9ED" },
    { POPUP_BACKGROUND_COLOR, "#F5F5F6" },
    { TEXT_FIELD_COLOR, "#FFFFFF" },
    { ACCENT_COLOR, "#70AFEA" },
    { STROKE_COLOR, "#CED1D4" },
    { BUTTON_COLOR, "#CFD5DD" },
    { FONT_PRIMARY_COLOR, "#111132" },
    { FONT_SECONDARY_COLOR, "#FFFFFF" },

    { ACCENT_OPACITY_NORMAL, 0.3 },
    { ACCENT_OPACITY_HOVER, 0.15 },
    { ACCENT_OPACITY_HIT, 0.5 },

    { BUTTON_OPACITY_NORMAL, 0.7 },
    { BUTTON_OPACITY_HOVER, 0.5 },
    { BUTTON_OPACITY_HIT, 1.0 },

    { ITEM_OPACITY_DISABLED, 0.3 }
};

Theme::Theme(QObject* parent)
    : QObject(parent)
{
}

void Theme::init()
{
    configuration()->themeTypeChanged().onReceive(this, [this](const IUiConfiguration::ThemeType) {
        update();
    });

    initFont();
    initMusicalFont();

    setupWidgetTheme();
}

void Theme::update()
{
    setupWidgetTheme();

    emit themeChanged();
}

QColor Theme::backgroundPrimaryColor() const
{
    return currentThemeProperites().value(BACKGROUND_PRIMARY_COLOR).toString();
}

QColor Theme::backgroundSecondaryColor() const
{
    return currentThemeProperites().value(BACKGROUND_SECONDARY_COLOR).toString();
}

QColor Theme::popupBackgroundColor() const
{
    return currentThemeProperites().value(POPUP_BACKGROUND_COLOR).toString();
}

QColor Theme::textFieldColor() const
{
    return currentThemeProperites().value(TEXT_FIELD_COLOR).toString();
}

QColor Theme::accentColor() const
{
    return currentThemeProperites().value(ACCENT_COLOR).toString();
}

QColor Theme::strokeColor() const
{
    return currentThemeProperites().value(STROKE_COLOR).toString();
}

QColor Theme::buttonColor() const
{
    return currentThemeProperites().value(BUTTON_COLOR).toString();
}

QColor Theme::fontPrimaryColor() const
{
    return currentThemeProperites().value(FONT_PRIMARY_COLOR).toString();
}

QColor Theme::fontSecondaryColor() const
{
    return currentThemeProperites().value(FONT_SECONDARY_COLOR).toString();
}

QFont Theme::font() const
{
    return m_font;
}

QFont Theme::musicalFont() const
{
    return m_musicalFont;
}

qreal Theme::accentOpacityNormal() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_NORMAL).toReal();
}

qreal Theme::accentOpacityHover() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_HOVER).toReal();
}

qreal Theme::accentOpacityHit() const
{
    return currentThemeProperites().value(ACCENT_OPACITY_HIT).toReal();
}

qreal Theme::buttonOpacityNormal() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_NORMAL).toReal();
}

qreal Theme::buttonOpacityHover() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_HOVER).toReal();
}

qreal Theme::buttonOpacityHit() const
{
    return currentThemeProperites().value(BUTTON_OPACITY_HIT).toReal();
}

qreal Theme::itemOpacityDisabled() const
{
    return currentThemeProperites().value(ITEM_OPACITY_DISABLED).toReal();
}

QHash<int, QVariant> Theme::currentThemeProperites() const
{
    if (configuration()->themeType() == IUiConfiguration::ThemeType::DARK_THEME) {
        return DARK_THEME;
    }

    return LIGHT_THEME;
}

void Theme::initFont()
{
    m_font.setFamily(configuration()->fontFamily());
    m_font.setPixelSize(configuration()->fontSize());

    configuration()->fontFamilyChanged().onReceive(this, [this](const QString& fontFamily) {
        m_font.setFamily(fontFamily);

        update();
    });

    configuration()->fontSizeChanged().onReceive(this, [this](const int fontSize) {
        m_font.setPixelSize(fontSize);

        update();
    });
}

void Theme::initMusicalFont()
{
    m_musicalFont.setFamily(configuration()->musicalFontFamily());
    m_musicalFont.setPixelSize(configuration()->musicalFontSize());

    configuration()->musicalFontFamilyChanged().onReceive(this, [this](const QString& fontFamily) {
        m_musicalFont.setFamily(fontFamily);

        update();
    });

    configuration()->musicalFontSizeChanged().onReceive(this, [this](const int fontSize) {
        m_musicalFont.setPixelSize(fontSize);

        update();
    });
}

void Theme::setupWidgetTheme()
{
    QPalette palette(QApplication::palette());
    palette.setColor(QPalette::Window, backgroundPrimaryColor());
    palette.setColor(QPalette::WindowText, fontPrimaryColor());
    palette.setColor(QPalette::Base, backgroundSecondaryColor());
    palette.setColor(QPalette::AlternateBase, backgroundSecondaryColor());
    palette.setColor(QPalette::Text, fontPrimaryColor());
    palette.setColor(QPalette::Button, backgroundSecondaryColor());
    palette.setColor(QPalette::ButtonText, fontPrimaryColor());
    palette.setColor(QPalette::ToolTipBase, popupBackgroundColor());
    palette.setColor(QPalette::ToolTipText, fontPrimaryColor());
    palette.setColor(QPalette::Highlight, accentColor());
    palette.setColor(QPalette::HighlightedText, fontPrimaryColor());
    palette.setColor(QPalette::PlaceholderText, fontPrimaryColor());

    QApplication::setPalette(palette);
}

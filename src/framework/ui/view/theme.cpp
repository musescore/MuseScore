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

#include <QApplication>
#include <QPalette>
#include <QVariant>

using namespace mu::framework;

static const QString SEMIBOLD_STYLE_NAME("SemiBold");

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

using FontSizeType = IUiConfiguration::FontSizeType;

struct FontConfig
{
    QFont::Weight weight = QFont::Normal;
    FontSizeType sizeType = FontSizeType::BODY;
};

Theme::Theme(QObject* parent)
    : QObject(parent)
{
}

void Theme::init()
{
    configuration()->actualThemeTypeChanged().onReceive(this, [this](const IUiConfiguration::ThemeType) {
        update();
    });

    initUiFonts();
    initIconsFont();
    initMusicalFont();

    setupWidgetTheme();
}

void Theme::update()
{
    setupWidgetTheme();

    notifyAboutThemeChanged();
}

QColor Theme::backgroundPrimaryColor() const
{
    return currentThemeProperties().value(BACKGROUND_PRIMARY_COLOR).toString();
}

QColor Theme::backgroundSecondaryColor() const
{
    return currentThemeProperties().value(BACKGROUND_SECONDARY_COLOR).toString();
}

QColor Theme::popupBackgroundColor() const
{
    return currentThemeProperties().value(POPUP_BACKGROUND_COLOR).toString();
}

QColor Theme::textFieldColor() const
{
    return currentThemeProperties().value(TEXT_FIELD_COLOR).toString();
}

QColor Theme::accentColor() const
{
    return currentThemeProperties().value(ACCENT_COLOR).toString();
}

QColor Theme::strokeColor() const
{
    return currentThemeProperties().value(STROKE_COLOR).toString();
}

QColor Theme::buttonColor() const
{
    return currentThemeProperties().value(BUTTON_COLOR).toString();
}

QColor Theme::fontPrimaryColor() const
{
    return currentThemeProperties().value(FONT_PRIMARY_COLOR).toString();
}

QColor Theme::fontSecondaryColor() const
{
    return currentThemeProperties().value(FONT_SECONDARY_COLOR).toString();
}

QFont Theme::bodyFont() const
{
    return m_bodyFont;
}

QFont Theme::bodyBoldFont() const
{
    return m_bodyBoldFont;
}

QFont Theme::largeBodyFont() const
{
    return m_largeBodyFont;
}

QFont Theme::largeBodyBoldFont() const
{
    return m_largeBodyBoldFont;
}

QFont Theme::tabFont() const
{
    return m_tabFont;
}

QFont Theme::tabBoldFont() const
{
    return m_tabBoldFont;
}

QFont Theme::headerFont() const
{
    return m_headerFont;
}

QFont Theme::headerBoldFont() const
{
    return m_headerBoldFont;
}

QFont Theme::titleBoldFont() const
{
    return m_titleBoldFont;
}

QFont Theme::iconsFont() const
{
    return m_iconsFont;
}

QFont Theme::musicalFont() const
{
    return m_musicalFont;
}

qreal Theme::accentOpacityNormal() const
{
    return currentThemeProperties().value(ACCENT_OPACITY_NORMAL).toReal();
}

qreal Theme::accentOpacityHover() const
{
    return currentThemeProperties().value(ACCENT_OPACITY_HOVER).toReal();
}

qreal Theme::accentOpacityHit() const
{
    return currentThemeProperties().value(ACCENT_OPACITY_HIT).toReal();
}

qreal Theme::buttonOpacityNormal() const
{
    return currentThemeProperties().value(BUTTON_OPACITY_NORMAL).toReal();
}

qreal Theme::buttonOpacityHover() const
{
    return currentThemeProperties().value(BUTTON_OPACITY_HOVER).toReal();
}

qreal Theme::buttonOpacityHit() const
{
    return currentThemeProperties().value(BUTTON_OPACITY_HIT).toReal();
}

qreal Theme::itemOpacityDisabled() const
{
    return currentThemeProperties().value(ITEM_OPACITY_DISABLED).toReal();
}

mu::async::Notification Theme::themeChanged() const
{
    return m_themeChanged;
}

QHash<int, QVariant> Theme::currentThemeProperties() const
{
    if (configuration()->actualThemeType() == IUiConfiguration::ThemeType::DARK_THEME) {
        return DARK_THEME;
    }

    return LIGHT_THEME;
}

void Theme::initUiFonts()
{
    setupUiFonts();

    configuration()->fontChanged().onNotify(this, [this]() {
        setupUiFonts();
        update();
    });
}

void Theme::initIconsFont()
{
    setupIconsFont();

    configuration()->iconsFontChanged().onNotify(this, [this]() {
        setupIconsFont();
        update();
    });
}

void Theme::initMusicalFont()
{
    setupMusicFont();

    configuration()->musicalFontChanged().onNotify(this, [this]() {
        setupMusicFont();
        update();
    });
}

void Theme::setupUiFonts()
{
    QMap<QFont*, FontConfig> fonts {
        { &m_bodyFont, { QFont::Normal, FontSizeType::BODY } },
        { &m_bodyBoldFont, { QFont::DemiBold, FontSizeType::BODY } },
        { &m_largeBodyFont, { QFont::Normal, FontSizeType::BODY_LARGE } },
        { &m_largeBodyBoldFont, { QFont::DemiBold, FontSizeType::BODY_LARGE } },
        { &m_tabFont, { QFont::Normal, FontSizeType::TAB } },
        { &m_tabBoldFont, { QFont::DemiBold, FontSizeType::TAB } },
        { &m_headerFont, { QFont::Normal, FontSizeType::HEADER } },
        { &m_headerBoldFont, { QFont::DemiBold, FontSizeType::HEADER } },
        { &m_titleBoldFont, { QFont::DemiBold, FontSizeType::TITLE } },
    };

    for (QFont* font : fonts.keys()) {
        std::string family = configuration()->fontFamily();
        int size = configuration()->fontSize(fonts[font].sizeType);
        QFont::Weight weight = fonts[font].weight;

        font->setPixelSize(size);
        font->setFamily(QString::fromStdString(family));

        if (weight == QFont::DemiBold) {
            font->setStyleName(SEMIBOLD_STYLE_NAME);
        }
    }
}

void Theme::setupIconsFont()
{
    m_iconsFont.setFamily(QString::fromStdString(configuration()->iconsFontFamily()));
    m_iconsFont.setPixelSize(configuration()->iconsFontSize());
}

void Theme::setupMusicFont()
{
    m_musicalFont.setFamily(QString::fromStdString(configuration()->musicalFontFamily()));
    m_musicalFont.setPixelSize(configuration()->musicalFontSize());
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

    platformTheme()->setAppThemeDark(configuration()->actualThemeType() == IUiConfiguration::ThemeType::DARK_THEME);
}

void Theme::notifyAboutThemeChanged()
{
    m_themeChanged.notify();
    emit dataChanged();
}

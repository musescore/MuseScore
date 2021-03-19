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

#include "uitheme.h"

#include <QApplication>
#include <QPalette>
#include <QVariant>

using namespace mu::ui;

static const QString SEMIBOLD_STYLE_NAME("SemiBold");

struct FontConfig
{
    QFont::Weight weight = QFont::Normal;
    FontSizeType sizeType = FontSizeType::BODY;
};

UiTheme::UiTheme(QObject* parent)
    : QObject(parent)
{
}

void UiTheme::init()
{
    configuration()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });

    initUiFonts();
    initIconsFont();
    initMusicalFont();

    setupWidgetTheme();
}

void UiTheme::update()
{
    setupWidgetTheme();

    notifyAboutThemeChanged();
}

QColor UiTheme::backgroundPrimaryColor() const
{
    return colorByKey(BACKGROUND_PRIMARY_COLOR);
}

QColor UiTheme::backgroundSecondaryColor() const
{
    return colorByKey(BACKGROUND_SECONDARY_COLOR);
}

QColor UiTheme::popupBackgroundColor() const
{
    return colorByKey(POPUP_BACKGROUND_COLOR);
}

QColor UiTheme::textFieldColor() const
{
    return colorByKey(TEXT_FIELD_COLOR);
}

QColor UiTheme::accentColor() const
{
    return colorByKey(ACCENT_COLOR);
}

QColor UiTheme::strokeColor() const
{
    return colorByKey(STROKE_COLOR);
}

QColor UiTheme::buttonColor() const
{
    return colorByKey(BUTTON_COLOR);
}

QColor UiTheme::fontPrimaryColor() const
{
    return colorByKey(FONT_PRIMARY_COLOR);
}

QColor UiTheme::fontSecondaryColor() const
{
    return colorByKey(FONT_SECONDARY_COLOR);
}

QColor UiTheme::linkColor() const
{
    return colorByKey(LINK_COLOR);
}

QFont UiTheme::bodyFont() const
{
    return m_bodyFont;
}

QFont UiTheme::bodyBoldFont() const
{
    return m_bodyBoldFont;
}

QFont UiTheme::largeBodyFont() const
{
    return m_largeBodyFont;
}

QFont UiTheme::largeBodyBoldFont() const
{
    return m_largeBodyBoldFont;
}

QFont UiTheme::tabFont() const
{
    return m_tabFont;
}

QFont UiTheme::tabBoldFont() const
{
    return m_tabBoldFont;
}

QFont UiTheme::headerFont() const
{
    return m_headerFont;
}

QFont UiTheme::headerBoldFont() const
{
    return m_headerBoldFont;
}

QFont UiTheme::titleBoldFont() const
{
    return m_titleBoldFont;
}

QFont UiTheme::iconsFont() const
{
    return m_iconsFont;
}

QFont UiTheme::toolbarIconsFont() const
{
    return m_toolbarIconsFont;
}

QFont UiTheme::musicalFont() const
{
    return m_musicalFont;
}

qreal UiTheme::accentOpacityNormal() const
{
    return realByKey(ACCENT_OPACITY_NORMAL);
}

qreal UiTheme::accentOpacityHover() const
{
    return realByKey(ACCENT_OPACITY_HOVER);
}

qreal UiTheme::accentOpacityHit() const
{
    return realByKey(ACCENT_OPACITY_HIT);
}

qreal UiTheme::buttonOpacityNormal() const
{
    return realByKey(BUTTON_OPACITY_NORMAL);
}

qreal UiTheme::buttonOpacityHover() const
{
    return realByKey(BUTTON_OPACITY_HOVER);
}

qreal UiTheme::buttonOpacityHit() const
{
    return realByKey(BUTTON_OPACITY_HIT);
}

qreal UiTheme::itemOpacityDisabled() const
{
    return realByKey(ITEM_OPACITY_DISABLED);
}

QColor UiTheme::colorByKey(ThemeStyleKey key) const
{
    return currentTheme().values[key].toString();
}

qreal UiTheme::realByKey(ThemeStyleKey key) const
{
    return currentTheme().values[key].toDouble();
}

ThemeInfo UiTheme::currentTheme() const
{
    return configuration()->currentTheme();
}

void UiTheme::initUiFonts()
{
    setupUiFonts();

    configuration()->fontChanged().onNotify(this, [this]() {
        setupUiFonts();
        update();
    });
}

void UiTheme::initIconsFont()
{
    setupIconsFont();

    configuration()->iconsFontChanged().onNotify(this, [this]() {
        setupIconsFont();
        update();
    });
}

void UiTheme::initMusicalFont()
{
    setupMusicFont();

    configuration()->musicalFontChanged().onNotify(this, [this]() {
        setupMusicFont();
        update();
    });
}

void UiTheme::setupUiFonts()
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

void UiTheme::setupIconsFont()
{
    QString family = QString::fromStdString(configuration()->iconsFontFamily());

    m_iconsFont.setFamily(family);
    m_iconsFont.setPixelSize(configuration()->iconsFontSize(IconSizeType::Regular));

    m_toolbarIconsFont.setFamily(family);
    m_toolbarIconsFont.setPixelSize(configuration()->iconsFontSize(IconSizeType::Toolbar));
}

void UiTheme::setupMusicFont()
{
    m_musicalFont.setFamily(QString::fromStdString(configuration()->musicalFontFamily()));
    m_musicalFont.setPixelSize(configuration()->musicalFontSize());
}

void UiTheme::setupWidgetTheme()
{
    QColor fontPrimaryColorDisabled = fontPrimaryColor();
    fontPrimaryColorDisabled.setAlphaF(itemOpacityDisabled());

    QColor backgroundPrimaryColorDisabled = backgroundPrimaryColor();
    backgroundPrimaryColorDisabled.setAlphaF(itemOpacityDisabled());

    QColor backgroundSecondaryColorDisabled = backgroundSecondaryColor();
    backgroundSecondaryColorDisabled.setAlphaF(itemOpacityDisabled());

    QPalette palette(QApplication::palette());
    palette.setColor(QPalette::Window, backgroundPrimaryColor());
    palette.setColor(QPalette::Disabled, QPalette::Window, backgroundPrimaryColorDisabled);
    palette.setColor(QPalette::WindowText, fontPrimaryColor());
    palette.setColor(QPalette::Disabled, QPalette::WindowText, fontPrimaryColorDisabled);

    palette.setColor(QPalette::Base, backgroundSecondaryColor());
    palette.setColor(QPalette::Disabled, QPalette::Base, backgroundSecondaryColorDisabled);
    palette.setColor(QPalette::AlternateBase, backgroundSecondaryColor());
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, backgroundSecondaryColorDisabled);

    palette.setColor(QPalette::Text, fontPrimaryColor());
    palette.setColor(QPalette::Disabled, QPalette::Text, fontPrimaryColorDisabled);

    palette.setColor(QPalette::Button, backgroundSecondaryColor());
    palette.setColor(QPalette::Disabled, QPalette::Button, backgroundSecondaryColorDisabled);
    palette.setColor(QPalette::ButtonText, fontPrimaryColor());
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, fontPrimaryColorDisabled);

    palette.setColor(QPalette::ToolTipBase, popupBackgroundColor());
    palette.setColor(QPalette::ToolTipText, fontPrimaryColor());

    palette.setColor(QPalette::Highlight, accentColor());
    palette.setColor(QPalette::HighlightedText, fontPrimaryColor());

    palette.setColor(QPalette::PlaceholderText, fontPrimaryColor());
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, fontPrimaryColorDisabled);

    QApplication::setPalette(palette);

    QFont widgetsFont = bodyFont();
    widgetsFont.setPointSize(configuration()->fontSize(FontSizeType::BODY));
    QApplication::setFont(widgetsFont);
}

void UiTheme::notifyAboutThemeChanged()
{
    emit themeChanged();
}

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

using namespace mu::ui;

static const QString SEMIBOLD_STYLE_NAME("SemiBold");

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
    m_currentTheme = configuration()->currentTheme();

    configuration()->currentThemeChanged().onNotify(this, [this]() {
        m_currentTheme = configuration()->currentTheme();

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
    return colorByKey(BACKGROUND_PRIMARY_COLOR);
}

QColor Theme::backgroundSecondaryColor() const
{
    return colorByKey(BACKGROUND_SECONDARY_COLOR);
}

QColor Theme::popupBackgroundColor() const
{
    return colorByKey(POPUP_BACKGROUND_COLOR);
}

QColor Theme::textFieldColor() const
{
    return colorByKey(TEXT_FIELD_COLOR);
}

QColor Theme::accentColor() const
{
    return colorByKey(ACCENT_COLOR);
}

QColor Theme::strokeColor() const
{
    return colorByKey(STROKE_COLOR);
}

QColor Theme::buttonColor() const
{
    return colorByKey(BUTTON_COLOR);
}

QColor Theme::fontPrimaryColor() const
{
    return colorByKey(FONT_PRIMARY_COLOR);
}

QColor Theme::fontSecondaryColor() const
{
    return colorByKey(FONT_SECONDARY_COLOR);
}

QColor Theme::linkColor() const
{
    return colorByKey(LINK_COLOR);
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

QFont Theme::toolbarIconsFont() const
{
    return m_toolbarIconsFont;
}

QFont Theme::musicalFont() const
{
    return m_musicalFont;
}

qreal Theme::accentOpacityNormal() const
{
    return realByKey(ACCENT_OPACITY_NORMAL);
}

qreal Theme::accentOpacityHover() const
{
    return realByKey(ACCENT_OPACITY_HOVER);
}

qreal Theme::accentOpacityHit() const
{
    return realByKey(ACCENT_OPACITY_HIT);
}

qreal Theme::buttonOpacityNormal() const
{
    return realByKey(BUTTON_OPACITY_NORMAL);
}

qreal Theme::buttonOpacityHover() const
{
    return realByKey(BUTTON_OPACITY_HOVER);
}

qreal Theme::buttonOpacityHit() const
{
    return realByKey(BUTTON_OPACITY_HIT);
}

qreal Theme::itemOpacityDisabled() const
{
    return realByKey(ITEM_OPACITY_DISABLED);
}

mu::async::Notification Theme::themeChanged() const
{
    return m_themeChanged;
}

QColor Theme::colorByKey(ThemeStyleKey key) const
{
    return m_currentTheme.values[key].toString();
}

qreal Theme::realByKey(ThemeStyleKey key) const
{
    return m_currentTheme.values[key].toDouble();
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
    QString family = QString::fromStdString(configuration()->iconsFontFamily());

    m_iconsFont.setFamily(family);
    m_iconsFont.setPixelSize(configuration()->iconsFontSize(IUiConfiguration::IconSizeType::Regular));

    m_toolbarIconsFont.setFamily(family);
    m_toolbarIconsFont.setPixelSize(configuration()->iconsFontSize(IUiConfiguration::IconSizeType::Toolbar));
}

void Theme::setupMusicFont()
{
    m_musicalFont.setFamily(QString::fromStdString(configuration()->musicalFontFamily()));
    m_musicalFont.setPixelSize(configuration()->musicalFontSize());
}

void Theme::setupWidgetTheme()
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

    platformTheme()->setAppThemeDark(m_currentTheme.type == ThemeType::DARK_THEME);
}

void Theme::notifyAboutThemeChanged()
{
    m_themeChanged.notify();
    emit dataChanged();
}

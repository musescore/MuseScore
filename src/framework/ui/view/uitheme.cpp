/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "uitheme.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QGroupBox>
#include <QMenu>
#include <QPalette>
#include <QStyleFactory>
#include <QStyleOption>
#include <QToolBar>
#include <QVariant>

#include "log.h"

using namespace mu::ui;

static const QString SEMIBOLD_STYLE_NAME("SemiBold");

static const QPen NO_BORDER(Qt::transparent, 0);
static const QBrush NO_FILL(Qt::transparent);

static const int GROUP_BOX_LABEL_SPACING = 2;

//! NOTE In QML, a border is drawn _inside_ a rectangle.
//!      In C++, a border would normally be drawn half inside the rectangle, half outside.
//!      In this function, we exactly replicate the behaviour from QML.
static void drawRoundedRect(QPainter* painter, const QRectF& rect, const qreal radius, const QBrush& brush = NO_FILL,
                            const QPen& pen = NO_BORDER)
{
    IF_ASSERT_FAILED(painter) {
        return;
    }

    const qreal bw = pen.width();

    if (bw <= 0 || pen.color().alpha() == 0) {
        if (brush == NO_FILL) {
            return;
        }

        painter->save();
        painter->setPen(NO_BORDER);
        painter->setBrush(brush);
        painter->drawRoundedRect(rect, radius, radius);
        painter->restore();
        return;
    }

    painter->save();
    painter->setPen(NO_BORDER);
    painter->setBrush(brush);
    painter->drawRoundedRect(rect.adjusted(bw, bw, -bw, -bw), radius - bw, radius - bw);

    const qreal corr = 0.5 * bw;
    painter->setPen(pen);
    painter->setBrush(NO_FILL);
    painter->drawRoundedRect(rect.adjusted(corr, corr, -corr, -corr), radius - corr, radius - corr);
    painter->restore();
}

struct FontConfig
{
    QFont::Weight weight = QFont::Normal;
    FontSizeType sizeType = FontSizeType::BODY;
};

UiTheme::UiTheme()
    : QProxyStyle(QStyleFactory::create("Fusion"))
{
    setObjectName("UiTheme");
}

void UiTheme::init()
{
    configuration()->currentThemeChanged().onNotify(this, [this]() {
        update();
    });

    initThemeValues();

    initUiFonts();
    initIconsFont();
    initMusicalFont();

    setupWidgetTheme();
}

void UiTheme::initThemeValues()
{
    QMap<ThemeStyleKey, QVariant> themeValues = configuration()->currentTheme().values;

    m_backgroundPrimaryColor = themeValues[BACKGROUND_PRIMARY_COLOR].toString();
    m_backgroundSecondaryColor = themeValues[BACKGROUND_SECONDARY_COLOR].toString();
    m_popupBackgroundColor = themeValues[POPUP_BACKGROUND_COLOR].toString();
    m_textFieldColor = themeValues[TEXT_FIELD_COLOR].toString();
    m_accentColor = themeValues[ACCENT_COLOR].toString();
    m_strokeColor = themeValues[STROKE_COLOR].toString();
    m_buttonColor = themeValues[BUTTON_COLOR].toString();
    m_fontPrimaryColor = themeValues[FONT_PRIMARY_COLOR].toString();
    m_fontSecondaryColor = themeValues[FONT_SECONDARY_COLOR].toString();
    m_linkColor = themeValues[LINK_COLOR].toString();
    m_focusColor = themeValues[FOCUS_COLOR].toString();

    m_accentOpacityNormal = themeValues[ACCENT_OPACITY_NORMAL].toReal();
    m_accentOpacityHover = themeValues[ACCENT_OPACITY_HOVER].toReal();
    m_accentOpacityHit = themeValues[ACCENT_OPACITY_HIT].toReal();
    m_buttonOpacityNormal = themeValues[BUTTON_OPACITY_NORMAL].toReal();
    m_buttonOpacityHover = themeValues[BUTTON_OPACITY_HOVER].toReal();
    m_buttonOpacityHit = themeValues[BUTTON_OPACITY_HIT].toReal();
    m_itemOpacityDisabled = themeValues[ITEM_OPACITY_DISABLED].toReal();
}

void UiTheme::update()
{
    initThemeValues();
    setupWidgetTheme();
    notifyAboutThemeChanged();
}

QColor UiTheme::backgroundPrimaryColor() const
{
    return m_backgroundPrimaryColor;
}

QColor UiTheme::backgroundSecondaryColor() const
{
    return m_backgroundSecondaryColor;
}

QColor UiTheme::popupBackgroundColor() const
{
    return m_popupBackgroundColor;
}

QColor UiTheme::textFieldColor() const
{
    return m_textFieldColor;
}

QColor UiTheme::accentColor() const
{
    return m_accentColor;
}

QColor UiTheme::strokeColor() const
{
    return m_strokeColor;
}

QColor UiTheme::buttonColor() const
{
    return m_buttonColor;
}

QColor UiTheme::fontPrimaryColor() const
{
    return m_fontPrimaryColor;
}

QColor UiTheme::fontSecondaryColor() const
{
    return m_fontSecondaryColor;
}

QColor UiTheme::linkColor() const
{
    return m_linkColor;
}

QColor UiTheme::focusColor() const
{
    return m_focusColor;
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
    return m_accentOpacityNormal;
}

qreal UiTheme::accentOpacityHover() const
{
    return m_accentOpacityHover;
}

qreal UiTheme::accentOpacityHit() const
{
    return m_accentOpacityHit;
}

qreal UiTheme::buttonOpacityNormal() const
{
    return m_buttonOpacityNormal;
}

qreal UiTheme::buttonOpacityHover() const
{
    return m_buttonOpacityHover;
}

qreal UiTheme::buttonOpacityHit() const
{
    return m_buttonOpacityHit;
}

qreal UiTheme::itemOpacityDisabled() const
{
    return m_itemOpacityDisabled;
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

    QColor linkColorDisabled = linkColor();
    linkColorDisabled.setAlphaF(itemOpacityDisabled());

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

    palette.setColor(QPalette::Link, linkColor());
    palette.setColor(QPalette::Disabled, QPalette::Link, linkColorDisabled);

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

    QApplication::setStyle(this);
    QApplication::setPalette(palette);

    QString styleSheet = QString("* { font: %1px \"%2\" } ")
                         .arg(QString::number(bodyFont().pixelSize()), bodyFont().family());
    qApp->setStyleSheet(styleSheet);
}

void UiTheme::notifyAboutThemeChanged()
{
    emit themeChanged();
}

// ====================================================
// QStyle
// ====================================================

void UiTheme::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);

    if (qobject_cast<QAbstractItemView*>(widget)
        || qobject_cast<QGroupBox*>(widget)) {
        // Make hovering work
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void UiTheme::unpolish(QWidget* widget)
{
    QProxyStyle::unpolish(widget);

    if (qobject_cast<QAbstractItemView*>(widget)
        || qobject_cast<QGroupBox*>(widget)) {
        widget->setMouseTracking(false);
        widget->setAttribute(Qt::WA_Hover, false);
    }
}

void UiTheme::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const bool enabled = option->state & State_Enabled;
    const bool hovered = option->state & State_MouseOver;
    const bool pressed = option->state & State_Sunken;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    //! NOTE This drawing code is based on the implementation in QML.
    switch (element) {
    // Buttons (and ComboBoxes)
    case QStyle::PE_PanelButtonCommand: {
        auto buttonOption = qstyleoption_cast<const QStyleOptionButton*>(option);
        const bool accentButton = buttonOption && buttonOption->features & QStyleOptionButton::DefaultButton;
        const bool flat = buttonOption && buttonOption->features & QStyleOptionButton::Flat;

        drawButtonBackground(painter, option->rect, enabled, hovered, pressed, accentButton, flat);
    } break;

    // Checkboxes
    case QStyle::PE_IndicatorCheckBox: {
        const bool indeterminate = option->state & State_NoChange;
        const bool checked = option->state & State_On;
        const bool inMenu = qobject_cast<const QMenu*>(widget);

        drawCheckboxIndicator(painter, option->rect, enabled, hovered, pressed, checked, indeterminate, inMenu);
    } break;

    // Radio buttons
    case QStyle::PE_IndicatorRadioButton: {
        const bool selected = option->state & State_On;

        drawRadioButtonIndicator(painter, option->rect, enabled, hovered, pressed, selected);
    } break;

    // Indicator icons
    case QStyle::PE_IndicatorSpinUp:
    case QStyle::PE_IndicatorSpinDown:
    case QStyle::PE_IndicatorSpinPlus:
    case QStyle::PE_IndicatorSpinMinus: {
        drawIndicatorIcon(painter, option->rect, enabled, element);
    } break;

    // ListView
    case QStyle::PE_PanelItemViewItem: {
        bool selected = option->state & State_Selected;

        drawListViewItemBackground(painter, option->rect, enabled, hovered, pressed, selected);
    } break;

    // Toolbar
    case QStyle::PE_PanelToolBar: {
        painter->fillRect(option->rect, backgroundPrimaryColor());
    } break;
    case QStyle::PE_IndicatorToolBarHandle: {
        drawToolbarGrip(painter, option->rect, option->state & State_Horizontal);
    } break;

    // GroupBox
    case QStyle::PE_FrameGroupBox: {
        drawRoundedRect(painter, option->rect, 3, QBrush("#03000000"), QPen(strokeColor(), 1));
    } break;

    // Menu
    case QStyle::PE_PanelMenu: {
        drawRoundedRect(painter, option->rect, 3, backgroundPrimaryColor());
    } break;
    case QStyle::PE_FrameMenu: {
        drawRoundedRect(painter, option->rect, 3, NO_FILL, QPen(strokeColor(), 1));
    } break;

    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }

    painter->restore();
}

QRect UiTheme::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option, QStyle::SubControl subControl,
                              const QWidget* widget) const
{
    //QRect commonStyleRect = QCommonStyle::subControlRect(control, option, subControl, widget);
    QRect proxyStyleRect = QProxyStyle::subControlRect(control, option, subControl, widget);

    switch (control) {
    case QStyle::CC_GroupBox:
        if (const QStyleOptionGroupBox* optionGroupBox = qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
            int indicatorWidth = 0;
            int indicatorHeight = 0;
            int indicatorSpacing = 0;
            const QSize textSize = option->fontMetrics.boundingRect(optionGroupBox->text).size() + QSize(2, 2);

            const bool checkable = option->subControls & QStyle::SC_GroupBoxCheckBox;

            if (checkable) {
                const bool isRadioButtonGroupBox
                    =widget && strcmp(widget->metaObject()->className(), "mu::uicomponents::RadioButtonGroupBox") == 0;

                indicatorWidth = pixelMetric(isRadioButtonGroupBox ? PM_ExclusiveIndicatorWidth : PM_IndicatorWidth, option, widget);
                indicatorHeight = pixelMetric(isRadioButtonGroupBox ? PM_ExclusiveIndicatorHeight : PM_IndicatorHeight, option, widget);
                indicatorSpacing
                    = pixelMetric(isRadioButtonGroupBox ? PM_RadioButtonLabelSpacing : PM_CheckBoxLabelSpacing, option, widget);
            }

            if (subControl == SC_GroupBoxFrame) {
                int topMargin = std::max(indicatorHeight, textSize.height()) + GROUP_BOX_LABEL_SPACING;
                return option->rect.adjusted(0, topMargin, 0, 0);
            }

            if (subControl == SC_GroupBoxContents) {
                int margin = 3;
                int topMargin = margin + std::max(indicatorHeight, textSize.height()) + GROUP_BOX_LABEL_SPACING;
                return option->rect.adjusted(margin, topMargin, -margin, -margin);
            }

            const int width = textSize.width() + (checkable ? indicatorWidth + indicatorSpacing : 0);
            QRect rect;

            if (option->rect.width() > width) {
                switch (optionGroupBox->textAlignment & Qt::AlignHorizontal_Mask) {
                case Qt::AlignHCenter:
                    rect.moveLeft((option->rect.width() - width) / 2);
                    break;
                case Qt::AlignRight:
                    rect.moveLeft(option->rect.width() - width);
                    break;
                }
            }

            if (subControl == SC_GroupBoxCheckBox) {
                rect.setWidth(indicatorWidth);
                rect.setHeight(indicatorHeight);
                rect.moveTop(textSize.height() > indicatorHeight ? (textSize.height() - indicatorHeight) / 2 : 0);
            } else if (subControl == SC_GroupBoxLabel) {
                rect.setSize(textSize);
                if (checkable) {
                    rect.moveTop(textSize.height() < indicatorHeight ? (indicatorHeight - textSize.height()) / 2 : 0);
                    rect.translate(indicatorWidth + indicatorSpacing, 0);
                }
            }

            return visualRect(option->direction, option->rect, rect);
        }
        break;
    default:
        break;
    }

    return proxyStyleRect;
}

int UiTheme::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    //! NOTE These metrics are based on the implementation in QML.
    switch (metric) {
    case PM_IndicatorWidth: // Checkbox
    case PM_IndicatorHeight:
        return 20;
    case PM_CheckBoxLabelSpacing:
        return 8;
    case PM_ExclusiveIndicatorWidth: // Radio button
    case PM_ExclusiveIndicatorHeight:
        return 20;
    case PM_RadioButtonLabelSpacing:
        return 6;
    case PM_ToolBarHandleExtent: // Toolbars
        return 32;
    default:
        break;
    }

    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize UiTheme::sizeFromContents(QStyle::ContentsType type, const QStyleOption* option, const QSize& contentsSize,
                                const QWidget* widget) const
{
    QSize commonStyleSize = QCommonStyle::sizeFromContents(type, option, contentsSize, widget);
    QSize proxyStyleSize = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);

    //! NOTE These calculations are based on the implementation in QML.
    switch (type) {
    case CT_PushButton:
        return QSize(std::max(contentsSize.width() + 32, 132),
                     contentsSize.height() + 14);
    case CT_ToolButton:
        return contentsSize.expandedTo(QSize(30, 30));
    case CT_ComboBox:
    case CT_LineEdit:
        return proxyStyleSize.expandedTo(QSize(30, 30));
    case CT_SpinBox:
        return QSize(proxyStyleSize.width(), 32); // results in the height begin 30
    case CT_GroupBox: {
        const QGroupBox* groupBox = qobject_cast<const QGroupBox*>(widget);
        const bool checkable = groupBox && groupBox->isCheckable();

        if (checkable) {
            const bool isRadioButtonGroupBox
                =widget && strcmp(widget->metaObject()->className(), "mu::uicomponents::RadioButtonGroupBox") == 0;

            int pm = pixelMetric(isRadioButtonGroupBox ? PM_ExclusiveIndicatorHeight : PM_IndicatorHeight, option, widget);
            return commonStyleSize + QSize(0, std::max(pm, option->fontMetrics.height()));
        }

        return commonStyleSize + QSize(0, option->fontMetrics.height());
    } break;
    case CT_ItemViewItem:
        return commonStyleSize.expandedTo(QSize(20, 20));
    case CT_MenuItem:
        if (const QStyleOptionMenuItem* optionMenuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            if (optionMenuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                return proxyStyleSize;
            }
        }
        return proxyStyleSize.expandedTo(QSize(30, 30));
    default:
        break;
    }

    return proxyStyleSize;
}

int UiTheme::styleHint(QStyle::StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_DitherDisabledText:
    case SH_EtchDisabledText:
        return false;
    case SH_ItemView_ScrollMode:
        return QAbstractItemView::ScrollPerPixel;
    default:
        break;
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

// ====================================================
// QStyle elements drawing
// ====================================================

void UiTheme::drawButtonBackground(QPainter* painter, const QRect& rect, bool enabled, bool hovered, bool pressed, bool accentButton,
                                   bool flat) const
{
    QColor backgroundColor(accentButton ? accentColor()
                           : flat ? Qt::transparent
                           : buttonColor());

    backgroundColor.setAlphaF(!enabled ? buttonOpacityNormal() * itemOpacityDisabled()
                              : pressed ? buttonOpacityHit()
                              : hovered ? buttonOpacityHover()
                              : !flat ? buttonOpacityNormal()
                              : 0);

    drawRoundedRect(painter, rect, 3, backgroundColor);
}

void UiTheme::drawCheckboxIndicator(QPainter* painter, const QRect& rect, bool enabled, bool hovered, bool pressed, bool checked,
                                    bool indeterminate, bool inMenu) const
{
    if (!inMenu) {
        QColor backgroundColor = buttonColor();
        backgroundColor.setAlphaF(!enabled ? buttonOpacityNormal() * itemOpacityDisabled()
                                  : pressed ? buttonOpacityHit()
                                  : hovered ? buttonOpacityHover()
                                  : buttonOpacityNormal());

        QColor borderColor = enabled && (hovered || pressed) ? strokeColor() : Qt::transparent;
        borderColor.setAlphaF(enabled && pressed ? buttonOpacityHit()
                              : enabled && hovered ? buttonOpacityHover()
                              : 0);

        const int borderWidth = enabled && (hovered || pressed) ? 1 : 0;
        drawRoundedRect(painter, rect, 2, backgroundColor, QPen(borderColor, borderWidth));
    }

    if (checked || indeterminate) {
        QColor tickColor = fontPrimaryColor();
        if (!enabled) {
            tickColor.setAlphaF(itemOpacityDisabled());
        }
        painter->setPen(tickColor);
        painter->setFont(m_iconsFont);
        painter->drawText(rect, Qt::AlignCenter,
                          iconCodeToChar(indeterminate ? IconCode::Code::MINUS : IconCode::Code::TICK_RIGHT_ANGLE));
    }
}

void UiTheme::drawRadioButtonIndicator(QPainter* painter, const QRect& rect, bool /*enabled*/, bool hovered, bool pressed,
                                       bool selected) const
{
    QColor borderColor = fontPrimaryColor();
    QColor backgroundColor = textFieldColor();

    if (pressed) {
        borderColor.setAlphaF(buttonOpacityHit());
    } else if (hovered) {
        borderColor.setAlphaF(buttonOpacityHover());
    } else {
        borderColor.setAlphaF(buttonOpacityNormal());
    }

    const int borderWidth = 1;
    const qreal outerCircleRadius = 10; // diameter = 20
    const QRect outerCircleRect(rect.center() + QPoint(1, 1) - QPoint(outerCircleRadius, outerCircleRadius),
                                QSize(outerCircleRadius, outerCircleRadius) * 2);
    drawRoundedRect(painter, outerCircleRect, outerCircleRadius, backgroundColor, QPen(borderColor, borderWidth));

    if (selected || pressed) {
        QColor centerColor = accentColor();
        const int innerCircleRadius = 5; // diameter = 10
        const QRect innerCircleRect(rect.center() + QPoint(1, 1) - QPoint(innerCircleRadius, innerCircleRadius),
                                    QSize(innerCircleRadius, innerCircleRadius) * 2);
        drawRoundedRect(painter, innerCircleRect, innerCircleRadius, centerColor);
    }
}

void UiTheme::drawIndicatorIcon(QPainter* painter, const QRect& rect, bool enabled, QStyle::PrimitiveElement element) const
{
    QColor color = fontPrimaryColor();
    if (!enabled) {
        color.setAlphaF(itemOpacityDisabled());
    }
    painter->setPen(color);
    painter->setFont(m_iconsFont);

    IconCode::Code code;
    switch (element) {
    case QStyle::PE_IndicatorSpinPlus:
        code = IconCode::Code::PLUS;
        break;
    case QStyle::PE_IndicatorSpinMinus:
        code = IconCode::Code::MINUS;
        break;
    case QStyle::PE_IndicatorSpinUp:
        code = IconCode::Code::SMALL_ARROW_UP;
        break;
    case QStyle::PE_IndicatorSpinDown:
        code = IconCode::Code::SMALL_ARROW_DOWN;
        break;
    default:
        return;
    }

    painter->drawText(rect, Qt::AlignCenter, iconCodeToChar(code));
}

void UiTheme::drawListViewItemBackground(QPainter* painter, const QRect& rect, bool enabled, bool hovered, bool pressed,
                                         bool selected) const
{
    QColor backgroundColor(Qt::transparent);
    if (selected) {
        backgroundColor = accentColor();
        backgroundColor.setAlphaF(enabled ? accentOpacityHit() : accentOpacityHit() * itemOpacityDisabled());
    } else if (enabled && pressed) {
        backgroundColor = buttonColor();
        backgroundColor.setAlphaF(buttonOpacityHit());
    } else if (enabled && hovered) {
        backgroundColor = buttonColor();
        backgroundColor.setAlphaF(buttonOpacityHover());
    } else {
        return; // filling a rect with transparent does not make sense
    }

    painter->fillRect(rect, backgroundColor);
}

void UiTheme::drawToolbarGrip(QPainter* painter, const QRect& rect, bool horizontal) const
{
    QColor gripColor(fontPrimaryColor());
    gripColor.setAlphaF(0.5);
    painter->setPen(gripColor);
    painter->setFont(m_iconsFont);

    int rotation = horizontal ? 0 : 90;
    QRect r = horizontal ? rect : QRect(rect.topLeft() + QPoint(0, -rect.width()),
                                        rect.size().transposed());
    painter->rotate(rotation);
    painter->drawText(r, Qt::AlignCenter, iconCodeToChar(IconCode::Code::TOOLBAR_GRIP));
    painter->rotate(-rotation);
}

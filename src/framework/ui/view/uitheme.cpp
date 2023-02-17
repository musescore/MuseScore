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
#include <QTextEdit>
#include <QLineEdit>
#include <QVariant>

#include <math.h>

#include "log.h"

using namespace mu::ui;

static const QPen NO_BORDER(Qt::transparent, 0);
static const QBrush NO_FILL(Qt::transparent);
static const int DEFAULT_RADIUS = 3;

static const int GROUP_BOX_LABEL_SPACING = 2;

//! In QML, a border is drawn _inside_ a rectangle.
//! In C++, a border would normally be drawn half inside the rectangle, half outside.
//! In this function, we exactly replicate the behaviour from QML.
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
    calculateDefaultButtonSize();

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

    m_borderWidth = themeValues[BORDER_WIDTH].toReal();
    m_navCtrlBorderWidth = themeValues[NAVIGATION_CONTROL_BORDER_WIDTH].toReal();
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
    calculateDefaultButtonSize();
    initThemeValues();
    setupWidgetTheme();
    notifyAboutThemeChanged();
}

bool UiTheme::isDark() const
{
    return configuration()->isDarkMode();
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

QFont UiTheme::defaultFont() const
{
    return m_defaultFont;
}

qreal UiTheme::defaultButtonSize() const
{
    return m_defaultButtonSize;
}

qreal UiTheme::borderWidth() const
{
    return m_borderWidth;
}

qreal UiTheme::navCtrlBorderWidth() const
{
    return m_navCtrlBorderWidth;
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

int UiTheme::flickableMaxVelocity() const
{
    return configuration()->flickableMaxVelocity();
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
        font->setWeight(weight);
    }

    m_defaultFont.setFamily(QString::fromStdString(configuration()->defaultFontFamily()));
    m_defaultFont.setPixelSize(configuration()->defaultFontSize());
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

void UiTheme::calculateDefaultButtonSize()
{
    constexpr qreal MINIMUM_BUTTON_SIZE = 30.0;
    constexpr qreal BUTTON_PADDING = 8.0;

    QFontMetricsF bodyFontMetrics(m_bodyFont);
    QFontMetricsF iconFontMetrics(m_iconsFont);

    qreal requiredSize = std::max(bodyFontMetrics.height(), iconFontMetrics.height()) + BUTTON_PADDING;
    m_defaultButtonSize = std::max(requiredSize, MINIMUM_BUTTON_SIZE);
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

    QColor buttonColorDisabled = buttonColor();
    buttonColorDisabled.setAlphaF(itemOpacityDisabled());

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

    palette.setColor(QPalette::Button, buttonColor());
    palette.setColor(QPalette::Disabled, QPalette::Button, buttonColorDisabled);
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
        || qobject_cast<QGroupBox*>(widget)
        || qobject_cast<QLineEdit*>(widget)) {
        // Make hovering work
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void UiTheme::unpolish(QWidget* widget)
{
    QProxyStyle::unpolish(widget);

    if (qobject_cast<QAbstractItemView*>(widget)
        || qobject_cast<QGroupBox*>(widget)
        || qobject_cast<QLineEdit*>(widget)) {
        widget->setMouseTracking(false);
        widget->setAttribute(Qt::WA_Hover, false);
    }
}

void UiTheme::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                            const QWidget* widget) const
{
    StyleState styleState;
    styleState.enabled = option->state & State_Enabled;
    styleState.hovered = option->state & State_MouseOver;
    styleState.pressed = option->state & State_Sunken;
    styleState.focused = (option->state & State_KeyboardFocusChange) && (option->state & State_HasFocus);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    //! NOTE This drawing code is based on the implementation in QML.
    switch (element) {
    // Buttons (and ComboBoxes)
    case QStyle::PE_PanelButtonCommand: {
        auto buttonOption = qstyleoption_cast<const QStyleOptionButton*>(option);
        const bool accentButton = (buttonOption && buttonOption->features & QStyleOptionButton::DefaultButton)
                                  || option->state & State_On;
        const bool flat = (buttonOption && buttonOption->features & QStyleOptionButton::Flat)
                          && !(option->state & State_On);

        QColor paletteColor = widget ? widget->palette().color(QPalette::Button) : QColor();
        const QColor background = paletteColor.isValid() ? paletteColor : buttonColor();

        drawButtonBackground(painter, option->rect, styleState, accentButton, flat, background);
    } break;
    case QStyle::PE_FrameDefaultButton: {
        auto buttonOption = qstyleoption_cast<const QStyleOptionButton*>(option);
        const bool flat = (buttonOption && buttonOption->features & QStyleOptionButton::Flat)
                          && !(option->state & State_On);
        if (flat && styleState.focused) {
            // For other buttons, this is done in `drawButtonBackground`, but that is not called for flat buttons
            drawRoundedRect(painter, option->rect, DEFAULT_RADIUS, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
        }
    } break;

    // Checkboxes
    case QStyle::PE_IndicatorCheckBox: {
        const bool indeterminate = option->state & State_NoChange;
        const bool checked = option->state & State_On;
        const bool inMenu = qobject_cast<const QMenu*>(widget);

        drawCheckboxIndicator(painter, option->rect, styleState, checked, indeterminate, inMenu);
    } break;

    // Radio buttons
    case QStyle::PE_IndicatorRadioButton: {
        const bool selected = option->state & State_On;

        drawRadioButtonIndicator(painter, option->rect, styleState, selected);
    } break;

    case QStyle::PE_FrameLineEdit: {
        const bool editing = option->state & State_HasFocus;

        drawLineEditBackground(painter, option->rect, styleState, editing);
    } break;

    case QStyle::PE_FrameFocusRect: {
        bool isTreeWidget = option->styleObject && option->styleObject->inherits("QTreeWidget");
        if (isTreeWidget) {
            drawRoundedRect(painter, option->rect, 1, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
        }

        //! NOTE: need for removing frame focus rectangle
    } break;

    // Indicator icons
    case QStyle::PE_IndicatorSpinUp:
    case QStyle::PE_IndicatorSpinDown:
    case QStyle::PE_IndicatorSpinPlus:
    case QStyle::PE_IndicatorSpinMinus: {
        drawIndicatorIcon(painter, option->rect, styleState, element);
    } break;

    // ViewItem
    case QStyle::PE_PanelItemViewItem: {
        bool selected = option->state & State_Selected;

        drawViewItemBackground(painter, option->rect, styleState, selected);
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
        drawRoundedRect(painter, option->rect, DEFAULT_RADIUS, QBrush("#03000000"), QPen(strokeColor(), fmax(borderWidth(), 1.0)));
    } break;

    // Menu
    case QStyle::PE_PanelMenu: {
        drawRoundedRect(painter, option->rect, 1, backgroundPrimaryColor(), QPen(strokeColor(), borderWidth()));
    } break;
    case QStyle::PE_FrameMenu: {
        drawRoundedRect(painter, option->rect, 1, NO_FILL, QPen(strokeColor(), borderWidth()));
    } break;

    case QStyle::PE_Frame: {
        if (qobject_cast<const QTextEdit*>(widget) != nullptr) {
            if (styleState.enabled) {
                drawRoundedRect(painter, option->rect, DEFAULT_RADIUS, NO_FILL, QPen(strokeColor(), borderWidth()));
            } else {
                QColor penBorderColor = strokeColor();
                penBorderColor.setAlphaF(itemOpacityDisabled());
                drawRoundedRect(painter, option->rect, DEFAULT_RADIUS, NO_FILL, QPen(penBorderColor, borderWidth()));
            }
        }
    } break;

    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }

    painter->restore();
}

void UiTheme::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const
{
    StyleState styleState;
    styleState.enabled = option->state & State_Enabled;
    styleState.hovered = option->state & State_MouseOver;
    styleState.pressed = option->state & State_Sunken;
    styleState.focused = (option->state & State_KeyboardFocusChange) && (option->state & State_HasFocus);

    switch (control) {
    case CC_ScrollBar: {
        QProxyStyle::drawComplexControl(control, option, painter, widget);

        if (configuration()->isHighContrast()) {
            QRect scrollBarHandle = QProxyStyle::subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);

            QColor handleColor = fontPrimaryColor();
            handleColor.setAlphaF(
                !styleState.enabled ? buttonOpacityNormal()
                * itemOpacityDisabled() : styleState.pressed ? buttonOpacityHit() : styleState.hovered ? buttonOpacityHover() : buttonOpacityNormal());

            drawRoundedRect(painter, option->rect, 1, NO_FILL, QPen(strokeColor(), borderWidth()));
            drawRoundedRect(painter, scrollBarHandle, 1, handleColor, NO_BORDER);
        }
    } break;

    case CC_SpinBox: {
        QProxyStyle::drawComplexControl(control, option, painter, widget);

        QRect spinBoxFrame = QProxyStyle::subControlRect(CC_SpinBox, option, SC_SpinBoxFrame, widget);
        if (styleState.focused) {
            drawRoundedRect(painter, spinBoxFrame, DEFAULT_RADIUS, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
        }
    } break;

    default: QProxyStyle::drawComplexControl(control, option, painter, widget);
    }
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
    case PM_ExclusiveIndicatorWidth: // Radio button
    case PM_ExclusiveIndicatorHeight:
        return 20;
    case PM_CheckBoxLabelSpacing: // Checkbox
    case PM_RadioButtonLabelSpacing: // Radio button
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

QIcon UiTheme::standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const
{
    switch (standardIcon) {
    case SP_DialogOkButton:
    case SP_DialogCancelButton:
    case SP_DialogHelpButton:
    case SP_DialogOpenButton:
    case SP_DialogSaveButton:
    case SP_DialogCloseButton:
    case SP_DialogApplyButton:
    case SP_DialogResetButton:
    case SP_DialogDiscardButton:
    case SP_DialogYesButton:
    case SP_DialogNoButton:
    case SP_DialogYesToAllButton:
    case SP_DialogNoToAllButton:
    case SP_DialogSaveAllButton:
    case SP_DialogAbortButton:
    case SP_DialogRetryButton:
    case SP_DialogIgnoreButton:
        return {};
    default:
        return QProxyStyle::standardIcon(standardIcon, option, widget);
    }
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

void UiTheme::drawButtonBackground(QPainter* painter, const QRect& rect, const StyleState& styleState, bool accentButton, bool flat,
                                   const QColor& defaultBackground) const
{
    QColor backgroundColor(accentButton ? accentColor() : defaultBackground);

    if (styleState.enabled) {
        backgroundColor.setAlphaF(styleState.pressed ? buttonOpacityHit()
                                  : styleState.hovered ? buttonOpacityHover()
                                  : !flat ? buttonOpacityNormal()
                                  : 0.0);

        if (configuration()->isHighContrast()) {
            QColor penBorderColor(strokeColor());
            penBorderColor.setAlphaF(styleState.pressed ? buttonOpacityHit()
                                     : styleState.hovered ? buttonOpacityHover()
                                     : buttonOpacityNormal());

            drawRoundedRect(painter, rect, DEFAULT_RADIUS, backgroundColor, QPen(penBorderColor, borderWidth()));
        } else {
            drawRoundedRect(painter, rect, DEFAULT_RADIUS, backgroundColor, NO_BORDER);
        }
    } else {
        backgroundColor.setAlphaF(flat ? 0.0 : buttonOpacityNormal() * itemOpacityDisabled());

        drawRoundedRect(painter, rect, DEFAULT_RADIUS, backgroundColor, NO_BORDER);
    }

    if (styleState.focused) {
        drawRoundedRect(painter, rect, DEFAULT_RADIUS, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
    }
}

void UiTheme::drawCheckboxIndicator(QPainter* painter, const QRect& rect, const StyleState& styleState, bool checked, bool indeterminate,
                                    bool inMenu) const
{
    QColor backgroundColor = buttonColor();
    const qreal borderRadius = 2;

    if (!inMenu) {
        backgroundColor.setAlphaF(!styleState.enabled ? buttonOpacityNormal() * itemOpacityDisabled()
                                  : styleState.pressed ? buttonOpacityHit()
                                  : styleState.hovered ? buttonOpacityHover()
                                  : buttonOpacityNormal());

        QColor penBorderColor(Qt::transparent);
        int penBorderWidth = 0;
        if (configuration()->isHighContrast()) {
            penBorderColor = strokeColor();
            penBorderWidth = styleState.enabled ? borderWidth() : 0;
        } else {
            penBorderWidth = styleState.enabled && (styleState.hovered || styleState.pressed) ? borderWidth() : 0;
        }

        penBorderColor.setAlphaF(styleState.pressed ? buttonOpacityHit() : styleState.hovered ? buttonOpacityHover() : buttonOpacityNormal());
        drawRoundedRect(painter, rect, borderRadius, backgroundColor, QPen(penBorderColor, penBorderWidth));
    }

    if (styleState.focused) {
        drawRoundedRect(painter, rect, borderRadius, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
    }

    if (checked || indeterminate) {
        QColor tickColor = fontPrimaryColor();
        if (!styleState.enabled) {
            tickColor.setAlphaF(itemOpacityDisabled());
        }
        painter->setPen(tickColor);
        painter->setFont(m_iconsFont);
        painter->drawText(rect, Qt::AlignCenter,
                          iconCodeToChar(indeterminate ? IconCode::Code::MINUS : IconCode::Code::TICK_RIGHT_ANGLE));
    }
}

void UiTheme::drawRadioButtonIndicator(QPainter* painter, const QRect& rect, const StyleState& styleState, bool selected) const
{
    QColor borderColor = fontPrimaryColor();
    QColor backgroundColor = textFieldColor();

    if (styleState.pressed) {
        borderColor.setAlphaF(buttonOpacityHit());
    } else if (styleState.hovered) {
        borderColor.setAlphaF(buttonOpacityHover());
    } else {
        borderColor.setAlphaF(buttonOpacityNormal());
    }

    const int borderWidth = 1;
    qreal outerCircleRadius = 10; // diameter = 20

    if (styleState.focused) {
        const qreal focusCircleRadius = outerCircleRadius;
        const QRect focusCircleRect(rect.center() + QPoint(1, 1) - QPoint(focusCircleRadius, focusCircleRadius),
                                    QSize(focusCircleRadius, focusCircleRadius) * 2);
        drawRoundedRect(painter, focusCircleRect, focusCircleRadius, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));

        outerCircleRadius -= navCtrlBorderWidth();
    }

    const QRect outerCircleRect(rect.center() + QPoint(1, 1) - QPoint(outerCircleRadius, outerCircleRadius),
                                QSize(outerCircleRadius, outerCircleRadius) * 2);
    drawRoundedRect(painter, outerCircleRect, outerCircleRadius, backgroundColor, QPen(borderColor, borderWidth));

    if (selected || styleState.pressed) {
        QColor centerColor = accentColor();
        const int innerCircleRadius = 5; // diameter = 10
        const QRect innerCircleRect(rect.center() + QPoint(1, 1) - QPoint(innerCircleRadius, innerCircleRadius),
                                    QSize(innerCircleRadius, innerCircleRadius) * 2);
        drawRoundedRect(painter, innerCircleRect, innerCircleRadius, centerColor);
    }
}

void UiTheme::drawLineEditBackground(QPainter* painter, const QRect& rect, const StyleState& styleState, bool editing) const
{
    QColor backgroundColor = textFieldColor();
    backgroundColor.setAlphaF(!styleState.enabled ? itemOpacityDisabled() : (editing ? 1 : (styleState.hovered ? 0.6 : 1)));

    if (styleState.focused) {
        drawRoundedRect(painter, rect, DEFAULT_RADIUS, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
    }

    QRect editRect = rect.adjusted(navCtrlBorderWidth(), navCtrlBorderWidth(), -navCtrlBorderWidth(), -navCtrlBorderWidth());
    QColor penBorderColor = editing ? accentColor() : strokeColor();
    penBorderColor.setAlphaF(editing ? 1 : (styleState.hovered ? 0.6 : 1));
    int borderWidth = configuration()->isHighContrast() ? this->borderWidth() : 1;

    if (styleState.enabled) {
        drawRoundedRect(painter, editRect, DEFAULT_RADIUS, backgroundColor, QPen(penBorderColor, borderWidth));
    } else {
        drawRoundedRect(painter, editRect, DEFAULT_RADIUS, backgroundColor, QPen(penBorderColor, 1));
    }
}

void UiTheme::drawIndicatorIcon(QPainter* painter, const QRect& rect, const StyleState& styleState, QStyle::PrimitiveElement element) const
{
    QColor color = fontPrimaryColor();
    if (!styleState.enabled) {
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
    drawRoundedRect(painter, rect, 1, NO_FILL, QPen(strokeColor(), borderWidth())); //does nothing apparently
}

void UiTheme::drawViewItemBackground(QPainter* painter, const QRect& rect, const StyleState& styleState, bool selected) const
{
    QColor backgroundColor(Qt::transparent);
    if (selected) {
        backgroundColor = accentColor();
        backgroundColor.setAlphaF(styleState.enabled ? accentOpacityHit() : accentOpacityHit() * itemOpacityDisabled());
    } else if (styleState.enabled && styleState.pressed) {
        backgroundColor = buttonColor();
        backgroundColor.setAlphaF(buttonOpacityHit());
    } else if (styleState.enabled && styleState.hovered) {
        backgroundColor = buttonColor();
        backgroundColor.setAlphaF(buttonOpacityHover());
    }

    painter->fillRect(rect, backgroundColor);

    if (configuration()->isHighContrast()) {
        drawRoundedRect(painter, rect, 1, NO_FILL, QPen(strokeColor(), borderWidth()));
    }

    if (styleState.focused) {
        drawRoundedRect(painter, rect, 1, NO_FILL, QPen(fontPrimaryColor(), navCtrlBorderWidth()));
    }
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

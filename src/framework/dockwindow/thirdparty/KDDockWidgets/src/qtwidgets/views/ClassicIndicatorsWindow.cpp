/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ClassicIndicatorsWindow.h"
#include "kddockwidgets/core/indicators/ClassicDropIndicatorOverlay.h"
#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/ViewFactory.h"
#include "kddockwidgets/Config.h"
#include "View.h"
#include "core/Utils_p.h"


#include <QPainter>

#include <utility>

#ifdef QT_X11EXTRAS_LIB
#include <QtX11Extras/QX11Info>
#endif

#define INDICATOR_WIDTH 40
#define OUTTER_INDICATOR_MARGIN 10

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;
using namespace KDDockWidgets::QtWidgets;

namespace KDDockWidgets {

inline bool windowManagerHasTranslucency()
{
    if (qEnvironmentVariableIsSet("KDDW_NO_TRANSLUCENCY")
        || (Config::self().internalFlags() & Config::InternalFlag_DisableTranslucency))
        return false;

#ifdef QT_X11EXTRAS_LIB
    if (isXCB())
        return QX11Info::isCompositingManagerRunning();
#endif

    // macOS and Windows are fine
    return true;
}

class Indicator : public QWidget
{
    Q_OBJECT
public:
    typedef QList<Indicator *> List;
    explicit Indicator(IndicatorWindow *parent, DropLocation location);
    void paintEvent(QPaintEvent *) override;

    void setHovered(bool hovered);
    QString iconName(bool active) const;
    QString iconFileName(bool active) const;

    QImage m_image;
    QImage m_imageActive;
    bool m_hovered = false;
    const DropLocation m_dropLocation;
};

static QString iconName(DropLocation loc, bool active)
{
    QString suffix = active ? QStringLiteral("_active") : QString();

    QString name;
    switch (loc) {
    case DropLocation_Center:
        name = QStringLiteral("center");
        break;
    case DropLocation_Left:
        name = QStringLiteral("inner_left");
        break;
    case DropLocation_Right:
        name = QStringLiteral("inner_right");
        break;
    case DropLocation_Bottom:
        name = QStringLiteral("inner_bottom");
        break;
    case DropLocation_Top:
        name = QStringLiteral("inner_top");
        break;
    case DropLocation_OutterLeft:
        name = QStringLiteral("outter_left");
        break;
    case DropLocation_OutterBottom:
        name = QStringLiteral("outter_bottom");
        break;
    case DropLocation_OutterRight:
        name = QStringLiteral("outter_right");
        break;
    case DropLocation_OutterTop:
        name = QStringLiteral("outter_top");
        break;
    case DropLocation_None:
    case DropLocation_Inner:
    case DropLocation_Outter:
    case DropLocation_Horizontal:
    case DropLocation_Vertical:
        return QString();
    }

    return name + suffix;
}
}

void Indicator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (m_hovered)
        p.drawImage(rect(), m_imageActive, rect());
    else
        p.drawImage(rect(), m_image, rect());
}

void Indicator::setHovered(bool hovered)
{
    if (hovered != m_hovered) {
        m_hovered = hovered;
        update();
    }
}

QString Indicator::iconName(bool active) const
{
    return KDDockWidgets::iconName(m_dropLocation, active);
}

QString Indicator::iconFileName(bool active) const
{
    const QString name = iconName(active);
    const QString path = Config::self().viewFactory()->classicIndicatorsPath();

    return KDDockWidgets::windowManagerHasTranslucency()
        ? QStringLiteral("%1/%2.png").arg(path, name)
        : QStringLiteral("%1/opaque/%2.png").arg(path, name);
}

static QWidget *parentForIndicatorWindow(ClassicDropIndicatorOverlay *classicIndicators_)
{
    // On Wayland it can't be a top-level, as we have no way of positioning it

    return isWayland() ? QtCommon::View_qt::asQWidget(classicIndicators_->view()) : nullptr;
}

static Qt::WindowFlags flagsForIndicatorWindow()
{
    return isWayland() ? Qt::Widget : (Qt::Tool | Qt::BypassWindowManagerHint);
}

IndicatorWindow::IndicatorWindow(ClassicDropIndicatorOverlay *classicIndicators_)
    : QWidget(parentForIndicatorWindow(classicIndicators_), flagsForIndicatorWindow())
    , classicIndicators(classicIndicators_)
    , m_center(new Indicator(this, DropLocation_Center)) // Each indicator is not a top-level. Otherwise
                                                         // there's noticeable delay.
    , m_left(new Indicator(this, DropLocation_Left))
    , m_right(new Indicator(this, DropLocation_Right))
    , m_bottom(new Indicator(this, DropLocation_Bottom))
    , m_top(new Indicator(this, DropLocation_Top))
    , m_outterLeft(new Indicator(this, DropLocation_OutterLeft))
    , m_outterRight(new Indicator(this, DropLocation_OutterRight))
    , m_outterBottom(new Indicator(this, DropLocation_OutterBottom))
    , m_outterTop(new Indicator(this, DropLocation_OutterTop))
{
    setWindowFlag(Qt::FramelessWindowHint, true);

    if (Config::self().flags() & Config::Flag_KeepAboveIfNotUtilityWindow) {
        // Ensure the overlay window is on top
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
    }

    setAttribute(Qt::WA_TranslucentBackground);

    m_indicators << m_center << m_left << m_right << m_top << m_bottom << m_outterBottom
                 << m_outterTop << m_outterLeft << m_outterRight;
}

Indicator *IndicatorWindow::indicatorForLocation(DropLocation loc) const
{
    switch (loc) {
    case DropLocation_Center:
        return m_center;
    case DropLocation_Left:
        return m_left;
    case DropLocation_Right:
        return m_right;
    case DropLocation_Bottom:
        return m_bottom;
    case DropLocation_Top:
        return m_top;
    case DropLocation_OutterLeft:
        return m_outterLeft;
    case DropLocation_OutterBottom:
        return m_outterBottom;
    case DropLocation_OutterRight:
        return m_outterRight;
    case DropLocation_OutterTop:
        return m_outterTop;
    case DropLocation_None:
    case DropLocation_Outter:
    case DropLocation_Inner:
    case DropLocation_Horizontal:
    case DropLocation_Vertical:
        return nullptr;
    }

    return nullptr;
}

void IndicatorWindow::updateMask()
{
    QRegion region;

    if (!KDDockWidgets::windowManagerHasTranslucency()) {
        for (Indicator *indicator : std::as_const(m_indicators)) {
            if (indicator->isVisible())
                region = region.united(QRegion(indicator->geometry(), QRegion::Rectangle));
        }
    }

    setMask(region);
}

void IndicatorWindow::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    updatePositions();
}

void IndicatorWindow::updateIndicatorVisibility()
{
    for (Indicator *indicator : { m_left, m_right, m_bottom, m_top, m_outterTop, m_outterLeft,
                                  m_outterRight, m_outterBottom, m_center })
        indicator->setVisible(classicIndicators->dropIndicatorVisible(indicator->m_dropLocation));

    updateMask();
}

QPoint IndicatorWindow::posForIndicator(DropLocation loc) const
{
    Indicator *indicator = indicatorForLocation(loc);
    return indicator->mapToGlobal(indicator->rect().center());
}

DropLocation IndicatorWindow::hover(QPoint globalPos)
{
    DropLocation loc = DropLocation_None;

    for (Indicator *indicator : std::as_const(m_indicators)) {
        if (indicator->isVisible()) {
            const bool hovered = indicator->rect().contains(indicator->mapFromGlobal(globalPos));
            indicator->setHovered(hovered);
            if (hovered)
                loc = indicator->m_dropLocation;
        }
    }

    return loc;
}

void IndicatorWindow::updatePositions()
{
    QRect r = rect();
    const int indicatorWidth = m_outterBottom->width();
    const int halfIndicatorWidth = m_outterBottom->width() / 2;

    m_outterLeft->move(r.x() + OUTTER_INDICATOR_MARGIN, r.center().y() - halfIndicatorWidth);
    m_outterBottom->move(r.center().x() - halfIndicatorWidth,
                         r.y() + height() - indicatorWidth - OUTTER_INDICATOR_MARGIN);
    m_outterTop->move(r.center().x() - halfIndicatorWidth, r.y() + OUTTER_INDICATOR_MARGIN);
    m_outterRight->move(r.x() + width() - indicatorWidth - OUTTER_INDICATOR_MARGIN,
                        r.center().y() - halfIndicatorWidth);
    Core::Group *hoveredGroup = classicIndicators->hoveredGroup();
    if (hoveredGroup) {
        QRect hoveredRect = hoveredGroup->view()->geometry();
        m_center->move(r.topLeft() + hoveredRect.center()
                       - QPoint(halfIndicatorWidth, halfIndicatorWidth));
        m_top->move(m_center->pos() - QPoint(0, indicatorWidth + OUTTER_INDICATOR_MARGIN));
        m_right->move(m_center->pos() + QPoint(indicatorWidth + OUTTER_INDICATOR_MARGIN, 0));
        m_bottom->move(m_center->pos() + QPoint(0, indicatorWidth + OUTTER_INDICATOR_MARGIN));
        m_left->move(m_center->pos() - QPoint(indicatorWidth + OUTTER_INDICATOR_MARGIN, 0));
    }
}

void IndicatorWindow::raise()
{
    QWidget::raise();
}

void IndicatorWindow::setGeometry(QRect rect)
{
    QWidget::setGeometry(rect);
}

void IndicatorWindow::setObjectName(const QString &name)
{
    QWidget::setObjectName(name);
}

void IndicatorWindow::setVisible(bool is)
{
    QWidget::setVisible(is);
}

void IndicatorWindow::resize(QSize size)
{
    QWidget::resize(size);
}

bool IndicatorWindow::isWindow() const
{
    return QWidget::isWindow();
}

Indicator::Indicator(IndicatorWindow *parent,
                     DropLocation location)
    : QWidget(parent)
    , m_dropLocation(location)
{
    m_image = QImage(iconFileName(/*active=*/false)).scaled(INDICATOR_WIDTH, INDICATOR_WIDTH);
    m_imageActive = QImage(iconFileName(/*active=*/true)).scaled(INDICATOR_WIDTH, INDICATOR_WIDTH);
    setFixedSize(m_image.size());
    setVisible(true);
}

#include "ClassicIndicatorsWindow.moc"

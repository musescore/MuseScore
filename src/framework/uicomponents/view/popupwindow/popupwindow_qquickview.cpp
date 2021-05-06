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
#include "popupwindow_qquickview.h"

#include <QTimer>
#include "log.h"

using namespace mu::uicomponents;

PopupWindow_QQuickView::PopupWindow_QQuickView(QObject* parent)
    : QObject(parent)
{
    setObjectName("PopupWindow");
}

PopupWindow_QQuickView::~PopupWindow_QQuickView()
{
    delete m_view;
}

void PopupWindow_QQuickView::init(QQmlEngine* engine, std::shared_ptr<ui::IUiConfiguration> uiConfiguration, bool isDialogMode)
{
    //! NOTE Without a parent on MacOS with FullScreen, the popup is shown on another virtual Desktop.
    //! With parent on WinOS not work transparent background.
#ifdef Q_OS_MAC
    m_view = new QQuickView(engine, mainWindow()->qWindow());
#else
    m_view = new QQuickView(engine, nullptr);
#endif

    m_view->setObjectName("PopupQQuickViewWindow");
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);

    // dialog
    if (isDialogMode) {
        m_view->setFlags(Qt::Dialog);
        QString bgColorStr = uiConfiguration->currentTheme().values.value(ui::BACKGROUND_PRIMARY_COLOR).toString();
        m_view->setColor(QColor(bgColorStr));
    }
    // popup
    else {
        m_view->setFlags(Qt::Dialog                             // The most appropriate behavior for us on all platforms
                         | Qt::FramelessWindowHint              // Without border
                         | Qt::NoDropShadowWindowHint           // Without system shadow
                         | Qt::BypassWindowManagerHint          // Otherwise, it does not work correctly on Gnome (Linux) when resizing)
                         );

        m_view->setColor(QColor(Qt::transparent));
    }

    m_view->installEventFilter(this);
}

void PopupWindow_QQuickView::setContent(QQuickItem* item)
{
    m_view->setContent(QUrl(), nullptr, item);

    connect(item, &QQuickItem::implicitWidthChanged, [this, item]() {
        if (!m_view->isVisible()) {
            return;
        }
        if (item->implicitWidth() != m_view->width()) {
            m_view->resize(item->implicitWidth(), item->implicitHeight());
        }
    });

    connect(item, &QQuickItem::implicitHeightChanged, [this, item]() {
        if (!m_view->isVisible()) {
            return;
        }
        if (item->implicitHeight() != m_view->height()) {
            m_view->resize(item->implicitWidth(), item->implicitHeight());
        }
    });
}

void PopupWindow_QQuickView::forceActiveFocus()
{
    if (!m_view->rootObject()->hasActiveFocus()) {
        m_view->rootObject()->forceActiveFocus();
    }
}

void PopupWindow_QQuickView::show(QPoint p)
{
    m_view->setPosition(p);
    m_view->show();

    m_view->requestActivate();

    QQuickItem* item = m_view->rootObject();
    m_view->resize(item->implicitWidth(), item->implicitHeight());

    QTimer::singleShot(0, [this]() {
        forceActiveFocus();
    });
}

void PopupWindow_QQuickView::hide()
{
    m_view->hide();
}

QWindow* PopupWindow_QQuickView::qWindow() const
{
    return m_view;
}

bool PopupWindow_QQuickView::isVisible() const
{
    return m_view->isVisible();
}

QRect PopupWindow_QQuickView::geometry() const
{
    return m_view->geometry();
}

void PopupWindow_QQuickView::setOnHidden(const std::function<void()>& callback)
{
    m_onHidden = callback;
}

bool PopupWindow_QQuickView::eventFilter(QObject* watched, QEvent* event)
{
// Please, don't remove
//#define POPUPWINDOW_DEBUG_EVENTS_ENABLED
#ifdef POPUPWINDOW_DEBUG_EVENTS_ENABLED
    static QMetaEnum typeEnum = QMetaEnum::fromType<QEvent::Type>();
    static QList<QEvent::Type> excludeLoggingTypes = { QEvent::MouseMove };
    const char* typeStr = typeEnum.key(event->type());
    if (!excludeLoggingTypes.contains(event->type())) {
        LOGI() << (watched ? watched->objectName() : "null") << " event: " << (typeStr ? typeStr : "unknown");
    }

    static QList<QEvent::Type> trackEvents = { QEvent::Hide, QEvent::Show };
    if (trackEvents.contains(event->type())) {
        int k = 1;
    }

    if (QString(typeStr) == "WindowDeactivate") {
        int k = 1;
    }
#endif

    if (watched == m_view) {
        if (event->type() == QEvent::Hide) {
            if (m_onHidden) {
                m_onHidden();
            }
        }

        if (event->type() == QEvent::FocusIn) {
            m_view->rootObject()->forceActiveFocus();
        }

        if (event->type() == QEvent::MouseButtonPress) {
            forceActiveFocus();
        }
    }

    return QObject::eventFilter(watched, event);
}

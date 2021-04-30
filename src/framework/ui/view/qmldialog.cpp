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
#include "qmldialog.h"

#include <functional>

#include <QQmlEngine>
#include <QDialog>
#include <QHBoxLayout>
#include <QApplication>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "log.h"

namespace mu::ui {
class QDialogWrap : public QDialog
{
public:
    QDialogWrap(QWidget* parent, Qt::WindowFlags f = Qt::WindowFlags())
        : QDialog(parent, f)
    {
    }

    std::function<void()> onShow;
    std::function<void()> onClose;

    bool event(QEvent* e) override
    {
        static QMetaEnum typeEnum = QMetaEnum::fromType<QEvent::Type>();
        const char* typeStr = typeEnum.key(e->type());
        LOGI() << " event: " << (typeStr ? typeStr : "unknown");

        return QDialog::event(e);
    }

    void showEvent(QShowEvent* e) override
    {
        QDialog::showEvent(e);
        if (onShow) {
            onShow();
        }
    }

    void closeEvent(QCloseEvent* e) override
    {
        QDialog::closeEvent(e);
        if (onClose) {
            onClose();
        }
    }
};
}

using namespace mu::ui;

QmlDialog::QmlDialog(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setErrCode(Ret::Code::Ok);
    setVisible(false);

    m_dialog = new QDialogWrap(QApplication::activeWindow());
    m_dialog->setObjectName("QmlDialog_QDialogWrap");
    m_dialog->installEventFilter(this);

    QPixmap dummyIcon(32, 32);
    dummyIcon.fill(Qt::transparent);
    m_dialog->setWindowIcon(dummyIcon);
    m_dialog->setWindowFlags(m_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_dialog->onShow = [this]() { onShow(); };
    m_dialog->onClose = [this]() { onClose(); };
    m_dialog->installEventFilter(this);

    connect(m_dialog, &QDialog::finished, [this](int code) {
        QDialog::DialogCode dialogCode = static_cast<QDialog::DialogCode>(code);

        switch (dialogCode) {
        case QDialog::Rejected: {
            setErrCode(Ret::Code::Cancel);
            break;
        }
        case QDialog::Accepted:
            break;
        }
    });

    m_navigation = new NavigationSection(this);
    m_navigation->setName("Dialog");
    m_navigation->setOrder(1);
}

void QmlDialog::setErrCode(Ret::Code code)
{
    QVariantMap ret;
    ret["errcode"] = static_cast<int>(code);
    setRet(ret);
}

void QmlDialog::componentComplete()
{
    QQuickItem::componentComplete();

    m_navigation->componentComplete();

    if (m_content) {
        QQmlEngine* engine = nullptr;
        engine = m_content->engine();

        m_view = new QQuickView(engine, m_dialog->windowHandle());
        m_view->setObjectName("QmlDialog_QQuickView");
        m_view->setResizeMode(QQuickView::SizeRootObjectToView);
        m_view->installEventFilter(this);

        m_widgetContainer = QWidget::createWindowContainer(m_view, m_dialog, Qt::Widget);
        m_widgetContainer->setObjectName("QmlDialog_QWidgetContainer");
        m_widgetContainer->installEventFilter(this);

        m_view->resize(width(), height());
        m_widgetContainer->resize(m_view->size());

        QQmlContext* ctx = QQmlEngine::contextForObject(this);
        QQuickItem* obj = qobject_cast<QQuickItem*>(m_content->create(ctx));
        obj->setParent(this);
        m_view->setContent(QUrl(), m_content, obj);

        m_dialog->resize(m_view->size());
        if (m_fixedSize.isValid()) {
            m_dialog->setFixedSize(m_fixedSize);
        } else {
            m_dialog->setMinimumSize(width(), height());
        }

        QHBoxLayout* layout = new QHBoxLayout(m_dialog);
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget(m_widgetContainer);
        m_dialog->setLayout(layout);
    }
}

bool QmlDialog::eventFilter(QObject* watched, QEvent* event)
{
    // Please, don't remove
#define QMLDIALOG_DEBUG_EVENTS_ENABLED
#ifdef QMLDIALOG_DEBUG_EVENTS_ENABLED
    static QMetaEnum typeEnum = QMetaEnum::fromType<QEvent::Type>();
    static QList<QEvent::Type> excludeLoggingTypes = { QEvent::MouseMove };
    const char* typeStr = typeEnum.key(event->type());
    if (!excludeLoggingTypes.contains(event->type())) {
        LOGI() << (watched ? watched->objectName() : "null") << " event: " << (typeStr ? typeStr : "unknown");
    }

    static QList<QEvent::Type> trackEvents = { QEvent::WindowDeactivate, QEvent::ActivationChange, QEvent::FocusAboutToChange };
    if (trackEvents.contains(event->type())) {
        int k = 1;
    }

    if (QString(typeStr) == "WindowDeactivate") {
        int k = 1;
    }
#endif

//    // QQuickView events
//    if (watched == m_view) {
//        if (event->type() == QEvent::FocusIn) {
//            m_view->contentItem()->forceActiveFocus();
//        }

//        if (event->type() == QEvent::MouseButtonPress) {
//            m_dialog->setFocus();
//            m_view->contentItem()->forceActiveFocus();
//        }
//    }

    return QObject::eventFilter(watched, event);
}

void QmlDialog::onShow()
{
    INavigationControl* ctrl = navigationController()->activeControl();
    NavigationControl* qmlCtrl = dynamic_cast<NavigationControl*>(ctrl);
    setParentControl(qmlCtrl);

    m_dialog->windowHandle()->installEventFilter(this);

    setVisible(true);
    m_dialog->activateWindow();
    m_dialog->setFocus();
//    m_widgetContainer->activateWindow();
//    m_widgetContainer->setFocus();
//    m_view->rootObject()->forceActiveFocus();
    emit opened();
}

void QmlDialog::onClose()
{
    emit closed();

    INavigationControl* ctrl = parentControl();
    if (ctrl) {
        ctrl->forceActiveRequested().send(ctrl);
    }
}

void QmlDialog::exec()
{
    m_dialog->exec();
}

void QmlDialog::show()
{
    m_dialog->show();
}

void QmlDialog::hide()
{
    m_dialog->hide();
}

void QmlDialog::reject()
{
    setErrCode(Ret::Code::Cancel);
    hide();
}

QQmlComponent* QmlDialog::content() const
{
    return m_content;
}

void QmlDialog::setContent(QQmlComponent* component)
{
    if (m_content == component) {
        return;
    }

    m_content = component;
    emit contentChanged();
}

QString QmlDialog::title() const
{
    return m_dialog->windowTitle();
}

void QmlDialog::setTitle(QString title)
{
    if (m_dialog->windowTitle() == title) {
        return;
    }

    m_dialog->setWindowTitle(title);
    emit titleChanged(title);
}

QString QmlDialog::objectID() const
{
    return m_objectID;
}

void QmlDialog::setObjectID(QString objectID)
{
    if (m_objectID == objectID) {
        return;
    }

    m_objectID = objectID;
    emit objectIDChanged(m_objectID);
}

bool QmlDialog::modal() const
{
    return m_dialog->isModal();
}

QSize QmlDialog::fixedSize() const
{
    return m_fixedSize;
}

void QmlDialog::setModal(bool modal)
{
    if (m_dialog->isModal() == modal) {
        return;
    }

    m_dialog->setModal(modal);
    emit modalChanged(m_dialog->isModal());
}

QVariantMap QmlDialog::ret() const
{
    return m_ret;
}

void QmlDialog::setRet(QVariantMap ret)
{
    if (m_ret == ret) {
        return;
    }

    m_ret = ret;
    emit retChanged(m_ret);
}

void QmlDialog::setFixedSize(QSize size)
{
    if (m_fixedSize == size) {
        return;
    }

    m_fixedSize = size;
    emit fixedSizeChanged(size);
}

NavigationSection* QmlDialog::navigation() const
{
    return m_navigation;
}

void QmlDialog::setParentControl(NavigationControl* parentControl)
{
    if (m_parentControl == parentControl) {
        return;
    }

    m_parentControl = parentControl;
    emit parentControlChanged();
}

NavigationControl* QmlDialog::parentControl() const
{
    return m_parentControl;
}

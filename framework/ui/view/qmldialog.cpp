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
#include "qmldialog.h"

#include <QQmlEngine>
#include <QDialog>
#include <QHBoxLayout>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::framework;

QmlDialog::QmlDialog(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setErrCode(Ret::Code::Ok);

    m_dialog = new QDialog();

    connect(m_dialog, &QDialog::finished, [this](int code) {
        QDialog::DialogCode dialogCode = static_cast<QDialog::DialogCode>(code);

        switch (dialogCode) {
        case QDialog::Rejected: {
            setErrCode(Ret::Code::Cancel);
            emit closed();
            break;
        }
        case QDialog::Accepted:
            break;
        }
    });
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

    if (m_content) {
        QQmlEngine* engine = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
        engine = framework::ioc()->resolve<framework::IUiEngine>("appshell")->qmlEngine();
#else
        engine = m_content->engine();
#endif

        m_view = new QQuickView(engine, nullptr);
        m_view->setResizeMode(QQuickView::SizeRootObjectToView);
        QWidget* widget = QWidget::createWindowContainer(m_view, nullptr, Qt::Widget);

        m_view->resize(width(), height());
        widget->resize(m_view->size());

        QQmlContext* ctx = QQmlEngine::contextForObject(this);
        QQuickItem* obj = qobject_cast<QQuickItem*>(m_content->create(ctx));
        obj->setParent(this);
        m_view->setContent(QUrl(), m_content, obj);

        m_dialog->resize(m_view->size());
        widget->setParent(m_dialog);
        QHBoxLayout* layout = new QHBoxLayout(m_dialog);
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget(widget);
        m_dialog->setLayout(layout);
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
    emit closed();
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

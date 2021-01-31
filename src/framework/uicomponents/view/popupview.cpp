#include "popupview.h"

#include <QWidget>
#include <QQmlEngine>
#include <QUrl>

PopupView::PopupView(QQuickItem* parent) : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
}

QQmlComponent* PopupView::contentItem() const
{
    return m_contentItem;
}

void PopupView::show()
{
    if (!m_view) {
        return;
    }

    m_view->show();
}

void PopupView::hide()
{
    if (!m_view) {
        return;
    }

    m_view->hide();
}

void PopupView::setContentItem(QQmlComponent *contentItem)
{
    if (m_contentItem == contentItem)
        return;

    m_contentItem = contentItem;
    emit contentItemChanged(m_contentItem);
}

void PopupView::componentComplete()
{
    QQuickItem::componentComplete();

    if (!m_contentItem) {
        return;
    }

    m_view = new QQuickView(m_contentItem->engine(), nullptr);
    m_view->setResizeMode(QQuickView::SizeViewToRootObject);

    QWidget* windowContainer = QWidget::createWindowContainer(m_view, nullptr, Qt::Popup | Qt::FramelessWindowHint |  Qt::WindowStaysOnTopHint);
    windowContainer->setAttribute(Qt::WA_TranslucentBackground);
    windowContainer->setAttribute(Qt::WA_ShowWithoutActivating);

    QQmlContext* ctx = QQmlEngine::contextForObject(this);
    QQuickItem* obj = qobject_cast<QQuickItem*>(m_contentItem->create(ctx));
    m_view->setContent(QUrl(), m_contentItem, obj);
}

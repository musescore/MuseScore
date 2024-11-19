/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

// Yet another widget viewer. To be merged with ObjectViewer.
// Header only.
// Useful for applications not compatible with Gammaray.

#pragma once

#include <QtCore/QDebug>
#include <QtCore/QPointer>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMainWindow>

enum MyRole {
    WidgetRole = Qt::UserRole + 1,
    PixmapRole
};

namespace {

class DebugWidgetViewer : public QWidget
{
public:
    explicit DebugWidgetViewer(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        auto pickButton = new QPushButton(QStringLiteral("Pick widget"));
        pickButton->setObjectName("pick button");
        auto vlay = new QVBoxLayout(this);
        auto hlay = new QHBoxLayout();

        m_tree.setMinimumWidth(700);
        m_tree.setModel(&m_model);
        hlay->addWidget(&m_tree);
        hlay->addWidget(&m_preview);
        vlay->addLayout(hlay);
        vlay->addWidget(pickButton);
        connect(pickButton, &QPushButton::clicked, this,
                &DebugWidgetViewer::pickWidget);

        connect(m_tree.selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &DebugWidgetViewer::onSelectionChanged);
    }

    ~DebugWidgetViewer();

    void pickWidget()
    {
        setFilteringEnabled(true);
    }

    void loadWidget(QWidget *widget)
    {
        m_model.clear();
        if (!widget)
            return;

        QWidgetList path;
        auto p = widget;
        while (p) {
            path.append(p);
            p = p->parentWidget();
        }

        QStandardItem *previous = nullptr;
        for (auto it = path.crbegin(), e = path.crend(); it != e; ++it) {
            QWidget *w = *it;
            QString name = QString::fromLatin1(w->metaObject()->className());
            if (!w->objectName().isEmpty()) {
                name += "[" + w->objectName() + "]";
            }

            auto item = new QStandardItem(name);
            item->setData(QVariant::fromValue(w), WidgetRole);
            item->setData(pixmapForWidget(w), PixmapRole);

            auto parent = previous ? previous : m_model.invisibleRootItem();
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            parent->insertRow(0, item);
            previous = item;
        }

        renderTree();
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() != QEvent::MouseButtonPress)
            return false;

        if (auto w = qobject_cast<QWidget *>(watched)) {
            setFilteringEnabled(false);
            loadWidget(w);
            return true;
        }

        return false;
    }

    void setFilteringEnabled(bool enabled)
    {
        if (enabled) {
            qApp->setOverrideCursor(Qt::CrossCursor);
            qApp->installEventFilter(this);
        } else {
            qApp->removeEventFilter(this);
            qApp->restoreOverrideCursor();
        }
    }

    QWidget *rootWidget() const
    {
        return m_model.item(0, 0)->data(WidgetRole).value<QWidget *>();
    }

    void renderTree()
    {
        QWidget *rw = rootWidget();
        m_preview.setFixedSize(rw->size());
        m_previewPx = QPixmap(m_preview.size());
        renderTree(m_model.invisibleRootItem(), rw);
        m_preview.setPixmap(m_previewPx);
    }

    void renderTree(QStandardItem *node, const QWidget *rootWidget)
    {

        QWidget *widget = node->data(WidgetRole).value<QWidget *>();
        if (widget && node->checkState() == Qt::Checked) {
            const QPoint offset = widget->mapTo(rootWidget, QPoint(0, 0));
            widget->render(&m_previewPx, offset, {}, DrawWindowBackground);
        }

        const int count = node->rowCount();
        for (int i = 0; i < count; ++i) {
            renderTree(node->child(i, 0), rootWidget);
        }
    }

    QPixmap pixmapForWidget(QWidget *w)
    {
        QPixmap px(w->size());
        w->render(&px, {}, {}, DrawWindowBackground);
        return px;
    }

    void onSelectionChanged()
    {
        const QModelIndexList indexes = m_tree.selectionModel()->selectedIndexes();
        if (indexes.size() != 1)
            return;

        auto widget = indexes.first().data(WidgetRole).value<QWidget *>();
        if (!widget)
            return;

        qDebug() << "START PRINT" << widget << "---------------------";
        qDebug() << "  geometry=" << widget->geometry()
                 << "\n  minimumSize=" << widget->minimumSize()
                 << "\n  minimumSizeHint=" << widget->minimumSizeHint()
                 << "\n  maximumSize=" << widget->maximumSize()
                 << "\n  policy=" << widget->sizePolicy()
                 << "\n  visible/enabled=" << widget->isVisible()
                 << widget->isEnabled()
                 << "\n  backgroundRole=" << widget->backgroundRole()
                 << "\n  autoFillBackground=" << widget->autoFillBackground()
                 << "\n  Window=" << widget->palette().color(QPalette::Window)
                 << "\n  Base=" << widget->palette().color(QPalette::Base)
                 << "\n  Button=" << widget->palette().color(QPalette::Button);

        if (auto mw = qobject_cast<QMainWindow *>(widget)) {
            qDebug() << "  centralWidget=" << mw->centralWidget();
        }

        qDebug() << "END PRINT-------------------";
    }

private:
    QLabel m_preview;
    QPixmap m_previewPx;
    QStandardItemModel m_model;
    QTreeView m_tree;
};

DebugWidgetViewer::~DebugWidgetViewer()
{
}

}

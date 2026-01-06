/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "menuview.h"

#include "log.h"
#include "defer.h"

using namespace muse::uicomponents;

static const QString MENU_VIEW_CONTENT_OBJECT_NAME("_MenuViewContent");

// Recursively traverse a flyout tree, collect all "leaves" (items without a sub item)...
static void flattenTreeModel(const QVariant& treeModel, const QString& categoryTitle, QVariantList& result)
{
    for (const QVariant& item : treeModel.toList()) {
        QVariantMap menuItem = item.toMap();
        if (menuItem.empty()) {
            continue;
        }

        const QString title = menuItem.value("title").toString();
        if (title.isEmpty()) {
            continue;
        }

        QVariantList subItems = menuItem.value("subitems").toList();
        if (!subItems.empty()) {
            // Found parent - if it's a "filter category" all child leaves under this item
            // will prepend the title of this item to their titles...
            const bool isFilterCategory = menuItem.value("isFilterCategory").toBool();
            const QString newCategoryTitle = isFilterCategory ? title : categoryTitle;
            flattenTreeModel(subItems, newCategoryTitle, result); // Recursive call...
            continue;
        }

        // Found leaf...
        if (!menuItem.value("includeInFilteredLists").toBool()) {
            continue;
        }

        QString prefix = categoryTitle.isEmpty() ? muse::qtrc("uicomponents", "Unknown") : categoryTitle;
        menuItem.insert("title", prefix + " - " + title);

        result << menuItem;
    }
}

MenuView::MenuView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("MenuView");

    setShowArrow(false);
    setPadding(8);
}

QVariant MenuView::model() const
{
    return m_filterText.isEmpty() ? m_treeModel : m_filteredModel;
}

void MenuView::setModel(const QVariant& model)
{
    if (m_treeModel == model) {
        return;
    }
    m_treeModel = model;

    QVariantList result;
    flattenTreeModel(m_treeModel, QString(), result);
    m_flattenedModel = result;

    emit modelChanged();
}

void MenuView::setFilterText(const QString& filterText)
{
    if (m_filterText == filterText) {
        return;
    }
    m_filterText = filterText;

    QVariantList newModel;
    newModel.reserve(m_flattenedModel.toList().size());

    QString currentPrefix;

    for (const QVariant& item : m_flattenedModel.toList()) {
        QVariantMap itemMap = item.toMap();
        const QString title = itemMap.value("title").toString();
        if (title.contains(m_filterText, Qt::CaseInsensitive)) {
            const QString prefix = title.section("-", 0, 0);
            if (prefix != currentPrefix && !newModel.empty()) {
                newModel << QVariantMap(); // Separate by prefix...
            }
            newModel << itemMap;
            currentPrefix = prefix;
        }
    }

    if (newModel.isEmpty()) {
        QVariantMap item;
        item.insert("checkable", true);
        item.insert("title", muse::qtrc("global", "No results found"));
        newModel << item;
    }

    m_filteredModel = newModel;

    emit modelChanged();
}

int MenuView::viewMargins() const
{
    return 4;
}

Qt::AlignmentFlag MenuView::cascadeAlign() const
{
    return m_cascadeAlign;
}

void MenuView::setCascadeAlign(Qt::AlignmentFlag cascadeAlign)
{
    if (m_cascadeAlign == cascadeAlign) {
        return;
    }

    m_cascadeAlign = cascadeAlign;
    emit cascadeAlignChanged(m_cascadeAlign);
}

void MenuView::componentComplete()
{
    m_contentItem->setObjectName(m_contentItem->objectName() + MENU_VIEW_CONTENT_OBJECT_NAME);

    PopupView::componentComplete();
}

void MenuView::updateGeometry()
{
    const QQuickItem* parent = parentItem();
    IF_ASSERT_FAILED(parent) {
        return;
    }

    QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

    if (m_globalPos.isNull()) {
        m_globalPos = parentTopLeft;
    }

    QRectF anchorRect = anchorGeometry();
    QRectF viewRect = viewGeometry();

    //! NOTE: should be after resolving anchor geometry
    //! because we can move out of the screen
    m_globalPos += m_localPos;

    setPopupPosition(PopupPosition::Bottom);
    setCascadeAlign(Qt::AlignmentFlag::AlignRight);

    auto movePos = [this, &viewRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        viewRect.moveTopLeft(m_globalPos);
    };

    DEFER {
        // remove padding for arrow
        movePos(m_globalPos.x() - padding(), m_globalPos.y());
        updateContentPosition();
    };

    const QQuickItem* parentMenuContentItem = this->parentMenuContentItem();
    bool isCascade = parentMenuContentItem != nullptr;

    if (isCascade) {
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - parent->height() - viewMargins());
    }

    if (placementPolicies().testFlag(PlacementPolicy::IgnoreFit)) {
        return;
    }

    if (viewRect.left() < anchorRect.left()) {
        // move to the right to an area that doesn't fit
        movePos(m_globalPos.x() + anchorRect.left() - viewRect.left(), m_globalPos.y());
    }

    if (viewRect.bottom() > anchorRect.bottom()) {
        if (isCascade) {
            // move to the top to an area that doesn't fit
            movePos(m_globalPos.x(), m_globalPos.y() - (viewRect.bottom() - anchorRect.bottom()));
        } else {
            qreal newY = parentTopLeft.y() - viewRect.height();
            if (anchorRect.top() < newY) {
                // move to the top of the parent
                movePos(m_globalPos.x(), newY);
                setPopupPosition(PopupPosition::Top);
            } else {
                // move to the right of the parent and move to top to an area that doesn't fit
                movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - (viewRect.bottom() - anchorRect.bottom()));
            }
        }
    }

    Qt::AlignmentFlag parentCascadeAlign = this->parentCascadeAlign(parentMenuContentItem);
    if (viewRect.right() > anchorRect.right() || parentCascadeAlign != Qt::AlignmentFlag::AlignRight) {
        if (isCascade) {
            // move to the right of the parent
            movePos(parentTopLeft.x() - viewRect.width() + padding() * 2, m_globalPos.y());
            setCascadeAlign(Qt::AlignmentFlag::AlignLeft);
        } else {
            // move to the left to an area that doesn't fit
            movePos(m_globalPos.x() - (viewRect.right() - anchorRect.right()) + padding() * 2, m_globalPos.y());
        }
    }
}

void MenuView::updateContentPosition()
{
    if (popupPosition() == PopupPosition::Top) {
        contentItem()->setY(padding());
    } else {
        contentItem()->setY(-padding());
    }
}

QRect MenuView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), QSize(contentWidth() + padding() * 2, contentHeight() + padding() * 2));
}

Qt::AlignmentFlag MenuView::parentCascadeAlign(const QQuickItem* parent) const
{
    if (!parent) {
        return Qt::AlignmentFlag::AlignRight;
    }

    return static_cast<Qt::AlignmentFlag>(parent->property("cascadeAlign").toInt());
}

QQuickItem* MenuView::parentMenuContentItem() const
{
    QQuickItem* parent = parentItem();
    while (parent) {
        if (parent->objectName().contains(MENU_VIEW_CONTENT_OBJECT_NAME)) {
            return parent;
        }

        parent = parent->parentItem();
    }

    return nullptr;
}

int MenuView::contentWidth() const
{
    return m_contentWidth;
}

void MenuView::setContentWidth(int newContentWidth)
{
    if (m_contentWidth == newContentWidth) {
        return;
    }

    m_contentWidth = newContentWidth;
    emit contentWidthChanged();
}

int MenuView::contentHeight() const
{
    return m_contentHeight;
}

void MenuView::setContentHeight(int newContentHeight)
{
    if (m_contentHeight == newContentHeight) {
        return;
    }

    m_contentHeight = newContentHeight;
    emit contentHeightChanged();
}

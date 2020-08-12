#include "pagetypelistmodel.h"

PageTypeListModel::PageTypeListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(IdRole, "idRole");
    m_roleNames.insert(NameRole, "nameRole");

    for (int i = 0; i < QPageSize::LastPageSize; ++i) {
        m_pageSizeIdList << i;
    }
}

int PageTypeListModel::rowCount(const QModelIndex&) const
{
    return m_pageSizeIdList.count();
}

QVariant PageTypeListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_pageSizeIdList.isEmpty()) {
        return QVariant();
    }

    int pageSizeId = m_pageSizeIdList.at(index.row());

    switch (role) {
    case IdRole: return pageSizeId;
    case NameRole: return QPageSize::name(static_cast<QPageSize::PageSizeId>(pageSizeId));
    default: return QVariant();
    }
}

QHash<int, QByteArray> PageTypeListModel::roleNames() const
{
    return m_roleNames;
}

int PageTypeListModel::currentPageSizeId() const
{
    return m_currentPageSizeId;
}

void PageTypeListModel::setCurrentPageSizeId(int currentPageSizeId)
{
    if (m_currentPageSizeId == currentPageSizeId) {
        return;
    }

    m_currentPageSizeId = currentPageSizeId;
    emit currentPageSizeIdChanged(m_currentPageSizeId);
}

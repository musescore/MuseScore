#ifndef PAGETYPELISTMODEL_H
#define PAGETYPELISTMODEL_H

#include <QPageSize>
#include <QList>
#include <QHash>
#include <QSizeF>
#include <QAbstractListModel>

class PageTypeListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int currentPageSizeId READ currentPageSizeId WRITE setCurrentPageSizeId NOTIFY currentPageSizeIdChanged)

public:
    enum RoleNames {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    explicit PageTypeListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentPageSizeId() const;

public slots:
    void setCurrentPageSizeId(int currentPageSizeId);

signals:
    void currentPageSizeIdChanged(int currentPageSizeId);

private:
    int m_currentPageSizeId = -1;

    QList<int> m_pageSizeIdList;
    QHash<int, QByteArray> m_roleNames;
};

#endif // PAGETYPELISTMODEL_H

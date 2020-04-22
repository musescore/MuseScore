#ifndef INSPECTORLISTMODEL_H
#define INSPECTORLISTMODEL_H

#include <QAbstractListModel>
#include "libmscore/element.h"
#include "models/abstractinspectormodel.h"
#include "services/elementrepositoryservice.h"

class InspectorListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum RoleNames {
        InspectorDataRole = Qt::UserRole + 1,
        InspectorTitleRole
        };

    explicit InspectorListModel(QObject* parent = nullptr);

    void setElementList(const QList<Ms::Element*>& elementList);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

signals:
    void elementsModified();

private:
    void createModelsByElementType(const Ms::ElementType elementType);

    bool isModelAlreadyExists(const AbstractInspectorModel::InspectorSectionType modelType) const;

    QHash<int, QByteArray> m_roleNames;
    QList<AbstractInspectorModel*> m_modelList;

    IElementRepositoryService* m_repository = nullptr;
    };

#endif // INSPECTORLISTMODEL_H

#include "inspectorlistmodel.h"

#include "general/generalsettingsmodel.h"
#include "notation/notationsettingsproxymodel.h"
#include "text/textsettingsmodel.h"
#include <QSet>

InspectorListModel::InspectorListModel(QObject *parent) : QAbstractListModel(parent)
{
    m_roleNames.insert(InspectorDataRole, "inspectorData");

    m_repository = new ElementRepositoryService(this);
}

void InspectorListModel::setElementList(const QList<Ms::Element*>& elementList)
{

    if (elementList.isEmpty() && !m_modelList.isEmpty()) {

        beginResetModel();

        qDeleteAll(m_modelList);
        m_modelList.clear();

        m_repository->updateElementList(elementList);

        endResetModel();

        return;
    }

    QSet<Ms::ElementType> newElementTypeSet;

    for (const Ms::Element* element : elementList) {
        newElementTypeSet << element->type();
    }

    removeUnusedModels(newElementTypeSet);

    for (const Ms::ElementType elementType : newElementTypeSet) {
        createModelsByElementType(elementType);
    }

    sortModels();

    m_repository->updateElementList(elementList);
}

int InspectorListModel::rowCount(const QModelIndex&) const
{
    return m_modelList.count();
}

QVariant InspectorListModel::data(const QModelIndex &index, int) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_modelList.isEmpty())
        return QVariant();

    AbstractInspectorModel* model = m_modelList.at(index.row());

    QObject* result = qobject_cast<QObject*>(model);

    return QVariant::fromValue(result);
}

QHash<int, QByteArray> InspectorListModel::roleNames() const
{
    return m_roleNames;
}

int InspectorListModel::columnCount(const QModelIndex&) const
{
    return 1;
}

void InspectorListModel::createModelsByElementType(const Ms::ElementType elementType)
{
    using SectionType = AbstractInspectorModel::InspectorSectionType;

    QList<SectionType> sectionTypeList = { SectionType::SECTION_GENERAL };

    sectionTypeList << AbstractInspectorModel::sectionTypeFromElementType(elementType);

    for (const SectionType modelType : sectionTypeList) {

        if (modelType == SectionType::SECTION_UNDEFINED)
            continue;

        if (isModelAlreadyExists(modelType))
            continue;

        beginInsertRows(QModelIndex(), rowCount(), rowCount());

        AbstractInspectorModel* newModel = nullptr;

        switch (modelType) {
        case SectionType::SECTION_GENERAL: newModel = new GeneralSettingsModel(this, m_repository); break;
        case SectionType::SECTION_TEXT: newModel = new TextSettingsModel(this, m_repository); break;
        case SectionType::SECTION_NOTATION: newModel = new NotationSettingsProxyModel(this, m_repository); break;
        default: break;
        }

        if (newModel) {
            connect(newModel, &AbstractInspectorModel::elementsModified, this, &InspectorListModel::elementsModified);
            m_modelList << newModel;
        }

        endInsertRows();
    }
}

void InspectorListModel::removeUnusedModels(const QSet<Ms::ElementType>& newElementTypeSet)
{
    for (int i = 0; i < m_modelList.count(); ++i) {

        AbstractInspectorModel* model = m_modelList.at(i);

        if (model->sectionType() == AbstractInspectorModel::SECTION_GENERAL) {
            continue;
        }

        QSet<Ms::ElementType> supportedElementTypes = AbstractInspectorModel::supportedElementTypesBySectionType(model->sectionType()).toSet();

        supportedElementTypes.intersect(newElementTypeSet);

        if (supportedElementTypes.isEmpty()) {
            beginRemoveRows(QModelIndex(), i, i);

            delete model;
            m_modelList.removeAt(i);

            endRemoveRows();
        }
    }
}

void InspectorListModel::sortModels()
{
    QList<AbstractInspectorModel*> sortedModelList = m_modelList;

    std::sort(sortedModelList.begin(), sortedModelList.end(), [](const AbstractInspectorModel* first, const AbstractInspectorModel* second) -> bool {
        return static_cast<int>(first->sectionType()) < static_cast<int>(second->sectionType());
    });

    for (int i = 0; i < m_modelList.count(); ++i) {

        if (m_modelList.at(i) != sortedModelList.at(i)) {
            beginMoveRows(QModelIndex(), i, i, QModelIndex(), sortedModelList.indexOf(m_modelList.at(i)));
        }
    }

    if (m_modelList == sortedModelList) {
        return;
    }

    m_modelList = sortedModelList;

    endMoveRows();
}

bool InspectorListModel::isModelAlreadyExists(const AbstractInspectorModel::InspectorSectionType modelType) const
{
    for (const AbstractInspectorModel* model : m_modelList) {
        if (model->sectionType() == modelType)
            return true;
    }

    return false;
}


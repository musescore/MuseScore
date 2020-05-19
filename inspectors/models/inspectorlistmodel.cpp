#include "inspectorlistmodel.h"

#include "general/generalsettingsmodel.h"
#include "notation/notationsettingsproxymodel.h"
#include "text/textsettingsmodel.h"

InspectorListModel::InspectorListModel(QObject *parent) : QAbstractListModel(parent)
{
    m_roleNames.insert(InspectorDataRole, "inspectorData");

    m_repository = new ElementRepositoryService(this);
}

void InspectorListModel::setElementList(const QList<Ms::Element*>& elementList)
{

    if (elementList.isEmpty() && !m_modelList.isEmpty()) {

        beginResetModel();

        m_repository->updateElementList(elementList);

        qDeleteAll(m_modelList);
        m_modelList.clear();

        endResetModel();

        return;
    }

    for (const Ms::Element* element : elementList) {
        createModelsByElementType(element->type());
    }

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

    sectionTypeList << AbstractInspectorModel::modelTypeFromElementType(elementType);

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

bool InspectorListModel::isModelAlreadyExists(const AbstractInspectorModel::InspectorSectionType modelType) const
{
    for (const AbstractInspectorModel* model : m_modelList) {
        if (model->sectionType() == modelType)
            return true;
    }

    return false;
}

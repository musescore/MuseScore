#include "inspectorlistmodel.h"

#include "general/generalsettingsmodel.h"
#include "notation/notationsettingsproxymodel.h"
#include "text/textsettingsmodel.h"
#include "score/scoredisplaysettingsmodel.h"
#include "score/scoreappearancesettingsmodel.h"
#include "notation/inotationinteraction.h"

using namespace mu::inspector;
using namespace mu::notation;

InspectorListModel::InspectorListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(InspectorDataRole, "inspectorData");

    m_repository = new ElementRepositoryService(this);

    subscribeOnSelectionChanges();
}

void InspectorListModel::buildModelsForSelectedElements(const QSet<Ms::ElementType>& selectedElementSet)
{
    static QList<AbstractInspectorModel::InspectorSectionType> persistentSectionList
        = { AbstractInspectorModel::InspectorSectionType::SECTION_GENERAL };

    removeUnusedModels(selectedElementSet, persistentSectionList);

    QList<AbstractInspectorModel::InspectorSectionType> buildingSectionTypeList(persistentSectionList);

    for (const Ms::ElementType elementType : selectedElementSet) {
        buildingSectionTypeList << AbstractInspectorModel::sectionTypeFromElementType(elementType);
    }

    createModelsBySectionType(buildingSectionTypeList);

    sortModels();
}

void InspectorListModel::buildModelsForEmptySelection(const QSet<Ms::ElementType>& selectedElementSet)
{
    static QList<AbstractInspectorModel::InspectorSectionType> persistentSectionList {
        AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_DISPLAY,
        AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_APPEARANCE
    };

    removeUnusedModels(selectedElementSet, persistentSectionList);

    createModelsBySectionType(persistentSectionList);
}

void InspectorListModel::setElementList(const QList<Ms::Element*>& selectedElementList)
{
    QSet<Ms::ElementType> newElementTypeSet;

    for (const Ms::Element* element : selectedElementList) {
        newElementTypeSet << element->type();
    }

    if (selectedElementList.isEmpty()) {
        buildModelsForEmptySelection(newElementTypeSet);
    } else {
        buildModelsForSelectedElements(newElementTypeSet);
    }

    m_repository->updateElementList(selectedElementList);
}

int InspectorListModel::rowCount(const QModelIndex&) const
{
    return m_modelList.count();
}

QVariant InspectorListModel::data(const QModelIndex& index, int) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_modelList.isEmpty()) {
        return QVariant();
    }

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

void InspectorListModel::createModelsBySectionType(const QList<AbstractInspectorModel::InspectorSectionType>& sectionTypeList)
{
    using SectionType = AbstractInspectorModel::InspectorSectionType;

    for (const SectionType modelType : sectionTypeList) {
        if (modelType == SectionType::SECTION_UNDEFINED) {
            continue;
        }

        if (isModelAlreadyExists(modelType)) {
            continue;
        }

        beginInsertRows(QModelIndex(), rowCount(), rowCount());

        switch (modelType) {
        case AbstractInspectorModel::InspectorSectionType::SECTION_GENERAL:
            m_modelList << new GeneralSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_TEXT:
            m_modelList << new TextSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_NOTATION:
            m_modelList << new NotationSettingsProxyModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_DISPLAY:
            m_modelList << new ScoreSettingsModel(this, m_repository);
            break;
        case AbstractInspectorModel::InspectorSectionType::SECTION_SCORE_APPEARANCE:
            m_modelList << new ScoreAppearanceSettingsModel(this, m_repository);
            break;
        default:
            break;
        }

        endInsertRows();
    }
}

void InspectorListModel::removeUnusedModels(const QSet<Ms::ElementType>& newElementTypeSet,
                                            const QList<AbstractInspectorModel::InspectorSectionType>& exclusions)
{
    QList<AbstractInspectorModel*> modelsToRemove;

    for (AbstractInspectorModel* model : m_modelList) {
        if (exclusions.contains(model->sectionType())) {
            continue;
        }

        auto supportedElementTypes = AbstractInspectorModel::supportedElementTypesBySectionType(model->sectionType());
        QSet<Ms::ElementType> supportedElementTypesSet(supportedElementTypes.begin(), supportedElementTypes.end());
        supportedElementTypesSet.intersect(newElementTypeSet);

        if (supportedElementTypesSet.isEmpty()) {
            modelsToRemove << model;
        }
    }

    for (AbstractInspectorModel* model : modelsToRemove) {
        int index = m_modelList.indexOf(model);

        beginRemoveRows(QModelIndex(), index, index);

        delete model;
        m_modelList.removeAt(index);

        endRemoveRows();
    }
}

void InspectorListModel::sortModels()
{
    QList<AbstractInspectorModel*> sortedModelList = m_modelList;

    std::sort(sortedModelList.begin(), sortedModelList.end(), [](const AbstractInspectorModel* first,
                                                                 const AbstractInspectorModel* second) -> bool {
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
        if (model->sectionType() == modelType) {
            return true;
        }
    }

    return false;
}

void InspectorListModel::subscribeOnSelectionChanges()
{
    if (!context() || !context()->currentNotation()) {
        setElementList(QList<Ms::Element*>());
    }

    context()->currentNotationChanged().onNotify(this, [this]() {
        m_notation = context()->currentNotation();

        if (!m_notation) {
            setElementList(QList<Ms::Element*>());
            return;
        }

        auto elements = m_notation->interaction()->selection()->elements();
        setElementList(QList(elements.cbegin(), elements.cend()));

        m_notation->interaction()->selectionChanged().onNotify(this, [this]() {
            auto elements = m_notation->interaction()->selection()->elements();
            setElementList(QList(elements.cbegin(), elements.cend()));
        });
    });
}

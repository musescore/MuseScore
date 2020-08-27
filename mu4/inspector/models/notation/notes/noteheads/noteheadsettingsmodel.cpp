#include "noteheadsettingsmodel.h"

#include "note.h"
#include "dataformatter.h"

NoteheadSettingsModel::NoteheadSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Head"));
    setModelType(TYPE_NOTEHEAD);

    createProperties();

    setNoteheadTypesModel(new NoteheadTypesModel(this));
}

void NoteheadSettingsModel::createProperties()
{
    m_isHeadHidden = buildPropertyItem(Ms::Pid::VISIBLE, [this](const int pid, const QVariant& isHeadHidden) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), !isHeadHidden.toBool());
    });

    m_headDirection = buildPropertyItem(Ms::Pid::MIRROR_HEAD);
    m_headGroup = buildPropertyItem(Ms::Pid::HEAD_GROUP);
    m_headType = buildPropertyItem(Ms::Pid::HEAD_TYPE);
    m_dotPosition = buildPropertyItem(Ms::Pid::DOT_POSITION);

    m_horizontalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void NoteheadSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::NOTE);
}

void NoteheadSettingsModel::loadProperties()
{
    loadPropertyItem(m_isHeadHidden, [this](const QVariant& isVisible) -> QVariant {
        return !isVisible.toBool();
    });

    loadPropertyItem(m_headDirection);
    loadPropertyItem(m_headGroup);
    loadPropertyItem(m_headType);
    loadPropertyItem(m_dotPosition);

    loadPropertyItem(m_horizontalOffset, [this](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    loadPropertyItem(m_verticalOffset, [this](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });
}

void NoteheadSettingsModel::resetProperties()
{
    m_isHeadHidden->resetToDefault();
    m_headDirection->resetToDefault();
    m_headGroup->resetToDefault();
    m_headType->resetToDefault();
    m_dotPosition->resetToDefault();

    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

QObject* NoteheadSettingsModel::noteheadTypesModel() const
{
    return m_noteheadTypesModel;
}

PropertyItem* NoteheadSettingsModel::isHeadHidden() const
{
    return m_isHeadHidden;
}

PropertyItem* NoteheadSettingsModel::headDirection() const
{
    return m_headDirection;
}

PropertyItem* NoteheadSettingsModel::headGroup() const
{
    return m_headGroup;
}

PropertyItem* NoteheadSettingsModel::headType() const
{
    return m_headType;
}

PropertyItem* NoteheadSettingsModel::dotPosition() const
{
    return m_dotPosition;
}

PropertyItem* NoteheadSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* NoteheadSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}

void NoteheadSettingsModel::setNoteheadTypesModel(NoteheadTypesModel* noteheadTypesModel)
{
    if (m_noteheadTypesModel == noteheadTypesModel) {
        return;
    }

    m_noteheadTypesModel = noteheadTypesModel;

    connect(m_noteheadTypesModel, &NoteheadTypesModel::noteHeadGroupSelected, [this](const int noteHeadGroup) {
        m_headGroup->setValue(noteHeadGroup);
    });

    connect(m_headGroup, &PropertyItem::valueChanged, [this](const QVariant noteHeadGroup) {
        if (m_headGroup->isUndefined()) {
            m_noteheadTypesModel->init(Ms::NoteHead::Group::HEAD_INVALID);
        } else {
            m_noteheadTypesModel->init(static_cast<Ms::NoteHead::Group>(noteHeadGroup.toInt()));
        }
    });

    emit noteheadTypesModelChanged(m_noteheadTypesModel);
}

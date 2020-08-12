#include "beammodesmodel.h"

BeamModesModel::BeamModesModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    createProperties();
}

void BeamModesModel::createProperties()
{
    m_mode = buildPropertyItem(Ms::Pid::BEAM_MODE);
    m_isFeatheringAvailable = buildPropertyItem(Ms::Pid::DURATION_TYPE, [](const int, const QVariant&) {}); //@note readonly property, there is no need to modify it

    setModeListModel(new BeamModeListModel(this));
}

void BeamModesModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::CHORD);
}

void BeamModesModel::loadProperties()
{
    loadPropertyItem(m_mode);

    loadPropertyItem(m_isFeatheringAvailable, [this](const QVariant& elementPropertyValue) -> QVariant {
        Ms::TDuration durationType = elementPropertyValue.value<Ms::TDuration>();

        switch (durationType.type()) {
        case Ms::TDuration::DurationType::V_INVALID:
        case Ms::TDuration::DurationType::V_MEASURE:
        case Ms::TDuration::DurationType::V_ZERO:
        case Ms::TDuration::DurationType::V_LONG:
        case Ms::TDuration::DurationType::V_BREVE:
        case Ms::TDuration::DurationType::V_WHOLE:
        case Ms::TDuration::DurationType::V_HALF:
        case Ms::TDuration::DurationType::V_QUARTER:
        case Ms::TDuration::DurationType::V_EIGHTH:
            return false;

        default:
            return true;
        }
    });
}

void BeamModesModel::resetProperties()
{
    m_mode->resetToDefault();
}

PropertyItem* BeamModesModel::mode() const
{
    return m_mode;
}

PropertyItem* BeamModesModel::isFeatheringAvailable() const
{
    return m_isFeatheringAvailable;
}

QObject* BeamModesModel::modeListModel() const
{
    return m_modeListModel;
}

void BeamModesModel::setModeListModel(BeamModeListModel* modeListModel)
{
    m_modeListModel = modeListModel;

    connect(m_modeListModel, &BeamModeListModel::beamModeSelected, [this](const BeamTypes::Mode beamMode) {
        m_mode->setValue(static_cast<int>(beamMode));
    });

    connect(m_mode, &PropertyItem::valueChanged, [this](const QVariant& beamMode) {
        if (m_mode->isUndefined()) {
            m_modeListModel->setSelectedBeamMode(BeamTypes::Mode::MODE_INVALID);
        } else {
            m_modeListModel->setSelectedBeamMode(static_cast<BeamTypes::Mode>(beamMode.toInt()));
        }
    });

    emit modeListModelChanged(m_modeListModel);
}

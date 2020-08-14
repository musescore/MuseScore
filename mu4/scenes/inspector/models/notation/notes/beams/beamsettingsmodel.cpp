#include "beamsettingsmodel.h"

#include "dataformatter.h"

BeamSettingsModel::BeamSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_BEAM);
    setTitle(tr("Beam"));
    setBeamModesModel(new BeamModesModel(this, repository));

    createProperties();
}

void BeamSettingsModel::createProperties()
{
    m_featheringHeightLeft = buildPropertyItem(Ms::Pid::GROW_LEFT);
    m_featheringHeightRight = buildPropertyItem(Ms::Pid::GROW_RIGHT, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);
    });

    m_isBeamHidden = buildPropertyItem(Ms::Pid::VISIBLE, [this](const int pid, const QVariant& isBeamHidden) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), !isBeamHidden.toBool());
    });

    m_beamVectorX = buildPropertyItem(Ms::Pid::BEAM_POS, [this](const int, const QVariant& newValue) {
        updateBeamHeight(newValue.toDouble(), m_beamVectorY->value().toDouble());
    });

    m_beamVectorY = buildPropertyItem(Ms::Pid::BEAM_POS, [this](const int, const QVariant& newValue) {
        updateBeamHeight(m_beamVectorX->value().toDouble(), newValue.toDouble());
    });
}

void BeamSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::BEAM);
}

void BeamSettingsModel::loadProperties()
{
    loadPropertyItem(m_featheringHeightLeft, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_featheringHeightRight, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    });

    loadPropertyItem(m_isBeamHidden, [](const QVariant& isVisible) -> QVariant {
        return !isVisible.toBool();
    });

    loadPropertyItem(m_beamVectorX, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    loadPropertyItem(m_beamVectorY, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });

    m_cachedBeamVector.setX(m_beamVectorX->value().toDouble());
    m_cachedBeamVector.setY(m_beamVectorY->value().toDouble());

    updateFeatheringMode(m_featheringHeightLeft->value().toDouble(), m_featheringHeightRight->value().toDouble());
}

void BeamSettingsModel::resetProperties()
{
    m_featheringHeightLeft->resetToDefault();
    m_featheringHeightRight->resetToDefault();
    m_beamVectorX->resetToDefault();
    m_beamVectorY->resetToDefault();

    m_cachedBeamVector = QPointF();

    setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
    setIsBeamHeightLocked(false);
}

void BeamSettingsModel::forceHorizontal()
{
    onPropertyValueChanged(Ms::Pid::BEAM_NO_SLOPE, true);

    emit requestReloadPropertyItems();
}

void BeamSettingsModel::updateBeamHeight(const qreal& x, const qreal& y)
{
    if (m_isBeamHeightLocked) {
        synchronizeLockedBeamHeight(x, y);
    }

    onPropertyValueChanged(Ms::Pid::USER_MODIFIED, true); // TODO проверить, можно ли это перенести в DOM модель
    onPropertyValueChanged(Ms::Pid::BEAM_POS, QPointF(m_beamVectorX->value().toDouble(), m_beamVectorY->value().toDouble()));

    m_cachedBeamVector.setX(m_beamVectorX->value().toDouble());
    m_cachedBeamVector.setY(m_beamVectorY->value().toDouble());
}

void BeamSettingsModel::synchronizeLockedBeamHeight(const qreal& currentX, const qreal& currentY)
{
    qreal deltaX = currentX - m_cachedBeamVector.x();
    qreal deltaY = currentY - m_cachedBeamVector.y();

    qreal maxDelta = qMax(qAbs(deltaX), qAbs(deltaY));

    if (qFuzzyCompare(qAbs(deltaX), maxDelta)) {
        m_beamVectorY->updateCurrentValue(m_cachedBeamVector.y() + deltaX);
    } else {
        m_beamVectorX->updateCurrentValue(m_cachedBeamVector.x() + deltaY);
    }
}

void BeamSettingsModel::updateFeatheringMode(const qreal& x, const qreal& y)
{
    if (x != y) {
        setFeatheringMode(x > y ? BeamTypes::FeatheringMode::FEATHERING_LEFT
                          : BeamTypes::FeatheringMode::FEATHERING_RIGHT);
    } else {
        setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
    }
}

bool BeamSettingsModel::isBeamHeightLocked() const
{
    return m_isBeamHeightLocked;
}

QObject* BeamSettingsModel::beamModesModel() const
{
    return m_beamModesModel;
}

PropertyItem* BeamSettingsModel::featheringHeightLeft() const
{
    return m_featheringHeightLeft;
}

PropertyItem* BeamSettingsModel::featheringHeightRight() const
{
    return m_featheringHeightRight;
}

BeamTypes::FeatheringMode BeamSettingsModel::featheringMode() const
{
    return m_featheringMode;
}

PropertyItem* BeamSettingsModel::isBeamHidden() const
{
    return m_isBeamHidden;
}

PropertyItem* BeamSettingsModel::beamVectorX() const
{
    return m_beamVectorX;
}

PropertyItem* BeamSettingsModel::beamVectorY() const
{
    return m_beamVectorY;
}

void BeamSettingsModel::setIsBeamHeightLocked(bool isBeamHeightLocked)
{
    if (m_isBeamHeightLocked == isBeamHeightLocked) {
        return;
    }

    m_isBeamHeightLocked = isBeamHeightLocked;
    emit isBeamHeightLockedChanged(m_isBeamHeightLocked);
}

void BeamSettingsModel::setFeatheringMode(BeamTypes::FeatheringMode featheringMode)
{
    if (m_featheringMode == featheringMode) {
        return;
    }

    m_featheringMode = featheringMode;

    switch (featheringMode) {
    case BeamTypes::FeatheringMode::FEATHERING_NONE:
        m_featheringHeightLeft->setValue(1.0);
        m_featheringHeightRight->setValue(1.0);
        break;
    case BeamTypes::FeatheringMode::FEATHERING_LEFT:
        m_featheringHeightLeft->setValue(1.0);
        m_featheringHeightRight->setValue(0.0);
        break;
    case BeamTypes::FeatheringMode::FEATHERING_RIGHT:
        m_featheringHeightLeft->setValue(0.0);
        m_featheringHeightRight->setValue(1.0);
        break;
    }

    emit featheringModeChanged(featheringMode);
}

void BeamSettingsModel::setBeamModesModel(BeamModesModel* beamModesModel)
{
    m_beamModesModel = beamModesModel;

    connect(m_beamModesModel->isFeatheringAvailable(), &PropertyItem::propertyModified, this, [this](const QVariant& newValue) {
        if (!newValue.toBool()) {
            setFeatheringMode(BeamTypes::FeatheringMode::FEATHERING_NONE);
        }
    });

    connect(m_beamModesModel->mode(), &PropertyItem::propertyModified, this, &AbstractInspectorModel::requestReloadPropertyItems);

    emit beamModesModelChanged(m_beamModesModel);
}

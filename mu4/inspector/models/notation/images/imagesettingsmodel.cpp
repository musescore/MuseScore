#include "imagesettingsmodel.h"

#include <QSizeF>

#include "dataformatter.h"

ImageSettingsModel::ImageSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_IMAGE);
    setTitle(tr("Image"));
    createProperties();
}

void ImageSettingsModel::createProperties()
{
    m_isAspectRatioLocked = buildPropertyItem(Ms::Pid::LOCK_ASPECT_RATIO);

    m_shouldScaleToFrameSize = buildPropertyItem(Ms::Pid::AUTOSCALE, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        emit requestReloadPropertyItems();
    });

    m_height = buildPropertyItem(Ms::Pid::IMAGE_HEIGHT, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        if (m_isAspectRatioLocked->value().toBool()) {
            emit requestReloadPropertyItems();
        }
    });

    m_width = buildPropertyItem(Ms::Pid::IMAGE_WIDTH, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        if (m_isAspectRatioLocked->value().toBool()) {
            emit requestReloadPropertyItems();
        }
    });

    m_isSizeInSpatiums = buildPropertyItem(Ms::Pid::SIZE_IS_SPATIUM, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        emit requestReloadPropertyItems();
    });

    m_isImageFramed = buildPropertyItem(Ms::Pid::IMAGE_FRAMED);
}

void ImageSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::IMAGE);
}

void ImageSettingsModel::loadProperties()
{
    auto formatDoubleFunc = [](const QVariant& elementPropertyValue) -> QVariant {
                                return DataFormatter::formatDouble(elementPropertyValue.toDouble());
                            };

    loadPropertyItem(m_shouldScaleToFrameSize);
    loadPropertyItem(m_height, formatDoubleFunc);
    loadPropertyItem(m_width, formatDoubleFunc);

    loadPropertyItem(m_isAspectRatioLocked);
    loadPropertyItem(m_isSizeInSpatiums);
    loadPropertyItem(m_isImageFramed);

    updateFrameScalingAvailability();
}

void ImageSettingsModel::resetProperties()
{
    m_shouldScaleToFrameSize->resetToDefault();
    m_height->resetToDefault();
    m_width->resetToDefault();
    m_isAspectRatioLocked->resetToDefault();
    m_isSizeInSpatiums->resetToDefault();
    m_isImageFramed->resetToDefault();
}

PropertyItem* ImageSettingsModel::shouldScaleToFrameSize() const
{
    return m_shouldScaleToFrameSize;
}

PropertyItem* ImageSettingsModel::height() const
{
    return m_height;
}

PropertyItem* ImageSettingsModel::width() const
{
    return m_width;
}

PropertyItem* ImageSettingsModel::isAspectRatioLocked() const
{
    return m_isAspectRatioLocked;
}

PropertyItem* ImageSettingsModel::isSizeInSpatiums() const
{
    return m_isSizeInSpatiums;
}

PropertyItem* ImageSettingsModel::isImageFramed() const
{
    return m_isImageFramed;
}

void ImageSettingsModel::updateFrameScalingAvailability()
{
    bool isAvailable = m_isImageFramed->value().toBool();

    m_shouldScaleToFrameSize->setIsEnabled(isAvailable);
}

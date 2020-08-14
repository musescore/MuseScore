#ifndef IMAGESETTINGSMODEL_H
#define IMAGESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class ImageSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * shouldScaleToFrameSize READ shouldScaleToFrameSize CONSTANT)
    Q_PROPERTY(PropertyItem * height READ height CONSTANT)
    Q_PROPERTY(PropertyItem * width READ width CONSTANT)
    Q_PROPERTY(PropertyItem * isAspectRatioLocked READ isAspectRatioLocked CONSTANT)
    Q_PROPERTY(PropertyItem * isSizeInSpatiums READ isSizeInSpatiums CONSTANT)
    Q_PROPERTY(PropertyItem * isImageFramed READ isImageFramed CONSTANT)

public:
    explicit ImageSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* shouldScaleToFrameSize() const;
    PropertyItem* height() const;
    PropertyItem* width() const;
    PropertyItem* isAspectRatioLocked() const;
    PropertyItem* isSizeInSpatiums() const;
    PropertyItem* isImageFramed() const;

private:
    void updateFrameScalingAvailability();

    PropertyItem* m_shouldScaleToFrameSize = nullptr;
    PropertyItem* m_height = nullptr;
    PropertyItem* m_width = nullptr;
    PropertyItem* m_isAspectRatioLocked = nullptr;
    PropertyItem* m_isSizeInSpatiums = nullptr;
    PropertyItem* m_isImageFramed = nullptr;
};

#endif // IMAGESETTINGSMODEL_H

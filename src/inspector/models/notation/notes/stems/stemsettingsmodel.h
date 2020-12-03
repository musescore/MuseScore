#ifndef NOTATIONINSPECTORMODEL_H
#define NOTATIONINSPECTORMODEL_H

#include "models/abstractinspectormodel.h"

class StemSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isStemHidden READ isStemHidden CONSTANT)
    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * length READ length CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalOffset READ horizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * stemDirection READ stemDirection CONSTANT)
public:
    explicit StemSettingsModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* isStemHidden() const;
    PropertyItem* thickness() const;
    PropertyItem* length() const;

    PropertyItem* horizontalOffset() const;
    PropertyItem* verticalOffset() const;
    PropertyItem* stemDirection() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_isStemHidden = nullptr;
    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_length = nullptr;
    PropertyItem* m_horizontalOffset = nullptr;
    PropertyItem* m_verticalOffset = nullptr;
    PropertyItem* m_stemDirection = nullptr;
};

#endif // NOTATIONINSPECTORMODEL_H

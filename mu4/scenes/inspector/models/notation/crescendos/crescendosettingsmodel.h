#ifndef CRESCENDOSETTINGSMODEL_H
#define CRESCENDOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class CrescendoSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isLineVisible READ isLineVisible CONSTANT)
    Q_PROPERTY(PropertyItem * endHookType READ endHookType CONSTANT)
    Q_PROPERTY(PropertyItem * thickness READ thickness CONSTANT)
    Q_PROPERTY(PropertyItem * hookHeight READ hookHeight CONSTANT)
    Q_PROPERTY(PropertyItem * lineStyle READ lineStyle CONSTANT)
    Q_PROPERTY(PropertyItem * dashLineLength READ dashLineLength CONSTANT)
    Q_PROPERTY(PropertyItem * dashGapLength READ dashGapLength CONSTANT)
    Q_PROPERTY(PropertyItem * placement READ placement CONSTANT)

    Q_PROPERTY(PropertyItem * beginningText READ beginningText CONSTANT)
    Q_PROPERTY(PropertyItem * beginningTextHorizontalOffset READ beginningTextHorizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * beginningTextVerticalOffset READ beginningTextVerticalOffset CONSTANT)

    Q_PROPERTY(PropertyItem * continiousText READ continiousText CONSTANT)
    Q_PROPERTY(PropertyItem * continiousTextHorizontalOffset READ continiousTextHorizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * continiousTextVerticalOffset READ continiousTextVerticalOffset CONSTANT)

public:
    explicit CrescendoSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isLineVisible() const;
    PropertyItem* endHookType() const;
    PropertyItem* thickness() const;
    PropertyItem* hookHeight() const;
    PropertyItem* lineStyle() const;
    PropertyItem* dashLineLength() const;
    PropertyItem* dashGapLength() const;
    PropertyItem* placement() const;

    PropertyItem* beginningText() const;
    PropertyItem* beginningTextHorizontalOffset() const;
    PropertyItem* beginningTextVerticalOffset() const;

    PropertyItem* continiousText() const;
    PropertyItem* continiousTextHorizontalOffset() const;
    PropertyItem* continiousTextVerticalOffset() const;

private:
    void updateLinePropertiesAvailability();

    PropertyItem* m_isLineVisible = nullptr;
    PropertyItem* m_endHookType = nullptr;
    PropertyItem* m_thickness = nullptr;
    PropertyItem* m_hookHeight = nullptr;
    PropertyItem* m_lineStyle = nullptr;
    PropertyItem* m_dashLineLength = nullptr;
    PropertyItem* m_dashGapLength = nullptr;
    PropertyItem* m_placement = nullptr;

    PropertyItem* m_beginningText = nullptr;
    PropertyItem* m_beginningTextHorizontalOffset = nullptr;
    PropertyItem* m_beginningTextVerticalOffset = nullptr;

    PropertyItem* m_continiousText = nullptr;
    PropertyItem* m_continiousTextHorizontalOffset = nullptr;
    PropertyItem* m_continiousTextVerticalOffset = nullptr;
};

#endif // CRESCENDOSETTINGSMODEL_H

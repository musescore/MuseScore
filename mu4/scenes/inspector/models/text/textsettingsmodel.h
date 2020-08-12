#ifndef TEXTSETTINGSMODEL_H
#define TEXTSETTINGSMODEL_H

#include "async/asyncable.h"
#include "models/abstractinspectormodel.h"

class TextSettingsModel : public AbstractInspectorModel, public mu::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(PropertyItem * fontStyle READ fontStyle CONSTANT)
    Q_PROPERTY(PropertyItem * fontSize READ fontSize CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalAlignment READ horizontalAlignment CONSTANT)
    Q_PROPERTY(PropertyItem * verticalAlignment READ verticalAlignment CONSTANT)

    Q_PROPERTY(PropertyItem * isSizeSpatiumDependent READ isSizeSpatiumDependent CONSTANT)
    Q_PROPERTY(PropertyItem * frameType READ frameType CONSTANT)
    Q_PROPERTY(PropertyItem * frameBorderColor READ frameBorderColor CONSTANT)
    Q_PROPERTY(PropertyItem * frameHighlightColor READ frameHighlightColor CONSTANT)
    Q_PROPERTY(PropertyItem * frameThickness READ frameThickness CONSTANT)
    Q_PROPERTY(PropertyItem * frameMargin READ frameMargin CONSTANT)
    Q_PROPERTY(PropertyItem * frameCornerRadius READ frameCornerRadius CONSTANT)

    Q_PROPERTY(PropertyItem * textType READ textType CONSTANT)
    Q_PROPERTY(PropertyItem * textPlacement READ textPlacement CONSTANT)
    Q_PROPERTY(PropertyItem * textScriptAlignment READ textScriptAlignment CONSTANT)

    Q_PROPERTY(bool areStaffTextPropertiesAvailable READ areStaffTextPropertiesAvailable NOTIFY areStaffTextPropertiesAvailableChanged)
    Q_PROPERTY(
        bool isSpecialCharactersInsertionAvailable READ isSpecialCharactersInsertionAvailable NOTIFY isSpecialCharactersInsertionAvailableChanged)

public:
    explicit TextSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void insertSpecialCharacters();
    Q_INVOKABLE void showStaffTextProperties();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* fontFamily() const;
    PropertyItem* fontStyle() const;
    PropertyItem* fontSize() const;
    PropertyItem* horizontalAlignment() const;
    PropertyItem* verticalAlignment() const;

    PropertyItem* isSizeSpatiumDependent() const;
    PropertyItem* frameType() const;
    PropertyItem* frameBorderColor() const;
    PropertyItem* frameHighlightColor() const;
    PropertyItem* frameThickness() const;
    PropertyItem* frameMargin() const;
    PropertyItem* frameCornerRadius() const;

    PropertyItem* textType() const;
    PropertyItem* textPlacement() const;
    PropertyItem* textScriptAlignment() const;

    bool areStaffTextPropertiesAvailable() const;

    bool isSpecialCharactersInsertionAvailable() const;

public slots:
    void setAreStaffTextPropertiesAvailable(bool areStaffTextPropertiesAvailable);
    void setIsSpecialCharactersInsertionAvailable(bool isSpecialCharactersInsertionAvailable);

signals:
    void areStaffTextPropertiesAvailableChanged(bool areStaffTextPropertiesAvailable);
    void isSpecialCharactersInsertionAvailableChanged(bool isSpecialCharactersInsertionAvailable);

private:
    void updateFramePropertiesAvailability();
    void updateStaffPropertiesAvailability();

    PropertyItem* m_fontFamily = nullptr;
    PropertyItem* m_fontStyle = nullptr;
    PropertyItem* m_fontSize = nullptr;
    PropertyItem* m_horizontalAlignment = nullptr;
    PropertyItem* m_verticalAlignment = nullptr;

    PropertyItem* m_isSizeSpatiumDependent = nullptr;
    PropertyItem* m_frameType = nullptr;
    PropertyItem* m_frameBorderColor = nullptr;
    PropertyItem* m_frameHighlightColor = nullptr;
    PropertyItem* m_frameThickness = nullptr;
    PropertyItem* m_frameMargin = nullptr;
    PropertyItem* m_frameCornerRadius = nullptr;

    PropertyItem* m_textType = nullptr;
    PropertyItem* m_textPlacement = nullptr;
    PropertyItem* m_textScriptAlignment = nullptr;

    bool m_areStaffTextPropertiesAvailable = false;
    bool m_isSpecialCharactersInsertionAvailable = false;
};

#endif // TEXTSETTINGSMODEL_H

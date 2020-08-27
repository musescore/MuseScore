#ifndef APPEARANCESETTINGSMODEL_H
#define APPEARANCESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class AppearanceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * leadingSpace READ leadingSpace CONSTANT)
    Q_PROPERTY(PropertyItem * barWidth READ barWidth CONSTANT)
    Q_PROPERTY(PropertyItem * minimumDistance READ minimumDistance CONSTANT)
    Q_PROPERTY(PropertyItem * color READ color CONSTANT)
    Q_PROPERTY(PropertyItem * arrangeOrder READ arrangeOrder CONSTANT)
    Q_PROPERTY(PropertyItem * horizontalOffset READ horizontalOffset CONSTANT)
    Q_PROPERTY(PropertyItem * verticalOffset READ verticalOffset CONSTANT)
    Q_PROPERTY(bool isSnappedToGrid READ isSnappedToGrid WRITE setIsSnappedToGrid NOTIFY isSnappedToGridChanged)

public:
    explicit AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    Q_INVOKABLE void pushBackInOrder();
    Q_INVOKABLE void pushFrontInOrder();

    Q_INVOKABLE void configureGrid();

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* leadingSpace() const;
    PropertyItem* barWidth() const;
    PropertyItem* minimumDistance() const;
    PropertyItem* color() const;
    PropertyItem* arrangeOrder() const;
    PropertyItem* horizontalOffset() const;
    PropertyItem* verticalOffset() const;

    bool isSnappedToGrid() const;

public slots:
    void setIsSnappedToGrid(bool isSnapped);

signals:
    void isSnappedToGridChanged(bool isSnappedToGrid);

private:
    PropertyItem* m_leadingSpace = nullptr;
    PropertyItem* m_barWidth = nullptr;
    PropertyItem* m_minimumDistance = nullptr;
    PropertyItem* m_color = nullptr;
    PropertyItem* m_arrangeOrder = nullptr;
    PropertyItem* m_horizontalOffset = nullptr;
    PropertyItem* m_verticalOffset = nullptr;

    bool m_verticallySnapToGrid = false;
    bool m_horizontallySnapToGrid = false;
};

#endif // APPEARANCESETTINGSMODEL_H

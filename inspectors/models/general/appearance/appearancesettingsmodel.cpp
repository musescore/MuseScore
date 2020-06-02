#include "appearancesettingsmodel.h"

#include "utils/dataformatter.h"

static const int REARRANGE_ORDER_STEP = 100;

AppearanceSettingsModel::AppearanceSettingsModel(QObject* parent, IElementRepositoryService* repository) : AbstractInspectorModel(parent, repository)
{
    createProperties();

    m_horizontallySnapToGridAction = Ms::Shortcut::getActionByName("hraster");
    m_verticallySnapToGridAction = Ms::Shortcut::getActionByName("vraster");
    m_horizontallySnapToGridAction->setCheckable(true);
    m_verticallySnapToGridAction->setCheckable(true);

    setTitle(tr("Appearance"));
}

void AppearanceSettingsModel::createProperties()
{
    m_leadingSpace = buildPropertyItem(Ms::Pid::LEADING_SPACE);
    m_barWidth = buildPropertyItem(Ms::Pid::USER_STRETCH);
    m_minimumDistance = buildPropertyItem(Ms::Pid::MIN_DISTANCE);
    m_color = buildPropertyItem(Ms::Pid::COLOR);
    m_arrangeOrder = buildPropertyItem(Ms::Pid::Z);

    m_horizontalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this] (const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_verticalOffset->value().toDouble()));
    });

    m_verticalOffset = buildPropertyItem(Ms::Pid::OFFSET, [this] (const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_horizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void AppearanceSettingsModel::requestElements()
{
    m_elementList = m_repository->takeAllElements();
}

void AppearanceSettingsModel::loadProperties()
{
    auto formatDoubleFunc = [] (const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble());
    };

    loadPropertyItem(m_leadingSpace, formatDoubleFunc);
    loadPropertyItem(m_minimumDistance, formatDoubleFunc);

    loadPropertyItem(m_barWidth);
    loadPropertyItem(m_color);
    loadPropertyItem(m_arrangeOrder);

    loadPropertyItem(m_horizontalOffset, [this] (const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().x());
    });

    loadPropertyItem(m_verticalOffset, [this] (const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toPointF().y());
    });
}

void AppearanceSettingsModel::resetProperties()
{
    m_leadingSpace->resetToDefault();
    m_minimumDistance->resetToDefault();
    m_barWidth->resetToDefault();
    m_color->resetToDefault();
    m_arrangeOrder->resetToDefault();
    m_horizontalOffset->resetToDefault();
    m_verticalOffset->resetToDefault();
}

void AppearanceSettingsModel::pushBackInOrder()
{
    m_arrangeOrder->setValue(m_arrangeOrder->value().toInt() - REARRANGE_ORDER_STEP);
}

void AppearanceSettingsModel::pushFrontInOrder()
{
    m_arrangeOrder->setValue(m_arrangeOrder->value().toInt() + REARRANGE_ORDER_STEP);
}

void AppearanceSettingsModel::configureGrid()
{
    Ms::Shortcut::getActionByName("config-raster")->trigger();
}

PropertyItem* AppearanceSettingsModel::leadingSpace() const
{
    return m_leadingSpace;
}

PropertyItem* AppearanceSettingsModel::barWidth() const
{
    return m_barWidth;
}

PropertyItem* AppearanceSettingsModel::minimumDistance() const
{
    return m_minimumDistance;
}

PropertyItem* AppearanceSettingsModel::color() const
{
    return m_color;
}

PropertyItem* AppearanceSettingsModel::arrangeOrder() const
{
    return m_arrangeOrder;
}

PropertyItem* AppearanceSettingsModel::horizontalOffset() const
{
    return m_horizontalOffset;
}

PropertyItem* AppearanceSettingsModel::verticalOffset() const
{
    return m_verticalOffset;
}

bool AppearanceSettingsModel::isSnappedToGrid() const
{
    return m_horizontallySnapToGridAction->isChecked() &&
           m_verticallySnapToGridAction->isChecked();
}

void AppearanceSettingsModel::setIsSnappedToGrid(bool isSnapped)
{
    if (isSnappedToGrid() == isSnapped)
        return;

    m_horizontallySnapToGridAction->setChecked(isSnapped);
    m_verticallySnapToGridAction->setChecked(isSnapped);
    emit isSnappedToGridChanged(isSnappedToGrid());
}

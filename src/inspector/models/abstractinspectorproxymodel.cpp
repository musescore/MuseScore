#include "abstractinspectorproxymodel.h"

using namespace mu::inspector;

AbstractInspectorProxyModel::AbstractInspectorProxyModel(QObject* parent)
    : AbstractInspectorModel(parent)
{
}

QObject* AbstractInspectorProxyModel::modelByType(const InspectorModelType type)
{
    return m_modelsHash.value(static_cast<int>(type));
}

void AbstractInspectorProxyModel::requestResetToDefaults()
{
    for (AbstractInspectorModel* model : m_modelsHash.values()) {
        model->requestResetToDefaults();
    }
}

bool AbstractInspectorProxyModel::hasAcceptableElements() const
{
    bool result = false;

    for (const AbstractInspectorModel* model : m_modelsHash.values()) {
        result |= model->hasAcceptableElements();
    }

    return result;
}

void AbstractInspectorProxyModel::addModel(AbstractInspectorModel* model)
{
    if (!model) {
        return;
    }

    connect(model, &AbstractInspectorModel::isEmptyChanged, this, [this]() {
        setIsEmpty(!hasAcceptableElements());
    });

    m_modelsHash.insert(static_cast<int>(model->modelType()), model);
}

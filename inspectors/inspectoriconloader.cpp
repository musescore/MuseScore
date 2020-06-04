#include "inspectoriconloader.h"

#include "global/gui/miconengine.h"

QMap<QString, QIcon*> InspectorIconLoader::m_iconCache = QMap<QString, QIcon*>();

QIcon* InspectorIconLoader::icon(const char* iconName)
{
    QIcon* result = m_iconCache.value(iconName);

    if (result) {
        return result;
    }

    QIcon* newIcon = new QIcon(new MIconEngine());
    newIcon->addFile(MIconEngine::iconDirPath + iconName);

    if (!newIcon->isNull()) {
        m_iconCache.insert(iconName, newIcon);
    }

    return newIcon;
}

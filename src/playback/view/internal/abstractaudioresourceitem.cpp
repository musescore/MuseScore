#include "abstractaudioresourceitem.h"

#include <QList>
#include <QTimer>

using namespace mu::playback;

AbstractAudioResourceItem::AbstractAudioResourceItem(QObject* parent)
    : QObject(parent)
{
}

void AbstractAudioResourceItem::requestToLaunchNativeEditorView()
{
    if (hasNativeEditorSupport()) {
        QTimer::singleShot(3000, this, &AbstractAudioResourceItem::nativeEditorViewLaunchRequested);
    }
}

QString AbstractAudioResourceItem::title() const
{
    return "";
}

bool AbstractAudioResourceItem::isBlank() const
{
    return true;
}

QVariantMap AbstractAudioResourceItem::buildMenuItem(const QString& itemId,
                                                     const QString& title,
                                                     const bool checked,
                                                     const QVariantList& subItems) const
{
    QVariantMap result;

    result["id"] = itemId;
    result["title"] = title;
    result["checkable"] = true;
    result["checked"] = checked;
    result["subitems"] = subItems;

    return result;
}

QVariantMap AbstractAudioResourceItem::buildSeparator() const
{
    static QVariantMap result;
    return result;
}

bool AbstractAudioResourceItem::hasNativeEditorSupport() const
{
    return false;
}

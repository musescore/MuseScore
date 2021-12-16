#include "abstractaudioresourceitem.h"

#include <QList>
#include <QTimer>

using namespace mu::playback;

//!Note Some resources like VST plugins are not able to work in a couple of msecs
//!     So we've to add explicit delay before the launching their 'native' editor views
static constexpr int EXPLICIT_DELAY_MSECS = 1000;

AbstractAudioResourceItem::AbstractAudioResourceItem(QObject* parent)
    : QObject(parent)
{
}

void AbstractAudioResourceItem::requestToLaunchNativeEditorView()
{
    if (hasNativeEditorSupport()) {
        QTimer::singleShot(EXPLICIT_DELAY_MSECS, this, &AbstractAudioResourceItem::nativeEditorViewLaunchRequested);
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

bool AbstractAudioResourceItem::isActive() const
{
    return false;
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

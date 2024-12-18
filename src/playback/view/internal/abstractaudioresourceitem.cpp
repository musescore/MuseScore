#include "abstractaudioresourceitem.h"

#include <QList>
#include <QTimer>

#include "stringutils.h"
#include "ui/view/iconcodes.h"

using namespace muse;
using namespace mu::playback;

//!Note Some resources like VST plugins are not able to work in a couple of msecs
//!     So we've to add explicit delay before the launching their 'native' editor views
static constexpr int EXPLICIT_DELAY_MSECS = 1000;

AbstractAudioResourceItem::AbstractAudioResourceItem(QObject* parent)
    : QObject(parent)
{
}

AbstractAudioResourceItem::~AbstractAudioResourceItem()
{
    if (m_editorUri.isValid()) {
        emit nativeEditorViewCloseRequested();
    }
}

void AbstractAudioResourceItem::requestToLaunchNativeEditorView()
{
    if (hasNativeEditorSupport()) {
        doRequestToLaunchNativeEditorView();
    }
}

void AbstractAudioResourceItem::updateNativeEditorView()
{
    if (hasNativeEditorSupport()) {
        doRequestToLaunchNativeEditorView();
    } else if (m_editorUri.isValid()) {
        emit nativeEditorViewCloseRequested();
    }
}

void AbstractAudioResourceItem::doRequestToLaunchNativeEditorView()
{
    QTimer::singleShot(EXPLICIT_DELAY_MSECS, this, &AbstractAudioResourceItem::nativeEditorViewLaunchRequested);
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

QVariantMap AbstractAudioResourceItem::buildExternalLinkMenuItem(const QString& menuId, const QString& title) const
{
    QVariantMap result;

    result["id"] = menuId;
    result["title"] = title;

    const int openLinkIcon = static_cast<int>(ui::IconCode::Code::OPEN_LINK);
    result["icon"] = openLinkIcon;

    return result;
}

void AbstractAudioResourceItem::sortResourcesList(audio::AudioResourceMetaList& list)
{
    std::sort(list.begin(), list.end(), [](const audio::AudioResourceMeta& m1, const audio::AudioResourceMeta& m2) {
        return strings::lessThanCaseInsensitive(m1.id, m2.id);
    });
}

bool AbstractAudioResourceItem::hasNativeEditorSupport() const
{
    return false;
}

const muse::UriQuery& AbstractAudioResourceItem::editorUri() const
{
    return m_editorUri;
}

void AbstractAudioResourceItem::setEditorUri(const UriQuery& uri)
{
    m_editorUri = uri;
}

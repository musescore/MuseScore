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
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

AbstractAudioResourceItem::~AbstractAudioResourceItem()
{
    requestToCloseNativeEditorView();
}

void AbstractAudioResourceItem::requestToLaunchNativeEditorView()
{
    if (hasNativeEditorSupport()) {
        doRequestToLaunchNativeEditorView();
    }
}

void AbstractAudioResourceItem::requestToCloseNativeEditorView()
{
    if (m_editorAction.isValid()) {
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
                                                     const QVariantList& subItems,
                                                     const bool includeInFilteredLists,
                                                     const bool isFilterCategory) const
{
    QVariantMap result;

    result["id"] = itemId;
    result["title"] = title;
    result["checkable"] = true;
    result["checked"] = checked;
    result["subitems"] = subItems;

    IF_ASSERT_FAILED(subItems.empty() || !includeInFilteredLists) {
        LOGW() << "Parent items are never included in filtered lists."; // See MenuView.cpp
    }
    result["includeInFilteredLists"] = includeInFilteredLists;

    IF_ASSERT_FAILED(!subItems.empty() || !isFilterCategory) {
        LOGW() << "Filter category items must contain subItems."; // See MenuView.cpp
    }
    result["isFilterCategory"] = isFilterCategory;

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

const actions::ActionQuery& AbstractAudioResourceItem::editorAction() const
{
    return m_editorAction;
}

void AbstractAudioResourceItem::setEditorAction(const actions::ActionQuery& action)
{
    m_editorAction = action;
}

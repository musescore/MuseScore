#include "partialtiepopupmodel.h"
#include "dom/partialtie.h"
#include "dom/tie.h"
#include "dom/undo.h"

using namespace mu::notation;
using namespace mu::engraving;
using namespace muse::uicomponents;
using namespace muse::ui;

PartialTiePopupModel::PartialTiePopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_PARTIAL_TIE, parent)
{
}

bool PartialTiePopupModel::tieDirection() const
{
    if (!m_item) {
        return false;
    }
    const Tie* tieItem = tie();
    return tieItem->up();
}

bool PartialTiePopupModel::canOpen(const EngravingItem* element)
{
    if (!element->isTieSegment()) {
        return false;
    }

    Tie* tieItem = toTieSegment(element)->tie();
    if (!tieItem || !tieItem->tieJumpPoints()) {
        return false;
    }

    if (tieItem->tieJumpPoints()->size() < 2) {
        return false;
    }

    return tieItem->isPartialTie() ? toPartialTie(tieItem)->isOutgoing() : true;
}

QPointF PartialTiePopupModel::dialogPosition() const
{
    const Tie* tieItem = tie();
    const Note* startNote = tieItem ? tieItem->startNote() : nullptr;
    const TieSegment* seg = toTieSegment(m_item);
    if (!seg || !startNote) {
        return QPointF();
    }
    const RectF segRect = seg->canvasBoundingRect();
    const double x = startNote->canvasBoundingRect().x();
    const int up = tieItem->up() ? -1 : 1;
    const double y = (tieItem->up() ? segRect.top() + segRect.height() * 2
                      / 3 : segRect.bottom() - segRect.height() / 3) + tieItem->spatium() * up;

    return fromLogical(PointF(x, y)).toQPointF();
}

QVariantList PartialTiePopupModel::items() const
{
    QVariantList items;

    for (MenuItem* item: m_items) {
        items << QVariant::fromValue(item);
    }

    return items;
}

void PartialTiePopupModel::init()
{
    AbstractElementPopupModel::init();

    connect(this, &AbstractElementPopupModel::dataChanged, [this]() {
        load();
    });

    load();
}

void PartialTiePopupModel::toggleItemChecked(const QString& id)
{
    Tie* tieItem = tie();
    if (!tieItem || !tieItem->tieJumpPoints()) {
        return;
    }

    for (MenuItem* item : m_items) {
        if (item->id() != id) {
            continue;
        }
        UiActionState state = item->state();
        state.checked = !state.checked;
        item->setState(state);
        break;
    }

    TieJumpPointList* jumpList = tieItem->tieJumpPoints();
    jumpList->toggleJumpPoint(id);
    Tie* newTie = jumpList->startTie();

    // Update popup item if it has changed
    if (newTie && newTie != tieItem) {
        m_item = newTie->segmentsEmpty() ? nullptr : newTie->frontSegment();

        interaction()->endEditGrip();
        interaction()->endEditElement();
        interaction()->startEditGrip(m_item, Grip::DRAG);
    }

    updateNotation();
    emit itemsChanged();
}

void PartialTiePopupModel::load()
{
    Tie* tieItem = tie();
    if (!tieItem) {
        return;
    }

    tieItem->updatePossibleJumpPoints();

    m_items = makeMenuItems();

    // load items
    emit tieDirectionChanged(tieDirection());
    emit itemsChanged();
}

MenuItemList PartialTiePopupModel::makeMenuItems()
{
    Tie* tieItem = tie();
    if (!tieItem || !tieItem->tieJumpPoints()) {
        return MenuItemList{};
    }

    MenuItemList itemList;

    std::set<Note*> foundNotes;

    for (const TieJumpPoint* jumpPoint : *tieItem->tieJumpPoints()) {
        if (muse::contains(foundNotes, jumpPoint->note())) {
            continue;
        }
        itemList << makeMenuItem(jumpPoint);
        foundNotes.insert(jumpPoint->note());
    }

    return itemList;
}

muse::uicomponents::MenuItem* PartialTiePopupModel::makeMenuItem(const engraving::TieJumpPoint* jumpPoint)
{
    MenuItem* item = new MenuItem(this);
    item->setId(jumpPoint->id());
    TranslatableString title = TranslatableString("notation", jumpPoint->menuTitle());
    item->setTitle(title);

    UiAction action;
    action.title = title;
    action.checkable = Checkable::Yes;
    item->setAction(action);

    UiActionState state;
    state.enabled = true;
    state.checked = jumpPoint->active();
    item->setState(state);

    return item;
}

Tie* PartialTiePopupModel::tie() const
{
    const TieSegment* tieSeg = m_item && m_item->isTieSegment() ? toTieSegment(m_item) : nullptr;

    return tieSeg ? tieSeg->tie() : nullptr;
}

void mu::notation::PartialTiePopupModel::onClosed()
{
    Tie* tieItem = tie();
    if (!tieItem) {
        return;
    }

    Note* startNote = tieItem->startNote();

    if (tieItem->allJumpPointsInactive() && startNote && startNote->tieFor() == tieItem) {
        Score* score = tieItem->score();
        beginCommand(TranslatableString("engraving", "Remove partial tie"));
        score->undoRemoveElement(tieItem);
        endCommand();

        // Combine this with the last undoable action (which will be to remove a tie) so the user cannot undo to get a translucent tie
        undoStack()->mergeCommands(undoStack()->currentStateIndex() - 2);
    }
}

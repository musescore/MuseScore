#include "instrumentspanelcontextmenumodel.h"

#include "log.h"
#include "translation.h"

#include "actions/actiontypes.h"

using namespace mu::context;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::ui;
using namespace mu::actions;

InstrumentsPanelContextMenuModel::InstrumentsPanelContextMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void InstrumentsPanelContextMenuModel::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr notation = globalContext()->currentNotation();
        m_masterNotation = globalContext()->currentMasterNotation();

        if (!m_masterNotation || !notation || m_masterNotation->notation() != notation) {
            clear();
        } else {
            loadItems();
        }
    });
}

void InstrumentsPanelContextMenuModel::loadItems()
{
    TRACEFUNC;

    m_orders.clear();

    RetValCh<InstrumentsMeta> meta = instrumentsRepository()->instrumentsMeta();
    if (!meta.ret) {
        LOGE() << meta.ret.toString();
        return;
    }

    m_orders = meta.val.scoreOrders;
    MenuItemList orderItems;

    dispatcher()->unReg(this);

    for (const ScoreOrder* order : m_orders) {
        MenuItem orderItem;

        orderItem.id = order->id;
        orderItem.title = order->name;
        orderItem.code = codeFromQString("set-order-" + order->id);
        orderItem.args = ActionData::make_arg1<QString>(order->id);
        orderItem.checkable = Checkable::Yes;
        orderItem.state.enabled = true;
        orderItem.state.checked = m_masterNotation->notation()->scoreOrder().id == order->id;

        orderItems << orderItem;

        dispatcher()->reg(this, orderItem.code, [this, order]() {
            m_masterNotation->parts()->setScoreOrder(*order);
        });
    }

    MenuItemList items {
        makeMenu(qtrc("instruments", "Instrument ordering"), orderItems)
    };

    setItems(items);
}

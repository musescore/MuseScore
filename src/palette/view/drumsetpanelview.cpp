#include "drumsetpanelview.h"

#include "internal/widgets/drumsetpanel.h"

using namespace mu::notation;

namespace mu::palette {
class DrumsetPanelAdapter : public ui::IDisplayableWidget
{
public:
    DrumsetPanelAdapter()
        : m_msDrumsetPanel(new Ms::DrumsetPanel())
    {
    }

    ~DrumsetPanelAdapter() override
    {
        delete m_msDrumsetPanel;
    }

    void setNotation(INotationPtr notation)
    {
        m_msDrumsetPanel->setNotation(notation);
    }

    void updateDrumset()
    {
        m_msDrumsetPanel->updateDrumset();
    }

    async::Channel<QString> pitchNameChanged() const
    {
        return m_msDrumsetPanel->pitchNameChanged();
    }

private:
    QWidget* qWidget() override
    {
        return m_msDrumsetPanel;
    }

    bool handleEvent(QEvent* event) override
    {
        return m_msDrumsetPanel->handleEvent(event);
    }

    Ms::DrumsetPanel* m_msDrumsetPanel = nullptr;
};
}

using namespace mu::palette;

DrumsetPanelView::DrumsetPanelView(QQuickItem* parent)
    : WidgetView(parent)
{
}

QString DrumsetPanelView::pitchName() const
{
    return m_pitchName;
}

void DrumsetPanelView::editDrumset()
{
    dispatcher()->dispatch("edit-drumset");
}

void DrumsetPanelView::componentComplete()
{
    WidgetView::componentComplete();

    auto drumsetPanel = std::make_shared<DrumsetPanelAdapter>();

    auto updateView = [this, drumsetPanel]() {
        drumsetPanel->updateDrumset();
        update();
    };

    globalContext()->currentNotationChanged().onNotify(this, [this, drumsetPanel, updateView]() {
        INotationPtr notation = globalContext()->currentNotation();
        drumsetPanel->setNotation(notation);
        updateView();

        if (!notation) {
            return;
        }

        notation->interaction()->noteInput()->stateChanged().onNotify(this, [updateView]() {
            updateView();
        });
    });

    drumsetPanel->pitchNameChanged().onReceive(this, [this](const QString& pitchName) {
        m_pitchName = pitchName;
        emit pitchNameChanged();
    });

    setWidget(drumsetPanel);
}

#include "drumsetpanelview.h"

#include "internal/widgets/drumsetpalette.h"

using namespace mu::notation;

namespace mu::palette {
class DrumsetPaletteAdapter : public ui::IDisplayableWidget
{
public:
    DrumsetPaletteAdapter()
        : m_msDrumsetPalette(new Ms::DrumsetPalette())
    {
    }

    ~DrumsetPaletteAdapter() override
    {
        delete m_msDrumsetPalette;
    }

    void setNotation(INotationPtr notation)
    {
        m_msDrumsetPalette->setNotation(notation);
    }

    void updateDrumset()
    {
        m_msDrumsetPalette->updateDrumset();
    }

    async::Channel<QString> pitchNameChanged() const
    {
        return m_msDrumsetPalette->pitchNameChanged();
    }

private:
    QWidget* qWidget() override
    {
        return m_msDrumsetPalette;
    }

    bool handleEvent(QEvent* event) override
    {
        return m_msDrumsetPalette->handleEvent(event);
    }

    Ms::DrumsetPalette* m_msDrumsetPalette = nullptr;
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

    auto drumsetPalette = std::make_shared<DrumsetPaletteAdapter>();

    auto updateView = [this, drumsetPalette]() {
        drumsetPalette->updateDrumset();
        update();
    };

    globalContext()->currentNotationChanged().onNotify(this, [this, drumsetPalette, updateView]() {
        INotationPtr notation = globalContext()->currentNotation();
        drumsetPalette->setNotation(notation);
        updateView();

        if (!notation) {
            return;
        }

        notation->interaction()->noteInput()->stateChanged().onNotify(this, [updateView]() {
            updateView();
        });
    });

    drumsetPalette->pitchNameChanged().onReceive(this, [this](const QString& pitchName) {
        m_pitchName = pitchName;
        emit pitchNameChanged();
    });

    setWidget(drumsetPalette);
}

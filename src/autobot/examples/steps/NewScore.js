
function openNewScoreDialog()
{
    api.navigation.triggerControl("AppTitleBar", "AppMenuBar", "&File")
    // wait popup open
    api.autobot.waitPopup()
    // New become automatically current, so just trigger
    api.navigation.trigger()
}

function сhooseFluteAndPiano()
{
    // Flute
    api.navigation.goToControl("NewScoreDialog", "FamilyView", "Woodwinds")
    api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Flute")
    api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")

    // just for see changes
    api.autobot.sleep(500)

    // Piano
    api.navigation.goToControl("NewScoreDialog", "FamilyView", "Keyboards")
    api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Piano")
    api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")

    // just for see changes
    api.autobot.sleep(500)

    // Done
    api.navigation.triggerControl("NewScoreDialog", "BottomPanel", "Done")
}

module.exports = {
    openNewScoreDialog: openNewScoreDialog,
    сhooseFluteAndPiano: сhooseFluteAndPiano
}

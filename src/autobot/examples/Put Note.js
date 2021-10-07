function main()
{
    var testCase = {
        name: "New Score and Put Note",
        steps: [
            {name: "Open Dialog", wait: true, func: function() {
                // api.dispatcher.dispatch("file-new")

                // OR

                api.navigation.triggerControl("AppTitleBar", "AppMenuBar", "&File")
                // wait popup open
                api.autobot.sleep(500)
                // New become automatically current, so just trigger
                api.navigation.trigger()
            }},
            {name: "Select Instruments", func: function() {

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

            }},
            {name: "Note input mode", func: function() {
                // api.dispatcher.dispatch("note-input")
                // api.dispatcher.dispatch("pad-note-8")

                // OR

                api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "note-input-steptime")
                // wait popup open
                api.autobot.sleep(500)
                // First item become automatically current, so just trigger
                api.navigation.trigger()

                // Select note
                api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "pad-note-8")
            }},
            {name: "Note input", func: function() {
                api.dispatcher.dispatch("note-c")
                api.dispatcher.dispatch("note-d")
                api.dispatcher.dispatch("note-e")
                api.dispatcher.dispatch("note-f")
                api.dispatcher.dispatch("note-g")
                api.dispatcher.dispatch("note-a")
                api.dispatcher.dispatch("note-b")

                // OR

                // Open menu "+" and so on
            }},
            {name: "Play", func: function() {
                // api.dispatcher.dispatch("play")

                // OR

                api.navigation.triggerControl("TopTool", "PlaybackToolBar", "Play")
            }},
        ]
    };

    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
    api.log.info("----------- end script ---------------")
}

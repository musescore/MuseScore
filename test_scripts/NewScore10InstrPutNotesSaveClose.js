var NewScore = require("steps/NewScore.js")

function main()
{
    var testCase = {
        name: "NewScore10InstrPutNotesSaveClose",
        steps: [
            {name: "Open Dialog", func: function() {
                NewScore.openNewScoreDialog()
            }},
            {name: "Select Instruments", func: function() {
                NewScore.chooseRandomInstruments(10)
            }},
            {name: "Note input mode", func: function() {

                api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "note-input-steptime")
                api.autobot.waitPopup()
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
            }},
            {name: "Save", func: function() {
                api.autobot.saveProject("NewScore10InstrPutNotesSaveClose.mscz")
            }},
            {name: "Close", func: function() {
                api.dispatcher.dispatch("file-close")
                api.autobot.seeChanges()

                // Go Home
                api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
            }}
        ]
    };

    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
    api.log.info("----------- end script ---------------")
}

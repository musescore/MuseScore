function main()
{
    var testCase = {
        name: "New Score",
        steps: [
            {name: "Open Dialog", wait: false, func: function() {
                api.dispatcher.dispatch("file-new")
            }},
            {name: "Select Flute", func: function() {
                api.navigation.goToControlByName("NewScoreDialog", "FamilyView", "Woodwinds")
                api.navigation.goToControlByName("NewScoreDialog", "InstrumentsView", "Flute")

            }},
            {name: "Select Flute (apply)", func: function() {
                api.navigation.goToControlByName("NewScoreDialog", "SelectPanel", "Select")
                api.navigation.triggerCurrentControl()

            }},
            {name: "Select Piano", func: function() {
                api.navigation.goToControlByName("NewScoreDialog", "FamilyView", "Keyboards")
                api.navigation.goToControlByName("NewScoreDialog", "InstrumentsView", "Piano")


            }},
            {name: "Select Piano (apply)", func: function() {
                api.navigation.goToControlByName("NewScoreDialog", "SelectPanel", "Select")
                api.navigation.triggerCurrentControl()

            }},
            {name: "Done", func: function() {
                api.navigation.goToControlByName("NewScoreDialog", "BottomPanel", "Done")
                api.navigation.triggerCurrentControl()

            }},
        ]
    };

    api.autobot.setInterval(500)
    api.autobot.runTestCase(testCase)
    api.log.info("----------- end script ---------------")
}

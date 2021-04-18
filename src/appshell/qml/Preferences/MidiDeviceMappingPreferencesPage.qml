import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Preferences 1.0
import MuseScore.Midi 1.0

PreferencesPage {
    id: root

    function apply() {
        return page.apply()
    }

    MidiDeviceMappingPage {
        id: page

        anchors.fill: parent
    }
}

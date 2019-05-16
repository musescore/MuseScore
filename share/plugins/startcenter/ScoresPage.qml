import QtQuick 2.12
import QtQuick.Controls 2.5
import "common"
import "style.js" as Style

Item {
    TabBar {
        id: scorePageTabBar
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 1 // TODO: make -1 and draw tab button border instead (see below)?
        padding: 1
        z: 1

        background: Rectangle {
            anchors.fill: parent
            color: Style.borderLinesColor // TODO: one pixel has wrong color at a border between tab buttons
            radius: Style.buttonRoundness
        }

        Repeater {
            model: [ qsTr("My Scores"), qsTr("Templates"), qsTr("Community") ]

            delegate: TabButton {
                text: modelData
                property bool highlighted: index == scorePageTabBar.currentIndex || hovered

                background: HalfRoundedRectangle {
                    color: highlighted ? Style.scoresPageTabActiveButtonColor : Style.scoresPageTabButtonColor
                    roundLeft: index == 0
                    roundRight: index == scorePageTabBar.count - 1
                    radius: Style.buttonRoundness
                }

                contentItem: Text {
                    text: modelData
                    color: Style.textColor
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    SwipeView {
        width: parent.width
        anchors.top: scorePageTabBar.verticalCenter
        anchors.bottom: openScoreButton.top
        anchors.bottomMargin: Style.layoutMargin
        clip: true // avoid drawing on page margins
        orientation: Qt.Horizontal
        interactive: false
        currentIndex: scorePageTabBar.currentIndex

        background: Rectangle {
            border.width: 1
            border.color: Style.borderLinesColor
            color: Style.scoresPageTabContentBackgroundColor
            radius: Style.buttonRoundness
        }

        Repeater {
            model: 3

            Column {
                anchors.top: parent.top
                anchors.topMargin: Style.layoutMargin + scorePageTabBar.height / 2
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Style.layoutMargin
                ScoreBrowser {
//                     flow: GridView.FlowTopToBottom
//                     focus: true
//                     keyNavigationEnabled: true

                    function getListWidth() {
                        const init = parent.width - 2 * Style.layoutMargin;
                        return init - (init % cellWidth);
                    }

                    width: getListWidth()
                    height: parent.height
                    anchors.horizontalCenter: parent.horizontalCenter

                    Component.onCompleted: model = mscore.recentScores

                    onScoreClicked: mscore.openScore(url)
                }
            }
        }
    }

    StartCenterButton {
        id: openScoreButton
        text: qsTr("Open a score")
        backgroundColor: hovered ? Style.scoresPageTabActiveButtonColor : Style.scoresPageBottomButtonsColor
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        onClicked: mscore.openScoreDialog()
    }
}

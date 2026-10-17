// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Layouts
import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

StyledPopupView {
    id:root
    required property var editor
    showArrow:false
    padding:12
    property real availableWidth:640
    property var boundsProvider:null
    property rect availableRect:Qt.rect(-200,-300,800,600)
    property real dragX:0
    property real dragY:0
    anchorItem:null
    x:Math.max(availableRect.x+12,Math.min(parent.width/2-width/2+dragX,availableRect.x+availableRect.width-width-12))
    y:Math.max(availableRect.y+12,Math.min((availableRect.y+availableRect.height-parent.height>=height?parent.height:-height)+dragY,availableRect.y+availableRect.height-height-12))
    contentWidth:Math.max(280,Math.min(420,availableWidth-48))
    contentHeight:formatting.implicitHeight+64
    placementPolicies:PopupView.ManualPosition
    openPolicies:PopupView.NoActivateFocus
    focusPolicies:PopupView.DefaultFocus & ~PopupView.ClickFocus
    closePolicies:PopupView.NoAutoClose
    closeOnEscape:false
    function begin(){adapter.refresh();if(boundsProvider)availableRect=boundsProvider(parent);open()}
    QtObject {
        id:adapter
        property bool refreshing:false
        readonly property bool isHorizontalAlignmentAvailable:true
        property var fontFamily:QtObject{property bool isUndefined:false;property string value:"Arial";onValueChanged:adapter.apply()}
        property var fontSize:QtObject{property bool isUndefined:false;property real value:24;onValueChanged:adapter.apply()}
        property var fontStyle:QtObject{property bool isUndefined:false;property int value:0;onValueChanged:adapter.apply()}
        property var horizontalAlignment:QtObject{property bool isUndefined:false;property int value:0;onValueChanged:adapter.apply()}
        property var verticalAlignment:QtObject{property bool isUndefined:false;property int value:0}
        property var textScriptAlignment:QtObject{property bool isUndefined:false;property int value:0}
        property var textLineSpacing:QtObject{property bool isUndefined:false;property real value:1}
        function refresh(){
            if(!root.editor.textEditing)return
            refreshing=true
            var control=root.editor.textEditorControl
            fontFamily.value=control.font.family;fontSize.value=control.font.pixelSize
            fontStyle.value=root.editor.textFontStyle()
            horizontalAlignment.value=control.horizontalAlignment===Text.AlignHCenter?2:control.horizontalAlignment===Text.AlignRight?1:control.horizontalAlignment===Text.AlignJustify?3:0
            refreshing=false
        }
        function apply(){
            if(refreshing||!root.editor||!root.editor.textEditing)return
            var control=root.editor.textEditorControl
            control.font.family=fontFamily.value;control.font.pixelSize=fontSize.value
            control.font.bold=!!(fontStyle.value&1);control.font.italic=!!(fontStyle.value&2)
            control.font.underline=!!(fontStyle.value&4);control.font.strikeout=!!(fontStyle.value&8)
            control.horizontalAlignment=horizontalAlignment.value===2?Text.AlignHCenter:horizontalAlignment.value===1?Text.AlignRight:horizontalAlignment.value===3?Text.AlignJustify:Text.AlignLeft
            root.editor.updateTextFormat()
        }
    }
    NavigationPanel{id:formatNav;name:"DrawingTextFormatting";direction:NavigationPanel.Vertical;section:root.navigationSection}
    ColumnLayout {
        width:root.contentWidth;spacing:8
        Rectangle {
            Layout.fillWidth:true;Layout.preferredHeight:14;color:"transparent"
            Rectangle {anchors.centerIn:parent;width:40;height:3;radius:1.5;color:ui.theme.fontPrimaryColor;opacity:0.45}
            MouseArea {
                anchors.fill:parent;cursorShape:Qt.SizeAllCursor
                property point origin
                property point offset
                onPressed:function(mouse){origin=mapToGlobal(mouse.x,mouse.y);offset=Qt.point(root.dragX,root.dragY)}
                onPositionChanged:function(mouse){if(pressed){var p=mapToGlobal(mouse.x,mouse.y);root.dragX=offset.x+p.x-origin.x;root.dragY=offset.y+p.y-origin.y}}
            }
        }
        // This is the same formatting component used by TextStylePopup for
        // ordinary MuseScore text. Only irrelevant frame/style controls stay out.
        TextStyleFormatting {
            id:formatting;Layout.fillWidth:true
            settingsModel:adapter;compact:true
            navigationPanel:formatNav
        }
        RowLayout {
            Layout.fillWidth:true
            FlatButton {text:qsTrc("inspector","Add symbols");onClicked:root.editor.characterPickerRequested()}
            Item{Layout.fillWidth:true}
            FlatButton {text:qsTrc("global","Done");onClicked:{root.editor.finishTextEditing();root.editor.forceActiveFocus()}}
        }
    }
    Connections {
        target:root.editor
        function onTextEditingChanged(){if(!root.editor.textEditing)root.close()}
    }
}

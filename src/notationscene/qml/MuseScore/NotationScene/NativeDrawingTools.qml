// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtQuick.Window
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs
import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene
import "drawing/ui"
Item {
    id:root
    implicitWidth:64
    readonly property var popupHost:Controls.Overlay.overlay||(Window.window?Window.window.contentItem:null)
    readonly property real availableWidth:popupHost?popupHost.width:Math.max(320,width)
    readonly property real availableHeight:popupHost?popupHost.height:Math.max(160,height)
    onWidthChanged:if(options.isOpened)options.updatePosition()
    onHeightChanged:if(options.isOpened)options.updatePosition()
    property string group:""
    property var activeButton:null
    property string shapeTool:"rectangle"
    property string pointTool:"nodes"
    property string selectionTool:"select"
    readonly property var editor:DrawingSession.editor
    readonly property var toolGroups:({
        select:[{key:"select",label:qsTrc("drawing-tools","Select")},{key:"transform",label:qsTrc("drawing-tools","Transform selection")}],
        combine:[{key:"group",label:qsTrc("drawing-tools","Group drawings")},{key:"ungroup",label:qsTrc("drawing-tools","Ungroup drawings")},{key:"fuse",label:qsTrc("drawing-tools","Fuse drawings")}],
        pencil:[{key:"pencil",label:qsTrc("drawing-tools","Pencil")}],
        pen:[{key:"pen",label:qsTrc("drawing-tools","Bézier pen")}],
        text:[{key:"text",label:qsTrc("drawing-tools","Text")},{key:"characters",label:qsTrc("drawing-tools","Special and musical characters")}],
        paint:[{key:"paint-fill",label:qsTrc("drawing-tools","Paint fill")},{key:"paint-stroke",label:qsTrc("drawing-tools","Paint stroke")},{key:"gradient-fill",label:qsTrc("drawing-tools","Fill gradient")},{key:"gradient-stroke",label:qsTrc("drawing-tools","Stroke gradient")}],
        shapes:[{key:"line",label:qsTrc("drawing-tools","Line")},{key:"rectangle",label:qsTrc("drawing-tools","Rectangle")},{key:"ellipse",label:qsTrc("drawing-tools","Ellipse")},{key:"triangle",label:qsTrc("drawing-tools","Triangle")},{key:"right-triangle",label:qsTrc("drawing-tools","Right triangle")},{key:"arrow",label:qsTrc("drawing-tools","Arrow")},{key:"arrow-double",label:qsTrc("drawing-tools","Double arrow")},{key:"arrow-filled",label:qsTrc("drawing-tools","Filled arrow")}],
        nodes:[{key:"nodes",label:qsTrc("drawing-tools","Edit points")},{key:"addnode",label:qsTrc("drawing-tools","Add point")},{key:"deletenode",label:qsTrc("drawing-tools","Delete point")},{key:"join",label:qsTrc("drawing-tools","Join points")},{key:"anchors",label:qsTrc("drawing-tools","Link points")},{key:"unlink",label:qsTrc("drawing-tools","Unlink points")}],
        eraser:[{key:"eraser",label:qsTrc("drawing-tools","Erase entire line")},{key:"eraser-partial",label:qsTrc("drawing-tools","Erase touched areas")},{key:"erase-rectangle",label:qsTrc("drawing-tools","Rectangular eraser")},{key:"erase-ellipse",label:qsTrc("drawing-tools","Elliptical eraser")},{key:"erase-triangle",label:qsTrc("drawing-tools","Triangular eraser")}]
    })
    function selectTool(key){
        if(!editor)return
        editor.finishTextEditing()
        editor.tool=key==="eraser-partial"?"eraser":key;DrawingSession.setDrawingMode(true)
        if(key==="eraser"||key==="eraser-partial")editor.eraserMode=key==="eraser"?"whole":"partial"
        editor.forceActiveFocus()
        if(key==="select"||key==="transform")selectionTool=key
        if(editor.isBasicShape(key))shapeTool=key
        if(["nodes","addnode","deletenode","join","anchors","unlink"].indexOf(key)>=0)pointTool=key
    }
    function shortcutFor(key){return ({select:"V",transform:"Shift+V",nodes:"A",addnode:"Shift+A",deletenode:"Shift+A",join:"Shift+A",anchors:"Shift+A",unlink:"Shift+A",pencil:"N",pen:"P",eraser:"E", "eraser-partial":"Shift+E","erase-rectangle":"Shift+E","erase-ellipse":"Shift+E","erase-triangle":"Shift+E",rectangle:"M",ellipse:"L",line:"Shift+M",triangle:"Shift+M","right-triangle":"Shift+M",arrow:"Shift+M","arrow-double":"Shift+M","arrow-filled":"Shift+M",text:"T",characters:"Shift+T",colors:"X","paint-fill":"B","paint-stroke":"Shift+B","gradient-fill":"G","gradient-stroke":"Shift+G"})[key]||""}
    function toolHelp(key){var help=toolDescription(key),shortcut=shortcutFor(key);return help+(shortcut?" ("+shortcut+")":"")+((key==="select"||key==="transform")?" · Ctrl+D":"")}
    readonly property var descriptionKeys:{
        var keys=["colors","shapes","combine"]
        Object.keys(toolGroups).forEach(function(group){toolGroups[group].forEach(function(tool){if(keys.indexOf(tool.key)<0)keys.push(tool.key)})})
        return keys
    }
    function toolDescription(key){
        if(key==="paint"||key.indexOf("paint-")===0)return qsTrc("drawing-tools","Click a drawing to paint its fill or stroke. Painting applies to the selected drawings together.")
        if(key.indexOf("gradient-")===0)return qsTrc("drawing-tools","Drag to set the gradient transition. Drag its ends to change the extent. Select a colour square to edit it; slide it along the axis to space colours. Add colours in this panel. Controls appear only in Gradient mode.")
        if(key==="colors")return qsTrc("drawing-tools","Choose primary and secondary colours. X switches the active colour.")
        if(key==="pen")return qsTrc("drawing-tools","Click or drag nodes. Enter or double-click the last node finishes a path. Connect to the first node to close a shape and keep drawing.")
        if(key==="select")return qsTrc("drawing-tools","Click or drag a box to select drawings. Shift-click adds or removes a drawing. Drag the selection to move it.")
        if(key==="transform")return qsTrc("drawing-tools","Transform selection")+". "+qsTrc("drawing-tools","Shift: mirrored sides. Ctrl: proportional size. Centre point: rotate.")
        if(key==="group"||key==="ungroup"||key==="fuse")return qsTrc("drawing-tools","Grouping keeps individual points and attachments. Fusing creates one editable drawing with one initial anchor.")
        if(key==="nodes")return qsTrc("drawing-tools","Drag points to reshape the drawing. Green endpoints belong to a derived fragment.")
        if(key==="anchors")return qsTrc("drawing-tools","Drag a point or text close to a drawing point or rhythmic anchor to attach it.")
        if(key==="unlink")return qsTrc("drawing-tools","Click an attachment line to unlink it. The red cross appears only over attachment lines.")
        if(key.indexOf("erase-")===0)return qsTrc("drawing-tools","Drag an eraser shape. Only drawing areas inside it are removed.")
        if(key==="eraser-partial")return qsTrc("drawing-tools","Erase only the areas touched by the circular eraser.")
        if(key==="eraser")return qsTrc("drawing-tools","Erase the entire drawing touched by the eraser.")
        if(key==="text"||key==="characters")return qsTrc("drawing-tools","Drag a box on the score to type. Double-click existing text to edit. Ctrl+Enter finishes.")
        if(key==="pencil")return qsTrc("drawing-tools","Drag to draw. Alt constrains the stroke to horizontal, vertical or diagonal movement.")
        if(key.indexOf("arrow")===0)return qsTrc("drawing-tools","Select an arrow. Drag the yellow diamond horizontally for head size, vertically for its opening. Cyan changes length and direction; lime changes hollow shaft width. The head stays unanchored unless you use Link points.")
        return qsTrc("drawing-tools","Shift draws from the centre; Ctrl keeps proportions; Alt constrains direction.")
    }
    function openGroup(key,button){
        if(options.isOpened&&group===key){options.close();return}
        group=key;activeButton=button
        if(key!=="combine"&&key!=="colors")selectTool(key==="select"?selectionTool:key==="shapes"?shapeTool:key==="nodes"?pointTool:key==="paint"?(editor&&editor.paintingTool?editor.tool:"paint-fill"):key)
        options.updatePosition();options.open()
    }
    function importSvg(url){
        var v=DrawingSession.view,e=editor
        if(!e||!v)return
        var f=v.drawingViewTransform
        try{e.importSvg(v.loadSvgDrawing(url),(f.left||0)+(f.visibleWidth||e.width)/2,(f.top||0)+(f.visibleHeight||e.height)/2)}catch(error){e.status=error.message}
    }
    Flickable {
        id:toolScroll
        anchors.fill:parent;anchors.topMargin:22;anchors.bottomMargin:22
        contentWidth:width;contentHeight:toolColumn.height+16
        flickableDirection:Flickable.VerticalFlick
        boundsBehavior:Flickable.StopAtBounds
        clip:true
        function scrollBy(amount){contentY=Math.max(0,Math.min(contentHeight-height,contentY+amount))}
        onContentYChanged:if(options.isOpened)options.updatePosition()
        Controls.ScrollBar.vertical:StyledScrollBar{policy:Controls.ScrollBar.AsNeeded;thickness:6}
    Column {
        id:toolColumn
        opacity:DrawingSession.modeActive?1:0.45
        anchors.top:parent.top;anchors.topMargin:8;anchors.horizontalCenter:parent.horizontalCenter;spacing:6
        FlatButton {
            id:colourButton;width:40;height:44;transparent:true
            property bool drawingToolTip:true
            toolTipTitle:qsTrc("drawing-tools","Colours");toolTipShortcut:"X"
            onClicked:{DrawingSession.setDrawingMode(true);root.openGroup("colors",colourButton)}
            Connections{target:colourButton.mouseArea;function onContainsMouseChanged(){settings.helpText=colourButton.mouseArea.containsMouse?root.toolHelp("colors"):""}}
            contentItem:Component {
                Item {
                    width:40;height:44
                    Rectangle{x:15;y:14;width:23;height:23;radius:4;color:root.editor?root.editor.secondaryColor:"white";border.width:1;border.color:ui.theme.fontPrimaryColor}
                    Rectangle{x:5;y:5;width:25;height:25;radius:4;color:root.editor?root.editor.strokeColor:"black";border.width:1;border.color:ui.theme.fontPrimaryColor}
                }
            }
        }
        Item{width:40;height:8}
        SeparatorLine{width:40}
        Repeater {
            model:[{key:"select",label:qsTrc("drawing-tools","Select")},{key:"pencil",label:qsTrc("drawing-tools","Pencil")},{key:"pen",label:qsTrc("drawing-tools","Bézier pen")},{key:"shapes",label:qsTrc("drawing-tools","Basic shapes")},{key:"paint",label:qsTrc("drawing-tools","Painting and gradients")},{key:"nodes",label:qsTrc("drawing-tools","Points and attachments")},{key:"eraser",label:qsTrc("drawing-tools","Eraser")},{key:"text",label:qsTrc("drawing-tools","Text")}]
            delegate:NativeDrawingIconButton {
                id:groupButton
                required property var modelData
                glyph:modelData.key==="select"?root.selectionTool:modelData.key==="shapes"?root.shapeTool:modelData.key==="nodes"?root.pointTool:modelData.key
                grouped:true;toolTipTitle:modelData.label;toolTipShortcut:root.shortcutFor(modelData.key)
                accentButton:DrawingSession.modeActive&&root.editor&&(root.toolGroups[modelData.key]||[]).some(function(t){return t.key===root.editor.tool})
                onHoveredChanged:settings.helpText=hovered?root.toolHelp(modelData.key):""
                onClicked:root.openGroup(modelData.key,groupButton)
            }
        }
        SeparatorLine{width:40}
        NativeDrawingIconButton{id:combineButton;glyph:"group";grouped:true;toolTipTitle:qsTrc("drawing-tools","Group and fuse drawings");onClicked:{DrawingSession.setDrawingMode(true);root.openGroup("combine",combineButton)}}
        NativeDrawingIconButton {
            glyph:"characters";toolTipTitle:qsTrc("drawing-tools","Special and musical characters");toolTipShowLocked:false
            onClicked:if(DrawingSession.view){DrawingSession.setDrawingMode(true);root.editor.tool="characters";DrawingSession.view.openDrawingCharacters()}
        }
        NativeDrawingIconButton{glyph:"import";toolTipTitle:qsTrc("drawing-tools","Import SVG");onClicked:{DrawingSession.setDrawingMode(true);svgDialog.open()}}
        NativeDrawingIconButton{glyph:"export";toolTipTitle:qsTrc("drawing-tools","Export SVG");onClicked:{DrawingSession.setDrawingMode(true);exportDialog.open()}}
        SeparatorLine{width:40}
        NativeDrawingIconButton{glyph:"undo";toolTipTitle:qsTrc("drawing-tools","Undo");onClicked:if(root.editor){DrawingSession.setDrawingMode(true);root.editor.undo()}}
        NativeDrawingIconButton{glyph:"redo";toolTipTitle:qsTrc("drawing-tools","Redo");onClicked:if(root.editor){DrawingSession.setDrawingMode(true);root.editor.redo()}}
        NativeDrawingIconButton{glyph:"delete";toolTipTitle:qsTrc("drawing-tools","Delete selection");onClicked:if(root.editor){DrawingSession.setDrawingMode(true);root.editor.removeSelected()}}
        NativeDrawingIconButton {
            glyph:root.editor&&root.editor.anchorGridMode==="note"?"anchor-note":"anchor-subdivision"
            toolTipShowLocked:false
            toolTipTitle:root.editor&&root.editor.anchorGridMode==="note"?qsTrc("drawing-tools","Anchor to notes"):qsTrc("drawing-tools","Anchor to subdivisions")
            toolTipDescription:qsTrc("drawing-tools","Click to switch between note positions and rhythmic subdivisions.")
            onClicked:if(root.editor){DrawingSession.setDrawingMode(true);root.editor.anchorGridMode=root.editor.anchorGridMode==="note"?"subdivision":"note";root.editor.forceActiveFocus()}
        }
    }
    }
    FlatButton {
        anchors.top:parent.top;anchors.horizontalCenter:parent.horizontalCenter
        width:40;height:20;icon:IconCode.SMALL_ARROW_UP;transparent:true
        visible:toolScroll.contentY>1
        toolTipTitle:qsTrc("drawing-tools","Scroll tools up")
        onClicked:toolScroll.scrollBy(-138)
    }
    FlatButton {
        anchors.bottom:parent.bottom;anchors.horizontalCenter:parent.horizontalCenter
        width:40;height:20;icon:IconCode.SMALL_ARROW_DOWN;transparent:true
        visible:toolScroll.contentY+toolScroll.height<toolScroll.contentHeight-1
        toolTipTitle:qsTrc("drawing-tools","Scroll tools down")
        onClicked:toolScroll.scrollBy(138)
    }
    StyledPopupView {
        id:options
        parent:root.activeButton||root
        property rect availableRect:Qt.rect(-320,-100,640,600)
        function updatePosition(){
            if(DrawingSession.view&&parent)availableRect=DrawingSession.view.drawingPopupBounds(parent)
            Qt.callLater(repositionWindowIfNeed)
        }
        readonly property real roomLeft:Math.max(0,-availableRect.x)
        readonly property real roomRight:Math.max(0,availableRect.x+availableRect.width-parent.width)
        readonly property real descriptionHeight:Math.ceil(descriptionSizer.childrenRect.height)
        // PopupView.anchorItem bounds the window; it does not identify the
        // button it opens beside. The visual parent supplies that reference.
        anchorItem:null
        x:0
        y:Math.max(availableRect.y+8,Math.min(parent.height/2-height/2,availableRect.y+availableRect.height-height-8))
        padding:12
        closePolicies:PopupView.CloseOnPressOutsideParent
        contentWidth:Math.max(160,Math.min(320,Math.max(roomLeft,roomRight)-48))
        contentHeight:Math.max(48,Math.min(availableRect.height-64,subtools.implicitHeight+settings.implicitHeight+descriptionHeight+36))
        placementPolicies:roomRight>=width&&(roomLeft<width||roomRight>=roomLeft)?PopupView.PreferRight:PopupView.PreferLeft
        showArrow:true
        closeOnEscape:false
        onClosed:{settings.helpText="";settings.pointerPreview=false;settings.sliderBusy=false;if(root.editor&&DrawingSession.modeActive&&!root.editor.textEditing)root.editor.forceActiveFocus()}
        ColumnLayout {
            width:options.contentWidth;height:options.contentHeight
            Keys.onShortcutOverride:function(event){
                if(!root.editor||!DrawingSession.modeActive||root.editor.textEditing)return
                if(root.editor.toolShortcut(event)!==""||event.key===Qt.Key_Escape||event.key===Qt.Key_Return||event.key===Qt.Key_Enter||event.key===Qt.Key_Delete||((event.modifiers&Qt.ControlModifier)&&(event.key===Qt.Key_D||event.key===Qt.Key_Z||event.key===Qt.Key_Y)))event.accepted=true
            }
            Keys.onPressed:function(event){
                if(!root.editor)return
                root.editor.handleDrawingKey(event)
                if(event.accepted)options.close()
            }
            spacing:12
            // Measure every translated description at the actual panel width.
            // The labels overlap, so childrenRect reserves only the tallest.
            Item {
                id:descriptionSizer
                visible:false;width:options.contentWidth;height:0
                Repeater {
                    model:root.descriptionKeys
                    delegate:StyledTextLabel {
                        required property string modelData
                        width:descriptionSizer.width;wrapMode:Text.WordWrap
                        text:root.toolHelp(modelData)
                    }
                }
            }
            Flow {
                id:subtools
                Layout.fillWidth:true;spacing:6
                Repeater {
                    model:root.toolGroups[root.group]||[]
                    delegate:NativeDrawingIconButton {
                        required property var modelData
                        glyph:modelData.key;toolTipTitle:modelData.label;toolTipShortcut:root.shortcutFor(modelData.key)
                        enabled:root.group!=="combine"||!!root.editor&&(modelData.key==="ungroup"?root.editor.selectionIndices().some(function(index){return !!root.editor.shapes[index].selectionGroup}):root.editor.selectedShapes.length>1)
                        accentButton:root.editor&&(modelData.key==="eraser-partial"?root.editor.tool==="eraser"&&root.editor.eraserMode==="partial":modelData.key==="eraser"?root.editor.tool==="eraser"&&root.editor.eraserMode==="whole":root.editor.tool===modelData.key)
                        onHoveredChanged:settings.helpText=hovered?root.toolHelp(modelData.key):""
                        onClicked:{
                            if(modelData.key==="group")root.editor.groupSelected()
                            else if(modelData.key==="ungroup")root.editor.ungroupSelected()
                            else if(modelData.key==="fuse")root.editor.mergeSelected()
                            else root.selectTool(modelData.key)
                        }
                    }
                }
            }
            StyledFlickable {
                id:settingsScroll
                Layout.fillWidth:true;Layout.fillHeight:true
                contentWidth:width;contentHeight:settings.implicitHeight;flickableDirection:Flickable.VerticalFlick
                NativeDrawingSettings {
                    id:settings
                    width:settingsScroll.width-12
                    editor:root.editor;group:root.group
                    Component.onCompleted:DrawingSession.panel=settings
                    Component.onDestruction:if(DrawingSession.panel===settings)DrawingSession.panel=null
                }
            }
            StyledTextLabel {
                id:description;Layout.fillWidth:true;wrapMode:Text.WordWrap
                Layout.minimumHeight:options.descriptionHeight
                Layout.preferredHeight:options.descriptionHeight
                Layout.maximumHeight:options.descriptionHeight
                horizontalAlignment:Text.AlignLeft
                verticalAlignment:Text.AlignTop
                text:settings.helpText||(root.editor?root.toolHelp(root.editor.tool):"")
                color:ui.theme.fontPrimaryColor
            }
        }
    }
    Connections {
        target:DrawingSession
        function onModeActiveChanged(){if(!DrawingSession.modeActive)options.close()}
    }
    Connections {
        target:root.editor
        function onToolChanged(){
            var key=root.editor.tool
            if(key==="select"||key==="transform")root.selectionTool=key
            if(root.editor.isBasicShape(key))root.shapeTool=key
            if(["nodes","addnode","deletenode","join","anchors","unlink"].indexOf(key)>=0)root.pointTool=key
        }
        function onTextEditRequested(){options.close()}
    }
    Connections {
        target:root.Window.window
        function onXChanged(){if(options.isOpened)options.updatePosition()}
        function onYChanged(){if(options.isOpened)options.updatePosition()}
        function onScreenChanged(){if(options.isOpened)options.updatePosition()}
    }
    Connections {
        target:root.popupHost
        function onWidthChanged(){if(options.isOpened)options.updatePosition()}
        function onHeightChanged(){if(options.isOpened)options.updatePosition()}
    }
    Dialogs.FileDialog {
        id:svgDialog
        title:qsTrc("drawing-tools","Import SVG")
        fileMode:Dialogs.FileDialog.OpenFile
        nameFilters:[qsTrc("drawing-tools","SVG files (*.svg)")]
        selectedNameFilter.index:0;defaultSuffix:"svg"
        onAccepted:root.importSvg(selectedFile)
    }
    Dialogs.FileDialog {
        id:exportDialog
        title:qsTrc("drawing-tools","Export SVG")
        fileMode:Dialogs.FileDialog.SaveFile
        nameFilters:[qsTrc("drawing-tools","SVG files (*.svg)")]
        selectedNameFilter.index:0;defaultSuffix:"svg"
        onAccepted:if(root.editor&&DrawingSession.view)root.editor.status=DrawingSession.view.saveSvgDrawing(root.editor.exportSvg(),"",selectedFile)
    }
}

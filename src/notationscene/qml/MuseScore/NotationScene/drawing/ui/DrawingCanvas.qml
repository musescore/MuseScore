// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

import "../core/Vector.js" as Vector
import "../core/Reflow.js" as Reflow
import "../core/SvgAsset.js" as SvgAsset
import "../core/Editing.js" as Editing
import "../core/Painting.js" as Painting

Item {

    id: root

    property string tool: "pencil"
    property string transformMode:"scale"
    property bool pressureEnabled:false
    property real tabletPressure:1
    property bool tabletActive:false
    property bool pressureGesture:false
    property int pointerModifiers:Qt.NoModifier
    property string secondaryColor:"#ffffff"
    property bool secondaryActive:false
    readonly property string primaryColor:secondaryActive?secondaryColor:strokeColor
    readonly property string alternateColor:secondaryActive?strokeColor:secondaryColor
    function swapColors(){var saved=strokeColor;strokeColor=secondaryColor;secondaryColor=saved;secondaryActive=!secondaryActive}
    function selectPrimaryColor(){if(secondaryActive)swapColors()}
    function selectSecondaryColor(){if(!secondaryActive)swapColors()}
    function returnToSelection(){
        finishTextEditing();finishPen();finishInteractionForHistory()
        tool="select";setSelection([]);selectedHandle=null;selectionBox=null
        pendingBinding=null;snapTarget=null;requestPaint();forceActiveFocus()
    }
    // A passive handler observes genuine tablet pressure. Mouse strokes keep a
    // constant width and do not enter the pressure-outline path.
    PointHandler {
        acceptedDevices:PointerDevice.Stylus|PointerDevice.Airbrush
        acceptedButtons:Qt.LeftButton
        target:null
        function pointer(){return {x:point.position.x,y:point.position.y,button:Qt.LeftButton,buttons:Qt.LeftButton,modifiers:root.pointerModifiers}}
        onPointChanged:{
            if(point.pressure>0)root.tabletPressure=Math.max(0.01,Math.min(1,point.pressure))
            root.tabletActive=active
            if(active&&root.pressureGesture)input.moveInput(pointer())
        }
        onActiveChanged:{
            root.tabletActive=active
            if(active&&root.drawingEnabled&&root.pressureEnabled&&root.tool==="pencil"){
                root.tabletPressure=Math.max(0.01,point.pressure)
                root.pressureGesture=true;input.beginInput(pointer())
            }else if(!active&&root.pressureGesture){input.endInput(pointer());root.pressureGesture=false}
        }
    }
    readonly property bool selectionTool:tool==="select"||tool==="transform"
    readonly property bool paintingTool:Painting.active(tool)
    property string gradientStopId:"start"
    readonly property var currentGradient:Painting.descriptor(root)
    readonly property var gradientHandles:Painting.handles(root)
    readonly property string gradientStopColor:{
        var g=currentGradient;if(!g)return strokeColor;
        var stop=g.stops.find(function(stop){return stop.id===gradientStopId;});return stop?stop.color:strokeColor
    }
    function setGradientStopColor(colour){Painting.changeStop(root,colour,false,false)}
    function addGradientStop(){if(currentGradient&&currentGradient.stops.length<128)Painting.changeStop(root,"",true,false)}
    function removeGradientStop(){Painting.changeStop(root,"",false,true)}
    function paintSelection(){
        var indices=selectionIndices();if(!paintingTool||!indices.length)return
        var reference=shapes[selected],g=currentGradient
        if(Painting.gradientTool(tool)&&!g){var box=Vector.bounds(indices.map(function(index){return shapes[index]}));g=Painting.create(root,reference,{x:box.x,y:box.y+box.height/2},{x:box.x+box.width,y:box.y+box.height/2})}
        gestureStarting();remember();var originals=shapes.slice()
        if(Painting.channel(tool)==="fill")indices.forEach(function(index){var sh=originals[index],b=Vector.bounds([sh]);if(sh.activePaintRange===undefined)originals[index]=Painting.prepare(sh,b.x+b.width/2,b.y+b.height/2,root)||sh})
        shapes=Painting.applySet(root,originals,indices,tool,strokeColor,g,reference);changed();gestureFinished()
    }
    readonly property bool interactionActive:input.dragging
    readonly property bool anchorEditingActive:drawingEnabled&&(["nodes","anchors","addnode","deletenode","join","unlink"].indexOf(tool)>=0||(selectionTool&&switchStaff&&input.dragging))
    property bool pencilPreviewDirty:false
    property bool mirrorPointerVisible:false
    property real mirrorPointerX:0
    property real mirrorPointerY:0
    property real viewScale:1
    property rect paintRegion:Qt.rect(0,0,width,height)
    property bool contextualRhythmMarkers:false
    property string coordinateSpace:"page"
    function trStatus(source,pt){return coordinateSpace==="score"?qsTrc("drawing-tools",source):pt}
    property var svgHitTest:null
    property var svgRegionHitTest:null
    property var svgVectorize:null
    property var nativeFragments:[]
    readonly property string fragmentHandleId:selected>=0&&shapes[selected]&&!shapes[selected].svgSource?shapes[selected].drawId:""
    readonly property var fragmentHandleCache:Editing.buildFragmentHandles(nativeFragments,fragmentHandleId)
    property var nativeShapeHitTest:null
    property var nativeFragmentFingerprints:({})
    function hasNativeFragments(shape){
        if(!shape)return false
        var rendered=nativeFragmentFingerprints[shape.drawId]
        return rendered===shape||!!rendered&&(liveFragmentEdit||liveContinuationPreview)
    }
    // Keep delegate membership stable while geometry changes. Rebuilding a
    // filtered JS model destroys every Text/Image, including unchanged ones.
    property var shapeLookup:({})
    property var assetIds:[]
    property var textIds:[]
    function indexShapes(){
        var next={},assets=[],texts=[],old=shapeLookup
        shapes.forEach(function(shape,index){
            var id=shape.drawId+(shape.fragmentId?"/"+shape.fragmentId:"")
            var previous=old[id]
            next[id]=previous&&previous.shape===shape&&previous.index===index?previous:{shape:shape,index:index,bounds:previous&&previous.shape===shape?previous.bounds:Vector.bounds([shape])}
            if(shape.kind==="text")texts.push(id)
            else if(shape.svgSource)assets.push(id)
        })
        shapeLookup=next
        function same(a,b){return a.length===b.length&&a.every(function(id,index){return id===b[index]})}
        if(!same(assetIds,assets))assetIds=assets
        if(!same(textIds,texts))textIds=texts
    }
    // Navigation changes placement immediately; sharpen SVG textures once the
    // wheel settles instead of re-rasterizing every image on every wheel event.
    property real svgRasterScale:1
    onViewScaleChanged:if(drawingEnabled)svgQualityTimer.restart()
    Timer {id:svgQualityTimer;interval:140;onTriggered:root.svgRasterScale=root.viewScale}
    property bool nativeHistory:false
    signal nativeUndoRequested(bool redo)
    signal rhythmAnchorsRequested()
    readonly property real rasterScale:Math.min(Math.max(0.000001,viewScale),2,2048/Math.max(1,paintRegion.width,paintRegion.height))
    onPaintRegionChanged:requestPaint()
    function requestPaint(){if(drawingEnabled&&visible)drawingSurface.requestPaint()}
    onRasterScaleChanged:requestPaint()
    property string pointerMode:"dynamic"
    readonly property real effectiveStrokeWidth:Math.max(0.001,Math.min(1000000,
        pointerMode==="fixed"?strokeWidth/Math.max(0.02,viewScale):strokeWidth))

    property real smoothing: 0.7

    property real pointCurvature: 0.5

    property real dragResistance: 0
    property real systemContinuation: 0
    property real systemStartContinuation: 0

    function setSystemStartContinuation(value) {
        liveContinuationPreview=true
        systemStartContinuation=Math.max(0,Math.min(1,value))
        updateBreakExtensions()
    }

    function setSystemContinuation(value) {
        liveContinuationPreview=true
        systemContinuation=Math.max(0,Math.min(1,value))
        updateBreakExtensions()
    }

    function updateBreakExtensions(){
        // Native fragments already own their musical geometry. Changing the
        // viewing window must not rebuild their page-space command sequence.
        if(coordinateSpace==="score"){
            deformationSession=null;requestPaint();continuationChanged();return
        }
        if(shapes.some(function(shape){return !!shape.derivedWholeCommands})){
            shapes=shapes.map(function(shape){return Reflow.extend(shape,width,systemContinuation,systemStartContinuation)})
            deformationSession=null;changed();return
        }
        var old=shapes,updated=shapes.map(function(shape){return Reflow.extend(shape,width,systemContinuation,systemStartContinuation)})
        bindings=bindings.map(function(binding){
            var link=Vector.copy(binding)
            link.command+=(updated[link.shape].extensionStart?1:0)-(old[link.shape].extensionStart?1:0)
            var command=updated[link.shape].commands[link.command]
            if(command)link.field=command.length-2
            return link
        })
        shapes=updated;deformationSession=null;changed()
    }
    property var deformationSession: null
    readonly property var pointDeformationPreview:Editing.deformationFragments(root)

    property real strokeWidth: 30
    property real eraserSize:24
    property string eraserMode:"whole"
    property bool switchStaff:false
    property var targetStaffMarker:null
    property bool liveFragmentEdit:false
    property bool liveContinuationPreview:false
    property string labelText:"Text"
    property string labelFont:"Arial"
    property real labelSize:24
    property string labelPlacement:"above"
    signal exitRequested()
    signal textEditRequested()
    signal characterPickerRequested()
    property string editingTextId:""
    readonly property int editingTextIndex:shapes.findIndex(function(shape){return shape.drawId===root.editingTextId})
    readonly property bool textEditing:editingTextIndex>=0
    property var editingTextOriginal:null
    function beginTextEditing(index){
        var sh=shapes[index];if(!sh||sh.kind!=="text")return
        finishTextEditing()
        selected=index;editingTextOriginal=Vector.copy(sh);editingTextId=sh.drawId
        inlineText.text=sh.textContent;inlineText.font.family=sh.textFont||"Arial"
        inlineText.font.pixelSize=Math.max(1,Math.round(sh.textSize))
        inlineText.font.bold=!!(sh.textFontStyle&1);inlineText.font.italic=!!(sh.textFontStyle&2)
        inlineText.font.underline=!!(sh.textFontStyle&4);inlineText.font.strikeout=!!(sh.textFontStyle&8)
        textEditRequested()
        Qt.callLater(function(){inlineText.forceActiveFocus();inlineText.selectAll()})
    }
    readonly property var textEditorControl:inlineText
    function updateTextFormat(){
        if(!textEditing)return
        inlineFrame.formatRevision++
    }
    function textFontStyle(){return (inlineText.font.bold?1:0)|(inlineText.font.italic?2:0)|(inlineText.font.underline?4:0)|(inlineText.font.strikeout?8:0)}
    function finishTextEditing(){
        var index=editingTextIndex
        if(index<0)return
        var content=inlineText.text,old=shapes[index],family=inlineText.font.family,size=inlineText.font.pixelSize,format={fontStyle:textFontStyle(),alignment:inlineText.horizontalAlignment}
        editingTextId="";editingTextOriginal=null
        if((!old.tightGlyph&&/^(Leland|Bravura|Emmentaler|Gonville|MuseJazz|Petaluma)$/i.test(family))||old.textContent!==content||old.textFont!==family||old.textSize!==size||(old.textFontStyle||0)!==format.fontStyle||(old.textAlignment||Text.AlignLeft)!==format.alignment){remember();Editing.replaceText(root,index,content,family,size,format)}
        labelText=content;labelFont=family;labelSize=size
    }
    function cancelTextEditing(){editingTextId="";editingTextOriginal=null;forceActiveFocus()}
    function insertCharacter(text,family,x,y){
        var index=editingTextIndex>=0?editingTextIndex:selected
        var sh=shapes[index]
        if(sh&&sh.kind==="text"){
            if(!textEditing)beginTextEditing(index)
            var position=inlineText.selectionStart
            inlineText.remove(inlineText.selectionStart,inlineText.selectionEnd)
            inlineText.insert(position,text);inlineText.cursorPosition=position+text.length
            inlineText.font.family=family;inlineText.forceActiveFocus();return
        }
        var box={x:x,y:y,width:labelSize*2,height:labelSize*1.6}
        var characterBox=Editing.textBox(root,box);characterBox.tightGlyph=true
        append(characterBox);Editing.replaceText(root,selected,text,family,labelSize)
        labelFont=family;beginTextEditing(selected)
    }
    TextMetrics {id:labelMetrics;font.family:"Arial"}
    FontMetrics {id:labelFontMetrics;font:labelMetrics.font}
    function measureText(text,size,family){labelMetrics.text=text;labelMetrics.font.pixelSize=Math.max(1,Math.round(size));labelMetrics.font.family=family||"Arial";return labelMetrics.width}
    function measureTextLayout(text,size,family,fontStyle){
        labelMetrics.font.bold=!!(fontStyle&1);labelMetrics.font.italic=!!(fontStyle&2)
        measureText(text,size,family)
        var b=labelMetrics.tightBoundingRect
        return {inkX:b.x,inkY:b.y,inkWidth:b.width,inkHeight:b.height,width:Math.max(labelMetrics.advanceWidth,b.x+b.width),left:Math.min(0,b.x),
            ascent:Math.max(labelFontMetrics.ascent,-b.y),descent:Math.max(labelFontMetrics.descent,b.y+b.height),height:labelFontMetrics.height}
    }
    function mergeSelected(){Editing.merge(root)}
    function groupSelected(){
        var targets=selectionIndices();if(targets.length<2)return
        remember();var id="group-"+Vector.ensureIdentity({}),updated=shapes.slice()
        targets.forEach(function(index){updated[index]=Object.assign({},updated[index],{selectionGroup:id})})
        shapes=updated;setSelection(targets);changed()
    }
    function duplicateSelected(){
        var targets=selectionIndices();if(!targets.length)return
        gestureStarting();remember();var all=shapes.slice(),links=bindings.slice(),ids={},groups={},copies=[]
        var delta=12/Math.max(0.02,viewScale)
        targets.forEach(function(index){
            var old=shapes[index],copy=Vector.translate(Vector.copy(old),delta,delta)
            delete copy.drawId;delete copy.fragmentId;delete copy.parentDrawId
            delete copy.reflowSource;delete copy.reflowGroup;delete copy.matrixId
            ids[old.drawId]=Vector.ensureIdentity(copy)
            if(old.selectionGroup){if(!groups[old.selectionGroup])groups[old.selectionGroup]="group-"+copy.drawId;copy.selectionGroup=groups[old.selectionGroup]}
            var next=all.length
            transformBindings(old,copy,bindings.filter(function(b){return b.shape===index}),index).forEach(function(b){var c=Object.assign({},b);c.shape=next;links.push(c)})
            copies.push(next);all.push(copy)
        })
        copies.forEach(function(index){var link=all[index].textLink;if(!link)return
            if(link.kind==="shape"&&ids[link.drawId])link.drawId=ids[link.drawId]
            else{link.dx+=delta;link.dy+=delta}
        })
        shapes=all;bindings=links;setSelection(copies);changed();gestureFinished()
    }
    function ungroupSelected(){
        var targets=selectionIndices()
        if(!targets.some(function(index){return !!shapes[index].selectionGroup}))return
        remember();var updated=shapes.slice()
        targets.forEach(function(index){var copy=Object.assign({},updated[index]);delete copy.selectionGroup;updated[index]=copy})
        shapes=updated;setSelection(targets);changed()
    }
    function selectionMembers(index){
        var shape=shapes[index],group=shape?shape.selectionGroup:null
        if(!group)return index>=0?[index]:[]
        var members=[];shapes.forEach(function(sh,i){if(sh.selectionGroup===group)members.push(i)});return members
    }
    function updateLabel(){Editing.updateText(root)}
    function addLabelToSelection(){var target=shapes[selected];if(target){append(Editing.label(root,0,0,target));beginTextEditing(selected)}}
    function isBasicShape(key){return ["line","rectangle","ellipse","triangle","right-triangle","arrow","arrow-double","arrow-filled"].indexOf(key)>=0}
    function reanchorMovedSelection(targets){
        if(!targets.length||!rhythmMarkers.length)return
        var moving={},ids={},updated=shapes.slice(),links=[]
        targets.forEach(function(index){moving[index]=true;ids[shapes[index].drawId]=true})
        bindings.forEach(function(link){if(!moving[link.shape])links.push(link)})
        targets.forEach(function(index){
            var sh=updated[index]
            // Text moved together with its parent retains that explicit link.
            if(sh.textLink&&sh.textLink.kind==="shape"&&ids[sh.textLink.drawId])return
            if(sh.textLink){sh=Object.assign({},sh);delete sh.textLink;updated[index]=sh}
            var handles
            if(sh.svgSource&&!sh.singleAnchor){
                var box=Vector.bounds([sh]);sh=Object.assign({},sh)
                sh.commands=sh.commands.concat([["M",box.x+box.width/2,box.y+box.height/2]])
                sh.singleAnchor=true;sh.anchorCommand=sh.commands.length-1;updated[index]=sh
            }
            handles=Vector.linkedHandles(sh,bindings,index)
            var heads=Vector.rigidArrowHeads(sh),manualHeads={}
            bindings.forEach(function(link){if(link.shape===index&&link.headManual)manualHeads[link.command]=true})
            handles.forEach(function(handle){
                if(heads[handle.command]&&!manualHeads[handle.command])return
                var target=sh.singleAnchor?nearestCreationAnchor(handle.x,handle.y):nearbyAnchor(handle.x,handle.y,null,Infinity,true)
                if(!target)return
                var scale=target.scale||1
                links.push({shape:index,command:handle.command,field:handle.field,editable:true,key:target.key,headManual:!!manualHeads[handle.command],
                    numerator:target.numerator,denominator:target.denominator,scale:scale,
                    offsetX:(handle.x-(target.bindingX===undefined?target.x:target.bindingX))/scale,
                    markerOffsetX:target.bindingX===undefined?0:target.x-target.bindingX,offsetY:(handle.y-target.y)/scale})
            })
        })
        deformationSession=null;shapes=updated;bindings=links
    }
    function linkSingleAnchor(index){
        var sh=shapes[index],command=sh.anchorCommand===undefined?5:sh.anchorCommand,c=sh.commands[command],target=c?nearestCreationAnchor(c[1],c[2]):null
        if(!target)return
        var old=selected,oldSelection=selectionIndices();settingSelection=true
        try{selected=index;bindHandle({command:command,field:1},target);selected=old}finally{settingSelection=false}
        selectedShapes=oldSelection
    }

    property real strokeOpacity: 1

    property string strokeColor: "#161616"

    property string fillColor: "#ffffff"

    property bool filled: false

    property bool drawingEnabled: true

    property var shapes: []

    property var history: []

    property var future: []

    property var preview: null

    property var penNodes: []

    property int selected: -1
    property var selectedShapes: []
    property bool settingSelection:false
    property var selectionBox: null
    onSelectedChanged:{
        if(!settingSelection)selectedShapes=selected>=0?[selected]:[]
        var sh=shapes[selected]
        if(sh&&sh.kind==="text"){labelText=sh.textContent;labelSize=sh.textSize;labelFont=sh.textFont||"Arial";labelPlacement=sh.textPlacement||"above"}
        requestPaint()
    }
    function selectionIndices(){
        return (selectedShapes.length?selectedShapes:(selected>=0?[selected]:[])).filter(function(index){return index>=0&&index<shapes.length})
    }
    function setSelection(indices,primary){
        var expanded=[]
        indices.forEach(function(index){selectionMembers(index).forEach(function(member){if(expanded.indexOf(member)<0)expanded.push(member)})})
        var valid=expanded.filter(function(index){return index>=0&&index<shapes.length})
        settingSelection=true
        try{selected=valid.indexOf(primary)>=0?primary:(valid.length?valid[valid.length-1]:-1)}finally{settingSelection=false}
        selectedShapes=valid;requestPaint()
    }
    function selectionBounds(){return Vector.bounds(selectionIndices().map(function(index){return shapes[index]}))}

    property string status: ""

    property string anchorMode: "note"
    property string anchorGridMode:"subdivision"

    property int subdivision: 8

    property var selectedHandle: null

    property var bindings: []

    property var rhythmMarkers: []

    property var pendingBinding: null
    property var snapTarget: null
    property real anchorSnapRadius: 12

    signal documentChanged()
    signal continuationChanged()
    signal gestureStarting()
    signal gestureFinished()
    function finishInteractionForHistory(){
        if(input.eraseDirty)input.flushEraser()
        var dirty=input.dragging&&input.mutated
        input.dragging=false;preview=null;pendingBinding=null;snapTarget=null
        if(dirty)changed();else requestPaint()
    }

    focus: true

    function setPointCurvature(value) {

        pointCurvature=Math.max(0,Math.min(1,value))

        var edit=deformationSession

        if(!edit||selected!==edit.shape||selected<0||selected>=shapes.length)return
        if(shapes[selected].singleAnchor)return

        var a=shapes.slice()

        a[selected]=Vector.deform(edit.original,edit.handle,edit.handles,edit.dx,edit.dy,pointCurvature,edit.plan)

        shapes=a

        bindings=edit.bindings.map(function(b){
            if(b.shape!==edit.shape)return b
            var c=Object.assign({},b)

            if(c.shape===edit.shape&&c.scale>0){

                var before=edit.original.commands[c.command],after=a[edit.shape].commands[c.command]

                if(before&&after){c.offsetX+=(after[c.field]-before[c.field])/c.scale;

                    c.offsetY+=(after[c.field+1]-before[c.field+1])/c.scale}

            }

            return c

        })

        changed()

    }

    function anchorPageAt(x,y){
        var page=null,distance=Infinity
        anchorPages.forEach(function(m){
            var dx=Math.max(m.pageX-x,0,x-m.pageX-m.pageWidth)
            var dy=Math.max(m.pageY-y,0,y-m.pageY-m.pageHeight)
            var d=Math.hypot(dx,dy)
            if(d<distance){distance=d;page=m.page}
        })
        return page
    }
    function nearestCreationAnchor(x,y){
        var page=anchorPageAt(x,y),target=null,distance=Infinity
        rhythmMarkers.forEach(function(m){
            if(page!==null&&m.page!==page)return
            var d=Math.hypot(m.x-x,(m.railY===undefined?m.y:m.railY)-y)
            if(d<distance){distance=d;target=m}
        })
        return target
    }
    function nearbyAnchor(x,y,handle,radius,gridOnly) {
        if(!rhythmMarkers.length)return null
        // Keep the staff of the current link even when artwork sits far above it.
        // Moving a node keeps its staff; the explicit attachment tool may
        // choose another staff without moving/remapping any other node.
        var referenceY=handle&&tool!=="anchors"?handle.y:y,referenceRow=null
        if(handle&&tool!=="anchors")for(var i=0;i<bindings.length;i++){
            var link=bindings[i]
            if(link.shape!==selected||link.command!==handle.command||link.field!==handle.field)continue
            var reference=markerForBinding(link)
            if(reference){referenceY=reference.y;referenceRow=reference.row}
            break
        }
        var rows=(gridOnly?gridMarkerTable:markerTable).groups,list=null,rowDistance=Infinity,page=anchorPageAt(x,y)
        Object.keys(rows).forEach(function(key){
            var row=rows[key],dy=Math.abs(row[0].y-referenceY)
            if(referenceRow!==null&&row[0].row!==referenceRow)return
            if(referenceRow===null&&page!==null&&row[0].page!==page)return
            var dx=Math.max(row[0].x-x,0,x-row[row.length-1].x)
            var distance=Math.hypot(dx,dy)
            if(distance<rowDistance){
                list=row;rowDistance=distance
            }
        })
        if(!list)return null
        var target=null,distance=radius===undefined?anchorSnapRadius/Math.max(0.000001,viewScale):radius
        var low=0,high=list.length
        while(low<high){var middle=Math.floor((low+high)/2);if(list[middle].x<x)low=middle+1;else high=middle}
        for(var index=Math.max(0,low-1);index<=Math.min(list.length-1,low);index++){
            var m=list[index]
            var d=Math.abs(m.x-x)
            if(d<distance){target=m;distance=d}
        }
        return target
    }
    function bindHandle(handle,target) {
        var head=Vector.rigidArrowHeads(shapes[selected])[handle.command]
        var manual=head&&(tool==="anchors"||bindings.some(function(link){return link.shape===selected&&link.command===handle.command&&link.headManual}))
        if(head&&!manual)return
        if(shapes[selected].unlinkedPoints){
            var all=shapes.slice(),shape=Vector.copy(all[selected])
            delete shape.unlinkedPoints[handle.command+":"+handle.field]
            if(!Object.keys(shape.unlinkedPoints).length){delete shape.unlinkedPoints;delete shape.suppressAutoAnchors}
            all[selected]=shape;shapes=all
        }
        var command=shapes[selected].commands[handle.command],scale=target.scale||1
        var link={shape:selected,command:handle.command,field:handle.field,editable:true,headManual:!!manual,
            key:target.key,numerator:target.numerator,denominator:target.denominator,
            scale:scale,offsetX:(command[handle.field]-(target.bindingX===undefined?target.x:target.bindingX))/scale,
            markerOffsetX:target.bindingX===undefined?0:target.x-target.bindingX,
            offsetY:(command[handle.field+1]-target.y)/scale}
        bindings=bindings.filter(function(b){return !(b.shape===selected&&b.command===handle.command&&b.field===handle.field)}).concat([link])
        // Slider re-evaluation must retain the new destination, not restore the old link.
        if(deformationSession){
            var edit=Object.assign({},deformationSession),base=Vector.copy(link)
            var original=edit.original.commands[handle.command]
            base.offsetX=(original[handle.field]-(target.bindingX===undefined?target.x:target.bindingX))/scale
            base.offsetY=(original[handle.field+1]-target.y)/scale
            edit.bindings=edit.bindings.filter(function(b){return !(b.shape===selected&&b.command===handle.command&&b.field===handle.field)}).concat([base])
            deformationSession=edit
        }
    }
    function currentStyle() { return Vector.style(strokeColor,filled?strokeColor:"none",effectiveStrokeWidth,strokeOpacity) }

    function reflowHorizontal(origin,ratio) {
        if(!isFinite(ratio)||ratio<=0||Math.abs(ratio-1)<0.000001)return
        function remap(snapshot){
            var out=Vector.copy(snapshot)
            out.shapes.forEach(function(sh,index){
                var rigid=(sh.anchorMode||root.anchorMode)==="note"
                var first=sh.commands[0],shift=first?(first[1]-origin)*(ratio-1):0
                sh.commands.forEach(function(c){for(var n=1;n<c.length;n+=2)
                    c[n]=rigid?c[n]+shift:origin+(c[n]-origin)*ratio})
                if(sh.rawPoints)sh.rawPoints.forEach(function(p){p.x=rigid?p.x+shift:origin+(p.x-origin)*ratio})
            })
            out.bindings.forEach(function(b){
                if(typeof b.offsetX==="number")b.offsetX*=ratio
            })
            return out
        }
        var current=remap(snapshot())
        history=history.map(remap);future=future.map(remap)
        if(deformationSession){
            var edit=Vector.copy(deformationSession),base=remap({shapes:[edit.original],bindings:edit.bindings})
            edit.original=base.shapes[0];edit.bindings=base.bindings;edit.dx*=ratio
            edit.handle.x=origin+(edit.handle.x-origin)*ratio
            edit.handles.forEach(function(h){h.x=origin+(h.x-origin)*ratio})
            deformationSession=edit
        }
        shapes=current.shapes;bindings=current.bindings;selectedHandle=null;changed()
    }
    function markerMeasureKey(key){
        var text=String(key),colon=text.lastIndexOf(":"),prefix=colon<0?"":text.slice(0,colon+1),time=text.slice(colon+1)
        var match=/^(-?\d+)\/([1-9]\d*)$/.exec(time)
        if(!match)return text
        var numerator=Number(match[1]),denominator=Number(match[2]),a=Math.abs(numerator),b=denominator
        while(b){var remainder=a%b;a=b;b=remainder}
        var divisor=a||1
        return prefix+(numerator/divisor)+"/"+(denominator/divisor)
    }
    function markerIdentity(p){
        if(!p||p.key===undefined||!(Number(p.denominator)>0))return ""
        // Fractions coming back from the score may have a different denominator.
        return markerMeasureKey(p.key)+":"+(Number(p.numerator)/Number(p.denominator)).toFixed(12)
    }
    function buildGridTable(grid){
        var groups={},indices={},markers={},byKey={}
        grid.forEach(function(p){
            var row=p.row===undefined?String(p.y):p.row,key=markerMeasureKey(p.key)
            if(!groups[row])groups[row]=[]
            if(!byKey[key])byKey[key]=[]
            groups[row].push(p);byKey[key].push(p);markers[markerIdentity(p)]=p
        })
        Object.keys(groups).forEach(function(row){
            groups[row].sort(function(a,b){return a.x-b.x})
            groups[row].forEach(function(p,index){indices[markerIdentity(p)]={row:row,index:index}})
        })
        return {groups:groups,indices:indices,markers:markers,byKey:byKey}
    }
    readonly property var anchorPages:{
        var seen={},pages=[]
        rhythmMarkers.forEach(function(m){
            if(m.page!==undefined&&!seen[m.page]&&m.pageWidth>0&&m.pageHeight>0){seen[m.page]=true;pages.push(m)}
        })
        return pages
    }
    readonly property var gridMarkerTable:buildGridTable(rhythmMarkers)
    function buildMarkerTable(grid,links,drawings){
        var base=gridMarkerTable,groups=null,indices=null,markers=null,changedRows={}
        // The common path reuses the musical grid. Only attachments between
        // subdivisions need a new entry, and only their rows need sorting.
        links.forEach(function(link){
            var id=markerIdentity(link),shape=drawings[link.shape]
            if(link.editable!==true||!id||(markers||base.markers)[id]||!shape||!shape.commands||!(link.scale>0))return
            var command=shape.commands[link.command]
            if(!command||!isFinite(link.offsetX)||!isFinite(link.offsetY))return
            var x=command[link.field]-link.offsetX*link.scale+(link.markerOffsetX||0)
            var y=command[link.field+1]-link.offsetY*link.scale
            if(!isFinite(x)||!isFinite(y))return
            var nearest=null,distance=Infinity
            ;(base.byKey[markerMeasureKey(link.key)]||[]).forEach(function(p){
                var d=Math.abs(p.x-x)+Math.abs(p.y-y)
                if(d<distance){nearest=p;distance=d}
            })
            if(!nearest)return
            if(!groups){groups=Object.assign({},base.groups);indices=Object.create(base.indices);markers=Object.create(base.markers)}
            var p={key:link.key,numerator:link.numerator,denominator:link.denominator,
                 x:x,y:y,railY:y+(nearest.railY===undefined?0:nearest.railY-nearest.y),
                 row:nearest.row,scale:link.scale,page:nearest.page,pageX:nearest.pageX,pageY:nearest.pageY,
                 pageWidth:nearest.pageWidth,pageHeight:nearest.pageHeight}
            var row=p.row===undefined?String(p.y):p.row
            if(!changedRows[row]){groups[row]=(groups[row]||[]).slice();changedRows[row]=true}
            groups[row].push(p);markers[id]=p
        })
        if(!groups)return base
        Object.keys(changedRows).forEach(function(row){
            groups[row].sort(function(a,b){return a.x-b.x})
            groups[row].forEach(function(p,index){indices[markerIdentity(p)]={row:row,index:index}})
        })
        return {groups:groups,indices:indices,markers:markers}
    }
    readonly property var markerTable:buildMarkerTable(rhythmMarkers,bindings,shapes)
    function markerForBinding(link){return markerTable.markers[markerIdentity(link)]||null}
    function rhythmGuide(){
        if(!anchorEditingActive)return {points:[],segments:[]}
        if(!contextualRhythmMarkers)return {points:rhythmMarkers,segments:[]}
        var groups=markerTable.groups,indices=markerTable.indices,weights={},activeRows={},segments=[],points=[]
        function reveal(marker){
            var at=indices[root.markerIdentity(marker)]
            if(!at)return
            activeRows[at.row]=true
            var neighbors=groups[at.row],fade=[1,0.65,0.35,0.12]
            for(var n=-3;n<=3;n++){
                var p=neighbors[at.index+n]
                if(p)weights[root.markerIdentity(p)]=Math.max(weights[root.markerIdentity(p)]||0,fade[Math.abs(n)])
            }
        }
        var related=selectionIndices()
        function includes(index){return tool==="join"||related.indexOf(index)>=0}
        bindings.forEach(function(link){
            if(includes(link.shape)&&shapes[link.shape]&&link.editable===true)reveal(link)
        })
        if(targetStaffMarker)reveal(targetStaffMarker)
        shapes.forEach(function(shape,index){
            if(!includes(index))return
            if(shape.textLink&&shape.textLink.kind==="rhythm")reveal(shape.textLink)
            Object.keys(shape.fragmentOverrides||{}).forEach(function(key){
                var f=shape.fragmentOverrides[key]
                if(f.start&&f.start.link)reveal(f.start.link)
                if(f.end&&f.end.link)reveal(f.end.link)
            })
        })
        if(input.dragging&&selectedHandle&&["nodes","anchors"].indexOf(tool)>=0){
            var near=nearbyAnchor(input.pointerX,input.pointerY,selectedHandle,Infinity)
            if(near)reveal(near)
        }
        Object.keys(activeRows).forEach(function(row){
            var list=groups[row]
            list.forEach(function(p,index){
                var alpha=weights[root.markerIdentity(p)]||0,py=p.railY===undefined?p.y:p.railY
                if(alpha)points.push({x:p.x,y:py,opacity:alpha})
                if(index+1<list.length){
                    var next=list[index+1],other=weights[root.markerIdentity(next)]||0
                    if(alpha||other)segments.push({x:p.x,y:py,x2:next.x,
                        y2:next.railY===undefined?next.y:next.railY,startOpacity:alpha,endOpacity:other})
                }
            })
        })
        return {points:points,segments:segments}
    }
    readonly property var rhythmPresentation:rhythmGuide()
    function markerRailY(marker){return marker&&marker.railY!==undefined?marker.railY:marker?marker.y:0}
    function snapshot() { return {shapes:Vector.copy(shapes),bindings:Vector.copy(bindings)} }

    signal reflowGroupEdited(string group)
    function clearReflowGroup(group){
        if(!group)return
        shapes=shapes.map(function(shape){
            if(shape.reflowGroup!==group)return shape
            var copy=Vector.copy(shape)
            if(copy.fragmentId){copy.parentDrawId=Vector.ensureIdentity(copy);copy.drawId=Vector.svgIdentity(copy);delete copy.fragmentId}
            delete copy.reflowSource;delete copy.reflowGroup;delete copy.matrixId;delete copy.derivedWholeCommands;delete copy.clipRect;return copy
        })
    }
    function invalidateReflow(index){
        var shape=shapes[index],group=shape?shape.reflowGroup:null
        if(group){clearReflowGroup(group);reflowGroupEdited(group)}
    }
    function remember(preserveReflow) {
        if(nativeHistory)return
        // Taking a history snapshot must not detach unrelated fragments.
        history=history.concat([snapshot()]);future=[]
    }

    function changed() { Editing.followLabels(root);requestPaint();documentChanged() }

    function autoLink(index) {
        if(shapes[index].singleAnchor){linkSingleAnchor(index);return}
        invalidateReflow(index)

        var linked=Vector.autoAnchors(shapes[index],rhythmMarkers,index,bindings.filter(function(b){return b.shape===index&&b.headManual})),a=shapes.slice()

        delete linked.shape.reflowSource;delete linked.shape.reflowGroup
        delete linked.shape.unlinkedPoints;delete linked.shape.suppressAutoAnchors
        a[index]=linked.shape;shapes=a

        bindings=bindings.filter(function(b){return b.shape!==index}).concat(linked.bindings)

    }

    function linkSelection(){
        var targets=selectedShapes.length?selectedShapes.slice():(selected>=0?[selected]:[])
        if(!targets.length){status=root.trStatus("Select a drawing to attach to rhythm.","Selecione um desenho para vincular ao ritmo.");return}
        remember()
        targets.forEach(function(index){autoLink(index)})
        status=root.trStatus("Drawings attached to rhythmic staff positions.","Desenhos vinculados às posições rítmicas da pauta.")
        changed()
    }
    function editHandles() {

        if(selected<0||selected>=shapes.length)return []

        if(shapes[selected].singleAnchor){var ci=shapes[selected].anchorCommand===undefined?5:shapes[selected].anchorCommand,c=shapes[selected].commands[ci];return c?[{command:ci,field:1,x:c[1],y:c[2],endpoint:true}]:[]}
        var handles=Vector.linkedHandles(shapes[selected],bindings,selected)
        var ends=contextualRhythmMarkers?Editing.fragmentHandles(root):[]
        return ends.length?handles.filter(function(h){return !h.endpoint}).concat(ends):handles

    }
    property var joiningPoint:null
    function pointAt(x,y){
        var best=null,distance=12/Math.max(0.000001,viewScale)
        for(var i=shapes.length-1;i>=0;i--){
            var handles=Vector.linkedHandles(shapes[i],bindings,i)
            handles.forEach(function(h){
                var d=Math.hypot(h.x-x,h.y-y)
                if(d<distance){distance=d;best={shape:i,handle:h}}
            })
        }
        return best
    }
    function deletePointAt(x,y){
        var hit=pointAt(x,y)
        if(!hit){status=root.trStatus("Click a visible point to delete it.","Clique em um ponto visível para excluir.");return}
        selected=hit.shape
        var handles=editHandles(),at=hit.handle.command
        var previous=null,next=null
        handles.forEach(function(h){if(h.command<at)previous=h;else if(h.command>at&&!next)next=h})
        var shape=Vector.copy(shapes[selected]),commands=shape.commands
        // Find this continuous contour; never splice across a separate path.
        var start=at,end=at
        while(start>0&&commands[start][0]!=="M")start--
        while(end+1<commands.length&&commands[end+1][0]!=="M"&&commands[end+1][0]!=="Z")end++
        if(previous&&previous.command<start)previous=null
        if(next&&next.command>end)next=null
        var from,to,replacement=[],mapping={}
        if(at===start&&next){
            from=start;to=next.command
            var c=commands[to];replacement=[["M",c[c.length-2],c[c.length-1]]]
            mapping[to]=from
        }else if(at===end&&previous){
            from=previous.command+1;to=end
        }else if(previous&&next){
            from=previous.command+1;to=next.command
            var outgoing=commands[from],incoming=commands[to]
            var target={x:incoming[incoming.length-2],y:incoming[incoming.length-1]}
            if(outgoing[0]==="C"&&incoming[0]==="C")
                replacement=[["C",outgoing[1],outgoing[2],incoming[3],incoming[4],target.x,target.y]]
            else replacement=[["L",target.x,target.y]]
            mapping[to]=from
        }else{status=root.trStatus("There is no other point in this section.","Não há outro ponto neste trecho.");return}
        remember();invalidateReflow(selected)
        var shift=replacement.length-(to-from+1)
        commands.splice.apply(commands,[from,to-from+1].concat(replacement))
        if(shape.paintRanges)shape.paintRanges=shape.paintRanges.map(function(range){var r=Object.assign({},range);r.start=r.start>to?r.start+shift:r.start>=from?from:r.start;r.end=r.end>to?r.end+shift:r.end>=from?from+replacement.length:r.end;return r}).filter(function(range){return range.end>range.start})
        if(shape.anchorCommand!==undefined&&shape.anchorCommand>to)shape.anchorCommand+=shift
        delete shape.rawPoints;delete shape.reflowSource;delete shape.reflowGroup
        var updated=shapes.slice();updated[selected]=shape;shapes=updated
        bindings=bindings.filter(function(b){return b.shape!==selected||b.command<from||b.command>to||typeof mapping[b.command]!=="undefined"}).map(function(b){
            var link=Vector.copy(b)
            if(link.shape===selected){
                if(typeof mapping[link.command]!=="undefined")link.command=mapping[link.command]
                else if(link.command>to)link.command+=shift
                link.field=commands[link.command].length-2
            }
            return link
        })
        selectedHandle=null;deformationSession=null;changed()
    }
    function joinPointAt(x,y){
        var hit=pointAt(x,y)
        if(!hit){status=root.trStatus("Click a drawing endpoint.","Clique na extremidade de um desenho.");return}
        var commands=shapes[hit.shape].commands,last=commands.length-1
        if(commands.some(function(c,i){return c[0]==="Z"||(i>0&&c[0]==="M");})||
            (hit.handle.command!==0&&hit.handle.command!==last)){
            status=root.trStatus("Choose the endpoint of an open line to join drawings.","Para unir desenhos, escolha uma extremidade de uma linha aberta.");return
        }
        if(!joiningPoint){joiningPoint=hit;selected=hit.shape;status=root.trStatus("Now click the endpoint of the second drawing.","Agora clique na extremidade do segundo desenho.");requestPaint();return}
        var first=joiningPoint
        joiningPoint=null
        if(first.shape===hit.shape){status=root.trStatus("Choose two different drawings.","Escolha dois desenhos diferentes.");return}
        function oriented(index,reverse){
            var shape=Vector.copy(shapes[index]),original=shape.commands,map={}
            if(!reverse){original.forEach(function(c,i){map[i]=i});return {shape:shape,map:map}}
            var lastCommand=original[original.length-1],out=[["M",lastCommand[lastCommand.length-2],lastCommand[lastCommand.length-1]]]
            map[original.length-1]=0
            for(var i=original.length-1;i>0;i--){
                var c=original[i],before=original[i-1],px=before[before.length-2],py=before[before.length-1]
                out.push(c[0]==="C"?["C",c[3],c[4],c[1],c[2],px,py]:["L",px,py]);map[i-1]=out.length-1
            }
            shape.commands=out;return {shape:shape,map:map}
        }
        var a=oriented(first.shape,first.handle.command===0),b=oriented(hit.shape,hit.handle.command!==0)
        var offset=a.shape.commands.length,move=b.shape.commands[0]
        b.shape.commands[0]=["L",move[1],move[2]]
        a.shape.commands=a.shape.commands.concat(b.shape.commands)
        delete a.shape.rawPoints;delete a.shape.reflowSource;delete a.shape.reflowGroup
        remember();invalidateReflow(first.shape);invalidateReflow(hit.shape)
        var remaining=[],indices={},joined=-1
        shapes.forEach(function(sh,i){
            if(i===hit.shape)return
            indices[i]=remaining.length
            if(i===first.shape){joined=remaining.length;remaining.push(a.shape)}else remaining.push(sh)
        })
        var links=bindings.map(function(binding){
            var link=Vector.copy(binding)
            if(link.shape===first.shape||link.shape===hit.shape){
                link.command=link.shape===first.shape?a.map[link.command]:offset+b.map[link.command]
                link.shape=joined;link.field=a.shape.commands[link.command].length-2
            }else link.shape=indices[link.shape]
            return link
        })
        ;[offset-1,offset].forEach(function(command){
            if(!links.some(function(link){return link.shape===joined&&link.command===command}))
                links.push({shape:joined,command:command,field:a.shape.commands[command].length-2,editable:true,key:"",numerator:0,denominator:1})
        })
        shapes=remaining;bindings=links;selected=joined;deformationSession=null
        status=root.trStatus("Drawings joined. The first drawing keeps its style.","Desenhos unidos. O estilo do primeiro desenho foi mantido.");changed()
    }

    function append(sh) { Vector.ensureIdentity(sh);sh.anchorMode=sh.anchorMode||anchorMode;deformationSession=null;remember();shapes=shapes.concat([sh]);selected=shapes.length-1;autoLink(selected);changed() }
    function importSvg(text,x,y){
        finishPen()
        append(SvgAsset.create(text,x,y,Math.min(width*0.45,400/Math.max(0.02,viewScale))))
        tool="transform";status=root.trStatus("SVG imported. Use the handles to transform it and the centre point to rotate.","SVG importado. Use as alças para transformar e o ponto central para girar.")
    }
    function transformHandles(){
        if(tool!=="transform"||!selectionIndices().length)return []
        var b=selectionBounds(),out=[]
        ;[[0,0],[0.5,0],[1,0],[1,0.5],[1,1],[0.5,1],[0,1],[0,0.5]].forEach(function(p){out.push({u:p[0],v:p[1],x:b.x+b.width*p[0],y:b.y+b.height*p[1],rotate:false})})
        out.push({x:b.x+b.width/2,y:b.y+b.height/2,rotate:true});return out
    }
    function arrowControlHandles(){
        if(!drawingEnabled||(!selectionTool&&tool!=="nodes")||selectionIndices().length!==1)return []
        var shape=shapes[selected]
        // Fragment endpoints have their own reflow controls; do not reinterpret
        // a clipped fragment as a complete parametric arrow.
        if(!shape||shape.fragmentId||shape.derivedWholeCommands)return []
        return Vector.arrowControls(shape,viewScale)
    }
    function transformBindings(before,after,originalLinks,index){
        return originalLinks.map(function(binding){
            if(binding.shape!==index)return binding
            var b=Object.assign({},binding),old=before.commands[b.command],now=after.commands[b.command]
            if(b.shape===index&&b.scale>0&&old&&now){b.offsetX+=(now[b.field]-old[b.field])/b.scale;b.offsetY+=(now[b.field+1]-old[b.field+1])/b.scale}
            return b
        })
    }
    function mirrorSelected(horizontal){
        var targets=selectedShapes.length?selectedShapes.slice():(selected>=0?[selected]:[])
        if(!targets.length)return
        remember();var links=Vector.copy(bindings),b=selectionBounds(),a=shapes.slice()
        targets.forEach(function(index){
            invalidateReflow(index)
        })
        a=shapes.slice()
        var m=horizontal?[-1,0,0,1,2*b.x+b.width,0]:[1,0,0,-1,0,2*b.y+b.height]
        targets.forEach(function(index){var before=shapes[index];a[index]=Vector.transform(before,m);links=transformBindings(before,a[index],links,index)})
        updateSelectionTextLinks(shapes,a,targets);shapes=a
        bindings=links;changed()
    }
    function updateSelectionTextLinks(before,after,targets){
        var moved={};targets.forEach(function(index){moved[before[index].drawId]=index})
        targets.forEach(function(index){
            var old=before[index],sh=after[index],link=sh.textLink
            if(sh.kind!=="text"||!link||!old.commands[5]||!sh.commands[5])return
            var dx=sh.commands[5][1]-old.commands[5][1],dy=sh.commands[5][2]-old.commands[5][2]
            if(link.kind==="shape"&&moved[link.drawId]!==undefined){
                var parent=moved[link.drawId],a=before[parent].commands[link.command],b=after[parent].commands[link.command]
                if(a&&b){dx-=b[link.field]-a[link.field];dy-=b[link.field+1]-a[link.field+1]}
            }
            link.dx+=dx;link.dy+=dy
        })
    }

    function undo() {
        if(nativeHistory){nativeUndoRequested(false);return}

        if(!history.length)return

        deformationSession=null

        future=future.concat([snapshot()]);var old=history[history.length-1];shapes=old.shapes;bindings=old.bindings

        history=history.slice(0,-1);selected=-1;changed()

    }

    function redo() {
        if(nativeHistory){nativeUndoRequested(true);return}

        if(!future.length)return

        deformationSession=null

        history=history.concat([snapshot()]);var old=future[future.length-1];shapes=old.shapes;bindings=old.bindings

        future=future.slice(0,-1);selected=-1;changed()

    }

    function clear() { if(shapes.length){remember();shapes=[]}bindings=[];preview=null;penNodes=[];selected=-1;changed() }

    function eraseAt(x,y,quiet){
        if(eraserMode==="partial"&&shapes.some(function(sh){return sh.kind==="text"||!!sh.svgSource||sh.style.fill!=="none"})){
            return Editing.eraseRegion(root,Editing.region("ellipse",{x:x-eraserSize/2,y:y-eraserSize/2},{x:x+eraserSize/2,y:y+eraserSize/2}),quiet)
        }
        var results=[],touched=false
        for(var i=0;i<shapes.length;i++){
            var shape=shapes[i],entry=shapeLookup[shape.drawId+(shape.fragmentId?"/"+shape.fragmentId:"")]
            var box=entry&&entry.shape===shape?entry.bounds:Vector.bounds([shape]),reach=eraserSize/2
            var nativeHit=eraserMode==="whole"&&nativeShapeHitTest?nativeShapeHitTest(shape,x,y,reach):null
            if(nativeHit!==null){var erased=nativeHit?{shape:null,mapping:{}}:null;results.push(erased);if(erased)touched=true;continue}
            if(x+reach<box.x||x-reach>box.x+box.width||y+reach<box.y||y-reach>box.y+box.height){results.push(null);continue}
            var result
            if(shape.svgSource&&eraserMode==="whole"){
                var hit=nativeShapeHitTest?nativeShapeHitTest(shape,x,y,reach):null
                if(hit===null)hit=svgHitTest?svgHitTest(shape,x,y,reach):Vector.hit([shape],x,y,reach)>=0
                result=hit?{shape:null,mapping:{}}:null
            }else result=Vector.eraseStroke(shape,x,y,reach,eraserMode==="whole")
            results.push(result);if(result)touched=true
        }
        if(!touched)return false
        var updated=[],links=[],indices={}
        results.forEach(function(result,i){
            if(result)root.invalidateReflow(i)
            var target=result?result.shape:root.shapes[i]
            if(target){indices[i]=updated.length;updated.push(target)}
        })
        bindings.forEach(function(b){
            if(indices[b.shape]===undefined)return
            var result=results[b.shape],link=Object.assign({},b)
            if(result){
                var mapped=result.mapping[b.command]
                if(!mapped||b.field!==shapes[b.shape].commands[b.command].length-2)return
                link.command=mapped.command;link.field=mapped.field
            }
            link.shape=indices[b.shape];links.push(link)
        })
        shapes=updated;bindings=links;selected=-1;selectedShapes=[];deformationSession=null;if(!quiet)changed();return true
    }

    function removeSelected() {
        var removed=selectedShapes.length?selectedShapes.slice():(selected>=0?[selected]:[])
        if(!removed.length)return
        remember();removed.forEach(function(index){invalidateReflow(index)});deformationSession=null
        var mapping={},a=[]
        shapes.forEach(function(sh,index){if(removed.indexOf(index)<0){mapping[index]=a.length;a.push(sh)}})
        bindings=bindings.filter(function(b){return removed.indexOf(b.shape)<0}).map(function(b){
            var copy=Vector.copy(b);copy.shape=mapping[b.shape];return copy
        })
        shapes=a;selected=-1;selectedShapes=[];selectionBox=null;changed()
    }
    function selectBox(box,base){
        if(!box)return
        var matches=base?base.slice():[]
        shapes.forEach(function(sh,index){
            var bounds=Vector.bounds([sh])
            if(bounds.x<=box.x+box.width&&bounds.x+bounds.width>=box.x&&
               bounds.y<=box.y+box.height&&bounds.y+bounds.height>=box.y&&matches.indexOf(index)<0)matches.push(index)
        })
        setSelection(matches)
    }
    function finishPen() {

        if(penNodes.length>1)append({kind:"pen",commands:Vector.pen(penNodes),style:currentStyle(),fillRule:"evenodd"})

        penNodes=[];preview=null;requestPaint()

    }

    function exportSvg() { return Vector.svg(shapes) }

    function exportDocument() {

        shapes.forEach(function(shape){Vector.ensureIdentity(shape);if(Vector.arrowGeometry(shape))shape.rigidArrowHeads=Vector.rigidArrowHeads(shape)})
        var d={format:"musescore-drawing",version:1,shapes:shapes}

        d.strokeColor=strokeColor;d.secondaryColor=secondaryColor;d.secondaryActive=secondaryActive;d.smoothing=smoothing;d.pressureEnabled=pressureEnabled;d.strokeOpacity=strokeOpacity;d.filled=filled;d.coordinateSpace=coordinateSpace;d.pointerMode=pointerMode;d.strokeWidth=strokeWidth;d.systemStartContinuation=systemStartContinuation;d.systemContinuation=systemContinuation;d.dragResistance=dragResistance;d.pointCurvature=pointCurvature;d.anchors=bindings;d.matrices={};shapes.forEach(function(shape){if(shape.matrixId&&shape.reflowSource)d.matrices[shape.matrixId]=Vector.copy(shape.reflowSource)});d.anchorMode=anchorMode;d.anchorGridMode=anchorGridMode;d.subdivision=subdivision;d.bounds=drawingBounds()

        d.anchors=d.anchors.filter(function(link){return !shapes[link.shape].rigidArrowHeads||!shapes[link.shape].rigidArrowHeads[link.command]||link.headManual})
        return JSON.stringify(d)

    }

    function drawingBounds() { return Vector.bounds(shapes) }

    function importDocument(text) {

        var a=Vector.readDocument(text),d=JSON.parse(text)
        a.forEach(function(shape){if(Vector.arrowGeometry(shape))shape.rigidArrowHeads=Vector.rigidArrowHeads(shape)})
        d.anchors=(d.anchors||[]).filter(function(link){return !a[link.shape]||!a[link.shape].rigidArrowHeads||!a[link.shape].rigidArrowHeads[link.command]||link.headManual})
        if(d.strokeColor!==undefined)strokeColor=Vector.color(d.strokeColor,"#161616")
        if(d.secondaryColor!==undefined)secondaryColor=Vector.color(d.secondaryColor,"#ffffff")
        if(d.secondaryActive!==undefined)secondaryActive=!!d.secondaryActive
        if(d.smoothing!==undefined)smoothing=Math.max(0,Math.min(1,d.smoothing))
        if(d.pressureEnabled!==undefined)pressureEnabled=!!d.pressureEnabled
        if(d.strokeOpacity!==undefined)strokeOpacity=Math.max(0,Math.min(1,d.strokeOpacity))
        if(d.filled!==undefined)filled=!!d.filled
        if(d.anchorGridMode!==undefined)anchorGridMode=d.anchorGridMode==="note"?"note":"subdivision"
        pointerMode=d.pointerMode==="fixed"?"fixed":"dynamic"
        if(typeof d.strokeWidth==="number"&&isFinite(d.strokeWidth))strokeWidth=Math.max(1,Math.min(1000,d.strokeWidth))
        systemContinuation=typeof d.systemContinuation==="number"?Math.max(0,Math.min(1,d.systemContinuation)):0
        systemStartContinuation=typeof d.systemStartContinuation==="number"?Math.max(0,Math.min(1,d.systemStartContinuation)):0

        dragResistance=typeof d.dragResistance==="number"?Math.max(0,Math.min(1,d.dragResistance)):0

        deformationSession=null;pointCurvature=typeof d.pointCurvature==="number"?Math.max(0,Math.min(1,d.pointCurvature)):0.5

        remember(true);shapes=a;bindings=d.anchors||[];anchorMode=d.anchorMode==="rhythm"?"rhythm":"note"

        subdivision=[4,8,16,32].indexOf(d.subdivision)>=0?d.subdivision:8;selected=-1;changed()

    }

    function updateStyle() {

        if(selected<0)return

        remember();var a=shapes.slice(),before=a[selected]
        a[selected]=Vector.arrowWithStyle(Vector.copy(before),currentStyle())
        if(a[selected].reflowSource)a[selected].reflowSource.shape=Vector.arrowWithStyle(a[selected].reflowSource.shape,a[selected].style)
        bindings=transformBindings(before,a[selected],bindings,selected);shapes=a;changed()

    }

    function setSmoothing(value) {
        // Pencil settings affect future strokes only, never stored geometry.
        smoothing=Math.max(0,Math.min(1,value))
    }

    function addNode(x,y){

        var point=selected>=0?Vector.nearestSegment(shapes[selected],x,y):null

        if(!point){

            var nearest=Infinity, index=-1

            for(var i=shapes.length-1;i>=0;i--){

                var candidate=Vector.nearestSegment(shapes[i],x,y)

                if(candidate&&candidate.distance<nearest){nearest=candidate.distance;point=candidate;index=i}

            }

            if(index>=0)selected=index

        }

        if(!point){status=root.trStatus("Click the stroke to add a point.","Clique sobre o traço para adicionar um ponto.");return}

        deformationSession=null

        remember();invalidateReflow(selected);var a=shapes.slice();a[selected]=Vector.copy(a[selected]);var old=a[selected].commands[point.command]

        var insertedCount=1

        if(old[0]==="Z"){

            var start=point.command-1

            while(start>0&&a[selected].commands[start][0]!=="M")start--

            var move=a[selected].commands[start]

            a[selected].commands.splice(point.command,1,["L",move[1],move[2]],["Z"])

            insertedCount=2

        }

        a[selected]=Vector.insertPoint(a[selected],point.command,point.t)
        if(a[selected].kind==="fused"){
            a[selected].singleAnchor=false;a[selected].anchorMode="rhythm";a[selected].suppressAutoAnchors=true
        }
        delete a[selected].reflowSource;delete a[selected].reflowGroup

        bindings=bindings.filter(function(b){

            return !(b.shape===selected&&b.command===point.command&&old[0]==="C"&&b.field<5)

        }).map(function(b){

            var c=Vector.copy(b);if(c.shape===selected&&c.command>=point.command)c.command+=insertedCount;return c

        })

        var node=a[selected].commands[point.command],field=node.length-2

        bindings=bindings.concat([{shape:selected,command:point.command,field:field,editable:true,manual:true}])

        shapes=a;selectedHandle={command:point.command,field:field,x:node[field],y:node[field+1]}

        status=root.trStatus("Point added. You can edit it or attach it to the score.","Ponto adicionado. Você pode editá-lo ou vinculá-lo à partitura.");changed()

    }

    onToolChanged: {
        joiningPoint=null

        finishPen();preview=null;selectedHandle=null;pendingBinding=null;snapTarget=null
        if(["nodes","anchors","addnode","deletenode","join"].indexOf(tool)>=0)rhythmAnchorsRequested()

        if(tool==="nodes")status=root.trStatus("Select a drawing and drag a node or handle. Add point creates new stroke nodes.","Selecione a figura e arraste um nó ou uma alça. Adicionar ponto cria novos nós no traço.")

        requestPaint()

    }

    onShapesChanged:{indexShapes();requestPaint()}
    onBindingsChanged:requestPaint()
    onRhythmPresentationChanged:requestPaint()
    onAnchorEditingActiveChanged:requestPaint()
    onNativeFragmentsChanged:requestPaint()
    onDrawingEnabledChanged:requestPaint()

    function toolShortcut(event){
        if(!drawingEnabled||textEditing||(event.modifiers&(Qt.ControlModifier|Qt.AltModifier|Qt.MetaModifier)))return ""
        var map={};map[Qt.Key_V]=(event.modifiers&Qt.ShiftModifier)?"transform":"select"
        var shift=!!(event.modifiers&Qt.ShiftModifier)
        var pointTools=["nodes","addnode","deletenode","join","anchors","unlink"]
        map[Qt.Key_A]=shift?pointTools[(pointTools.indexOf(tool)+1)%pointTools.length]:"nodes"
        map[Qt.Key_N]="pencil";map[Qt.Key_P]="pen"
        var erasers=["eraser","eraser-partial","erase-rectangle","erase-ellipse","erase-triangle"]
        var currentEraser=tool==="eraser"&&eraserMode==="partial"?"eraser-partial":tool
        var forms=["rectangle","ellipse","line","triangle","right-triangle","arrow","arrow-double","arrow-filled"]
        map[Qt.Key_E]=shift?erasers[(erasers.indexOf(currentEraser)+1)%erasers.length]:"eraser"
        map[Qt.Key_M]=shift?forms[(forms.indexOf(tool)+1)%forms.length]:"rectangle";map[Qt.Key_L]="ellipse"
        map[Qt.Key_T]=shift?"characters":"text";map[Qt.Key_X]="swap-colors"
        map[Qt.Key_B]=shift?"paint-stroke":"paint-fill";map[Qt.Key_G]=shift?"gradient-stroke":"gradient-fill"
        return map[event.key]||""
    }
    function activateToolShortcut(key){
        if(key==="swap-colors"){swapColors();return}
        finishPen()
        if(key==="eraser"||key==="eraser-partial"){eraserMode=key==="eraser"?"whole":"partial";tool="eraser"}
        else tool=key
        forceActiveFocus()
    }
    Keys.onShortcutOverride:function(event){
        if(drawingEnabled&&!textEditing&&activeFocus&&(event.key===Qt.Key_Delete||event.key===Qt.Key_Escape||event.key===Qt.Key_Return||event.key===Qt.Key_Enter||toolShortcut(event)!==""||((event.modifiers&Qt.ControlModifier)&&(event.key===Qt.Key_Z||event.key===Qt.Key_Y||event.key===Qt.Key_D))))event.accepted=true
    }
    Keys.onReleased:function(event){pointerModifiers=event.modifiers}
    Keys.onPressed:function(event){handleDrawingKey(event)}
    function handleDrawingKey(event) {
        pointerModifiers=event.modifiers

        if(!drawingEnabled||textEditing)return
        if(event.key===Qt.Key_Escape){returnToSelection();event.accepted=true}
        else if(toolShortcut(event)!==""){activateToolShortcut(toolShortcut(event));event.accepted=true}
        else if((event.modifiers&Qt.ControlModifier)&&event.key===Qt.Key_D){duplicateSelected();event.accepted=true}

        else if(event.key===Qt.Key_Return||event.key===Qt.Key_Enter){finishPen();event.accepted=true}

        else if(event.key===Qt.Key_Delete){removeSelected();event.accepted=true}

        else if((event.modifiers&Qt.ControlModifier)&&event.key===Qt.Key_Z){if(event.modifiers&Qt.ShiftModifier)redo();else undo();event.accepted=true}

        else if((event.modifiers&Qt.ControlModifier)&&event.key===Qt.Key_Y){redo();event.accepted=true}

    }

    Timer {
        interval:16;repeat:true;running:root.pencilPreviewDirty&&input.dragging
        onTriggered:{
            root.pencilPreviewDirty=false
            root.preview=Vector.pencilStroke(input.points,input.gestureSmoothing,root.currentStyle(),root.viewScale,input.points.some(function(p){return p.pressure!==undefined}))
            root.requestPaint()
        }
    }
    Timer {interval:16;repeat:true;running:input.dragging&&input.eraseDirty;onTriggered:input.flushEraser()}
    Repeater {
        model:root.drawingEnabled?root.nativeFragments:[]
        delegate:Image {
            required property var modelData
            readonly property var entry:root.shapeLookup[modelData.id]
            readonly property int shapeIndex:entry?entry.index:-1
            visible:!!entry&&entry.shape.kind!=="text"&&root.hasNativeFragments(entry.shape)
                &&x+width>=root.paintRegion.x&&x<=root.paintRegion.x+root.paintRegion.width
                &&y+height>=root.paintRegion.y&&y<=root.paintRegion.y+root.paintRegion.height
            x:modelData.x;y:modelData.y;width:modelData.width;height:modelData.height
            source:"data:image/svg+xml,"+encodeURIComponent(!root.paintingTool&&root.selectedShapes.indexOf(shapeIndex)>=0?SvgAsset.tinted(modelData.svg):modelData.svg)
            retainWhileLoading:true
            sourceSize.width:Math.min(2048,Math.max(1,Math.ceil(width*root.svgRasterScale)))
            sourceSize.height:Math.min(2048,Math.max(1,Math.ceil(height*root.svgRasterScale)))
        }
    }
    Repeater {
        model:root.assetIds
        delegate:Item {
            required property var modelData
            readonly property var entry:root.shapeLookup[modelData]
            readonly property var shape:entry?entry.shape:null
            readonly property var m:shape?shape.svgMatrix:[1,0,0,1,0,0]
            readonly property var rect:shape&&shape.clipRect?shape.clipRect:{x:0,y:0,width:root.width,height:root.height}
            visible:!!shape&&!root.hasNativeFragments(shape)
            x:rect.x;y:rect.y;width:rect.width;height:rect.height;clip:true
            Image {
                width:1;height:1
                source:parent.visible?"data:image/svg+xml,"+encodeURIComponent(!root.paintingTool&&root.selectedShapes.indexOf(entry.index)>=0?SvgAsset.tinted(shape.svgSource):shape.svgSource):""
                opacity:shape?shape.style.opacity:1
                sourceSize.width:root.interactionActive?256:Math.min(2048,Math.max(64,Math.ceil(Math.hypot(m[0],m[1])*root.svgRasterScale)))
                sourceSize.height:root.interactionActive?256:Math.min(2048,Math.max(64,Math.ceil(Math.hypot(m[2],m[3])*root.svgRasterScale)))
                transform:Matrix4x4 {matrix:Qt.matrix4x4(m[0],m[2],0,m[4]-rect.x,m[1],m[3],0,m[5]-rect.y,0,0,1,0,0,0,0,1)}
            }
        }
    }
    Canvas {
        id:drawingSurface
        x:root.paintRegion.x;y:root.paintRegion.y
        width:Math.max(1,Math.ceil(root.paintRegion.width*root.rasterScale))
        height:Math.max(1,Math.ceil(root.paintRegion.height*root.rasterScale))
        scale:1/root.rasterScale
        transformOrigin:Item.TopLeft
        onWidthChanged:requestPaint()
        onHeightChanged:requestPaint()
    onPaint: {

        var ctx=getContext("2d");ctx.reset();ctx.clearRect(0,0,canvasSize.width,canvasSize.height);ctx.scale(root.rasterScale,root.rasterScale)
        ctx.translate(-root.paintRegion.x,-root.paintRegion.y)
        var region=root.paintRegion,padding=12/root.viewScale
        function lineOnScreen(x,y,x2,y2){
            return Math.max(x,x2)>=region.x-padding&&Math.min(x,x2)<=region.x+region.width+padding
                &&Math.max(y,y2)>=region.y-padding&&Math.min(y,y2)<=region.y+region.height+padding
        }

        var deformation=root.pointDeformationPreview
        Vector.paint(ctx,shapes.filter(function(shape){return !root.hasNativeFragments(shape)&&(!deformation||shape!==deformation.shape)}),root.paintingTool?[]:selectedShapes.map(function(index){return shapes[index]}))
        if(deformation)Vector.paint(ctx,deformation.pieces,deformation.pieces)

        if(preview)Vector.paint(ctx,[preview])
        if(root.drawingEnabled&&Painting.gradientTool(root.tool)&&root.selected>=0&&root.currentGradient){
            var g=root.gradientHandles,a=g[0],b=g[1];ctx.strokeStyle="#20aaad";ctx.lineWidth=2/root.viewScale
            ctx.beginPath();ctx.moveTo(a.x,a.y);ctx.lineTo(b.x,b.y);ctx.stroke()
            g.forEach(function(handle){
                var r=5/root.viewScale;ctx.lineWidth=(handle.id===root.gradientStopId?2.5:1.5)/root.viewScale;ctx.strokeStyle=handle.id===root.gradientStopId?"#20aaad":"white"
                if(handle.role==="stop"){
                    ctx.beginPath();ctx.moveTo(handle.axisX,handle.axisY);ctx.lineTo(handle.x,handle.y);ctx.stroke()
                    ctx.fillStyle=handle.color;ctx.fillRect(handle.x-r,handle.y-r,2*r,2*r);ctx.strokeRect(handle.x-r,handle.y-r,2*r,2*r)
                }else{ctx.fillStyle="#20aaad";ctx.beginPath();ctx.arc(handle.x,handle.y,r,0,Math.PI*2);ctx.fill();ctx.stroke()}
            })
        }
        selectedShapes.forEach(function(index){
            if(root.selectionTool||index===selected||!shapes[index])return
            var box=Vector.bounds([shapes[index]])
            ctx.strokeStyle="#398df1";ctx.lineWidth=1;ctx.strokeRect(box.x,box.y,box.width,box.height)
        })
        if(selectionBox){
            ctx.fillStyle="#398df122";ctx.strokeStyle="#398df1";ctx.lineWidth=1
            ctx.fillRect(selectionBox.x,selectionBox.y,selectionBox.width,selectionBox.height)
            ctx.strokeRect(selectionBox.x,selectionBox.y,selectionBox.width,selectionBox.height)
        }

        if(!root.selectionTool&&selected>=0&&selected<shapes.length){

            var box=Vector.bounds([shapes[selected]]);ctx.strokeStyle="#398df1";ctx.lineWidth=3/root.viewScale

            ctx.strokeRect(box.x,box.y,box.width,box.height)

        }
        if(root.drawingEnabled){
            var pointTools=["nodes","anchors","addnode","deletenode","join"].indexOf(tool)>=0
            if(root.anchorEditingActive){
                var guide=root.rhythmPresentation
                guide.segments.forEach(function(line){
                    if(!lineOnScreen(line.x,line.y,line.x2,line.y2))return
                    var gradient=ctx.createLinearGradient(line.x,line.y,line.x2,line.y2)
                    gradient.addColorStop(0,Qt.rgba(0.83,0.62,0.89,line.startOpacity))
                    gradient.addColorStop(1,Qt.rgba(0.83,0.62,0.89,line.endOpacity))
                    ctx.strokeStyle=gradient;ctx.lineWidth=1.5/root.viewScale
                    ctx.beginPath();ctx.moveTo(line.x,line.y);ctx.lineTo(line.x2,line.y2);ctx.stroke()
                })
                guide.points.forEach(function(p){
                    if(p.x<root.paintRegion.x-10/root.viewScale||p.x>root.paintRegion.x+root.paintRegion.width+10/root.viewScale||p.y<root.paintRegion.y-10/root.viewScale||p.y>root.paintRegion.y+root.paintRegion.height+10/root.viewScale)return
                    ctx.globalAlpha=p.opacity===undefined?1:p.opacity
                    ctx.fillStyle="#c32bdb";ctx.strokeStyle="white";ctx.lineWidth=2/root.viewScale
                    ctx.beginPath();ctx.arc(p.x,p.y,6/root.viewScale,0,Math.PI*2);ctx.fill();ctx.stroke()
                });ctx.globalAlpha=1
                bindings.forEach(function(link){
                    if(tool!=="join"&&selectedShapes.indexOf(link.shape)<0)return
                    if(contextualRhythmMarkers&&link.editable!==true)return
                    var shape=shapes[link.shape],c=shape?shape.commands[link.command]:null,target=markerForBinding(link)
                    if(!c||!target)return
                    if(!lineOnScreen(c[link.field],c[link.field+1],target.x,root.markerRailY(target)))return
                    ctx.strokeStyle="#d637ef";ctx.lineWidth=2/root.viewScale
                    ctx.beginPath();ctx.moveTo(c[link.field],c[link.field+1]);ctx.lineTo(target.x,root.markerRailY(target));ctx.stroke()
                })
            }
            if(pointTools){
                var handles=tool==="join"?shapes.reduce(function(all,shape,index){return all.concat(Vector.linkedHandles(shape,bindings,index))},[]):editHandles()
                handles.forEach(function(h){
                    if(!lineOnScreen(h.x,h.y,h.x,h.y))return
                    ctx.fillStyle=h.derived?"#19bfa5":h.endpoint?"#ffae42":"#398df1"
                    ctx.strokeStyle="white";ctx.lineWidth=2/root.viewScale
                    ctx.beginPath();ctx.arc(h.x,h.y,6/root.viewScale,0,Math.PI*2);ctx.fill();ctx.stroke()
                })
            }
            if(root.anchorEditingActive)Editing.bindingLines(root,false).forEach(function(line){
                if(tool!=="join"&&selectedShapes.indexOf(line.shape)<0)return
                if(!lineOnScreen(line.x,line.y,line.x2,line.y2))return
                ctx.strokeStyle=line.kind==="text"?"#00b8b0":"#d637ef";ctx.lineWidth=2/root.viewScale
                ctx.beginPath();ctx.moveTo(line.x,line.y);ctx.lineTo(line.x2,line.y2);ctx.stroke()
            })
        }

        if(snapTarget){
            ctx.strokeStyle="#26a269";ctx.lineWidth=3;ctx.beginPath()
            ctx.arc(snapTarget.x,root.markerRailY(snapTarget),8/root.viewScale,0,Math.PI*2);ctx.stroke()
        }
        penNodes.forEach(function(p) {

            ctx.strokeStyle="#398df1";ctx.lineWidth=1;ctx.beginPath()

            ctx.moveTo(p.x-p.dx,p.y-p.dy);ctx.lineTo(p.x+p.dx,p.y+p.dy);ctx.stroke()

            ctx.fillStyle=p.close?"#19bfa5":"#398df1";ctx.strokeStyle="white";ctx.lineWidth=1.5/root.viewScale;ctx.beginPath();ctx.arc(p.x,p.y,4/root.viewScale,0,Math.PI*2);ctx.fill();ctx.stroke()

        })

        if(pendingBinding){ctx.strokeStyle="#b347d3";ctx.beginPath();ctx.moveTo(pendingBinding.x,pendingBinding.y);ctx.lineTo(pendingBinding.mouseX,pendingBinding.mouseY);ctx.stroke()}
    }
    }
    Text {
        visible:root.drawingEnabled&&root.tool==="unlink"&&input.containsMouse&&Editing.lineAt(root,input.pointerX,input.pointerY)!==null
        x:input.pointerX-width/2;y:input.pointerY-height/2;text:"×";color:"#f43737";font.pixelSize:26/root.viewScale;z:12
    }

    Item {
        visible:root.drawingEnabled&&root.tool==="transform"&&root.selectedShapes.length>0
        anchors.fill:parent
        property var box:root.selectionBounds()
        Rectangle {x:parent.box.x;y:parent.box.y;width:parent.box.width;height:parent.box.height;color:"#10398df1";border.color:"#2675e6";border.width:3/root.viewScale}
        Repeater {
            model:parent.visible?root.transformHandles():[]
            delegate:Rectangle {
                required property var modelData
                x:modelData.x-width/2;y:modelData.y-height/2
                width:(modelData.rotate?18:10)/root.viewScale;height:width
                radius:modelData.rotate?width/2:0;color:modelData.rotate?"#ffffff":"#2675e6"
                border.color:modelData.rotate?"#2675e6":"white";border.width:2/root.viewScale
                Text {visible:modelData.rotate;anchors.centerIn:parent;text:"↻";color:"#2675e6";font.pixelSize:13/root.viewScale}
            }
        }
    }
    Item {
        anchors.fill:parent;z:11
        Repeater {
            model:root.arrowControlHandles()
            delegate:Item {
                required property var modelData
                x:modelData.x;y:modelData.y
                Rectangle {
                    x:modelData.targetX-modelData.x;y:modelData.targetY-modelData.y
                    width:Math.hypot(modelData.x-modelData.targetX,modelData.y-modelData.targetY)
                    height:1.5/root.viewScale;color:modelData.color;opacity:0.65
                    rotation:Math.atan2(modelData.y-modelData.targetY,modelData.x-modelData.targetX)*180/Math.PI
                    transformOrigin:Item.TopLeft
                }
                Rectangle {
                    width:12/root.viewScale;height:width;x:-width/2;y:-height/2;rotation:45
                    radius:2/root.viewScale;color:modelData.color
                    border.color:"white";border.width:2/root.viewScale
                }
            }
        }
    }
    MouseArea {

        id: input

        anchors.fill: parent

        enabled: root.drawingEnabled

        acceptedButtons: Qt.LeftButton|Qt.RightButton

        hoverEnabled: true
        onWheel:function(event){event.accepted=false}

        cursorShape: (root.tool==="pencil"||root.tool==="pen"||root.tool==="eraser"||(root.tool==="unlink"&&Editing.lineAt(root,pointerX,pointerY)))?Qt.BlankCursor:
            (root.selectionTool?Qt.ArrowCursor:Qt.CrossCursor)
        property real pointerX:0
        property real pointerY:0

        property string gestureTool: ""
        property real gestureSmoothing: 0.7

        property var points: []

        property var first: null

        property var original: null
        property var originalHandles:[]
        property var deformationPlan:null
        property var transformHandle:null
        property var arrowHandle:null
        property var gradientHandle:null
        property var transformBox:null
        property var rotationAngle:null
        property var selectionOriginals:({})
        property var selectionOrder:[]
        property var boxSelectionBase:[]
        function captureSelection(){
            selectionOrder=root.selectionIndices();var originals={}
            selectionOrder.forEach(function(index){originals[index]=Vector.copy(root.shapes[index])})
            selectionOriginals=originals;originalBindings=root.bindings.slice()
        }
        function beginSelectionChange(){
            if(mutated)return
            root.remember()
            selectionOrder.forEach(function(index){root.invalidateReflow(index)})
            captureSelection();mutated=true
        }
        function applySelectionMatrix(matrix,translation){
            var updated=root.shapes.slice(),before=root.shapes.slice()
            selectionOrder.forEach(function(index){
                before[index]=selectionOriginals[index]
                updated[index]=translation?Vector.translate(before[index],matrix[4],matrix[5]):Vector.transform(before[index],matrix)
            })
            root.updateSelectionTextLinks(before,updated,selectionOrder)
            root.bindings=originalBindings.map(function(link){
                var old=selectionOriginals[link.shape]
                if(!old||!(link.scale>0))return link
                var a=old.commands[link.command],b=updated[link.shape].commands[link.command]
                if(!a||!b)return link
                return Object.assign({},link,{offsetX:link.offsetX+(b[link.field]-a[link.field])/link.scale,
                    offsetY:link.offsetY+(b[link.field+1]-a[link.field+1])/link.scale})
            })
            root.shapes=updated
        }

        property bool dragging: false

        property bool mutated: false
        property bool eraseDirty:false
        property var erasePosition:null
        function flushEraser(){
            if(!eraseDirty||!erasePosition)return
            eraseDirty=false
            var p=erasePosition,dx=p.x-first.x,dy=p.y-first.y,length=Math.hypot(dx,dy)
            var steps=Math.max(1,Math.min(16,Math.ceil(length/Math.max(1,root.eraserSize/4)))),touched=false
            for(var e=1;e<=steps;e++)touched=root.eraseAt(first.x+dx*e/steps,first.y+dy*e/steps,true)||touched
            first=p;if(touched)root.changed()
        }

        property var originalBindings:[]
        property bool boxSelecting:false

        onPressed:function(mouse){if(!root.pressureGesture)beginInput(mouse)}
        function beginInput(mouse) {

            pointerX=mouse.x;pointerY=mouse.y
            root.finishTextEditing()
            root.forceActiveFocus()
            if(mouse.button===Qt.LeftButton)root.gestureStarting()
            boxSelecting=false;root.selectionBox=null;transformHandle=null;arrowHandle=null;gradientHandle=null;rotationAngle=null
            selectionOrder=[];selectionOriginals={};boxSelectionBase=[]

            if(mouse.button===Qt.RightButton){root.finishPen();return}

            root.snapTarget=null;root.deformationSession=null;gestureTool=root.tool;gestureSmoothing=root.smoothing;points=[];original=null;originalHandles=[];deformationPlan=null;first={x:mouse.x,y:mouse.y};dragging=true;mutated=false
            eraseDirty=false;erasePosition=null
            root.liveFragmentEdit=false;root.targetStaffMarker=null
            if(root.paintingTool){
                var reach=10/root.viewScale
                root.gradientHandles.slice().reverse().forEach(function(handle){var distance=Math.hypot(mouse.x-handle.x,mouse.y-handle.y);if(distance<reach){reach=distance;gradientHandle=handle}})
                if(gradientHandle){
                    if(gradientHandle.role==="stop")root.gradientStopId=gradientHandle.id
                    captureSelection();original=selectionOriginals[root.selected];root.requestPaint();return
                }
                var painted=Painting.hit(root,mouse.x,mouse.y)
                if(painted<0){dragging=false;return}
                if(root.selectionIndices().indexOf(painted)<0)root.setSelection([painted],painted)
                captureSelection();original=selectionOriginals[root.selected]
                if(Painting.channel(root.tool)==="fill"){
                    var seed=Painting.fraction(original,first);
                    selectionOrder.forEach(function(index){var sh=selectionOriginals[index],p=index===root.selected?first:Painting.position(sh,seed);selectionOriginals[index]=Painting.prepare(sh,p.x,p.y,root)||sh})
                    original=selectionOriginals[root.selected]
                }
                if(!Painting.gradientTool(root.tool)){
                    root.remember();mutated=true
                    root.shapes=Painting.applySet(root,selectionOriginals,selectionOrder,root.tool,root.strokeColor,null,original)
                }
                root.requestPaint();return
            }
            if(root.tool==="unlink"){dragging=false;Editing.unlink(root,Editing.lineAt(root,mouse.x,mouse.y));return}
            if(root.tool==="text"||root.tool==="characters"){
                var existing=Vector.hit(root.shapes,mouse.x,mouse.y,0,root.svgHitTest,root.nativeShapeHitTest)
                if(existing>=0&&root.shapes[existing].kind==="text"){
                    dragging=false;root.beginTextEditing(existing)
                    if(root.tool==="characters")root.characterPickerRequested()
                }
                return
            }

            var arrowControls=root.arrowControlHandles(),arrowDistance=9/Math.max(0.000001,root.viewScale)
            arrowControls.forEach(function(handle){
                var distance=Math.hypot(handle.x-mouse.x,handle.y-mouse.y)
                if(distance<arrowDistance){arrowDistance=distance;arrowHandle=handle}
            })
            if(arrowHandle){captureSelection();return}

            if(root.tool==="transform"&&root.selected>=0){
                var controls=root.transformHandles(),nearest=11/Math.max(0.000001,root.viewScale)
                controls.forEach(function(handle){var distance=Math.hypot(handle.x-mouse.x,handle.y-mouse.y);if(distance<nearest){nearest=distance;transformHandle=handle}})
                if(transformHandle&&transformHandle.rotate){
                    var underneath=Vector.hit(root.shapes,mouse.x,mouse.y,6/Math.max(0.000001,root.viewScale),root.svgHitTest,root.nativeShapeHitTest)
                    if(underneath>=0&&root.selectedShapes.indexOf(underneath)<0)transformHandle=null
                }
                if(transformHandle){
                    captureSelection();transformBox=root.selectionBounds();return
                }
            }

            if(root.tool==="deletenode"||root.tool==="join"){
                dragging=false
                if(root.tool==="deletenode")root.deletePointAt(mouse.x,mouse.y)
                else root.joinPointAt(mouse.x,mouse.y)
                return
            }
            if(root.tool==="eraser"){
                root.remember();mutated=true;root.eraseAt(mouse.x,mouse.y)
            }else if(root.tool==="pen"){

                var a=root.penNodes.slice(),last=a[a.length-1],reach=8/root.viewScale
                // Clicking the last node manipulates it; it never adds the
                // phantom continuation that used to precede a double-click.
                if(!last||Math.hypot(mouse.x-last.x,mouse.y-last.y)>reach){
                    var start=a.length-1;while(start>0&&!a[start].move)start--
                    var firstNode=a[start],close=last&&!last.close&&a.length-start>2&&Math.hypot(mouse.x-firstNode.x,mouse.y-firstNode.y)<=reach
                    a.push({x:close?firstNode.x:mouse.x,y:close?firstNode.y:mouse.y,dx:0,dy:0,close:!!close,move:!!(last&&last.close)})
                }
                root.penNodes=a;root.preview={commands:Vector.pen(a),style:root.currentStyle(),fillRule:"evenodd"}

            }else if(root.tool==="pencil"){if(root.pressureEnabled&&root.tabletActive)first.pressure=root.tabletPressure;points=[first]}

            else if(root.selectionTool||root.tool==="nodes"||root.tool==="anchors"||root.tool==="addnode") {

                root.selectedHandle=null

                if((root.tool==="nodes"||root.tool==="anchors")&&root.selected>=0){

                    var hs=root.editHandles(),distance=10/Math.max(0.000001,root.viewScale)

                    for(var i=0;i<hs.length;i++){

                        var d=Math.hypot(hs[i].x-mouse.x,hs[i].y-mouse.y)

                        if(d<distance){distance=d;root.selectedHandle=hs[i]}

                    }

                }

                if(!root.selectedHandle){

                    var hit=Vector.hit(root.shapes,mouse.x,mouse.y,6/Math.max(0.000001,root.viewScale),root.svgHitTest,root.nativeShapeHitTest)
                    if(root.selectionTool){
                        var previous=root.selectionIndices(),additive=!!(mouse.modifiers&(Qt.ShiftModifier|Qt.ControlModifier))
                        if(hit<0){
                            boxSelectionBase=additive?previous:[];root.setSelection(boxSelectionBase)
                            boxSelecting=true;root.selectionBox={x:mouse.x,y:mouse.y,width:0,height:0}
                        }else if(additive&&previous.indexOf(hit)>=0){
                            var removed=root.selectionMembers(hit)
                            root.setSelection(previous.filter(function(index){return removed.indexOf(index)<0}));dragging=false;return
                        }else if(additive)root.setSelection(previous.concat([hit]),hit)
                        else root.setSelection(previous.indexOf(hit)>=0?previous:[hit],hit)
                    }else root.selected=hit

                    if(root.selected>=0&&(root.tool==="nodes"||root.tool==="anchors")){

                        var newlySelected=root.editHandles(),nearest=10/Math.max(0.000001,root.viewScale)

                        for(var nh=0;nh<newlySelected.length;nh++){

                            var nd=Math.hypot(newlySelected[nh].x-mouse.x,newlySelected[nh].y-mouse.y)

                            if(nd<nearest){nearest=nd;root.selectedHandle=newlySelected[nh]}

                        }

                    }

                }

                if(root.selectionTool&&!boxSelecting)captureSelection()
                original=root.selected>=0?Vector.copy(root.shapes[root.selected]):null
                if(original){delete original.reflowSource;delete original.reflowGroup}
                if(root.selectedHandle&&root.selectedHandle.fragment&&root.tool==="anchors"&&original){
                    var current=root.selectedHandle
                    var exact=Vector.linkedHandles(original,root.bindings,root.selected).filter(function(h){return Math.hypot(h.x-current.x,h.y-current.y)<10/root.viewScale})
                    if(exact.length)root.selectedHandle=exact[0]
                }
                if(original&&root.selectedHandle&&!root.selectedHandle.fragment){
                    originalHandles=root.editHandles()
                    if(!original.singleAnchor)deformationPlan=Vector.deformationPlan(original,root.selectedHandle,originalHandles)
                }

                originalBindings=root.bindings.slice()

                if(root.tool==="anchors"&&root.selectedHandle){

                    root.pendingBinding=Vector.copy(root.selectedHandle)

                    root.pendingBinding.mouseX=mouse.x;root.pendingBinding.mouseY=mouse.y

                }

            }

            root.requestPaint()

        }

        onPositionChanged:function(mouse){if(!root.pressureGesture)moveInput(mouse)}
        function moveInput(mouse) {
            pointerX=mouse.x;pointerY=mouse.y

            if(!dragging||!(mouse.buttons&Qt.LeftButton))return

            var p={x:mouse.x,y:mouse.y}
            if(Painting.gradientTool(gestureTool)&&original){
                if(!mutated&&Math.hypot(p.x-first.x,p.y-first.y)*root.viewScale<1)return
                if(!mutated){root.remember();mutated=true}
                var gradient=gradientHandle?Painting.move(root,original,gradientHandle,p):Painting.create(root,original,first,p)
                root.shapes=Painting.applySet(root,selectionOriginals,selectionOrder,gestureTool,root.strokeColor,gradient,original)
                root.requestPaint();return
            }
            if(arrowHandle){
                if(!mutated&&Math.hypot(p.x-first.x,p.y-first.y)*root.viewScale<0.5)return
                beginSelectionChange()
                var arrowIndex=selectionOrder[0],beforeArrow=selectionOriginals[arrowIndex],updatedArrows=root.shapes.slice()
                updatedArrows[arrowIndex]=arrowHandle.role==="head"?Vector.adjustArrowHead(beforeArrow,p.x-first.x,p.y-first.y,root.viewScale):Vector.adjustArrow(beforeArrow,arrowHandle.role,
                    {x:arrowHandle.targetX+p.x-first.x,y:arrowHandle.targetY+p.y-first.y},
                    {alt:!!(mouse.modifiers&Qt.AltModifier),shift:!!(mouse.modifiers&Qt.ShiftModifier)})
                root.bindings=root.transformBindings(beforeArrow,updatedArrows[arrowIndex],originalBindings,arrowIndex)
                root.shapes=updatedArrows;root.requestPaint();return
            }
            if(gestureTool==="text"||gestureTool==="characters"){
                root.preview=Vector.shape("rectangle",first,p,Vector.style("#398df1","none",1/root.viewScale,1));root.requestPaint();return
            }
            if((mouse.modifiers&Qt.AltModifier)&&(gestureTool==="pencil"||root.isBasicShape(gestureTool))){
                p=Editing.constrained(first,p,{alt:true}).b
            }
            if(root.selectedHandle&&root.selectedHandle.fragment&&original&&(gestureTool==="nodes"||gestureTool==="anchors")){
                if(!mutated){root.remember();mutated=true}
                Editing.moveFragment(root,original,root.selectedHandle,p.x-first.x,p.y-first.y);return
            }
            if(gestureTool.indexOf("erase-")===0){
                var region=Editing.constrained(first,p,{shift:!!(mouse.modifiers&Qt.ShiftModifier),control:!!(mouse.modifiers&Qt.ControlModifier),alt:!!(mouse.modifiers&Qt.AltModifier)})
                root.preview=Editing.basicShape(gestureTool.slice(6),region.a,region.b,Vector.style("#eb6536","#eb6536",1/root.viewScale,0.25));root.requestPaint();return
            }
            if(transformHandle){
                beginSelectionChange()
                var b=transformBox,cx=b.x+b.width/2,cy=b.y+b.height/2,dx=p.x-first.x,dy=p.y-first.y,m=[1,0,0,1,0,0]
                if(transformHandle.rotate){
                    var angle=Math.atan2(p.y-cy,p.x-cx)
                    if(rotationAngle===null)rotationAngle=angle
                    var cos=Math.cos(angle-rotationAngle),sin=Math.sin(angle-rotationAngle)
                    m=[cos,sin,-sin,cos,cx-cos*cx+sin*cy,cy-sin*cx-cos*cy]
                }else{
                    var h=transformHandle,centered=!!(mouse.modifiers&(Qt.ShiftModifier|Qt.ControlModifier)),uniform=!!(mouse.modifiers&Qt.ControlModifier)
                    var ax=centered?cx:b.x+b.width*(1-h.u),ay=centered?cy:b.y+b.height*(1-h.v)
                    if(root.transformMode==="skew"&&!uniform){
                        var kx=h.v!==0.5?dx/(b.y+b.height*h.v-ay):0,ky=h.u!==0.5?dy/(b.x+b.width*h.u-ax):0
                        m=[1,ky,kx,1,-kx*ay,-ky*ax]
                    }else{
                        var factor=centered?2:1,sx=h.u===0.5?1:1+factor*dx*(h.u===0?-1:1)/Math.max(1,b.width),sy=h.v===0.5?1:1+factor*dy*(h.v===0?-1:1)/Math.max(1,b.height)
                        if(uniform){var scale=h.u===0.5?sy:h.v===0.5?sx:Math.abs(sx-1)>Math.abs(sy-1)?sx:sy;sx=scale;sy=scale}
                        sx=Math.max(0.01,Math.min(100,sx));sy=Math.max(0.01,Math.min(100,sy))
                        m=[sx,0,0,sy,ax*(1-sx),ay*(1-sy)]
                    }
                }
                applySelectionMatrix(m,false)
                root.requestPaint();return
            }
            if(gestureTool==="eraser"){
                erasePosition=p;eraseDirty=true;return
            }
            if(boxSelecting){
                root.selectionBox={x:Math.min(first.x,p.x),y:Math.min(first.y,p.y),
                    width:Math.abs(p.x-first.x),height:Math.abs(p.y-first.y)}
                root.selectBox(root.selectionBox,boxSelectionBase);return
            }
            if((gestureTool==="select"||gestureTool==="transform")&&selectionOrder.length){
                var moveX=p.x-first.x,moveY=p.y-first.y
                if(!mutated&&Math.hypot(moveX,moveY)*root.viewScale<0.5)return
                beginSelectionChange();applySelectionMatrix([1,0,0,1,moveX,moveY],true)
                if(root.switchStaff){var movedBox=root.selectionBounds();root.targetStaffMarker=root.nearbyAnchor(movedBox.x+movedBox.width/2,movedBox.y+movedBox.height/2,null,Infinity)}
                root.requestPaint();return
            }

            if(gestureTool==="nodes"||gestureTool==="anchors"){

                var follow=1-root.dragResistance

                p={x:first.x+(mouse.x-first.x)*follow,y:first.y+(mouse.y-first.y)*follow}

            }

            if(gestureTool==="pencil"){

                var last=points[points.length-1]

                if(root.pressureEnabled&&root.tabletActive)p.pressure=root.tabletPressure
                if(Math.hypot(p.x-last.x,p.y-last.y)*root.viewScale>1)points.push(p)

                root.pencilPreviewDirty=true

            }else if(gestureTool==="pen"){

                var a=Vector.copy(root.penNodes),n=a[a.length-1];n.dx=p.x-n.x;n.dy=p.y-n.y

                root.penNodes=a;root.preview={commands:Vector.pen(a),style:root.currentStyle(),fillRule:"evenodd"}

            }else if(gestureTool==="anchors"){
                if(original&&original.kind==="text"){
                    if(!mutated){root.remember();mutated=true}
                    var texts=root.shapes.slice();texts[root.selected]=Vector.translate(original,p.x-first.x,p.y-first.y)
                    root.shapes=texts;Editing.attachLabel(root,root.selected,p.x,p.y);root.requestPaint();return
                }

                if(root.pendingBinding){
                    root.snapTarget=root.dragResistance<1?root.nearbyAnchor(p.x,p.y,root.selectedHandle):null
                    var pb=Vector.copy(root.pendingBinding)
                    pb.mouseX=root.snapTarget?root.snapTarget.x:p.x
                    pb.mouseY=root.snapTarget?root.snapTarget.y:p.y
                    root.pendingBinding=pb
                    if(root.snapTarget){
                        if(!mutated){root.remember();root.invalidateReflow(root.selected);mutated=true}
                        root.bindHandle(root.selectedHandle,root.snapTarget)
                    }

                }

            }else if(original&&(gestureTool==="select"||gestureTool==="nodes")){

                if(gestureTool==="nodes"&&!root.selectedHandle)return

                if(!mutated){root.remember();root.invalidateReflow(root.selected);mutated=true}

                // translate/deform return a new selected shape. Copying the
                // entire document here serialized every unrelated path per move.
                var s=root.shapes.slice()

                if(root.selectedHandle){var h=root.selectedHandle

                    root.snapTarget=root.nearbyAnchor(h.x+p.x-first.x,h.y+p.y-first.y,h)
                    if(root.snapTarget&&root.dragResistance<1){
                        p={x:first.x+root.snapTarget.x-h.x,y:p.y}
                    }else root.snapTarget=null
                    var editHandles=originalHandles

                    s[root.selected]=original.singleAnchor?Vector.translate(original,p.x-first.x,p.y-first.y):Vector.deform(original,h,editHandles,p.x-first.x,p.y-first.y,root.pointCurvature,deformationPlan)

                    root.deformationSession={shape:root.selected,original:original,handle:h,handles:editHandles,

                        dx:p.x-first.x,dy:p.y-first.y,bindings:originalBindings,plan:deformationPlan}

                }

                else {
                    s[root.selected]=Vector.translate(original,p.x-first.x,p.y-first.y)
                    if(s[root.selected].kind==="text"&&s[root.selected].textLink){
                        s[root.selected].textLink.dx+=p.x-first.x;s[root.selected].textLink.dy+=p.y-first.y
                    }
                    if(root.switchStaff){
                        var movedBox=Vector.bounds([s[root.selected]])
                        root.targetStaffMarker=root.nearbyAnchor(movedBox.x+movedBox.width/2,movedBox.y+movedBox.height/2,null,Infinity)
                    }
                }

                root.shapes=s

                root.bindings=originalBindings.map(function(b){

                    if(b.shape!==root.selected)return b
                    var c=Object.assign({},b)

                    if(c.shape===root.selected&&c.scale>0){

                        var before=original.commands[c.command],after=s[root.selected].commands[c.command]

                        if(before&&after){c.offsetX+=(after[c.field]-before[c.field])/c.scale;

                            c.offsetY+=(after[c.field+1]-before[c.field+1])/c.scale}

                    }

                    return c

                })
                if(gestureTool==="nodes"&&root.selectedHandle&&root.snapTarget){
                    root.bindHandle(root.selectedHandle,root.snapTarget)
                    originalBindings=root.deformationSession.bindings.slice()
                }

            }else if(root.isBasicShape(gestureTool)){

                var ends=Editing.constrained(first,p,{shift:!!(mouse.modifiers&Qt.ShiftModifier),control:!!(mouse.modifiers&Qt.ControlModifier),alt:!!(mouse.modifiers&Qt.AltModifier)})
                root.preview=Editing.basicShape(gestureTool,ends.a,ends.b,root.currentStyle())

            }

            root.requestPaint()

        }

        onReleased:function(mouse){if(!root.pressureGesture)endInput(mouse)}
        function endInput(mouse) {

            if(!dragging)return

            if(gestureTool==="eraser")flushEraser()
            dragging=false;root.pencilPreviewDirty=false
            if(Painting.active(gestureTool)){
                if(mutated)root.changed()
                gradientHandle=null;root.requestPaint();root.gestureFinished();return
            }
            if(arrowHandle){
                if(mutated){
                    if(arrowHandle.role==="start"||arrowHandle.role==="end")root.reanchorMovedSelection(selectionOrder)
                    root.changed()
                }
                arrowHandle=null;root.requestPaint();return
            }
            if(gestureTool==="text"||gestureTool==="characters"){
                var box={x:Math.min(first.x,mouse.x),y:Math.min(first.y,mouse.y),width:Math.abs(mouse.x-first.x),height:Math.abs(mouse.y-first.y)}
                if(box.width*root.viewScale<8||box.height*root.viewScale<8){box.width=160/root.viewScale;box.height=32/root.viewScale}
                root.preview=null;root.append(Editing.textBox(root,box));root.beginTextEditing(root.selected)
                if(gestureTool==="characters")root.characterPickerRequested()
                return
            }
            if(gestureTool.indexOf("erase-")===0){
                if(root.preview){root.remember();var box=Vector.bounds([root.preview]);Editing.eraseRegion(root,Editing.region(gestureTool.slice(6),{x:box.x,y:box.y},{x:box.x+box.width,y:box.y+box.height}))}
                root.preview=null;root.requestPaint();return
            }
            if(boxSelecting){
                root.selectBox(root.selectionBox,boxSelectionBase);root.selectionBox=null;boxSelecting=false
                root.status=root.trStatus("%1 drawing(s) selected. Delete to remove.","%1 desenho(s) selecionado(s). Delete para excluir.").arg(root.selectedShapes.length)
                root.requestPaint();return
            }

            if(gestureTool==="eraser"){root.requestPaint();root.gestureFinished();return}
            if(gestureTool==="pen")return

            if(gestureTool==="addnode"){root.addNode(mouse.x,mouse.y);return}

            if(gestureTool==="anchors")root.pendingBinding=null
            root.snapTarget=null
            if(gestureTool==="nodes"||gestureTool==="select"||gestureTool==="transform"||gestureTool==="anchors"){
                if(mutated&&root.selectionTool&&!transformHandle){
                    // Rebind once, on drop. Do not touch geometry or perform a
                    // score-wide lookup/layout for each pointer movement.
                    root.reanchorMovedSelection(selectionOrder)
                }else if(root.switchStaff&&root.targetStaffMarker&&root.selected>=0){
                    root.reanchorMovedSelection([root.selected]);mutated=true
                }
                root.targetStaffMarker=null

                root.preview=null

                if(mutated)root.changed()

                root.requestPaint();return

            }

            if(gestureTool==="pencil")root.append(Vector.pencilStroke(points,gestureSmoothing,root.currentStyle(),root.viewScale,points.some(function(p){return p.pressure!==undefined})))

            else if(root.isBasicShape(gestureTool)&&root.preview)root.append(root.preview)

            root.preview=null

            if(mutated)root.documentChanged()

            root.requestPaint()

        }

        onDoubleClicked: function(mouse) {
            var text=root.shapes[root.selected]
            if(root.tool!=="pen"&&text&&text.kind==="text"){root.beginTextEditing(root.selected);return}

            if(root.tool==="pen"){var last=root.penNodes[root.penNodes.length-1];if(last&&Math.hypot(mouse.x-last.x,mouse.y-last.y)*root.viewScale<=8)root.finishPen()}

            else if(root.tool==="nodes")root.addNode(mouse.x,mouse.y)

        }

        onCanceled: { dragging=false;root.preview=null;root.snapTarget=null;root.selectionBox=null;boxSelecting=false;root.requestPaint() }

    }

    Repeater {
        model:root.textIds
        delegate:Text {
            id:glyphText
            required property var modelData
            readonly property var entry:root.shapeLookup[modelData]
            readonly property var shape:entry?entry.shape:{svgMatrix:[1,0,0,1,0,0],textSize:1,textContent:"",style:{stroke:"#161616",opacity:1}}
            visible:!!entry&&shape.drawId!==root.editingTextId
            readonly property var m:shape.svgMatrix
            readonly property real frameWidth:shape.textFrameWidth||Math.max(1,Math.hypot(m[0],m[1]))
            readonly property real frameHeight:shape.textFrameHeight||shape.textSize*1.5
            text:shape.textContent;textFormat:Text.PlainText
            font.family:shape.textFont||"Arial";font.pixelSize:Math.max(1,Math.round(shape.textSize))
            font.bold:!!(shape.textFontStyle&1);font.italic:!!(shape.textFontStyle&2)
            font.underline:!!(shape.textFontStyle&4);font.strikeout:!!(shape.textFontStyle&8)
            width:frameWidth;horizontalAlignment:shape.textAlignment||Text.AlignLeft
            FontMetrics {id:glyphMetrics;font:glyphText.font}
            leftPadding:shape.textLayout?shape.textLayout.left:shape.textSize*0.15
            topPadding:shape.textLayout?shape.textLayout.baseline-glyphMetrics.ascent:shape.textSize*0.15
            rightPadding:shape.textSize*0.15
            lineHeightMode:Text.FixedHeight;lineHeight:shape.textLayout?shape.textLayout.leading:shape.textSize*1.35
            color:root.shapes[root.selected]&&root.shapes[root.selected].drawId===shape.drawId?"#2675e6":shape.style.stroke
            opacity:shape.style.opacity
            transform:Matrix4x4 {matrix:Qt.matrix4x4(m[0]/frameWidth,m[2]/frameHeight,0,m[4],m[1]/frameWidth,m[3]/frameHeight,0,m[5],0,0,1,0,0,0,0,1)}
        }
    }
    Item {
        id:inlineFrame
        property int formatRevision:0
        visible:root.textEditing
        property var shape:root.textEditing?root.shapes[root.editingTextIndex]:null
        property var m:shape?shape.svgMatrix:[1,0,0,1,0,0]
        property real fw:shape?(shape.textFrameWidth||Math.max(1,Math.hypot(m[0],m[1]))):1
        property real fh:shape?(shape.textFrameHeight||shape.textSize*1.5):1
        readonly property bool tight:!!shape&&(shape.tightGlyph||/^(Leland|Bravura|Emmentaler|Gonville|MuseJazz|Petaluma)$/i.test(inlineText.font.family))
        readonly property var liveLayout:tight?Editing.textDimensions(root,inlineText.text,inlineText.font.pixelSize,inlineText.font.family,root.textFontStyle(),true):null
        width:tight?Math.max(1,liveLayout.width):Math.max(fw,inlineText.contentWidth+inlineText.leftPadding+inlineText.rightPadding)
        height:tight?Math.max(1,liveLayout.height):Math.max(fh,inlineText.contentHeight+inlineText.topPadding+inlineText.bottomPadding)
        transform:Matrix4x4 {matrix:Qt.matrix4x4(inlineFrame.m[0]/inlineFrame.fw,inlineFrame.m[2]/inlineFrame.fh,0,inlineFrame.m[4],inlineFrame.m[1]/inlineFrame.fw,inlineFrame.m[3]/inlineFrame.fh,0,inlineFrame.m[5],0,0,1,0,0,0,0,1)}
        Controls.TextArea {
            id:inlineText
            clip:inlineFrame.tight
            anchors.fill:parent;selectByMouse:true;wrapMode:TextEdit.NoWrap;textFormat:TextEdit.PlainText
            persistentSelection:true
            horizontalAlignment:inlineFrame.shape?(inlineFrame.shape.textAlignment||Text.AlignLeft):Text.AlignLeft
            FontMetrics {id:inlineMetrics;font:inlineText.font}
            padding:inlineFrame.tight?0:font.pixelSize*0.15
            leftPadding:inlineFrame.tight?inlineFrame.liveLayout.left:(inlineFrame.shape&&inlineFrame.shape.textLayout?inlineFrame.shape.textLayout.left:padding)
            topPadding:inlineFrame.tight?inlineFrame.liveLayout.baseline-inlineMetrics.ascent:(inlineFrame.shape&&inlineFrame.shape.textLayout?inlineFrame.shape.textLayout.baseline-inlineMetrics.ascent:padding)
            color:inlineFrame.shape?inlineFrame.shape.style.stroke:"#161616"
            background:Rectangle{color:"#eaffffff";border.color:"#398df1";border.width:1/Math.max(0.000001,root.viewScale)}
                Keys.onShortcutOverride:function(event){event.accepted=true}
            Keys.onPressed:function(event){
                if(event.key===Qt.Key_Escape){root.finishTextEditing();root.forceActiveFocus();root.tool="select";root.setSelection([]);event.accepted=true}
                else if((event.modifiers&Qt.ControlModifier)&&(event.key===Qt.Key_Return||event.key===Qt.Key_Enter)){
                    root.finishTextEditing();root.tool="select";root.forceActiveFocus();event.accepted=true
                }
            }
            // Formatting lives in a separate native popup window. Losing
            // keyboard focus to its controls must not end this text session.
        }
    }
    DropArea {
        anchors.fill:parent;enabled:root.drawingEnabled
        keys:["application/x-musedraw-character"]
        onDropped:function(drop){
            try{
                var payload=JSON.parse(drop.getDataAsString("application/x-musedraw-character"))
                root.finishTextEditing()
                var hit=Vector.hit(root.shapes,drop.x,drop.y,0,root.svgHitTest,root.nativeShapeHitTest)
                if(hit>=0&&root.shapes[hit].kind==="text")root.selected=hit
                else{root.finishTextEditing();root.selected=-1}
                root.insertCharacter(payload.text,payload.family,drop.x,drop.y);drop.acceptProposedAction()
            }catch(error){root.status=error.message}
        }
    }
    Rectangle {
        visible:root.drawingEnabled&&(input.containsMouse||root.mirrorPointerVisible)&&(root.tool==="pencil"||root.tool==="eraser")
        width:root.tool==="eraser"?root.eraserSize:root.effectiveStrokeWidth;height:width
        x:(root.mirrorPointerVisible&&!input.dragging?root.mirrorPointerX:input.pointerX)-width/2;y:(root.mirrorPointerVisible&&!input.dragging?root.mirrorPointerY:input.pointerY)-height/2
        radius:width/2
        color:root.tool==="pencil"?root.strokeColor:"transparent"
        opacity:root.tool==="pencil"?root.strokeOpacity:1
        border.color:"#202020"
        border.width:Math.min(width/3,1/Math.max(0.02,root.viewScale))
        z:100
        Rectangle {
            anchors.fill:parent
            anchors.margins:parent.border.width
            radius:width/2;color:"transparent"
            border.color:"#ffffff"
            border.width:Math.min(width/3,1/Math.max(0.02,root.viewScale))
        }
    }
    Image {
        visible:root.drawingEnabled&&(input.containsMouse||root.mirrorPointerVisible)&&root.tool==="pen"
        x:root.mirrorPointerVisible&&!input.dragging?root.mirrorPointerX:input.pointerX;y:(root.mirrorPointerVisible&&!input.dragging?root.mirrorPointerY:input.pointerY)-27/Math.max(0.02,root.viewScale)
        width:30/Math.max(0.02,root.viewScale);height:width;z:100
        sourceSize.width:60;sourceSize.height:60
        source:"data:image/svg+xml,"+encodeURIComponent('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 30 30"><path d="M1 29 L5 13 L16 5 L24 13 L16 25 Z" fill="#b9c5d3" stroke="white" stroke-width="3"/><path d="M1 29 L5 13 L16 5 L24 13 L16 25 Z" fill="#b9c5d3" stroke="#202020" stroke-width="1.4"/><path d="M16 5 L22 1 L29 8 L24 13 Z" fill="#398df1" stroke="#202020"/><path d="M1 29 L12 18" stroke="#202020"/><circle cx="12" cy="18" r="2" fill="#202020"/></svg>')
    }

}

// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick

import QtQuick.Controls

import QtQuick.Layouts

ScrollView {

    id: root

    property var editor: null
    property string helpText:""
    property bool pointerPreview:false
    property bool sliderBusy:false
    property bool eraserPreview:false

    property bool nativeMode: false
    property bool paletteMode: false
    function tr(source, portuguese) { return nativeMode ? qsTrc("drawing-tools", source) : portuguese }
    function chooseTool(key) {
        if(editor){editor.tool=key;editor.drawingEnabled=true}
    }

    property bool supportsRhythmicAnchors:true

    property string applyText:nativeMode?"Inserir na partitura":"Aplicar vínculo"

    readonly property alias smoothingControl: smoothingSlider

    function toolButton(key) {

        for(var i=0;i<toolButtons.count;i++){

            var button=toolButtons.itemAt(i)

            if(button&&button.modelData.key===key)return button

        }

        return null

    }

    signal insertRequested()
    signal directInsertRequested()

    signal exportRequested()

    signal importRequested()
    signal importSvgRequested()

    clip: true

    implicitWidth:250

    contentWidth: availableWidth

    Column {

        width: root.availableWidth

        spacing: 10

        Component.onCompleted: {

            for(var i=0;i<children.length;i++)

                children[i].width=Qt.binding(function(){return root.availableWidth})

        }

        Label { visible:!root.paletteMode; text:root.tr("Vector drawing","Desenho vetorial");font.bold:true;font.pixelSize:18;Layout.margins:8 }

        CheckBox {
            visible:!root.paletteMode

            text:root.tr("Draw","Desenhar");checked:root.editor?root.editor.drawingEnabled:false

            onToggled: if(root.editor)root.editor.drawingEnabled=checked

            onHoveredChanged:root.helpText=hovered?root.tr("Turn off to select and edit the score.","Desative para selecionar e editar a partitura."):""

        }

        GridLayout {
            visible:!root.paletteMode

            columns:2;Layout.fillWidth:true

            Repeater {

                id:toolButtons

                model:[{key:"select",name:root.tr("Select","Selecionar")},{key:"transform",name:root.tr("Transform selection","Transformar seleção")},{key:"pencil",name:root.tr("Pencil","Lápis")},{key:"pen",name:root.tr("Bézier pen","Caneta Bézier")},

                    {key:"nodes",name:root.tr("Edit points","Editar pontos")},{key:"line",name:root.tr("Line","Linha")},{key:"rectangle",name:root.tr("Rectangle","Retângulo")},

                    {key:"ellipse",name:root.tr("Ellipse","Elipse")},{key:"anchors",name:root.tr("Link points","Vincular pontos")},{key:"addnode",name:root.tr("Add point","Adicionar ponto")},{key:"eraser",name:root.tr("Eraser","Borracha")},{key:"deletenode",name:root.tr("Delete point","Excluir ponto")},{key:"join",name:root.tr("Join points","Unir pontos")}]

                delegate: Button {

                    required property var modelData

                    objectName:"tool_"+modelData.key
                    onHoveredChanged:{
                        var tips={select:root.tr("Click a drawing or drag a selection box. Delete removes the selection.","Clique em um desenho ou arraste uma caixa de selecao. Delete exclui a selecao."),
                            pencil:root.tr("Drag to draw. The circle shows the width of the new stroke.","Arraste para desenhar. O circulo indica a largura visual do novo traco."),
                            pen:root.tr("Click to create Bézier points; drag to adjust handles. Enter finishes.","Clique para criar pontos Bezier; arraste para ajustar as alcas. Enter termina."),
                            nodes:root.tr("Drag visible points to reshape the drawing.","Arraste os pontos visiveis para deformar o desenho."),
                            anchors:root.tr("Align the point horizontally to change its rhythmic attachment.","Alinhe horizontalmente o ponto para trocar seu vinculo ritmico."),
                            line:root.tr("Drag to draw a line.","Arraste para desenhar uma linha."),rectangle:root.tr("Drag to draw a rectangle.","Arraste para desenhar um retangulo."),
                            ellipse:root.tr("Drag to draw an ellipse.","Arraste para desenhar uma elipse."),addnode:root.tr("Click the stroke to add an editable point.","Clique sobre o traco para adicionar um ponto editavel."),
                            deletenode:root.tr("Click a visible point. Endpoints trim the line; interior points reconnect their neighbours.","Clique num ponto visível. Nas extremidades, apaga até o próximo ponto; no meio, reconecta os vizinhos."),
                            join:root.tr("Click the endpoints of two open lines to join them. The first drawing keeps its style.","Clique na extremidade de duas linhas abertas para uni-las. O estilo do primeiro desenho é mantido."),
                            eraser:root.tr("Erase the entire stroke or only the areas touched by the eraser.","Apague o traco inteiro ou somente os trechos por onde a borracha passar.")}
                        root.helpText=hovered?tips[modelData.key]:""
                    }

                    text:modelData.name;checkable:true

                    enabled:modelData.key!=="anchors"||root.supportsRhythmicAnchors

                    checked:root.editor?root.editor.tool===modelData.key:false

                    Layout.fillWidth:true

                    Layout.minimumWidth:0

                    Layout.preferredWidth:Math.floor((root.availableWidth-12)/2)

                    Layout.maximumWidth:Math.floor((root.availableWidth-12)/2)

                    onClicked: if(root.editor){

                        root.editor.tool=modelData.key

                        root.editor.drawingEnabled=true

                    }

                }

            }

        }

        Column {
            visible:root.editor&&root.editor.tool==="select"
            spacing:4
            Label {text:root.tr("Transform selection","Transformar seleção")}
            ComboBox {
                width:root.availableWidth;model:[root.tr("Resize / stretch","Redimensionar / esticar"),root.tr("Distort / skew","Distorcer / inclinar")]
                currentIndex:root.editor&&root.editor.transformMode==="skew"?1:0
                onActivated:if(root.editor)root.editor.transformMode=currentIndex===1?"skew":"scale"
            }
            Label {width:root.availableWidth;wrapMode:Text.WordWrap;text:root.tr("Shift: mirrored sides. Ctrl: proportional size. Centre point: rotate.","Shift: lados espelhados. Ctrl: tamanho proporcional. Ponto central: girar.")}
            Button {text:root.tr("Flip horizontally","Espelhar horizontal");width:root.availableWidth;onClicked:if(root.editor)root.editor.mirrorSelected(true)}
            Button {text:root.tr("Flip vertically","Espelhar vertical");width:root.availableWidth;onClicked:if(root.editor)root.editor.mirrorSelected(false)}
        }
        Button {visible:!root.nativeMode;text:root.tr("Import SVG","Importar SVG");width:root.availableWidth;onClicked:root.importSvgRequested()}
        Column {

            objectName:"pointEditingControls"

            visible:root.editor&&(root.editor.tool==="nodes"||root.editor.tool==="anchors")

            spacing:4

            Label {text:root.tr("Point curvature: ","Curvatura do ponto: ")+Math.round(curvatureSlider.value*100)+"%"}

            InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}

                id:curvatureSlider

                objectName:"pointCurvature"

                width:parent.width

                from:0;to:1;stepSize:0.01

                value:root.editor?root.editor.pointCurvature:0.5

                onPressedChanged:if(pressed&&root.editor)root.editor.remember()

                onMoved:if(root.editor)root.editor.setPointCurvature(value)

            }

            Label {text:root.tr("Drag resistance: ","Resistência ao arrasto: ")+Math.round(resistanceSlider.value*100)+"%"}

            InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}

                id:resistanceSlider

                objectName:"pointDragResistance"

                width:parent.width

                from:0;to:1;stepSize:0.01

                value:root.editor?root.editor.dragResistance:0

                onMoved:if(root.editor)root.editor.dragResistance=value



                onHoveredChanged:root.helpText=hovered?root.tr("0% follows the mouse; 100% keeps the point still.","0% acompanha o mouse; 100% mantém o ponto parado."):""

            }

        }

        Label { text:root.tr("Pencil smoothing","Suavidade do lápis") }

        InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}

            id:smoothingSlider

            objectName:"pencilSmoothing"

            Layout.minimumWidth:0

            Layout.maximumWidth:root.availableWidth

            from:0;to:1;value:root.editor?root.editor.smoothing:0.7;Layout.fillWidth:true


            onMoved: if(root.editor)root.editor.setSmoothing(value)

            onHoveredChanged:root.helpText=hovered?root.tr("Applies only to new pencil drawings.","Aplica-se somente aos novos desenhos feitos com o lapis."):""

        }

        Label {text:root.tr("Pencil pointer","Ponteiro do lapis")}
        ComboBox {
            model:[root.tr("Dynamic (follows zoom)","Dinamico (acompanha o zoom)"),root.tr("Fixed (constant screen size)","Fixo (tamanho constante na tela)")]
            currentIndex:root.editor&&root.editor.pointerMode==="fixed"?1:0
            onActivated:if(root.editor)root.editor.pointerMode=currentIndex===1?"fixed":"dynamic"
        }
        Label {text:(root.editor&&root.editor.tool==="pencil"?root.tr("Pointer size: ","Tamanho do ponteiro: "):root.tr("Stroke width: ","Espessura do traco: "))+(root.editor?Number(root.editor.strokeWidth.toFixed(2)):2)}
        InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}
            from:1;to:1000;stepSize:1
            value:root.editor?Math.max(1,Math.min(1000,root.editor.strokeWidth)):30
            onPressedChanged:{root.pointerPreview=pressed;root.eraserPreview=false}
            onMoved:if(root.editor)root.editor.strokeWidth=value
            onHoveredChanged:root.helpText=hovered?root.tr("Drag to set the size from 1 to 1000.","Arraste para definir o tamanho entre 1 e 1000."):""

        }
        Column {
            visible:root.editor&&root.editor.tool==="eraser"
            spacing:6
            ComboBox {
                width:parent.width
                model:[root.tr("Erase entire line","Apagar linha inteira"),root.tr("Erase touched areas","Apagar onde passar")]
                onActivated:if(root.editor)root.editor.eraserMode=currentIndex===0?"whole":"partial"
            }
            Label {text:root.tr("Eraser size: ","Tamanho da borracha: ")+(root.editor?Number(root.editor.eraserSize.toFixed(2)):24)}
            InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}
                width:parent.width;from:1;to:1000;stepSize:1
                value:root.editor?Math.max(1,Math.min(1000,root.editor.eraserSize)):24
                onPressedChanged:{root.pointerPreview=pressed;root.eraserPreview=pressed}
                onMoved:if(root.editor)root.editor.eraserSize=value

            }
        }
        Label { text:root.tr("Stroke colour (hexadecimal)","Cor do traço (hexadecimal)") }

        TextField {
            focusPolicy:Qt.StrongFocus
            selectByMouse:true
            Keys.onShortcutOverride:function(event){if(activeFocus)event.accepted=true}

            Layout.minimumWidth:0

            Layout.maximumWidth:root.availableWidth

            text:root.editor?root.editor.strokeColor:"#161616";Layout.fillWidth:true

            validator:RegularExpressionValidator {regularExpression:/#[0-9a-fA-F]{6}/}

            onEditingFinished: if(root.editor&&acceptableInput)root.editor.strokeColor=text

        }

        CheckBox {text:root.tr("Fill shapes","Preencher formas");checked:root.editor?root.editor.filled:false;onToggled:if(root.editor)root.editor.filled=checked}

        TextField {
            focusPolicy:Qt.StrongFocus
            selectByMouse:true
            Keys.onShortcutOverride:function(event){if(activeFocus)event.accepted=true}

            text:root.editor?root.editor.fillColor:"#ffffff";Layout.fillWidth:true

            Layout.minimumWidth:0

            Layout.maximumWidth:root.availableWidth

            validator:RegularExpressionValidator {regularExpression:/#[0-9a-fA-F]{6}/}

            onEditingFinished: if(root.editor&&acceptableInput)root.editor.fillColor=text

        }

        Label {text:root.tr("Opacity","Opacidade")}

        InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}from:0.05;to:1;value:root.editor?root.editor.strokeOpacity:1;Layout.fillWidth:true;Layout.maximumWidth:root.availableWidth;onMoved:if(root.editor)root.editor.strokeOpacity=value}

        Button {text:root.tr("Apply style to selection","Aplicar estilo à seleção");Layout.fillWidth:true;onClicked:if(root.editor)root.editor.updateStyle()}

        RowLayout {
            visible:!root.paletteMode

            Layout.maximumWidth:root.availableWidth

            Layout.fillWidth:true

            Button {text:root.tr("Undo","Desfazer");Layout.maximumWidth:(root.availableWidth-12)/2;onClicked:if(root.editor)root.editor.undo()}

            Button {text:root.tr("Redo","Refazer");Layout.maximumWidth:(root.availableWidth-12)/2;onClicked:if(root.editor)root.editor.redo()}

        }

        RowLayout {
            visible:!root.paletteMode

            Layout.maximumWidth:root.availableWidth

            Layout.fillWidth:true

            Button {text:root.tr("Delete","Excluir");Layout.maximumWidth:(root.availableWidth-12)/2;onClicked:if(root.editor)root.editor.removeSelected()}

            Button {text:root.tr("Clear","Limpar");Layout.maximumWidth:(root.availableWidth-12)/2;onClicked:if(root.editor)root.editor.clear()}

        }

        Label {text:root.tr("Continuation after break: ","Continuação após quebra: ")+Math.round(continuationSlider.value*100)+"%";wrapMode:Text.WordWrap}
        InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}
            onPressedChanged:if(pressed&&root.editor)root.editor.remember(true)
            id:continuationSlider
            objectName:"systemContinuation"
            from:0;to:1;stepSize:0.01
            value:root.editor?root.editor.systemContinuation:0
            onMoved:if(root.editor)root.editor.setSystemContinuation(value)

            onHoveredChanged:root.helpText=hovered?root.tr("0% stops at the barline; 100% extends to the page edge.","0% corta na barra de compasso; 100% permite continuar até a borda da página."):""
        }
        Label {text:root.tr("Start after break: ","Início após quebra: ")+Math.round(startContinuationSlider.value*100)+"%";wrapMode:Text.WordWrap}
        InfiniteSlider {
            onAdjusting:function(active){root.sliderBusy=active}
            onPressedChanged:if(pressed&&root.editor)root.editor.remember(true)
            id:startContinuationSlider
            objectName:"systemStartContinuation"
            from:0;to:1;stepSize:0.01
            value:root.editor?root.editor.systemStartContinuation:0
            onMoved:if(root.editor)root.editor.setSystemStartContinuation(value)

            onHoveredChanged:root.helpText=hovered?root.tr("0% starts at the barline; 100% extends to the left page edge.","0% começa na barra do compasso; 100% permite estender até a borda esquerda da página."):""
        }
        Label {text:root.tr("Musical attachment","Vínculo musical")}

        ComboBox {

            Layout.minimumWidth:0

            Layout.maximumWidth:root.availableWidth

            model:[root.tr("Note (without deformation)","Nota (sem deformar)"),root.tr("Measures / rhythmic spacing","Compassos / espaço rítmico")]

            currentIndex:root.editor&&root.editor.anchorMode==="rhythm"?1:0

            Layout.fillWidth:true

            onActivated:if(root.editor)root.editor.anchorMode=currentIndex===0?"note":"rhythm"

        }

        Label {text:root.tr("Rhythmic subdivision","Subdivisão rítmica")}

        ComboBox {

            model:["1/4","1/8","1/16","1/32"];currentIndex:1;Layout.fillWidth:true

            Layout.minimumWidth:0

            Layout.maximumWidth:root.availableWidth

            onActivated:if(root.editor)root.editor.subdivision=[4,8,16,32][currentIndex]

        }

        Button {text:root.tr("Attach selection to rhythm","Vincular seleção ao ritmo");Layout.fillWidth:true
            onClicked:if(root.editor)root.editor.linkSelection()
            onHoveredChanged:root.helpText=hovered?root.tr("Attaches the selected drawing to rhythmic staff positions, preserving its height.","Vincula a figura selecionada às posições rítmicas da pauta, preservando sua altura."):""
        }
        Button {visible:!root.nativeMode;text:root.applyText;Layout.fillWidth:true;onClicked:root.insertRequested()}

        RowLayout {
            visible:!root.paletteMode

            Layout.maximumWidth:root.availableWidth

            Layout.fillWidth:true

            Button {text:root.tr("Export SVG","Exportar SVG");Layout.maximumWidth:(root.availableWidth-12)/2;onClicked:root.exportRequested()}

            Button {text:root.nativeMode?root.tr("Import SVG","Importar SVG"):"Abrir desenho";Layout.maximumWidth:(root.availableWidth-12)/2;onClicked:root.nativeMode?root.importSvgRequested():root.importRequested()}

        }

        Label {

            text:root.editor?root.editor.status:root.tr("Open a score.","Abra uma partitura.")

            wrapMode:Text.WordWrap;Layout.fillWidth:true;Layout.margins:6

        }

        Label {

            text:root.tr("Pen: drag to create handles; Enter finishes. Edit points: drag nodes; double-click the stroke to add a node. Esc cancels.","Caneta: clique e arraste para criar alças; Enter termina. Editar pontos: arraste os nós; duplo clique no traço cria um nó. Esc cancela.")

            wrapMode:Text.WordWrap;Layout.fillWidth:true;Layout.margins:6;font.pixelSize:11

        }

        Button {
            visible:!root.nativeMode
            text:"IMPORTAR PARA O MUSESCORE"
            Layout.fillWidth:true
            onClicked:root.directInsertRequested()
            onHoveredChanged:root.helpText=hovered?"Insere os SVGs diretamente na partitura aberta. Ctrl+Z desfaz a inserção.":""
        }

    }

}

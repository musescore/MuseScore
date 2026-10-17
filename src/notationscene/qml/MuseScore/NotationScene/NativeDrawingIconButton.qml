// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import Muse.Ui
import Muse.UiComponents
FlatButton {
    id: root
    property string glyph: "select"
    property bool grouped: false
    readonly property bool hovered:mouseArea.containsMouse
    width:40;height:40
    transparent:!accentButton
    property bool drawingToolTip:true
    toolTipShowLocked:false
    iconFont:ui.theme.toolbarIconsFont
    navigation.name:"drawing-"+glyph
    navigation.accessible.name:toolTipTitle
    contentItem:Component {
        Item {
            width:40;height:40
            StyledIconLabel {visible:root.glyph==="pencil";anchors.centerIn:parent;iconCode:IconCode.EDIT;font:root.iconFont;color:root.iconColor}
            Canvas {
                anchors.fill:parent
                property color ink:root.iconColor
                property string glyph:root.glyph
                onInkChanged:requestPaint()
                onGlyphChanged:requestPaint()
                onPaint:{
                    var c=getContext("2d")
                    c.reset();c.clearRect(0,0,width,height)
                    c.strokeStyle=ink;c.fillStyle=ink;c.lineWidth=1.5;c.lineCap="round";c.lineJoin="round"
                    function line(x,y,u,v){c.beginPath();c.moveTo(x,y);c.lineTo(u,v);c.stroke()}
                    function dot(x,y){c.beginPath();c.arc(x,y,2.2,0,Math.PI*2);c.fill()}
                    function box(x,y){c.strokeRect(x-2.2,y-2.2,4.4,4.4)}
                    if(glyph==="paint-stroke"){
                        c.lineWidth=3;line(8,29,30,10);c.lineWidth=1.5;line(10,10,19,10);line(10,10,10,19);box(30,29)
                    }else if(glyph==="paint"||glyph.indexOf("paint-")===0){
                        c.beginPath();c.moveTo(10,19);c.lineTo(20,9);c.lineTo(30,19);c.lineTo(20,29);c.closePath();c.stroke();line(15,9,13,5);line(10,19,30,19)
                        c.beginPath();c.moveTo(31,23);c.quadraticCurveTo(25,31,31,32);c.quadraticCurveTo(37,31,31,23);c.fill()
                    }else if(glyph==="gradient-stroke"){
                        var stripe=c.createLinearGradient(8,28,32,12);stripe.addColorStop(0,ink);stripe.addColorStop(1,"transparent");c.strokeStyle=stripe;c.lineWidth=5;line(8,28,32,12);c.strokeStyle=ink;c.lineWidth=1.5;box(8,28);box(32,12)
                    }else if(glyph.indexOf("gradient-")===0){
                        c.strokeRect(8,10,24,20);var g=c.createLinearGradient(9,20,31,20);g.addColorStop(0,ink);g.addColorStop(1,"transparent");c.fillStyle=g;c.fillRect(9,11,22,18);c.fillStyle=ink;line(9,31,31,9);box(9,31);box(31,9)
                    }else if(glyph==="characters"){
                        c.beginPath();c.moveTo(9,30);c.lineTo(16,30);c.bezierCurveTo(5,20,11,9,20,9);c.bezierCurveTo(29,9,35,20,24,30);c.lineTo(31,30);c.stroke()
                    }else if(glyph==="anchor-note"){
                        c.beginPath();c.ellipse(12,24,11,7);c.fill();line(23,27,23,8);line(23,8,30,12)
                    }else if(glyph==="anchor-subdivision"){
                        line(7,13,33,13);dot(8,13);dot(16,13);dot(24,13);dot(32,13);line(8,17,8,29);line(16,17,16,25);line(24,17,24,29);line(32,17,32,25)
                    }else if(glyph==="select"){
                        c.beginPath();c.moveTo(13,8);c.lineTo(28,22);c.lineTo(21,23);c.lineTo(24,31);c.lineTo(20,33);c.lineTo(17,25);c.lineTo(12,30);c.closePath();c.stroke()
                    }else if(glyph==="transform"){
                        c.strokeRect(11,11,18,18);box(11,11);box(29,11);box(29,29);box(11,29)
                        line(15,25,25,15);line(20,15,25,15);line(25,15,25,20)
                    }else if(glyph==="pen"){
                        c.beginPath();c.moveTo(12,29);c.lineTo(16,14);c.lineTo(27,10);c.lineTo(30,21);c.closePath();c.stroke();line(12,29,23,18);dot(23,18)
                    }else if(glyph==="text"){line(10,10,30,10);line(20,10,20,30);line(15,30,25,30)
                    }else if(glyph==="group"||glyph==="ungroup"){
                        c.strokeRect(10,11,10,10);c.strokeRect(21,20,10,10)
                        if(glyph==="group"){c.setLineDash([3,2]);c.strokeRect(6,7,29,27);c.setLineDash([])}
                        else{line(7,30,12,30);line(7,30,7,25);line(28,8,33,8);line(33,8,33,13)}
                    }else if(glyph==="merge"||glyph==="fuse"){c.strokeRect(8,9,14,14);c.strokeRect(18,18,14,14);line(14,28,26,12)
                    }else if(glyph==="triangle"||glyph==="right-triangle"){
                        c.beginPath();c.moveTo(glyph==="triangle"?20:10,9);c.lineTo(31,30);c.lineTo(9,30);c.closePath();c.stroke()
                    }else if(glyph.indexOf("arrow")===0){
                        line(8,20,32,20);line(25,13,32,20);line(25,27,32,20)
                        if(glyph==="arrow-double"){line(15,13,8,20);line(15,27,8,20)}
                        if(glyph==="arrow-filled"){c.beginPath();c.moveTo(25,13);c.lineTo(32,20);c.lineTo(25,27);c.closePath();c.fill()}
                    }else if(glyph==="unlink"){
                        line(9,28,18,19);line(24,13,31,6);c.strokeStyle="#ef5252";line(13,9,29,25);line(13,25,29,9)
                    }else if(glyph.indexOf("erase-")===0){
                        c.setLineDash([3,2]);
                        if(glyph==="erase-rectangle")c.strokeRect(8,9,24,22)
                        else if(glyph==="erase-ellipse"){c.beginPath();c.arc(20,20,12,0,Math.PI*2);c.stroke()}
                        else{c.beginPath();c.moveTo(20,8);c.lineTo(32,31);c.lineTo(8,31);c.closePath();c.stroke()}
                        c.setLineDash([]);line(15,16,25,26);line(15,26,25,16)
                    }else if(glyph==="line"){line(10,29,30,10);dot(10,29);dot(30,10)
                    }else if(glyph==="rectangle"||glyph==="shapes"){c.strokeRect(10,11,20,18)
                    }else if(glyph==="ellipse"){
                        c.beginPath();c.moveTo(31,20);c.bezierCurveTo(31,9,9,9,9,20);c.bezierCurveTo(9,31,31,31,31,20);c.stroke()
                    }else if(["nodes","addnode","deletenode","join","anchors"].indexOf(glyph)>=0){
                        c.beginPath();c.moveTo(9,28);c.bezierCurveTo(12,5,27,35,31,12);c.stroke();box(9,28);box(31,12)
                        if(glyph==="addnode"||glyph==="deletenode"){line(22,9,32,9);if(glyph==="addnode")line(27,4,27,14)}
                        else if(glyph==="join"){dot(17,21);dot(23,21);line(17,21,23,21)}
                        else if(glyph==="anchors"){line(9,5,31,5);dot(20,5);line(20,5,20,21)}
                        else box(20,20)
                    }else if(glyph==="eraser"||glyph==="eraser-partial"){
                        c.beginPath();c.moveTo(10,23);c.lineTo(22,9);c.lineTo(31,17);c.lineTo(19,31);c.lineTo(16,31);c.closePath();c.stroke();line(14,19,24,27);line(19,31,31,31)
                        if(glyph==="eraser-partial"){c.setLineDash([2,3]);line(7,34,32,34);c.setLineDash([])}
                    }else if(glyph==="import"||glyph==="export"){
                        c.strokeRect(9,10,20,20);line(10,27,17,19);line(17,19,22,24);dot(24,15)
                        var d=glyph==="import"?1:-1;line(32,7,32,7+12*d);line(28,7+8*d,32,7+12*d);line(36,7+8*d,32,7+12*d)
                    }else if(glyph==="undo"||glyph==="redo"){
                        c.save();if(glyph==="redo"){c.translate(40,0);c.scale(-1,1)}
                        c.beginPath();c.moveTo(29,29);c.bezierCurveTo(34,12,18,10,10,17);c.stroke();line(10,17,10,9);line(10,17,18,18);c.restore()
                    }else if(glyph==="delete"){
                        line(10,12,30,12);line(16,8,24,8);line(14,13,15,31);line(15,31,25,31);line(25,31,26,13);line(19,17,19,27);line(22,17,22,27)
                    }
                    if(root.grouped){c.beginPath();c.moveTo(31,33);c.lineTo(35,33);c.lineTo(35,29);c.closePath();c.fill()}
                }
            }
        }
    }
}

// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtTest
import "../../qml/MuseScore/NotationScene/drawing/core/Vector.js" as V
import "../../qml/MuseScore/NotationScene/drawing/core/Context.js" as C
import "../../qml/MuseScore/NotationScene/drawing/core/Painting.js" as Paint
TestCase {
    name: "LineStyleScope"
    function shape() {
        return {drawId:"line",commands:[["M",0,0],["L",20,0],["L",40,0],["L",60,0],["L",80,0],["L",100,0]],
            style:V.style("#123456","none",2,1),segmentStyles:[{command:2,style:{stroke:"#ff0000",width:5}},{command:4,style:{stroke:"#00ff00",width:3}}]};
    }
    function test_single_node_stops_at_next_edit() {
        var original=shape(),s=C.changedStyle(original,1,-1,{width:9},false,[]);
        compare(C.effective(s,0).width,2);compare(C.effective(s,1).width,9);
        compare(C.effective(s,2).width,5);compare(C.effective(s,2).stroke,"#ff0000");
        compare(original.segmentStyles.length,2);compare(s.commands,original.commands);
    }
    function test_interval_preserves_both_sides_and_inner_colours() {
        var s=C.changedStyle(shape(),1,4,{width:8},false,[]);
        compare(C.effective(s,0).width,2);compare(C.effective(s,1).width,8);
        compare(C.effective(s,2).width,8);compare(C.effective(s,2).stroke,"#ff0000");
        compare(C.effective(s,3).width,8);compare(C.effective(s,4).width,3);compare(C.effective(s,4).stroke,"#00ff00");
        var ranges=s.paintRanges;compare(ranges[1].start,2);compare(ranges[1].end,3);
    }
    function test_delete_properties_inherits_previous_without_deleting_node() {
        var original=shape(),s=C.withoutStyle(original,2,-1);
        compare(C.effective(s,2).width,2);compare(C.effective(s,3).stroke,"#123456");
        compare(C.effective(s,4).width,3);compare(s.commands,original.commands);
        var ranged=C.withoutStyle(original,1,4);compare(C.effective(ranged,3).width,2);compare(C.effective(ranged,4).width,3);
    }
    function test_plain_colour_removes_inherited_gradient() {
        var original=shape(),g={from:{x:0,y:0},to:{x:1,y:0},stops:[{id:"a",offset:0,color:"#000000"},{id:"b",offset:1,color:"#ffffff"}]};
        original.style.strokeGradient=g;
        var s=C.changedStyle(original,1,4,{stroke:"#abcdef"},false,["strokeGradient"]);
        verify(!C.effective(s,1).strokeGradient);verify(!C.effective(s,2).strokeGradient);
        verify(!!C.effective(s,4).strokeGradient);verify(!!C.effective(original,1).strokeGradient);
        V.readDocument(V.document([s]));
    }
    function editor() {
        var e={styleEditActive:true,styleEditIndex:0,styleNodeCommand:1,styleRangeEnd:4,shapes:[shape()],selected:0,
            tool:"gradient-stroke",nodeStyleClipboard:null,remember:function(){},changed:function(){},loadNodeStyle:function(){},selectionIndices:function(){return [0];}};
        Object.defineProperty(e,"selectedNodeStyle",{get:function(){return C.effective(e.shapes[0],e.styleNodeCommand);}});
        e.applyNodeStyle=function(patch,replace){e.shapes=[C.changedStyle(e.shapes[0],e.styleNodeCommand,e.styleRangeEnd,patch,replace,[])];};
        return e;
    }
    function test_clipboard_uses_effective_style_and_is_session_only() {
        var e=editor();e.styleNodeCommand=3;C.copyNodeProperties(e);compare(e.nodeStyleClipboard.width,5);
        e.styleNodeCommand=1;e.styleRangeEnd=-1;C.pasteNodeProperties(e);compare(C.effective(e.shapes[0],1).width,5);
        C.removeNodeProperties(e);compare(C.effective(e.shapes[0],1).width,2);
        var before=JSON.stringify(e.shapes);e.styleEditActive=false;C.pasteNodeProperties(e);C.removeNodeProperties(e);compare(JSON.stringify(e.shapes),before);
    }
    function test_gradient_tools_are_scoped_to_interval() {
        var e=editor(),g={from:{x:0,y:0},to:{x:1,y:0},stops:[{id:"a",offset:0,color:"#000000"},{id:"b",offset:1,color:"#ffffff"}]};
        e.shapes=Paint.applySet(e,e.shapes,[0],e.tool,"#123456",g,e.shapes[0]);
        verify(!C.effective(e.shapes[0],0).strokeGradient);verify(!!C.effective(e.shapes[0],1).strokeGradient);
        verify(!!C.effective(e.shapes[0],3).strokeGradient);verify(!C.effective(e.shapes[0],4).strokeGradient);
        compare(Paint.descriptor(e).stops.length,2);var changed=Paint.move(e,e.shapes[0],{role:"to"},{x:50,y:20});verify(!!changed.to);
        V.readDocument(V.document(e.shapes));
    }
}

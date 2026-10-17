// SPDX-License-Identifier: GPL-3.0-only
import QtQuick
import QtTest
import "../../qml/MuseScore/NotationScene/drawing/core/Vector.js" as V
TestCase {
 name: "NibStability"
 Item {
  id: geometryRoot; width:200; height:200
  Item {
   id: rotatedNib; width:100; height:100; rotation:73
   Rectangle {id: flattenedNib; width:100; height:100;transform: Scale {origin.x:50;origin.y:50;yScale:0.05}}
  }
 }
 function test_high_flattening_preserves_rotation() {
  var a=flattenedNib.mapToItem(geometryRoot,0,50),b=flattenedNib.mapToItem(geometryRoot,100,50)
  var angle=Math.atan2(b.y-a.y,b.x-a.x)*180/Math.PI
  verify(Math.abs(angle-73)<0.001)
  var c=flattenedNib.mapToItem(geometryRoot,50,0),d=flattenedNib.mapToItem(geometryRoot,50,100)
  verify(Math.abs(Math.hypot(d.x-c.x,d.y-c.y)-5)<0.001)
 }

 function test_repeated_strokes() {
  for(var k=0;k<80;k++){
   var pts=[];for(var i=0;i<80;i++)pts.push({x:i*2,y:80+40*Math.sin(i/8),pressure:0.5+i/160})
   var st=V.style("#161616","none",12,1);st.nib={shape:k%2?"square":"round",flatten:0.8,angle:45}
   var sh=V.pencilStroke(pts,0.8,st,2,false)
   var svg=V.svg([sh],V.bounds([sh]));verify(svg.length>100)
   sh=null;svg=null
   gc()
  }
 }
}

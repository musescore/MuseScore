// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as V
.import "SvgAsset.js" as Assets

function active(tool){return tool.indexOf("paint-")===0||tool.indexOf("gradient-")===0;}
function gradientTool(tool){return tool.indexOf("gradient-")===0;}
function channel(tool){return tool.indexOf("fill")>=0?"fill":"stroke";}
function key(tool){return channel(tool)+"Gradient";}
function frame(shape){return V.paintBounds(shape.derivedWholeCommands||shape.commands);}
function fraction(shape,p){
    if(shape.svgSource&&shape.svgMatrix){
        var m=shape.svgMatrix,det=m[0]*m[3]-m[1]*m[2];
        if(Math.abs(det)>1e-12){var x=p.x-m[4],y=p.y-m[5];return {x:(m[3]*x-m[2]*y)/det,y:(-m[1]*x+m[0]*y)/det};}
    }
    var b=frame(shape);return {x:(p.x-b.x)/b.width,y:(p.y-b.y)/b.height};
}
function position(shape,p){
    if(shape.svgSource&&shape.svgMatrix){var m=shape.svgMatrix;return {x:m[0]*p.x+m[2]*p.y+m[4],y:m[1]*p.x+m[3]*p.y+m[5]};}
    var b=frame(shape);return {x:b.x+p.x*b.width,y:b.y+p.y*b.height};
}
function projected(shape,p,tool,scale){
    if(channel(tool)!=="stroke"||shape.svgSource)return p;
    var match=V.nearestSegment(shape,p.x,p.y,Math.max(24/Math.max(0.02,scale),shape.style.width));
    if(!match)return p;
    var last=null,start=null;
    for(var i=0;i<=match.command;i++){
        var c=shape.commands[i];
        if(c[0]==="M"){last={x:c[1],y:c[2]};start=last;continue;}
        var end=c[0]==="Z"?start:{x:c[c.length-2],y:c[c.length-1]};
        if(i===match.command)return c[0]==="C"?V.curvePoint(last,c,match.t):V.blend(last,end,match.t);
        last=end;
    }
    return p;
}
function hit(editor,x,y){
    var result=V.hit(editor.shapes,x,y,6/Math.max(0.02,editor.viewScale),editor.svgHitTest,editor.nativeShapeHitTest);
    if(result>=0||channel(editor.tool)!=="fill")return result;
    for(var i=editor.shapes.length-1;i>=0;i--){var sh=editor.shapes[i];if(!sh.svgSource&&V.insideFill(sh,x,y))return i;}
    return -1;
}
function descriptor(editor){
    var shape=editor.shapes[editor.selected];return gradientTool(editor.tool)&&shape?shape.style[key(editor.tool)]:null;
}
function handles(editor){
    var sh=editor.shapes[editor.selected],g=descriptor(editor);if(!editor.drawingEnabled||!sh||!g)return [];
    var a=position(sh,g.from),b=position(sh,g.to),dx=b.x-a.x,dy=b.y-a.y,length=Math.hypot(dx,dy)||1;
    var gap=13/Math.max(0.02,editor.viewScale),nx=-dy/length*gap,ny=dx/length*gap;
    var result=[{role:"from",x:a.x,y:a.y},{role:"to",x:b.x,y:b.y}];
    g.stops.forEach(function(stop,index){var p=V.blend(a,b,stop.offset);result.push({role:"stop",id:stop.id,index:index,color:stop.color,x:p.x+nx,y:p.y+ny,axisX:p.x,axisY:p.y});});
    return result;
}
function apply(shape,tool,colour,gradient){
    var out=Object.assign({},shape),target=channel(tool),gradientKey=key(tool);
    function styled(old){var style=Object.assign({},old);style[target]=colour;if(gradient)style[gradientKey]=gradient;else delete style[gradientKey];return style;}
    out.style=styled(shape.style);
    if(shape.paintRanges)out.paintRanges=shape.paintRanges.map(function(range){return Object.assign({},range,{style:styled(range.style)});});
    if(shape.svgSource){
        out.paintSource=shape.paintSource||shape.svgSource;
        out.paintChannels=Object.assign({},shape.paintChannels||{});out.paintChannels[target]=true;
        out.svgSource=Assets.painted(out.paintSource,out.style,out.paintChannels,out.drawId);
    }
    return out;
}
function create(editor,shape,from,to){
    from=projected(shape,from,editor.tool,editor.viewScale);to=projected(shape,to,editor.tool,editor.viewScale);
    if(Math.hypot(to.x-from.x,to.y-from.y)*editor.viewScale<2)to={x:from.x+60/editor.viewScale,y:from.y};
    return {from:fraction(shape,from),to:fraction(shape,to),stops:[{id:"start",offset:0,color:editor.strokeColor},{id:"end",offset:1,color:editor.secondaryColor}]};
}
function applySet(editor,originals,indices,tool,colour,gradient,reference){
    var shapes=editor.shapes.slice(),from=gradient?position(reference,gradient.from):null,to=gradient?position(reference,gradient.to):null;
    indices.forEach(function(index){
        var shape=originals[index],g=gradient?Object.assign({},gradient,{from:fraction(shape,from),to:fraction(shape,to)}):null;
        shapes[index]=apply(shape,tool,colour,g);
    });return shapes;
}
function move(editor,shape,handle,p){
    var g=V.copy(shape.style[key(editor.tool)]);if(!g)return shape;
    if(handle.role==="stop"){
        var a=position(shape,g.from),b=position(shape,g.to),dx=b.x-a.x,dy=b.y-a.y;
        var t=Math.max(0,Math.min(1,((p.x-a.x)*dx+(p.y-a.y)*dy)/(dx*dx+dy*dy||1)));
        g.stops.forEach(function(stop){if(stop.id===handle.id)stop.offset=t;});
        g.stops.sort(function(a,b){return a.offset-b.offset;});
    }else g[handle.role]=fraction(shape,projected(shape,p,editor.tool,editor.viewScale));
    return g;
}
function changeStop(editor,colour,add,remove){
    var shape=editor.shapes[editor.selected],saved=descriptor(editor);if(!shape||!saved)return;
    var g=V.copy(saved),index=g.stops.findIndex(function(stop){return stop.id===editor.gradientStopId;});
    if(add){
        var left=0,right=1,gap=-1;
        for(var i=1;i<g.stops.length;i++){var distance=g.stops[i].offset-g.stops[i-1].offset;if(distance>gap){gap=distance;left=g.stops[i-1].offset;right=g.stops[i].offset;}}
        var id="colour-"+Date.now().toString(36)+"-"+Math.random().toString(36).slice(2,7);
        g.stops.push({id:id,offset:(left+right)/2,color:editor.strokeColor});editor.gradientStopId=id;
    }else if(index>=0){
        if(remove){if(g.stops.length<=2)return;g.stops.splice(index,1);editor.gradientStopId=g.stops[0].id;}
        else g.stops[index].color=V.color(colour,g.stops[index].color);
    }else return;
    g.stops.sort(function(a,b){return a.offset-b.offset;});
    editor.gestureStarting();editor.remember();
    editor.shapes=applySet(editor,editor.shapes,editor.selectionIndices(),editor.tool,shape.style[channel(editor.tool)],g,shape);
    editor.changed();editor.gestureFinished();
}

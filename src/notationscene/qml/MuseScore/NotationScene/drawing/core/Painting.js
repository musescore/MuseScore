// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as V
.import "SvgAsset.js" as Assets
.import "Context.js" as Context

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

// Build bounded faces from actual segments. Open ends are never implicitly joined.
// Work is restricted to a paint click, never navigation or pointer movement.
function region(shape,x,y){
    var segments=[],last=null,start=null,traceLength=0,explicitClose=false;
    var commands=shape.commands;
    if(shape.paintRanges){commands=[];shape.paintRanges.forEach(function(r){if(!r.regionFill&&(r.style.stroke!=="none"||r.style.fill!=="none"))commands=commands.concat(shape.commands.slice(r.start,r.end));});}
    function edge(a,b){var length=Math.hypot(a.x-b.x,a.y-b.y);if(length>1e-7){traceLength+=length;segments.push({a:a,b:b,cuts:[0,1]});}}
    function finishSubpath(){
        // Legacy pencil/pen paths may omit Z even though the visible caps meet.
        // Never bridge a visible gap or close imported open SVG subpaths.
        var reach=Math.max(0.00001,(shape.style.width||0)*0.5);
        if(!explicitClose&&last&&start&&(shape.kind==="pencil"||shape.kind==="pen")&&traceLength>reach*6&&Math.hypot(last.x-start.x,last.y-start.y)<=reach)edge(last,start);
    }
    commands.forEach(function(c){
        if(c[0]==="M"){finishSubpath();last={x:c[1],y:c[2]};start=last;traceLength=0;explicitClose=false;}
        else if(last&&c[0]==="C"){
            // Subdivide by geometric error, not 32 samples for every tiny pencil curve.
            var tolerance=Math.max(0.02,Math.min(0.2,(shape.style.width||1)*0.1));
            function flatten(a,b,d,e,depth){
                var dx=e.x-a.x,dy=e.y-a.y,length=Math.hypot(dx,dy);
                var error=length?Math.max(Math.abs(dx*(b.y-a.y)-dy*(b.x-a.x)),Math.abs(dx*(d.y-a.y)-dy*(d.x-a.x)))/length:Math.max(Math.hypot(b.x-a.x,b.y-a.y),Math.hypot(d.x-a.x,d.y-a.y));
                var excess=Math.hypot(b.x-a.x,b.y-a.y)+Math.hypot(d.x-b.x,d.y-b.y)+Math.hypot(e.x-d.x,e.y-d.y)-length;
                if(depth>=12||(error<=tolerance&&excess<=tolerance)){edge(a,e);return;}
                var ab=V.blend(a,b,0.5),bd=V.blend(b,d,0.5),de=V.blend(d,e,0.5),u=V.blend(ab,bd,0.5),v=V.blend(bd,de,0.5),mid=V.blend(u,v,0.5);
                flatten(a,ab,u,mid,depth+1);flatten(mid,v,de,e,depth+1);
            }
            var end={x:c[5],y:c[6]};flatten(last,{x:c[1],y:c[2]},{x:c[3],y:c[4]},end,0);last=end;
        }
        else if(last&&c[0]==="L"){var b={x:c[1],y:c[2]};edge(last,b);last=b;}
        else if(last&&c[0]==="Z"){edge(last,start);last=start;explicitClose=true;}
    });
    finishSubpath();
    if(!segments.length||segments.length>12000)return null;
    var ordered=segments.slice().sort(function(a,b){return Math.min(a.a.x,a.b.x)-Math.min(b.a.x,b.b.x);}),checks=0;
    for(var i=0;i<ordered.length;i++)for(var j=i+1;j<ordered.length;j++){
        var a=ordered[i],b=ordered[j];
        if(Math.min(b.a.x,b.b.x)>Math.max(a.a.x,a.b.x)+1e-7)break;
        if(Math.max(a.a.y,a.b.y)<Math.min(b.a.y,b.b.y)||Math.max(b.a.y,b.b.y)<Math.min(a.a.y,a.b.y))continue;
        if(++checks>250000)return null;
        var dx=a.b.x-a.a.x,dy=a.b.y-a.a.y,ex=b.b.x-b.a.x,ey=b.b.y-b.a.y,det=dx*ey-dy*ex;
        if(Math.abs(det)<1e-12)continue;
        var px=b.a.x-a.a.x,py=b.a.y-a.a.y,t=(px*ey-py*ex)/det,u=(px*dy-py*dx)/det;
        if(t>=-1e-8&&t<=1+1e-8&&u>=-1e-8&&u<=1+1e-8){a.cuts.push(Math.max(0,Math.min(1,t)));b.cuts.push(Math.max(0,Math.min(1,u)));}
    }
    var vertices=[],lookup={},edges={},visited={};
    function vertex(p){var key=Math.round(p.x*100000)+":"+Math.round(p.y*100000);if(lookup[key]!==undefined)return lookup[key];lookup[key]=vertices.length;vertices.push({x:p.x,y:p.y,links:[]});return vertices.length-1;}
    segments.forEach(function(s){s.cuts.sort(function(a,b){return a-b;});for(var k=1;k<s.cuts.length;k++){
        if(s.cuts[k]-s.cuts[k-1]<1e-8)continue;
        var a=vertex(V.blend(s.a,s.b,s.cuts[k-1])),b=vertex(V.blend(s.a,s.b,s.cuts[k]));if(a===b||edges[a+":"+b])continue;
        edges[a+":"+b]=edges[b+":"+a]=true;vertices[a].links.push(b);vertices[b].links.push(a);
    }});
    vertices.forEach(function(v){v.links.sort(function(a,b){return Math.atan2(vertices[a].y-v.y,vertices[a].x-v.x)-Math.atan2(vertices[b].y-v.y,vertices[b].x-v.x);});});
    var best=null,bestArea=Infinity,faces=[];
    vertices.forEach(function(v,origin){v.links.forEach(function(next){
        if(visited[origin+":"+next])return;
        var polygon=[],a=origin,b=next,closed=false;
        for(var n=0;n<=segments.length*8;n++){
            if(visited[a+":"+b])break;visited[a+":"+b]=true;polygon.push(vertices[a]);
            var links=vertices[b].links,pos=links.indexOf(a),c=links[(pos+links.length-1)%links.length];a=b;b=c;
            if(a===origin&&b===next){closed=true;break;}
        }
        if(!closed||polygon.length<3)return;
        var area=0,inside=false;
        for(var k=0,l=polygon.length-1;k<polygon.length;l=k++){
            var p=polygon[l],q=polygon[k];area+=p.x*q.y-q.x*p.y;
            if((p.y>y)!==(q.y>y)&&x<(q.x-p.x)*(y-p.y)/(q.y-p.y)+p.x)inside=!inside;
        }
        if(area>1e-8)faces.push({polygon:polygon,area:area,inside:inside});
        if(inside&&area>1e-8&&area<bestArea){bestArea=area;best=polygon;}
    });});
    if(!best)return null;
    function contains(poly,p){var inside=false;for(var k=0,l=poly.length-1;k<poly.length;l=k++){var a=poly[l],b=poly[k];if((a.y>p.y)!==(b.y>p.y)&&p.x<(b.x-a.x)*(p.y-a.y)/(b.y-a.y)+a.x)inside=!inside;}return inside;}
    var holes=faces.filter(function(f){return !f.inside&&f.area<bestArea&&contains(best,f.polygon[0]);});
    holes=holes.filter(function(f){return !holes.some(function(other){return other!==f&&other.area>f.area&&contains(other.polygon,f.polygon[0]);});});
    var result=[];[best].concat(holes.map(function(f){return f.polygon;})).forEach(function(poly){result=result.concat(poly.map(function(p,i){return [i?"L":"M",p.x,p.y];}),[["Z"]]);});return result;
}
function prepare(shape,x,y,editor){
    if(shape.kind==="text")return null;
    if(shape.svgSource){
        if(!editor||!editor.svgVectorize)return null;
        var box=V.bounds([shape]);if(x<box.x||y<box.y||x>box.x+box.width||y>box.y+box.height)return null;
        var converted=editor.svgVectorize(shape.svgSource,shape.svgMatrix);if(!converted)return null;
        var paths=JSON.parse(converted);if(!paths.length)return null;
        var asset=Object.assign({},shape),commands=shape.commands.slice();
        var ranges=[{start:0,end:commands.length,style:V.style("none","none",0.001,1)}];
        paths.forEach(function(path){var start=commands.length;commands=commands.concat(path.commands);ranges.push({start:start,end:commands.length,style:path.style,fillRule:path.fillRule});});
        asset.commands=commands;asset.paintRanges=ranges;delete asset.svgSource;delete asset.svgMatrix;delete asset.paintSource;delete asset.paintChannels;
        shape=asset;
    }
    var face=region(shape,x,y);if(!face)return null;
    var out=Object.assign({},shape),ranges=(shape.paintRanges||[{start:0,end:shape.commands.length,style:shape.style,fillRule:shape.fillRule}]).slice();
    // Repainting an existing face preserves its geometry and other faces.
    for(var i=ranges.length-1;i>=0;i--){var r=ranges[i];if(r.regionFill&&V.insideFill({commands:shape.commands.slice(r.start,r.end)},x,y)){out.activePaintRange=i;return out;}}
    out.commands=shape.commands.concat(face);
    ranges.push({start:shape.commands.length,end:out.commands.length,regionFill:true,fillRule:"evenodd",style:V.style("none","none",0.001,shape.style.opacity)});
    out.paintRanges=ranges;out.activePaintRange=ranges.length-1;return out;
}

function hit(editor,x,y){
    if(channel(editor.tool)==="fill"){
        for(var index=editor.shapes.length-1;index>=0;index--){if(prepare(editor.shapes[index],x,y,editor))return index;}
        return -1;
    }
    var result=V.hit(editor.shapes,x,y,6/Math.max(0.02,editor.viewScale),editor.svgHitTest,editor.nativeShapeHitTest);
    if(result>=0||channel(editor.tool)!=="fill")return result;
    for(var i=editor.shapes.length-1;i>=0;i--){var sh=editor.shapes[i];if(!sh.svgSource&&V.insideFill(sh,x,y))return i;}
    return -1;
}
function descriptor(editor){
    var shape=editor.shapes[editor.selected],style=editor.styleEditActive?editor.selectedNodeStyle:(shape?shape.style:null);
    return gradientTool(editor.tool)&&style?style[key(editor.tool)]:null;
}
function handles(editor){
    var sh=editor.shapes[editor.selected],g=descriptor(editor);if(!editor.drawingEnabled||!sh||!g)return [];
    var a=position(sh,g.from),b=position(sh,g.to),dx=b.x-a.x,dy=b.y-a.y,length=Math.hypot(dx,dy)||1;
    var gap=13/Math.max(0.02,editor.viewScale),nx=-dy/length*gap,ny=dx/length*gap;
    var result=[{role:"from",x:a.x,y:a.y},{role:"to",x:b.x,y:b.y}];
    g.stops.forEach(function(stop,index){var p=V.blend(a,b,stop.offset);
        result.push({role:"stop",id:stop.id,index:index,color:stop.color,x:p.x+nx,y:p.y+ny,axisX:p.x,axisY:p.y});
        result.push({role:"opacity",id:stop.id,index:index,opacity:stop.opacity===undefined?1:stop.opacity,x:p.x-nx,y:p.y-ny,axisX:p.x,axisY:p.y});
    });
    return result;
}
function apply(shape,tool,colour,gradient){
    var out=Object.assign({},shape),target=channel(tool),gradientKey=key(tool);
    function styled(old){var style=Object.assign({},old);style[target]=colour;if(gradient)style[gradientKey]=gradient;else delete style[gradientKey];return style;}
    out.style=styled(shape.style);
    if(shape.paintRanges)out.paintRanges=shape.paintRanges.map(function(range,index){
        var change=target==="fill"?index===shape.activePaintRange:!range.regionFill;
        return change?Object.assign({},range,{style:styled(range.style)}):range;
    });
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
        var shape=originals[index];
        if(!editor.styleEditActive&&channel(tool)==="fill"&&shape.activePaintRange===undefined)return;
        var g=gradient?Object.assign({},gradient,{from:fraction(shape,from),to:fraction(shape,to)}):null;
        if(editor.styleEditActive){
            if(index!==editor.styleEditIndex)return;
            var patch={},target=channel(tool);patch[target]=colour;
            if(g)patch[key(tool)]=g;
            shapes[index]=Context.changedStyle(shape,editor.styleNodeCommand,editor.styleRangeEnd,patch,false,g?[]:[key(tool)]);
        }else shapes[index]=apply(shape,tool,colour,g);
    });return shapes;
}
function move(editor,shape,handle,p){
    var saved=editor.styleEditActive?editor.selectedNodeStyle[key(editor.tool)]:shape.style[key(editor.tool)];
    if(!saved)return null;var g=V.copy(saved);
    if(handle.role==="stop"){
        var a=position(shape,g.from),b=position(shape,g.to),dx=b.x-a.x,dy=b.y-a.y;
        var t=Math.max(0,Math.min(1,((p.x-a.x)*dx+(p.y-a.y)*dy)/(dx*dx+dy*dy||1)));
        g.stops.forEach(function(stop){if(stop.id===handle.id)stop.offset=t;});
        g.stops.sort(function(a,b){return a.offset-b.offset;});
    }else g[handle.role]=fraction(shape,projected(shape,p,editor.tool,editor.viewScale));
    return g;
}
function changeStop(editor,colour,add,remove,opacity,continuous){
    var shape=editor.shapes[editor.selected],saved=descriptor(editor);if(!shape||!saved)return;
    var g=V.copy(saved),index=g.stops.findIndex(function(stop){return stop.id===editor.gradientStopId;});
    if(add){
        var left=0,right=1,gap=-1;
        for(var i=1;i<g.stops.length;i++){var distance=g.stops[i].offset-g.stops[i-1].offset;if(distance>gap){gap=distance;left=g.stops[i-1].offset;right=g.stops[i].offset;}}
        var id="colour-"+Date.now().toString(36)+"-"+Math.random().toString(36).slice(2,7);
        g.stops.push({id:id,offset:(left+right)/2,color:index>=0?g.stops[index].color:editor.gradientStopColor,opacity:index>=0?(g.stops[index].opacity===undefined?1:g.stops[index].opacity):1});editor.gradientStopId=id;
    }else if(index>=0){
        if(remove){if(g.stops.length<=2)return;g.stops.splice(index,1);editor.gradientStopId=g.stops[0].id;}
        else if(opacity!==undefined)g.stops[index].opacity=Math.max(0,Math.min(1,opacity));
        else g.stops[index].color=V.color(colour,g.stops[index].color);
    }else return;
    g.stops.sort(function(a,b){return a.offset-b.offset;});
    if(!continuous){editor.gestureStarting();editor.remember();}
    editor.shapes=applySet(editor,editor.shapes,editor.selectionIndices(),editor.tool,(editor.styleEditActive?editor.selectedNodeStyle:shape.style)[channel(editor.tool)],g,shape);
    editor.changed();if(!continuous)editor.gestureFinished();
}

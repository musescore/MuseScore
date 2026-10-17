// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as V
var eraserCounter=0;

function escapeText(s){return String(s).replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;").replace(/"/g,"&quot;");}
function constrained(a,b,modifiers){
    var dx=b.x-a.x,dy=b.y-a.y;
    if(modifiers.alt){var r=Math.hypot(dx,dy),angle=Math.round(Math.atan2(dy,dx)/(Math.PI/4))*Math.PI/4;dx=r*Math.cos(angle);dy=r*Math.sin(angle);}
    if(modifiers.control){var size=Math.max(Math.abs(dx),Math.abs(dy));dx=(dx<0?-1:1)*size;dy=(dy<0?-1:1)*size;}
    return {a:modifiers.shift||modifiers.control?{x:a.x-dx,y:a.y-dy}:a,b:{x:a.x+dx,y:a.y+dy}};
}
function basicShape(kind,a,b,style){
    if(["line","rectangle","ellipse"].indexOf(kind)>=0)return V.shape(kind,a,b,style);
    if(["arrow","arrow-double","arrow-filled"].indexOf(kind)>=0)return V.arrowShape(kind,a,b,style);
    var x=Math.min(a.x,b.x),y=Math.min(a.y,b.y),w=Math.abs(b.x-a.x),h=Math.abs(b.y-a.y),c;
    if(kind==="triangle")c=[["M",x+w/2,y],["L",x+w,y+h],["L",x,y+h],["Z"]];
    else if(kind==="right-triangle")c=[["M",x,y],["L",x+w,y+h],["L",x,y+h],["Z"]];
    return {kind:kind,commands:c,style:V.copy(style)};
}
function frameCommands(x,y,w,h){return [["M",x,y],["L",x+w,y],["L",x+w,y+h],["L",x,y+h],["Z"],["M",x+w/2,y+h/2]];}
function textSource(content,family,size,width,height,color,layout,fontStyle,alignment){
    var lines=String(content).split("\n"),body="";
    var left=layout?layout.left:size*0.15,baseline=layout?layout.baseline:size*1.15,leading=layout?layout.leading:size*1.35;
    // SVG Tiny positions text elements, not their tspan children.
    lines.forEach(function(line,index){
        var x=alignment===4?(width+left-size*0.15)/2:alignment===2?width-size*0.15:left;
        // QML AlignHCenter is 4; horizontal alignment is stored in Qt flags.
        if(alignment===4)x=(width+left-size*0.15)/2;
        var anchor=alignment===4?'middle':alignment===2?'end':'start';
        body+='<text x="'+x+'" y="'+(baseline+index*leading)+'" text-anchor="'+anchor+'">'+escapeText(line)+'</text>';
    });
    return '<svg xmlns="http://www.w3.org/2000/svg" width="1" height="1" viewBox="0 0 '+width+' '+height+'"><g font-family="'+escapeText(family)+'" font-size="'+Math.max(1,Math.round(size))+'" fill="'+escapeText(color)+'" font-weight="'+((fontStyle&1)?'bold':'normal')+'" font-style="'+((fontStyle&2)?'italic':'normal')+'" text-decoration="'+((fontStyle&4)?'underline ': '')+((fontStyle&8)?'line-through':'')+'" xml:space="preserve">'+body+'</g></svg>';
}
function textDimensions(editor,content,size,family,fontStyle,tight){
    if(tight&&String(content).length&&String(content).indexOf("\n")<0){
        var ink=editor.measureTextLayout(content,size,family,fontStyle);
        return {width:Math.max(1,ink.inkWidth),height:Math.max(1,ink.inkHeight),left:-ink.inkX,baseline:-ink.inkY,leading:ink.height};
    }
    var lines=String(content).split("\n"),width=size,ascent=0,descent=0,leading=size*1.35,left=0;
    lines.forEach(function(line){
        var m=editor.measureTextLayout(line||" ",size,family,fontStyle);
        width=Math.max(width,m.width);left=Math.min(left,m.left);ascent=Math.max(ascent,m.ascent);descent=Math.max(descent,m.descent);leading=Math.max(leading,m.height);
    });
    return {width:width-left+size*0.3,height:Math.max(size*1.5,ascent+descent+(lines.length-1)*leading+size*0.3),left:size*0.15-left,baseline:ascent+size*0.15,leading:leading};
}
function label(editor,x,y,target){
    var content=editor.labelText===undefined?"":editor.labelText,size=editor.labelSize,family=editor.labelFont||"Arial";
    var dimensions=textDimensions(editor,content,size,family),w=dimensions.width,h=dimensions.height;
    var out={kind:"text",textContent:content,textSize:size,textFont:family,textFrameWidth:w,textFrameHeight:h,textPlacement:editor.labelPlacement,
        singleAnchor:true,anchorMode:"note",commands:frameCommands(x,y,w,h),
        svgMatrix:[w,0,0,h,x,y],style:V.style(editor.strokeColor,"none",1,editor.strokeOpacity)};
    out.textLayout=dimensions;out.svgSource=textSource(content,family,size,w,h,editor.strokeColor,dimensions);
    if(target){
        var box=V.bounds([target]),px=box.x+box.width/2,py=box.y+box.height/2;
        var linear=target.kind==="line"||String(target.kind).indexOf("arrow")===0||target.commands.length===2;
        var start=linear?target.commands[0]:null,end=linear?target.commands[1]:null;
        if(start&&end){px=(start[1]+end[1])/2;py=(start[2]+end[2])/2;}
        if(editor.labelPlacement==="above"){x=px-w/2;y=box.y-h-size/3;}
        else if(editor.labelPlacement==="after"){
            if(start&&end){var dx=end[1]-start[1],dy=end[2]-start[2],length=Math.hypot(dx,dy)||1;x=end[1]+dx/length*(size/3+w/2)-w/2;y=end[2]+dy/length*(size/3+h/2)-h/2;}
            else{x=box.x+box.width+size/3;y=py-h/2;}
        }
        else{x=px-w/2;y=py-h/2;}
        out=V.translate(out,x-out.svgMatrix[4],y-out.svgMatrix[5]);
        // Creation chooses placement only. Magnetic linking is an explicit tool.
    }
    return out;
}
function textBox(editor,box){
    var size=Math.max(1,box.height/1.6),w=Math.max(size,box.width),h=Math.max(size*1.6,box.height);
    var sh={kind:"text",textContent:"",textFont:editor.labelFont||"Arial",textSize:size,
        textFrameWidth:w,textFrameHeight:h,textPlacement:"above",singleAnchor:true,anchorMode:"note",
        commands:frameCommands(box.x,box.y,w,h),svgMatrix:[w,0,0,h,box.x,box.y],style:V.style(editor.strokeColor,"none",1,editor.strokeOpacity)};
    sh.textLayout=textDimensions(editor,"",size,sh.textFont);sh.textFrameHeight=Math.max(h,sh.textLayout.height);
    sh.svgMatrix[3]=sh.textFrameHeight;sh.commands=frameCommands(box.x,box.y,w,sh.textFrameHeight);
    sh.svgSource=textSource("",sh.textFont,size,w,sh.textFrameHeight,editor.strokeColor,sh.textLayout);return sh;
}
function replaceText(editor,index,content,family,size,format){
    var old=editor.shapes[index];if(!old||old.kind!=="text")return;
    var sh=V.copy(old),m=old.svgMatrix;
    sh.tightGlyph=old.tightGlyph||/^(Leland|Bravura|Emmentaler|Gonville|MuseJazz|Petaluma)$/i.test(family);
    var oldW=old.textFrameWidth||Math.max(old.textSize,editor.measureText(old.textContent,old.textSize,old.textFont||"Arial")+old.textSize/4);
    var oldH=old.textFrameHeight||old.textSize*1.5;
    var fontStyle=format?format.fontStyle:old.textFontStyle||0,alignment=format?format.alignment:old.textAlignment||1;
    var natural=textDimensions(editor,content,size,family,fontStyle,sh.tightGlyph);
    var w=sh.tightGlyph?natural.width:Math.max(oldW,natural.width),h=sh.tightGlyph?natural.height:Math.max(oldH,natural.height);
    sh.textContent=content;sh.textFont=family;sh.textSize=size;sh.textFontStyle=fontStyle;sh.textAlignment=alignment;sh.textFrameWidth=w;sh.textFrameHeight=h;
    sh.svgMatrix=[m[0]/oldW*w,m[1]/oldW*w,m[2]/oldH*h,m[3]/oldH*h,m[4],m[5]];
    sh.commands=V.transform({commands:frameCommands(0,0,w,h),style:sh.style},[m[0]/oldW,m[1]/oldW,m[2]/oldH,m[3]/oldH,m[4],m[5]]).commands;
    sh.textLayout=natural;sh.svgSource=textSource(content,family,size,w,h,sh.style.stroke,natural,fontStyle,alignment);
    var all=editor.shapes.slice();all[index]=sh;editor.shapes=all;
    editor.bindings=editor.transformBindings(old,sh,editor.bindings,index);
    if(sh.textLink){sh.textLink.dx+=sh.commands[5][1]-old.commands[5][1];sh.textLink.dy+=sh.commands[5][2]-old.commands[5][2];}
    editor.changed();
}
function updateText(editor){
    var old=editor.shapes[editor.selected];if(!old||old.kind!=="text")return;
    editor.remember();replaceText(editor,editor.selected,editor.labelText,editor.labelFont||old.textFont||"Arial",editor.labelSize);
}
function nearestTarget(editor,x,y,excluded){
    var best=null,distance=16/editor.viewScale;
    editor.shapes.forEach(function(sh,index){
        if(index===excluded||sh.kind==="text")return;
        V.linkedHandles(sh,editor.bindings,index).forEach(function(h){
            if(sh.singleAnchor&&h.command!==(sh.anchorCommand===undefined?5:sh.anchorCommand))return;
            var d=Math.hypot(x-h.x,y-h.y);
            if(d<distance){distance=d;best={kind:"shape",drawId:sh.drawId,command:h.command,field:h.field,x:h.x,y:h.y};}
        });
    });
    editor.rhythmMarkers.forEach(function(p){
        var d=Math.hypot(x-p.x,y-(p.railY===undefined?p.y:p.railY));
        if(d<distance){distance=d;best={kind:"rhythm",key:p.key,numerator:p.numerator,denominator:p.denominator,x:p.x,y:p.y,scale:p.scale||1,markerOffsetX:p.bindingX===undefined?0:p.x-p.bindingX};}
    });
    return best;
}
function textTarget(editor,link){
    if(!link)return null;
    if(link.kind==="rhythm")return editor.markerForBinding(link);
    var entry=editor.shapeLookup&&editor.shapeLookup[link.drawId];
    var shape=entry?entry.shape:editor.shapes.find(function(sh){return sh.drawId===link.drawId;});
    var c=shape&&shape.commands[link.command];
    return c?{x:c[link.field],y:c[link.field+1]}:null;
}
function followLabels(editor){
    var touched=false;
    var all=editor.shapes.map(function(sh){
        if(sh.kind!=="text"||!sh.textLink)return sh;
        var p=textTarget(editor,sh.textLink);if(!p)return sh;
        var c=sh.commands[5],dx=p.x+sh.textLink.dx-c[1],dy=p.y+sh.textLink.dy-c[2];
        if(Math.hypot(dx,dy)<0.00001)return sh;
        touched=true;return V.translate(sh,dx,dy);
    });
    if(touched)editor.shapes=all;
    return touched;
}
function attachLabel(editor,index,x,y){
    var p=nearestTarget(editor,x,y,index);if(!p)return;
    var all=editor.shapes.slice(),sh=V.copy(all[index]),c=sh.commands[5];
    sh.textLink=V.copy(p);sh.textLink.dx=c[1]-p.x;sh.textLink.dy=c[2]-p.y;
    all[index]=sh;editor.shapes=all;
    editor.bindings=editor.bindings.filter(function(b){return b.shape!==index;});
}
function bindingLines(editor,includeRhythm){
    var lines=[];
    if(includeRhythm!==false)editor.bindings.forEach(function(b,index){
        if(!b.editable)return;
        var s=editor.shapes[b.shape],c=s&&s.commands[b.command],p=editor.markerForBinding(b);
        if(c&&p)lines.push({kind:"rhythm",index:index,shape:b.shape,x:c[b.field],y:c[b.field+1],x2:p.x,y2:editor.markerRailY(p)});
    });
    editor.shapes.forEach(function(s,index){
        var p=textTarget(editor,s.textLink);
        if(s.kind==="text"&&p){var c=s.commands[5];lines.push({kind:"text",shape:index,x:c[1],y:c[2],x2:p.x,y2:s.textLink.kind==="rhythm"?editor.markerRailY(p):p.y});}
    });
    var selected=editor.shapes[editor.selected];
    if(selected)fragmentHandles(editor).forEach(function(h){
        var override=selected.fragmentOverrides&&selected.fragmentOverrides[h.partKey];
        var link=override&&override[h.side]&&override[h.side].link,p=link?editor.markerForBinding(link):null;
        if(p)lines.push({kind:"fragment",shape:editor.selected,handle:h,x:h.x,y:h.y,x2:p.x,y2:editor.markerRailY(p)});
    });
    return lines;
}
function lineAt(editor,x,y){
    var hit=null,limit=7/editor.viewScale;
    bindingLines(editor).forEach(function(l){
        var dx=l.x2-l.x,dy=l.y2-l.y,t=Math.max(0,Math.min(1,((x-l.x)*dx+(y-l.y)*dy)/(dx*dx+dy*dy||1)));
        var d=Math.hypot(x-l.x-t*dx,y-l.y-t*dy);
        if(d<limit){hit=l;limit=d;}
    });return hit;
}
function unlink(editor,line){
    if(!line)return;
    editor.remember();var all=editor.shapes.slice(),sh=V.copy(all[line.shape]);
    if(line.kind==="text"){delete sh.textLink;sh.suppressAutoAnchors=true;}
    else if(line.kind==="fragment"){
        var h=line.handle,edit=sh.fragmentOverrides[h.partKey],delta=edit[h.side];
        delta.dx=(h.x-h.unitOriginX)/h.spatium-edit[h.side+"Pivot"];
        delta.dy=(h.y-h.unitOriginY)/h.spatium-edit[h.side+"PivotY"];
        delete delta.link;
    }
    else{
        var b=editor.bindings[line.index];sh.suppressAutoAnchors=true;
        sh.unlinkedPoints=sh.unlinkedPoints||{};sh.unlinkedPoints[b.command+":"+b.field]=true;
        editor.bindings=editor.bindings.filter(function(b,i){return i!==line.index;});
    }
    all[line.shape]=sh;editor.shapes=all;editor.changed();
}
function merge(editor){
    var indices=editor.selectedShapes.slice().sort(function(a,b){return a-b;});if(indices.length<2)return;
    var originals=indices.map(function(i){return editor.shapes[i];}),box=V.bounds(originals),paths=[];
    if(editor.svgVectorize){
        var converted=editor.svgVectorize(V.svg(originals,box),[box.width,0,0,box.height,box.x,box.y]);
        if(converted)paths=JSON.parse(converted);
    }
    if(!paths.length){
        editor.status=editor.trStatus("These drawings cannot be fused as editable paths. Use Group to preserve them.","Estes desenhos não podem ser fundidos como caminhos editáveis. Use Agrupar para preservá-los.");return;
    }
    var merged={kind:"fused",singleAnchor:true,anchorMode:"note",style:V.style("none","none",0.001,1),commands:[],paintRanges:[]};
    paths.forEach(function(path){
        var start=merged.commands.length;merged.commands=merged.commands.concat(path.commands);
        merged.paintRanges.push({start:start,end:merged.commands.length,style:path.style,fillRule:path.fillRule});
    });
    if(merged.commands.length>=100000){editor.status=editor.trStatus("These drawings cannot be fused as editable paths. Use Group to preserve them.","Estes desenhos não podem ser fundidos como caminhos editáveis. Use Agrupar para preservá-los.");return;}
    merged.anchorCommand=merged.commands.length;merged.commands.push(["M",box.x+box.width/2,box.y+box.height/2]);
    editor.remember();
    V.ensureIdentity(merged);var oldToNew={},all=[];
    editor.shapes.forEach(function(sh,i){if(indices.indexOf(i)<0){oldToNew[i]=all.length;all.push(sh);}});
    all=all.map(function(sh){
        if(sh.kind!=="text"||!sh.textLink||!originals.some(function(s){return s.drawId===sh.textLink.drawId;}))return sh;
        var out=V.copy(sh),center=out.commands[5];
        out.textLink={kind:"shape",drawId:merged.drawId,command:merged.anchorCommand,field:1,dx:center[1]-box.x-box.width/2,dy:center[2]-box.y-box.height/2};
        return out;
    });
    editor.bindings=editor.bindings.filter(function(b){return indices.indexOf(b.shape)<0;}).map(function(b){var c=V.copy(b);c.shape=oldToNew[b.shape];return c;});
    all.push(merged);editor.shapes=all;editor.selected=all.length-1;editor.linkSingleAnchor(editor.selected);editor.changed();
}
function flatten(commands,clipLeft,clipRight){
    var result=[],last=null,start=null;
    commands.forEach(function(c,index){
        if(c[0]==="M"){last={x:c[1],y:c[2]};start=last;result.push({x:last.x,y:last.y,move:true,command:index});return;}
        if(!last)return;
        var end=c[0]==="Z"?start:{x:c[c.length-2],y:c[c.length-1]};
        var count=c[0]==="C"?48:1;
        if(count>1&&clipLeft!==undefined&&clipRight!==undefined){
            var minX=Math.min(last.x,end.x,c[1],c[3]),maxX=Math.max(last.x,end.x,c[1],c[3]);
            // Endpoint handles need detailed samples only at a clipping edge.
            if(maxX<clipLeft||minX>clipRight||(minX>=clipLeft&&maxX<=clipRight))count=1;
        }
        for(var j=1;j<=count;j++){
            var p=c[0]==="C"?V.curvePoint(last,c,j/count):end;
            result.push({x:p.x,y:p.y,command:j===count?index:-1});
        }last=end;
    });return result;
}
function buildFragmentHandles(fragments,drawId){
    if(!drawId)return [];
    var handles=[];
    fragments.forEach(function(f){
        if(f.id!==drawId||!f.commands||(!f.startsPiece&&!f.endsPiece))return;
        var points=flatten(f.commands,f.cutLeft,f.cutRight),visible=[],last=null;
        points.forEach(function(p){
            if(p.move)last=null;
            if(last){
                [f.cutLeft,f.cutRight].forEach(function(x){
                    if((last.x-x)*(p.x-x)<0){var t=(x-last.x)/(p.x-last.x);visible.push({x:x,y:last.y+(p.y-last.y)*t});}
                });
            }
            if(p.x>=f.cutLeft-0.001&&p.x<=f.cutRight+0.001)visible.push(p);
            last=p;
        });
        if(!visible.length)return;
        var first=visible[0],end=visible[visible.length-1];
        if(f.startsPiece)handles.push({x:first.x,y:first.y,endpoint:true,fragment:true,derived:f.derived,partKey:f.partKey,side:"start",spatium:f.spatium,unitOriginX:f.unitOriginX,unitOriginY:f.unitOriginY,before:f.before,after:f.after});
        if(f.endsPiece)handles.push({x:end.x,y:end.y,endpoint:true,fragment:true,derived:f.derived,partKey:f.partKey,side:"end",spatium:f.spatium,unitOriginX:f.unitOriginX,unitOriginY:f.unitOriginY,before:f.before,after:f.after});
    });return handles;
}
function fragmentHandles(editor){
    if(editor.fragmentHandleCache!==undefined)return editor.fragmentHandleCache;
    var sh=editor.shapes[editor.selected];
    return buildFragmentHandles(editor.nativeFragments,sh&&!sh.svgSource?sh.drawId:"");
}
function moveFragment(editor,original,handle,dx,dy){
    // A derived endpoint changes one override, never its complete source SVG.
    var sh=Object.assign({},original),all=editor.shapes.slice();sh.fragmentOverrides=Object.assign({},original.fragmentOverrides||{});
    var edit=Object.assign({},sh.fragmentOverrides[handle.partKey]||{});
    edit.detached=true;edit.before=handle.before;edit.after=handle.after;
    var old=edit[handle.side]||{dx:0,dy:0};
    if(edit[handle.side+"Pivot"]===undefined)edit[handle.side+"Pivot"]=(handle.x-handle.unitOriginX)/handle.spatium-old.dx;
    if(edit[handle.side+"PivotY"]===undefined)edit[handle.side+"PivotY"]=(handle.y-handle.unitOriginY)/handle.spatium-old.dy;
    edit[handle.side]={dx:old.dx+dx/handle.spatium,dy:old.dy+dy/handle.spatium};
    if(editor.tool==="anchors"){
        var target=editor.nearbyAnchor(handle.x+dx,handle.y+dy,null);
        if(target)edit[handle.side].link={key:target.key,numerator:target.numerator,denominator:target.denominator,
            markerOffsetX:target.bindingX===undefined?0:target.x-target.bindingX,offsetY:(handle.y+dy-target.y)/handle.spatium};
    }
    sh.fragmentOverrides[handle.partKey]=edit;all[editor.selected]=sh;editor.shapes=all;
    editor.liveFragmentEdit=true;editor.changed();
}
function deformationFragments(editor){
    var session=editor.deformationSession,sh=session&&editor.shapes[session.shape];
    if(!session||session.shape!==editor.selected||!sh||sh.svgSource||sh.singleAnchor||sh.drawId!==session.original.drawId)return null;
    var original=session.original.commands,current=sh.commands;
    if(original.length!==current.length)return null;
    var fragments=editor.nativeFragments.filter(function(f){return f.id===sh.drawId&&f.commands&&f.commands.length===current.length;});
    if(!fragments.length)return null;
    var changes=[],minDx=0,minDy=0,maxDx=0,maxDy=0;
    current.forEach(function(c,index){
        var old=original[index];if(c===old)return;
        for(var field=1;field+1<c.length;field+=2){
            var dx=c[field]-old[field],dy=c[field+1]-old[field+1];
            if(!dx&&!dy)continue;
            changes.push({command:index,field:field,dx:dx,dy:dy});
            minDx=Math.min(minDx,dx);maxDx=Math.max(maxDx,dx);minDy=Math.min(minDy,dy);maxDy=Math.max(maxDy,dy);
        }
    });
    var region=editor.paintRegion,pieces=[];
    fragments.forEach(function(f,index){
        if(f.x+f.width+maxDx<region.x||f.x+minDx>region.x+region.width||f.y+f.height+maxDy<region.y||f.y+minDy>region.y+region.height)return;
        // Full curves are mapped into each system by native layout. Reuse that
        // map for the transient delta, never connect page-space system ends.
        var commands=f.commands.slice(),copied={};
        changes.forEach(function(change){
            var ci=change.command;
            if(!copied[ci]){commands[ci]=commands[ci].slice();copied[ci]=true;}
            commands[ci][change.field]+=change.dx;commands[ci][change.field+1]+=change.dy;
        });
        var left=index===0?region.x:Math.max(region.x,f.x),right=index===fragments.length-1?region.x+region.width:Math.min(region.x+region.width,f.x+f.width);
        if(right<left)return;
        pieces.push({commands:commands,style:sh.style,clipRect:{x:left,y:region.y,width:right-left,height:region.height}});
    });
    return {shape:sh,pieces:pieces};
}
function inside(p,polygon){
    var yes=false;
    for(var i=0,j=polygon.length-1;i<polygon.length;j=i++){
        var a=polygon[i],b=polygon[j];
        if((a.y>p.y)!==(b.y>p.y)&&p.x<(b.x-a.x)*(p.y-a.y)/(b.y-a.y)+a.x)yes=!yes;
    }return yes;
}
function eraseOpenRegion(sh,polygon,region){
    var commands=[],mapping={},last=null,start=null,startIndex=-1,pen=null,changed=false,broken=false;
    function at(a,b){return a&&b&&Math.abs(a.x-b.x)<0.00001&&Math.abs(a.y-b.y)<0.00001;}
    function move(p){
        if(at(pen,p))return;
        var index=commands.length;commands.push(["M",p.x,p.y]);pen=p;
        if(at(p,start)&&startIndex>=0)mapping[startIndex]={command:index,field:1};
    }
    sh.commands.forEach(function(c,index){
        if(c[0]==="M"){last={x:c[1],y:c[2]};start=last;startIndex=index;pen=null;broken=false;return;}
        if(!last)return;
        var end=c[0]==="Z"?start:{x:c[c.length-2],y:c[c.length-1]};
        var minX=Math.min(last.x,end.x),maxX=Math.max(last.x,end.x),minY=Math.min(last.y,end.y),maxY=Math.max(last.y,end.y);
        if(c[0]==="C"){minX=Math.min(minX,c[1],c[3]);maxX=Math.max(maxX,c[1],c[3]);minY=Math.min(minY,c[2],c[4]);maxY=Math.max(maxY,c[2],c[4]);}
        if(maxX<region.minX||minX>region.maxX||maxY<region.minY||minY>region.maxY){
            move(last);
            mapping[index]={command:commands.length,preserveFields:c[0]!=="Z",field:1};
            commands.push(c[0]==="Z"&&broken?["L",end.x,end.y]:c);pen=end;last=end;return;
        }
        // Only intersected cubic bounds need sampling. Keeping distant curves
        // exact also avoids turning a complete pencil path into thousands of
        // straight segments after a small eraser stroke.
        var count=c[0]==="C"?48:1,previous=last;
        for(var n=1;n<=count;n++){
            var p=c[0]==="C"?V.curvePoint(last,c,n/count):end;
            var cuts=segmentCuts(previous,p,polygon);
            for(var k=0;k<cuts.length-1;k++){
                var a=cuts[k],b=cuts[k+1],middle=(a+b)/2;
                if(inside({x:previous.x+(p.x-previous.x)*middle,y:previous.y+(p.y-previous.y)*middle},polygon)){
                    pen=null;changed=true;broken=true;continue;
                }
                var from={x:previous.x+(p.x-previous.x)*a,y:previous.y+(p.y-previous.y)*a},to={x:previous.x+(p.x-previous.x)*b,y:previous.y+(p.y-previous.y)*b};
                move(from);commands.push(["L",to.x,to.y]);pen=to;
            }
            previous=p;
        }
        if(pen&&at(pen,end))mapping[index]={command:commands.length-1,field:1};
        last=end;
    });
    if(!changed)return null;
    var out=Object.assign({},sh);out.commands=commands;delete out.rawPoints;
    return {shape:commands.some(function(c){return c[0]==="L"||c[0]==="C";})?out:null,mapping:mapping};
}
function eraseRegion(editor,polygon,quiet){
    var changed=false,all=[],links=[];
    var linksByShape={};
    editor.bindings.forEach(function(binding){if(!linksByShape[binding.shape])linksByShape[binding.shape]=[];linksByShape[binding.shape].push(binding);});
    var minX=Infinity,minY=Infinity,maxX=-Infinity,maxY=-Infinity;
    polygon.forEach(function(p){minX=Math.min(minX,p.x);minY=Math.min(minY,p.y);maxX=Math.max(maxX,p.x);maxY=Math.max(maxY,p.y);});
    editor.shapes.forEach(function(sh,index){
        var entry=editor.shapeLookup&&editor.shapeLookup[sh.drawId+(sh.fragmentId?"/"+sh.fragmentId:"")];
        var bounds=entry&&entry.shape===sh?entry.bounds:V.bounds([sh]),candidates=!(bounds.x>maxX||bounds.y>maxY||bounds.x+bounds.width<minX||bounds.y+bounds.height<minY);
        var atomicText=sh.kind==="text";
        var textHit=atomicText&&candidates&&editor.svgRegionHitTest?editor.svgRegionHitTest(sh,polygon):false;
        var open=candidates&&!atomicText&&!sh.svgSource&&sh.style.fill==="none"?eraseOpenRegion(sh,polygon,{minX:minX,minY:minY,maxX:maxX,maxY:maxY}):null;
        var samples=candidates&&!atomicText&&(sh.svgSource||sh.style.fill!=="none")?flatten(sh.commands):[],hit=textHit||!!open||samples.some(function(p){return inside(p,polygon);});
        if(!hit){
            // Include segments crossing the region with both endpoints outside.
            hit=samples.some(function(p,i){return i>0&&!p.move&&segmentCuts(samples[i-1],p,polygon).length>2;});
        }
        if(candidates&&!hit&&!atomicText&&(sh.svgSource||sh.style.fill!=="none"))hit=polygon.some(function(p){return V.hit([sh],p.x,p.y,0,editor.svgHitTest,null)>=0;});
        var out=sh,mapping={};
        if(hit){
            changed=true;
            if(atomicText)out=null;
            else if(open){out=open.shape;mapping=open.mapping;}
            else if(sh.svgSource||sh.style.fill!=="none"){
                out=Object.assign({},sh);
                var b=V.bounds([sh]),hole=polygon.map(function(p){return p.x+" "+p.y;}).join(" L ");
                var outer=(b.x-1)+" "+(b.y-1)+" L "+(b.x+b.width+1)+" "+(b.y-1)+" L "+(b.x+b.width+1)+" "+(b.y+b.height+1)+" L "+(b.x-1)+" "+(b.y+b.height+1);
                var id="erase"+Date.now().toString(36)+"-"+(++eraserCounter).toString(36)+"-"+index;
                out.svgSource='<svg xmlns="http://www.w3.org/2000/svg" viewBox="'+[b.x,b.y,b.width,b.height].join(" ")+'"><defs><clipPath id="'+id+'"><path clip-rule="evenodd" fill-rule="evenodd" d="M '+outer+' Z M '+hole+' Z"/></clipPath></defs><g clip-path="url(#'+id+')">'+V.body([sh])+'</g></svg>';
                out.svgMatrix=[b.width,0,0,b.height,b.x,b.y];out.commands=frameCommands(b.x,b.y,b.width,b.height);out.singleAnchor=true;out.anchorMode="note";
            }
        }
        if(!out)return;
        var next=all.length;all.push(out);
        (linksByShape[index]||[]).forEach(function(b){
            var link=Object.assign({},b);
            if(hit){
                var target=mapping[b.command];if(!target)return;
                if(!target.preserveFields&&b.field!==sh.commands[b.command].length-2)return;
                link.command=target.command;link.field=target.preserveFields?b.field:target.field;
            }
            link.shape=next;links.push(link);
        });
    });
    if(changed){editor.shapes=all;editor.bindings=links;editor.selected=-1;if(!quiet)editor.changed();}
    return changed;
}
function segmentCuts(a,b,polygon){
    var cuts=[0,1],dx=b.x-a.x,dy=b.y-a.y;
    for(var i=0;i<polygon.length;i++){
        var p=polygon[i],q=polygon[(i+1)%polygon.length],ex=q.x-p.x,ey=q.y-p.y,det=dx*ey-dy*ex;
        if(Math.abs(det)<1e-12)continue;
        var t=((p.x-a.x)*ey-(p.y-a.y)*ex)/det,u=((p.x-a.x)*dy-(p.y-a.y)*dx)/det;
        if(t>0&&t<1&&u>=0&&u<=1)cuts.push(t);
    }return cuts.sort(function(x,y){return x-y;});
}
function region(kind,a,b){
    var x=Math.min(a.x,b.x),y=Math.min(a.y,b.y),w=Math.abs(a.x-b.x),h=Math.abs(a.y-b.y);
    if(kind==="triangle")return [{x:x+w/2,y:y},{x:x+w,y:y+h},{x:x,y:y+h}];
    if(kind!=="ellipse")return [{x:x,y:y},{x:x+w,y:y},{x:x+w,y:y+h},{x:x,y:y+h}];
    var p=[];for(var i=0;i<64;i++){var t=i*Math.PI/32;p.push({x:x+w/2+w/2*Math.cos(t),y:y+h/2+h/2*Math.sin(t)});}return p;
}

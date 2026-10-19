// SPDX-License-Identifier: GPL-3.0-only
.import "Parametric.js" as P

// Shared by the QML editor, standalone plugin and preview. Coordinates are editor units.

function copy(v) { return JSON.parse(JSON.stringify(v)); }

var identityCounter=0;
function ensureIdentity(shape) {
    if(!shape.drawId){
        var source=shape.reflowSource&&shape.reflowSource.shape;
        shape.drawId=source&&source.drawId?source.drawId:
            "draw-"+Date.now().toString(36)+"-"+(++identityCounter).toString(36)+"-"+Math.random().toString(36).slice(2,12);
    }
    return shape.drawId;
}
function svgIdentity(shape) {
    return ensureIdentity(shape)+(shape.fragmentId?"/"+shape.fragmentId:"");
}

function num(v) { return Math.round(v * 1000) / 1000; }

function color(v, fallback) { return /^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$/.test(v) ? v : fallback; }

function style(stroke, fill, width, opacity) {

    return {stroke:stroke === "none" ? "none" : color(stroke,"#161616"),fill:fill === "none" ? "none" : color(fill,"#ffffff"),

            width:Math.max(0.001,Math.min(1000000,width)),opacity:Math.max(0,Math.min(1,opacity))};

}

function line(x,y) { return ["L",num(x),num(y)]; }

// Arc-length sampling makes smoothing independent of input event density.
// Bounded linear passes keep the preview responsive even at maximum smoothing.
function filteredPoints(points,amount,scale){
    if(points.length<3||!amount)return points;
    var a=Math.max(0,Math.min(1,amount)),step=(1.5+8*a*a)/Math.max(0.02,scale||1),length=0;
    for(var i=1;i<points.length;i++)length+=Math.hypot(points[i].x-points[i-1].x,points[i].y-points[i-1].y);
    step=Math.max(step,length/2048);
    var sampled=[points[0]],carry=step;
    for(var i=1;i<points.length;i++){
        var p=points[i-1],q=points[i],d=Math.hypot(q.x-p.x,q.y-p.y);
        if(d===0)continue;
        for(var at=carry;at<d;at+=step){var t=at/d,r={x:p.x+(q.x-p.x)*t,y:p.y+(q.y-p.y)*t};
            if(p.pressure!==undefined||q.pressure!==undefined)r.pressure=(p.pressure===undefined?1:p.pressure)*(1-t)+(q.pressure===undefined?1:q.pressure)*t;
            sampled.push(r);
        }
        carry=at-d;
    }
    sampled.push(points[points.length-1]);
    var radius=1+Math.round(a*a*5),passes=1+Math.round(a*2);
    for(var pass=0;pass<passes;pass++){
        var original=sampled;
        sampled=original.map(function(p,index){
            if(index===0||index===original.length-1)return p;
            var x=0,y=0,total=0;
            for(var j=Math.max(0,index-radius);j<=Math.min(original.length-1,index+radius);j++){
                var weight=radius+1-Math.abs(j-index);x+=original[j].x*weight;y+=original[j].y*weight;total+=weight;
            }
            var out={x:p.x*(1-a)+x/total*a,y:p.y*(1-a)+y/total*a};
            if(p.pressure!==undefined)out.pressure=p.pressure;
            return out;
        });
    }
    return sampled;
}
function smooth(points,amount,scale){return smoothFiltered(filteredPoints(points,amount,scale),amount);}
function smoothFiltered(p,amount){
    if(!p.length)return [];
    var a=Math.max(0,Math.min(1,amount)),out=[["M",num(p[0].x),num(p[0].y)]];
    for(var i=1;i<p.length;i++){
        if(!a){out.push(line(p[i].x,p[i].y));continue;}
        var p0=p[Math.max(0,i-2)],p1=p[i-1],p2=p[i],p3=p[Math.min(p.length-1,i+1)];
        out.push(["C",num(p1.x+(p2.x-p0.x)/6),num(p1.y+(p2.y-p0.y)/6),num(p2.x-(p3.x-p1.x)/6),num(p2.y-(p3.y-p1.y)/6),num(p2.x),num(p2.y)]);
    }
    if(p.length===1)out.push(line(p[0].x+0.1,p[0].y));
    return out;
}

// Sweep a rotated, flattened nib along the centreline. Controls remain centreline nodes.
function nibHull(points) {
    var sorted=points.slice().sort(function(a,b){return a.x-b.x||a.y-b.y}),lower=[],upper=[];
    function cross(a,b,c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
    sorted.forEach(function(p){while(lower.length>1&&cross(lower[lower.length-2],lower[lower.length-1],p)<=0)lower.pop();lower.push(p);});
    sorted.reverse().forEach(function(p){while(upper.length>1&&cross(upper[upper.length-2],upper[upper.length-1],p)<=0)upper.pop();upper.push(p);});
    lower.pop();upper.pop();return lower.concat(upper);
}
function nibPolygons(paths,style) {
    var nib=style.nib,angle=nib.angle*Math.PI/180,co=Math.cos(angle),si=Math.sin(angle),ratio=Math.max(0.05,1-nib.flatten),shear=nib.shape==="square"?Math.tan(Math.max(-60,Math.min(60,Number(nib.tilt)||0))*Math.PI/180):0,out=[];
    function stamp(p,direction){
        var co=Math.cos(angle),si=Math.sin(angle),automatic=nib.followDirection?direction:0,ac=Math.cos(automatic),as=Math.sin(automatic);
        var radius=style.width*Math.max(0.05,p.pressure===undefined?1:p.pressure)/2,points=[];
        var count=nib.shape==="square"?4:32;
        for(var i=0;i<count;i++){
            var x,y;if(count===4){x=(i===0||i===3?-1:1)*radius;y=(i<2?-1:1)*radius;}
            else{x=Math.cos(i*2*Math.PI/count)*radius;y=Math.sin(i*2*Math.PI/count)*radius;}
            x+=shear*y;
            y*=ratio;
            var rotatedX=x*co-y*si,rotatedY=x*si+y*co;
            points.push({x:p.x+rotatedX*ac-rotatedY*as,y:p.y+rotatedX*as+rotatedY*ac});
        }return points;
    }
    paths.forEach(function(points){
        var last=null;points.forEach(function(p,index){var before=points[Math.max(0,index-1)],after=points[Math.min(points.length-1,index+1)];var next=stamp(p,Math.atan2(after.y-before.y,after.x-before.x)),polygon=nibHull(last?last.concat(next):next);last=next;
            polygon.forEach(function(p,i){out.push([i?"L":"M",p.x,p.y]);});out.push(["Z"]);
        });
    });return out;
}
// Keep a bounded strong cache: QV4 WeakMap lookups can crash after collection
// of the temporary centreline arrays produced by pencil previews.
var nibOutlineCache = [];
function nibOutline(commands,style) {
    if(style.stroke==="none"||!style.nib||(style.nib.shape==="round"&&style.nib.flatten===0))return null;
    var signature=[style.width,style.nib.shape,style.nib.flatten,style.nib.angle,style.nib.tilt||0,!!style.nib.followDirection,pathData(commands)].join("/");
    for(var i=0;i<nibOutlineCache.length;i++){
        var saved=nibOutlineCache[i];
        if(saved.source===commands&&saved.signature===signature)return saved.commands;
    }
    var outline=buildNibOutline(commands,style);
    nibOutlineCache.push({source:commands,signature:signature,commands:outline});
    var weight=nibOutlineCache.reduce(function(n,entry){return n+entry.commands.length;},0);
    while(nibOutlineCache.length>1 && (nibOutlineCache.length>64 || weight>100000))weight-=nibOutlineCache.shift().commands.length;
    return outline;
}
function buildNibOutline(commands,style) {
    if(style.stroke==="none"||!style.nib||(style.nib.shape==="round"&&style.nib.flatten===0))return null;
    var paths=[],points=[],current=null,start=null,tolerance=Math.max(0.05,Math.min(0.4,style.width*0.025));
    function finish(){if(points.length)paths.push(points);points=[];}
    function cubic(a,b,c,d,depth){
        var dx=d.x-a.x,dy=d.y-a.y,length=Math.hypot(dx,dy);
        var error=length?Math.max(Math.abs(dx*(b.y-a.y)-dy*(b.x-a.x)),Math.abs(dx*(c.y-a.y)-dy*(c.x-a.x)))/length:Math.max(Math.hypot(b.x-a.x,b.y-a.y),Math.hypot(c.x-a.x,c.y-a.y));
        if(depth>=10||(error<=tolerance&&Math.hypot(b.x-a.x,b.y-a.y)+Math.hypot(c.x-b.x,c.y-b.y)+Math.hypot(d.x-c.x,d.y-c.y)-length<=tolerance)){points.push(d);return;}
        function mid(p,q){return {x:(p.x+q.x)/2,y:(p.y+q.y)/2};}
        var ab=mid(a,b),bc=mid(b,c),cd=mid(c,d),abc=mid(ab,bc),bcd=mid(bc,cd),m=mid(abc,bcd);
        cubic(a,ab,abc,m,depth+1);cubic(m,bcd,cd,d,depth+1);
    }
    commands.forEach(function(c){
        if(c[0]==="M"){finish();current={x:c[1],y:c[2]};start=current;points.push(current);}
        else if(c[0]==="L"){current={x:c[1],y:c[2]};points.push(current);}
        else if(c[0]==="C"&&current){var end={x:c[5],y:c[6]};cubic(current,{x:c[1],y:c[2]},{x:c[3],y:c[4]},end,0);current=end;}
        else if(c[0]==="Z"&&start){points.push(start);current=start;}
    });finish();return nibPolygons(paths,style);
}

function pencilStroke(points,amount,style,scale,pressure){
    var p=filteredPoints(points,amount,scale),closed=false;
    if(p.length>=4){
        var first=p[0],last=p[p.length-1],reach=Math.max(0.05,(style.width||1)*0.5),length=0;
        for(var k=1;k<p.length;k++)length+=Math.hypot(p[k].x-p[k-1].x,p[k].y-p[k-1].y);
        closed=length>reach*6&&Math.hypot(last.x-first.x,last.y-first.y)<=reach;
        if(closed){p=p.slice();p[p.length-1]=Object.assign({},last,{x:first.x,y:first.y});}
    }
    if(!pressure||p.length<2){var commands=smoothFiltered(p,amount);if(closed)commands.push(["Z"]);return {kind:"pencil",rawPoints:points,commands:commands,style:style};}
    if(style.nib&&(style.nib.shape!=="round"||style.nib.flatten>0)){
        return {kind:"pencil-pressure",rawPoints:points,commands:nibPolygons([p],style),style:{stroke:"none",fill:style.stroke,width:0.001,opacity:style.opacity}};
    }
    var left=[],right=[];
    p.forEach(function(point,index){
        var before=p[Math.max(0,index-1)],after=p[Math.min(p.length-1,index+1)],dx=after.x-before.x,dy=after.y-before.y,d=Math.hypot(dx,dy)||1;
        var radius=style.width*Math.max(0.05,point.pressure===undefined?1:point.pressure)/2;
        left.push({x:point.x-dy/d*radius,y:point.y+dx/d*radius});right.push({x:point.x+dy/d*radius,y:point.y-dx/d*radius});
    });
    function cap(point,before,after,start){
        var angle=Math.atan2(after.y-before.y,after.x-before.x)+(start?-Math.PI/2:Math.PI/2),radius=style.width*Math.max(0.05,point.pressure===undefined?1:point.pressure)/2,out=[];
        for(var i=1;i<8;i++){var t=angle-Math.PI*i/8;out.push({x:point.x+Math.cos(t)*radius,y:point.y+Math.sin(t)*radius});}
        return out;
    }
    var contour=left.concat(cap(p[p.length-1],p[p.length-2],p[p.length-1],false),right.reverse(),cap(p[0],p[0],p[1],true)),commands=contour.map(function(point,index){return [index?"L":"M",num(point.x),num(point.y)];});commands.push(["Z"]);
    return {kind:"pencil-pressure",rawPoints:points,commands:commands,style:{stroke:"none",fill:style.stroke,width:0.001,opacity:style.opacity}};
}

function pen(nodes) {

    if (!nodes.length) return [];

    var out=[["M",nodes[0].x,nodes[0].y]];

    for(var i=1;i<nodes.length;i++) {

        var a=nodes[i-1],b=nodes[i];
        if(b.move){out.push(["M",b.x,b.y]);continue;}

        out.push(["C",a.x+a.dx,a.y+a.dy,b.x-b.dx,b.y-b.dy,b.x,b.y]);
        if(b.close)out.push(["Z"]);

    }

    return out;

}

function shape(kind,a,b,s) {

    var x=Math.min(a.x,b.x),y=Math.min(a.y,b.y),w=Math.abs(b.x-a.x),h=Math.abs(b.y-a.y),k=0.5522847498;

    var c;

    if(kind==="line") c=[["M",a.x,a.y],line(b.x,b.y)];

    else if(kind==="rectangle") c=[["M",x,y],line(x+w,y),line(x+w,y+h),line(x,y+h),["Z"]];

    else if(kind==="ellipse") {

        var cx=x+w/2,cy=y+h/2,rx=w/2,ry=h/2;

        c=[["M",cx+rx,cy],["C",cx+rx,cy+k*ry,cx+k*rx,cy+ry,cx,cy+ry],

           ["C",cx-k*rx,cy+ry,cx-rx,cy+k*ry,cx-rx,cy],

           ["C",cx-rx,cy-k*ry,cx-k*rx,cy-ry,cx,cy-ry],

           ["C",cx+k*rx,cy-ry,cx+rx,cy-k*ry,cx+rx,cy],["Z"]];

    } else c=[["M",a.x,a.y],line(b.x,b.y)];

    return {commands:c,style:copy(s)};

}

function nibExtentMultiplier(style){
    var nib=style.nib;if(!nib||nib.shape!=="square")return 1;
    var ratio=Math.max(0.05,1-(Number(nib.flatten)||0)),shear=Math.tan(Math.max(-60,Math.min(60,Number(nib.tilt)||0))*Math.PI/180);
    return Math.hypot(1+Math.abs(shear),ratio);
}
function bounds(shapes) {

    var x=Infinity,y=Infinity,r=-Infinity,b=-Infinity;
    for(var i=0;i<shapes.length;i++){
        var sh=shapes[i],extent=sh.style.width*nibExtentMultiplier(sh.style);
        (sh.paintRanges||[]).forEach(function(range){extent=Math.max(extent,range.style.width*nibExtentMultiplier(range.style));});
        var pad=sh.svgSource?0:extent/2+1,lx=Infinity,ly=Infinity,rx=-Infinity,by=-Infinity;
        (sh.derivedWholeCommands||sh.commands).forEach(function(c){for(var n=1;n<c.length;n+=2){
            lx=Math.min(lx,c[n]-pad);ly=Math.min(ly,c[n+1]-pad);rx=Math.max(rx,c[n]+pad);by=Math.max(by,c[n+1]+pad);
        }});
        if(sh.clipRect){var clip=sh.clipRect;lx=Math.max(lx,clip.x);ly=Math.max(ly,clip.y);rx=Math.min(rx,clip.x+clip.width);by=Math.min(by,clip.y+clip.height);}
        if(rx>=lx&&by>=ly){x=Math.min(x,lx);y=Math.min(y,ly);r=Math.max(r,rx);b=Math.max(b,by);}
    }
    return isFinite(x)?{x:x,y:y,width:Math.max(1,r-x),height:Math.max(1,b-y)}:{x:0,y:0,width:1,height:1};

}

function pathData(commands) {

    return commands.map(function(c) {return c[0]+c.slice(1).map(num).join(" ");}).join(" ");

}

function paintBounds(commands){
    var x=Infinity,y=Infinity,right=-Infinity,bottom=-Infinity;
    commands.forEach(function(c){for(var i=1;i<c.length;i+=2){x=Math.min(x,c[i]);y=Math.min(y,c[i+1]);right=Math.max(right,c[i]);bottom=Math.max(bottom,c[i+1]);}});
    return isFinite(x)?{x:x,y:y,width:Math.max(1,right-x),height:Math.max(1,bottom-y)}:{x:0,y:0,width:1,height:1};
}
function gradientPoints(gradient,box){return {x1:box.x+gradient.from.x*box.width,y1:box.y+gradient.from.y*box.height,x2:box.x+gradient.to.x*box.width,y2:box.y+gradient.to.y*box.height};}
function gradientStopRgba(stop){
    var hex=color(stop.color,"#161616").slice(1),a=stop.opacity===undefined?1:stop.opacity;
    return "rgba("+parseInt(hex.slice(0,2),16)+","+parseInt(hex.slice(2,4),16)+","+parseInt(hex.slice(4,6),16)+","+Math.max(0,Math.min(1,a))+")";
}
function gradientSvg(gradient,id,box){
    var p=gradientPoints(gradient,box);
    return '<linearGradient id="'+id+'" gradientUnits="userSpaceOnUse" spreadMethod="pad" x1="'+num(p.x1)+'" y1="'+num(p.y1)+'" x2="'+num(p.x2)+'" y2="'+num(p.y2)+'">'+gradient.stops.map(function(stop){return '<stop offset="'+num(stop.offset)+'" stop-color="'+color(stop.color,'#161616')+'" stop-opacity="'+num(stop.opacity===undefined?1:stop.opacity)+'"/>';}).join('')+'</linearGradient>';
}
function checkedStyle(saved){
    var result=style(saved.stroke,saved.fill,saved.width,saved.opacity);
    result.lineCap=["round","butt","square"].indexOf(saved.lineCap)>=0?saved.lineCap:"round";result.linePattern=["solid","dash","dot"].indexOf(saved.linePattern)>=0?saved.linePattern:"solid";
    if(saved.nib)result.nib={followDirection:!!saved.nib.followDirection,shape:saved.nib.shape==="square"?"square":"round",flatten:Math.max(0,Math.min(0.95,Number(saved.nib.flatten)||0)),angle:Math.max(-180,Math.min(180,Number(saved.nib.angle)||0)),tilt:Math.max(-60,Math.min(60,Number(saved.nib.tilt)||0))};
    ["strokeGradient","fillGradient"].forEach(function(key){
        var g=saved[key];if(!g)return;
        if(!g.from||!g.to||![g.from.x,g.from.y,g.to.x,g.to.y].every(function(v){return typeof v==="number"&&isFinite(v)&&Math.abs(v)<=100000;})||!Array.isArray(g.stops)||g.stops.length<2||g.stops.length>128)throw Error("Gradiente inválido.");
        result[key]={from:{x:g.from.x,y:g.from.y},to:{x:g.to.x,y:g.to.y},stops:g.stops.map(function(stop,index){
            if(typeof stop.offset!=="number"||!isFinite(stop.offset)||stop.offset<0||stop.offset>1)throw Error("Gradiente inválido.");
            return {id:String(stop.id||index).slice(0,64),offset:stop.offset,color:color(stop.color,"#161616"),opacity:stop.opacity===undefined?1:Math.max(0,Math.min(1,Number(stop.opacity)||0))};
        }).sort(function(a,b){return a.offset-b.offset;})};
    });return result;
}

function body(shapes, parentId) {

    return shapes.map(function(sh,index) {var s=sh.style;

        var clip=sh.clipRect,id=(parentId||'draw')+'clip'+index
        var prefix=clip?'<defs><clipPath id="'+id+'"><rect x="'+num(clip.x)+'" y="'+num(clip.y)+'" width="'+num(clip.width)+'" height="'+num(clip.height)+'"/></clipPath></defs><g clip-path="url(#'+id+')">':''
        if(sh.paintRanges)return prefix+body(paintedParts(sh),id+'range')+(clip?'</g>':'');
        var outline=nibOutline(sh.derivedWholeCommands||sh.commands,s);
        if(outline){
            var fillShape=copy(sh);delete fillShape.style.nib;fillShape.style.stroke="none";delete fillShape.style.strokeGradient;
            var nibStyle=copy(s);delete nibStyle.nib;nibStyle.fill=s.stroke;nibStyle.stroke="none";
            if(s.strokeGradient)nibStyle.fillGradient=s.strokeGradient;else delete nibStyle.fillGradient;
            delete nibStyle.strokeGradient;
            var strokeShape={commands:outline,style:nibStyle,paintFrame:sh.paintFrame||paintBounds(sh.derivedWholeCommands||sh.commands),clipRect:sh.clipRect};
            return body(s.fill!=="none"?[fillShape,strokeShape]:[strokeShape],id+"nib");
        }
        if(sh.svgSource)return prefix+'<g opacity="'+num(s.opacity)+'" transform="matrix('+sh.svgMatrix.map(num).join(' ')+')">'+sh.svgSource+'</g>'+(clip?'</g>':'');
        var definitions='',stroke=s.stroke,fill=s.fill,box=sh.paintFrame||paintBounds(sh.derivedWholeCommands||sh.commands);
        if(s.strokeGradient){definitions+=gradientSvg(s.strokeGradient,id+'stroke',box);stroke='url(#'+id+'stroke)';}
        if(s.fillGradient){definitions+=gradientSvg(s.fillGradient,id+'fill',box);fill='url(#'+id+'fill)';}
        return prefix+(definitions?'<defs>'+definitions+'</defs>':'')+'<path d="'+pathData(sh.derivedWholeCommands||sh.commands)+'" stroke="'+stroke+'" fill="'+fill+

        '" stroke-width="'+num(s.width)+'" opacity="'+num(s.opacity)+

        '" fill-rule="'+(sh.fillRule==="evenodd"?"evenodd":"nonzero")+'" stroke-linecap="'+(s.lineCap||"round")+'" stroke-dasharray="'+(s.linePattern==="dash"?num(s.width*4)+" "+num(s.width*2):s.linePattern==="dot"?num(s.width)+" "+num(s.width*2):"none")+'" stroke-linejoin="round"/>'+(clip?'</g>':'');}).join("\n");

}
function rangeCommands(commands,start,end){
 var out=commands.slice(start,end);if(start<=0||!out.length||out[0][0]==="M")return out;
 var previous=commands[start-1],origin=null;for(var i=0;i<end;i++){if(commands[i][0]==="M")origin=commands[i];}
 if(previous.length>=3)out=[["M",previous[previous.length-2],previous[previous.length-1]]].concat(out);
 if(origin)out=out.map(function(c){return c[0]==="Z"?["L",origin[1],origin[2]]:c;});return out;
}
var paintedPartsCache = [];
function paintedParts(shape){
    for(var i=0;i<paintedPartsCache.length;i++){
        var saved=paintedPartsCache[i];
        if(saved.commands===shape.commands && saved.ranges===shape.paintRanges && saved.frame===shape.paintFrame)return saved.parts;
    }
    var frame=shape.paintFrame||paintBounds(shape.commands);
    var parts=shape.paintRanges.map(function(range){return {commands:rangeCommands(shape.commands,range.start,range.end),style:range.style,fillRule:range.fillRule,paintFrame:frame};});
    paintedPartsCache.push({commands:shape.commands,ranges:shape.paintRanges,frame:shape.paintFrame,parts:parts});
    var weight=paintedPartsCache.reduce(function(n,entry){return n+entry.commands.length;},0);
    while(paintedPartsCache.length>1 && (paintedPartsCache.length>64 || weight>100000))weight-=paintedPartsCache.shift().commands.length;
    return parts;
}

function svg(shapes, box) {

    box=box||bounds(shapes);

    var ids=shapes.map(svgIdentity).join(' ').replace(/&/g,'&amp;').replace(/"/g,'&quot;').replace(/</g,'&lt;');
    return '<svg xmlns="http://www.w3.org/2000/svg" data-musedraw-origin="drawing-tools" data-musedraw-id="'+ids+'" width="'+num(box.width)+'" height="'+num(box.height)+

        '" viewBox="'+[box.x,box.y,box.width,box.height].map(num).join(" ")+

        '" preserveAspectRatio="none">\n'+body(shapes)+'\n</svg>';

}

function translate(sh, dx,dy) {
    // SVG source strings and untouched metadata are immutable. Serializing
    // them for each pointer event also copied every saved fragment/source.
    var out=Object.assign({},sh);
    if(sh.wave)out.wave=P.transform(sh.wave,[1,0,0,1,dx,dy]);
    if(sh.parametric){out.parametric=copy(sh.parametric);out.parametric.matrix=multiply([1,0,0,1,dx,dy],sh.parametric.matrix||[1,0,0,1,0,0]);}
    if(out.svgMatrix){out.svgMatrix=out.svgMatrix.slice();out.svgMatrix[4]+=dx;out.svgMatrix[5]+=dy;}
    out.commands=sh.commands.map(function(command){var c=command.slice();for(var n=1;n<c.length;n+=2){c[n]+=dx;c[n+1]+=dy;}return c;});
    if(out.rawPoints)out.rawPoints=out.rawPoints.map(function(p){return Object.assign({},p,{x:p.x+dx,y:p.y+dy});});
    if(out.textLink)out.textLink=Object.assign({},out.textLink);

    return out;

}

function multiply(a,b){return [a[0]*b[0]+a[2]*b[1],a[1]*b[0]+a[3]*b[1],a[0]*b[2]+a[2]*b[3],a[1]*b[2]+a[3]*b[3],a[0]*b[4]+a[2]*b[5]+a[4],a[1]*b[4]+a[3]*b[5]+a[5]];}

// Arrow parameters are stored with ordinary path commands. Saving, printing and
// older readers still render the path; only the editor needs these parameters.
function arrowGeometry(shape){
    if(!shape||shape.svgSource||shape.paintRanges||["arrow","arrow-double","arrow-filled"].indexOf(shape.kind)<0)return null;
    var original=shape.commands,filled=shape.kind==="arrow-filled";
    var ops=filled?"MLLLLLLZ":shape.kind==="arrow-double"?"MLMLLMLL":"MLMLL";
    if(!original||!original.length||original.some(function(command){
        return ["M","L","Z"].indexOf(command[0])<0||command.slice(1).some(function(value){return !isFinite(value);});
    }))return null;
    var indices=shape.arrowTopology,c=[];
    if(Array.isArray(indices)&&indices.length===ops.length&&indices.every(function(index,i){
        return Number.isInteger(index)&&index>=0&&index<original.length&&original[index][0]===ops[i]&&(!i||index>indices[i-1]);
    }))c=indices.map(function(index){return original[index];});
    else{
        indices=[];
        // Rhythmic anchors insert collinear L commands. Recognize the outline
        // without removing those commands or changing any binding indices.
        original.forEach(function(command,i){
            if(command[0]==="L"&&i>0&&i+1<original.length&&original[i+1][0]==="L"&&original[i-1][0]!=="Z"){
                var a=original[i-1],b=original[i+1],dx=command[1]-a[1],dy=command[2]-a[2],ex=b[1]-command[1],ey=b[2]-command[2];
                var product=Math.hypot(dx,dy)*Math.hypot(ex,ey);
                if(Math.abs(dx*ey-dy*ex)<=Math.max(0.000001,product*0.000001)&&dx*ex+dy*ey>=0)return;
            }
            indices.push(i);c.push(command);
        });
    }
    if(c.map(function(command){return command[0];}).join("")!==ops)return null;
    var layout=[],corner=0;
    original.forEach(function(command,i){
        while(corner<indices.length-1&&indices[corner]<i)corner++;
        if(indices[corner]===i){layout.push({at:corner});return;}
        if(!corner||command[0]!=="L"){layout.push(null);return;}
        var a=c[corner-1],b=c[corner],dx=b[1]-a[1],dy=b[2]-a[2],d=dx*dx+dy*dy;
        layout.push({at:corner,t:d>0?Math.max(0,Math.min(1,((command[1]-a[1])*dx+(command[2]-a[2])*dy)/d)):0});
    });
    if(layout.some(function(slot){return !slot;}))return null;
    var a=filled?{x:(c[0][1]+c[6][1])/2,y:(c[0][2]+c[6][2])/2}:{x:c[0][1],y:c[0][2]};
    var b={x:c[filled?3:1][1],y:c[filled?3:1][2]},length=Math.hypot(b.x-a.x,b.y-a.y);
    if(length<0.000001)return null;
    var ux=(b.x-a.x)/length,uy=(b.y-a.y)/length,back=c[2];
    var head=Math.max(0.001,(b.x-back[1])*ux+(b.y-back[2])*uy);
    var half=Math.abs((back[1]-b.x)*(-uy)+(back[2]-b.y)*ux);
    var shaft=filled?Math.hypot(c[0][1]-a.x,c[0][2]-a.y):Math.max(0.001,shape.style.width/2);
    return {a:a,b:b,length:length,ux:ux,uy:uy,headLength:head,headAngle:Math.atan2(half,head),shaftHalfWidth:shaft,
        indices:indices,layout:layout};
}
function arrowParameters(shape,geometry){
    if(shape.arrowParameters)return Object.assign({},shape.arrowParameters);
    // Legacy arrows retain their saved dimensions until explicitly adjusted.
    return {headLength:geometry.headLength,headAngle:geometry.headAngle,shaftHalfWidth:geometry.shaftHalfWidth,
        headSizeManual:true,headAngleManual:true,shaftManual:true};
}
function rigidArrowHeads(shape){
    var g=arrowGeometry(shape),heads={};if(!g)return heads;
    g.layout.forEach(function(slot,index){
        var at=slot.at,tip=-1;
        if(shape.kind==="arrow-filled"){
            if((at===1&&slot.t===undefined)||at===2||(at===3&&slot.t!==undefined)||at===4||at===5)tip=g.indices[3];
        }else if(at>=2)tip=shape.kind==="arrow-double"&&at>=5?g.indices[0]:g.indices[1];
        if(tip<0)return;
        heads[index]={tip:tip,direction:shape.kind==="arrow-filled"?g.indices[0]:tip===g.indices[0]?g.indices[1]:g.indices[0]};
        if(shape.kind==="arrow-filled")heads[index].direction2=g.indices[6];
    });
    return heads;
}
function rebuildArrow(shape,a,b,parameters){
    var out=Object.assign({},shape),p=Object.assign({},parameters),dx=b.x-a.x,dy=b.y-a.y,length=Math.hypot(dx,dy);
    if(length<0.000001)return out;
    var ux=dx/length,uy=dy/length;
    var head=Math.min(Math.max(0.001,p.headLength),length*(shape.kind==="arrow-double"?0.45:0.9));
    var shaft=Math.min(Math.max(0.001,p.shaftHalfWidth),length/4);
    var half=head*Math.tan(Math.max(0.03,Math.min(1.4,p.headAngle)));
    if(shape.kind==="arrow-filled")half=Math.max(half,shaft*1.15);
    var commands=[];
    function add(op,t,n){commands.push([op,a.x+ux*t-uy*n,a.y+uy*t+ux*n]);}
    if(shape.kind==="arrow-filled"){
        add("M",0,-shaft);add("L",length-head,-shaft);add("L",length-head,-half);
        add("L",length,0);add("L",length-head,half);add("L",length-head,shaft);add("L",0,shaft);commands.push(["Z"]);
    }else{
        add("M",0,0);add("L",length,0);add("M",length-head,-half);add("L",length,0);add("L",length-head,half);
        if(shape.kind==="arrow-double"){add("M",head,-half);add("L",0,0);add("L",head,half);}
    }
    var previous=arrowGeometry(shape);
    out.arrowTopology=previous?previous.indices.slice():commands.map(function(command,index){return index;});
    if(previous&&previous.layout.length!==commands.length){
        var corners=commands;
        commands=previous.layout.map(function(slot){
            var b=corners[slot.at];if(slot.t===undefined)return b.slice();
            var a=corners[slot.at-1];return ["L",a[1]+(b[1]-a[1])*slot.t,a[2]+(b[2]-a[2])*slot.t];
        });
    }
    out.commands=commands;out.arrowParameters=p;out.rigidArrowHeads=rigidArrowHeads(out);return out;
}
function arrowShape(kind,a,b,style){
    var shaft=Math.max(0.001,style.width*(kind==="arrow-filled"?1.5:0.5));
    return rebuildArrow({kind:kind,commands:[],style:copy(style)},a,b,
        {headLength:Math.max(style.width*4,shaft*4),headAngle:Math.atan(0.55),shaftHalfWidth:shaft,
            headSizeManual:false,headAngleManual:false,shaftManual:false});
}
function arrowControls(shape,scale){
    var g=arrowGeometry(shape);if(!g)return [];
    var gap=24/Math.max(0.000001,scale||1)+Math.max(0.001,shape.style.width)/2;
    var half=g.headLength*Math.tan(g.headAngle),outside=Math.max(half,g.shaftHalfWidth)+gap;
    function control(role,t,n,displayT,displayN,color){
        return {role:role,targetX:g.a.x+g.ux*t-g.uy*n,targetY:g.a.y+g.uy*t+g.ux*n,
            x:g.a.x+g.ux*displayT-g.uy*displayN,y:g.a.y+g.uy*displayT+g.ux*displayN,color:color};
    }
    var controls=[control("start",0,0,-gap,0,"#00a9d6"),control("end",g.length,0,g.length+gap,0,"#00a9d6"),
        control("head",g.length-g.headLength,0,g.length-g.headLength,-outside,"#e5b700")];
    if(shape.kind==="arrow-filled")controls.push(control("shaft-width",(g.length-g.headLength)*0.45,g.shaftHalfWidth,
        (g.length-g.headLength)*0.45,g.shaftHalfWidth+gap,"#83af13"));
    return controls;
}
function adjustArrowHead(shape,dx,dy,scale){
    var g=arrowGeometry(shape);if(!g)return shape;
    var p=arrowParameters(shape,g);
    // Screen-horizontal and screen-vertical motions are independent, even
    // when the arrow itself points diagonally or vertically.
    if(Math.abs(dx*scale)>0.5){p.headLength=Math.max(0.01,g.headLength+dx);p.headSizeManual=true;}
    if(Math.abs(dy*scale)>0.5){p.headAngle=Math.max(0.03,Math.min(1.4,g.headAngle-dy*scale*0.008));p.headAngleManual=true;}
    return rebuildArrow(shape,g.a,g.b,p);
}
function adjustArrow(shape,role,point,modifiers){
    var g=arrowGeometry(shape);if(!g)return shape;
    var p=arrowParameters(shape,g),a=g.a,b=g.b;
    var t=(point.x-a.x)*g.ux+(point.y-a.y)*g.uy,n=(point.x-a.x)*(-g.uy)+(point.y-a.y)*g.ux;
    if(role==="start"||role==="end"){
        var fixed=role==="start"?b:a,old=role==="start"?a:b;
        if(modifiers&&modifiers.alt){
            var angle=Math.round(Math.atan2(point.y-fixed.y,point.x-fixed.x)/(Math.PI/4))*Math.PI/4;
            var radius=Math.hypot(point.x-fixed.x,point.y-fixed.y);
            point={x:fixed.x+radius*Math.cos(angle),y:fixed.y+radius*Math.sin(angle)};
        }
        if(modifiers&&modifiers.shift)fixed={x:fixed.x-(point.x-old.x),y:fixed.y-(point.y-old.y)};
        a=role==="start"?point:fixed;b=role==="start"?fixed:point;
        if(Math.hypot(b.x-a.x,b.y-a.y)<0.01)return shape;
    }else if(role==="head-size"){
        p.headLength=Math.max(0.01,g.length-t);p.headSizeManual=true;
    }else if(role==="head-angle"){
        p.headAngle=Math.max(0.03,Math.min(1.4,Math.atan2(Math.abs(n),Math.max(0.01,g.length-t))));p.headAngleManual=true;
    }else if(role==="shaft-width"){
        p.shaftHalfWidth=Math.max(0.01,Math.abs(n));p.shaftManual=true;
        if(!p.headSizeManual)p.headLength=Math.max(shape.style.width*4,p.shaftHalfWidth*4);
    }
    return rebuildArrow(shape,a,b,p);
}
function arrowWithStyle(shape,style){
    var g=arrowGeometry(shape),out=Object.assign({},shape,{style:copy(style)});if(!g)return out;
    var p=arrowParameters(shape,g);
    if(!p.shaftManual)p.shaftHalfWidth=style.width*(shape.kind==="arrow-filled"?1.5:0.5);
    if(!p.headSizeManual)p.headLength=Math.max(style.width*4,p.shaftHalfWidth*4);
    return rebuildArrow(out,g.a,g.b,p);
}
function transform(shape,m){
    var arrow=arrowGeometry(shape);
    if(arrow){
        function mapped(p){return {x:m[0]*p.x+m[2]*p.y+m[4],y:m[1]*p.x+m[3]*p.y+m[5]};}
        // Stretch endpoints, not the head contour or the stroke width.
        return transformedPaint(shape,rebuildArrow(shape,mapped(arrow.a),mapped(arrow.b),arrowParameters(shape,arrow)),m);
    }
    var out=Object.assign({},shape);
    if(shape.wave)out.wave=P.transform(shape.wave,m);
    if(shape.parametric){out.parametric=copy(shape.parametric);out.parametric.matrix=multiply(m,shape.parametric.matrix||[1,0,0,1,0,0]);}
    out.commands=shape.commands.map(function(command){var c=command.slice();for(var i=1;i<c.length;i+=2){var x=c[i],y=c[i+1];c[i]=m[0]*x+m[2]*y+m[4];c[i+1]=m[1]*x+m[3]*y+m[5]}return c;});
    if(out.svgMatrix)out.svgMatrix=multiply(m,out.svgMatrix);
    if(out.rawPoints)out.rawPoints=out.rawPoints.map(function(p){return Object.assign({},p,{x:m[0]*p.x+m[2]*p.y+m[4],y:m[1]*p.x+m[3]*p.y+m[5]});});
    if(out.textLink)out.textLink=Object.assign({},out.textLink);
    out.style=Object.assign({},shape.style);
    out.style.width=Math.max(0.001,Math.min(1000000,out.style.width*Math.sqrt(Math.abs(m[0]*m[3]-m[1]*m[2]))));
    return transformedPaint(shape,out,m);
}

function transformedPaint(before,after,m){
    if(before.svgSource)return after;
    var oldBox=paintBounds(before.commands),box=paintBounds(after.commands);
    function styled(style){
        var result=Object.assign({},style);
        if(style.nib){var a=style.nib.angle*Math.PI/180,x=Math.cos(a),y=Math.sin(a);result.nib=Object.assign({},style.nib,{angle:Math.atan2(m[1]*x+m[3]*y,m[0]*x+m[2]*y)*180/Math.PI});}
        ["strokeGradient","fillGradient"].forEach(function(key){var saved=style[key];if(!saved)return;
            function map(p){var x=oldBox.x+p.x*oldBox.width,y=oldBox.y+p.y*oldBox.height;return {x:(m[0]*x+m[2]*y+m[4]-box.x)/box.width,y:(m[1]*x+m[3]*y+m[5]-box.y)/box.height};}
            result[key]=Object.assign({},saved,{from:map(saved.from),to:map(saved.to)});
        });return result;
    }
    after.style=styled(after.style);
    if(after.paintRanges)after.paintRanges=after.paintRanges.map(function(range){return Object.assign({},range,{style:styled(range.style)});});
    return after;
}

function hit(shapes,x,y,tolerance,svgContains,nativeContains) {
    tolerance=typeof tolerance==="number"?tolerance:6;
    for(var i=shapes.length-1;i>=0;i--){
        var sh=shapes[i],b=bounds([sh]);
        if(sh.kind==="text"&&sh.svgMatrix){
            var m=sh.svgMatrix,det=m[0]*m[3]-m[1]*m[2];
            if(Math.abs(det)>1e-12){
                var px=x-m[4],py=y-m[5],u=(m[3]*px-m[2]*py)/det,v=(-m[1]*px+m[0]*py)/det;
                if(u>=0&&u<=1&&v>=0&&v<=1)return i;
            }
            continue;
        }
        if(nativeContains){var nativeHit=nativeContains(sh,x,y,tolerance);if(nativeHit!==null){if(nativeHit)return i;continue;}}
        if(sh.paintRanges){if(hit(paintedParts(sh),x,y,tolerance,svgContains,null)>=0)return i;continue;}
        if(x<b.x-tolerance||x>b.x+b.width+tolerance||y<b.y-tolerance||y>b.y+b.height+tolerance)continue;
        if(sh.style&&sh.style.opacity===0)continue;
        if(sh.svgSource&&svgContains){if(svgContains(sh,x,y,tolerance))return i;continue;}
        if(nearestSegment(sh,x,y,tolerance+(sh.style.width||0)/2))return i;
        if(sh.style.fill&&sh.style.fill!=="none"&&insideFill(sh,x,y))return i;
    }
    return -1;
}

function insideFill(sh,x,y){
    var contours=[],points=[],last=null;
    sh.commands.forEach(function(c){
        if(c[0]==="M"){if(points.length)contours.push(points);points=[{x:c[1],y:c[2]}];last=points[0];}
        else if(c[0]==="C"&&last){for(var k=1;k<=32;k++)points.push(curvePoint(last,c,k/32));last=points[points.length-1];}
        else if(c[0]==="L"){last={x:c[1],y:c[2]};points.push(last);}
        else if(c[0]==="Z"){if(points.length)contours.push(points);points=[];last=null;}
    });
    if(points.length)contours.push(points);
    var winding=0,crossings=0;
    contours.forEach(function(p){for(var i=0,j=p.length-1;i<p.length;j=i++){
        var a=p[j],b=p[i];
        if((a.y>y)!==(b.y>y)&&x<(b.x-a.x)*(y-a.y)/(b.y-a.y)+a.x){crossings++;winding+=b.y>a.y?1:-1;}
    }});
    return sh.fillRule==="evenodd"?crossings%2===1:winding!==0;
}

function handles(sh) {

    var out=[];

    sh.commands.forEach(function(c,i) {for(var n=1;n<c.length;n+=2)

        out.push({command:i,field:n,x:c[n],y:c[n+1],control:c[0]==="C"&&n<5});});

    return out;

}

function blend(a,b,t) {return {x:a.x+(b.x-a.x)*t,y:a.y+(b.y-a.y)*t};}

function curvePoint(a,c,t) {

    var b={x:c[1],y:c[2]},d={x:c[3],y:c[4]},e={x:c[5],y:c[6]};

    return blend(blend(blend(a,b,t),blend(b,d,t),t),blend(blend(b,d,t),blend(d,e,t),t),t);

}

function nearestSegment(sh,x,y,maxDistance) {

    var last=null,start=null,best=null,dist=typeof maxDistance==="number"?maxDistance*maxDistance:12*12;

    for(var i=0;i<sh.commands.length;i++){

        var c=sh.commands[i]

        if(c[0]==="M"){last={x:c[1],y:c[2]};start=last;continue;}

        if(!last)continue;

        var end=c[0]==="Z"?start:{x:c[c.length-2],y:c[c.length-1]};

        // A Bezier curve stays inside its control-point bounds. Reject distant
        // segments before performing the expensive closest-point search.
        var minX=Math.min(last.x,end.x),maxX=Math.max(last.x,end.x),minY=Math.min(last.y,end.y),maxY=Math.max(last.y,end.y);
        if(c[0]==="C"){minX=Math.min(minX,c[1],c[3]);maxX=Math.max(maxX,c[1],c[3]);minY=Math.min(minY,c[2],c[4]);maxY=Math.max(maxY,c[2],c[4]);}
        var boxDx=Math.max(minX-x,0,x-maxX),boxDy=Math.max(minY-y,0,y-maxY);
        if(boxDx*boxDx+boxDy*boxDy>dist){last=end;continue;}

        var t=0;

        if(c[0]==="C"){

            var localDistance=Infinity, sample=0;

            for(var j=0;j<=24;j++){

                var q=curvePoint(last,c,j/24),sd=(q.x-x)*(q.x-x)+(q.y-y)*(q.y-y);

                if(sd<localDistance){localDistance=sd;sample=j;}

            }

            var lo=Math.max(0,(sample-1)/24),hi=Math.min(1,(sample+1)/24);

            for(var k=0;k<24;k++){

                var t1=lo+(hi-lo)/3,t2=hi-(hi-lo)/3;

                var p1=curvePoint(last,c,t1),p2=curvePoint(last,c,t2);

                var d1=(p1.x-x)*(p1.x-x)+(p1.y-y)*(p1.y-y);

                var d2=(p2.x-x)*(p2.x-x)+(p2.y-y)*(p2.y-y);

                if(d1<d2)hi=t2;else lo=t1;

            }

            t=(lo+hi)/2;

        }else{

            var dx=end.x-last.x,dy=end.y-last.y,length=dx*dx+dy*dy;

            t=length?Math.max(0,Math.min(1,((x-last.x)*dx+(y-last.y)*dy)/length)):0;

        }

        t=Math.max(0.000001,Math.min(0.999999,t));

        var p=c[0]==="C"?curvePoint(last,c,t):blend(last,end,t);

        var d=(p.x-x)*(p.x-x)+(p.y-y)*(p.y-y);

        if(d<dist){dist=d;best={command:i,t:t,distance:Math.sqrt(d)};}

        last=end;

    }

    return best;

}

function insertPoint(sh,index,t,inPlace) {

    var out=inPlace?sh:copy(sh),c=out.commands[index],prev=out.commands[index-1];

    if(!c||!prev||(c[0]!=="C"&&c[0]!=="L"))return out;

    var a={x:prev[prev.length-2],y:prev[prev.length-1]},end={x:c[c.length-2],y:c[c.length-1]},pieces;

    if(c[0]==="L"){

        // Keep the subdivision in full precision. Quantizing its endpoint
        // can leave it before the rhythmic cut and cause repeated splitting.
        var p=blend(a,end,t);pieces=[["L",p.x,p.y],["L",end.x,end.y]];

    }else{

        var b={x:c[1],y:c[2]},d={x:c[3],y:c[4]};

        var ab=blend(a,b,t),bd=blend(b,d,t),de=blend(d,end,t);

        var left=blend(ab,bd,t),right=blend(bd,de,t),middle=blend(left,right,t);

        pieces=[["C",ab.x,ab.y,left.x,left.y,middle.x,middle.y],

                ["C",right.x,right.y,de.x,de.y,end.x,end.y]];

    }

    out.commands.splice(index,1,pieces[0],pieces[1]);
    if(out.segmentStyles)out.segmentStyles=out.segmentStyles.map(function(m){return Object.assign({},m,{command:m.command>=index?m.command+1:m.command});});
    if(out.arrowTopology)out.arrowTopology=out.arrowTopology.map(function(at){return at>=index?at+1:at;});
    if(out.paintRanges)out.paintRanges=out.paintRanges.map(function(range){var r=Object.assign({},range);if(r.start>index)r.start++;if(r.end>index)r.end++;return r});
    if(out.anchorCommand!==undefined&&out.anchorCommand>=index)out.anchorCommand++;
    return out;

}

// Insert exact curve intersections with the nearest staff's rhythmic grid.

function autoAnchors(sh, markers, shapeIndex, manualHeads) {

    var out=copy(sh), links=[],manual=copy(manualHeads||[]),commandMapping=sh.commands.map(function(c,i){return i;});
    if(sh.wave){
        [out.commands.length-2,out.commands.length-1].forEach(function(ci){var c=out.commands[ci],target=null,best=Infinity;markers.forEach(function(m){var d=Math.hypot(c[1]-m.x,c[2]-m.y);if(d<best){best=d;target=m;}});if(target)links.push({shape:shapeIndex,command:ci,field:1,key:target.key,numerator:target.numerator,denominator:target.denominator,scale:target.scale||1,offsetX:(c[1]-(target.bindingX===undefined?target.x:target.bindingX))/(target.scale||1),offsetY:(c[2]-target.y)/(target.scale||1),editable:true});});
        return {shape:out,bindings:links};
    }
    var arrow=arrowGeometry(out);
    if(arrow)out.arrowTopology=arrow.indices.slice();

    if(!markers.length)return {shape:out,bindings:links};

    var box=bounds([sh]), middle=box.y+box.height/2, nearest=null, distance=Infinity;
    markers.forEach(function(m){
        var dx=Math.max(box.x-m.x,0,m.x-box.x-box.width),dy=Math.abs(m.y-middle);
        var d=Math.hypot(dx,dy);
        if(d<distance){nearest=m;distance=d;}
    });
    var grid=markers.filter(function(m){
        var sameRow=nearest.row!==undefined?m.row===nearest.row:Math.abs(m.y-nearest.y)<1;
        return sameRow&&m.x>=box.x&&m.x<=box.x+box.width;
    }).sort(function(a,b){return a.x-b.x;});

    grid=grid.filter(function(m,i){return !i||Math.abs(m.x-grid[i-1].x)>0.01;});

    grid.forEach(function(m){

        var previous=null;

        for(var i=0;i<out.commands.length;i++){

            var c=out.commands[i];

            if(c[0]==="Z"){previous=null;continue;}

            var end={x:c[c.length-2],y:c[c.length-1]},field=c.length-2;

            if(previous&&(c[0]==="L"||c[0]==="C")&&(previous.x-m.x)*(end.x-m.x)<-0.00001){

                var split=(m.x-previous.x)/(end.x-previous.x);
                if(c[0]==="C"){
                    var lo=0,hi=1;
                    for(var j=0;j<40;j++){
                        var t=(lo+hi)/2,p=curvePoint(previous,c,t);
                        if((p.x<m.x)===(previous.x<m.x))lo=t;else hi=t;
                    }
                    split=(lo+hi)/2;
                }
                out=insertPoint(out,i,split,true);
                commandMapping=commandMapping.map(function(ci){return ci>=i?ci+1:ci;});

                links.forEach(function(b){if(b.command>=i)b.command++;});
                manual.forEach(function(b){if(b.command>=i)b.command++;});

                c=out.commands[i];field=c.length-2;
                // The following segment starts exactly on the cut. Its
                // endpoint product is therefore zero, so it cannot be split
                // again at this same anchor because of floating point error.
                c[field]=m.x;
                end={x:c[field],y:c[field+1]};

            }

            if(Math.abs(end.x-m.x)<0.01){

                links.push({shape:shapeIndex,command:i,field:field,key:m.key,numerator:m.numerator,

                    denominator:m.denominator,scale:m.scale||1,offsetX:(end.x-(m.bindingX===undefined?m.x:m.bindingX))/(m.scale||1),markerOffsetX:m.bindingX===undefined?0:m.x-m.bindingX,

                    offsetY:(end.y-m.y)/(m.scale||1),editable:true});

            }

            previous=end;

        }

    });

    out.rigidArrowHeads=rigidArrowHeads(out);
    links=links.filter(function(link){return !out.rigidArrowHeads[link.command];});
    return {shape:out,bindings:links.concat(manual),commandMapping:commandMapping};

}

function linkedHandles(sh, bindings, index) {

    if(!sh||!sh.commands)return [];
    if(sh.singleAnchor){var anchor=sh.anchorCommand===undefined?5:sh.anchorCommand,c=sh.commands[anchor];return c?[{command:anchor,field:1,x:c[1],y:c[2],endpoint:true}]:[];}

    var handles=bindings.filter(function(b){return b.shape===index&&b.editable===true;}).map(function(b){

        var c=sh.commands[b.command];

        return c?{command:b.command,field:b.field,x:c[b.field],y:c[b.field+1]}:null;

    }).filter(function(h){return h!==null;});
    if(sh.kind==="fused")return handles.sort(function(a,b){return a.command-b.command;});
    sh.commands.forEach(function(c,i){
        if(c[0]==="Z")return;
        var next=sh.commands[i+1];
        if(c[0]==="M"||!next||next[0]==="M"||next[0]==="Z"){
            var existing=handles.filter(function(h){return h.command===i;})[0];
            if(existing)existing.endpoint=true;
            else handles.push({command:i,field:c.length-2,x:c[c.length-2],y:c[c.length-1],endpoint:true});
        }
    });
    var arrow=arrowGeometry(sh),heads=arrow?rigidArrowHeads(sh):{};
    if(arrow)arrow.indices.forEach(function(i){
        if(sh.commands[i][0]==="Z"||handles.some(function(h){return h.command===i;}))return;
        var c=sh.commands[i];handles.push({command:i,field:1,x:c[1],y:c[2],arrowHead:!!heads[i]});
    });
    return handles.sort(function(a,b){return a.command-b.command;});

}

// Smooth displacement falls to zero at the preceding and following linked vertices.

function deformationPlan(sh,h,handles){
    var commands=sh.commands,center=h.command;
    if(center<0||center>=commands.length)return null;
    // Never propagate displacement across separate paths or closing commands.
    var start=center,end=center;
    while(start>0&&commands[start][0]!=="M")start--;
    while(end+1<commands.length&&commands[end+1][0]!=="M"&&commands[end+1][0]!=="Z")end++;
    var left=start,right=end;
    handles.forEach(function(p){
        if(p.command<center&&p.command>=start)left=Math.max(left,p.command);
        if(p.command>center&&p.command<=end)right=Math.min(right,p.command);
    });
    var distance=[],total=0;
    distance[left]=0;
    for(var i=left+1;i<=right;i++){
        var c=commands[i],prev=commands[i-1];
        var origin={x:prev[prev.length-2],y:prev[prev.length-1]},last=origin,length=0;
        if(c[0]==="C"){
            for(var j=1;j<=16;j++){
                var p=curvePoint(origin,c,j/16);
                length+=Math.hypot(p.x-last.x,p.y-last.y);last=p;
            }
        }else length=Math.hypot(c[c.length-2]-origin.x,c[c.length-1]-origin.y);
        total+=Math.max(0.000001,length);distance[i]=total;
    }
    return {commands:commands,center:center,start:start,end:end,left:left,right:right,distance:distance};
}

function deform(sh, h, handles, dx, dy, curvature, prepared) {
    var arrow=arrowGeometry(sh);
    if(arrow){
        var vertices=arrow.indices;
        var end=sh.kind==="arrow-filled"?h.command===vertices[3]:h.command===vertices[1]||h.command===vertices[3];
        var start=sh.kind==="arrow-filled"?h.command===vertices[0]||h.command===vertices[6]:h.command===vertices[0]||(sh.kind==="arrow-double"&&h.command===vertices[6]);
        if(start||end){var point=end?arrow.b:arrow.a;return adjustArrow(sh,end?"end":"start",{x:point.x+dx,y:point.y+dy});}
    }
    var out=Object.assign({},sh),commands=sh.commands,center=h.command;
    out.commands=commands.slice();
    var plan=prepared&&prepared.commands===commands&&prepared.center===center?prepared:deformationPlan(sh,h,handles);
    if(!plan)return out;
    var start=plan.start,end=plan.end,left=plan.left,right=plan.right,distance=plan.distance;
    var degree=typeof curvature==="number"?Math.max(0,Math.min(1,curvature)):0.5;
    // This cubic displacement profile is represented exactly by its endpoint
    // values and derivatives. Control points must not be independently sampled.
    function profile(s, rising){
        var extent=rising?distance[center]-distance[left]:distance[right]-distance[center];
        if(extent<=0)return {value:1,slope:0};
        var u=rising?(s-distance[left])/extent:(distance[right]-s)/extent;
        u=Math.max(0,Math.min(1,u));
        return {value:(1-degree)*u+degree*u*u*(3-2*u),
                slope:((1-degree)+degree*6*u*(1-u))/extent*(rising?1:-1)};
    }
    function shift(c,field,w){c[field]+=dx*w;c[field+1]+=dy*w;}
    if(center===start){out.commands[start]=commands[start].slice();shift(out.commands[start],1,1);}
    for(var k=Math.max(start+1,left+1);k<=right;k++){
        var target=commands[k].slice(),rising=k<=center;out.commands[k]=target;
        var from=profile(distance[k-1],rising),to=profile(distance[k],rising);
        var span=distance[k]-distance[k-1];
        if(target[0]==="C"){
            shift(target,1,from.value+from.slope*span/3);
            shift(target,3,to.value-to.slope*span/3);
        }
        shift(target,target.length-2,to.value);
    }
    if(out.rawPoints)out.rawPoints=out.commands.filter(function(c){return c[0]!=="Z";}).map(function(c){
        return {x:c[c.length-2],y:c[c.length-1]};});
    return out;
}
// Sample only shapes touched by the eraser; untouched paths remain exact.
function eraseStroke(sh,x,y,radius,whole){
    var box=bounds([sh]),reach=radius+sh.style.width/2;
    if(x+reach<box.x||x-reach>box.x+box.width||y+reach<box.y||y-reach>box.y+box.height)return null;
    if(whole){
        var nearest=nearestSegment(sh,x,y,reach);
        return nearest&&nearest.distance<=reach?{shape:null,mapping:{}}:null;
    }
    var points=[],last=null,start=null,total=0;
    sh.commands.forEach(function(c,index){
        if(c[0]==="M"){
            last={x:c[1],y:c[2]};start=last;
            points.push({x:last.x,y:last.y,move:true,command:index});return;
        }
        if(!last)return;
        var end=c[0]==="Z"?start:{x:c[c.length-2],y:c[c.length-1]};
        var length=Math.hypot(end.x-last.x,end.y-last.y);
        if(c[0]==="C")length+=Math.hypot(c[1]-last.x,c[2]-last.y)+Math.hypot(c[3]-c[1],c[4]-c[2])+Math.hypot(end.x-c[3],end.y-c[4]);
        var steps=Math.max(1,Math.min(32,Math.ceil(length/Math.max(0.5,radius/4))));
        for(var j=1;j<=steps;j++){
            var p=c[0]==="C"?curvePoint(last,c,j/steps):blend(last,end,j/steps);
            points.push({x:p.x,y:p.y,command:j===steps?index:-1});
        }
        last=end;
    });
    var distance=radius+sh.style.width/2,touched=false;
    points.forEach(function(p){p.erased=Math.hypot(p.x-x,p.y-y)<=distance;if(p.erased)touched=true;});
    if(!touched)return null;
    if(whole)return {shape:null,mapping:{}};
    var out=copy(sh),commands=[],mapping={},moving=true;
    points.forEach(function(p){
        if(p.erased){moving=true;return;}
        if(p.move)moving=true;
        commands.push([moving?"M":"L",p.x,p.y]);moving=false;
        if(p.command>=0)mapping[p.command]={command:commands.length-1,field:1};
    });
    out.commands=commands;delete out.rawPoints;delete out.reflowSource;delete out.reflowGroup;out.kind="erased";
    return {shape:commands.some(function(c){return c[0]==="L";})?out:null,mapping:mapping};
}
function document(shapes) { shapes.forEach(ensureIdentity);return JSON.stringify({format:"musescore-drawing",version:1,shapes:shapes}); }

function readDocument(text) {

    var d=JSON.parse(text);

    if(d.format!=="musescore-drawing"||d.version!==1||!Array.isArray(d.shapes)||d.shapes.length>1000)

        throw Error("Formato de desenho inválido.");

    d.shapes.forEach(function(sh) {

        ensureIdentity(sh);
        if(sh.svgSource&&(typeof sh.svgSource!=="string"||!Array.isArray(sh.svgMatrix)||sh.svgMatrix.length!==6||!sh.svgMatrix.every(function(v){return typeof v==="number"&&isFinite(v)})))throw Error("Transformação SVG inválida.");

        if(!Array.isArray(sh.commands)||sh.commands.length>100000||!sh.style) throw Error("Traço inválido.");

        sh.commands.forEach(function(c) {

            var len={M:3,L:3,C:7,Z:1}[c[0]];

            if(c.length!==len) throw Error("Comando vetorial inválido.");

            for(var n=1;n<c.length;n++) if(typeof c[n]!=="number"||!isFinite(c[n])||Math.abs(c[n])>10000000)

                throw Error("Coordenada inválida.");

        });

        sh.style=checkedStyle(sh.style);
        if(sh.paintRanges){
            if(!Array.isArray(sh.paintRanges)||sh.paintRanges.length>10000)throw Error("Caminhos fundidos inválidos.");
            sh.paintRanges.forEach(function(range){
                if(!Number.isInteger(range.start)||!Number.isInteger(range.end)||range.start<0||range.end<=range.start||range.end>sh.commands.length||!range.style)throw Error("Caminhos fundidos inválidos.");
                range.style=checkedStyle(range.style);
            });
        }

    });

    return d.shapes;

}

function paint(ctx, shapes, selected) {

    shapes.forEach(function(sh) {
        if(sh.paintRanges){
            ctx.save();if(sh.clipRect){var r=sh.clipRect;ctx.beginPath();ctx.rect(r.x,r.y,r.width,r.height);ctx.clip();}
            var parts=paintedParts(sh);paint(ctx,parts,selected&&selected.indexOf(sh)>=0?parts:[]);ctx.restore();return;
        }
        if(sh.svgSource)return;
        var outline=nibOutline(sh.derivedWholeCommands||sh.commands,sh.style);
        if(outline){
            var fillShape=copy(sh);delete fillShape.style.nib;fillShape.style.stroke="none";delete fillShape.style.strokeGradient;
            var nibStyle=copy(sh.style);delete nibStyle.nib;nibStyle.fill=nibStyle.stroke;nibStyle.stroke="none";
            if(nibStyle.strokeGradient)nibStyle.fillGradient=nibStyle.strokeGradient;else delete nibStyle.fillGradient;delete nibStyle.strokeGradient;
            var strokeShape={commands:outline,style:nibStyle,paintFrame:sh.paintFrame||paintBounds(sh.derivedWholeCommands||sh.commands),clipRect:sh.clipRect};
            var parts=sh.style.fill!=="none"?[fillShape,strokeShape]:[strokeShape];
            paint(ctx,parts,selected&&selected.indexOf(sh)>=0?parts:[]);return;
        }
        var active=selected&&selected.indexOf(sh)>=0;
        ctx.save()
        if(sh.clipRect){var clip=sh.clipRect;ctx.beginPath();ctx.rect(clip.x,clip.y,clip.width,clip.height);ctx.clip()}
        ctx.beginPath();(sh.derivedWholeCommands||sh.commands).forEach(function(c) {

        if(c[0]==="M")ctx.moveTo(c[1],c[2]); else if(c[0]==="L")ctx.lineTo(c[1],c[2]);

        else if(c[0]==="C")ctx.bezierCurveTo(c[1],c[2],c[3],c[4],c[5],c[6]);else ctx.closePath();

    });ctx.globalAlpha=sh.style.opacity;ctx.lineWidth=sh.style.width;ctx.lineCap=sh.style.lineCap||"round";ctx.lineJoin="round";ctx.setLineDash(sh.style.linePattern==="dash"?[sh.style.width*4,sh.style.width*2]:sh.style.linePattern==="dot"?[sh.style.width,sh.style.width*2]:[]);

        ctx.fillRule=sh.fillRule==="evenodd"?Qt.OddEvenFill:Qt.WindingFill;
        function paintValue(channel){
            var g=sh.style[channel+"Gradient"];if(!g)return sh.style[channel];
            var p=gradientPoints(g,sh.paintFrame||paintBounds(sh.derivedWholeCommands||sh.commands));
            if(Math.hypot(p.x2-p.x1,p.y2-p.y1)<0.000001)p.x2+=0.000001;
            var gradient=ctx.createLinearGradient(p.x1,p.y1,p.x2,p.y2);g.stops.forEach(function(stop){gradient.addColorStop(stop.offset,gradientStopRgba(stop));});return gradient;
        }
        if(sh.style.fill!=="none"){ctx.fillStyle=active?"#628fea":paintValue("fill");ctx.fill();}

        if(sh.style.stroke!=="none"){ctx.strokeStyle=active?"#2675e6":paintValue("stroke");ctx.stroke();}ctx.globalAlpha=1;ctx.restore();});

}

if(typeof module!=="undefined") module.exports={smooth:smooth,pen:pen,shape:shape,bounds:bounds,svg:svg,insertPoint:insertPoint,curvePoint:curvePoint,

    style:style,readDocument:readDocument,document:document,translate:translate,hit:hit,handles:handles};

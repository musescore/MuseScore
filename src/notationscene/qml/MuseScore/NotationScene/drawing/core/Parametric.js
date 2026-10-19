// SPDX-License-Identifier: GPL-3.0-only
// Native drawing primitives. Independent implementation; no external editor code.
function clone(v){return JSON.parse(JSON.stringify(v));}
function clamp(v,a,b){return Math.max(a,Math.min(b,v));}
function sample(stops,t,key,fallback){
    if(!stops||!stops.length)return fallback;
    var a=stops[0],b=a;
    for(var i=1;i<stops.length;i++){b=stops[i];if(t<=b.t)break;a=b;}
    if(t<=a.t)return a[key];if(t>=b.t)return b[key];
    return a[key]+(b[key]-a[key])*(t-a.t)/Math.max(0.00001,b.t-a.t);
}
function waveCommands(w){
    var dx=w.b.x-w.a.x,dy=w.b.y-w.a.y,length=Math.hypot(dx,dy);
    if(length<0.001)return [["M",w.a.x,w.a.y]];
    var ux=dx/length,uy=dy/length,minPeriod=Math.max(2,Math.min.apply(null,w.stops.map(function(s){return s.period;})));
    var count=Math.max(16,Math.min(4096,Math.ceil(length/minPeriod*24))),phase=(w.phase||0)*Math.PI/180,result=[];
    for(var i=0;i<=count;i++){
        var t=i/count,amp=sample(w.stops,t,"amplitude",20),period=Math.max(2,sample(w.stops,t,"period",40));
        if(i)phase+=2*Math.PI*(length/count)/period;
        var value=w.waveform==="zigzag"?2/Math.PI*Math.asin(Math.sin(phase)):Math.sin(phase);
        var y=amp*value;
        result.push([i?"L":"M",w.a.x+dx*t-uy*y,w.a.y+dy*t+ux*y]);
    }
    result.push(["M",w.a.x,w.a.y],["M",w.b.x,w.b.y]);return result;
}
function create(kind,a,b,style,options){
    options=options||{};
    if(kind==="wave"){
        var w={a:clone(a),b:clone(b),waveform:options.waveform||"sine",phase:0,layoutMode:options.layoutMode||"continue",stops:[{t:0,amplitude:options.amplitude===undefined?20:options.amplitude,period:options.period||40},{t:0.5,amplitude:options.amplitude===undefined?20:options.amplitude,period:options.period||40},{t:1,amplitude:options.amplitude===undefined?20:options.amplitude,period:options.period||40}]};
        return {kind:kind,wave:w,commands:waveCommands(w),style:clone(style)};
    }
    var x=Math.min(a.x,b.x),y=Math.min(a.y,b.y),width=Math.abs(b.x-a.x),height=Math.abs(b.y-a.y),cx=x+width/2,cy=y+height/2,c=[],sides=clamp(Math.round(options.sides||5),3,24),points=[];
    if(kind==="rounded-rectangle"){
        var r=Math.min(width,height)*clamp(options.roundness===undefined?0.2:options.roundness,0,0.5),k=0.5522847498;
        c=[["M",x+r,y],["L",x+width-r,y],["C",x+width-r+k*r,y,x+width,y+r-k*r,x+width,y+r],["L",x+width,y+height-r],["C",x+width,y+height-r+k*r,x+width-r+k*r,y+height,x+width-r,y+height],["L",x+r,y+height],["C",x+r-k*r,y+height,x,y+height-r+k*r,x,y+height-r],["L",x,y+r],["C",x,y+r-k*r,x+r-k*r,y,x+r,y],["Z"]];
    } else {
        if(kind==="diamond")points=[[cx,y],[x+width,cy],[cx,y+height],[x,cy]];
        else if(kind==="cross")points=[[x+width/3,y],[x+2*width/3,y],[x+2*width/3,y+height/3],[x+width,y+height/3],[x+width,y+2*height/3],[x+2*width/3,y+2*height/3],[x+2*width/3,y+height],[x+width/3,y+height],[x+width/3,y+2*height/3],[x,y+2*height/3],[x,y+height/3],[x+width/3,y+height/3]];
        else for(var i=0,n=kind==="star"?sides*2:sides;i<n;i++){
            var angle=-Math.PI/2+2*Math.PI*i/n,ratio=kind==="star"&&i%2?clamp(options.innerRatio||0.4,0.05,0.95):1;
            points.push([cx+width/2*Math.cos(angle)*ratio,cy+height/2*Math.sin(angle)*ratio]);
        }
        c=points.map(function(p,i){return [i?"L":"M",p[0],p[1]];});c.push(["Z"]);
    }
    return {kind:kind,parametric:{frame:{a:clone(a),b:clone(b)},matrix:[1,0,0,1,0,0],sides:sides,innerRatio:options.innerRatio||0.4,roundness:options.roundness===undefined?0.2:options.roundness},commands:c,style:clone(style)};
}
function rebuild(shape,options){
    var box=shape.commands.reduce(function(b,c){for(var i=1;i<c.length;i+=2){b.x=Math.min(b.x,c[i]);b.y=Math.min(b.y,c[i+1]);b.right=Math.max(b.right,c[i]);b.bottom=Math.max(b.bottom,c[i+1]);}return b;},{x:Infinity,y:Infinity,right:-Infinity,bottom:-Infinity});
    var frame=shape.parametric.frame||{a:{x:box.x,y:box.y},b:{x:box.right,y:box.bottom}},m=shape.parametric.matrix||[1,0,0,1,0,0],made=create(shape.kind,frame.a,frame.b,shape.style,options);
    made.commands=made.commands.map(function(c){var out=c.slice();for(var i=1;i<c.length;i+=2){out[i]=m[0]*c[i]+m[2]*c[i+1]+m[4];out[i+1]=m[1]*c[i]+m[3]*c[i+1]+m[5];}return out;});
    made.parametric.matrix=clone(m);return Object.assign({},shape,{commands:made.commands,parametric:made.parametric});
}
function handles(shape,scale){
    if(!shape)return [];
    if(shape.parametric){
        var box=shape.commands.reduce(function(b,c){for(var i=1;i<c.length;i+=2){b.x=Math.min(b.x,c[i]);b.y=Math.min(b.y,c[i+1]);b.right=Math.max(b.right,c[i]);b.bottom=Math.max(b.bottom,c[i+1]);}return b;},{x:Infinity,y:Infinity,right:-Infinity,bottom:-Infinity});
        var role=shape.kind==="rounded-rectangle"?"corners":"sides",out=[];
        if(["polygon","star","rounded-rectangle"].indexOf(shape.kind)>=0)out.push({role:role,x:box.right+20/scale,y:box.y,color:"#a75aee"});
        if(shape.kind==="star")out.push({role:"inner-radius",x:box.right+20/scale,y:box.bottom,color:"#17bfa1"});return out;
    }
    if(!shape.wave)return [];
    var w=shape.wave,dx=w.b.x-w.a.x,dy=w.b.y-w.a.y,length=Math.hypot(dx,dy)||1,ux=dx/length,uy=dy/length,gap=18/Math.max(0.02,scale),result=[];
    w.stops.forEach(function(s,i){
        var x=w.a.x+dx*s.t,y=w.a.y+dy*s.t;
        result.push({role:"wave-amplitude",index:i,x:x-uy*s.amplitude,y:y+ux*s.amplitude,color:"#a75aee"});
        result.push({role:"wave-period",index:i,x:x+ux*s.period/2+uy*gap,y:y+uy*s.period/2-ux*gap,color:"#17bfa1"});
    });return result;
}
function adjust(shape,handle,dx,dy,scale){
    if(shape.parametric){var p=clone(shape.parametric),pixels=dx*(scale||1);if(handle.role==="corners")p.roundness=clamp(p.roundness+pixels/200,0,0.5);else if(handle.role==="inner-radius")p.innerRatio=clamp(p.innerRatio+pixels/200,0.05,0.95);else p.sides=clamp(p.sides+Math.round(pixels/12),3,24);return rebuild(shape,p);}
    var out=Object.assign({},shape,{wave:clone(shape.wave)}),w=out.wave,s=w.stops[handle.index],vx=w.b.x-w.a.x,vy=w.b.y-w.a.y,length=Math.hypot(vx,vy)||1,ux=vx/length,uy=vy/length;
    if(handle.role==="wave-period")s.period=Math.max(2,s.period+2*(dx*ux+dy*uy));
    else {s.amplitude=Math.max(0,s.amplitude-dx*uy+dy*ux);if(handle.index>0&&handle.index<w.stops.length-1)s.t=clamp(s.t+(dx*ux+dy*uy)/length,w.stops[handle.index-1].t+0.01,w.stops[handle.index+1].t-0.01);}
    out.commands=waveCommands(w);return out;
}
function transform(w,m){
    var out=clone(w),map=function(p){return {x:m[0]*p.x+m[2]*p.y+m[4],y:m[1]*p.x+m[3]*p.y+m[5]};},dx=w.b.x-w.a.x,dy=w.b.y-w.a.y,length=Math.hypot(dx,dy)||1;
    out.a=map(w.a);out.b=map(w.b);
    var along=Math.hypot(out.b.x-out.a.x,out.b.y-out.a.y)/length,normal=Math.hypot(m[0]*(-dy/length)+m[2]*(dx/length),m[1]*(-dy/length)+m[3]*(dx/length));
    out.stops.forEach(function(s){s.amplitude*=normal;s.period*=along;});return out;
}

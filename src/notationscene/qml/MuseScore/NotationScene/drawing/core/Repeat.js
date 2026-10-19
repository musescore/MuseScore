// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as V
// Arc-length placement. Moves never mutate the source SVG or its path.
var traceCache=[];
function trace(commands){
 var signature=commands.map(function(c){return c.join(",");}).join(";");
 for(var cacheIndex=0;cacheIndex<traceCache.length;cacheIndex++)if(traceCache[cacheIndex].commands===commands&&traceCache[cacheIndex].signature===signature)return traceCache[cacheIndex].trace;
 var points=[],last=null,start=null,total=0;
 function add(p,move){if(last&&!move){var length=Math.hypot(p.x-last.x,p.y-last.y);if(length>1e-8){total+=length;points.push({x:p.x,y:p.y,length:total,from:last});}}last=p;}
 commands.forEach(function(c){
  if(c[0]==="M"){last={x:c[1],y:c[2]};start=last;}
  else if(c[0]==="L"&&last)add({x:c[1],y:c[2]});
  else if(c[0]==="Z"&&start)add(start);
  else if(c[0]==="C"&&last){var a=last;
   function curve(p,b,d,e,depth){if(points.length>=50000)return;var chord=Math.hypot(e.x-p.x,e.y-p.y),excess=Math.hypot(b.x-p.x,b.y-p.y)+Math.hypot(d.x-b.x,d.y-b.y)+Math.hypot(e.x-d.x,e.y-d.y)-chord;
    if(depth>=12||excess<0.05){add(e);return;}
    var ab=V.blend(p,b,0.5),bd=V.blend(b,d,0.5),de=V.blend(d,e,0.5),u=V.blend(ab,bd,0.5),v=V.blend(bd,de,0.5),mid=V.blend(u,v,0.5);curve(p,ab,u,mid,depth+1);curve(mid,v,de,e,depth+1);
   }curve(a,{x:c[1],y:c[2]},{x:c[3],y:c[4]},{x:c[5],y:c[6]},0);
  }
 });var result={points:points,length:total};traceCache.push({commands:commands,signature:signature,trace:result});
 while(traceCache.length>8)traceCache.shift();return result;
}
function at(path,t){
 if(!path.points.length)return null;var distance=Math.max(0,Math.min(1,t))*path.length,p=path.points[path.points.length-1];
 var lo=0,hi=path.points.length-1;while(lo<hi){var mid=(lo+hi)>>1;if(path.points[mid].length<distance)lo=mid+1;else hi=mid;}p=path.points[lo];
 var size=Math.hypot(p.x-p.from.x,p.y-p.from.y),f=size?(distance-(p.length-size))/size:0;
 return {x:p.from.x+(p.x-p.from.x)*f,y:p.from.y+(p.y-p.from.y)*f,angle:Math.atan2(p.y-p.from.y,p.x-p.from.x)*180/Math.PI};
}
function placed(shape,p){
 var link=shape.repeatLink,angle=(link.rotation+(link.follow?p.angle:0))*Math.PI/180,c=Math.cos(angle),s=Math.sin(angle),w=link.width,h=link.height,x=p.x+link.dx,y=p.y+link.dy;
 if(Math.abs(link.poseX-x)<1e-7&&Math.abs(link.poseY-y)<1e-7&&Math.abs(link.poseAngle-angle)<1e-7)return shape;
 var out=Object.assign({},shape),m=[w*c,w*s,-h*s,h*c,x-w*c/2+h*s/2,y-w*s/2-h*c/2];
 out.svgMatrix=m;out.commands=[["M",m[4],m[5]],["L",m[0]+m[4],m[1]+m[5]],["L",m[0]+m[2]+m[4],m[1]+m[3]+m[5]],["L",m[2]+m[4],m[3]+m[5]],["Z"],["M",x,y]];
 out.repeatLink=Object.assign({},link,{poseX:x,poseY:y,poseAngle:angle});return out;
}
function rebuild(editor){
 var session=editor.repeatSession;if(!session||!session.pathId||!session.sourceId)return;
 var guide=editor.shapes.find(function(sh){return sh.drawId===session.pathId;}),source=editor.shapes.find(function(sh){return sh.drawId===session.sourceId;});if(!guide||!source)return;
 var path=trace(guide.commands);if(!path.length)return;
 var previous=editor.shapes.filter(function(sh){return sh.repeatLink&&sh.repeatLink.guideId===guide.drawId;}),removed={},group=session.groupId;
 previous.forEach(function(sh){removed[sh.drawId]=true;});
 var old=editor.shapes,all=old.filter(function(sh){return !removed[sh.drawId];}),box=V.bounds([source]),svg=V.svg([source],box),baseName=source.name||source.kind||"SVG";
 all=all.map(function(sh){return sh.drawId===guide.drawId?Object.assign({},sh,{selectionGroup:group,groupName:baseName,repeatSpec:{sourceId:source.drawId,count:editor.repeatCount}}):sh;});
 var count=Math.min(editor.repeatCount,Math.max(0,1000-all.length));
 for(var i=0;i<count;i++){
  var copy={kind:"svg",name:baseName+" "+(i+1),commands:[],svgSource:svg,style:V.style("#161616","none",0.001,1),singleAnchor:true,anchorCommand:5,selectionGroup:group,groupName:baseName,ownerPage:guide.ownerPage,
   repeatLink:{guideId:guide.drawId,u:count===1?0.5:i/(count-1),dx:0,dy:0,rotation:editor.repeatRotation,follow:editor.repeatFollow,width:box.width,height:box.height}};
  if(previous[i])copy.drawId=previous[i].drawId;V.ensureIdentity(copy);all.push(placed(copy,at(path,copy.repeatLink.u)));
 }
 var index={};all.forEach(function(sh,i){index[sh.drawId]=i;});
 editor.bindings=editor.bindings.filter(function(b){return old[b.shape]&&!removed[old[b.shape].drawId];}).map(function(b){return Object.assign({},b,{shape:index[old[b.shape].drawId]});});editor.shapes=all;editor.requestPaint();
}
function follow(editor){
 var paths={},byId={},dirty=false;editor.shapes.forEach(function(sh){byId[sh.drawId]=sh;});
 var all=editor.shapes.map(function(sh){var link=sh.repeatLink,guide=link&&byId[link.guideId];if(!guide||guide.repeatLink)return sh;
  if(!paths[guide.drawId])paths[guide.drawId]=trace(guide.commands);var p=at(paths[guide.drawId],link.u);if(!p)return sh;var out=placed(sh,p);if(out!==sh)dirty=true;return out;
 });if(dirty)editor.shapes=all;
}
function captureMoves(before,after,targets){
 var moved={};targets.forEach(function(i){if(before[i])moved[before[i].drawId]=true;});
 targets.forEach(function(i){var sh=after[i],link=sh&&sh.repeatLink;if(!link||moved[link.guideId])return;
  var guide=after.find(function(s){return s.drawId===link.guideId;}),point=guide?at(trace(guide.commands),link.u):null,c=sh.commands[sh.anchorCommand||5];if(!point||!c)return;
  after[i]=Object.assign({},sh,{repeatLink:Object.assign({},link,{dx:c[1]-point.x,dy:c[2]-point.y,poseX:c[1],poseY:c[2]})});
 });
}
function detach(shape){var out=Object.assign({},shape);delete out.repeatLink;delete out.repeatSpec;out.repeatDetached=true;return out;}

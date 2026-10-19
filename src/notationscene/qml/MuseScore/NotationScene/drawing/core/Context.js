// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as V
.import "Repeat.js" as Repeat
var clipboard=null, propertyClipboard=null;
function copy(editor,cut){
 var indices=editor.selectionIndices();if(!indices.length)return;
 clipboard={shapes:indices.map(function(i){return V.copy(editor.shapes[i]);}),bindings:editor.bindings.filter(function(b){return indices.indexOf(b.shape)>=0;}).map(function(b){var c=V.copy(b);c.shape=indices.indexOf(b.shape);return c;})};
 if(cut)editor.removeSelected();
}
function paste(editor,x,y){
 if(!clipboard)return;editor.remember();
 var offset=editor.shapes.length,box=V.bounds(clipboard.shapes),dx=x===undefined?12/editor.viewScale:x-box.x,dy=y===undefined?12/editor.viewScale:y-box.y,ids={},groups={};
 var pasteNames=editor.shapes.slice();var copied=clipboard.shapes.map(function(s){var c=V.translate(V.copy(s),dx,dy),old=c.drawId;delete c.drawId;delete c.nativeIndex;c.name=nextName(pasteNames,c.name||c.kind||"SVG");pasteNames.push(c);if(c.repeatLink||c.repeatSpec)c=Repeat.detach(c);delete c.reflowSource;delete c.reflowGroup;delete c.fragmentOverrides;V.ensureIdentity(c);ids[old]=c.drawId;
 if(c.selectionGroup){if(!groups[c.selectionGroup])groups[c.selectionGroup]=c.drawId+"-group";c.selectionGroup=groups[c.selectionGroup];}c.ownerPage=editor.anchorPageAt(x===undefined?box.x+dx:x,y===undefined?box.y+dy:y);return c;});
 copied.forEach(function(c){if(c.textLink&&ids[c.textLink.drawId])c.textLink.drawId=ids[c.textLink.drawId];(c.nodeLinks||[]).forEach(function(l){if(ids[l.drawId])l.drawId=ids[l.drawId];});});
 editor.shapes=editor.shapes.concat(copied);
 copied.forEach(function(s,i){editor.autoLink(offset+i);});
 editor.setSelection(copied.map(function(s,i){return offset+i;}),offset,true);editor.changed();
}
function effective(shape,command){
 var style=V.copy(shape.style);(shape.segmentStyles||[]).slice().sort(function(a,b){return a.command-b.command;}).forEach(function(m){if(m.command<=command)style=m.replace?V.copy(m.style):Object.assign(style,m.style);});return style;
}
function ranges(shape){
 var marks=(shape.segmentStyles||[]).slice().sort(function(a,b){return a.command-b.command;}),out=[],start=0,style=V.copy(shape.style);
 marks.forEach(function(m){var boundary=m.command===0?0:m.command+1;if(boundary>start)out.push({start:start,end:boundary,style:V.copy(style)});style=m.replace?V.copy(m.style):Object.assign(style,m.style);start=boundary;});
 if(start<shape.commands.length)out.push({start:start,end:shape.commands.length,style:V.copy(style)});
 out.forEach(function(r){r.style.fill="none";delete r.style.fillGradient;});
 var fills=[],subpath=0,closed=false;
 shape.commands.forEach(function(c,i){
  if(c[0]==="M")subpath=i;
  if(c[0]!=="Z")return;closed=true;var fill=effective(shape,subpath);
  if(fill.fill==="none")return;fill.stroke="none";delete fill.strokeGradient;
  fills.push({start:subpath,end:i+1,style:fill,fillRule:shape.fillRule});
 });
 if(!closed&&shape.style.fill!=="none"){var fill=V.copy(shape.style);fill.stroke="none";delete fill.strokeGradient;fills.push({start:0,end:shape.commands.length,style:fill});}
 shape.paintRanges=fills.concat(out);
}
function withStyle(shape,command,style){
 var sh=V.copy(shape);
 var following=(shape.segmentStyles||[]).filter(function(m){return m.command>command;}).sort(function(a,b){return a.command-b.command;})[0];
 var after=following?effective(shape,following.command):null;
 sh.segmentStyles=(sh.segmentStyles||[]).filter(function(m){return m.command!==command&&(!following||m.command!==following.command);});
 sh.segmentStyles.push({command:command,style:V.copy(style),replace:true});
 if(following)sh.segmentStyles.push({command:following.command,style:after,replace:true});
 ranges(sh);return sh;
}
function changedStyle(shape,start,end,patch,replaceStyle,removeKeys){
 function edited(old){var st=replaceStyle?V.copy(patch):Object.assign({},old,patch);(removeKeys||[]).forEach(function(key){delete st[key];});return st;}
 if(!(end>start))return withStyle(shape,start,edited(effective(shape,start)));
 var sh=V.copy(shape),after=effective(shape,end),marks=(shape.segmentStyles||[]);
 sh.segmentStyles=marks.filter(function(m){return m.command<start||m.command>=end;}).map(function(m){return V.copy(m);});
 sh.segmentStyles.push({command:start,style:edited(effective(shape,start)),replace:true});
 marks.forEach(function(m){if(m.command>start&&m.command<end)sh.segmentStyles.push({command:m.command,style:edited(effective(shape,m.command)),replace:true});});
 sh.segmentStyles=sh.segmentStyles.filter(function(m){return m.command!==end;});sh.segmentStyles.push({command:end,style:after,replace:true});
 ranges(sh);return sh;
}
function withoutStyle(shape,command,end){
 var sh=V.copy(shape),after=end>command?effective(shape,end):null;
 sh.segmentStyles=(sh.segmentStyles||[]).filter(function(m){return end>command?m.command<command||m.command>=end:m.command!==command;});
 if(after){sh.segmentStyles=sh.segmentStyles.filter(function(m){return m.command!==end;});sh.segmentStyles.push({command:end,style:after,replace:true});}
 if(sh.segmentStyles.length)ranges(sh);else{delete sh.segmentStyles;delete sh.paintRanges;}
 return sh;
}
function applyStyle(editor,hit,style){
 if(!hit||!editor.shapes[hit.shape])return;editor.remember();var all=editor.shapes.slice();
 all[hit.shape]=withStyle(all[hit.shape],hit.handle.command,style);editor.shapes=all;editor.changed();
}
function copyNodeProperties(editor){
 if(!editor.styleEditActive)return;editor.nodeStyleClipboard=V.copy(editor.selectedNodeStyle);
}
function pasteNodeProperties(editor){
 if(!editor.styleEditActive||!editor.nodeStyleClipboard)return;
 editor.applyNodeStyle(editor.nodeStyleClipboard,true);
}
function removeNodeProperties(editor){
 if(!editor.styleEditActive)return;editor.remember();var all=editor.shapes.slice();
 all[editor.styleEditIndex]=withoutStyle(all[editor.styleEditIndex],editor.styleNodeCommand,editor.styleRangeEnd);
 editor.shapes=all;editor.loadNodeStyle();editor.changed();
}
function copyProperties(editor,hit){propertyClipboard=hit?effective(editor.shapes[hit.shape],hit.handle.command):V.copy(editor.shapes[editor.selected].style);}
function pasteProperties(editor,hit){
 if(!propertyClipboard)return;if(hit){applyStyle(editor,hit,propertyClipboard);return;}
 editor.remember();var indices=editor.selectionIndices(),all=editor.shapes.slice();indices.forEach(function(i){var s=V.copy(all[i]);s.style=V.copy(propertyClipboard);if(s.segmentStyles)ranges(s);all[i]=s;});editor.shapes=all;editor.changed();
}
function reorder(editor,front){
 var chosen=editor.selectionIndices(),order=editor.shapes.map(function(s,i){return i;}),rest=order.filter(function(i){return chosen.indexOf(i)<0;}),next=front?rest.concat(chosen):chosen.concat(rest),map={};
 editor.remember();next.forEach(function(old,i){map[old]=i;});editor.shapes=next.map(function(i){return editor.shapes[i];});editor.bindings=editor.bindings.map(function(b){return Object.assign({},b,{shape:map[b.shape]});});editor.setSelection(chosen.map(function(i){return map[i];}),map[editor.selected],true);editor.changed();
}
function rows(editor){
 var region=editor.paintRegion,page=editor.anchorPageAt(region.x+region.width/2,region.y+region.height/2),groups={},out=[];
 for(var i=editor.shapes.length-1;i>=0;i--){var sh=editor.shapes[i];if(sh.fragmentId||sh.parentDrawId||sh.derived)continue;
 var first=sh.commands&&sh.commands[0],owner=sh.ownerPage===undefined?(first?editor.anchorPageAt(first[1],first[2]):null):sh.ownerPage;
 if(page!==null&&owner!==null&&owner!==page)continue;
 var row={index:i,title:sh.name||sh.kind||"SVG",indices:[i],children:[],group:false};
 if(sh.selectionGroup){if(!groups[sh.selectionGroup]){groups[sh.selectionGroup]={title:sh.groupName||qsTr("Group"),id:sh.selectionGroup,indices:[],children:[],group:true};out.push(groups[sh.selectionGroup]);}var g=groups[sh.selectionGroup];g.indices.push(i);g.children.push(row);}else out.push(row);
 }return out;
}
function anchorHit(editor,x,y){
 var best=null,reach=10/Math.max(0.02,editor.viewScale);
 editor.rhythmMarkers.forEach(function(m){var yy=m.railY===undefined?m.y:m.railY,d=Math.hypot(m.x-x,yy-y);if(d<reach){best=m;reach=d;}});return best;
}
function targets(editor,hit,anchor){
 if(hit)return [{shape:hit.shape,handle:hit.handle}];if(!anchor)return [];
 return editor.bindings.filter(function(b){return b.key===anchor.key&&b.numerator*anchor.denominator===anchor.numerator*b.denominator;}).map(function(b){return {shape:b.shape,handle:{command:b.command,field:b.field}};});
}
function unlink(editor,targets){
 editor.remember();
 editor.shapes=editor.shapes.map(function(sh,index){var own=targets.filter(function(t){return t.shape===index;});if(!own.length||!sh.nodeLinks)return sh;var copy=V.copy(sh);copy.nodeLinks=copy.nodeLinks.filter(function(l){return !own.some(function(t){return t.handle.command===l.ownCommand&&t.handle.field===l.ownField;});});return copy;});targets.forEach(function(t){var i=editor.bindings.findIndex(function(b){return b.shape===t.shape&&b.command===t.handle.command&&b.field===t.handle.field;});if(i>=0){var b=editor.bindings[i],all=editor.shapes.slice(),s=V.copy(all[b.shape]);s.unlinkedPoints=s.unlinkedPoints||{};s.unlinkedPoints[b.command+":"+b.field]=true;all[b.shape]=s;editor.shapes=all;editor.bindings=editor.bindings.filter(function(v,n){return n!==i;});}});editor.changed();
}
function reaches(shapes,fromId,toId,seen){
 if(fromId===toId)return true;seen=seen||{};if(seen[fromId])return false;seen[fromId]=true;var sh=shapes.find(function(s){return s.drawId===fromId;});return !!sh&&(sh.nodeLinks||[]).some(function(l){return reaches(shapes,l.drawId,toId,seen);});
}
function remapLinks(editor,drawId,remap){
 editor.shapes=editor.shapes.map(function(sh){if(!(sh.nodeLinks||[]).length)return sh;var s=V.copy(sh);s.nodeLinks=s.nodeLinks.map(function(l){var n=Object.assign({},l);if(sh.drawId===drawId)n.ownCommand=remap(n.ownCommand);if(n.drawId===drawId)n.command=remap(n.command);return n;}).filter(function(l){return l.command>=0&&l.ownCommand>=0;});return s;});
}
// Count distinct linked vertices, never the number of destinations for one vertex.
function linkedNodes(shape,bindings,index){
 var nodes={};(bindings||[]).forEach(function(b){if(b.shape===index)nodes[b.command+":"+b.field]=true;});
 (shape.nodeLinks||[]).forEach(function(l){nodes[l.ownCommand+":"+l.ownField]=true;});
 return Object.keys(nodes).map(function(key){var a=key.split(":");return {command:Number(a[0]),field:Number(a[1])};});
}
function followPoints(editor){
 var all=editor.shapes.slice(),byId={};all.forEach(function(sh,i){byId[sh.drawId]=i;});var visiting={},done={},changed=false;
 function visit(i){if(done[i]||visiting[i])return;visiting[i]=true;var original=all[i],sh=original,links=sh.nodeLinks||[],handles=linkedNodes(sh,editor.bindings,i);
 links.forEach(function(l){var target=byId[l.drawId];if(target===undefined||visiting[target])return;visit(target);
 var c=all[target].commands[l.command],own=sh.commands[l.ownCommand];if(!c||!own)return;
 var dx=c[l.field]+l.dx-own[l.ownField],dy=c[l.field+1]+l.dy-own[l.ownField+1];if(Math.hypot(dx,dy)<1e-7)return;
 var before=sh;
 if(handles.length===1)sh=V.translate(sh,dx,dy);
 else if(sh.svgSource){
  // Keep the asset frame consistent with its edited nodes.
  var frame=sh.commands,first=frame[0],right=frame[1],bottom=frame[3];
  if(first&&right&&bottom){
   var ux=right[1]-first[1],uy=right[2]-first[2],vx=bottom[1]-first[1],vy=bottom[2]-first[2],det=ux*vy-uy*vx;
   if(Math.abs(det)>1e-9){
    var constraints=handles.filter(function(h){return h.command!==l.ownCommand||h.field!==l.ownField;});
    var fixed=constraints.length?frame[constraints[0].command]:first,fx=fixed[fixed.length-2],fy=fixed[fixed.length-1],ox=own[l.ownField]-fx,oy=own[l.ownField+1]-fy,n=ox*ox+oy*oy;
    if(n>1e-9)sh=V.transform(sh,[1+dx*ox/n,dy*ox/n,dx*oy/n,1+dy*oy/n,-(dx*ox*fx+dx*oy*fy)/n,-(dy*ox*fx+dy*oy*fy)/n]);
   }
  }
 }else sh=V.deform(sh,{command:l.ownCommand,field:l.ownField},handles,dx,dy,0.5);
 editor.bindings=editor.bindings.map(function(b){if(b.shape!==i||!(b.scale>0))return b;
 var a=before.commands[b.command],c=sh.commands[b.command];return a&&c?Object.assign({},b,{offsetX:b.offsetX+(c[b.field]-a[b.field])/b.scale,offsetY:b.offsetY+(c[b.field+1]-a[b.field+1])/b.scale}):b;});changed=true;
 });all[i]=sh;delete visiting[i];done[i]=true;}
 all.forEach(function(sh,i){visit(i);});if(changed)editor.shapes=all;
}
function curvature(editor,hit,smooth){
 if(!hit)return;editor.remember();var all=editor.shapes.slice(),s=V.copy(all[hit.shape]),i=hit.handle.command,c=s.commands[i],next=s.commands[i+1],prev=s.commands[i-1];
 if(c&&c[0]==="C"){
  if(smooth&&next&&next[0]==="C"){var x=c[5],y=c[6],vx=next[1]-x,vy=next[2]-y,n=Math.hypot(vx,vy)||1,r=Math.hypot(c[3]-x,c[4]-y);c[3]=x-vx/n*r;c[4]=y-vy/n*r;}
  else if(!smooth){c[3]=c[5];c[4]=c[6];}
 }else if(smooth&&c&&c[0]==="L"&&prev){var px=prev[prev.length-2],py=prev[prev.length-1];s.commands[i]=["C",px+(c[1]-px)/3,py+(c[2]-py)/3,px+2*(c[1]-px)/3,py+2*(c[2]-py)/3,c[1],c[2]];}
 if(!smooth&&next&&next[0]==="C"){next[1]=c[c.length-2];next[2]=c[c.length-1];}
 delete s.wave;delete s.reflowSource;delete s.reflowGroup;all[hit.shape]=s;editor.shapes=all;editor.autoLink(hit.shape);editor.changed();
}

function nextName(shapes,name){
 var base=String(name||"SVG").replace(/ \d+$/,""),n=1;
 shapes.forEach(function(sh){var text=sh.name||"";if(text.indexOf(base+" ")===0){var value=Number(text.slice(base.length+1));if(Number.isInteger(value)&&value>=n)n=value+1;}});return base+" "+n;
}
function moveObjects(editor,ids,row,mode){
 if(editor.styleEditActive||editor.repeatEditActive||!ids.length)return;
 var old=editor.shapes,chosen=old.filter(function(sh){return ids.indexOf(sh.drawId)>=0;}),targetIds=row.indices.map(function(i){return old[i].drawId;});
 if(chosen.length===0||targetIds.some(function(id){return ids.indexOf(id)>=0;}))return;
 editor.gestureStarting();editor.remember();
 var target=old[row.indices[0]],group=mode==="inside"?(row.group?row.id:target.selectionGroup||V.ensureIdentity({})+"-group"):(row.parentGroup||null);
 var sourceGroup=chosen[0].selectionGroup,wholeGroup=!!sourceGroup&&chosen.every(function(sh){return sh.selectionGroup===sourceGroup;})&&old.filter(function(sh){return sh.selectionGroup===sourceGroup;}).length===chosen.length;
 if(wholeGroup&&mode!=="inside"&&!row.parentGroup)group=sourceGroup;
 var allGuideDetached=null;
 var all=old.map(function(sh){var moving=ids.indexOf(sh.drawId)>=0,changeTarget=mode==="inside"&&!row.group&&targetIds.indexOf(sh.drawId)>=0;
  if(!moving&&!changeTarget)return sh;var copy=Object.assign({},sh);
  if(moving&&!wholeGroup&&copy.selectionGroup!==group){
   if(copy.repeatLink)copy=Repeat.detach(copy);
   if(copy.repeatSpec){var guide=copy.drawId;copy=Repeat.detach(copy);allGuideDetached=guide;}
  }
  if(group){copy.selectionGroup=group;copy.groupName=wholeGroup&&group===sourceGroup?sh.groupName:row.group?row.title:target.groupName||qsTr("Group");}else{delete copy.selectionGroup;delete copy.groupName;}return copy;
 });
 // A removed guide must not keep controlling members of a different group.
 chosen.forEach(function(sh){if(!wholeGroup&&sh.repeatSpec&&sh.selectionGroup!==group)allGuideDetached=sh.drawId;});
 if(allGuideDetached)all=all.map(function(sh){return sh.repeatLink&&sh.repeatLink.guideId===allGuideDetached?Repeat.detach(sh):sh;});
 var moving=all.filter(function(sh){return ids.indexOf(sh.drawId)>=0;}),rest=all.filter(function(sh){return ids.indexOf(sh.drawId)<0;});
 var positions=targetIds.map(function(id){return rest.findIndex(function(sh){return sh.drawId===id;});}).filter(function(i){return i>=0;}),at=mode==="after"?Math.min.apply(null,positions):Math.max.apply(null,positions)+1;
 rest.splice.apply(rest,[at,0].concat(moving));var index={};rest.forEach(function(sh,i){index[sh.drawId]=i;});
 editor.bindings=editor.bindings.map(function(b){return Object.assign({},b,{shape:index[old[b.shape].drawId]});});
 editor.shapes=rest;
 var oldById={};old.forEach(function(sh){oldById[sh.drawId]=sh;});
 rest.forEach(function(sh,i){if(oldById[sh.drawId].repeatLink&&!sh.repeatLink)editor.autoLink(i);});
 editor.setSelection(ids.map(function(id){return index[id];}),index[ids[0]],true);editor.changed();editor.gestureFinished();
}

// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as Vector

function rows(pages){
    var groups={}
    pages.forEach(function(page,index){(page.anchors||[]).forEach(function(m){
        if(typeof m.tick!=="number")return
        var key=index+":"+m.system+":"+m.staff
        if(!groups[key])groups[key]={page:index,system:m.system,staff:m.staff,y:m.y,markers:[]}
        groups[key].markers.push(m)
    })})
    return Object.keys(groups).map(function(key){
        var row=groups[key]
        row.markers.sort(function(a,b){return a.tick-b.tick||a.x-b.x})
        row.start=row.markers[0].tick;row.end=row.markers[row.markers.length-1].tick
        row.left=row.markers[0].measureLeft===undefined?row.markers[0].x:row.markers[0].measureLeft
        return row
    })
}
function interpolate(list,value,from,to){
    var a=list[0],b=list[list.length-1]
    for(var i=1;i<list.length;i++)if(list[i][from]>value){a=list[i-1];b=list[i];break}
    if(value<=list[0][from]){a=list[0];b=list[Math.min(1,list.length-1)]}
    if(value>=list[list.length-1][from]){b=list[list.length-1];a=list[Math.max(0,list.length-2)]}
    var span=b[from]-a[from]
    return span?a[to]+(b[to]-a[to])*(value-a[from])/span:a[to]
}
function barlinePosition(row,tick){
    var marker=row.markers.filter(function(m){return m.barline&&Math.abs(m.tick-tick)<0.00001})[0]
    return marker?marker.x:position(row,tick)
}
function position(row,tick){
    if(Math.abs(tick-row.end)<0.00001)return row.markers[row.markers.length-1].x
    return interpolate(row.markers,tick,"tick","x")
}
function extend(shape,pageWidth,endAmount,startAmount){
    if(shape.derivedWholeCommands){
        var derived=Vector.copy(shape)
        var left=derived.breakStart?derived.systemLeft*(1-startAmount):0
        var right=derived.breakEnd?derived.systemRight+(pageWidth-derived.systemRight)*endAmount:pageWidth
        derived.clipRect={x:left,y:0,width:Math.max(0,right-left),height:derived.pageHeight||1000000}
        return derived
    }
    var out=Vector.copy(shape),commands=out.commands
    if(out.extensionStart){commands.shift();if(commands.length){var start=commands[0];commands[0]=["M",start[start.length-2],start[start.length-1]]};delete out.extensionStart}
    if(out.extensionEnd){commands.pop();delete out.extensionEnd}
    if(!commands.length)return out
    if(out.breakStart&&startAmount>0){
        var first=commands[0],next=commands[1],x=first[1],y=first[2],left=x*(1-startAmount)
        var tx=next?(next[1]-x):1,ty=next?(next[2]-y):0
        var slope=Math.abs(tx)>0.00001?Math.max(-4,Math.min(4,ty/tx)):0,span=x-left
        commands[0]=["C",left+span/3,y-slope*span*2/3,left+span*2/3,y-slope*span/3,x,y]
        commands.unshift(["M",left,y-slope*span]);out.extensionStart=true
    }
    if(out.breakEnd&&endAmount>0){
        var last=commands[commands.length-1],x=last[last.length-2],y=last[last.length-1]
        var before=commands.length>1?commands[commands.length-2]:last
        var tx=last[0]==="C"?x-last[3]:x-before[before.length-2]
        var ty=last[0]==="C"?y-last[4]:y-before[before.length-1]
        var slope=Math.abs(tx)>0.00001?Math.max(-4,Math.min(4,ty/tx)):0,span=Math.max(0,pageWidth-x)*endAmount
        commands.push(["C",x+span/3,y+slope*span/3,x+span*2/3,y+slope*span*2/3,x+span,y+slope*span]);out.extensionEnd=true
    }
    return out
}
function documents(saved,oldPages,newPages){
    var oldRows=rows(oldPages),newRows=rows(newPages),result={},processed={},matrices={}
    Object.keys(saved).forEach(function(key){var d=JSON.parse(saved[key]);Object.keys(d.matrices||{}).forEach(function(id){matrices[id]=d.matrices[id]})})
    newPages.forEach(function(page,i){
        var settings=saved[String(i)]?JSON.parse(saved[String(i)]):null
        if(!settings&&Object.keys(saved).length)settings=JSON.parse(saved[Object.keys(saved)[0]])
        result[String(i)]=settings?Vector.copy(settings):{format:"musescore-drawing",version:1}
        result[String(i)].shapes=[];result[String(i)].anchors=[];result[String(i)].matrices={}
    })
    function append(page,shape,manualPoints){
        var document=result[String(page)],index=document.shapes.length
        var linked=Vector.autoAnchors(shape,newPages[page].anchors||[],index)
        ;(manualPoints||[]).forEach(function(point){
            var closest=-1,distance=0.01
            linked.shape.commands.forEach(function(c,i){
                if(c[0]==="Z")return
                var d=Math.hypot(c[c.length-2]-point.x,c[c.length-1]-point.y)
                if(d<distance){distance=d;closest=i}
            })
            if(closest>=0&&!linked.bindings.some(function(b){return b.command===closest}))
                linked.bindings.push({shape:index,command:closest,field:linked.shape.commands[closest].length-2,editable:true,manual:true})
        })
        var base=linked.shape
        linked.shape=extend(base,newPages[page].width,document.systemContinuation||0,document.systemStartContinuation||0)
        if(linked.shape.extensionStart)linked.bindings.forEach(function(b){b.command++;b.field=linked.shape.commands[b.command].length-2})
        document.shapes.push(linked.shape)
        document.anchors=document.anchors.concat(linked.bindings)
        if(shape.matrixId&&shape.reflowSource)document.matrices[shape.matrixId]=Vector.copy(shape.reflowSource)
    }
    Object.keys(saved).forEach(function(pageKey){
        var document=JSON.parse(saved[pageKey]),pageIndex=Number(pageKey)
        document.shapes.forEach(function(original,index){
            Vector.ensureIdentity(original)
            var group=original.matrixId||original.reflowGroup||("reflow-"+Date.now()+"-"+pageKey+"-"+index)
            if(processed[group])return
            processed[group]=true
            var shape=extend(original,oldPages[pageIndex].width,0,0)
            var links=(document.anchors||[]).filter(function(b){return b.shape===index}).map(function(b){
                var link=Vector.copy(b);if(original.extensionStart)link.command--;return link
            })
            var box=Vector.bounds([shape]),middle=box.y+box.height/2,source=null,distance=Infinity
            oldRows.forEach(function(row){
                if(row.page!==pageIndex)return
                var score=Math.abs(row.y-middle)
                var matched=links.some(function(b){return row.markers.some(function(m){return m.key===b.key})})
                if(links.length&&!matched)score+=100000000
                if(score<distance){distance=score;source=row}
            })
            var matrix=matrices[group]||original.reflowSource
            if(matrix){
                shape=Vector.copy(matrix.shape)
                if(!shape.drawId)shape.drawId=original.drawId
                source=Vector.copy(matrix.row)
                links=Vector.copy(matrix.links)
                links.forEach(function(link){link.shape=index})
            }
            var baseline=source?{shape:Vector.copy(shape),row:Vector.copy(source),links:Vector.copy(links)}:null
            if(baseline){delete baseline.shape.reflowSource;delete baseline.shape.reflowGroup;delete baseline.shape.matrixId;delete baseline.shape.derivedWholeCommands;delete baseline.shape.clipRect}
            if(!source||!links.length){if(result[pageKey])append(pageIndex,shape,[]);return}
            var destinations=newRows.filter(function(row){return row.staff===source.staff&&row.end>source.start&&row.start<source.end})
            function destination(tick){
                // Artwork beyond the original system barline is decoration,
                // not a rhythmic position in the following original system.
                tick=Math.max(source.start,Math.min(source.end-0.00001,tick))
                var row=destinations.filter(function(r){return tick>=r.start-0.00001&&tick<r.end-0.00001})[0]
                if(!row)row=destinations.filter(function(r){return tick>=r.start-0.00001&&tick<=r.end+0.00001})[0]
                return row
            }
            function tickAt(x){
                var exact=source.markers.filter(function(m){return m.barline&&Math.abs(m.x-x)<0.00001})[0]
                return exact?exact.tick:interpolate(source.markers,x,"x","tick")
            }
            function mapped(x,y,row){
                var tick=tickAt(x)
                var onBarline=source.markers.some(function(m){return m.barline&&Math.abs(m.x-x)<0.00001})
                var px=onBarline&&Math.abs(tick-row.start)<0.00001?row.left:position(row,tick)
                return {x:px,y:row.y+(y-source.y)}
            }
            var first=shape.commands[0]
            if(!first)return
            if(shape.anchorMode==="note"){
                var row=destination(tickAt(first[1]))
                if(!row)return
                var point=mapped(first[1],first[2],row),dx=point.x-first[1],dy=point.y-first[2]
                shape.commands.forEach(function(c){for(var k=1;k<c.length;k+=2){c[k]+=dx;c[k+1]+=dy}})
                if(shape.svgMatrix){shape.svgMatrix[4]+=dx;shape.svgMatrix[5]+=dy}
                shape.reflowSource=baseline;shape.reflowGroup=group
                append(row.page,shape,[]);return
            }
            // Split exact Bezier geometry wherever a new system starts.
            var working=Vector.copy(shape)
            destinations.forEach(function(row){
                if(row.start<=source.start||row.start>=source.end)return
                var cut=barlinePosition(source,row.start),previous=null
                for(var i=0;i<working.commands.length;i++){
                    var c=working.commands[i]
                    if(c[0]==="Z"){previous=null;continue}
                    var end={x:c[c.length-2],y:c[c.length-1]}
                    if(previous&&(c[0]==="C"||c[0]==="L")&&(previous.x-cut)*(end.x-cut)<-0.000001){
                        var split=(cut-previous.x)/(end.x-previous.x)
                        if(c[0]==="C"){
                            var lo=0,hi=1
                            for(var j=0;j<40;j++){
                                var t=(lo+hi)/2,p=Vector.curvePoint(previous,c,t)
                                if((p.x<cut)===(previous.x<cut))lo=t;else hi=t
                            }
                            split=(lo+hi)/2
                        }
                        Vector.insertPoint(working,i,split,true)
                        c=working.commands[i]
                        // Keep the boundary exact so the new trailing segment
                        // is not subdivided repeatedly at the same system cut.
                        c[c.length-2]=cut
                        end={x:cut,y:c[c.length-1]}
                    }
                    previous=end
                }
            })
            var current=null,currentRow=null,previous=null,manual=[],issuedRows={}
            var visible=Vector.linkedHandles(shape,links,index)
            function wholeForRow(row){
                var unchanged=source.markers.length===row.markers.length&&source.markers.every(function(marker,i){
                    var target=row.markers[i]
                    return marker.tick===target.tick&&Math.abs(marker.x-target.x)<0.000001&&Math.abs(source.y-row.y)<0.000001
                })
                if(unchanged)return Vector.copy(shape.commands)
                var last=null
                return shape.commands.map(function(c){
                    var out=c.slice()
                    if(c[0]==="Z")return out
                    var end={x:c[c.length-2],y:c[c.length-1]}
                    if(c[0]==="M"){var p=mapped(end.x,end.y,row);out=["M",p.x,p.y]}
                    else if(last){
                        var a=mapped(last.x,last.y,row),b=mapped(end.x,end.y,row)
                        var ratio=Math.abs(end.x-last.x)>0.000001?(b.x-a.x)/(end.x-last.x):1
                        for(var k=1;k<c.length;k+=2){out[k]=a.x+(c[k]-last.x)*ratio;out[k+1]=c[k+1]+row.y-source.y}
                    }
                    last=end;return out
                })
            }
            function flush(){
                if(current&&current.commands.length>1&&!issuedRows[currentRow.page+":"+currentRow.system]){
                    issuedRows[currentRow.page+":"+currentRow.system]=true
                    current.matrixId=group
                    current.fragmentId="rhythm-"+currentRow.staff+"-"+currentRow.start+"-"+currentRow.end
                    if(shape.svgMatrix){
                        var m=shape.svgMatrix,p0=mapped(m[4],m[5],currentRow),px=mapped(m[4]+m[0],m[5]+m[1],currentRow),py=mapped(m[4]+m[2],m[5]+m[3],currentRow)
                        current.svgMatrix=[px.x-p0.x,px.y-p0.y,py.x-p0.x,py.y-p0.y,p0.x,p0.y]
                    }
                    current.derivedWholeCommands=wholeForRow(currentRow)
                    current.systemLeft=currentRow.left;current.systemRight=position(currentRow,currentRow.end)
                    current.pageHeight=newPages[currentRow.page].height
                    delete current.extensionStart;delete current.extensionEnd
                    append(currentRow.page,current,manual)
                }
                current=null;manual=[]
            }
            working.commands.forEach(function(c){
                if(c[0]==="Z"){if(current)current.commands.push(["Z"]);return}
                var end={x:c[c.length-2],y:c[c.length-1]}
                if(c[0]==="M"){flush();previous=end;return}
                if(!previous)return
                var tick=tickAt((previous.x+end.x)/2),row=destination(tick)
                if(!row){previous=end;return}
                if(!current||row!==currentRow){
                    var split=current!==null
                    if(current)current.breakEnd=true
                    flush();current=Vector.copy(shape);current.commands=[];delete current.rawPoints
                    delete current.extensionStart;delete current.extensionEnd
                    current.breakStart=split||(!!shape.breakStart&&Math.abs(row.start-tickAt(previous.x))<0.0001);current.breakEnd=false
                    current.reflowSource=baseline;current.reflowGroup=group
                    currentRow=row
                    var start=mapped(previous.x,previous.y,row);current.commands.push(["M",start.x,start.y])
                }
                var transformed=c.slice()
                // Map controls with the interval's affine transform. This keeps
                // curves continuous and follows measure compression/expansion.
                var startPoint=mapped(previous.x,previous.y,row),endPoint=mapped(end.x,end.y,row)
                var span=end.x-previous.x,ratio=span?(endPoint.x-startPoint.x)/span:1
                for(var k=1;k<c.length;k+=2){transformed[k]=startPoint.x+(c[k]-previous.x)*ratio;transformed[k+1]=c[k+1]+row.y-source.y}
                current.commands.push(transformed)
                if(visible.some(function(h){return Math.hypot(h.x-end.x,h.y-end.y)<0.01}))manual.push(endPoint)
                previous=end
            })
            if(current)current.breakEnd=!!shape.breakEnd&&Math.abs(currentRow.end-tickAt(previous.x))<0.0001
            flush()
        })
    })
    Object.keys(result).forEach(function(key){result[key]=JSON.stringify(result[key])})
    return result
}

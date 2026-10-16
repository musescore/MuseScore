// SPDX-License-Identifier: GPL-3.0-only
.import "Vector.js" as Vector
function create(text,x,y,limit){
    var opening=/<svg\b([^>]*)>/i.exec(text),end=text.lastIndexOf('</svg>');
    if(!opening||end<opening.index)throw Error("O arquivo não contém um SVG válido.");
    var attrs=opening[1],view=/\bviewBox\s*=\s*["']([^"']+)["']/i.exec(attrs);
    var box=view?view[1].trim().split(/[\s,]+/).map(Number):null;
    function size(name,fallback){var match=new RegExp('\\b'+name+'\\s*=\\s*["\']([^"\']+)["\']','i').exec(attrs);return match?parseFloat(match[1]):fallback}
    if(!box)box=[0,0,size('width',300),size('height',150)];
    if(box.length!==4||!box.every(isFinite)||box[2]<=0||box[3]<=0)throw Error("Dimensões do SVG inválidas.");
    var width=Math.min(box[2],limit),height=width*box[3]/box[2];
    if(height>limit){height=limit;width=height*box[2]/box[3]}
    var shape={kind:'svg',anchorMode:'note',style:Vector.style('#161616','none',0.001,1),commands:[['M',x,y],['L',x+width,y],['L',x+width,y+height],['L',x,y+height],['Z']],svgMatrix:[width,0,0,height,x,y]};
    var id=Vector.ensureIdentity(shape),body=text.slice(opening.index+opening[0].length,end);
    // Prefix definition IDs so two imported SVGs do not share gradients/clips.
    var identifiers={};
    body=body.replace(/\bid\s*=\s*(["'])([^"']+)\1/g,function(all,quote,value){identifiers[value]=id+'-'+value;return 'id='+quote+identifiers[value]+quote});
    body=body.replace(/url\(\s*#([^\s)]+)\s*\)/g,function(all,value){return identifiers[value]?'url(#'+identifiers[value]+')':all});
    body=body.replace(/((?:xlink:)?href\s*=\s*["'])#([^"']+)/g,function(all,start,value){return start+'#'+(identifiers[value]||value)});
    attrs=attrs.replace(/(?:^|\s)(?:width|height|x|y|viewBox|preserveAspectRatio)\s*=\s*(["'])[^"']*\1/gi,' ');
    if(!/\bxmlns\s*=/.test(attrs))attrs+=' xmlns="http://www.w3.org/2000/svg"';
    shape.svgSource='<svg '+attrs+' width="1" height="1" viewBox="'+box.join(' ')+'" preserveAspectRatio="none">'+body+'</svg>';
    return shape;
}
function tinted(source){
    var result=source.replace(/((?:fill|stroke|stop-color)\s*=\s*["'])([^"']+)(["'])/gi,function(all,start,color,end){return /^(none|transparent)$/i.test(color)?all:start+'#2675e6'+end})
        .replace(/((?:fill|stroke|stop-color)\s*:\s*)([^;"}<]+)/gi,function(all,start,color){return /^(none|transparent)\s*$/i.test(color)?all:start+'#2675e6'});
    return result.replace(/<svg\b([^>]*)>/i,function(all,attrs){return /(?:^|\s)fill\s*=/.test(attrs)?all:'<svg fill="#2675e6" '+attrs+'>'});
}

// Apply only explicit paint operations. Retain the original SVG so changing a
// gradient does not accumulate definitions or destroy its paths/transforms.
function painted(source,style,channels,identity){
    var opening=/<svg\b([^>]*)>/i.exec(source);if(!opening)return source;
    var view=/\bviewBox\s*=\s*["']([^"']+)["']/i.exec(opening[1]);
    var dimensions=view?view[1].trim().split(/[\s,]+/).map(Number):[0,0,1,1];
    var frame={x:dimensions[0],y:dimensions[1],width:dimensions[2],height:dimensions[3]},defs='',values={};
    ["stroke","fill"].forEach(function(channel){
        if(!channels[channel])return;
        var gradient=style[channel+"Gradient"],id=(identity||"drawing").replace(/[^a-zA-Z0-9_-]/g,'-')+'-paint-'+channel;
        if(gradient){defs+=Vector.gradientSvg(gradient,id,frame);values[channel]='url(#'+id+')';}
        else values[channel]=Vector.color(style[channel],'#161616');
    });
    var protectedBlocks=[];
    source=source.replace(/<(defs|clipPath|mask)\b[\s\S]*?<\/\1>/gi,function(block){var index=protectedBlocks.length;protectedBlocks.push(block);return '<!--musedraw-protected-'+index+'-->';});
    var result=source.replace(/<(path|rect|circle|ellipse|polygon|polyline|line|text|tspan|use)\b([^>]*?)(\/?)>/gi,function(all,tag,attributes,end){
        var css=/\bstyle\s*=\s*(["'])(.*?)\1/i.exec(attributes),rules=css?css[2]:'';
        if(css)attributes=attributes.replace(css[0],'');
        Object.keys(values).forEach(function(channel){
            attributes=attributes.replace(new RegExp('\\b'+channel+'\\s*=\\s*(["\'])(.*?)\\1','gi'),'');
            rules=rules.replace(new RegExp('(^|;)\\s*'+channel+'\\s*:[^;]*','gi'),'$1');
            rules+=';'+channel+':'+values[channel]+' !important';
            attributes+=' '+channel+'="'+values[channel]+'"';
        });
        return '<'+tag+attributes+' style="'+rules.replace(/"/g,'&quot;')+'"'+end+'>';
    });
    result=result.replace(/<!--musedraw-protected-(\d+)-->/g,function(all,index){return protectedBlocks[Number(index)];});
    return result.replace(/<svg\b[^>]*>/i,function(tag){return tag+(defs?'<defs>'+defs+'</defs>':'');});
}

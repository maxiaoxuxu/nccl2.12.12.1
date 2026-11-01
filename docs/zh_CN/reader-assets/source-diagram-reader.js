'use strict';
const M=JSON.parse(document.getElementById('source-model').textContent);
const IMAGES=JSON.parse(document.getElementById('source-images').textContent);
const $=id=>document.getElementById(id);
const B=new Map(M.bindings.map(b=>[b.id,b]));
const SYMBOLS=new Map(M.symbols.map(s=>[s.id,s]));
const diagrams=Object.values(M.diagrams);
let currentFile='',currentBinding=null,currentLine=1,rows=[],ignoreScroll=0;
const fileBindings=file=>M.bindings.filter(b=>b.file===file);
function element(tag,text,cls){const n=document.createElement(tag);if(text!==undefined)n.textContent=text;if(cls)n.className=cls;return n}
function paramsHash(params){return '#'+new URLSearchParams(params).toString()}
function navigate(params){const hash=paramsHash(params);if(location.hash===hash)route();else location.hash=hash}
function lineBinding(file,line){
 const choices=fileBindings(file).filter(b=>b.start<=line&&line<=b.end);
 choices.sort((a,b)=>(a.scope==='code'?0:1)-(b.scope==='code'?0:1)||(a.end-a.start)-(b.end-b.start)||a.id.localeCompare(b.id));
 return choices[0]||null;
}
function sourceText(code){
 const frag=document.createDocumentFragment();let previous=0;
 for(const m of code.matchAll(/\bB\d{4}\b/g)){
  if(!B.has(m[0]))continue;
  frag.append(document.createTextNode(code.slice(previous,m.index)));
  const button=element('button',m[0],'binding-token');button.dataset.binding=m[0];button.title='查看这段源码对应的图和分析';frag.append(button);previous=m.index+m[0].length;
 }
 frag.append(document.createTextNode(code.slice(previous)));return frag;
}
function loadFile(file){
 if(file===currentFile)return;
 if(!M.files[file])throw Error('Unknown source file: '+file);
 currentFile=file;currentBinding=null;rows=[];$('fileSelect').value=file;
 const fragment=document.createDocumentFragment(),hasBinding=new Set(fileBindings(file).map(b=>b.start));
 let comment=false;
 M.files[file].text.split('\n').forEach((text,i)=>{
  const row=element('div',undefined,'source-line');row.id='L'+(i+1);row.dataset.line=i+1;
  if(comment||/^\s*(\/\*|\*|\/\/)/.test(text))row.classList.add('comment');
  if(text.includes('/*'))comment=true;if(text.includes('*/'))comment=false;
  if(hasBinding.has(i+1))row.classList.add('has-binding');
  const number=element('button',String(i+1),'ln');number.title='定位这一行的相关图示';number.setAttribute('aria-label','源码第'+(i+1)+'行');
  const code=element('code');code.append(sourceText(text));row.append(number,code);fragment.append(row);rows.push(row);
 });
 $('sourceScroll').replaceChildren(fragment);$('sourceFile').textContent=file;
 $('sourceInfo').textContent=M.files[file].lines+'行 · 原始执行代码和全部中文注释';
 $('bindingSelect').replaceChildren();
 const empty=element('option','选择本文件的图示绑定');empty.value='';$('bindingSelect').append(empty);
 for(const b of fileBindings(file).sort((a,b)=>a.start-b.start||a.id.localeCompare(b.id))){const o=element('option',b.id+' · '+b.symbol+' · '+b.title);o.value=b.id;$('bindingSelect').append(o)}
}
function drawDiagram(id){
 if(!M.diagrams[id])return;
 $('diagramImage').src=IMAGES[id];$('diagramImage').alt=M.diagrams[id].title;
 $('diagramCaption').textContent=id+' · '+M.diagrams[id].title;$('diagramSelect').value=id;
 $('diagramLink').href='01_通信原语与算法图解_离线版.html#diagram-'+id;
 $('diagramLink').textContent='在图解页打开（完整ZIP）';
 $('modalTitle').textContent=M.diagrams[id].title;$('modalImage').src=IMAGES[id];$('modalImage').alt=M.diagrams[id].title;
}
function showBinding(binding,preferred){
 const same=currentBinding?.id===binding?.id;
 currentBinding=binding;
 $('noBinding').hidden=Boolean(binding);$('diagramDetails').hidden=!binding;
 $('bindingSelect').value=binding?.id||'';
 for(const row of rows){const line=Number(row.dataset.line);row.classList.toggle('scope',Boolean(binding&&binding.start<=line&&line<=binding.end));}
 if(!binding){$('bindingSummary').textContent='本行暂无人工指定图示；可搜索函数或使用图示反查。';return}
 $('bindingSummary').textContent=binding.id+' · '+binding.symbol+' · '+binding.relation;
 $('bindingTitle').textContent=binding.title;$('bindingNote').textContent=binding.explanation;
 $('bindingMeta').textContent=binding.file+' · 第'+binding.start+'—'+binding.end+'行 · '+binding.name;
 $('diagramSelect').replaceChildren();
 for(const id of binding.diagrams){const o=element('option',id+' · '+M.diagrams[id].title);o.value=id;$('diagramSelect').append(o)}
 const before=same?$('diagramImage').dataset.diagram:'';
 const selected=binding.diagrams.includes(preferred)?preferred:(binding.diagrams.includes(before)?before:binding.diagrams[0]);
 drawDiagram(selected);$('diagramImage').dataset.diagram=selected;reverse(selected);
 const list=fileBindings(currentFile).sort((a,b)=>a.start-b.start||a.id.localeCompare(b.id)),idx=list.findIndex(b=>b.id===binding.id);
 $('previousBinding').disabled=idx<=0;$('nextBinding').disabled=idx<0||idx>=list.length-1;
}
function chooseLine(line,binding,scroll=true,preferred){
 line=Math.max(1,Math.min(Number(line)||1,rows.length));
 rows[currentLine-1]?.classList.remove('current');currentLine=line;rows[line-1].classList.add('current');
 const next=binding||lineBinding(currentFile,line);
 if(next?.id!==currentBinding?.id||preferred||!next)showBinding(next,preferred);
 $('lineInput').value=line;$('status').textContent='当前第'+line+'行';
 const params=next?{symbol:next.symbol,binding:next.id,line:String(line)}:{file:currentFile,line:String(line)};
 $('permalink').href=paramsHash(params);
 if(scroll){ignoreScroll++;rows[line-1].scrollIntoView({block:'center'});requestAnimationFrame(()=>requestAnimationFrame(()=>{ignoreScroll=Math.max(0,ignoreScroll-1)}));}
}
function reverse(id){
 if(!M.diagrams[id])return;
 $('reverseSelect').value=id;$('reverseResults').replaceChildren();
 const matches=M.bindings.filter(b=>b.diagrams.includes(id));
 const exact=matches.filter(b=>b.relation==='直接对应'&&b.scope==='code');
 const sorted=[...exact,...matches.filter(b=>!exact.includes(b))];
 for(const b of sorted){const button=element('button',b.id+' · '+b.symbol+' · '+b.name+' · '+b.title,'reverse-link');button.dataset.reverseBinding=b.id;button.dataset.diagram=id;$('reverseResults').append(button)}
 $('reverseCount').textContent=matches.length+'处绑定（直接代码段优先）';
}
function route(){
 const p=new URLSearchParams(location.hash.slice(1));
 if(p.has('diagram')){
  const id=p.get('diagram');reverse(id);
  const choices=M.bindings.filter(b=>b.diagrams.includes(id)).sort((a,b)=>(a.scope==='code'?0:1)-(b.scope==='code'?0:1));
  const b=choices[0];if(b){loadFile(b.file);chooseLine(b.start,b,true,id)}return;
 }
 const b=B.get((p.get('binding')||'').toUpperCase()),s=SYMBOLS.get((p.get('symbol')||'').toUpperCase());
 const file=b?.file||s?.file||p.get('file')||'src/collectives/device/all_reduce.h';
 loadFile(M.files[file]?file:'src/collectives/device/all_reduce.h');
 const line=Number(p.get('line'))||b?.start||s?.line||M.scopes.F0058.start;
 chooseLine(line,b,true);if(b)reverse(b.diagrams[0]);
}
function search(){
 const query=$('symbolSearch').value.trim().toLowerCase();$('searchResults').replaceChildren();
 if(!query){$('searchResults').style.display='none';return}
 if(/^b\d+$/.test(query)){
  const bindings=M.bindings.filter(b=>b.id.toLowerCase().startsWith(query));
  $('searchResults').append(element('div',bindings.length+'处图示绑定','muted'));
  for(const b of bindings.slice(0,30)){const button=element('button',b.id+' · '+b.symbol+' · '+b.title);button.dataset.bindingChoice=b.id;$('searchResults').append(button)}
  $('searchResults').style.display='block';return;
 }
 const matches=M.symbols.filter(s=>(s.id+' '+s.name+' '+s.file).toLowerCase().includes(query));
 $('searchResults').append(element('div',matches.length+'个匹配；显示前30个','muted'));
 for(const s of matches.slice(0,30)){const button=element('button',s.id+' · '+s.name+' · '+s.file+':'+s.line);button.dataset.symbol=s.id;$('searchResults').append(button)}
 $('searchResults').style.display='block';
}
for(const file of Object.keys(M.files).sort()){const o=element('option',file);o.value=file;$('fileSelect').append(o)}
for(const d of diagrams){const o=element('option',d.id+' · '+d.title);o.value=d.id;$('reverseSelect').append(o)}
$('sourceScroll').addEventListener('click',event=>{
 const token=event.target.closest('[data-binding]');if(token){const b=B.get(token.dataset.binding);navigate({symbol:b.symbol,binding:b.id});return}
 const row=event.target.closest('.source-line');if(row)chooseLine(Number(row.dataset.line),null,false);
});
let scrollFrame=0;
$('sourceScroll').addEventListener('scroll',()=>{
 if(ignoreScroll||!$('followSource').checked)return;cancelAnimationFrame(scrollFrame);
 scrollFrame=requestAnimationFrame(()=>{
  const y=$('sourceScroll').getBoundingClientRect().top+Math.min(95,$('sourceScroll').clientHeight/4);
  let lo=0,hi=rows.length-1;while(lo<hi){const mid=(lo+hi+1)>>1;if(rows[mid].getBoundingClientRect().top<=y)lo=mid;else hi=mid-1}
  chooseLine(lo+1,null,false);
 });
},{passive:true});
$('fileSelect').onchange=()=>navigate({file:$('fileSelect').value,line:'1'});
$('bindingSelect').onchange=()=>{const b=B.get($('bindingSelect').value);if(b)navigate({symbol:b.symbol,binding:b.id})};
$('symbolSearch').oninput=search;
$('symbolSearch').onkeydown=event=>{if(event.key==='Enter')$('searchResults').querySelector('button')?.click();if(event.key==='Escape')$('searchResults').style.display='none'};
$('searchResults').onclick=event=>{const button=event.target.closest('button');if(!button)return;$('searchResults').style.display='none';if(button.dataset.bindingChoice){const b=B.get(button.dataset.bindingChoice);navigate({symbol:b.symbol,binding:b.id})}else if(button.dataset.symbol)navigate({symbol:button.dataset.symbol})};
document.addEventListener('click',event=>{if(!event.target.closest('#searchBox'))$('searchResults').style.display='none'});
$('lineGo').onclick=()=>chooseLine($('lineInput').value,null,true);
$('lineInput').onkeydown=e=>{if(e.key==='Enter')$('lineGo').click()};
$('diagramSelect').onchange=()=>{const id=$('diagramSelect').value;drawDiagram(id);$('diagramImage').dataset.diagram=id;reverse(id)};
$('reverseSelect').onchange=()=>reverse($('reverseSelect').value);
$('reverseResults').onclick=event=>{const button=event.target.closest('[data-reverse-binding]');if(button){const b=B.get(button.dataset.reverseBinding);loadFile(b.file);chooseLine(b.start,b,true,button.dataset.diagram);history.replaceState(null,'',paramsHash({symbol:b.symbol,binding:b.id}));}};
for(const [id,delta] of [['previousBinding',-1],['nextBinding',1]])$(id).onclick=()=>{const list=fileBindings(currentFile).sort((a,b)=>a.start-b.start||a.id.localeCompare(b.id)),i=list.findIndex(b=>b.id===currentBinding?.id),b=list[i+delta];if(b)navigate({symbol:b.symbol,binding:b.id})};
$('quickAllReduce').onclick=()=>navigate({symbol:'F0058'});$('quickGeneric').onclick=()=>navigate({symbol:'F0232'});
$('zoomDiagram').onclick=()=>{$('imageModal').classList.add('open');$('closeModal').focus()};
$('closeModal').onclick=()=>{$('imageModal').classList.remove('open');$('zoomDiagram').focus()};
document.addEventListener('keydown',e=>{if(e.key==='Escape'&&$('imageModal').classList.contains('open'))$('closeModal').click()});
window.addEventListener('hashchange',route);route();
window.NCCL_READER={lineBinding,chooseLine,navigate,model:M};

/* Test real offline navigation, all reviewed bindings, all diagrams and source
 * links. Usage: node tools/check_source_diagram_browser.cjs [screenshots-dir]
 */
const {chromium}=require('playwright');
const fs=require('fs'),os=require('os'),path=require('path');
const assert=require('assert/strict');
const {pathToFileURL}=require('url');

(async()=>{
 const root=path.resolve(__dirname,'..'),docs=path.join(root,'docs/zh_CN');
 const reader='11_源码注释与图示对照_离线版.html',guide='01_通信原语与算法图解_离线版.html',search='05_全部函数内部步骤_离线搜索.html';
 const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'nccl-source-diagram-'));
 const evidence=process.argv[2];if(evidence)fs.mkdirSync(evidence,{recursive:true});
 const browser=await chromium.launch({headless:true,...(process.env.NCCL_DOCS_CHROMIUM?{executablePath:process.env.NCCL_DOCS_CHROMIUM}:{})});
 const context=await browser.newContext({offline:true,viewport:{width:1500,height:1000}});
 const page=await context.newPage(),errors=[],network=[];
 page.on('pageerror',e=>errors.push(e.message));page.on('request',r=>{if(/^https?:/.test(r.url()))network.push(r.url())});
 const local=name=>pathToFileURL(path.join(tmp,name)).href;
 try{
  fs.copyFileSync(path.join(docs,reader),path.join(tmp,reader));
  // Only one HTML exists here: original source and SVG files are absent.
  await page.goto(local(reader));await page.waitForFunction(()=>window.NCCL_READER);
  // Beginner help is embedded: no separate file is needed to explain IDs.
  await page.click('#openUsage');
  assert(await page.locator('#helpModal').evaluate(el=>el.classList.contains('open')));
  assert.match(await page.locator('#helpModal').textContent(),/不需要记任何编号/);
  assert.match(await page.locator('#helpModal').textContent(),/X0\(C2\)/);
  await page.keyboard.press('Escape');
  assert(!(await page.locator('#helpModal').evaluate(el=>el.classList.contains('open'))));
  assert.equal(await page.evaluate(()=>document.activeElement.id),'openUsage');
  await page.click('#openNumberHelp');
  await page.waitForFunction(()=>{const h=document.getElementById('number-reference').getBoundingClientRect();return h.top>=0&&h.top<innerHeight});
  assert.match(await page.locator('#number-reference').textContent(),/编号是什么意思/);
  await page.keyboard.press('Escape');
  assert.equal(await page.evaluate(()=>document.activeElement.id),'openNumberHelp');
  await page.click('#openUsage');await page.click('#usageStartSimple');
  await page.waitForFunction(()=>document.getElementById('sourceFile').textContent.endsWith('prims_simple.h'));
  assert(!(await page.locator('#helpModal').evaluate(el=>el.classList.contains('open'))));
  assert(!/^[BFT]\d/.test(await page.locator('#bindingSelect option:checked').textContent()));
  assert.match(await page.locator('#bindingMeta').textContent(),/函数定位号 F0232/);
  const model=await page.evaluate(()=>({bindings:NCCL_READER.model.bindings,diagrams:Object.keys(NCCL_READER.model.diagrams)}));
  for(const b of model.bindings){
   await page.evaluate(b=>NCCL_READER.navigate({symbol:b.symbol,binding:b.id}),b);
   await page.waitForFunction(id=>document.getElementById('bindingSelect').value===id,b.id);
   assert.equal(await page.locator('#sourceFile').textContent(),b.file);
   assert.equal(await page.locator('#bindingNote').textContent(),b.explanation);
   await page.locator('#diagramImage').evaluate(img=>img.decode());
   assert(await page.locator('#diagramImage').evaluate(img=>img.naturalWidth>0));
  }
  for(const id of model.diagrams){
   await page.selectOption('#reverseSelect',id);
   assert(await page.locator('#reverseResults button').count()>0,id);
   await page.locator('#reverseResults button').first().click();
   assert.equal(await page.locator('#diagramSelect').inputValue(),id);
   await page.locator('#diagramImage').evaluate(img=>img.decode());
  }
  // Search by symbol and by the diagram-binding ID written into source.
  await page.fill('#symbolSearch','F0232');await page.press('#symbolSearch','Enter');
  await page.waitForFunction(()=>document.getElementById('sourceFile').textContent.endsWith('prims_simple.h'));
  const worker=model.bindings.find(b=>b.symbol==='F0232'&&b.title.includes('空slice和非worker'));
  await page.fill('#symbolSearch',worker.id);await page.press('#symbolSearch','Enter');
  await page.waitForFunction(id=>document.getElementById('bindingSelect').value===id,worker.id);
  // A real scroll chooses the corresponding phase without changing file.
  await page.evaluate(()=>new Promise(r=>requestAnimationFrame(()=>requestAnimationFrame(r))));
  const wait=model.bindings.find(b=>b.symbol==='F0230');
  await page.locator('#L'+wait.code_start).evaluate(el=>el.scrollIntoView({block:'start'}));
  await page.waitForFunction(id=>document.getElementById('bindingSelect').value===id,wait.id);
  // Click a B-number in the actual embedded source comment.
  await page.uncheck('#followSource');
  await page.locator('.binding-token[data-binding="'+worker.id+'"]').first().click();
  await page.waitForFunction(id=>document.getElementById('bindingSelect').value===id,worker.id);
  await page.check('#followSource');
  // Variables inherit only the relevant enclosing reviewed code/structure.
  const variable=await page.evaluate(()=>{
   const scope=NCCL_READER.model.scopes.F0232;
   return NCCL_READER.model.symbols.find(s=>s.kind==='变量'&&s.name==='sliceSize'&&s.file===scope.file&&s.line>=scope.start&&s.line<=scope.end);
  });assert(variable);
  await page.evaluate(id=>NCCL_READER.navigate({symbol:id}),variable.id);
  await page.waitForFunction(()=>document.getElementById('sourceFile').textContent.endsWith('prims_simple.h'));
  // An unrelated SDK function must not be forced onto an NCCL algorithm graph.
  const unbound=await page.evaluate(()=>NCCL_READER.model.symbols.find(s=>s.kind==='函数'&&s.file.includes('/nvtx3/')));
  await page.evaluate(id=>NCCL_READER.navigate({symbol:id}),unbound.id);
  await page.waitForFunction(()=>!document.getElementById('noBinding').hidden);
  const final=model.bindings.find(b=>b.symbol==='F0058'&&b.scope==='code'&&b.diagrams.includes('allreduce-step-3'));
  await page.evaluate(b=>NCCL_READER.navigate({symbol:b.symbol,binding:b.id}),final);
  await page.waitForFunction(id=>document.getElementById('bindingSelect').value===id,final.id);
  await page.click('#zoomDiagram');assert(await page.locator('#imageModal').evaluate(el=>el.classList.contains('open')));
  await page.keyboard.press('Escape');assert(!(await page.locator('#imageModal').evaluate(el=>el.classList.contains('open'))));
  if(evidence)await page.screenshot({path:path.join(evidence,'AllReduce_源码与图示.png')});
  await page.setViewportSize({width:390,height:844});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));
  await page.click('#openUsage');assert(await page.locator('#helpModal').evaluate(el=>el.scrollWidth<=el.clientWidth+1));
  await page.keyboard.press('Escape');
  await page.setViewportSize({width:1500,height:1000});
  // Now copy the other standalone pages, then exercise cross-file links.
  fs.copyFileSync(path.join(docs,guide),path.join(tmp,guide));fs.copyFileSync(path.join(docs,search),path.join(tmp,search));
  await page.goto(local(guide));
  for(let i=0;i<3;i++)await page.click('#next');
  const link=await page.locator('#frameSource').getAttribute('href');assert(link.includes(final.id));
  await page.click('#frameSource');await page.waitForFunction(id=>document.getElementById('bindingSelect')?.value===id,final.id);
  await page.click('#diagramLink');await page.waitForFunction(()=>document.getElementById('all-diagrams')?.open);
  assert(page.url().endsWith('#diagram-allreduce-step-3'));
  await page.goto(local(search));await page.fill('#q','F0232');await page.locator('#f0232 > summary').click();
  const panel=page.locator('#f0232 section.panel');await panel.locator('img').evaluate(img=>img.decode());
  await panel.locator('select').first().selectOption(worker.id);
  assert((await panel.locator('p').textContent()).includes(worker.explanation));
  await panel.getByRole('link',{name:'打开带注释的源码对照（完整ZIP）'}).click();
  await page.waitForFunction(id=>document.getElementById('bindingSelect')?.value===id,worker.id);
  if(evidence)await page.screenshot({path:path.join(evidence,'genericOp_源码与图示.png')});
  const usage='12_使用方法与编号说明_离线版.html';
  const guideOnly=path.join(tmp,'guide-only');fs.mkdirSync(guideOnly);fs.copyFileSync(path.join(docs,usage),path.join(guideOnly,usage));
  await page.goto(pathToFileURL(path.join(guideOnly,usage)).href);
  assert.match(await page.locator('main').textContent(),/F0232/);
  assert.match(await page.locator('main').textContent(),/不是NCCL的GPU编号/);
  await page.locator('nav a[href="#number-reference"]').click();
  assert(page.url().endsWith('#number-reference'));
  await page.setViewportSize({width:390,height:844});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));
  assert.deepEqual(errors,[]);assert.deepEqual(network,[]);
  console.log(JSON.stringify({status:'PASS',offlineSingleReader:true,bindings:model.bindings.length,reverseDiagrams:model.diagrams.length,usageHelp:true,numberReference:true,humanReadableLabels:true,standaloneUsageGuide:true,sourceCommentClicks:true,variableNavigation:true,scrollFollows:true,crossPageLinks:true,unboundSymbolsStayUnbound:true,mobileWidth:390,pageErrors:0,httpRequests:0}));
 }finally{await browser.close();fs.rmSync(tmp,{recursive:true,force:true})}
})().catch(e=>{console.error(e);process.exitCode=1});

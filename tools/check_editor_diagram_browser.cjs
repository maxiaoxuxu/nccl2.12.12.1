/* Validate actual launcher URLs and the OS-handler relay without network.
 * Requires Playwright/Chromium; no Source Insight or VS Code host is assumed.
 */
const {chromium}=require('playwright');
const {execFileSync}=require('child_process');
const fs=require('fs'),os=require('os'),path=require('path');
const assert=require('assert/strict');
const {pathToFileURL}=require('url');

(async()=>{
 const root=path.resolve(__dirname,'..'),python=process.env.NCCL_DOCS_PYTHON||'python3';
 const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'NCCL 中文 路径 #50% & '));
 const browser=await chromium.launch({headless:true,...(process.env.NCCL_DOCS_CHROMIUM?{executablePath:process.env.NCCL_DOCS_CHROMIUM}:{})});
 const context=await browser.newContext({offline:true,viewport:{width:1500,height:1000}});
 const page=await context.newPage(),errors=[],network=[];
 page.on('pageerror',e=>errors.push(e.message));page.on('request',r=>{if(/^https?:/.test(r.url()))network.push(r.url())});
 try{
  const reader='docs/zh_CN/11_源码注释与图示对照_离线版.html';
  // Copy only the adapter, index, embedded reader, and one saved source. The
  // path deliberately contains Chinese, spaces, #, %, and an ampersand.
  for(const file of ['tools/open_source_diagram.py','docs/zh_CN/source-diagram-index.json',reader,'src/collectives/device/all_reduce.h']){
   fs.mkdirSync(path.dirname(path.join(tmp,file)),{recursive:true});fs.copyFileSync(path.join(root,file),path.join(tmp,file));
  }
  const locate=(base,file,line)=>execFileSync(python,[path.join(base,'tools/open_source_diagram.py'),'--file',path.join(base,file),'--line',String(line),'--print-url'],{encoding:'utf8'}).trim();
  const target=locate(tmp,'src/collectives/device/all_reduce.h',306);
  assert.equal(new URL(target).searchParams.size,0);
  const relay=execFileSync(python,['-c','import sys; from pathlib import Path; sys.path.insert(0,sys.argv[1]); import open_source_diagram as j; print(j.write_jump_page(sys.argv[2],sys.argv[3]))',path.join(tmp,'tools'),target,tmp],{encoding:'utf8'}).trim();
  await page.goto(pathToFileURL(relay).href);await page.waitForFunction(()=>window.NCCL_READER);
  assert.equal(page.url(),target);assert.equal(await page.locator('#sourceFile').textContent(),'src/collectives/device/all_reduce.h');
  assert.equal(await page.locator('#lineInput').inputValue(),'306');assert.equal(await page.locator('#bindingSelect').inputValue(),'B0007');
  await page.locator('#diagramImage').evaluate(img=>img.decode());assert(await page.locator('#diagramImage').evaluate(img=>img.naturalWidth>0));
  for(const [file,line,binding] of [['src/collectives/device/all_reduce.h',328,'B0009'],['src/collectives/device/prims_simple.h',657,'B0030']]){
   await page.goto(locate(root,file,line));await page.waitForFunction(()=>window.NCCL_READER);
   assert.equal(await page.locator('#lineInput').inputValue(),String(line));assert.equal(await page.locator('#bindingSelect').inputValue(),binding);
   await page.locator('#diagramImage').evaluate(img=>img.decode());
  }
  const model=JSON.parse(fs.readFileSync(path.join(root,'docs/zh_CN/source-diagram-index.json'),'utf8'));
  const unbound=model.symbols.find(s=>s.kind==='函数'&&s.file.includes('/nvtx3/'));
  await page.goto(locate(root,unbound.file,unbound.line));await page.waitForFunction(()=>window.NCCL_READER);
  assert.equal(await page.locator('#sourceFile').textContent(),unbound.file);assert.equal(await page.locator('#lineInput').inputValue(),String(unbound.line));
  assert.equal(await page.locator('#noBinding').isVisible(),true);
  fs.appendFileSync(path.join(tmp,'src/collectives/device/all_reduce.h'),'\n// changed source\n');
  assert.throws(()=>locate(tmp,'src/collectives/device/all_reduce.h',306),/本地源码与图解版本不同/);
  // Standalone editor guide has no renderer assets or horizontal overflow.
  const guide='docs/zh_CN/13_SourceInsight与VSCode_源码图示跳转_离线版.html';
  const only=path.join(tmp,'guide-only');fs.mkdirSync(only);fs.copyFileSync(path.join(root,guide),path.join(only,path.basename(guide)));
  await page.goto(pathToFileURL(path.join(only,path.basename(guide))).href);
  assert.match(await page.locator('main').textContent(),/Ctrl\+Alt\+G/);
  await page.setViewportSize({width:390,height:844});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));
  assert.deepEqual(errors,[]);assert.deepEqual(network,[]);
  console.log(JSON.stringify({status:'PASS',launcherURLs:true,relayPreservesFragment:true,unicodeSpaceAndReservedPaths:true,exactSourceLines:true,narrowPhaseBindings:true,unboundSource:true,staleSourceRejected:true,standaloneEditorGuide:true,httpRequests:0,pageErrors:0}));
 }finally{await browser.close();fs.rmSync(tmp,{recursive:true,force:true})}
})().catch(e=>{console.error(e);process.exitCode=1});

/* Regression: copy each HTML alone into an empty directory, disable networking,
 * and exercise every frame/primitive plus function search, bodies and hashes.
 * Requires Playwright with Chromium installed; reading the HTML does not.
 * Usage: node tools/check_offline_docs_browser.cjs [evidence-directory]
 */
const { chromium } = require('playwright');
const fs = require('fs');
const os = require('os');
const path = require('path');
const { pathToFileURL } = require('url');
const assert = require('assert/strict');

(async () => {
 const root = path.resolve(__dirname, '..');
 const docs = path.join(root, 'docs/zh_CN');
 const isolated = fs.mkdtempSync(path.join(os.tmpdir(), 'nccl-html-alone-'));
 const evidence = process.argv[2];
 if (evidence) fs.mkdirSync(evidence, { recursive: true });
 const browser = await chromium.launch({headless:true,
  ...(process.env.NCCL_DOCS_CHROMIUM ? {executablePath:process.env.NCCL_DOCS_CHROMIUM} : {})});
 const context = await browser.newContext({offline:true, viewport:{width:1280,height:900}});
 const page = await context.newPage();
 const errors = [], network = [];
 page.on('pageerror', e => errors.push(e.message));
 page.on('request', r => {if (/^https?:/.test(r.url())) network.push(r.url());});
 let frames=0, primitives=0;
 try {
  // Include legacy HTML: this is the exact filename from the user's report.
  for (const name of ['01_通信原语与算法图解_离线版.html', 'visual-guide.html']) {
   const local=path.join(isolated,name);fs.copyFileSync(path.join(docs,name),local);
   await page.goto(pathToFileURL(local).href);
   await page.waitForFunction(()=>document.getElementById('counter').textContent==='1 / 7');
   for (const [kind,count] of [['allreduce',7],['allgather',4],['reducescatter',4]]) {
    await page.selectOption('#collective',kind);
    assert(await page.locator('#prev').isDisabled());
    for(let i=0;i<count;i++){
     await page.locator('#frame').evaluate(img=>img.decode());
     assert(await page.locator('#frame').evaluate(img=>img.naturalWidth>0&&img.src.startsWith('data:image/svg+xml;base64,')));
     assert.equal(await page.locator('#counter').textContent(),`${i+1} / ${count}`);
     if(i<count-1)await page.click('#next');frames++;
    }
    assert(await page.locator('#next').isDisabled());
    assert.equal(await page.locator('#ownership tr').count(),4);
   }
   for(let i=0;i<7;i++){
    await page.selectOption('#primitive',String(i));
    await page.locator('#primitiveImage').evaluate(img=>img.decode());
    assert(await page.locator('#primitiveImage').evaluate(img=>img.naturalWidth>0));primitives++;
   }
   const broken=await page.locator('img').evaluateAll(async imgs=>{
    await Promise.all(imgs.map(img=>img.decode().catch(()=>{})));
    return imgs.filter(img=>!img.naturalWidth||!img.complete).map(img=>img.alt);
   });
   assert.deepEqual(broken,[]);
   if(name!=='visual-guide.html'&&evidence){
    const title=page.getByRole('heading',{name:'9.1 AllGather没有归约',exact:true});
    await title.scrollIntoViewIfNeeded();
    await page.screenshot({path:path.join(evidence,'AllGather_图片离线显示.png')});
   }
   await page.setViewportSize({width:390,height:844});
   assert(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth+1));
   await page.setViewportSize({width:1280,height:900});
  }
  for(const name of ['05_全部函数内部步骤_离线搜索.html','function-logic.html']){
   const local=path.join(isolated,name);fs.copyFileSync(path.join(docs,name),local);
   await page.goto(pathToFileURL(local).href);
   assert.match(await page.locator('#count').textContent(),/1178/);
   await page.fill('#q','genericOp');
   assert(await page.locator('#results > details').count()>0);
   await page.locator('#results > details > summary').first().click();
   await page.waitForFunction(()=>document.querySelector('.step'));
   await page.locator('.source > summary').first().click();
   assert((await page.locator('.source pre').first().textContent()).includes('nelem'));
   await page.fill('#q','F1104');
   await page.locator('#f1104 > summary').click();
   assert.match(await page.locator('#f1104 .fn-body > p').first().textContent(),/sysfs/);
   await page.fill('#q','no_such_nccl_function_999');
   assert.match(await page.locator('#count').textContent(),/0/);
   await page.fill('#q','');await page.click('#more');
   assert.equal(await page.locator('#results > details').count(),80);
   await page.evaluate(()=>{location.hash='f0890'});
   await page.waitForFunction(()=>document.getElementById('f0890')?.open);
   if(evidence&&name!=='function-logic.html')await page.screenshot({path:path.join(evidence,'函数离线搜索与展开.png')});
   await page.setViewportSize({width:390,height:844});
   assert(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth+1));
   await page.setViewportSize({width:1280,height:900});
  }
  assert.deepEqual(errors,[]);assert.deepEqual(network,[]);
  console.log(JSON.stringify({status:'PASS',frames,primitives,standalonePages:4,functions:1178,networkRequests:network.length,pageErrors:errors.length,mobileWidth:390}));
 } finally {await browser.close();fs.rmSync(isolated,{recursive:true,force:true});}
})().catch(e=>{console.error(e);process.exitCode=1});

#!/usr/bin/env python3
"""Build the standalone source/comment/diagram reader. Standard library only."""
import argparse
import base64
from pathlib import Path

from source_diagram_model import DOC, READER, resolve
from build_offline_docs import json_script


def build(update_annotations=False):
    model = resolve(update_annotations)
    images = {ident:'data:image/svg+xml;base64,'+base64.b64encode((DOC/'visuals'/(ident+'.svg')).read_bytes()).decode('ascii')
              for ident in model['diagrams']}
    css = (DOC/'reader-assets/source-diagram-reader.css').read_text(encoding='utf-8')
    script = (DOC/'reader-assets/source-diagram-reader.js').read_text(encoding='utf-8')
    body = '''<header><h1>NCCL 2.12.12 · 源码、中文注释与图示对照</h1>
<p>V3.3 · 点源码行或注释中的B编号看对应分析；滚动可自动跟随。全部源码和图示已内嵌，单独下载即可断网阅读。</p></header>
<div class="toolbar" id="searchBox"><label for="symbolSearch">找函数/结构体/变量 <input id="symbolSearch" type="search" placeholder="例如 genericOp、ncclConnInfo、F0232、V编号"></label>
<button id="quickAllReduce">Ring AllReduce</button><button id="quickGeneric">Simple genericOp</button><div id="searchResults"></div></div>
<div class="toolbar"><label for="fileSelect">源码文件 <select id="fileSelect"></select></label>
<label for="lineInput">行 <input id="lineInput" type="number" min="1" value="1"></label><button id="lineGo">定位</button>
<label><input id="followSource" type="checkbox" checked>滚动源码时跟随图示</label><a id="permalink" href="#">当前定位链接</a><span id="status"></span></div>
<div class="workspace"><section class="pane source-pane"><div class="pane-title"><h2 id="sourceFile"></h2><div id="sourceInfo" class="muted"></div></div>
<div id="sourceScroll" aria-label="带中文注释的完整源码"></div><div class="legend"><span>浅蓝：对应代码范围</span><span>黄色：当前行</span><span>B编号可点击</span></div></section>
<section class="pane diagram-pane"><div class="pane-title"><h2>当前代码对应的图与分析</h2><div id="bindingSummary" class="muted"></div><select id="bindingSelect" aria-label="本文件图示绑定"></select></div>
<div id="diagramScroll"><div id="noBinding" hidden>这一行尚无专属图示。可点本文件的绑定条目，或在下方按图反查源码；未绑定不等于该源码没有作用。</div>
<div id="diagramDetails"><h3 id="bindingTitle"></h3><div id="bindingMeta"></div><p id="bindingNote"></p>
<div class="actions"><button id="previousBinding">上一处</button><button id="nextBinding">下一处</button><button id="zoomDiagram">放大图示</button></div>
<div class="actions"><label for="diagramSelect">本段图示</label><select id="diagramSelect"></select></div><img id="diagramImage" alt="当前源码对应图示"><div id="diagramCaption"></div>
<a id="diagramLink" href="#">在图解页打开（完整ZIP）</a></div>
<h3 class="section-title">按图示反查源码</h3><div class="actions"><select id="reverseSelect" aria-label="按图示反查源码"></select></div><p id="reverseCount" class="muted"></p><div id="reverseResults"></div></div>
<div class="legend">“直接对应”与“调用/结构上下文”分别标明；图示展示逻辑关系，运行条件仍以源码分支为准。</div></section></div>
<div id="imageModal" class="reader-modal" role="dialog" aria-modal="true" aria-label="放大图示"><div class="modal-head"><strong id="modalTitle"></strong><button id="closeModal">关闭（Esc）</button></div><div class="modal-body"><img id="modalImage" alt="放大后的图示"></div></div>
<noscript>此对照页需要启用JavaScript。无需安装Python、Node或网络服务。</noscript>'''
    result = '<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>NCCL 2.12.12 源码注释与图示对照 V3.3</title><style>'+css+'</style></head><body>'+body+json_script('source-model',model)+json_script('source-images',images)+'<script>'+script+'</script></body></html>\n'
    path=DOC/READER
    path.write_text(result,encoding='utf-8')
    print(f'Built {READER}: {len(model["files"])} source/build files, {len(model["symbols"])} symbols, {len(model["bindings"])} bindings, {len(model["diagrams"])} diagrams, {len(result.encode()):,} bytes')
    return model


if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--update-annotations',action='store_true',help='refresh diagram comments and annotated checksums; original blob IDs remain unchanged')
    args=p.parse_args()
    build(args.update_annotations)

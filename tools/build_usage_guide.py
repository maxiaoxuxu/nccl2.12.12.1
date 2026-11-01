#!/usr/bin/env python3
"""Render the beginner usage/notation guide; also shared by the reader help."""
from pathlib import Path
from build_offline_docs import DOC, markdown, page

USAGE = '12_使用方法与编号说明_离线版.html'


def usage_body():
    text=(DOC/'12_使用方法与编号说明.md').read_text(encoding='utf-8')
    body,headings=markdown(text,{},'usage')
    number_heading=next(ident for level,ident,title in headings if level==2 and '注释里的编号' in title)
    return body.replace('id="'+number_heading+'"','id="number-reference"')


def build():
    body=usage_body()
    intro='<nav><strong>NCCL 2.12.12 · 使用方法与编号说明</strong><p>这是一份可单独离线阅读的操作说明。完整ZIP内可从下面的链接开始看源码。</p><a href="11_源码注释与图示对照_离线版.html">打开源码与图示对照（完整ZIP）</a> · <a href="#number-reference">直接查编号含义</a></nav>'
    result=page('NCCL源码图解 使用方法与编号说明 V3.3.1',intro+body)
    (DOC/USAGE).write_text(result,encoding='utf-8')
    print('Built '+USAGE)
    return result


if __name__=='__main__':build()

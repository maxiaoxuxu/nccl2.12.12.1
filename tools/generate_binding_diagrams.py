#!/usr/bin/env python3
"""Draw exact source-navigation diagrams with the Python standard library."""
from pathlib import Path
from html import escape

OUT = Path(__file__).resolve().parents[1] / 'docs/zh_CN/visuals'


class Diagram:
    def __init__(self, title, height):
        self.height = height
        self.parts = [f'<title>{escape(title)}</title>',
                      '<defs><marker id="a" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M0 0L10 5L0 10Z" fill="#536b83"/></marker></defs>',
                      '<rect width="1040" height="100%" fill="#fff"/>',
                      f'<text x="28" y="40" font-size="25" font-weight="600">{escape(title)}</text>']

    def box(self, x, y, w, h, lines, color='#edf4ff'):
        self.parts.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="10" fill="{color}" stroke="#b9cce0"/>')
        first = y+h/2-(len(lines)-1)*13+7
        for i, line in enumerate(lines):
            self.parts.append(f'<text x="{x+w/2}" y="{first+i*26}" text-anchor="middle" font-size="19">{escape(line)}</text>')

    def edge(self, points, label='', lx=0, ly=0):
        p = ' '.join(f'{x},{y}' for x, y in points)
        self.parts.append(f'<polyline points="{p}" fill="none" stroke="#536b83" stroke-width="2" marker-end="url(#a)"/>')
        if label:
            self.parts.append(f'<text x="{lx}" y="{ly}" font-size="17">{escape(label)}</text>')

    def save(self, name):
        text = (f'<svg xmlns="http://www.w3.org/2000/svg" width="1040" height="{self.height}" viewBox="0 0 1040 {self.height}" role="img">'
                '<style>text{font-family:"Microsoft YaHei","Noto Sans SC",sans-serif;fill:#172d43}</style>'
                + ''.join(self.parts) + '</svg>\n')
        (OUT / (name+'.svg')).write_text(text, encoding='utf-8')


def main():
    d = Diagram('Channel生成：硬件候选图、全局拼接与每次操作选择', 860)
    d.box(250, 80, 540, 78, ['硬件发现与路径分析', 'ncclTopoGetSystem / ComputePaths'])
    for x, title, mode in [(40, 'Ring候选', 'MAXCHANNELS/2起始上限'),
                           (365, 'Tree候选', 'BALANCED_TREE'),
                           (690, 'CollNet候选', '支持时另行搜索')]:
        d.box(x, 235, 310, 85, [title, mode])
        d.edge([(520, 158), (520, 195), (x+155, 195), (x+155, 235)])
        d.edge([(x+155, 320), (x+155, 358), (520, 358), (520, 395)])
    d.box(250, 395, 540, 90, ['Bootstrap交换 + TopoPreset/Postset', '协调通道数、拼接全局Ring/Tree邻居'])
    d.box(250, 550, 540, 90, ['initChannel / setupChannel / transport建链', '保存ring.index、userRanks；设备视图随后复制'])
    d.edge([(520, 485), (520, 550)])
    d.box(40, 715, 455, 90, ['P2P通道编号空间与每peer预算', 'ncclTopoComputeP2pChannels'])
    d.box(545, 715, 455, 90, ['每次操作再选算法/协议/通道子集', 'getAlgoInfo → computeColl → bid'])
    d.edge([(520, 640), (520, 678), (267, 678), (267, 715)])
    d.edge([(520, 640), (520, 678), (772, 678), (772, 715)])
    d.save('28-channel-generation')

    d = Diagram('IB数据QP创建：双方建链与接收侧可选flush资源', 890)
    d.box(50, 85, 435, 80, ['发送侧 ncclIbConnect', '分阶段推进，sendComm可暂为空'])
    d.box(555, 85, 435, 80, ['接收侧 ncclIbAccept', '分阶段推进，recvComm可暂为空'])
    d.box(50, 230, 435, 85, ['循环q次：ncclIbCreateQp', 'RC QP创建并转INIT；持有本对象CQ'])
    d.box(555, 230, 435, 85, ['循环q次：ncclIbCreateQp', 'RC QP创建并转INIT；持有本对象CQ'])
    d.edge([(267, 165), (267, 230)])
    d.edge([(772, 165), (772, 230)])
    d.box(250, 390, 540, 90, ['通过建链socket交换QPN、MTU、地址', '对应QP配对；不是GPU用户张量传输'])
    d.edge([(267, 315), (267, 352), (520, 352), (520, 390)])
    d.edge([(772, 315), (772, 352), (520, 352), (520, 390)])
    d.box(250, 545, 540, 80, ['ncclIbRtrQp → ncclIbRtsQp', '经ready握手后可用于后续数据请求'])
    d.edge([(520, 480), (520, 545)])
    d.box(40, 710, 460, 115, ['发送/接收控制FIFO各自注册MR', '数据缓冲由NET层另行regMr', '多个数据QP可使用同一缓冲MR'])
    d.box(540, 710, 460, 115, ['接收侧满足gpuFlush.enabled时', '额外1个自连接QP + 1个host MR', '该附加QP数不乘q'])
    d.edge([(520, 625), (520, 665), (270, 665), (270, 710)])
    d.edge([(520, 625), (520, 665), (770, 665), (770, 710)])
    d.save('29-ib-qp-creation')

    d = Diagram('数据MR缓存：按页范围精确命中与引用释放', 940)
    d.box(250, 85, 540, 85, ['ncclIbRegMr(data, size)', '页对齐起址addr，计算覆盖页数pages'])
    d.box(250, 230, 540, 85, ['锁住本进程IB设备的mrCache', '查找addr和pages均相同的条目'])
    d.edge([(520, 170), (520, 230)])
    d.box(40, 395, 450, 85, ['精确命中：refs++', '返回已有ibv_mr，不重新注册'], '#e7f5ee')
    d.box(550, 395, 450, 85, ['未命中：reg_mr / reg_mr_iova2', '新增条目，refs=1'])
    d.edge([(520, 315), (520, 350), (265, 350), (265, 395)], '命中', 280, 340)
    d.edge([(520, 315), (520, 350), (775, 350), (775, 395)], '未命中', 790, 340)
    d.box(250, 555, 540, 85, ['mhandle供连接/数据请求复用', '本地lkey；向远端公布目标rkey'])
    d.edge([(265, 480), (265, 515), (520, 515), (520, 555)])
    d.edge([(775, 480), (775, 515), (520, 515), (520, 555)])
    d.box(250, 710, 540, 90, ['ncclIbDeregMr：refs--', '仅refs归零才删除条目并真正注销'])
    d.edge([(520, 640), (520, 710)])
    d.box(130, 845, 780, 65, ['范围包含不等于命中；控制FIFO和flush MR未经过本缓存'], '#fff3df')
    d.save('30-mr-cache')
    print('Generated 3 source-binding diagrams')


if __name__ == '__main__':
    main()

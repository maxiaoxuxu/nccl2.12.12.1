# 用Source Insight或VS Code阅读源码并跳转到图示

V3.3.2。**可以在源码中把光标放到想理解的位置，执行“查看当前代码图解”，让浏览器打开对应的源码、注释、图示与分析。无需记F/B编号。** 两个编辑器共用同一个离线跳转工具，不需要启动服务器。

## 1. 先分清两种查看方式

| 使用方式 | 当前提供的效果 | 需要准备什么 |
| --- | --- | --- |
| 浏览器直接打开11对照页 | 同页显示源码、注释、图示；滚动源码可跟随切图，B编号可点击 | 单个11对照HTML即可，不用Python |
| VS Code阅读真实源码 | 执行项目任务，按当前文件及光标行跳到浏览器图解 | 完整ZIP、Python 3.9或更新版本、打开仓库根文件夹 |
| Source Insight阅读真实源码 | 执行自定义命令，按当前文件及光标行跳到浏览器图解 | 完整ZIP、Python 3.9或更新版本、配置一次命令 |
| 编辑器内部原生图示侧栏、光标移动自动切图 | 本版没有提供；VS Code可另做Webview扩展 | 不能把下面的快捷键任务当成已经安装的侧栏插件 |

这里的“跳转”是 **源码编辑器 → 本地浏览器对照页**。源码里的普通文本注释不会因为出现B编号就自动变成可点击图片。可把编辑器和浏览器并排放置；换到另一段代码后再执行同一命令。

## 2. 下载与准备

1. 下载分析分支的完整源码与图解ZIP，右键“全部解压”。请使用 `codex/nccl-2.12.12-full-source-walkthrough` 分支。
2. 仓库根目录是同时包含 `src`、`docs`、`tools` 的文件夹。不要在ZIP预览窗口里运行。
3. 安装Python 3.9或更新版本。Windows终端中执行 `python --version`；macOS/Linux使用 `python3 --version`。只读11对照HTML仍不需要Python。
4. 保留 `tools/open_source_diagram.py`、`docs/zh_CN/source-diagram-index.json` 和11对照HTML及对应源码；不要混用不同版本。
5. 源码使用UTF-8。为准确定位，请先保存编辑器中的修改；实际修改源码后必须同步重建和核验图解，详见第6节。

文件夹路径含中文或空格可以使用；下面的路径参数引号请保留。

## 3. VS Code：先用任务，再设快捷键

1. 选择 **文件 → 打开文件夹**，打开仓库根目录。如果已打开其他项目，先确认当前工作区根目录是这份NCCL。项目任务需要工作区信任；仅对你信任的下载内容启用。
2. 打开 `src/collectives/device/all_reduce.h`，把光标放到 `prims.directRecvReduceCopySend` 所在行，或它前面的“完成本块归约并开始结果传播”中文注释中。
3. 按 `Ctrl+Shift+P`，查找 **Tasks: Run Task / 任务: 运行任务**，选择 **NCCL：查看当前代码图解**。
4. 浏览器应打开同一文件、同一行，右侧显示该代码段的图与分析。页面黄色表示当前行，浅蓝色表示对应代码范围。

任务已经保存在仓库的 `.vscode/tasks.json` 中，传入 `${file}` 和 `${lineNumber}`，不用每次填写路径。Windows默认执行 `python`，macOS/Linux默认执行 `python3`。macOS命令面板快捷键为 `Cmd+Shift+P`。

如果需要一键执行：在命令面板打开 **Preferences: Open Keyboard Shortcuts (JSON) / 首选项: 打开键盘快捷方式(JSON)**，将 [VSCode_图示跳转快捷键示例.json](../../tools/editor-integration/VSCode_图示跳转快捷键示例.json) 中的那一个对象加入自己的数组；已有其他快捷键时不要覆盖它们。之后在源码中按 **Ctrl+Alt+G**。如果有快捷键冲突，可以改成自己习惯的组合。

项目文件不会自动修改你的个人快捷键；只复制示例JSON文件到项目中也不会自动生效。

## 4. Source Insight：配置一次自定义命令

以下菜单按Source Insight 4说明；Source Insight 3也有Custom Commands以及`%f/%l`替换，菜单或编码设置请以实际版本为准。

1. 打开本份源码项目，选择 **Tools → Custom Commands**，新建命令，名称可填 **查看NCCL图示**。
2. 假设完整解压目录是 `D:\学习资料\NCCL源码`，在 **Run** 中填写下面这一整行。请将示例目录替换为你的真实仓库目录：

```text
python "D:\学习资料\NCCL源码\tools\open_source_diagram.py" --file "%f" --line %l
```

3. **Dir**可填写实际仓库目录。脚本本身不依赖工作目录，路径定位以脚本所在的这份仓库为准。
4. 点击 **Define** 保存。可用对话框中的 **Keys** 将它绑定到例如 **Ctrl+Alt+G**；若已有占用，请换其他键。
5. 打开同一份 `src/collectives/device/all_reduce.h`，把光标放到上述归约完成代码段，执行自定义命令或快捷键。

`%f`会被Source Insight替换为当前文件完整路径，`%l`替换为当前行号。不要手动把它们改成固定的文件或固定的数字。Python路径如果有空格，同样需要引号，例如将开头的 `python` 改为 `"C:\Program Files\Python311\python.exe"`。

不需要启用“将输出粘贴到当前选择”或“替换当前选择”等选项；此命令用于阅读，不应改写源码。首次排查问题时可以启用输出捕获/Pause When Done，查看脚本报错。若使用Save Files First，修改后的文件仍须与配套图解一致才会允许跳转。

## 5. 不用编辑器也能测试

在仓库根目录执行下面命令。306是本版AllReduce归约完成调用所在的行号；实际使用编辑器时行号会自动传入。

```text
python tools/open_source_diagram.py --file src/collectives/device/all_reduce.h --line 306
```

macOS/Linux将python改成python3。只想检查定位链接、暂时不启动浏览器时：

```text
python tools/open_source_diagram.py --file src/collectives/device/all_reduce.h --line 306 --print-url
```

如果系统把HTML交给文本编辑器，或默认浏览器启动失败，可以显式指定浏览器，例如Windows的Edge：

```text
python tools/open_source_diagram.py --file src/collectives/device/all_reduce.h --line 306 --browser "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
```

路径以你电脑上的实际安装位置为准。Source Insight可在Run命令末尾追加同样的 `--browser "完整路径"`；VS Code则在任务的 `args` 数组末尾增加 `"--browser"` 和 `"浏览器完整路径"` 两项，JSON中的Windows反斜杠写成 `\\`。也可以复制 `--print-url` 的输出到浏览器地址栏。

## 6. 对应规则与常见问题

跳转按当前文件和行号找已有人工绑定：优先选包含本行的具体代码段，其次选所在函数/结构体的整体说明。没有人工绑定时，仍会定位源码，并显示“暂无专属图示”。这不表示该代码没有作用，也不会强行套用邻近函数的图。

| 现象 | 原因与处理 |
| --- | --- |
| 找不到NCCL任务 | VS Code需要打开整个仓库根文件夹；检查 `.vscode/tasks.json` 是否随ZIP解压，以及工作区是否允许执行任务 |
| 找不到python或打开Microsoft Store | 命令没有指向实际Python；在任务的Windows `command` 或Source Insight的Run开头填写Python可执行文件完整路径 |
| 找不到HTML/索引 | 只下载了部分文件，或移动了工具；重新完整解压配套ZIP |
| 当前文件不在这份NCCL目录中 | 编辑器打开了其他副本，或命令指向另一个解压目录；统一为同一份源码 |
| 当前文件尚未收录 | 工具、文档等非源码文件没有收入121份源码/构建文件清单；请在已收录源码中执行 |
| 本地源码与图解版本不同 | 已修改源码/注释、使用了原始master、或混用了旧HTML与新源码；请先用配套版本。单纯LF/CRLF换行转换不影响检查 |
| 光标行号不对 | 先保存文件，确认不是未保存的编辑内容；行号从1开始，F/B编号不能当行号传入 |
| 浏览器打开了但没有自动定位 | 先复制 `--print-url` 输出验证，或使用 `--browser` 指定浏览器。默认启动会先打开一个本地临时跳转页，由浏览器应用完整定位链接 |
| 想在编辑器内部直接显示交互图 | 当前交付的是任务/自定义命令跳转；VS Code原生侧栏和自动跟随需要单独实现扩展 |

维护者确实修改源码后，在仓库根目录运行 `python tools/build_source_diagram_reader.py`，再运行 `python tools/verify_source_diagram_bindings.py` 及相关源码核验；应人工复核代码范围和分析含义。**仅仅重新生成HTML不能证明旧图示仍然语义正确。** 索引与HTML必须一并重新生成和分发。

## 7. 验证范围与官方资料

本次通过Python脚本、VS Code任务参数的实际展开模拟、中文/空格路径、未绑定源码、版本不匹配拒绝、以及断网浏览器的源码/行号/图示检查。当前环境没有Source Insight或VS Code桌面程序，未实机点击这两个编辑器；其接入方式按官方文档配置，不宣称原生应用端到端实测。

- [VS Code任务变量：当前文件与行号](https://code.visualstudio.com/docs/reference/variables-reference)
- [VS Code任务与任务快捷键](https://code.visualstudio.com/docs/debugtest/tasks)
- [Source Insight 4 Custom Commands及%f/%l替换](https://www.sourceinsight.com/doc/v4/userguide/Manual/Command_Reference/Custom_Commands1.htm)
- [Source Insight 3命令行替换](https://www.sourceinsight.com/doc/v3/ad917808.htm)
- [VS Code Webview扩展能力](https://code.visualstudio.com/api/extension-guides/webview)

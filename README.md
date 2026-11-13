# 实验二：简单文本编辑器（Qt Widgets）

东莞理工学院《Qt 应用程序开发》2026 秋季实验二。
基于 **Qt 5.12.11（实验环境）/ Qt 6.x（本机验证）** 的 CMake 工程，代码同时兼容两个大版本。

## 功能清单（对照实验要求）

| 实验要求 | 实现 |
|---|---|
| 混合式 UI 设计、Action 编辑器、按功能分组 | 菜单栏 + 4 个分组工具栏（文件/编辑/格式/视图），全部条目由 `QAction` 创建（`MainWindow::createActions`），等价于 Designer 的 Action 编辑器 |
| 文件打开/保存 | 新建、打开、保存、另存为、退出；UTF-8 读写；打开失败/写入失败有提示 |
| 复制/粘贴/剪切 | 编辑菜单 + 工具栏；Action 使能与选区/剪贴板状态同步 |
| 字体大小、字体颜色 | 格式菜单（QFontDialog/QColorDialog）+ 工具栏字体族/字号下拉框 |
| 显示/关闭行号 | `CodeEditor` 从 `QPlainTextEdit` 派生，画行号区（参考 Qt 官方 Code Editor 示例），视图菜单可开关 |
| 打开/关闭自动换行 | 视图菜单 `lineWrapAct`（checkable） |
| 撤销/重做 | 编辑菜单 + 工具栏，使能与文档状态同步 |
| 背景色修改 | 格式菜单 QColorDialog |
| 修改状态自动改窗口标题 | `textChanged → updateWindowTitle`，标题带 `*` |
| 关于对话框 | `AboutDialog`（独立 QDialog） |
| 查找对话框 | `FindDialog`：区分大小写、向上查找、循环查找、未找到提示 |
| 替换对话框 | `ReplaceDialog`：查找下一个、替换当前、全部替换、未找到提示 |
| 定时自动保存（选做） | 每 60 秒自动保存到当前文件（`MainWindow::autoSave`，未命名文件不自动保存） |
| 多窗口 | 主窗口 + 查找/替换/关于三个独立对话框 |

## 健壮性设计（实验“任务完成度”要求）

- **未保存关闭提示**：`closeEvent → maybeSave()`，提供“保存 / 不保存 / 取消”。
- **打开异常文件**：`loadFile` 打开失败/读取失败弹警告框并返回 false，不崩溃。
- **保存与另存为**：`saveFile` 未命名时自动转 `saveFileAs`；写入失败有提示。
- **Action 使能状态**：`copyAvailable / undoAvailable / redoAvailable / textChanged` 信号联动更新剪切/复制/粘贴/撤销/重做/全选。
- **查找/替换边界**：未找到有提示；支持连续查找；“替换全部”统计并提示替换个数；空输入禁用按钮。

## 项目结构

```
TextEditor/
├── CMakeLists.txt      # Qt5/Qt6 兼容，AUTOMOC/AUTOUIC
├── main.cpp            # 程序入口
├── mainwindow.h/.cpp   # 主窗口：Action/菜单/工具栏/状态栏/文件与编辑逻辑/自动保存
├── codeeditor.h/.cpp   # 带行号的编辑器（QPlainTextEdit 派生）
├── finddialog.h/.cpp   # 查找对话框
├── replacedialog.h/.cpp# 替换对话框
├── aboutdialog.h/.cpp  # 关于对话框
└── build.ps1           # Windows 一键构建脚本
```

## 构建与运行

### 方式一：Qt Creator（推荐）

1. 打开 Qt Creator → `File → Open File or Project` → 选择 `CMakeLists.txt`。
2. 选择已配置的 Qt 套件（Kit）→ 点击绿色运行按钮。
3. **注意**：若源码路径含中文（如 `C:\Users\111\Downloads\文本编辑器\...`），请把**构建目录**（Build Directory）设为英文路径，例如 `C:\Users\111\Downloads\TextEditor-build`。原因是 Qt 的 `moc` 无法在中文路径下创建生成文件（本机已验证）。

### 方式二：命令行（本机已验证）

以 Qt 6.11.2 + MinGW 13 为例：

```powershell
# 1. 把 Qt 工具链加入 PATH
$env:Path = "C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.2\mingw_64\bin;" + $env:Path

# 2. 配置（构建目录务必用英文路径！）
cmake -S "C:\Users\111\Downloads\文本编辑器\TextEditor" `
      -B "C:\Users\111\Downloads\TextEditor-build" -G Ninja `
      -DCMAKE_BUILD_TYPE=Debug `
      -DCMAKE_PREFIX_PATH="C:\Qt\6.11.2\mingw_64" `
      -DCMAKE_CXX_COMPILER="C:\Qt\Tools\mingw1310_64\bin\g++.exe"

# 3. 编译
ninja -C "C:\Users\111\Downloads\TextEditor-build"

# 4. 运行
& "C:\Users\111\Downloads\TextEditor-build\bin\TextEditor.exe"
```

也可以直接运行项目内的 `build.ps1`（已内置上述步骤，自动把构建目录放到英文路径）。

实验环境为 **Qt 5.12.11**：使用 Qt 5 的 MinGW Kit，把上面 `-DCMAKE_PREFIX_PATH` 换成 Qt 5.12.11 的安装路径即可，代码无需改动。

## 已验证的测试结果

已通过自动化冒烟测试（38 项全部通过）：

- 初始标题 / 行号默认显示
- 输入后标题带 `*`、撤销可用、无选区剪切/复制禁用、粘贴/全选启用
- 有选区剪切/复制启用
- 撤销清空 / 重做恢复
- 行号与自动换行开关
- 保存到临时文件：文件内容一致、标题更新、修改标志清除
- 再次修改后 `saveFile` 直接保存成功
- 打开不存在的文件：返回 false、弹出警告、不崩溃
- 打开已有文件：内容正确加载
- 查找：首次命中、连续查找、未找到提示
- 替换：全部替换（2 处）、替换当前选中
- 修改后关闭：弹出未保存提示，放弃修改后窗口正常关闭

## 实验报告填写提示

每个“实验步骤及结果”小节建议截图 + 贴关键代码：

1. **UI 布局/菜单/工具栏/QPlainTextEdit/layoutMargins**：主窗口整体截图，贴 `createActions/createMenus/createToolBars` 关键片段。
2. **关于对话框**：`aboutdialog.cpp`，运行 `帮助 → 关于` 截图。
3. **查找对话框**：`finddialog.cpp`，运行 `编辑 → 查找` 截图。
4. **替换对话框**：`replacedialog.cpp`，运行 `编辑 → 替换` 截图。
5. **新建/另存/保存/打开**：`newFile/openFile/saveFile/saveFileAs/loadFile/saveToFile`，逐项操作截图。
6. **撤销/恢复/剪切/复制/粘贴/全选**：`updateEditActions` + 对应槽。
7. **字体/颜色/背景/自动换行**：格式菜单与工具栏截图。
8. **工具栏/状态栏显示隐藏、状态栏信息**：`视图` 菜单与 `updateStatusBar`。
9. **cut/copy/paste 使能**：选中前后按钮状态对比截图。
10. **查找窗口操作**：命中、未找到、循环查找截图。
11. **替换窗口操作**：替换一个、全部替换截图。
12. **行号（promote QPlainTextEdit）**：`codeeditor.cpp`，开启/关闭行号对比截图。

**边界测试（至少 3 个，实验要求）**，可直接使用以下已验证场景：

| 测试输入 | 程序现象 | 原因分析 | 修改后结果 |
|---|---|---|---|
| 修改后直接关闭窗口 | 弹出“是否保存”提示 | `closeEvent → maybeSave` | 选择保存/不保存/取消均正常 |
| 打开不存在的文件 | 弹警告框，程序不崩溃 | `loadFile` 检查 `QFile::open` 返回值 | 返回 false 并提示 |
| 查找不存在的词 | 弹“未找到”提示 | `findText` 检查 `QTextDocument::find` 返回值 | 正常提示，可继续输入 |
| 空查找框 | “查找下一个”按钮禁用 | `enableFindButton` 联动文本变化 | 无法发起空查找 |
| 全部替换 0 处 | 弹“未找到”提示 | `replaceAll` 统计 count==0 | 正常提示 |

**AI 辅助开发记录（1 个代表性过程，可写）**：
> 问题：输入文字后窗口标题不更新、剪切/复制按钮不使能。
> 提示词：`QPlainTextEdit 修改后如何让 QAction 使能状态和窗口标题实时更新？`
> AI 方案：连接 `copyAvailable / undoAvailable / redoAvailable / textChanged` 信号到统一的更新函数。
> 运行验证：编写自动化冒烟测试，初始 6 项失败，确认正是缺少信号连接。
> 发现的问题：测试中 6 项 Action/标题状态断言失败。
> 自己的修改：在构造函数中补齐 5 个信号连接，重跑测试 38 项全部通过。

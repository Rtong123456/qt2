#include "mainwindow.h"

#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFontDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPalette>
#include <QRegularExpression>
#include <QStatusBar>
#include <QStyle>
#include <QTextCursor>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isUntitled(true)
{
    // 中央编辑器
    editor = new CodeEditor(this);
    setCentralWidget(editor);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createAutoSaveTimer();

    // 编辑状态信号 → 同步 Action 使能、窗口标题、状态栏
    connect(editor, &CodeEditor::copyAvailable, this,
            [this](bool) { updateEditActions(); });
    connect(editor, &CodeEditor::undoAvailable, this,
            [this](bool) { updateEditActions(); });
    connect(editor, &CodeEditor::redoAvailable, this,
            [this](bool) { updateEditActions(); });
    connect(editor, &CodeEditor::textChanged, this, [this]() {
        updateEditActions();
        updateWindowTitle();
        updateStatusBar();
    });
    connect(editor, &CodeEditor::cursorPositionChanged, this,
            [this]() { updateStatusBar(); });

    // 初始字体：等宽字体，便于编辑
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSize(12);
    editor->setFont(fixedFont);
    if (fontCombo) {
        fontCombo->setCurrentFont(fixedFont);
        fontSizeCombo->setCurrentText(QString::number(fixedFont.pointSize()));
    }

    // 初始状态
    setCurrentFile(QString());
    updateEditActions();
    updateStatusBar();
    updateWindowTitle();

    resize(900, 640);
}

// ---------------------------------------------------------------- 内部 ----

void MainWindow::createActions()
{
    // 图标优先使用系统标准图标，避免额外资源文件；
    // 其余操作使用纯文本按钮，可通过 QStyle::standardIcon 自行替换。

    // ---- 文件 ----
    newAct = new QAction(style()->standardIcon(QStyle::SP_FileIcon),
                         QStringLiteral("新建(&N)"), this);
    newAct->setShortcut(QKeySequence::New);
    newAct->setStatusTip(QStringLiteral("新建一个空白文档"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton),
                          QStringLiteral("打开(&O)..."), this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip(QStringLiteral("打开一个已有文档"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    saveAct = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton),
                          QStringLiteral("保存(&S)"), this);
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setStatusTip(QStringLiteral("保存当前文档"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    saveAsAct = new QAction(style()->standardIcon(QStyle::SP_DriveFDIcon),
                            QStringLiteral("另存为(&A)..."), this);
    saveAsAct->setShortcut(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(QStringLiteral("将当前文档另存为新文件"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);

    exitAct = new QAction(QStringLiteral("退出(&X)"), this);
    exitAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    exitAct->setStatusTip(QStringLiteral("退出程序"));
    connect(exitAct, &QAction::triggered, this, &MainWindow::close);

    // ---- 编辑 ----
    undoAct = new QAction(style()->standardIcon(QStyle::SP_ArrowBack),
                          QStringLiteral("撤销(&U)"), this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setStatusTip(QStringLiteral("撤销上一次操作"));
    connect(undoAct, &QAction::triggered, editor, &CodeEditor::undo);

    redoAct = new QAction(style()->standardIcon(QStyle::SP_ArrowForward),
                          QStringLiteral("重做(&R)"), this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setStatusTip(QStringLiteral("重做上一次撤销的操作"));
    connect(redoAct, &QAction::triggered, editor, &CodeEditor::redo);

    cutAct = new QAction(QStringLiteral("剪切(&T)"), this);
    cutAct->setShortcut(QKeySequence::Cut);
    cutAct->setStatusTip(QStringLiteral("剪切选中的内容"));
    connect(cutAct, &QAction::triggered, editor, &CodeEditor::cut);

    copyAct = new QAction(QStringLiteral("复制(&C)"), this);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setStatusTip(QStringLiteral("复制选中的内容"));
    connect(copyAct, &QAction::triggered, editor, &CodeEditor::copy);

    pasteAct = new QAction(QStringLiteral("粘贴(&P)"), this);
    pasteAct->setShortcut(QKeySequence::Paste);
    pasteAct->setStatusTip(QStringLiteral("粘贴剪贴板内容"));
    connect(pasteAct, &QAction::triggered, editor, &CodeEditor::paste);

    selectAllAct = new QAction(QStringLiteral("全选(&A)"), this);
    selectAllAct->setShortcut(QKeySequence::SelectAll);
    selectAllAct->setStatusTip(QStringLiteral("选中全部内容"));
    connect(selectAllAct, &QAction::triggered, editor, &CodeEditor::selectAll);

    findAct = new QAction(style()->standardIcon(QStyle::SP_FileDialogContentsView),
                          QStringLiteral("查找(&F)..."), this);
    findAct->setShortcut(QKeySequence::Find);
    findAct->setStatusTip(QStringLiteral("在文档中查找内容"));
    connect(findAct, &QAction::triggered, this, &MainWindow::find);

    replaceAct = new QAction(style()->standardIcon(QStyle::SP_FileDialogInfoView),
                             QStringLiteral("替换(&E)..."), this);
    replaceAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    replaceAct->setStatusTip(QStringLiteral("查找并替换文档内容"));
    connect(replaceAct, &QAction::triggered, this, &MainWindow::replace);

    // ---- 格式 ----
    fontDialogAct = new QAction(QStringLiteral("字体(&F)..."), this);
    fontDialogAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    fontDialogAct->setStatusTip(QStringLiteral("设置编辑器字体"));
    connect(fontDialogAct, &QAction::triggered, this, &MainWindow::setFontDialog);

    fontColorAct = new QAction(QStringLiteral("字体颜色(&C)..."), this);
    fontColorAct->setStatusTip(QStringLiteral("设置文字颜色"));
    connect(fontColorAct, &QAction::triggered, this, &MainWindow::setFontColor);

    bgColorAct = new QAction(QStringLiteral("背景色(&B)..."), this);
    bgColorAct->setStatusTip(QStringLiteral("设置编辑器背景颜色"));
    connect(bgColorAct, &QAction::triggered, this, &MainWindow::setBackgroundColor);

    // ---- 视图 ----
    lineWrapAct = new QAction(QStringLiteral("自动换行(&W)"), this);
    lineWrapAct->setCheckable(true);
    lineWrapAct->setChecked(true);
    lineWrapAct->setStatusTip(QStringLiteral("打开/关闭自动换行"));
    connect(lineWrapAct, &QAction::toggled, this, &MainWindow::setLineWrap);

    lineNumberAct = new QAction(QStringLiteral("显示行号(&L)"), this);
    lineNumberAct->setCheckable(true);
    lineNumberAct->setChecked(true);
    lineNumberAct->setStatusTip(QStringLiteral("显示/隐藏行号"));
    connect(lineNumberAct, &QAction::toggled, this, &MainWindow::setLineNumberVisible);

    toolBarAct = new QAction(QStringLiteral("工具栏(&T)"), this);
    toolBarAct->setCheckable(true);
    toolBarAct->setChecked(true);
    toolBarAct->setStatusTip(QStringLiteral("显示/隐藏工具栏"));
    connect(toolBarAct, &QAction::toggled, this, &MainWindow::setToolBarVisible);

    statusBarAct = new QAction(QStringLiteral("状态栏(&S)"), this);
    statusBarAct->setCheckable(true);
    statusBarAct->setChecked(true);
    statusBarAct->setStatusTip(QStringLiteral("显示/隐藏状态栏"));
    connect(statusBarAct, &QAction::toggled, this, &MainWindow::setStatusBarVisible);

    // ---- 帮助 ----
    aboutAct = new QAction(QStringLiteral("关于(&A)..."), this);
    aboutAct->setStatusTip(QStringLiteral("显示关于本程序的信息"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(QStringLiteral("关于 Qt"), this);
    aboutQtAct->setStatusTip(QStringLiteral("显示 Qt 版本信息"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createMenus()
{
    // ---- 文件菜单 ----
    fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // ---- 编辑菜单 ----
    editMenu = menuBar()->addMenu(QStringLiteral("编辑(&E)"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();
    editMenu->addAction(selectAllAct);
    editMenu->addSeparator();
    editMenu->addAction(findAct);
    editMenu->addAction(replaceAct);

    // ---- 格式菜单 ----
    formatMenu = menuBar()->addMenu(QStringLiteral("格式(&O)"));
    formatMenu->addAction(fontDialogAct);
    formatMenu->addAction(fontColorAct);
    formatMenu->addAction(bgColorAct);

    // ---- 视图菜单 ----
    viewMenu = menuBar()->addMenu(QStringLiteral("视图(&V)"));
    viewMenu->addAction(lineWrapAct);
    viewMenu->addAction(lineNumberAct);
    viewMenu->addSeparator();
    viewMenu->addAction(toolBarAct);
    viewMenu->addAction(statusBarAct);

    // ---- 帮助菜单 ----
    helpMenu = menuBar()->addMenu(QStringLiteral("帮助(&H)"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    // 四个工具栏，按功能分组
    fileToolBar = addToolBar(QStringLiteral("文件工具栏"));
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));
    fileToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addAction(saveAsAct);

    editToolBar = addToolBar(QStringLiteral("编辑工具栏"));
    editToolBar->setObjectName(QStringLiteral("editToolBar"));
    editToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);
    editToolBar->addSeparator();
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addSeparator();
    editToolBar->addAction(findAct);
    editToolBar->addAction(replaceAct);

    // 格式工具栏：字体族 + 字号 + 颜色
    formatToolBar = addToolBar(QStringLiteral("格式工具栏"));
    formatToolBar->setObjectName(QStringLiteral("formatToolBar"));
    formatToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QLabel *fontLabel = new QLabel(QStringLiteral(" 字体 "), this);
    formatToolBar->addWidget(fontLabel);

    fontCombo = new QFontComboBox(this);
    fontCombo->setMaximumWidth(160);
    fontCombo->setToolTip(QStringLiteral("字体"));
    formatToolBar->addWidget(fontCombo);

    QLabel *sizeLabel = new QLabel(QStringLiteral(" 字号 "), this);
    formatToolBar->addWidget(sizeLabel);

    fontSizeCombo = new QComboBox(this);
    fontSizeCombo->setEditable(true);
    fontSizeCombo->setMaximumWidth(70);
    const int sizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 32, 36, 48, 72};
    for (int s : sizes)
        fontSizeCombo->addItem(QString::number(s));
    fontSizeCombo->setToolTip(QStringLiteral("字号"));
    formatToolBar->addWidget(fontSizeCombo);

    formatToolBar->addSeparator();
    formatToolBar->addAction(fontColorAct);
    formatToolBar->addAction(bgColorAct);

    viewToolBar = addToolBar(QStringLiteral("视图工具栏"));
    viewToolBar->setObjectName(QStringLiteral("viewToolBar"));
    viewToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    viewToolBar->addAction(lineWrapAct);
    viewToolBar->addAction(lineNumberAct);
    viewToolBar->addSeparator();
    viewToolBar->addAction(toolBarAct);
    viewToolBar->addAction(statusBarAct);

    connect(fontCombo, &QFontComboBox::currentFontChanged, this,
            [this](const QFont &font) { applyFontFamily(font.family()); });
    connect(fontSizeCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::applyFontSize);
}

void MainWindow::createStatusBar()
{
    positionLabel = new QLabel(this);
    infoLabel = new QLabel(this);

    statusBar()->addWidget(positionLabel);            // 左侧：行、列
    statusBar()->addPermanentWidget(infoLabel);       // 右侧：字符数/行数/状态
    statusBar()->showMessage(QStringLiteral("就绪"), 3000);
}

void MainWindow::createAutoSaveTimer()
{
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(60 * 1000); // 每 60 秒自动保存一次
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    autoSaveTimer->start();
}

// ---------------------------------------------------------------- 文件 ----

void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    editor->clear();
    setCurrentFile(QString());
    statusBar()->showMessage(QStringLiteral("已新建文档"), 3000);
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString fileName = QFileDialog::getOpenFileName(
                this, QStringLiteral("打开文件"), QString(),
                QStringLiteral("文本文件 (*.txt *.md);;所有文件 (*)"));
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::saveFile()
{
    if (isUntitled)
        return saveFileAs();
    return saveToFile(curFile);
}

bool MainWindow::saveFileAs()
{
    const QString fileName = QFileDialog::getSaveFileName(
                this, QStringLiteral("另存为"), curFile,
                QStringLiteral("文本文件 (*.txt);;所有文件 (*)"));
    if (fileName.isEmpty())
        return false;
    return saveToFile(fileName);
}

bool MainWindow::maybeSave()
{
    if (!editor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(
                this, QStringLiteral("简单文本编辑器"),
                QStringLiteral("文档已修改但尚未保存，是否保存更改？"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save);

    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default:
        return true;
    }
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("打开文件"),
                             QStringLiteral("无法打开文件：\n%1\n\n原因：%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    const QString content = in.readAll();
    if (in.status() != QTextStream::Ok) {
        QMessageBox::warning(this, QStringLiteral("打开文件"),
                             QStringLiteral("读取文件内容时出错：%1").arg(fileName));
        file.close();
        return false;
    }
    file.close();

    editor->setPlainText(content);
    setCurrentFile(fileName);
    statusBar()->showMessage(QStringLiteral("已打开 %1").arg(fileName), 3000);
    return true;
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, QStringLiteral("保存文件"),
                             QStringLiteral("无法保存文件：\n%1\n\n原因：%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out << editor->toPlainText();
    out.flush();

    if (out.status() != QTextStream::Ok || file.error() != QFile::NoError) {
        QMessageBox::warning(this, QStringLiteral("保存文件"),
                             QStringLiteral("写入文件时出错：%1").arg(fileName));
        file.close();
        return false;
    }
    file.close();

    setCurrentFile(fileName);
    statusBar()->showMessage(QStringLiteral("已保存 %1").arg(fileName), 3000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    isUntitled = curFile.isEmpty();
    editor->document()->setModified(false);
    updateWindowTitle();
    updateEditActions();
    updateStatusBar();
}

// ---------------------------------------------------------------- 编辑 ----

void MainWindow::find()
{
    FindDialog dlg(editor, this);
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection())
        dlg.setFindText(cursor.selectedText());
    dlg.exec();
}

void MainWindow::replace()
{
    ReplaceDialog dlg(editor, this);
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection())
        dlg.setFindText(cursor.selectedText());
    dlg.exec();
}

// ---------------------------------------------------------------- 格式 ----

void MainWindow::setFontDialog()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, editor->font(), this,
                                            QStringLiteral("选择字体"));
    if (ok) {
        editor->setFont(font);
        fontCombo->setCurrentFont(font);
        fontSizeCombo->setCurrentText(QString::number(font.pointSize()));
    }
}

void MainWindow::applyFontFamily(const QString &family)
{
    if (family.isEmpty())
        return;
    QFont f = editor->font();
    f.setFamily(family);
    editor->setFont(f);
}

void MainWindow::applyFontSize(const QString &sizeText)
{
    bool ok = false;
    const int size = sizeText.toInt(&ok);
    if (!ok || size <= 0 || size > 300)
        return;
    QFont f = editor->font();
    f.setPointSize(size);
    editor->setFont(f);
}

void MainWindow::setFontColor()
{
    const QColor color = QColorDialog::getColor(editor->palette().color(QPalette::Text),
                                                this, QStringLiteral("字体颜色"));
    if (!color.isValid())
        return;

    QPalette pal = editor->palette();
    pal.setColor(QPalette::Text, color);
    editor->setPalette(pal);
}

void MainWindow::setBackgroundColor()
{
    const QColor color = QColorDialog::getColor(editor->palette().color(QPalette::Base),
                                                this, QStringLiteral("背景色"));
    if (!color.isValid())
        return;

    QPalette pal = editor->palette();
    pal.setColor(QPalette::Base, color);
    editor->setPalette(pal);
}

// ---------------------------------------------------------------- 视图 ----

void MainWindow::setLineWrap(bool on)
{
    editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                               : QPlainTextEdit::NoWrap);
}

void MainWindow::setLineNumberVisible(bool on)
{
    editor->setLineNumberAreaVisible(on);
}

void MainWindow::setToolBarVisible(bool on)
{
    fileToolBar->setVisible(on);
    editToolBar->setVisible(on);
    formatToolBar->setVisible(on);
    viewToolBar->setVisible(on);
}

void MainWindow::setStatusBarVisible(bool on)
{
    statusBar()->setVisible(on);
}

// ---------------------------------------------------------------- 帮助 ----

void MainWindow::about()
{
    AboutDialog dlg(this);
    dlg.exec();
}

// ---------------------------------------------------------------- 状态 ----

void MainWindow::updateEditActions()
{
    const bool hasSelection = editor->textCursor().hasSelection();
    cutAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);
    pasteAct->setEnabled(editor->canPaste());
    undoAct->setEnabled(editor->document()->isUndoAvailable());
    redoAct->setEnabled(editor->document()->isRedoAvailable());
    selectAllAct->setEnabled(!editor->document()->isEmpty());
}

void MainWindow::updateStatusBar()
{
    const QTextCursor cursor = editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int col = cursor.positionInBlock() + 1;
    positionLabel->setText(QStringLiteral("行 %1，列 %2").arg(line).arg(col));

    const QString text = editor->toPlainText();
    const int charCount = text.size();
    const int lineCount = editor->document()->blockCount();
    const int wordCount = text.isEmpty()
            ? 0
            : text.split(QRegularExpression(QStringLiteral("\\s+")),
                         Qt::SkipEmptyParts).size();
    const QString state = editor->document()->isModified()
            ? QStringLiteral("已修改") : QStringLiteral("未修改");

    infoLabel->setText(QStringLiteral("字符 %1 | 行 %2 | 单词 %3 | %4")
                           .arg(charCount).arg(lineCount).arg(wordCount).arg(state));
}

void MainWindow::updateWindowTitle()
{
    const QString name = isUntitled
            ? QStringLiteral("未命名")
            : QFileInfo(curFile).fileName();
    const QString modified = editor->document()->isModified()
            ? QStringLiteral("*") : QString();
    setWindowTitle(QStringLiteral("%1%2 - 简单文本编辑器").arg(name, modified));
}

void MainWindow::autoSave()
{
    if (isUntitled || !editor->document()->isModified())
        return;

    if (saveToFile(curFile)) {
        // saveToFile 内部已调用 setCurrentFile，会把 modified 置为 false；
        // 这里额外提示"自动保存"。
        statusBar()->showMessage(QStringLiteral("已自动保存：%1").arg(curFile), 3000);
    }
}

// ---------------------------------------------------------------- 关闭 ----

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

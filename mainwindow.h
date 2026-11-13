#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CodeEditor;
class QAction;
class QLabel;
class QComboBox;
class QFontComboBox;
class QTimer;

/**
 * 主窗口：菜单栏 + 多个分组工具栏 + 状态栏 + 中央行号编辑器。
 * 所有菜单/工具栏条目均由 QAction（等价于 Qt Designer 的 Action 编辑器）创建，
 * 并按"文件 / 编辑 / 格式 / 视图 / 帮助"分组。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // ---- 文件 ----
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    // ---- 编辑 ----
    void find();
    void replace();

    // ---- 格式 ----
    void setFontDialog();
    void setFontColor();
    void setBackgroundColor();
    void applyFontFamily(const QString &family);
    void applyFontSize(const QString &sizeText);

    // ---- 视图 ----
    void setLineWrap(bool on);
    void setLineNumberVisible(bool on);
    void setToolBarVisible(bool on);
    void setStatusBarVisible(bool on);

    // ---- 帮助 ----
    void about();

    // ---- 内部状态 ----
    void updateEditActions();
    void updateStatusBar();
    void updateWindowTitle();
    void autoSave();

private:
    void createActions();   // 等价于 Action 编辑器
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createAutoSaveTimer();

    bool maybeSave();
    bool loadFile(const QString &fileName);
    bool saveToFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);

    // ---- 中央控件 ----
    CodeEditor *editor;

    // ---- 工具栏控件 ----
    QFontComboBox *fontCombo;
    QComboBox *fontSizeCombo;

    // ---- 状态栏控件 ----
    QLabel *positionLabel;   // 行、列
    QLabel *infoLabel;       // 字符数 / 行数 / 状态

    // ---- 文件状态 ----
    QString curFile;
    bool isUntitled;

    // ---- 自动保存 ----
    QTimer *autoSaveTimer;

    // ---- Actions（文件） ----
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;

    // ---- Actions（编辑） ----
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *selectAllAct;
    QAction *findAct;
    QAction *replaceAct;

    // ---- Actions（格式） ----
    QAction *fontDialogAct;
    QAction *fontColorAct;
    QAction *bgColorAct;

    // ---- Actions（视图） ----
    QAction *lineWrapAct;
    QAction *lineNumberAct;
    QAction *toolBarAct;
    QAction *statusBarAct;

    // ---- Actions（帮助） ----
    QAction *aboutAct;
    QAction *aboutQtAct;

    // ---- 菜单 / 工具栏 ----
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *formatMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *formatToolBar;
    QToolBar *viewToolBar;
};

#endif // MAINWINDOW_H

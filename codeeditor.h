#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class LineNumberArea;

/**
 * 从 QPlainTextEdit 派生的编辑器，左侧支持行号显示。
 * 参考 Qt 官方示例：Code Editor Example。
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    /** 绘制行号区域（由 LineNumberArea 调用） */
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    /** 计算行号区域宽度（随行数位数变化） */
    int lineNumberAreaWidth() const;

    bool lineNumberAreaVisible() const { return m_lineNumberAreaVisible; }
    void setLineNumberAreaVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    QWidget *lineNumberArea;
    bool m_lineNumberAreaVisible;
};

/** 行号区域控件 */
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor)
        : QWidget(editor), codeEditor(editor)
    {
    }

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H

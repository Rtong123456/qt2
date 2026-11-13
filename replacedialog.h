#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class QLineEdit;
class QCheckBox;
class QPushButton;

/**
 * 替换对话框：支持查找下一个、替换当前、全部替换、
 * 区分大小写、"未找到"提示。
 */
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    /** 预填查找内容（例如主窗口选中的文本） */
    void setFindText(const QString &text);

private slots:
    void findNext();
    void replaceOne();
    void replaceAll();
    void enableButtons(const QString &text);

private:
    QPlainTextEdit *editor;

    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QCheckBox *caseCheckBox;
    QPushButton *findButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
};

#endif // REPLACEDIALOG_H

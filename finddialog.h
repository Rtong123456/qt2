#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;

/**
 * 查找对话框：支持区分大小写、向上/向下查找、
 * 循环查找、"未找到"提示。
 */
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    /** 预填查找内容（例如主窗口选中的文本） */
    void setFindText(const QString &text);

private slots:
    void findNext();
    void enableFindButton(const QString &text);

private:
    void findText(const QString &text, bool backward);

    QPlainTextEdit *editor;

    QLineEdit *lineEdit;
    QCheckBox *caseCheckBox;
    QCheckBox *backwardCheckBox;
    QPushButton *findButton;
    QLabel *hintLabel;
};

#endif // FINDDIALOG_H

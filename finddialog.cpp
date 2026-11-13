#include "finddialog.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

FindDialog::FindDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent), editor(editor)
{
    setWindowTitle(QStringLiteral("查找"));
    setModal(true); // 模态对话框，阻塞主窗口

    QLabel *findLabel = new QLabel(QStringLiteral("查找内容(N)："), this);
    lineEdit = new QLineEdit(this);
    lineEdit->setMinimumWidth(220);
    findLabel->setBuddy(lineEdit);

    caseCheckBox = new QCheckBox(QStringLiteral("区分大小写(C)"), this);
    backwardCheckBox = new QCheckBox(QStringLiteral("向上查找(U)"), this);

    findButton = new QPushButton(QStringLiteral("查找下一个(F)"), this);
    findButton->setDefault(true);
    findButton->setEnabled(false);
    QPushButton *closeButton = new QPushButton(QStringLiteral("关闭"), this);

    hintLabel = new QLabel(this);
    hintLabel->setStyleSheet(QStringLiteral("color: #666;"));
    hintLabel->setVisible(false);

    connect(lineEdit, &QLineEdit::textChanged,
            this, &FindDialog::enableFindButton);
    connect(findButton, &QPushButton::clicked,
            this, &FindDialog::findNext);
    connect(closeButton, &QPushButton::clicked,
            this, &FindDialog::close);

    // 布局
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(findLabel);
    inputLayout->addWidget(lineEdit);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(findButton);
    rightLayout->addWidget(closeButton);
    rightLayout->addStretch();

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(inputLayout);
    leftLayout->addWidget(caseCheckBox);
    leftLayout->addWidget(backwardCheckBox);
    leftLayout->addWidget(hintLabel);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    lineEdit->setFocus();
}

void FindDialog::enableFindButton(const QString &text)
{
    findButton->setEnabled(!text.isEmpty());
}

void FindDialog::setFindText(const QString &text)
{
    lineEdit->setText(text);
    lineEdit->selectAll();
    lineEdit->setFocus();
}

void FindDialog::findNext()
{
    findText(lineEdit->text(), backwardCheckBox->isChecked());
}

void FindDialog::findText(const QString &text, bool backward)
{
    if (text.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    QTextCursor cursor = editor->textCursor();
    int position = backward ? cursor.selectionStart() : cursor.selectionEnd();

    QTextCursor result = editor->document()->find(text, position, flags);

    // 未找到时从另一端再找一次，实现循环查找
    if (result.isNull()) {
        int restartPos = backward
                ? editor->document()->characterCount()
                : 0;
        result = editor->document()->find(text, restartPos, flags);
        if (!result.isNull()) {
            hintLabel->setText(backward
                                   ? QStringLiteral("已从文档末尾重新查找")
                                   : QStringLiteral("已从文档开头重新查找"));
            hintLabel->setVisible(true);
        }
    } else {
        hintLabel->setVisible(false);
    }

    if (result.isNull()) {
        hintLabel->setVisible(false);
        QMessageBox::information(this, QStringLiteral("查找"),
                                 QStringLiteral("未找到 \"%1\"").arg(text));
        return;
    }

    editor->setTextCursor(result);
    editor->ensureCursorVisible();
}

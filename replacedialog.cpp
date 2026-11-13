#include "replacedialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QVBoxLayout>

ReplaceDialog::ReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent), editor(editor)
{
    setWindowTitle(QStringLiteral("替换"));
    setModal(true);

    QLabel *findLabel = new QLabel(QStringLiteral("查找内容(N)："), this);
    findLineEdit = new QLineEdit(this);
    findLabel->setBuddy(findLineEdit);

    QLabel *replaceLabel = new QLabel(QStringLiteral("替换为(R)："), this);
    replaceLineEdit = new QLineEdit(this);
    replaceLabel->setBuddy(replaceLineEdit);

    caseCheckBox = new QCheckBox(QStringLiteral("区分大小写(C)"), this);

    findButton = new QPushButton(QStringLiteral("查找下一个(F)"), this);
    findButton->setDefault(true);
    findButton->setEnabled(false);
    replaceButton = new QPushButton(QStringLiteral("替换"), this);
    replaceButton->setEnabled(false);
    replaceAllButton = new QPushButton(QStringLiteral("全部替换"), this);
    replaceAllButton->setEnabled(false);
    QPushButton *closeButton = new QPushButton(QStringLiteral("关闭"), this);

    connect(findLineEdit, &QLineEdit::textChanged,
            this, &ReplaceDialog::enableButtons);
    connect(findButton, &QPushButton::clicked,
            this, &ReplaceDialog::findNext);
    connect(replaceButton, &QPushButton::clicked,
            this, &ReplaceDialog::replaceOne);
    connect(replaceAllButton, &QPushButton::clicked,
            this, &ReplaceDialog::replaceAll);
    connect(closeButton, &QPushButton::clicked,
            this, &ReplaceDialog::close);

    // 布局：左侧输入区 + 右侧按钮区
    QGridLayout *inputLayout = new QGridLayout;
    inputLayout->addWidget(findLabel, 0, 0);
    inputLayout->addWidget(findLineEdit, 0, 1);
    inputLayout->addWidget(replaceLabel, 1, 0);
    inputLayout->addWidget(replaceLineEdit, 1, 1);
    inputLayout->addWidget(caseCheckBox, 2, 0, 1, 2);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(findButton);
    rightLayout->addWidget(replaceButton);
    rightLayout->addWidget(replaceAllButton);
    rightLayout->addWidget(closeButton);
    rightLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(rightLayout);

    findLineEdit->setFocus();
}

void ReplaceDialog::enableButtons(const QString &text)
{
    bool enable = !text.isEmpty();
    findButton->setEnabled(enable);
    replaceButton->setEnabled(enable);
    replaceAllButton->setEnabled(enable);
}

void ReplaceDialog::setFindText(const QString &text)
{
    findLineEdit->setText(text);
    findLineEdit->selectAll();
    findLineEdit->setFocus();
}

void ReplaceDialog::findNext()
{
    const QString text = findLineEdit->text();
    if (text.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor cursor = editor->textCursor();
    QTextCursor result = editor->document()->find(text, cursor.selectionEnd(), flags);

    if (result.isNull())
        result = editor->document()->find(text, 0, flags); // 从头循环

    if (result.isNull()) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("未找到 \"%1\"").arg(text));
        return;
    }

    editor->setTextCursor(result);
    editor->ensureCursorVisible();
}

void ReplaceDialog::replaceOne()
{
    const QString findText = findLineEdit->text();
    const QString replaceText = replaceLineEdit->text();
    if (findText.isEmpty())
        return;

    QTextCursor cursor = editor->textCursor();

    // 仅当当前选区内容与查找内容一致时才替换
    if (cursor.hasSelection() && cursor.selectedText() == findText) {
        cursor.insertText(replaceText);
    }

    // 继续查找下一个
    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor result = editor->document()->find(findText, cursor.position(), flags);
    if (result.isNull())
        result = editor->document()->find(findText, 0, flags);

    if (result.isNull()) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("未找到 \"%1\"").arg(findText));
        return;
    }

    editor->setTextCursor(result);
    editor->ensureCursorVisible();
}

void ReplaceDialog::replaceAll()
{
    const QString findText = findLineEdit->text();
    const QString replaceText = replaceLineEdit->text();
    if (findText.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (caseCheckBox->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;
    QTextCursor cursor(editor->document());
    cursor.beginEditBlock();

    // 每替换一处后，从替换文本的末尾继续查找，
    // 避免"替换文本本身包含查找词"时重复命中刚写入的内容而陷入死循环。
    int from = 0;
    QTextCursor result = editor->document()->find(findText, from, flags);
    while (!result.isNull()) {
        const int hitStart = result.selectionStart();
        result.insertText(replaceText);
        ++count;
        from = hitStart + replaceText.size();
        result = editor->document()->find(findText, from, flags);
    }

    cursor.endEditBlock();

    if (count == 0) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("未找到 \"%1\"").arg(findText));
    } else {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("已替换 %1 处。").arg(count));
    }
}

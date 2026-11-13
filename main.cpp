#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("TextEditor"));
    QApplication::setApplicationDisplayName(QStringLiteral("简单文本编辑器"));
    QApplication::setOrganizationName(QStringLiteral("DGUT"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));

    // 加载 Qt 自带中文翻译，使标准对话框按钮显示中文
    QTranslator qtTranslator;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QString transPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    const QStringList candidates{QStringLiteral("qtbase_zh_CN"), QStringLiteral("qt_zh_CN")};
#else
    const QString transPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    const QStringList candidates{QStringLiteral("qt_zh_CN"), QStringLiteral("qtbase_zh_CN")};
#endif
    for (const QString &name : candidates) {
        if (qtTranslator.load(name, transPath)) {
            app.installTranslator(&qtTranslator);
            break;
        }
    }

    MainWindow w;
    w.show();

    return app.exec();
}

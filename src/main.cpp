#include <QCoreApplication>
#include <QTextStream>
#include <QApplication>
#include <QStyleFactory>
#include <iostream>
#include <string>
#ifdef _WIN32
extern "C" {
__declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
__declspec(dllimport) int __stdcall SetConsoleCP(unsigned int);
__declspec(dllimport) void* __stdcall GetStdHandle(unsigned long);
__declspec(dllimport) int __stdcall GetConsoleMode(void*, unsigned long*);
__declspec(dllimport) int __stdcall SetConsoleMode(void*, unsigned long);
}
#define CP_UTF8 65001
#define WIN_STD_OUTPUT_HANDLE ((unsigned long)-11)
#define WIN_ENABLE_VTP 0x0004
#endif
#include "mainwindow.h"
#include "filesystem.h"
#include "cli.h"

// Group information - please update with your actual group info
extern const QString GROUP_NAME = "Group 8";
extern const QString GROUP_MEMBERS = "姚文杰 (202330351861), 郑崇 (202330452371), 朱文轩 (202330453241)";
extern const QString GROUP_INFO = "Operating Systems Course Project - Summer 2026";

int runCLI() {
    QTextStream out(stdout);
    QTextStream in(stdin);

    // Colorful welcome banner (soft colors)
    out << "\n";
    out << "\033[90m  ════════════════════════════════════════════════════════════════════════\033[0m\n";
    out << "    \033[37mUnix-Style File System\033[0m  \033[90mv2.0\033[0m\n";
    out << "    \033[90m" << GROUP_INFO << "\033[0m\n";
    out << "\033[90m  ────────────────────────────────────────────────────────────────────────\033[0m\n";
    out << "    \033[34m" << GROUP_NAME << "\033[0m\n";
    out << "    \033[36m" << GROUP_MEMBERS << "\033[0m\n";
    out << "\033[90m  ────────────────────────────────────────────────────────────────────────\033[0m\n";
    out << "    Type '\033[33mhelp\033[0m' for commands, '\033[33mexit\033[0m' to quit\n";
    out << "\033[90m  ════════════════════════════════════════════════════════════════════════\033[0m\n";
    out << "\n";

    FileSystem fs;
    if (!fs.mount("filesystem.dat")) {
        out << "\033[1;31m[ERROR]\033[0m Failed to mount filesystem!\n";
        return 1;
    }

    CLI cli(&fs);

    out << "\033[34m" << fs.get_current_path() << "\033[0m \033[33m$\033[0m ";
    out.flush();

    QString line;
    while (true) {
        line = in.readLine();

        if (line.isNull()) {
            break;
        }

        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            out << "\033[34m" << fs.get_current_path() << "\033[0m \033[33m$\033[0m ";
            out.flush();
            continue;
        }

        QString cmd = trimmed.toLower();

        if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            out << "\n\033[33mSaving filesystem and exiting...\033[0m\n";
            fs.unmount();
            out << "\033[1;32mGoodbye!\033[0m\n";
            break;
        } else if (cmd == "help" || cmd == "?") {
            cli.printHelp();
        } else if (cmd == "dir" || cmd == "ls") {
            cli.handleListDir(QStringList());
        } else if (cmd == "sum" || cmd == "df") {
            cli.handleSum();
        } else if (cmd == "pwd") {
            out << "\033[34m" << fs.get_current_path() << "\033[0m\n";
        } else if (cmd == "clear" || cmd == "cls") {
            out << "\n";
        } else if (cmd.startsWith("createfile ") || cmd.startsWith("create ")) {
            QStringList args = trimmed.mid(cmd.indexOf(' ') + 1).trimmed().split(' ');
            cli.handleCreateFile(args);
        } else if (cmd.startsWith("deletefile ") || cmd.startsWith("delete ")) {
            QStringList args = trimmed.mid(cmd.indexOf(' ') + 1).trimmed().split(' ');
            cli.handleDeleteFile(args);
        } else if (cmd.startsWith("createdir ") || cmd.startsWith("mkdir ")) {
            QStringList args = trimmed.mid(cmd.indexOf(' ') + 1).trimmed().split(' ');
            cli.handleCreateDir(args);
        } else if (cmd.startsWith("deletedir ") || cmd.startsWith("rmdir ")) {
            QStringList args = trimmed.mid(cmd.indexOf(' ') + 1).trimmed().split(' ');
            cli.handleDeleteDir(args);
        } else if (cmd.startsWith("cd ")) {
            QStringList args;
            args << trimmed.mid(3).trimmed();
            cli.handleChangeDir(args);
        } else if (cmd.startsWith("cp ") || cmd.startsWith("copy ")) {
            QString body = trimmed.mid(3).trimmed();
            QStringList parts = body.split(' ');
            if (parts.size() >= 2) {
                QStringList args;
                args << parts[0] << parts.mid(1).join(' ');
                cli.handleCopyFile(args);
            } else {
                cli.printError("Usage: cp <source> <destination>");
            }
        } else if (cmd.startsWith("mv ") || cmd.startsWith("move ")) {
            QString body = trimmed.mid(3).trimmed();
            QStringList parts = body.split(' ');
            if (parts.size() >= 2) {
                QStringList args;
                args << parts[0] << parts.mid(1).join(' ');
                cli.handleMoveFile(args);
            } else {
                cli.printError("Usage: mv <source> <destination>");
            }
        } else if (cmd.startsWith("chmod ")) {
            QString body = trimmed.mid(6).trimmed();
            QStringList parts = body.split(' ');
            if (parts.size() >= 2) {
                QStringList args;
                args << parts[0] << parts[1];
                cli.handleChmod(args);
            } else {
                cli.printError("Usage: chmod <path> <permissions(octal)>");
            }
        } else if (cmd.startsWith("cat ") || cmd.startsWith("type ")) {
            QStringList args;
            args << trimmed.mid(4).trimmed();
            cli.handleCat(args);
        } else {
            out << "\033[1;31m[ERROR]\033[0m Unknown command: \033[33m" << trimmed << "\033[0m\n";
            out << "Type '\033[33mhelp\033[0m' for available commands.\n";
        }

        out << "\n\033[34m" << fs.get_current_path() << "\033[0m \033[33m$\033[0m ";
        out.flush();
    }

    return 0;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Enable ANSI escape codes on Windows 10+
    void* hOut = GetStdHandle(WIN_STD_OUTPUT_HANDLE);
    unsigned long mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | WIN_ENABLE_VTP);
#endif
    QCoreApplication::setApplicationName("Unix File System");
    QCoreApplication::setApplicationVersion("2.0");

    bool useCli = false;
    for (int i = 1; i < argc; i++) {
        QString arg(argv[i]);
        if (arg == "--cli" || arg == "-c" || arg == "/cli") {
            useCli = true;
            break;
        }
    }

    if (useCli) {
        QCoreApplication app(argc, argv);
        return runCLI();
    } else {
        QApplication app(argc, argv);

        app.setApplicationName("Unix File System Explorer");
        app.setApplicationVersion("2.0");
        app.setOrganizationName("OS Course Project");
        app.setOrganizationDomain("example.com");

        app.setStyle(QStyleFactory::create("Fusion"));

        MainWindow window;
        window.show();

        return app.exec();
    }
}

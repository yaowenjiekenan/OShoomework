#include <QCoreApplication>
#include <QTextStream>
#include <QApplication>
#include <QStyleFactory>
#include <iostream>
#include <string>
#include "mainwindow.h"
#include "filesystem.h"
#include "cli.h"

// Group information - please update with your actual group info
const QString GROUP_NAME = "Group 1";
const QString GROUP_MEMBERS = "Student Name 1 (ID: 12345678), Student Name 2 (ID: 23456789)";
const QString GROUP_INFO = "Operating Systems Course Project - Spring 2026";

int runCLI() {
    QTextStream out(stdout);
    QTextStream in(stdin);

    out << "\n";
    out << "==================================================\n";
    out << "   Welcome to Unix-Style File System\n";
    out << "   " << qPrintable(GROUP_INFO) << "\n";
    out << "==================================================\n";
    out << "\n";
    out << "   " << qPrintable(GROUP_NAME) << "\n";
    out << "   " << qPrintable(GROUP_MEMBERS) << "\n";
    out << "\n";
    out << "--------------------------------------------------\n";
    out << "   Type 'help' for available commands\n";
    out << "   Type 'exit' to quit\n";
    out << "--------------------------------------------------\n";
    out << "\n";

    FileSystem fs;
    QDir appDir = QCoreApplication::applicationDirPath();
    if (!fs.mount(appDir.filePath("filesystem.dat"))) {
        out << "[ERROR] Failed to mount filesystem!\n";
        return 1;
    }

    CLI cli(&fs);

    out << fs.get_current_path() << " $ ";
    out.flush();

    QString line;
    while (true) {
        line = in.readLine();

        if (line.isNull()) {
            break;
        }

        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            out << fs.get_current_path() << " $ ";
            out.flush();
            continue;
        }

        QString cmd = trimmed.toLower();

        if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            out << "\nSaving filesystem and exiting...\n";
            fs.unmount();
            out << "Goodbye!\n";
            break;
        } else if (cmd == "help" || cmd == "?") {
            cli.printHelp();
        } else if (cmd == "dir" || cmd == "ls") {
            cli.handleListDir(QStringList());
        } else if (cmd == "sum" || cmd == "df") {
            cli.handleSum();
        } else if (cmd == "pwd") {
            out << fs.get_current_path() << "\n";
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
            out << "[ERROR] Unknown command: " << trimmed << "\n";
            out << "Type 'help' for available commands.\n";
        }

        out << "\n" << fs.get_current_path() << " $ ";
        out.flush();
    }

    return 0;
}

int main(int argc, char *argv[]) {
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

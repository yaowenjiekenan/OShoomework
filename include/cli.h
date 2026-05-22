#ifndef CLI_H
#define CLI_H

#include <QObject>
#include <QTextStream>
#include <QString>

class FileSystem;

class CLI : public QObject {
    Q_OBJECT

public:
    explicit CLI(FileSystem* fs, QObject* parent = nullptr);
    ~CLI();

    void printHelp();
    void handleCreateFile(const QStringList& args);
    void handleDeleteFile(const QStringList& args);
    void handleCreateDir(const QStringList& args);
    void handleDeleteDir(const QStringList& args);
    void handleChangeDir(const QStringList& args);
    void handleListDir(const QStringList& args);
    void handleCopyFile(const QStringList& args);
    void handleMoveFile(const QStringList& args);
    void handleChmod(const QStringList& args);
    void handleCat(const QStringList& args);
    void handleSum();

    void printError(const QString& msg);
    void printSuccess(const QString& msg);
    QString formatSize(uint32_t size) const;
    QString formatTime(time_t t) const;

private:
    FileSystem* m_fileSystem;
};

#endif // CLI_H

#include "cli.h"
#include "filesystem.h"
#include <QTextStream>

CLI::CLI(FileSystem* fs, QObject* parent)
    : QObject(parent)
    , m_fileSystem(fs)
{
}

CLI::~CLI() {
}

void CLI::printError(const QString& msg) {
    QTextStream out(stdout);
    out << "[ERROR] " << msg << "\n";
    out.flush();
}

void CLI::printSuccess(const QString& msg) {
    QTextStream out(stdout);
    out << "[OK] " << msg << "\n";
    out.flush();
}

QString CLI::formatSize(uint32_t size) const {
    if (size < 1024) {
        return QString("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        return QString("%1 KB").arg(size / 1024);
    } else {
        return QString("%1 MB").arg(size / (1024.0 * 1024), 0, 'f', 2);
    }
}

QString CLI::formatTime(time_t t) const {
    struct tm* tm_info = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm_info);
    return QString(buf);
}

void CLI::printHelp() {
    QTextStream out(stdout);
    out << "\nAvailable Commands:\n";
    out << "-------------------\n";
    out << "  createFile <path> <size(KB)>  Create a new file\n";
    out << "  deleteFile <path>             Delete a file\n";
    out << "  createDir <path>              Create a directory\n";
    out << "  deleteDir [-r] <path>         Delete a directory (-r for recursive)\n";
    out << "  changeDir <path>              Change current directory\n";
    out << "  dir                           List directory contents\n";
    out << "  cp <src> <dst>               Copy a file or directory\n";
    out << "  mv <src> <dst>               Move/rename a file or directory\n";
    out << "  cat <path>                    Display file contents\n";
    out << "  chmod <path> <permissions>    Change file permissions (octal)\n";
    out << "  sum                           Show storage usage\n";
    out << "  clear                         Clear screen\n";
    out << "  help                          Show this help message\n";
    out << "  exit                          Exit program\n";
    out << "\nShortcuts:\n";
    out << "  cd = changeDir, ls/dir = dir\n";
    out << "  mkdir = createDir, rm = deleteDir\n";
    out << "  cp = copy, mv = move\n";
    out << "  df = sum\n";
    out << "\nExamples:\n";
    out << "  createFile /dir1/myFile 10\n";
    out << "  createDir /home/user/docs\n";
    out << "  changeDir /home/user\n";
    out << "  cp /file1 /backup/file1\n";
    out << "  mv /oldname /newname\n";
    out << "  deleteDir -r /tmp/junk\n";
    out << "  chmod /myfile 644\n";
    out << "  cat /etc/config.conf\n";
    out << "\n";
    out.flush();
}

void CLI::handleCreateFile(const QStringList& args) {
    QTextStream out(stdout);

    if (args.size() < 2) {
        printError("Usage: createFile <path> <size(KB)>");
        return;
    }

    QString path = args[0];
    bool ok;
    uint32_t size = args[1].toUInt(&ok);
    if (!ok || size == 0) {
        printError("Invalid size. Please specify size in KB (positive integer).");
        return;
    }

    // Max size with double indirect: 10 + 256 + 65536 = 65802 blocks = 64.26 MB
    uint32_t maxSize = (DIRECT_BLOCKS + POINTERS_PER_BLOCK + POINTERS_PER_BLOCK * POINTERS_PER_BLOCK);
    if (size > maxSize) {
        out << "[ERROR] File size exceeds maximum allowed size (" << maxSize << " KB = " << (maxSize/1024) << " MB).\n";
        out.flush();
        return;
    }

    if (m_fileSystem->create_file(path, size)) {
        printSuccess(QString("File created: %1 (%2 KB)").arg(path).arg(size));
    } else {
        printError(QString("Failed to create file: %1").arg(path));
    }
}

void CLI::handleDeleteFile(const QStringList& args) {
    if (args.size() < 1) {
        printError("Usage: deleteFile <path>");
        return;
    }

    QString path = args[0];
    if (m_fileSystem->delete_file(path)) {
        printSuccess(QString("File deleted: %1").arg(path));
    } else {
        printError(QString("Failed to delete file: %1").arg(path));
    }
}

void CLI::handleCreateDir(const QStringList& args) {
    if (args.size() < 1) {
        printError("Usage: createDir <path>");
        return;
    }

    QString path = args[0];
    if (m_fileSystem->create_directory(path)) {
        printSuccess(QString("Directory created: %1").arg(path));
    } else {
        printError(QString("Failed to create directory: %1").arg(path));
    }
}

void CLI::handleDeleteDir(const QStringList& args) {
    QTextStream out(stdout);

    if (args.isEmpty()) {
        printError("Usage: deleteDir [-r] <path>");
        return;
    }

    QString path = args.last();
    bool recursive = false;

    if (args.size() >= 2 && args.first() == "-r") {
        recursive = true;
        path = args.last();
    }

    if (path == "/" || path.isEmpty()) {
        printError("Cannot delete root directory.");
        return;
    }

    if (path == m_fileSystem->get_current_path()) {
        printError("Cannot delete current working directory.");
        return;
    }

    bool success;
    if (recursive) {
        success = m_fileSystem->delete_directory_recursive(path);
    } else {
        success = m_fileSystem->delete_directory(path);
    }

    if (success) {
        printSuccess(QString("Directory deleted: %1").arg(path));
    } else {
        printError(QString("Failed to delete directory: %1").arg(path));
    }
}

void CLI::handleChangeDir(const QStringList& args) {
    if (args.size() < 1) {
        printError("Usage: changeDir <path>");
        return;
    }

    QString path = args[0];
    if (m_fileSystem->change_directory(path)) {
        printSuccess(QString("Changed to: %1").arg(m_fileSystem->get_current_path()));
    } else {
        printError(QString("Failed to change directory: %1").arg(path));
    }
}

void CLI::handleListDir(const QStringList& args) {
    QTextStream out(stdout);
    Q_UNUSED(args)

    auto entries = m_fileSystem->get_directory_entries(m_fileSystem->current_inode);

    if (entries.empty()) {
        out << "\n  (empty directory)\n";
        out.flush();
        return;
    }

    out << "\n";
    out << QString("%1/\n").arg(m_fileSystem->get_current_path());
    out << QString().leftJustified(60, '-') << "\n";

    int files = 0, dirs = 0;

    for (const auto& e : entries) {
        std::string name(e.filename);
        if (name == "." || name == "..") continue;

        const INode& inode = m_fileSystem->inode_table[e.inode_id];
        QString type = inode.file_type == FileType::DIRECTORY ? "DIR " : "FILE";
        QString size = inode.file_type == FileType::DIRECTORY ? "     -" : formatSize(inode.file_size);
        QString time = formatTime(inode.modified_time);
        // Format permissions from actual inode value
        QString perms;
        perms += inode.file_type == FileType::DIRECTORY ? 'd' : '-';
        perms += (inode.permissions & 0x0100) ? 'r' : '-';
        perms += (inode.permissions & 0x0080) ? 'w' : '-';
        perms += (inode.permissions & 0x0040) ? 'x' : '-';
        perms += (inode.permissions & 0x0020) ? 'r' : '-';
        perms += (inode.permissions & 0x0010) ? 'w' : '-';
        perms += (inode.permissions & 0x0008) ? 'x' : '-';
        perms += (inode.permissions & 0x0004) ? 'r' : '-';
        perms += (inode.permissions & 0x0002) ? 'w' : '-';
        perms += (inode.permissions & 0x0001) ? 'x' : '-';

        out << QString("  %1 %2 %3 %4\n")
                   .arg(perms, 11)
                   .arg(size, 10)
                   .arg(time, 18)
                   .arg(QString::fromStdString(name));

        if (inode.file_type == FileType::DIRECTORY) {
            dirs++;
        } else {
            files++;
        }
    }

    out << QString().leftJustified(60, '-') << "\n";
    out << QString("  %1 file(s), %2 dir(s)\n").arg(files).arg(dirs);
    out.flush();
}

void CLI::handleCopyFile(const QStringList& args) {
    if (args.size() < 2) {
        printError("Usage: cp <source_path> <destination_path>");
        return;
    }

    QString src = args[0];
    QString dst = args[1];

    // Check if source is a directory
    auto [srcIn, _] = m_fileSystem->resolve_path(src);
    if (srcIn != UINT32_MAX && m_fileSystem->inode_table[srcIn].file_type == FileType::DIRECTORY) {
        if (m_fileSystem->copy_directory_recursive(src, dst)) {
            printSuccess(QString("Directory copied: %1 -> %2").arg(src).arg(dst));
        } else {
            printError(QString("Failed to copy directory: %1 -> %2").arg(src).arg(dst));
        }
        return;
    }

    if (m_fileSystem->copy_file(src, dst)) {
        printSuccess(QString("File copied: %1 -> %2").arg(src).arg(dst));
    } else {
        printError(QString("Failed to copy file: %1 -> %2").arg(src).arg(dst));
    }
}

void CLI::handleMoveFile(const QStringList& args) {
    if (args.size() < 2) {
        printError("Usage: mv <source_path> <destination_path>");
        return;
    }

    QString src = args[0];
    QString dst = args[1];

    if (m_fileSystem->move_item(src, dst)) {
        printSuccess(QString("Moved: %1 -> %2").arg(src).arg(dst));
    } else {
        printError(QString("Failed to move: %1 -> %2").arg(src).arg(dst));
    }
}

void CLI::handleChmod(const QStringList& args) {
    QTextStream out(stdout);

    if (args.size() < 2) {
        printError("Usage: chmod <path> <permissions(octal)>");
        out << "Examples: chmod /myfile 644, chmod /mydir 755\n";
        out.flush();
        return;
    }

    QString path = args[0];
    QString permStr = args[1];

    bool ok;
    uint16_t permissions = permStr.toUInt(&ok, 8);
    if (!ok) {
        printError("Invalid permissions. Use octal format (e.g., 644, 755, 777)");
        return;
    }

    if (m_fileSystem->change_permissions(path, permissions)) {
        printSuccess(QString("Permissions changed for %1 to %2").arg(path).arg(permStr));
    } else {
        printError(QString("Failed to change permissions: %1").arg(path));
    }
}

void CLI::handleCat(const QStringList& args) {
    QTextStream out(stdout);

    if (args.size() < 1) {
        printError("Usage: cat <file_path>");
        return;
    }

    QString path = args[0];
    auto result = m_fileSystem->resolve_path(path);
    uint32_t inode = result.first;

    if (inode == UINT32_MAX) {
        printError(QString("File not found: %1").arg(path));
        return;
    }

    const INode& fileInode = m_fileSystem->inode_table[inode];
    if (fileInode.file_type != FileType::REGULAR) {
        printError(QString("Not a regular file: %1").arg(path));
        return;
    }

    out << "\n[File: " << path << "]\n";
    out << "[Size: " << formatSize(fileInode.file_size) << "]\n";
    out << "[Blocks: " << fileInode.block_count << "]\n";
    out << "[Created: " << formatTime(fileInode.created_time) << "]\n";
    out << "[Modified: " << formatTime(fileInode.modified_time) << "]\n";
    out << QString().leftJustified(40, '-') << "\n\n";

    // Read and display file content (first 4KB as preview)
    uint32_t displaySize = qMin(fileInode.file_size, (uint32_t)4096);
    QByteArray content;
    content.reserve(displaySize);

    const uint8_t* data = m_fileSystem->get_disk_data().data();

    for (uint32_t i = 0; i < fileInode.block_count && (uint32_t)content.size() < displaySize; i++) {
        uint32_t blockNum = 0;

        if (i < DIRECT_BLOCKS) {
            // Direct block
            blockNum = fileInode.direct_blocks[i];
        } else if (i < DIRECT_BLOCKS + POINTERS_PER_BLOCK) {
            // Single indirect block
            if (fileInode.indirect_block != 0) {
                const uint32_t* indirectPtr = (const uint32_t*)(data + fileInode.indirect_block * BLOCK_SIZE);
                blockNum = indirectPtr[i - DIRECT_BLOCKS];
            }
        } else {
            // Double indirect block
            if (fileInode.double_indirect_block != 0) {
                const uint32_t* doublePtr = (const uint32_t*)(data + fileInode.double_indirect_block * BLOCK_SIZE);
                uint32_t doubleIndex = (i - DIRECT_BLOCKS - POINTERS_PER_BLOCK) / POINTERS_PER_BLOCK;
                uint32_t innerIndex = (i - DIRECT_BLOCKS - POINTERS_PER_BLOCK) % POINTERS_PER_BLOCK;

                if (doublePtr[doubleIndex] != 0) {
                    const uint32_t* innerPtr = (const uint32_t*)(data + doublePtr[doubleIndex] * BLOCK_SIZE);
                    blockNum = innerPtr[innerIndex];
                }
            }
        }

        if (blockNum == 0) continue;

        const uint8_t* blockData = data + blockNum * BLOCK_SIZE;
        uint32_t blockSize = qMin<uint32_t>(BLOCK_SIZE, displaySize - (uint32_t)content.size());

        content.append((const char*)blockData, blockSize);
    }

    // Display content
    out << "Content preview (first 4KB):\n\n";
    out << content.left(1024).constData() << "\n";
    out << "\n";

    if (fileInode.file_size > 4096) {
        out << "... (file continues, " << formatSize(fileInode.file_size - 4096) << " more)\n";
    }

    out.flush();
}

void CLI::handleSum() {
    QTextStream out(stdout);

    uint32_t totalBlocks = TOTAL_BLOCKS;
    uint32_t usedBlocks = m_fileSystem->get_bitmap()->get_used_count();
    uint32_t freeBlocks = m_fileSystem->get_bitmap()->get_free_count();

    double usedPercent = 100.0 * usedBlocks / totalBlocks;
    double freePercent = 100.0 * freeBlocks / totalBlocks;

    out << "\n";
    out << "========================================\n";
    out << "        Storage Usage Report\n";
    out << "========================================\n";
    out << "\n";
    out << QString("  Total Space:     %1 MB (%2 blocks)\n")
                   .arg(TOTAL_SIZE / (1024.0 * 1024), 0, 'f', 2)
                   .arg(totalBlocks);
    out << QString("  Block Size:     %1 bytes\n").arg(BLOCK_SIZE);
    out << "\n";
    out << QString("  Used Blocks:    %1 (%2%)\n").arg(usedBlocks).arg(usedPercent, 0, 'f', 1);
    out << QString("  Free Blocks:    %1 (%2%)\n").arg(freeBlocks).arg(freePercent, 0, 'f', 1);
    out << "\n";

    // Simple bar chart
    int barWidth = 40;
    int usedBars = (int)(usedPercent * barWidth / 100);
    int freeBars = barWidth - usedBars;

    out << "  [";
    for (int i = 0; i < usedBars; i++) out << "#";
    for (int i = 0; i < freeBars; i++) out << "-";
    out << "]\n";
    out << "\n";
    out << "  Used: #  Free: -\n";
    out << "\n";
    out << "========================================\n";
    out.flush();
}

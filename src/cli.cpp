#include "cli.h"
#include "filesystem.h"
#include <QTextStream>

// ANSI Color Codes (soft palette)
#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_WHITE   "\033[37m"
#define C_BRED    "\033[31m"
#define C_BGREEN  "\033[32m"
#define C_BYELLOW "\033[33m"
#define C_BBLUE   "\033[34m"
#define C_BCYAN   "\033[36m"
#define C_GRAY    "\033[90m"

CLI::CLI(FileSystem* fs, QObject* parent)
    : QObject(parent)
    , m_fileSystem(fs)
{
}

CLI::~CLI() {
}

void CLI::printError(const QString& msg) {
    QTextStream out(stdout);
    out << C_BRED " [ERROR] " C_RESET C_RED << msg << C_RESET "\n";
    out.flush();
}

void CLI::printSuccess(const QString& msg) {
    QTextStream out(stdout);
    out << C_BGREEN " [OK] " C_RESET C_GREEN << msg << C_RESET "\n";
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
    out << "\n";
    out << C_BYELLOW " Available Commands" C_RESET "\n";
    out << C_GRAY " -------------------" C_RESET "\n";
    out << "  " C_BCYAN "createFile" C_RESET " <path> <size(KB)>  " C_GRAY "Create a new file" C_RESET "\n";
    out << "  " C_BCYAN "deleteFile" C_RESET " <path>             " C_GRAY "Delete a file" C_RESET "\n";
    out << "  " C_BCYAN "createDir" C_RESET "  <path>              " C_GRAY "Create a directory" C_RESET "\n";
    out << "  " C_BCYAN "deleteDir" C_RESET "  [-r] <path>         " C_GRAY "Delete a directory (-r for recursive)" C_RESET "\n";
    out << "  " C_BCYAN "changeDir" C_RESET "  <path>              " C_GRAY "Change current directory" C_RESET "\n";
    out << "  " C_BCYAN "dir" C_RESET "                           " C_GRAY "List directory contents" C_RESET "\n";
    out << "  " C_BCYAN "cp" C_RESET "  <src> <dst>               " C_GRAY "Copy a file or directory" C_RESET "\n";
    out << "  " C_BCYAN "mv" C_RESET "  <src> <dst>               " C_GRAY "Move/rename a file or directory" C_RESET "\n";
    out << "  " C_BCYAN "cat" C_RESET " <path>                    " C_GRAY "Display file contents" C_RESET "\n";
    out << "  " C_BCYAN "chmod" C_RESET " <path> <permissions>    " C_GRAY "Change file permissions (octal)" C_RESET "\n";
    out << "  " C_BCYAN "sum" C_RESET "                           " C_GRAY "Show storage usage" C_RESET "\n";
    out << "  " C_BCYAN "clear" C_RESET "                         " C_GRAY "Clear screen" C_RESET "\n";
    out << "  " C_BCYAN "help" C_RESET "                          " C_GRAY "Show this help message" C_RESET "\n";
    out << "  " C_BCYAN "exit" C_RESET "                          " C_GRAY "Exit program" C_RESET "\n";
    out << "\n" C_BYELLOW " Shortcuts" C_RESET "\n";
    out << C_GRAY "  cd = changeDir, ls/dir = dir" C_RESET "\n";
    out << C_GRAY "  mkdir = createDir, rm = deleteDir" C_RESET "\n";
    out << C_GRAY "  cp = copy, mv = move, df = sum" C_RESET "\n";
    out << "\n" C_BYELLOW " Examples" C_RESET "\n";
    out << "  " C_GREEN "createFile /dir1/myFile 10" C_RESET "\n";
    out << "  " C_GREEN "createDir /home/user/docs" C_RESET "\n";
    out << "  " C_GREEN "changeDir /home/user" C_RESET "\n";
    out << "  " C_GREEN "cp /file1 /backup/file1" C_RESET "\n";
    out << "  " C_GREEN "chmod /myfile 644" C_RESET "\n";
    out << "  " C_GREEN "cat /etc/config.conf" C_RESET "\n";
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
        out << C_BRED " [ERROR] " C_RESET C_RED "File size exceeds maximum allowed size (" << maxSize << " KB = " << (maxSize/1024) << " MB)." C_RESET "\n";
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
        out << "\n  " C_GRAY "(empty directory)" C_RESET "\n";
        out.flush();
        return;
    }

    out << "\n";
    out << C_BBLUE << QString("%1/").arg(m_fileSystem->get_current_path()) << C_RESET "\n";
    out << C_GRAY << QString().leftJustified(60, '-') << C_RESET "\n";

    int files = 0, dirs = 0;

    for (const auto& e : entries) {
        std::string name(e.filename);
        if (name == "." || name == "..") continue;

        const INode& inode = m_fileSystem->inode_table[e.inode_id];
        bool isDir = inode.file_type == FileType::DIRECTORY;
        QString size = isDir ? "     -" : formatSize(inode.file_size);
        QString time = formatTime(inode.modified_time);
        // Format permissions with color
        QString perms;
        perms += isDir ? C_BBLUE "d" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0100) ? C_GREEN "r" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0080) ? C_YELLOW "w" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0040) ? C_RED "x" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0020) ? C_GREEN "r" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0010) ? C_YELLOW "w" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0008) ? C_RED "x" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0004) ? C_GREEN "r" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0002) ? C_YELLOW "w" C_RESET : C_GRAY "-" C_RESET;
        perms += (inode.permissions & 0x0001) ? C_RED "x" C_RESET : C_GRAY "-" C_RESET;

        QString nameStr = QString::fromStdString(name);
        QString coloredName = isDir
            ? QString(C_BBLUE "%1/" C_RESET).arg(nameStr)
            : QString(C_WHITE "%1" C_RESET).arg(nameStr);

        out << "  " << perms
            << " " C_CYAN << QString("%1").arg(size, 10) << C_RESET
            << " " C_GRAY << QString("%1").arg(time, 18) << C_RESET
            << " " << coloredName << "\n";

        if (isDir) {
            dirs++;
        } else {
            files++;
        }
    }

    out << C_GRAY << QString().leftJustified(60, '-') << C_RESET "\n";
    out << "  " C_CYAN << files << " file(s)" C_RESET ", " C_BBLUE << dirs << " dir(s)" C_RESET "\n";
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
        out << C_GRAY "Examples: chmod /myfile 644, chmod /mydir 755" C_RESET "\n";
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

    out << "\n" C_BYELLOW "[File: " << path << "]" C_RESET "\n";
    out << C_CYAN "[Size: " << formatSize(fileInode.file_size) << "]" C_RESET "\n";
    out << C_CYAN "[Blocks: " << fileInode.block_count << "]" C_RESET "\n";
    out << C_GRAY "[Created: " << formatTime(fileInode.created_time) << "]" C_RESET "\n";
    out << C_GRAY "[Modified: " << formatTime(fileInode.modified_time) << "]" C_RESET "\n";
    out << C_GRAY << QString().leftJustified(40, '-') << C_RESET "\n\n";

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
    out << C_BYELLOW "Content preview (first 4KB):" C_RESET "\n\n";
    out << C_WHITE << content.left(1024).constData() << C_RESET "\n";
    out << "\n";

    if (fileInode.file_size > 4096) {
        out << C_GRAY "... (file continues, " << formatSize(fileInode.file_size - 4096) << " more)" C_RESET "\n";
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
    out << C_BYELLOW "  ╔════════════════════════════════════════╗" C_RESET "\n";
    out << C_BYELLOW "  ║       Storage Usage Report             ║" C_RESET "\n";
    out << C_BYELLOW "  ╠════════════════════════════════════════╣" C_RESET "\n";
    out << C_BYELLOW "  ║" C_RESET
        << QString("  Total Space:   " C_BCYAN "%1 MB" C_RESET " (%2 blocks)")
                   .arg(TOTAL_SIZE / (1024.0 * 1024), 0, 'f', 2)
                   .arg(totalBlocks);
    out << "\n";
    out << C_BYELLOW "  ║" C_RESET
        << QString("  Block Size:    " C_CYAN "%1 bytes" C_RESET).arg(BLOCK_SIZE);
    out << "\n";
    out << C_BYELLOW "  ║" C_RESET "\n";
    out << C_BYELLOW "  ║" C_RESET
        << QString("  " C_RED "Used Blocks:" C_RESET "   " C_BRED "%1" C_RESET " (%2%)")
                   .arg(usedBlocks).arg(usedPercent, 0, 'f', 1);
    out << "\n";
    out << C_BYELLOW "  ║" C_RESET
        << QString("  " C_GREEN "Free Blocks:" C_RESET "   " C_BGREEN "%1" C_RESET " (%2%)")
                   .arg(freeBlocks).arg(freePercent, 0, 'f', 1);
    out << "\n";
    out << C_BYELLOW "  ║" C_RESET "\n";

    // Colored bar chart
    int barWidth = 36;
    int usedBars = (int)(usedPercent * barWidth / 100);
    int freeBars = barWidth - usedBars;

    out << C_BYELLOW "  ║" C_RESET "  [";
    // Used portion in red/yellow gradient
    for (int i = 0; i < usedBars; i++) {
        if (usedPercent > 80) out << C_BRED;
        else if (usedPercent > 50) out << C_BYELLOW;
        else out << C_BGREEN;
        out << "\xe2\x96\x88";  // Unicode full block character
    }
    // Free portion in dim
    out << C_GRAY;
    for (int i = 0; i < freeBars; i++) out << "\xe2\x96\x91";  // Unicode light shade
    out << C_RESET "]";
    out << "\n";

    out << C_BYELLOW "  ║" C_RESET "  ";
    if (usedPercent > 80) out << C_BRED;
    else if (usedPercent > 50) out << C_BYELLOW;
    else out << C_BGREEN;
    out << "\xe2\x96\x88" C_RESET " Used  ";
    out << C_GRAY "\xe2\x96\x91" C_RESET " Free\n";
    out << C_BYELLOW "  ╚════════════════════════════════════════╝" C_RESET "\n";
    out.flush();
}

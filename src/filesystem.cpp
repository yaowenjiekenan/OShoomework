#include "filesystem.h"
#include <QTextStream>
#include <QDebug>

// ============================================================================
// INode Implementation
// ============================================================================
INode::INode() {
    reset();
}

void INode::reset() {
    inode_id = 0;
    file_type = FileType::REGULAR;
    file_size = 0;
    block_count = 0;
    memset(direct_blocks, 0, sizeof(direct_blocks));
    indirect_block = 0;
    double_indirect_block = 0;
    created_time = 0;
    modified_time = 0;
    accessed_time = 0;
    permissions = DEFAULT_FILE_PERM;
    link_count = 0;
    memset(reserved, 0, sizeof(reserved));
}

bool INode::is_empty() const {
    return inode_id == 0 && file_type == FileType::REGULAR && file_size == 0;
}

void INode::pack(uint8_t* buffer) const {
    uint32_t offset = 0;

    memcpy(buffer + offset, &inode_id, 4); offset += 4;
    buffer[offset] = static_cast<uint8_t>(file_type); offset += 4;
    memcpy(buffer + offset, &file_size, 4); offset += 4;
    memcpy(buffer + offset, &block_count, 4); offset += 4;
    memcpy(buffer + offset, direct_blocks, 40); offset += 40;
    memcpy(buffer + offset, &indirect_block, 4); offset += 4;
    memcpy(buffer + offset, &double_indirect_block, 4); offset += 4;
    memcpy(buffer + offset, &created_time, 8); offset += 8;
    memcpy(buffer + offset, &modified_time, 8); offset += 8;
    memcpy(buffer + offset, &accessed_time, 8); offset += 8;
    memcpy(buffer + offset, &permissions, 2); offset += 2;
    memcpy(buffer + offset, &link_count, 2); offset += 2;
    memcpy(buffer + offset, reserved, 14);
}

INode INode::unpack(const uint8_t* buffer) {
    INode i;
    uint32_t offset = 0;

    memcpy(&i.inode_id, buffer + offset, 4); offset += 4;
    uint8_t fileTypeByte;
    memcpy(&fileTypeByte, buffer + offset, 1);
    i.file_type = static_cast<FileType>(fileTypeByte);
    offset += 4;  // Match pack() alignment - FileType occupies 4 bytes due to struct alignment
    memcpy(&i.file_size, buffer + offset, 4); offset += 4;
    memcpy(&i.block_count, buffer + offset, 4); offset += 4;
    memcpy(i.direct_blocks, buffer + offset, 40); offset += 40;
    memcpy(&i.indirect_block, buffer + offset, 4); offset += 4;
    memcpy(&i.double_indirect_block, buffer + offset, 4); offset += 4;
    memcpy(&i.created_time, buffer + offset, 8); offset += 8;
    memcpy(&i.modified_time, buffer + offset, 8); offset += 8;
    memcpy(&i.accessed_time, buffer + offset, 8); offset += 8;
    memcpy(&i.permissions, buffer + offset, 2); offset += 2;
    memcpy(&i.link_count, buffer + offset, 2); offset += 2;
    memcpy(i.reserved, buffer + offset, 14);

    return i;
}

// ============================================================================
// DirEntry Implementation
// ============================================================================
DirEntry::DirEntry() {
    clear();
}

void DirEntry::clear() {
    inode_id = 0;
    memset(filename, 0, MAX_FILENAME_LEN);
}

void DirEntry::pack(uint8_t* buffer) const {
    memcpy(buffer, &inode_id, 4);
    memcpy(buffer + 4, filename, MAX_FILENAME_LEN);
}

DirEntry DirEntry::unpack(const uint8_t* buffer) {
    DirEntry e;
    memcpy(&e.inode_id, buffer, 4);
    memcpy(e.filename, buffer + 4, MAX_FILENAME_LEN);
    return e;
}

// ============================================================================
// BlockBitmap Implementation
// ============================================================================
BlockBitmap::BlockBitmap() {
    bitmap.resize((TOTAL_BLOCKS + 7) / 8, 0);
}

void BlockBitmap::initialize() {
    std::fill(bitmap.begin(), bitmap.end(), 0);
}

uint32_t BlockBitmap::allocate_block() {
    for (uint32_t i = 0; i < TOTAL_BLOCKS; i++) {
        if (!is_used(i)) {
            set_used(i);
            return i;
        }
    }
    return UINT32_MAX;
}

bool BlockBitmap::free_block(uint32_t b) {
    if (b >= TOTAL_BLOCKS || !is_used(b)) return false;
    set_free(b);
    return true;
}

bool BlockBitmap::is_used(uint32_t b) const {
    if (b >= TOTAL_BLOCKS) return true;
    return (bitmap[b / 8] & (1 << (b % 8))) != 0;
}

uint32_t BlockBitmap::get_free_count() const {
    uint32_t count = 0;
    for (uint32_t i = 0; i < TOTAL_BLOCKS; i++) {
        if (!is_used(i)) count++;
    }
    return count;
}

uint32_t BlockBitmap::get_used_count() const {
    return TOTAL_BLOCKS - get_free_count();
}

void BlockBitmap::set_used(uint32_t b) {
    if (b < TOTAL_BLOCKS) bitmap[b / 8] |= (1 << (b % 8));
}

void BlockBitmap::set_free(uint32_t b) {
    if (b < TOTAL_BLOCKS) bitmap[b / 8] &= ~(1 << (b % 8));
}

// ============================================================================
// Superblock Implementation
// ============================================================================
Superblock::Superblock() {
    magic = 0;
    total_blocks = TOTAL_BLOCKS;
    block_size = BLOCK_SIZE;
    total_inodes = 0;
    free_blocks = 0;
    free_inodes = 0;
    inode_table_start = 1;
    inode_table_blocks = 0;
    data_block_start = 0;
    root_inode = 0;
}

bool Superblock::is_valid() const {
    return magic == FS_MAGIC && total_blocks == TOTAL_BLOCKS;
}

// ============================================================================
// FileSystem Implementation
// ============================================================================
FileSystem::FileSystem() : current_inode(0), current_path("/") {
    disk_data.resize(TOTAL_SIZE, 0);
}

FileSystem::~FileSystem() {
    unmount();
}

uint8_t* FileSystem::get_block_ptr(uint32_t b) {
    return disk_data.data() + b * BLOCK_SIZE;
}

const uint8_t* FileSystem::get_block_ptr(uint32_t b) const {
    return disk_data.data() + b * BLOCK_SIZE;
}

uint32_t FileSystem::allocate_block_internal() {
    for (uint32_t i = 0; i < TOTAL_BLOCKS; i++) {
        if (!bitmap.is_used(i)) {
            bitmap.set_used(i);
            memset(get_block_ptr(i), 0, BLOCK_SIZE);
            return i;
        }
    }
    return UINT32_MAX;
}

bool FileSystem::free_block_internal(uint32_t b) {
    return bitmap.free_block(b);
}

uint32_t FileSystem::allocate_inode() {
    for (uint32_t i = 0; i < inode_table.size(); i++) {
        if (inode_table[i].is_empty()) {
            inode_table[i].reset();
            inode_table[i].inode_id = i;
            inode_table[i].created_time = time(nullptr);
            superblock.free_inodes--;
            return i;
        }
    }
    return UINT32_MAX;
}

bool FileSystem::free_inode(uint32_t n) {
    if (n >= inode_table.size() || inode_table[n].is_empty()) return false;

    INode& i = inode_table[n];

    // Free direct blocks
    for (uint32_t j = 0; j < DIRECT_BLOCKS; j++) {
        if (i.direct_blocks[j] != 0) {
            free_block_internal(i.direct_blocks[j]);
        }
    }

    // Free single indirect blocks
    if (i.indirect_block != 0) {
        uint32_t* ptr = (uint32_t*)get_block_ptr(i.indirect_block);
        for (uint32_t j = 0; j < POINTERS_PER_BLOCK; j++) {
            if (ptr[j] != 0) {
                free_block_internal(ptr[j]);
            }
        }
        free_block_internal(i.indirect_block);
    }

    // Free double indirect blocks
    if (i.double_indirect_block != 0) {
        uint32_t* double_ptr = (uint32_t*)get_block_ptr(i.double_indirect_block);
        for (uint32_t j = 0; j < POINTERS_PER_BLOCK; j++) {
            if (double_ptr[j] != 0) {
                uint32_t* inner_ptr = (uint32_t*)get_block_ptr(double_ptr[j]);
                for (uint32_t k = 0; k < POINTERS_PER_BLOCK; k++) {
                    if (inner_ptr[k] != 0) {
                        free_block_internal(inner_ptr[k]);
                    }
                }
                free_block_internal(double_ptr[j]);
            }
        }
        free_block_internal(i.double_indirect_block);
    }

    i.reset();
    superblock.free_inodes++;
    return true;
}

std::vector<std::string> FileSystem::split_path(const std::string& p) const {
    std::vector<std::string> result;
    std::string s = p;

    while (!s.empty() && s[0] == '/') s = s.substr(1);
    while (!s.empty() && s.back() == '/') s.pop_back();
    if (s.empty()) return result;

    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, '/')) {
        if (!part.empty() && part != ".") {
            if (part == "..") {
                if (!result.empty()) result.pop_back();
            } else {
                result.push_back(part);
            }
        }
    }
    return result;
}

std::pair<uint32_t, uint32_t> FileSystem::resolve_path(const QString& path) {
    uint32_t cur = current_inode;
    std::string p = path.toStdString();

    if (p.empty() || p == ".") return {cur, cur};
    if (p == "/") return {0, UINT32_MAX};
    if (p[0] == '/') cur = 0;

    auto parts = split_path(p);
    for (const auto& pt : parts) {
        const INode& i = inode_table[cur];
        if (i.file_type != FileType::DIRECTORY) return {UINT32_MAX, UINT32_MAX};

        auto entries = read_directory(cur);
        bool found = false;
        for (const auto& e : entries) {
            if (std::string(e.filename) == pt) {
                cur = e.inode_id;
                found = true;
                break;
            }
        }
        if (!found) return {UINT32_MAX, cur};
    }

    uint32_t parent = 0;
    if (!parts.empty()) {
        parent = current_inode;
        if (p[0] == '/') parent = 0;
        for (size_t i = 0; i < parts.size() - 1; i++) {
            auto entries = read_directory(parent);
            bool found = false;
            for (const auto& e : entries) {
                if (std::string(e.filename) == parts[i]) {
                    parent = e.inode_id;
                    found = true;
                    break;
                }
            }
            if (!found) {
                parent = UINT32_MAX;
                break;
            }
        }
    }

    return {cur, parent};
}

std::vector<DirEntry> FileSystem::read_directory(uint32_t n) {
    std::vector<DirEntry> result;

    if (n >= inode_table.size()) return result;

    const INode& i = inode_table[n];
    if (i.file_type != FileType::DIRECTORY) return result;

    for (uint32_t j = 0; j < i.block_count && j < DIRECT_BLOCKS; j++) {
        if (i.direct_blocks[j] == 0) continue;
        const uint8_t* blk = get_block_ptr(i.direct_blocks[j]);

        for (uint32_t k = 0; k < BLOCK_SIZE / DIR_ENTRY_SIZE; k++) {
            DirEntry e = DirEntry::unpack(blk + k * DIR_ENTRY_SIZE);
            if (e.inode_id != 0) {
                result.push_back(e);
            }
        }
    }
    return result;
}

bool FileSystem::add_dir_entry(uint32_t p, const std::string& name, uint32_t n) {
    if (p >= inode_table.size()) return false;

    INode& pi = inode_table[p];
    if (pi.file_type != FileType::DIRECTORY) return false;

    for (uint32_t i = 0; i < DIRECT_BLOCKS; i++) {
        if (pi.direct_blocks[i] == 0) {
            uint32_t b = allocate_block_internal();
            if (b == UINT32_MAX) return false;
            pi.direct_blocks[i] = b;
            pi.block_count++;
        }

        uint8_t* blk = get_block_ptr(pi.direct_blocks[i]);
        for (uint32_t j = 0; j < BLOCK_SIZE / DIR_ENTRY_SIZE; j++) {
            DirEntry e = DirEntry::unpack(blk + j * DIR_ENTRY_SIZE);
            if (e.inode_id == 0) {
                e.inode_id = n;
                strncpy(e.filename, name.c_str(), MAX_FILENAME_LEN - 1);
                e.pack(blk + j * DIR_ENTRY_SIZE);
                pi.file_size += DIR_ENTRY_SIZE;
                pi.modified_time = time(nullptr);
                return true;
            }
        }
    }
    return false;
}

bool FileSystem::remove_dir_entry(uint32_t p, const std::string& name) {
    if (p >= inode_table.size()) return false;

    INode& pi = inode_table[p];
    for (uint32_t i = 0; i < pi.block_count && i < DIRECT_BLOCKS; i++) {
        if (pi.direct_blocks[i] == 0) continue;
        uint8_t* blk = get_block_ptr(pi.direct_blocks[i]);

        for (uint32_t j = 0; j < BLOCK_SIZE / DIR_ENTRY_SIZE; j++) {
            DirEntry e = DirEntry::unpack(blk + j * DIR_ENTRY_SIZE);
            if (e.inode_id != 0 && std::string(e.filename) == name) {
                e.clear();
                e.pack(blk + j * DIR_ENTRY_SIZE);
                pi.file_size -= DIR_ENTRY_SIZE;
                pi.modified_time = time(nullptr);
                return true;
            }
        }
    }
    return false;
}

void FileSystem::generate_random_content(uint8_t* buf, uint32_t sz) {
    static std::mt19937 rng(static_cast<unsigned>(time(nullptr)));
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (uint32_t i = 0; i < sz; i++) {
        buf[i] = charset[rng() % (sizeof(charset) - 1)];
    }
}

bool FileSystem::mount(const QString& path) {
    disk_path = path;
    std::ifstream chk(path.toStdString(), std::ios::binary);
    if (chk.good()) {
        chk.close();
        return load_disk();
    }
    chk.close();
    return initialize_fresh();
}

bool FileSystem::initialize_fresh() {
    qDebug() << "Initializing fresh filesystem...";

    superblock.magic = FS_MAGIC;
    superblock.total_blocks = TOTAL_BLOCKS;
    superblock.block_size = BLOCK_SIZE;

    // Disk layout:
    // Block 0: Superblock
    // Block 1: Bitmap (1 block, exactly 2048 bytes = 16384 bits for 16384 blocks)
    // Blocks 2-2049: Inode table (2048 blocks = 16384 inodes * 128 bytes)
    // Blocks 2050+: Data blocks
    // Root directory data stored at data_block_start (2050)

    superblock.inode_table_blocks = (TOTAL_BLOCKS * INODE_SIZE + BLOCK_SIZE - 1) / BLOCK_SIZE;
    superblock.total_inodes = TOTAL_BLOCKS;
    superblock.inode_table_start = 2;  // After superblock + bitmap
    superblock.data_block_start = superblock.inode_table_start + superblock.inode_table_blocks;
    superblock.root_inode = 0;

    qDebug() << "Disk layout: inode_table_start=" << superblock.inode_table_start
             << "inode_table_blocks=" << superblock.inode_table_blocks
             << "data_block_start=" << superblock.data_block_start;

    bitmap.initialize();
    // Mark superblock as used
    bitmap.set_used(0);
    // Mark bitmap block as used
    bitmap.set_used(1);
    // Mark inode table blocks as used
    for (uint32_t i = 2; i < superblock.data_block_start; i++) {
        bitmap.set_used(i);
    }

    inode_table.resize(superblock.total_inodes);

    INode& root = inode_table[0];
    root.inode_id = 0;
    root.file_type = FileType::DIRECTORY;
    root.file_size = 2 * DIR_ENTRY_SIZE;
    root.block_count = 1;
    // Allocate root directory data block from DATA region
    root.direct_blocks[0] = superblock.data_block_start;
    bitmap.set_used(root.direct_blocks[0]);
    root.created_time = root.modified_time = root.accessed_time = time(nullptr);
    root.permissions = DEFAULT_DIR_PERM;
    root.link_count = 2;
    root.indirect_block = 0;
    root.double_indirect_block = 0;
    memset(root.direct_blocks + 1, 0, sizeof(uint32_t) * (DIRECT_BLOCKS - 1));

    // Write root directory entries to the allocated data block
    uint8_t* root_block = get_block_ptr(root.direct_blocks[0]);
    DirEntry d1, d2;
    d1.inode_id = 0;
    strncpy(d1.filename, ".", MAX_FILENAME_LEN - 1);
    d1.pack(root_block);

    d2.inode_id = 0;
    strncpy(d2.filename, "..", MAX_FILENAME_LEN - 1);
    d2.pack(root_block + DIR_ENTRY_SIZE);

    superblock.free_blocks = bitmap.get_free_count();
    superblock.free_inodes = superblock.total_inodes - 1;
    current_inode = 0;
    current_path = "/";

    // Create sample directories and files for demonstration
    create_sample_data();

    bool saved = save_disk();
    qDebug() << "Fresh filesystem initialized and" << (saved ? "saved" : "FAILED to save");
    return saved;
}

// Helper function to create sample data
void FileSystem::create_sample_data() {
    // Create root level directories
    create_directory("/bin");
    create_directory("/sbin");
    create_directory("/usr");
    create_directory("/usr/bin");
    create_directory("/usr/lib");
    create_directory("/usr/local");
    create_directory("/usr/local/bin");
    create_directory("/usr/local/lib");
    create_directory("/opt");
    create_directory("/opt/apps");
    create_directory("/opt/games");
    create_directory("/dev");
    create_directory("/proc");
    create_directory("/sys");
    create_directory("/run");
    create_directory("/srv");

    // Create /home directory structure
    create_directory("/home");
    create_directory("/home/user");
    create_directory("/home/user/Documents");
    create_directory("/home/user/Downloads");
    create_directory("/home/user/Pictures");
    create_directory("/home/user/Music");
    create_directory("/home/user/Videos");
    create_directory("/home/user/Projects");
    create_directory("/home/user/Projects/cpp");
    create_directory("/home/user/Projects/python");
    create_directory("/home/user/Projects/web");
    create_directory("/home/user/Desktop");

    // Create /root directory (superuser home)
    create_directory("/root");
    create_directory("/root/Documents");
    create_directory("/root/Downloads");

    // Create /etc directory structure
    create_directory("/etc");
    create_directory("/etc/systemd");
    create_directory("/etc/systemd/system");
    create_directory("/etc/network");
    create_directory("/etc/network/if-up.d");
    create_directory("/etc/X11");
    create_directory("/etc/profile.d");

    // Create /var directory structure
    create_directory("/var");
    create_directory("/var/log");
    create_directory("/var/tmp");
    create_directory("/var/cache");
    create_directory("/var/cache/apt");
    create_directory("/var/spool");
    create_directory("/var/spool/mail");
    create_directory("/var/spool/cron");
    create_directory("/var/lib");
    create_directory("/var/lib/dpkg");
    create_directory("/var/lib/mysql");
    create_directory("/var/ftp");
    create_directory("/var/backups");

    // Create /tmp directory (already exists, add subdirs)
    create_directory("/tmp");
    create_directory("/tmp/systemd-private");
    create_directory("/tmp/xauth-1000");

    // Create /boot directory
    create_directory("/boot");
    create_directory("/boot/grub");
    create_directory("/boot/grub/i386-pc");

    // Create /media directory
    create_directory("/media");
    create_directory("/media/cdrom");
    create_directory("/media/usb");

    // Create /mnt directory
    create_directory("/mnt");
    create_directory("/mnt/backup");
    create_directory("/mnt/shared");

    // Create sample files in various directories
    create_file("/readme.txt", 2);
    create_file("/license.txt", 4);
    create_file("/bin/ls", 8);
    create_file("/bin/cp", 6);
    create_file("/bin/mv", 5);
    create_file("/bin/bash", 12);
    create_file("/sbin/init", 10);
    create_file("/sbin/ifconfig", 7);
    create_file("/usr/bin/python3", 15);
    create_file("/usr/bin/gcc", 9);
    create_file("/usr/bin/git", 11);
    create_file("/usr/bin/vim", 8);
    create_file("/usr/local/bin/myscript.sh", 3);
    create_file("/opt/apps/calculator.exe", 20);
    create_file("/opt/games/tetris.exe", 50);
    create_file("/home/user/Documents/notes.txt", 3);
    create_file("/home/user/Documents/report.txt", 8);
    create_file("/home/user/Documents/todo.txt", 2);
    create_file("/home/user/Documents/resume.pdf", 15);
    create_file("/home/user/Documents/letter.txt", 4);
    create_file("/home/user/Downloads/package.zip", 50);
    create_file("/home/user/Downloads/archive.tar", 30);
    create_file("/home/user/Downloads/installer.exe", 100);
    create_file("/home/user/Downloads/video.mp4", 200);
    create_file("/home/user/Pictures/wallpaper.png", 100);
    create_file("/home/user/Pictures/photo1.jpg", 80);
    create_file("/home/user/Pictures/screenshot.png", 60);
    create_file("/home/user/Music/song1.mp3", 40);
    create_file("/home/user/Music/song2.wav", 70);
    create_file("/home/user/Music/playlist.m3u", 1);
    create_file("/home/user/Videos/movie.avi", 150);
    create_file("/home/user/Videos/tutorial.mp4", 90);
    create_file("/home/user/Projects/cpp/main.cpp", 5);
    create_file("/home/user/Projects/cpp/Makefile", 2);
    create_file("/home/user/Projects/python/script.py", 3);
    create_file("/home/user/Projects/python/requirements.txt", 1);
    create_file("/home/user/Projects/web/index.html", 4);
    create_file("/home/user/Projects/web/style.css", 2);
    create_file("/home/user/Desktop/shortcut.lnk", 1);
    create_file("/root/Documents/secret.txt", 5);
    create_file("/etc/config.conf", 5);
    create_file("/etc/passwd", 3);
    create_file("/etc/group", 2);
    create_file("/etc/hosts", 2);
    create_file("/etc/fstab", 3);
    create_file("/etc/network/interfaces", 4);
    create_file("/etc/X11/xorg.conf", 10);
    create_file("/var/log/system.log", 20);
    create_file("/var/log/error.log", 15);
    create_file("/var/log/access.log", 25);
    create_file("/var/log/auth.log", 12);
    create_file("/var/spool/mail/user", 8);
    create_file("/var/backups/daily.tar.gz", 80);
    create_file("/var/cache/apt/packages.dat", 30);
    create_file("/boot/grub/grub.cfg", 15);
}

bool FileSystem::load_disk() {
    std::ifstream f(disk_path.toStdString(), std::ios::binary);
    if (!f) {
        qWarning() << "Failed to open disk file:" << disk_path;
        return false;
    }

    // Check file size
    f.seekg(0, std::ios::end);
    std::streamsize fileSize = f.tellg();
    f.seekg(0, std::ios::beg);

    if (fileSize != TOTAL_SIZE) {
        qWarning() << "Invalid disk file size:" << fileSize << "expected:" << TOTAL_SIZE;
        f.close();
        return initialize_fresh();
    }

    // Read entire disk image
    f.read(reinterpret_cast<char*>(disk_data.data()), TOTAL_SIZE);
    if (!f.good() && !f.eof()) {
        qWarning() << "Failed to read disk file:" << disk_path;
        f.close();
        return false;
    }
    f.close();

    // Parse superblock
    uint32_t offset = 0;
    memcpy(&superblock.magic, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.total_blocks, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.block_size, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.total_inodes, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.free_blocks, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.free_inodes, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.inode_table_start, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.inode_table_blocks, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.data_block_start, disk_data.data() + offset, 4); offset += 4;
    memcpy(&superblock.root_inode, disk_data.data() + offset, 4); offset += 4;

    qDebug() << "Loaded superblock: magic=" << QString::number(superblock.magic, 16)
             << "total_blocks=" << superblock.total_blocks
             << "total_inodes=" << superblock.total_inodes
             << "inode_table_start=" << superblock.inode_table_start;

    if (!superblock.is_valid()) {
        qWarning() << "Invalid filesystem detected (magic:"
                  << QString::number(superblock.magic, 16)
                  << "expected:" << QString::number(FS_MAGIC, 16) << ")"
                  << "- reinitializing...";
        return initialize_fresh();
    }

    // Validate inode table parameters
    if (superblock.total_inodes == 0 || superblock.total_inodes > TOTAL_BLOCKS) {
        qWarning() << "Invalid total_inodes:" << superblock.total_inodes;
        return initialize_fresh();
    }

    // Load bitmap
    const auto& bmp = bitmap.getBitmap();
    memcpy(const_cast<std::vector<uint8_t>&>(bmp).data(), disk_data.data() + BLOCK_SIZE, bmp.size());

    // Load inode table
    inode_table.resize(superblock.total_inodes);
    uint64_t inode_offset = superblock.inode_table_start * BLOCK_SIZE;

    // Validate inode offset is within disk bounds
    if (inode_offset + superblock.total_inodes * INODE_SIZE > TOTAL_SIZE) {
        qWarning() << "Invalid inode table offset or size - reinitializing...";
        inode_table.clear();
        return initialize_fresh();
    }

    for (uint32_t i = 0; i < superblock.total_inodes; i++) {
        inode_table[i] = INode::unpack(disk_data.data() + inode_offset + i * INODE_SIZE);
    }

    current_inode = superblock.root_inode;
    current_path = "/";

    qDebug() << "Filesystem loaded successfully from:" << disk_path;
    return true;
}

bool FileSystem::save_disk() {
    superblock.free_blocks = bitmap.get_free_count();

    // Step 1: Write superblock to disk_data buffer
    char sb[BLOCK_SIZE] = {0};
    size_t offset = 0;
    memcpy(sb + offset, &superblock.magic, 4); offset += 4;
    memcpy(sb + offset, &superblock.total_blocks, 4); offset += 4;
    memcpy(sb + offset, &superblock.block_size, 4); offset += 4;
    memcpy(sb + offset, &superblock.total_inodes, 4); offset += 4;
    memcpy(sb + offset, &superblock.free_blocks, 4); offset += 4;
    memcpy(sb + offset, &superblock.free_inodes, 4); offset += 4;
    memcpy(sb + offset, &superblock.inode_table_start, 4); offset += 4;
    memcpy(sb + offset, &superblock.inode_table_blocks, 4); offset += 4;
    memcpy(sb + offset, &superblock.data_block_start, 4); offset += 4;
    memcpy(sb + offset, &superblock.root_inode, 4); offset += 4;
    memcpy(disk_data.data(), sb, BLOCK_SIZE);

    // Step 2: Write bitmap to disk_data buffer
    const auto& bmp = bitmap.getBitmap();
    memcpy(disk_data.data() + BLOCK_SIZE, bmp.data(), bmp.size());

    // Step 3: Write inode table to disk_data buffer
    uint64_t inode_offset = superblock.inode_table_start * BLOCK_SIZE;
    for (uint32_t i = 0; i < superblock.total_inodes; i++) {
        uint8_t ib[INODE_SIZE];
        inode_table[i].pack(ib);
        memcpy(disk_data.data() + inode_offset + i * INODE_SIZE, ib, INODE_SIZE);
    }

    // Step 4: Write entire disk_data to file in one operation
    std::ofstream o(disk_path.toStdString(), std::ios::binary | std::ios::trunc);
    if (!o) {
        qWarning() << "Failed to open disk file for writing:" << disk_path;
        return false;
    }

    o.write(reinterpret_cast<const char*>(disk_data.data()), TOTAL_SIZE);
    if (!o.good()) {
        qWarning() << "Failed to write disk data";
        o.close();
        return false;
    }

    o.flush();
    if (!o.good()) {
        qWarning() << "Failed to flush disk file";
        o.close();
        return false;
    }

    o.close();
    qDebug() << "Filesystem saved successfully to:" << disk_path;
    return true;
}

bool FileSystem::unmount() {
    return save_disk();
}

bool FileSystem::create_file(const QString& path, uint32_t size_kb) {
    if (size_kb == 0) return false;

    uint32_t size_bytes = size_kb * 1024;
    if (size_bytes > MAX_FILE_SIZE) return false;

    std::string p = path.toStdString();
    size_t last_slash = p.find_last_of('/');
    std::string parent_path, filename;
    if (last_slash == std::string::npos) {
        parent_path = "";
        filename = p;
    } else {
        parent_path = p.substr(0, last_slash);
        filename = p.substr(last_slash + 1);
    }

    if (filename.empty()) return false;

    auto [pi, _] = resolve_path(QString::fromStdString(parent_path));
    if (pi == UINT32_MAX) return false;

    auto entries = read_directory(pi);
    for (const auto& e : entries) {
        if (std::string(e.filename) == filename) return false;
    }

    uint32_t in = allocate_inode();
    if (in == UINT32_MAX) return false;

    INode& inode = inode_table[in];
    inode.file_type = FileType::REGULAR;
    inode.file_size = size_bytes;
    inode.block_count = (size_bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
    inode.created_time = inode.modified_time = inode.accessed_time = time(nullptr);
    inode.permissions = DEFAULT_FILE_PERM;
    inode.link_count = 1;
    inode.indirect_block = 0;
    inode.double_indirect_block = 0;

    uint32_t needed = inode.block_count;
    uint32_t allocated = 0;

    // Allocate direct blocks
    for (uint32_t i = 0; i < DIRECT_BLOCKS && allocated < needed; i++) {
        uint32_t b = allocate_block_internal();
        if (b == UINT32_MAX) {
            free_inode(in);
            return false;
        }
        inode.direct_blocks[i] = b;
        generate_random_content(get_block_ptr(b), BLOCK_SIZE);
        allocated++;
    }

    // Allocate single indirect blocks
    if (needed > DIRECT_BLOCKS) {
        inode.indirect_block = allocate_block_internal();
        if (inode.indirect_block == UINT32_MAX) {
            free_inode(in);
            return false;
        }
        uint32_t* ptr = (uint32_t*)get_block_ptr(inode.indirect_block);

        uint32_t single_indirect_count = needed - DIRECT_BLOCKS;
        uint32_t max_single_indirect = POINTERS_PER_BLOCK;

        uint32_t single_to_allocate = qMin(single_indirect_count, max_single_indirect);
        for (uint32_t i = 0; i < single_to_allocate && allocated < needed; i++) {
            uint32_t b = allocate_block_internal();
            if (b == UINT32_MAX) {
                free_inode(in);
                return false;
            }
            ptr[i] = b;
            generate_random_content(get_block_ptr(b), BLOCK_SIZE);
            allocated++;
        }

        // Allocate double indirect blocks if needed
        uint32_t remaining = needed - allocated;
        if (remaining > 0 && single_to_allocate == max_single_indirect) {
            inode.double_indirect_block = allocate_block_internal();
            if (inode.double_indirect_block == UINT32_MAX) {
                free_inode(in);
                return false;
            }
            uint32_t* double_ptr = (uint32_t*)get_block_ptr(inode.double_indirect_block);

            while (remaining > 0 && allocated < needed) {
                // Allocate a new indirect block for the double indirect level
                uint32_t indirect_block_num = allocate_block_internal();
                if (indirect_block_num == UINT32_MAX) {
                    free_inode(in);
                    return false;
                }
                double_ptr[(allocated - DIRECT_BLOCKS - max_single_indirect) / POINTERS_PER_BLOCK] = indirect_block_num;

                uint32_t* inner_ptr = (uint32_t*)get_block_ptr(indirect_block_num);
                for (uint32_t j = 0; j < POINTERS_PER_BLOCK && remaining > 0 && allocated < needed; j++) {
                    uint32_t b = allocate_block_internal();
                    if (b == UINT32_MAX) {
                        free_inode(in);
                        return false;
                    }
                    inner_ptr[j] = b;
                    generate_random_content(get_block_ptr(b), BLOCK_SIZE);
                    allocated++;
                    remaining--;
                }
            }
        }
    }

    if (!add_dir_entry(pi, filename, in)) {
        free_inode(in);
        return false;
    }

    inode_table[pi].link_count++;
    emit filesystemChanged();
    return true;
}

bool FileSystem::delete_file(const QString& path) {
    auto [in, pn] = resolve_path(path);
    if (in == UINT32_MAX) return false;

    if (inode_table[in].file_type != FileType::REGULAR) return false;

    std::string p = path.toStdString();
    std::string filename = p.find_last_of('/') == std::string::npos
                           ? p : p.substr(p.find_last_of('/') + 1);

    remove_dir_entry(pn, filename);
    inode_table[pn].link_count--;
    free_inode(in);

    emit filesystemChanged();
    return true;
}

bool FileSystem::copy_file(const QString& src, const QString& dst) {
    auto [si, _] = resolve_path(src);
    if (si == UINT32_MAX) return false;

    INode& s = inode_table[si];
    if (s.file_type != FileType::REGULAR) return false;

    uint32_t skb = (s.file_size + 1023) / 1024;
    if (skb == 0) skb = 1;

    bool result = create_file(dst, skb);
    if (result) emit filesystemChanged();
    return result;
}

bool FileSystem::create_directory(const QString& path) {
    std::string p = path.toStdString();
    size_t last_slash = p.find_last_of('/');
    std::string parent_path, dir_name;
    if (last_slash == std::string::npos) {
        parent_path = "";
        dir_name = p;
    } else {
        parent_path = p.substr(0, last_slash);
        dir_name = p.substr(last_slash + 1);
    }

    if (dir_name.empty()) return false;

    auto [pi, _] = resolve_path(QString::fromStdString(parent_path));
    if (pi == UINT32_MAX) return false;

    auto entries = read_directory(pi);
    for (const auto& e : entries) {
        if (std::string(e.filename) == dir_name) return false;
    }

    uint32_t in = allocate_inode();
    if (in == UINT32_MAX) return false;

    INode& inode = inode_table[in];
    inode.file_type = FileType::DIRECTORY;
    inode.file_size = 2 * DIR_ENTRY_SIZE;
    inode.block_count = 1;
    inode.direct_blocks[0] = allocate_block_internal();
    inode.created_time = inode.modified_time = inode.accessed_time = time(nullptr);
    inode.permissions = DEFAULT_DIR_PERM;
    inode.link_count = 2;

    uint8_t* blk = get_block_ptr(inode.direct_blocks[0]);
    DirEntry d1, d2;
    d1.inode_id = in;
    strncpy(d1.filename, ".", MAX_FILENAME_LEN - 1);
    d1.pack(blk);

    d2.inode_id = pi;
    strncpy(d2.filename, "..", MAX_FILENAME_LEN - 1);
    d2.pack(blk + DIR_ENTRY_SIZE);

    if (!add_dir_entry(pi, dir_name, in)) {
        free_inode(in);
        return false;
    }

    inode_table[pi].link_count++;
    emit filesystemChanged();
    return true;
}

bool FileSystem::delete_directory(const QString& path) {
    std::string p = path.toStdString();
    if (p == "/" || p.empty()) return false;

    auto [in, pn] = resolve_path(path);
    if (in == UINT32_MAX) return false;

    if (inode_table[in].file_type != FileType::DIRECTORY) return false;

    auto entries = read_directory(in);
    std::vector<DirEntry> user_entries;
    for (const auto& e : entries) {
        std::string n(e.filename);
        if (n != "." && n != "..") user_entries.push_back(e);
    }

    if (!user_entries.empty()) return false;

    std::string dir_name = p.find_last_of('/') == std::string::npos
                           ? p : p.substr(p.find_last_of('/') + 1);

    remove_dir_entry(pn, dir_name);
    inode_table[pn].link_count--;
    free_inode(in);

    emit filesystemChanged();
    return true;
}

bool FileSystem::delete_directory_recursive(const QString& path) {
    std::string p = path.toStdString();
    if (p == "/" || p.empty()) return false;

    auto [in, pn] = resolve_path(path);
    if (in == UINT32_MAX) return false;

    if (inode_table[in].file_type != FileType::DIRECTORY) return false;

    std::string dir_name = p.find_last_of('/') == std::string::npos
                           ? p : p.substr(p.find_last_of('/') + 1);

    // Recursively delete all contents
    auto entries = read_directory(in);
    for (const auto& e : entries) {
        std::string n(e.filename);
        if (n == "." || n == "..") continue;

        QString childPath = path + "/" + QString::fromStdString(n);
        if (inode_table[e.inode_id].file_type == FileType::DIRECTORY) {
            if (!delete_directory_recursive(childPath)) return false;
        } else {
            if (!delete_file(childPath)) return false;
        }
    }

    remove_dir_entry(pn, dir_name);
    inode_table[pn].link_count--;
    free_inode(in);

    emit filesystemChanged();
    return true;
}

bool FileSystem::rename_item(const QString& oldPath, const QString& newPath) {
    auto [oldIn, oldPn] = resolve_path(oldPath);
    if (oldIn == UINT32_MAX) return false;

    std::string oldName = oldPath.toStdString();
    size_t lastSlash = oldName.find_last_of('/');
    std::string oldBaseName = (lastSlash == std::string::npos) ? oldName : oldName.substr(lastSlash + 1);

    std::string newName = newPath.toStdString();
    lastSlash = newName.find_last_of('/');
    std::string newBaseName = (lastSlash == std::string::npos) ? newName : newName.substr(lastSlash + 1);

    // Check if new name already exists
    auto [newIn, newPn] = resolve_path(newPath);
    if (newIn != UINT32_MAX) return false;

    // Determine target parent directory
    uint32_t targetPn = oldPn;
    QString targetDir = oldPath;
    if (lastSlash != std::string::npos) {
        QString targetParent = QString::fromStdString(newName.substr(0, lastSlash));
        auto [tpIn, _] = resolve_path(targetParent);
        if (tpIn == UINT32_MAX) return false;
        targetPn = tpIn;
        targetDir = targetParent;
    }

    // Remove from old location
    if (!remove_dir_entry(oldPn, oldBaseName)) return false;

    // Add to new location
    if (!add_dir_entry(targetPn, newBaseName, oldIn)) {
        add_dir_entry(oldPn, oldBaseName, oldIn);
        return false;
    }

    inode_table[oldIn].modified_time = time(nullptr);
    emit filesystemChanged();
    return true;
}

bool FileSystem::move_item(const QString& src, const QString& dst) {
    return rename_item(src, dst);
}

bool FileSystem::copy_directory_recursive(const QString& src, const QString& dst) {
    auto [srcIn, srcPn] = resolve_path(src);
    if (srcIn == UINT32_MAX) return false;

    if (inode_table[srcIn].file_type != FileType::DIRECTORY) return false;

    std::string srcPath = src.toStdString();
    std::string dstPath = dst.toStdString();

    // Create destination directory
    if (!create_directory(dst)) return false;

    // Copy all contents recursively
    auto entries = read_directory(srcIn);
    for (const auto& e : entries) {
        std::string name(e.filename);
        if (name == "." || name == "..") continue;

        QString srcChild = src + "/" + QString::fromStdString(name);
        QString dstChild = dst + "/" + QString::fromStdString(name);

        if (inode_table[e.inode_id].file_type == FileType::DIRECTORY) {
            if (!copy_directory_recursive(srcChild, dstChild)) return false;
        } else {
            if (!copy_file(srcChild, dstChild)) return false;
        }
    }

    return true;
}

bool FileSystem::change_permissions(const QString& path, uint16_t permissions) {
    auto [in, pn] = resolve_path(path);
    if (in == UINT32_MAX) return false;

    inode_table[in].permissions = permissions;
    inode_table[in].modified_time = time(nullptr);
    emit filesystemChanged();
    return true;
}

bool FileSystem::read_file_content(const QString& path, QByteArray& content) {
    auto result = resolve_path(path);
    uint32_t inode = result.first;

    if (inode == UINT32_MAX) return false;

    const INode& fileInode = inode_table[inode];
    if (fileInode.file_type != FileType::REGULAR) return false;

    content.clear();
    content.reserve(fileInode.file_size);

    for (uint32_t i = 0; i < fileInode.block_count; i++) {
        uint32_t blockNum = 0;

        if (i < DIRECT_BLOCKS) {
            blockNum = fileInode.direct_blocks[i];
        } else if (i < DIRECT_BLOCKS + POINTERS_PER_BLOCK) {
            if (fileInode.indirect_block != 0) {
                const uint32_t* indirectPtr = (const uint32_t*)get_block_ptr(fileInode.indirect_block);
                blockNum = indirectPtr[i - DIRECT_BLOCKS];
            }
        } else {
            if (fileInode.double_indirect_block != 0) {
                const uint32_t* doublePtr = (const uint32_t*)get_block_ptr(fileInode.double_indirect_block);
                uint32_t doubleIndex = (i - DIRECT_BLOCKS - POINTERS_PER_BLOCK) / POINTERS_PER_BLOCK;
                uint32_t innerIndex = (i - DIRECT_BLOCKS - POINTERS_PER_BLOCK) % POINTERS_PER_BLOCK;

                if (doublePtr[doubleIndex] != 0) {
                    const uint32_t* innerPtr = (const uint32_t*)get_block_ptr(doublePtr[doubleIndex]);
                    blockNum = innerPtr[innerIndex];
                }
            }
        }

        if (blockNum == 0) continue;

        const uint8_t* blockData = get_block_ptr(blockNum);
        uint32_t bytesLeft = fileInode.file_size - content.size();
        uint32_t blockSize = qMin<uint32_t>(BLOCK_SIZE, bytesLeft);

        content.append((const char*)blockData, blockSize);
    }

    return true;
}

bool FileSystem::change_directory(const QString& path) {
    std::string p = path.toStdString();
    if (p.empty() || p == "/") {
        current_inode = 0;
        current_path = "/";
        emit filesystemChanged();
        return true;
    }

    auto [in, _] = resolve_path(path);
    if (in == UINT32_MAX) return false;

    if (inode_table[in].file_type != FileType::DIRECTORY) return false;

    current_inode = in;

    if (p[0] == '/') {
        current_path = path;
    } else {
        current_path = current_path == "/" ? ("/" + path) : (current_path + "/" + path);
    }

    while (current_path.length() > 1 && current_path.back() == '/') {
        current_path.chop(1);
    }

    emit filesystemChanged();
    return true;
}

void FileSystem::list_directory() {
    // Reserved for CLI mode - not used in GUI
}

void FileSystem::show_storage() {
    // Reserved for CLI mode - not used in GUI
}

QString FileSystem::get_current_path() const {
    return current_path;
}

std::vector<DirEntry> FileSystem::get_directory_entries(uint32_t n) {
    return read_directory(n);
}

BlockBitmap* FileSystem::get_bitmap() {
    return &bitmap;
}

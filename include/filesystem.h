#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <iostream>

// File system constants
constexpr uint32_t BLOCK_SIZE = 1024;
constexpr uint32_t TOTAL_SIZE = 16 * 1024 * 1024;
constexpr uint32_t TOTAL_BLOCKS = TOTAL_SIZE / BLOCK_SIZE;

constexpr uint32_t INODE_SIZE = 128;
constexpr uint32_t DIRECT_BLOCKS = 10;
constexpr uint32_t POINTER_SIZE = 4;
constexpr uint32_t POINTERS_PER_BLOCK = BLOCK_SIZE / POINTER_SIZE;

// Max file size with single indirect: 10 + 256 = 266 blocks = 266 KB
// Max file size with double indirect: 10 + 256 + 256*256 = 65802 blocks = 64.26 MB
constexpr uint32_t MAX_FILE_SIZE_SINGLE = (DIRECT_BLOCKS + POINTERS_PER_BLOCK) * BLOCK_SIZE;
constexpr uint32_t MAX_FILE_SIZE_DOUBLE = (DIRECT_BLOCKS + POINTERS_PER_BLOCK + POINTERS_PER_BLOCK * POINTERS_PER_BLOCK) * BLOCK_SIZE;
constexpr uint32_t MAX_FILE_SIZE = MAX_FILE_SIZE_DOUBLE;

enum class FileType : uint8_t {
    REGULAR = 0,
    DIRECTORY = 1
};

constexpr uint32_t FS_MAGIC = 0x2026B1F0;
constexpr uint32_t MAX_FILENAME_LEN = 56;
constexpr uint32_t DIR_ENTRY_SIZE = 64;
constexpr uint16_t DEFAULT_FILE_PERM = 0644;
constexpr uint16_t DEFAULT_DIR_PERM = 0755;

struct INode {
    uint32_t inode_id;
    FileType file_type;
    uint32_t file_size;
    uint32_t block_count;
    uint32_t direct_blocks[DIRECT_BLOCKS];
    uint32_t indirect_block;      // Single indirect block
    uint32_t double_indirect_block; // Double indirect block
    time_t created_time;
    time_t modified_time;
    time_t accessed_time;
    uint16_t permissions;
    uint16_t link_count;
    uint8_t reserved[14];

    INode();
    void reset();
    bool is_empty() const;
    void pack(uint8_t* buffer) const;
    static INode unpack(const uint8_t* buffer);
};

struct DirEntry {
    uint32_t inode_id;
    char filename[MAX_FILENAME_LEN];

    DirEntry();
    void clear();
    void pack(uint8_t* buffer) const;
    static DirEntry unpack(const uint8_t* buffer);
};

class BlockBitmap {
public:
    BlockBitmap();
    void initialize();
    uint32_t allocate_block();
    bool free_block(uint32_t b);
    bool is_used(uint32_t b) const;
    uint32_t get_free_count() const;
    uint32_t get_used_count() const;
    void set_used(uint32_t b);
    void set_free(uint32_t b);

    std::vector<uint8_t>& getBitmap() { return bitmap; }
    const std::vector<uint8_t>& getBitmap() const { return bitmap; }

private:
    std::vector<uint8_t> bitmap;
};

class Superblock {
public:
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t block_size;
    uint32_t total_inodes;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t inode_table_start;
    uint32_t inode_table_blocks;
    uint32_t data_block_start;
    uint32_t root_inode;

    Superblock();
    bool is_valid() const;
};

class FileSystem : public QObject {
    Q_OBJECT
public:
    FileSystem();
    ~FileSystem();

    bool mount(const QString& path);
    bool unmount();
    bool create_file(const QString& path, uint32_t size_kb);
    bool delete_file(const QString& path);
    bool copy_file(const QString& src, const QString& dst);
    bool create_directory(const QString& path);
    bool delete_directory(const QString& path);
    bool delete_directory_recursive(const QString& path);
    bool rename_item(const QString& oldPath, const QString& newPath);
    bool move_item(const QString& src, const QString& dst);
    bool copy_directory_recursive(const QString& src, const QString& dst);
    bool change_permissions(const QString& path, uint16_t permissions);
    bool change_directory(const QString& path);
    void list_directory();
    void show_storage();
    QString get_current_path() const;
    std::vector<DirEntry> get_directory_entries(uint32_t n);
    BlockBitmap* get_bitmap();
    std::pair<uint32_t, uint32_t> resolve_path(const QString& path);
    const Superblock& get_superblock() const { return superblock; }
    const std::vector<uint8_t>& get_disk_data() const { return disk_data; }
    bool read_file_content(const QString& path, QByteArray& content);

    std::vector<INode> inode_table;
    uint32_t current_inode;

signals:
    void filesystemChanged();

private:
    QString disk_path;
    std::vector<uint8_t> disk_data;
    Superblock superblock;
    BlockBitmap bitmap;
    QString current_path;

    uint8_t* get_block_ptr(uint32_t b);
    const uint8_t* get_block_ptr(uint32_t b) const;
    uint32_t allocate_block_internal();
    bool free_block_internal(uint32_t b);
    uint32_t allocate_inode();
    bool free_inode(uint32_t n);
    std::vector<std::string> split_path(const std::string& p) const;
    std::vector<DirEntry> read_directory(uint32_t n);
    bool add_dir_entry(uint32_t p, const std::string& name, uint32_t n);
    bool remove_dir_entry(uint32_t p, const std::string& name);
    void generate_random_content(uint8_t* buf, uint32_t sz);
    bool load_disk();
    bool save_disk();
    bool initialize_fresh();
    void create_sample_data();
};

#endif

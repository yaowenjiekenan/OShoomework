#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <cstdio>
#include <cstring>
#include <vector>
#include <ctime>
#include <fstream>

// Constants
constexpr uint32_t BLOCK_SIZE = 1024;
constexpr uint32_t TOTAL_SIZE = 16 * 1024 * 1024;
constexpr uint32_t TOTAL_BLOCKS = TOTAL_SIZE / BLOCK_SIZE;
constexpr uint32_t INODE_SIZE = 128;
constexpr uint32_t DIRECT_BLOCKS = 10;
constexpr uint32_t POINTER_SIZE = 4;
constexpr uint32_t POINTERS_PER_BLOCK = BLOCK_SIZE / POINTER_SIZE;
constexpr uint32_t FS_MAGIC = 0x2026B1F0;
constexpr uint32_t MAX_FILENAME_LEN = 56;
constexpr uint32_t DIR_ENTRY_SIZE = 64;

enum class FileType : uint8_t { REGULAR = 0, DIRECTORY = 1 };

struct Superblock {
    uint32_t magic = 0;
    uint32_t total_blocks = 0;
    uint32_t block_size = 0;
    uint32_t total_inodes = 0;
    uint32_t free_blocks = 0;
    uint32_t free_inodes = 0;
    uint32_t inode_table_start = 0;
    uint32_t inode_table_blocks = 0;
    uint32_t data_block_start = 0;
    uint32_t root_inode = 0;
};

struct INode {
    uint32_t inode_id = 0;
    FileType file_type = FileType::REGULAR;
    uint32_t file_size = 0;
    uint32_t block_count = 0;
    uint32_t direct_blocks[DIRECT_BLOCKS];
    uint32_t indirect_block = 0;
    uint32_t double_indirect_block = 0;
    time_t created_time = 0;
    time_t modified_time = 0;
    time_t accessed_time = 0;
    uint16_t permissions = 0;
    uint16_t link_count = 0;
    uint8_t reserved[14];

    void reset() {
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
        permissions = 0;
        link_count = 0;
        memset(reserved, 0, sizeof(reserved));
    }
};

bool is_valid_superblock(const Superblock& sb) {
    return sb.magic == FS_MAGIC && sb.total_blocks == TOTAL_BLOCKS;
}

INode unpack_inode(const uint8_t* buffer) {
    INode i;
    uint32_t offset = 0;

    memcpy(&i.inode_id, buffer + offset, 4); offset += 4;
    uint8_t fileTypeByte;
    memcpy(&fileTypeByte, buffer + offset, 1);
    i.file_type = static_cast<FileType>(fileTypeByte);
    offset += 4;  // Match pack() alignment
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

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    printf("=== OS.exe Load Simulation ===\n\n");

    QString diskPath = "filesystem.dat";
    std::vector<uint8_t> disk_data(TOTAL_SIZE, 0);
    std::vector<INode> inode_table;

    // Step 1: Open and read file
    printf("Step 1: Opening file...\n");
    std::ifstream f(diskPath.toStdString(), std::ios::binary);
    if (!f) {
        printf("ERROR: Cannot open file for reading\n");
        return 1;
    }

    f.read(reinterpret_cast<char*>(disk_data.data()), TOTAL_SIZE);
    if (!f.good() && !f.eof()) {
        printf("ERROR: Failed to read file\n");
        f.close();
        return 1;
    }
    f.close();
    printf("OK: File read into memory (%d bytes)\n\n", TOTAL_SIZE);

    // Step 2: Parse superblock
    printf("Step 2: Parsing superblock...\n");
    Superblock superblock;
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

    printf("  Magic: 0x%08X\n", superblock.magic);
    printf("  Total Blocks: %d\n", superblock.total_blocks);
    printf("  Total Inodes: %d\n", superblock.total_inodes);
    printf("  Inode Table Start: %d\n", superblock.inode_table_start);
    printf("  Data Block Start: %d\n", superblock.data_block_start);

    if (!is_valid_superblock(superblock)) {
        printf("ERROR: Invalid superblock\n");
        return 1;
    }
    printf("OK: Superblock valid\n\n");

    // Step 3: Load inode table
    printf("Step 3: Loading inode table...\n");
    inode_table.resize(superblock.total_inodes);
    uint64_t inode_offset = superblock.inode_table_start * BLOCK_SIZE;

    printf("  Inode offset: %llu\n", (unsigned long long)inode_offset);
    printf("  Inode count: %d\n", superblock.total_inodes);

    // Check bounds
    if (inode_offset + superblock.total_inodes * INODE_SIZE > TOTAL_SIZE) {
        printf("ERROR: Inode table exceeds disk bounds!\n");
        printf("  Required: %llu bytes\n", (unsigned long long)(inode_offset + superblock.total_inodes * INODE_SIZE));
        printf("  Available: %d bytes\n", TOTAL_SIZE);
        return 1;
    }

    for (uint32_t i = 0; i < superblock.total_inodes; i++) {
        inode_table[i] = unpack_inode(disk_data.data() + inode_offset + i * INODE_SIZE);
    }
    printf("OK: Inode table loaded\n\n");

    // Step 4: Verify root inode
    printf("Step 4: Verifying root inode...\n");
    INode& root = inode_table[0];
    printf("  Root inode_id: %d\n", root.inode_id);
    printf("  Root file_type: %d\n", static_cast<int>(root.file_type));
    printf("  Root file_size: %d\n", root.file_size);
    printf("  Root block_count: %d\n", root.block_count);
    printf("  Root direct_blocks[0]: %d\n", root.direct_blocks[0]);

    if (root.direct_blocks[0] == 0) {
        printf("ERROR: Root inode has no data block!\n");
        return 1;
    }

    // Step 5: Try to read root directory
    printf("\nStep 5: Reading root directory...\n");
    uint32_t root_block = root.direct_blocks[0];
    printf("  Root block number: %d\n", root_block);

    if (root_block >= TOTAL_BLOCKS) {
        printf("ERROR: Root block number out of bounds!\n");
        return 1;
    }

    uint8_t* block_ptr = disk_data.data() + root_block * BLOCK_SIZE;

    struct DirEntry {
        uint32_t inode_id;
        char filename[MAX_FILENAME_LEN];
    };

    DirEntry* entries = reinterpret_cast<DirEntry*>(block_ptr);
    int entry_count = 0;
    for (int i = 0; i < BLOCK_SIZE / DIR_ENTRY_SIZE; i++) {
        if (entries[i].inode_id != 0) {
            printf("  Entry %d: inode=%d, name='%.56s'\n", i, entries[i].inode_id, entries[i].filename);
            entry_count++;
        }
    }
    printf("  Total entries: %d\n", entry_count);

    if (entry_count == 0) {
        printf("WARNING: Root directory is empty!\n");
    }

    printf("\n=== All steps completed successfully! ===\n");
    printf("The filesystem data appears to be valid.\n");
    printf("Problem must be in the GUI initialization.\n");

    return 0;
}

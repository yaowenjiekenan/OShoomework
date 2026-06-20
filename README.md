# Unix-style File System - Qt Version

操作系统课程设计 - 基于 i-node 的 Unix 风格文件系统（Qt 实现）

## 项目要求

**课程**: Operating Systems, Module Practice
**学期**: Summer 2026
**截止日期**: 2026/06/20 23:59

### 任务要求

1. 深入研究类 Unix 文件系统的核心设计原理：
   - i-node 的结构和作用
   - 数据块的分配和管理
   - 目录的实现
   - Superblock 的作用
   - 文件访问权限机制

2. **设计和实现**一个基于 i-node 的 Unix 风格文件系统

3. **必须使用 C++**

---

## 技术规格

| 参数 | 值 |
|------|-----|
| **总存储空间** | 16 MB |
| **块大小** | 1 KB |
| **总块数** | 16,384 |
| **地址长度** | 24 位 |
| **i-node 大小** | 128 字节 |
| **直接块指针** | 10 个 |
| **单级间接块指针** | 1 个（256 块） |
| **双级间接块指针** | 1 个（65,536 块） |
| **最大文件大小** | ~64 MB |

### 磁盘布局

```
┌─────────┬───────────┬───────────┬───────────────────────┐
│ Block 0 │ Blocks 1-2│ Blocks 3- │ Block 2051+            │
│ 超级块   │  位图      │  i-node表  │  数据块                 │
│ (1 KB)  │ (2 KB)    │  (2048块) │  (14,333块)           │
└─────────┴───────────┴───────────┴───────────────────────┘
```

### 虚拟地址结构

```
24-bit address: ┌─────────────────────┬──────────────┐
                │  Block Number (14b)  │  Offset (10b)│
                └─────────────────────┴──────────────┘
```

### i-node 结构

```
┌────────────────────────────────────────┐
│  i-node Number                        │
│  File Type (REGULAR/DIRECTORY)        │
│  File Size                            │
│  Block Count                          │
│  ──────────────────────────────────── │
│  Direct Block Pointers [0-9]          │  ← 10 blocks
│  Single Indirect Block Pointer        │  ← 256 more blocks
│  Double Indirect Block Pointer        │  ← 65536 more blocks
│  ──────────────────────────────────── │
│  Created/Modified/Accessed Time       │
│  Permissions                          │
│  Link Count                           │
└────────────────────────────────────────┘
= 128 bytes
```

---

## 编译和运行

### 环境要求

- Qt 6.5+ (推荐 Qt 6.8 LTS)
- MinGW-w64 编译器
- Windows 10/11

### 编译方式

#### 方式一：使用批处理文件（最简单）

```bash
双击运行 build_qt.bat
```

> 脚本会自动检测 Qt 路径、编译、部署运行时并启动程序。
> 如果你的 Qt 安装路径不同，请修改 `build_qt.bat` 开头两行：
> ```bat
> set QT_DIR=D:\你的Qt路径
> set QT_VERSION=你的版本号
> ```

#### 方式二：直接运行（已编译好的 exe）

```bash
双击运行 run.bat
```

> 该脚本会先调用 windeployqt 部署所需的 Qt DLL，然后启动程序。

#### 方式三：使用 Qt Creator

1. 用 Qt Creator 打开 `OS.pro` 文件
2. 点击 "Configure Project"，选择 MinGW 64-bit Kit
3. 点击左下角的 "Run" 按钮编译运行

#### 方式四：手动命令行

```bash
qmake -spec win32-g++ OS.pro -o Makefile
mingw32-make -j4
windeployqt --no-translations OS.exe
OS.exe
```

### 启动选项

| 命令 | 说明 |
|------|------|
| `OS.exe` | 启动 GUI 模式（默认） |
| `OS.exe --cli` | 启动 CLI 命令行模式 |

---

## 功能特性

### GUI 功能

- **资源管理器风格界面**：左侧目录树，右侧文件列表
- **导航功能**：前进、后退、向上、刷新
- **搜索过滤**：文件名模糊搜索
- **排序**：按名称、大小、日期、类型排序
- **文件操作**：新建文件夹、新建文件、删除、重命名、复制、移动
- **文件内容查看**：双击或右键查看文件内容
- **权限管理**：GUI 可视化修改 rwx 权限
- **属性查看**：显示 inode 编号、路径、权限等详细信息
- **存储信息**：显示磁盘使用情况

### CLI 命令

| 命令 | 别名 | 描述 |
|------|------|------|
| `createFile <path> <sizeKB>` | `create` | 创建文件（随机内容填充） |
| `deleteFile <path>` | `delete` | 删除文件 |
| `createDir <path>` | `mkdir` | 创建目录（支持嵌套） |
| `deleteDir <path>` | `rmdir` | 删除空目录 |
| `deleteDir -r <path>` | | 递归删除目录 |
| `changeDir <path>` | `cd` | 切换工作目录 |
| `dir` | `ls` | 列出当前目录（含权限、大小、时间） |
| `cp <src> <dst>` | `copy` | 复制文件或目录 |
| `mv <src> <dst>` | `move` | 移动/重命名 |
| `cat <path>` | | 查看文件内容 |
| `chmod <path> <perm>` | | 修改权限（八进制，如 644/755） |
| `sum` | `df` | 显示存储使用情况和柱状图 |
| `help` | | 显示帮助 |

### GUI 快捷键

| 操作 | 快捷键 | 描述 |
|------|--------|------|
| 后退 | Alt+左 | 返回上一目录 |
| 向上 | Backspace | 跳转到父目录 |
| 刷新 | F5 | 刷新当前视图 |
| 新建文件夹 | Ctrl+Shift+N | 创建新文件夹 |
| 新建文件 | Ctrl+N | 创建新文件 |
| 删除 | Delete | 删除选中项 |
| 复制 | Ctrl+C | 复制选中项 |
| 移动 | Ctrl+X | 移动选中项 |
| 重命名 | F2 | 重命名选中项 |
| 查看内容 | Ctrl+O | 查看文件内容 |
| 属性 | Alt+Enter | 查看文件/目录属性 |

---

## 项目结构

```
OShoomework/
├── OS.pro              # Qt 项目文件
├── build_qt.bat        # 一键编译运行脚本
├── run.bat             # 直接运行脚本（部署 DLL + 启动）
├── README.md           # 本文件
├── filesystem.dat      # 虚拟磁盘文件（运行时自动生成）
├── qt.conf             # Qt 路径配置（编译用）
├── include/
│   ├── filesystem.h    # 文件系统核心类
│   ├── mainwindow.h    # 主窗口 / 树模型 / 列表模型
│   └── cli.h           # 命令行接口类
├── src/
│   ├── main.cpp        # 程序入口（GUI / CLI 模式切换）
│   ├── filesystem.cpp  # 文件系统核心实现
│   ├── mainwindow.cpp  # 主窗口 / 树模型 / 列表模型实现
│   └── cli.cpp         # 命令行接口实现
└── forms/
    └── mainwindow.ui   # Qt UI 文件
```

---

## 注意事项

1. **必须使用 C++** - 课程要求
2. **数据持久化** - 文件系统数据保存在 `filesystem.dat`
3. **新文件随机填充** - `createFile` 创建的文件内容为随机字符串
4. **`cp` 复制实际内容** - `cp` 命令会复制源文件的真实数据
5. **磁盘布局变更需重建** - 如更改布局常量，需删除 `filesystem.dat` 重新生成
6. **截止日期** - 2026/06/20 23:59

---

## 更新日志

### 2026-05-28 — Bug 修复

#### 修复的 Bug
1. **搜索框导航 Bug**：后退箭头（←）返回后，双击目录提示 `Cannot navigate to: /opt/games/games`
   - 根因：`onGoBack()` 手动维护状态，漏同步 `m_currentPath`
   - 修复：`onGoBack()` 统一调用 `navigateTo()`，与其他导航路径一致
2. **位图溢出**：位图需要 2048 字节（2 块），但只分配了 1 块（块1），后半部分溢出到 i-node 表
   - 修复：磁盘布局改为 Block 1-2=位图，Block 3+=i-node 表
3. **删除当前工作目录无保护**：Filesystem 核心层未阻止删除当前所在目录
   - 修复：`delete_directory()` 和 `delete_directory_recursive()` 加入 `if (in == current_inode) return false`

#### 改进
4. **权限显示**：GUI 列表和 CLI `dir` 命令现在从 inode 实际权限值读取，而非硬编码
5. **`cp` 命令**：现复制源文件的真实数据内容，而非随机填充

## 评分要点

1. 按照课程报告要求完成，内容完整
2. 核心组件或步骤的原理有清晰的解释和描述
3. 实验结果有详细的解释和讨论
4. 使用正确模板；布局整洁，格式规范；有见地的总结

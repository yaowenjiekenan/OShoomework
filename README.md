# Unix-style File System - Qt Version

操作系统课程设计 - 基于 i-node 的 Unix 风格文件系统（Qt 实现）

## 项目要求

**课程**: Operating Systems, Module Practice
**学期**: Spring 2026
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
| **间接块指针** | 1 个 |
| **最大文件大小** | ~257 KB |

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
│  Indirect Block Pointer               │  ← 256 more blocks
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

- Qt 6.x (推荐 Qt 6.5+)
- MinGW-w64 编译器
- Windows 10/11

### 安装 Qt

1. 下载 [Qt Online Installer](https://www.qt.io/download-qt-installer)
2. 安装时选择：
   - Qt 6.5+ (LTS)
   - MSVC 2019/2022 64-bit 或 MinGW 编译器
   - Qt Creator IDE

### 编译方式

#### 方式一：使用 Qt Creator（推荐）

1. 用 Qt Creator 打开 `OS.pro` 文件
2. 点击 "Configure Project"
3. 点击左下角的 "Run" 按钮编译运行

#### 方式二：使用命令行

```bash
# 使用 Qt qmake
qmake -spec win32-g++ OS.pro -o Makefile
mingw32-make -j4
./OS.exe
```

#### 方式三：使用批处理文件

```bash
双击运行 build_qt.bat
```

#### 方式四：使用 Makefile

```bash
mingw32-make -f Makefile.qt
```

---

## 功能特性

### GUI 功能

- **资源管理器风格界面**：左侧目录树，右侧文件列表
- **导航功能**：前进、后退、向上、刷新
- **路径导航**：地址栏快速跳转
- **文件操作**：新建文件夹、新建文件、删除
- **存储信息**：显示磁盘使用情况

### 支持的操作

| 操作 | 快捷键 | 描述 |
|------|--------|------|
| 后退 | Alt+左 | 返回上一目录 |
| 向上 | Backspace | 跳转到父目录 |
| 刷新 | F5 | 刷新当前视图 |
| 新建文件夹 | Ctrl+Shift+N | 创建新文件夹 |
| 新建文件 | Ctrl+N | 创建新文件 |
| 删除 | Delete | 删除选中项 |

---

## 项目结构

```
f:/OS/
├── OS.pro              # Qt 项目文件
├── Makefile.qt         # Qt Makefile
├── build_qt.bat        # Windows 编译脚本
├── README.md           # 本文件
├── filesystem.dat      # 虚拟磁盘文件（运行时生成）
├── include/
│   ├── filesystem.h    # 文件系统核心类
│   └── mainwindow.h    # 主窗口类
├── src/
│   ├── main.cpp        # 程序入口
│   ├── mainwindow.cpp  # 主窗口实现
│   └── filesystem.cpp   # 文件系统实现
└── forms/
    └── mainwindow.ui   # Qt UI 文件
```

---

## 注意事项

1. **必须使用 C++** - 课程要求
2. **数据持久化** - 文件系统数据保存在 `filesystem.dat`
3. **随机内容** - 文件内容为随机字符串填充
4. **截止日期** - 2026/06/20 23:59

---

## 评分要点

1. 按照课程报告要求完成，内容完整
2. 核心组件或步骤的原理有清晰的解释和描述
3. 实验结果有详细的解释和讨论
4. 使用正确模板；布局整洁，格式规范；有见地的总结

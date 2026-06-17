#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTreeView>
#include <QListView>
#include <QTableView>
#include <QSplitter>
#include <QLineEdit>
#include <QLabel>
#include <QAction>
#include <QModelIndex>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QCloseEvent>
#include <QProgressBar>

#include "filesystem.h"

class FileSystemTreeModel;
class FileSystemListModel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onTreeItemClicked(const QModelIndex& index);
    void onListItemDoubleClicked(const QModelIndex& index);
    void onPathReturnPressed();
    void onNewFolder();
    void onNewFile();
    void onDelete();
    void onRefresh();
    void onShowStorage();
    void onAbout();
    void onGoBack();
    void onGoUp();
    void onSearch(const QString& text);
    void onSortChanged(int index);
    void onRename();
    void onCopy();
    void onMove();
    void onViewContent();
    void onChangePermissions();
    void onShowProperties();
    void onSelectionChanged();
    QString formatSize(uint32_t size) const;

private:
    void setupUi();
    void createActions();
    void createMenus();
    void createToolBar();
    void populateTree();
    void populateList();
    void updateStatusBar();
    void navigateTo(const QString& path);
    void refreshView();

    FileSystem* m_fileSystem;
    FileSystemTreeModel* m_treeModel;
    FileSystemListModel* m_listModel;
    QSortFilterProxyModel* m_proxyModel;

    QTreeView* m_treeView;
    QTableView* m_listView;
    QLineEdit* m_pathEdit;
    QLineEdit* m_searchEdit;
    QComboBox* m_sortCombo;
    QLabel* m_statusLabel;
    QLabel* m_itemCountLabel;
    QProgressBar* m_diskBar;

    QAction* m_backAction;
    QAction* m_upAction;
    QAction* m_newFolderAction;
    QAction* m_newFileAction;
    QAction* m_deleteAction;
    QAction* m_refreshAction;
    QAction* m_storageAction;
    QAction* m_copyAction;
    QAction* m_renameAction;
    QAction* m_moveAction;
    QAction* m_viewContentAction;
    QAction* m_permissionsAction;
    QAction* m_propertiesAction;

    QString m_currentPath;
    QStringList m_history;
    int m_historyIndex;

    QString m_diskPath;
};

// ============================================================================
// Tree Model for Left Panel
// ============================================================================
class FileSystemTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit FileSystemTreeModel(FileSystem* fs, QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void refresh();
    uint32_t inodeFromIndex(const QModelIndex& index) const;
    QString buildPathFromIndex(const QModelIndex& index) const;

private:
    void buildTree();
    struct TreeNode {
        uint32_t inode;
        QString name;
        TreeNode* parent;
        QVector<TreeNode*> children;
    };

    TreeNode* m_rootNode;
    FileSystem* m_fs;
};

// ============================================================================
// List Model for Right Panel
// ============================================================================
struct FileSystemListItem {
    uint32_t inode;
    QString name;
    bool isDirectory;
    uint32_t size;
    QString modified;
    QString permissions;
    QString fileExtension;  // e.g., "txt", "cpp", "exe", etc.

    enum FileKind {
        KindFolder,
        KindDocument,
        KindCode,
        KindImage,
        KindAudio,
        KindVideo,
        KindArchive,
        KindExecutable,
        KindConfig,
        KindOther
    };
    FileKind fileKind;
};

class FileSystemListModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit FileSystemListModel(FileSystem* fs, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void refresh();
    void setDirectory(uint32_t inode);
    QVector<FileSystemListItem> getItems() const;
    QString formatSize(uint32_t size) const;

    enum Column {
        ColName = 0,
        ColSize,
        ColType,
        ColModified,
        ColPermissions,
        ColCount
    };

private:
    QVector<FileSystemListItem> m_items;
    FileSystem* m_fs;
    uint32_t m_currentInode;
};

#endif // MAINWINDOW_H

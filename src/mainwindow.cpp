#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QIcon>
#include <QFont>
#include <QFileDialog>
#include <QDateTime>
#include <QModelIndexList>
#include <QVariant>
#include <QPainter>
#include <QPixmap>
#include <QBitmap>
#include <QPalette>
#include <QPainterPath>
#include <QDialog>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>

// Helper function to create colored folder icon
static QIcon createFolderIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Folder body (yellow/amber)
    QPainterPath body;
    body.moveTo(2, 10);
    body.lineTo(2, 26);
    body.lineTo(30, 26);
    body.lineTo(30, 10);
    body.lineTo(10, 10);
    body.lineTo(8, 7);
    body.lineTo(4, 7);
    body.lineTo(4, 10);
    body.closeSubpath();
    p.fillPath(body, QColor(255, 193, 7));

    // Folder tab
    QPainterPath tab;
    tab.moveTo(2, 10);
    tab.lineTo(2, 7);
    tab.lineTo(4, 7);
    tab.lineTo(8, 7);
    tab.lineTo(10, 10);
    tab.closeSubpath();
    p.fillPath(tab, QColor(255, 193, 7));

    // Darker outline
    p.setPen(QPen(QColor(200, 150, 0), 1));
    p.drawPath(body);
    p.end();

    return QIcon(pix);
}

// Helper function to create colored document icon
static QIcon createDocumentIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Document body (white with border)
    QPainterPath doc;
    doc.moveTo(6, 4);
    doc.lineTo(20, 4);
    doc.lineTo(26, 10);
    doc.lineTo(26, 28);
    doc.lineTo(6, 28);
    doc.closeSubpath();
    p.fillPath(doc, Qt::white);
    p.setPen(QPen(Qt::gray, 1));
    p.drawPath(doc);

    // Folded corner
    QPainterPath corner;
    corner.moveTo(20, 4);
    corner.lineTo(20, 10);
    corner.lineTo(26, 10);
    corner.closeSubpath();
    p.fillPath(corner, QColor(220, 220, 220));

    // Text lines
    p.setPen(Qt::darkBlue);
    p.drawLine(9, 14, 23, 14);
    p.drawLine(9, 18, 23, 18);
    p.drawLine(9, 22, 18, 22);

    p.end();
    return QIcon(pix);
}

// Helper function to create code icon
static QIcon createCodeIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Code brackets background
    QPainterPath bg;
    bg.addRoundedRect(QRectF(4, 4, 24, 24), 4, 4);
    p.fillPath(bg, QColor(50, 50, 60));

    // Brackets
    p.setPen(QPen(Qt::white, 2));
    p.drawText(QRect(4, 4, 24, 24), Qt::AlignCenter, "<>");

    p.end();
    return QIcon(pix);
}

// Helper function to create image icon
static QIcon createImageIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Image frame
    QPainterPath frame;
    frame.addRoundedRect(QRectF(3, 3, 26, 26), 3, 3);
    p.fillPath(frame, QColor(100, 149, 237));
    p.setPen(QPen(Qt::darkBlue, 1));
    p.drawPath(frame);

    // Sun
    p.setBrush(Qt::yellow);
    p.drawEllipse(QPoint(10, 12), 3, 3);

    // Mountain
    QPainterPath mountain;
    mountain.moveTo(5, 24);
    mountain.lineTo(14, 14);
    mountain.lineTo(20, 20);
    mountain.lineTo(27, 12);
    mountain.lineTo(27, 24);
    mountain.closeSubpath();
    p.fillPath(mountain, Qt::darkGreen);

    p.end();
    return QIcon(pix);
}

// Helper function to create audio icon
static QIcon createAudioIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Music note
    QPainterPath note;
    note.addEllipse(QRectF(8, 20, 8, 8));
    note.addRect(QRectF(15, 6, 3, 16));
    note.addEllipse(QRectF(11, 4, 10, 6));
    p.fillPath(note, QColor(156, 39, 176));
    p.end();

    return QIcon(pix);
}

// Helper function to create video icon
static QIcon createVideoIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Video frame
    QPainterPath frame;
    frame.addRoundedRect(QRectF(3, 7, 26, 18), 2, 2);
    p.fillPath(frame, QColor(244, 67, 54));
    p.setPen(QPen(Qt::darkRed, 1));
    p.drawPath(frame);

    // Play triangle
    QPainterPath play;
    play.moveTo(13, 12);
    play.lineTo(13, 20);
    play.lineTo(20, 16);
    play.closeSubpath();
    p.fillPath(play, Qt::white);

    p.end();
    return QIcon(pix);
}

// Helper function to create archive icon
static QIcon createArchiveIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Zipper/box
    QPainterPath box;
    box.addRoundedRect(QRectF(4, 6, 24, 22), 2, 2);
    p.fillPath(box, QColor(121, 85, 72));
    p.setPen(QPen(QColor(80, 50, 30), 1));
    p.drawPath(box);

    // Zipper line
    p.setPen(QPen(QColor(255, 200, 0), 2));
    p.drawLine(16, 6, 16, 28);

    // Pull
    p.setBrush(Qt::yellow);
    p.drawRect(QRect(13, 13, 6, 6));

    p.end();
    return QIcon(pix);
}

// Helper function to create executable icon
static QIcon createExecutableIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Gear/cog background
    QPainterPath bg;
    bg.addEllipse(QRectF(4, 4, 24, 24));
    p.fillPath(bg, QColor(33, 150, 243));

    // Gear teeth
    p.setBrush(QColor(33, 150, 243));
    p.setPen(QPen(QColor(21, 101, 192), 1));
    for (int i = 0; i < 6; i++) {
        QRectF rect(11, 2, 10, 28);
        p.save();
        p.translate(16, 16);
        p.rotate(i * 60);
        p.drawRect(QRect(-2, -14, 4, 8));
        p.restore();
    }

    // Inner circle
    p.setBrush(Qt::white);
    p.drawEllipse(QRectF(10, 10, 12, 12));

    p.end();
    return QIcon(pix);
}

// Helper function to create config icon
static QIcon createConfigIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Gear/settings icon
    QPainterPath gear;
    gear.addEllipse(QRectF(6, 6, 20, 20));

    // Gear teeth
    for (int i = 0; i < 8; i++) {
        QRectF tooth(13, 2, 6, 6);
        p.save();
        p.translate(16, 16);
        p.rotate(i * 45);
        p.translate(-3, -14);
        p.fillRect(tooth, QColor(96, 125, 139));
        p.restore();
    }

    // Center hole
    p.setBrush(QColor(224, 224, 224));
    p.drawEllipse(QRectF(11, 11, 10, 10));

    p.end();
    return QIcon(pix);
}

// Helper function to create generic file icon
static QIcon createFileIcon() {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);

    // Simple file
    QPainterPath file;
    file.moveTo(8, 2);
    file.lineTo(20, 2);
    file.lineTo(26, 8);
    file.lineTo(26, 30);
    file.lineTo(6, 30);
    file.lineTo(6, 6);
    file.closeSubpath();
    p.fillPath(file, Qt::lightGray);
    p.setPen(QPen(Qt::gray, 1));
    p.drawPath(file);

    // Fold
    QPainterPath fold;
    fold.moveTo(20, 2);
    fold.lineTo(20, 8);
    fold.lineTo(26, 8);
    fold.closeSubpath();
    p.fillPath(fold, Qt::gray);

    p.end();
    return QIcon(pix);
}

// ============================================================================
// MainWindow Implementation
// ============================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_fileSystem(nullptr)
    , m_treeModel(nullptr)
    , m_listModel(nullptr)
    , m_proxyModel(nullptr)
    , m_treeView(nullptr)
    , m_listView(nullptr)
    , m_pathEdit(nullptr)
    , m_searchEdit(nullptr)
    , m_sortCombo(nullptr)
    , m_statusLabel(nullptr)
    , m_itemCountLabel(nullptr)
    , m_historyIndex(-1)
{
    setWindowTitle("Unix File System Explorer - Qt Version");
    setMinimumSize(1200, 700);
    setStyleSheet(
        "QMainWindow { background: #f3f5f9; }"
        "QToolBar { background: white; border: none; spacing: 4px; padding: 4px; }"
        "QToolButton { background: transparent; border: none; padding: 6px; border-radius: 4px; }"
        "QToolButton:hover { background: #e6f0fa; }"
        "QToolButton:pressed { background: #c8dcf0; }"
        "QLineEdit { padding: 6px 10px; border: 1px solid #c8c8c8; border-radius: 4px; background: white; }"
        "QLineEdit:focus { border: 1px solid #0078d4; }"
        "QTreeView { background: white; border: 1px solid #e0e0e0; border-radius: 4px; }"
        "QTableView { background: white; border: 1px solid #e0e0e0; border-radius: 4px; }"
        "QTableView::item:hover { background: #e6f0fa; }"
        "QTableView::item:selected { background: #c8dcf0; }"
        "QHeaderView::section { background: #f8f8f8; padding: 8px; border: none; border-bottom: 1px solid #e0e0e0; }"
        "QStatusBar { background: white; border-top: 1px solid #e0e0e0; }"
        "QLabel { color: #202020; }"
    );

    // Initialize filesystem
    m_diskPath = "filesystem.dat";
    m_fileSystem = new FileSystem();
    if (!m_fileSystem->mount(m_diskPath)) {
        QMessageBox::critical(this, "Error", "Failed to mount filesystem!");
    }

    setupUi();
    createActions();
    createMenus();
    createToolBar();

    m_treeModel = new FileSystemTreeModel(m_fileSystem, this);
    m_listModel = new FileSystemListModel(m_fileSystem, this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_listModel);

    m_treeView->setModel(m_treeModel);
    m_listView->setModel(m_proxyModel);

    connect(m_fileSystem, &FileSystem::filesystemChanged, this, &MainWindow::refreshView);
    connect(m_treeView, &QTreeView::clicked, this, &MainWindow::onTreeItemClicked);
    connect(m_listView, &QTableView::doubleClicked, this, &MainWindow::onListItemDoubleClicked);
    connect(m_pathEdit, &QLineEdit::returnPressed, this, &MainWindow::onPathReturnPressed);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearch);
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortChanged);
    connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);

    // Enable sorting on the proxy model
    m_listView->setSortingEnabled(true);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_treeModel->refresh();
    navigateTo("/");
}

MainWindow::~MainWindow() {
    if (m_fileSystem) {
        m_fileSystem->unmount();
        delete m_fileSystem;
        m_fileSystem = nullptr;
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Ensure filesystem is unmounted (saved) before closing
    if (m_fileSystem) {
        qDebug() << "Closing application - unmounting filesystem...";
        m_fileSystem->unmount();
    }
    event->accept();
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);

    // Path bar with search
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->setSpacing(8);

    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setText("/");
    m_pathEdit->setMinimumHeight(32);
    m_pathEdit->setPlaceholderText("Path");
    QFont pathFont = m_pathEdit->font();
    pathFont.setPointSize(12);
    m_pathEdit->setFont(pathFont);

    // Search box
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setMinimumHeight(32);
    m_searchEdit->setMaximumWidth(250);
    m_searchEdit->setPlaceholderText("Search files...");
    m_searchEdit->setClearButtonEnabled(true);
    QFont searchFont = m_searchEdit->font();
    searchFont.setPointSize(11);
    m_searchEdit->setFont(searchFont);
    m_searchEdit->setStyleSheet("QLineEdit { padding: 6px 10px; border: 1px solid #c8c8c8; border-radius: 4px; background: white; } QLineEdit:focus { border: 1px solid #0078d4; }");

    // Sort combo box
    m_sortCombo = new QComboBox(this);
    m_sortCombo->setMinimumHeight(32);
    m_sortCombo->setMaximumWidth(150);
    m_sortCombo->addItem("Sort by Name", Qt::DisplayRole);
    m_sortCombo->addItem("Sort by Size", Qt::UserRole + 1);
    m_sortCombo->addItem("Sort by Date", Qt::UserRole + 2);
    m_sortCombo->addItem("Sort by Type", Qt::UserRole + 3);
    m_sortCombo->setCurrentIndex(0);
    m_sortCombo->setStyleSheet("QComboBox { padding: 6px 10px; border: 1px solid #c8c8c8; border-radius: 4px; background: white; } QComboBox::dropDown { border: none; } QComboBox::down-arrow { width: 12px; }");

    pathLayout->addWidget(m_pathEdit);
    pathLayout->addStretch();
    pathLayout->addWidget(m_searchEdit);
    pathLayout->addWidget(m_sortCombo);
    mainLayout->addLayout(pathLayout);

    // Splitter for tree and list
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    m_treeView = new QTreeView(this);
    m_treeView->setHeaderHidden(false);
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(20);
    m_treeView->setSortingEnabled(false);
    m_treeView->setColumnWidth(0, 200);

    m_listView = new QTableView(this);
    m_listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setShowGrid(false);
    m_listView->verticalHeader()->setVisible(false);
    m_listView->horizontalHeader()->setStretchLastSection(true);
    m_listView->setColumnWidth(0, 300);
    m_listView->setColumnWidth(1, 80);
    m_listView->setColumnWidth(2, 80);
    m_listView->setColumnWidth(3, 180);
    m_listView->setColumnWidth(4, 100);

    // Context menu for list view
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_listView, &QTableView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu contextMenu(this);
        contextMenu.addAction(m_newFileAction);
        contextMenu.addAction(m_newFolderAction);
        contextMenu.addSeparator();
        contextMenu.addAction(m_copyAction);
        contextMenu.addAction(m_moveAction);
        contextMenu.addAction(m_renameAction);
        contextMenu.addSeparator();
        contextMenu.addAction(m_deleteAction);
        contextMenu.addSeparator();
        contextMenu.addAction(m_viewContentAction);
        contextMenu.addAction(m_permissionsAction);
        contextMenu.addSeparator();
        contextMenu.addAction(m_propertiesAction);
        contextMenu.exec(m_listView->viewport()->mapToGlobal(pos));
    });

    splitter->addWidget(m_treeView);
    splitter->addWidget(m_listView);
    splitter->setSizes({280, 920});

    mainLayout->addWidget(splitter, 1);

    // Status bar
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(16);

    m_itemCountLabel = new QLabel("0 items", this);
    m_statusLabel = new QLabel("Ready", this);

    statusLayout->addWidget(m_itemCountLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_statusLabel);

    QWidget* statusWidget = new QWidget(this);
    statusWidget->setLayout(statusLayout);
    statusWidget->setMinimumHeight(28);

    mainLayout->addWidget(statusWidget);

    setCentralWidget(centralWidget);

    // Status bar
    statusBar()->showMessage("Ready");
}

void MainWindow::createActions() {
    m_backAction = new QAction(QIcon::fromTheme("go-previous"), "Back", this);
    m_backAction->setShortcut(QKeySequence::Back);
    connect(m_backAction, &QAction::triggered, this, &MainWindow::onGoBack);

    m_upAction = new QAction(QIcon::fromTheme("go-up"), "Up", this);
    m_upAction->setShortcut(Qt::Key_Backspace);
    connect(m_upAction, &QAction::triggered, this, &MainWindow::onGoUp);

    m_refreshAction = new QAction(QIcon::fromTheme("view-refresh"), "Refresh", this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::onRefresh);

    m_newFolderAction = new QAction(QIcon::fromTheme("folder-new"), "New Folder", this);
    m_newFolderAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    connect(m_newFolderAction, &QAction::triggered, this, &MainWindow::onNewFolder);

    m_newFileAction = new QAction(QIcon::fromTheme("document-new"), "New File", this);
    m_newFileAction->setShortcut(Qt::CTRL | Qt::Key_N);
    connect(m_newFileAction, &QAction::triggered, this, &MainWindow::onNewFile);

    m_deleteAction = new QAction(QIcon::fromTheme("edit-delete"), "Delete", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setEnabled(false);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDelete);

    m_copyAction = new QAction(QIcon::fromTheme("edit-copy"), "Copy", this);
    m_copyAction->setShortcut(Qt::CTRL | Qt::Key_C);
    m_copyAction->setEnabled(false);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::onCopy);

    m_moveAction = new QAction(QIcon::fromTheme("transform-move"), "Move", this);
    m_moveAction->setShortcut(Qt::CTRL | Qt::Key_X);
    m_moveAction->setEnabled(false);
    connect(m_moveAction, &QAction::triggered, this, &MainWindow::onMove);

    m_renameAction = new QAction("Rename", this);
    m_renameAction->setShortcut(Qt::Key_F2);
    m_renameAction->setEnabled(false);
    connect(m_renameAction, &QAction::triggered, this, &MainWindow::onRename);

    m_viewContentAction = new QAction(QIcon::fromTheme("document-open"), "View Content", this);
    m_viewContentAction->setShortcut(Qt::CTRL | Qt::Key_O);
    m_viewContentAction->setEnabled(false);
    connect(m_viewContentAction, &QAction::triggered, this, &MainWindow::onViewContent);

    m_permissionsAction = new QAction("Permissions", this);
    m_permissionsAction->setEnabled(false);
    connect(m_permissionsAction, &QAction::triggered, this, &MainWindow::onChangePermissions);

    m_propertiesAction = new QAction("Properties", this);
    m_propertiesAction->setShortcut(Qt::ALT | Qt::Key_Return);
    m_propertiesAction->setEnabled(false);
    connect(m_propertiesAction, &QAction::triggered, this, &MainWindow::onShowProperties);

    m_storageAction = new QAction("Storage Info", this);
    connect(m_storageAction, &QAction::triggered, this, &MainWindow::onShowStorage);
}

void MainWindow::createMenus() {
    QMenuBar* menuBar = this->menuBar();
    menuBar->setStyleSheet("QMenuBar { background: white; border-bottom: 1px solid #e0e0e0; } "
                           "QMenuBar::item { padding: 6px 12px; } "
                           "QMenuBar::item:selected { background: #e6f0fa; } "
                           "QMenu { background: white; border: 1px solid #e0e0e0; } "
                           "QMenu::item:selected { background: #c8dcf0; }");

    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(m_newFileAction);
    fileMenu->addAction(m_newFolderAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_copyAction);
    fileMenu->addAction(m_renameAction);
    fileMenu->addAction(m_deleteAction);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);

    QMenu* editMenu = menuBar->addMenu("&Edit");
    editMenu->addAction(m_deleteAction);
    editMenu->addAction(m_renameAction);
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_moveAction);
    editMenu->addSeparator();
    editMenu->addAction(m_viewContentAction);
    editMenu->addAction(m_permissionsAction);
    editMenu->addSeparator();
    editMenu->addAction(m_propertiesAction);

    QMenu* viewMenu = menuBar->addMenu("&View");
    viewMenu->addAction(m_refreshAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_backAction);
    viewMenu->addAction(m_upAction);

    QMenu* toolsMenu = menuBar->addMenu("&Tools");
    toolsMenu->addAction(m_storageAction);

    QMenu* helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("About", this, &MainWindow::onAbout);
}

void MainWindow::createToolBar() {
    QToolBar* toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));

    toolbar->addAction(m_backAction);
    toolbar->addAction(m_upAction);
    toolbar->addSeparator();
    toolbar->addAction(m_refreshAction);
    toolbar->addSeparator();
    toolbar->addAction(m_newFolderAction);
    toolbar->addAction(m_newFileAction);
    toolbar->addAction(m_deleteAction);
}

void MainWindow::navigateTo(const QString& path) {
    QString fullPath = path;
    if (!path.startsWith("/") && path != "/") {
        fullPath = m_currentPath == "/" ? "/" + path : m_currentPath + "/" + path;
    }

    if (m_fileSystem->change_directory(fullPath)) {
        m_currentPath = m_fileSystem->get_current_path();
        m_pathEdit->setText(m_currentPath);

        // Update history
        if (m_historyIndex < m_history.size() - 1) {
            m_history = m_history.mid(0, m_historyIndex + 1);
        }
        m_history.append(m_currentPath);
        m_historyIndex = m_history.size() - 1;

        refreshView();
        setWindowTitle("Unix File System - " + m_currentPath);
    } else {
        QMessageBox::warning(this, "Navigation Error", "Cannot navigate to: " + fullPath);
    }
}

void MainWindow::refreshView() {
    if (m_treeModel) m_treeModel->refresh();
    if (m_listModel) {
        m_listModel->setDirectory(m_fileSystem->current_inode);
        m_listModel->refresh();
    }
    updateStatusBar();
}

void MainWindow::updateStatusBar() {
    auto entries = m_fileSystem->get_directory_entries(m_fileSystem->current_inode);
    int files = 0, folders = 0;

    for (const auto& e : entries) {
        std::string name(e.filename);
        if (name == "." || name == "..") continue;
        if (m_fileSystem->inode_table[e.inode_id].file_type == FileType::DIRECTORY) {
            folders++;
        } else {
            files++;
        }
    }

    int total = files + folders;
    m_itemCountLabel->setText(QString("%1 items (%2 files, %3 folders)").arg(total).arg(files).arg(folders));

    uint32_t freeBlocks = m_fileSystem->get_bitmap()->get_free_count();
    m_statusLabel->setText(QString("%1 blocks free").arg(freeBlocks));

    statusBar()->showMessage(QString("Current: %1").arg(m_currentPath));
}

void MainWindow::onTreeItemClicked(const QModelIndex& index) {
    if (!index.isValid()) return;

    uint32_t inode = m_treeModel->inodeFromIndex(index);
    if (inode == 0) {
        navigateTo("/");
    } else {
        // Use the tree model to build path from root to clicked node
        QString path = m_treeModel->buildPathFromIndex(index);
        if (!path.isEmpty()) {
            navigateTo(path);
        }
    }
}

void MainWindow::onListItemDoubleClicked(const QModelIndex& index) {
    if (!index.isValid()) return;

    // Get the actual row from source model (to handle sorting)
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    auto items = m_listModel->getItems();
    int row = sourceIndex.row();

    if (row >= 0 && row < items.size()) {
        const auto& item = items[row];
        if (item.isDirectory) {
            // Construct full path
            QString fullPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;
            navigateTo(fullPath);
        }
    }
}

void MainWindow::onPathReturnPressed() {
    QString path = m_pathEdit->text();
    navigateTo(path);
}

void MainWindow::onNewFolder() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Enter folder name:",
                                         QLineEdit::Normal, "New Folder", &ok);
    if (ok && !name.isEmpty()) {
        QString fullPath = m_currentPath == "/" ? "/" + name : m_currentPath + "/" + name;
        if (!m_fileSystem->create_directory(fullPath)) {
            QMessageBox::warning(this, "Error", "Failed to create folder!");
        }
    }
}

void MainWindow::onNewFile() {
    bool ok;
    QString name = QInputDialog::getText(this, "New File", "Enter file name:",
                                          QLineEdit::Normal, "NewFile.txt", &ok);
    if (ok && !name.isEmpty()) {
        QString fullPath = m_currentPath == "/" ? "/" + name : m_currentPath + "/" + name;
        bool sizeOk;

        // Max size with double indirect: 10 + 256 + 65536 = 65802 KB = 64.26 MB
        int maxSize = DIRECT_BLOCKS + POINTERS_PER_BLOCK + POINTERS_PER_BLOCK * POINTERS_PER_BLOCK;
        int size = QInputDialog::getInt(this, "File Size",
                                        QString("Enter file size (KB):\nMax size: %1 KB (%2 MB)").arg(maxSize).arg(maxSize/1024),
                                        1, 1, maxSize, 1, &sizeOk);
        if (sizeOk) {
            if (!m_fileSystem->create_file(fullPath, size)) {
                QMessageBox::warning(this, "Error", "Failed to create file!");
            }
        }
    }
}

void MainWindow::onDelete() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Delete", "Please select an item to delete.");
        return;
    }

    QModelIndex sourceIndex = m_proxyModel->mapToSource(selected.first());
    if (!sourceIndex.isValid()) return;

    auto items = m_listModel->getItems();
    if (sourceIndex.row() >= items.size()) return;

    const auto& item = items[sourceIndex.row()];
    QString fullPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;

    QString msg = item.isDirectory ? "Delete folder '" : "Delete file '";
    msg += item.name + "'?";

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", msg,
                                                             QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        bool success;
        if (item.isDirectory) {
            // Try recursive delete for non-empty directories
            success = m_fileSystem->delete_directory_recursive(fullPath);
            if (!success) {
                // Fallback to regular delete (empty directory)
                success = m_fileSystem->delete_directory(fullPath);
            }
        } else {
            success = m_fileSystem->delete_file(fullPath);
        }

        if (!success) {
            QMessageBox::warning(this, "Error", "Failed to delete item!");
        }
    }
}

void MainWindow::onRefresh() {
    refreshView();
}

void MainWindow::onShowStorage() {
    uint32_t tb = TOTAL_BLOCKS;
    uint32_t ub = m_fileSystem->get_bitmap()->get_used_count();
    uint32_t fb = m_fileSystem->get_bitmap()->get_free_count();
    double usedPct = 100.0 * ub / tb;
    double freePct = 100.0 * fb / tb;

    QString info = QString(
        "<h3>Storage Usage</h3>"
        "<table>"
        "<tr><td><b>Total Blocks:</b></td><td>%1</td></tr>"
        "<tr><td><b>Block Size:</b></td><td>%2 bytes</td></tr>"
        "<tr><td><b>Total Space:</b></td><td>%3 MB</td></tr>"
        "<tr><td colspan='2'>&nbsp;</td></tr>"
        "<tr><td><b>Used Blocks:</b></td><td>%4 (%5%)</td></tr>"
        "<tr><td><b>Free Blocks:</b></td><td>%6 (%7%)</td></tr>"
        "</table>"
    ).arg(tb).arg(BLOCK_SIZE).arg(TOTAL_SIZE / (1024*1024))
     .arg(ub).arg(usedPct, 0, 'f', 1)
     .arg(fb).arg(freePct, 0, 'f', 1);

    QMessageBox::about(this, "Storage Information", info);
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About",
        "<h3>Unix File System Explorer</h3>"
        "<p>Version 2.0 - Qt Version</p>"
        "<p>A Qt-based implementation of a Unix-style file system with i-node structure.</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>16 MB virtual disk</li>"
        "<li>10 direct + 1 indirect block pointers</li>"
        "<li>128-byte i-nodes</li>"
        "<li>Modern Fluent Design UI</li>"
        "</ul>");
}

void MainWindow::onGoBack() {
    if (m_historyIndex > 0) {
        m_historyIndex--;
        QString path = m_history[m_historyIndex];
        m_pathEdit->setText(path);
        m_fileSystem->change_directory(path);
        refreshView();
    }
}

void MainWindow::onGoUp() {
    if (m_currentPath != "/") {
        QString parent = m_currentPath;
        int lastSlash = parent.lastIndexOf('/');
        if (lastSlash > 0) {
            parent = parent.left(lastSlash);
        } else {
            parent = "/";
        }
        navigateTo(parent);
    }
}

void MainWindow::onSearch(const QString& text) {
    if (text.isEmpty()) {
        m_proxyModel->setFilterWildcard("");
    } else {
        m_proxyModel->setFilterWildcard("*" + text + "*");
    }
}

void MainWindow::onSortChanged(int index) {
    switch (index) {
        case 0: // Name
            m_proxyModel->setSortRole(Qt::DisplayRole);
            m_proxyModel->sort(0, Qt::AscendingOrder);
            break;
        case 1: // Size
            m_proxyModel->setSortRole(Qt::UserRole);
            m_proxyModel->sort(1, Qt::DescendingOrder);
            break;
        case 2: // Date
            m_proxyModel->setSortRole(Qt::UserRole + 1);
            m_proxyModel->sort(3, Qt::DescendingOrder);
            break;
        case 3: // Type
            m_proxyModel->setSortRole(Qt::UserRole + 2);
            m_proxyModel->sort(2, Qt::AscendingOrder);
            break;
    }
}

void MainWindow::onRename() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    QModelIndex index = m_proxyModel->mapToSource(selected.first());
    FileSystemListItem item = m_listModel->getItems().at(index.row());

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename",
        "Enter new name:", QLineEdit::Normal, item.name, &ok);

    if (ok && !newName.isEmpty() && newName != item.name) {
        // Validate filename
        if (newName.contains('/') || newName.contains('\\') || newName.contains(':') ||
            newName.contains('*') || newName.contains('?') || newName.contains('"') ||
            newName.contains('<') || newName.contains('>') || newName.contains('|')) {
            QMessageBox::warning(this, "Error", "Invalid filename! Special characters are not allowed.");
            return;
        }

        QString oldPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;
        QString newPath = m_currentPath == "/" ? "/" + newName : m_currentPath + "/" + newName;

        if (m_fileSystem->rename_item(oldPath, newPath)) {
            refreshView();
        } else {
            QMessageBox::warning(this, "Error", "Failed to rename item!");
        }
    }
}

void MainWindow::onCopy() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    QModelIndex index = m_proxyModel->mapToSource(selected.first());
    FileSystemListItem item = m_listModel->getItems().at(index.row());

    QString srcPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;

    bool ok;
    QString dstName = QInputDialog::getText(this, "Copy",
        "Enter destination name:", QLineEdit::Normal,
        item.isDirectory ? item.name + "_copy" : item.name + ".copy", &ok);

    if (!ok || dstName.isEmpty()) return;

    // Validate filename
    if (dstName.contains('/') || dstName.contains('\\') || dstName.contains(':') ||
        dstName.contains('*') || dstName.contains('?') || dstName.contains('"') ||
        dstName.contains('<') || dstName.contains('>') || dstName.contains('|')) {
        QMessageBox::warning(this, "Error", "Invalid filename! Special characters are not allowed.");
        return;
    }

    QString dstPath = m_currentPath == "/" ? "/" + dstName : m_currentPath + "/" + dstName;

    bool success;
    if (item.isDirectory) {
        success = m_fileSystem->copy_directory_recursive(srcPath, dstPath);
    } else {
        success = m_fileSystem->copy_file(srcPath, dstPath);
    }

    if (success) {
        QMessageBox::information(this, "Copy", "Copied to:\n" + dstPath);
        refreshView();
    } else {
        QMessageBox::warning(this, "Copy", "Failed to copy!");
    }
}

void MainWindow::onShowProperties() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    QModelIndex index = m_proxyModel->mapToSource(selected.first());
    FileSystemListItem item = m_listModel->getItems().at(index.row());

    QString fullPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;

    QString info = QString(
        "<table>"
        "<tr><td><b>Name:</b></td><td>%1</td></tr>"
        "<tr><td><b>Type:</b></td><td>%2</td></tr>"
        "<tr><td><b>Size:</b></td><td>%3</td></tr>"
        "<tr><td><b>Modified:</b></td><td>%4</td></tr>"
        "<tr><td><b>Permissions:</b></td><td>%5</td></tr>"
        "<tr><td><b>Inode:</b></td><td>%6</td></tr>"
        "<tr><td><b>Path:</b></td><td>%7</td></tr>"
        "</table>"
    ).arg(item.name)
     .arg(item.isDirectory ? "Directory" : item.fileExtension.isEmpty() ? "File" : item.fileExtension.toUpper())
     .arg(item.isDirectory ? "-" : formatSize(item.size))
     .arg(item.modified)
     .arg(item.permissions)
     .arg(item.inode)
     .arg(fullPath);

    QMessageBox::about(this, "Properties", info);
}

void MainWindow::onSelectionChanged() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        m_deleteAction->setEnabled(true);
        m_copyAction->setEnabled(true);
        m_moveAction->setEnabled(true);
        m_renameAction->setEnabled(true);
        m_propertiesAction->setEnabled(true);

        // Enable view content and permissions for files only
        QModelIndex index = m_proxyModel->mapToSource(selected.first());
        FileSystemListItem item = m_listModel->getItems().at(index.row());
        m_viewContentAction->setEnabled(!item.isDirectory);
        m_permissionsAction->setEnabled(true);
    } else {
        m_deleteAction->setEnabled(false);
        m_copyAction->setEnabled(false);
        m_moveAction->setEnabled(false);
        m_renameAction->setEnabled(false);
        m_propertiesAction->setEnabled(false);
        m_viewContentAction->setEnabled(false);
        m_permissionsAction->setEnabled(false);
    }
}

QString MainWindow::formatSize(uint32_t size) const {
    if (size < 1024) {
        return QString("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        return QString("%1 KB").arg(size / 1024);
    } else {
        return QString("%1 MB").arg(size / (1024.0 * 1024), 0, 'f', 2);
    }
}

void MainWindow::onMove() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    QModelIndex index = m_proxyModel->mapToSource(selected.first());
    FileSystemListItem item = m_listModel->getItems().at(index.row());

    QString srcPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;

    bool ok;
    QString dstName = QInputDialog::getText(this, "Move",
        "Enter destination path:", QLineEdit::Normal, item.name, &ok);

    if (!ok || dstName.isEmpty()) return;

    // Determine destination path
    QString dstPath;
    if (dstName.startsWith('/')) {
        // Absolute path
        dstPath = dstName;
    } else {
        // Relative path from current directory
        dstPath = m_currentPath == "/" ? "/" + dstName : m_currentPath + "/" + dstName;
    }

    // Validate destination path
    if (dstPath.contains("//") || dstPath.endsWith('/')) {
        dstPath.chop(1);
    }

    // Check if source and destination are the same
    if (srcPath == dstPath) {
        QMessageBox::warning(this, "Error", "Source and destination are the same!");
        return;
    }

    if (m_fileSystem->move_item(srcPath, dstPath)) {
        QMessageBox::information(this, "Move", "Moved to:\n" + dstPath);
        refreshView();
    } else {
        QMessageBox::warning(this, "Error", "Failed to move item!");
    }
}

void MainWindow::onViewContent() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    QModelIndex index = m_proxyModel->mapToSource(selected.first());
    FileSystemListItem item = m_listModel->getItems().at(index.row());

    if (item.isDirectory) return;

    QString fullPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;

    QByteArray content;
    if (!m_fileSystem->read_file_content(fullPath, content)) {
        QMessageBox::warning(this, "Error", "Failed to read file content!");
        return;
    }

    // Create a dialog to display the content
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("View: " + item.name);
    dialog->setMinimumSize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    QLabel* infoLabel = new QLabel(QString("Path: %1\nSize: %2").arg(fullPath).arg(formatSize(item.size)), dialog);
    layout->addWidget(infoLabel);

    QTextEdit* textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier New", 10));

    // Convert content to string (treat as ASCII)
    QString contentStr = QString::fromLatin1(content.constData(), content.size());
    textEdit->setPlainText(contentStr);

    layout->addWidget(textEdit);

    QPushButton* closeButton = new QPushButton("Close", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeButton);

    dialog->setLayout(layout);
    dialog->exec();
    delete dialog;
}

void MainWindow::onChangePermissions() {
    QModelIndexList selected = m_listView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    QModelIndex index = m_proxyModel->mapToSource(selected.first());
    FileSystemListItem item = m_listModel->getItems().at(index.row());

    QString fullPath = m_currentPath == "/" ? "/" + item.name : m_currentPath + "/" + item.name;

    // Get current permissions from inode
    auto [inode, pn] = m_fileSystem->resolve_path(fullPath);
    if (inode == UINT32_MAX) {
        QMessageBox::warning(this, "Error", "Could not find item!");
        return;
    }

    uint16_t currentPerm = m_fileSystem->inode_table[inode].permissions;

    // Create permissions dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Change Permissions: " + item.name);
    dialog->setMinimumWidth(350);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    layout->addWidget(new QLabel("Select permissions:", dialog));

    // Owner permissions
    QGroupBox* ownerBox = new QGroupBox("Owner", dialog);
    QHBoxLayout* ownerLayout = new QHBoxLayout(ownerBox);
    QCheckBox* ownerRead = new QCheckBox("Read", ownerBox);
    QCheckBox* ownerWrite = new QCheckBox("Write", ownerBox);
    QCheckBox* ownerExec = new QCheckBox("Execute", ownerBox);
    ownerRead->setChecked(currentPerm & 0x0100);
    ownerWrite->setChecked(currentPerm & 0x0080);
    ownerExec->setChecked(currentPerm & 0x0040);
    ownerLayout->addWidget(ownerRead);
    ownerLayout->addWidget(ownerWrite);
    ownerLayout->addWidget(ownerExec);
    ownerLayout->addStretch();
    ownerBox->setLayout(ownerLayout);
    layout->addWidget(ownerBox);

    // Group permissions
    QGroupBox* groupBox = new QGroupBox("Group", dialog);
    QHBoxLayout* groupLayout = new QHBoxLayout(groupBox);
    QCheckBox* groupRead = new QCheckBox("Read", groupBox);
    QCheckBox* groupWrite = new QCheckBox("Write", groupBox);
    QCheckBox* groupExec = new QCheckBox("Execute", groupBox);
    groupRead->setChecked(currentPerm & 0x0020);
    groupWrite->setChecked(currentPerm & 0x0010);
    groupExec->setChecked(currentPerm & 0x0008);
    groupLayout->addWidget(groupRead);
    groupLayout->addWidget(groupWrite);
    groupLayout->addWidget(groupExec);
    groupLayout->addStretch();
    groupBox->setLayout(groupLayout);
    layout->addWidget(groupBox);

    // Other permissions
    QGroupBox* otherBox = new QGroupBox("Others", dialog);
    QHBoxLayout* otherLayout = new QHBoxLayout(otherBox);
    QCheckBox* otherRead = new QCheckBox("Read", otherBox);
    QCheckBox* otherWrite = new QCheckBox("Write", otherBox);
    QCheckBox* otherExec = new QCheckBox("Execute", otherBox);
    otherRead->setChecked(currentPerm & 0x0004);
    otherWrite->setChecked(currentPerm & 0x0002);
    otherExec->setChecked(currentPerm & 0x0001);
    otherLayout->addWidget(otherRead);
    otherLayout->addWidget(otherWrite);
    otherLayout->addWidget(otherExec);
    otherLayout->addStretch();
    otherBox->setLayout(otherLayout);
    layout->addWidget(otherBox);

    // Quick presets
    QLabel* presetLabel = new QLabel("Quick presets:", dialog);
    layout->addWidget(presetLabel);

    QHBoxLayout* presetLayout = new QHBoxLayout();
    QPushButton* btn644 = new QPushButton("644", dialog);
    QPushButton* btn755 = new QPushButton("755", dialog);
    QPushButton* btn777 = new QPushButton("777", dialog);
    presetLayout->addWidget(btn644);
    presetLayout->addWidget(btn755);
    presetLayout->addWidget(btn777);
    presetLayout->addStretch();
    layout->addLayout(presetLayout);

    // Connect preset buttons
    connect(btn644, &QPushButton::clicked, [=]() {
        ownerRead->setChecked(true); ownerWrite->setChecked(true); ownerExec->setChecked(false);
        groupRead->setChecked(true); groupWrite->setChecked(false); groupExec->setChecked(false);
        otherRead->setChecked(true); otherWrite->setChecked(false); otherExec->setChecked(false);
    });
    connect(btn755, &QPushButton::clicked, [=]() {
        ownerRead->setChecked(true); ownerWrite->setChecked(true); ownerExec->setChecked(true);
        groupRead->setChecked(true); groupWrite->setChecked(false); groupExec->setChecked(true);
        otherRead->setChecked(true); otherWrite->setChecked(false); otherExec->setChecked(true);
    });
    connect(btn777, &QPushButton::clicked, [=]() {
        ownerRead->setChecked(true); ownerWrite->setChecked(true); ownerExec->setChecked(true);
        groupRead->setChecked(true); groupWrite->setChecked(true); groupExec->setChecked(true);
        otherRead->setChecked(true); otherWrite->setChecked(true); otherExec->setChecked(true);
    });

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", dialog);
    QPushButton* cancelButton = new QPushButton("Cancel", dialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, [=]() {
        // Calculate new permissions
        uint16_t newPerm = 0;
        if (ownerRead->isChecked()) newPerm |= 0x0100;
        if (ownerWrite->isChecked()) newPerm |= 0x0080;
        if (ownerExec->isChecked()) newPerm |= 0x0040;
        if (groupRead->isChecked()) newPerm |= 0x0020;
        if (groupWrite->isChecked()) newPerm |= 0x0010;
        if (groupExec->isChecked()) newPerm |= 0x0008;
        if (otherRead->isChecked()) newPerm |= 0x0004;
        if (otherWrite->isChecked()) newPerm |= 0x0002;
        if (otherExec->isChecked()) newPerm |= 0x0001;

        if (m_fileSystem->change_permissions(fullPath, newPerm)) {
            refreshView();
            dialog->accept();
        } else {
            QMessageBox::warning(this, "Error", "Failed to change permissions!");
        }
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->setLayout(layout);
    dialog->exec();
    delete dialog;
}

// ============================================================================
// FileSystemTreeModel Implementation
// ============================================================================
FileSystemTreeModel::FileSystemTreeModel(FileSystem* fs, QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootNode(nullptr)
    , m_fs(fs)
{
    buildTree();
}

void FileSystemTreeModel::buildTree() {
    // Clear existing tree
    delete m_rootNode;

    m_rootNode = new TreeNode();
    m_rootNode->inode = 0;
    m_rootNode->name = "This PC";
    m_rootNode->parent = nullptr;

    // Recursively build tree
    std::function<void(TreeNode*, uint32_t)> addChildren = [&](TreeNode* parent, uint32_t inode) {
        auto entries = m_fs->get_directory_entries(inode);
        for (const auto& e : entries) {
            std::string name(e.filename);
            if (name == "." || name == "..") continue;

            const INode& inode = m_fs->inode_table[e.inode_id];
            if (inode.file_type != FileType::DIRECTORY) continue;

            TreeNode* child = new TreeNode();
            child->inode = e.inode_id;
            child->name = QString::fromStdString(name);
            child->parent = parent;
            parent->children.append(child);

            addChildren(child, e.inode_id);
        }
    };

    addChildren(m_rootNode, 0);
}

QModelIndex FileSystemTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!m_rootNode || column != 0) return QModelIndex();

    TreeNode* parentNode = parent.isValid()
        ? static_cast<TreeNode*>(parent.internalPointer())
        : m_rootNode;

    if (row < 0 || row >= parentNode->children.size()) return QModelIndex();

    return createIndex(row, column, parentNode->children.at(row));
}

QModelIndex FileSystemTreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) return QModelIndex();

    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    if (!node || !node->parent || node->parent == m_rootNode) return QModelIndex();

    TreeNode* parentNode = node->parent;
    int row = parentNode->parent ? parentNode->parent->children.indexOf(parentNode) : 0;

    return createIndex(row, 0, parentNode);
}

int FileSystemTreeModel::rowCount(const QModelIndex& parent) const {
    if (!m_rootNode) return 0;

    TreeNode* parentNode = parent.isValid()
        ? static_cast<TreeNode*>(parent.internalPointer())
        : m_rootNode;

    return parentNode->children.size();
}

int FileSystemTreeModel::columnCount(const QModelIndex& /*parent*/) const {
    return 1;
}

QVariant FileSystemTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    if (!node) return QVariant();

    if (role == Qt::DisplayRole) {
        return node->name;
    }
    if (role == Qt::DecorationRole) {
        return createFolderIcon();
    }

    return QVariant();
}

Qt::ItemFlags FileSystemTreeModel::flags(const QModelIndex& /*index*/) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void FileSystemTreeModel::refresh() {
    beginResetModel();
    buildTree();
    endResetModel();
}

uint32_t FileSystemTreeModel::inodeFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) return 0;
    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    return node ? node->inode : 0;
}

QString FileSystemTreeModel::buildPathFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) return QString();

    // Build path by walking up the model index hierarchy
    QStringList parts;
    QModelIndex idx = index;

    while (idx.isValid()) {
        QString name = idx.data().toString();
        if (!name.isEmpty() && name != "This PC") {
            parts.prepend(name);
        }
        idx = idx.parent();
    }

    if (parts.isEmpty()) return "/";
    return "/" + parts.join("/");
}

// ============================================================================
// FileSystemListModel Implementation
// ============================================================================
FileSystemListModel::FileSystemListModel(FileSystem* fs, QObject* parent)
    : QAbstractTableModel(parent)
    , m_fs(fs)
    , m_currentInode(0)
{
    refresh();
}

int FileSystemListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : m_items.size();
}

int FileSystemListModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColCount;
}

QVariant FileSystemListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    int row = index.row();
    int col = index.column();

    if (row < 0 || row >= m_items.size()) return QVariant();

    const FileSystemListItem& item = m_items.at(row);

    if (role == Qt::DisplayRole) {
        switch (col) {
            case ColName: return item.name;
            case ColSize: return item.isDirectory ? QString("") : formatSize(item.size);
            case ColType: {
                if (item.isDirectory) return QString("Folder");
                switch (item.fileKind) {
                    case FileSystemListItem::KindDocument: return "Document";
                    case FileSystemListItem::KindCode: return "Code";
                    case FileSystemListItem::KindImage: return "Image";
                    case FileSystemListItem::KindAudio: return "Audio";
                    case FileSystemListItem::KindVideo: return "Video";
                    case FileSystemListItem::KindArchive: return "Archive";
                    case FileSystemListItem::KindExecutable: return "Executable";
                    case FileSystemListItem::KindConfig: return "Config";
                    default: return "File";
                }
            }
            case ColModified: return item.modified;
            case ColPermissions: return item.permissions;
        }
    }

    // Sort roles
    if (role == Qt::UserRole) {  // Size for sorting
        return item.isDirectory ? UINT_MAX : item.size;
    }
    if (role == Qt::UserRole + 1) {  // Date for sorting
        return item.modified;
    }
    if (role == Qt::UserRole + 2) {  // Type for sorting
        if (item.isDirectory) return 0;  // Folders first
        return (int)item.fileKind + 1;
    }
    if (role == Qt::UserRole + 3) {  // Name for sorting
        return item.name.toLower();
    }

    if (role == Qt::DecorationRole && col == ColName) {
        if (item.isDirectory) {
            return createFolderIcon();
        }
        switch (item.fileKind) {
            case FileSystemListItem::KindDocument:
                return createDocumentIcon();
            case FileSystemListItem::KindCode:
                return createCodeIcon();
            case FileSystemListItem::KindImage:
                return createImageIcon();
            case FileSystemListItem::KindAudio:
                return createAudioIcon();
            case FileSystemListItem::KindVideo:
                return createVideoIcon();
            case FileSystemListItem::KindArchive:
                return createArchiveIcon();
            case FileSystemListItem::KindExecutable:
                return createExecutableIcon();
            case FileSystemListItem::KindConfig:
                return createConfigIcon();
            default:
                return createFileIcon();
        }
    }

    return QVariant();
}

QString FileSystemListModel::formatSize(uint32_t size) const {
    if (size < 1024) {
        return QString("%1 B").arg(size);
    } else if (size < 1024 * 1024) {
        return QString("%1 KB").arg(size / 1024);
    } else {
        return QString("%1 MB").arg(size / (1024.0 * 1024), 0, 'f', 1);
    }
}

QVariant FileSystemListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return QVariant();

    switch (section) {
        case ColName: return "Name";
        case ColSize: return "Size";
        case ColType: return "Type";
        case ColModified: return "Modified";
        case ColPermissions: return "Permissions";
        default: return QVariant();
    }
}

Qt::ItemFlags FileSystemListModel::flags(const QModelIndex& /*index*/) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void FileSystemListModel::refresh() {
    beginResetModel();
    m_items.clear();

    auto entries = m_fs->get_directory_entries(m_currentInode);

    for (const auto& e : entries) {
        std::string name(e.filename);
        if (name == "." || name == "..") continue;

        const INode& inode = m_fs->inode_table[e.inode_id];
        bool isDir = inode.file_type == FileType::DIRECTORY;

        FileSystemListItem item;
        item.inode = e.inode_id;
        item.name = QString::fromStdString(name);
        item.isDirectory = isDir;
        item.size = inode.file_size;

        struct tm* tm_info = localtime(&inode.modified_time);
        char time_buf[32];
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", tm_info);
        item.modified = time_buf;

        // Format permissions
        item.permissions = isDir ? "drwxr-xr-x" : "-rw-r--r--";

        // Determine file extension and kind
        item.fileExtension = "";
        if (isDir) {
            item.fileKind = FileSystemListItem::KindFolder;
        } else {
            QString fullName = QString::fromStdString(name);
            int dotIndex = fullName.lastIndexOf('.');
            if (dotIndex > 0 && dotIndex < fullName.length() - 1) {
                item.fileExtension = fullName.mid(dotIndex + 1).toLower();
            }

            // Determine file kind based on extension
            if (item.fileExtension == "txt" || item.fileExtension == "doc" ||
                item.fileExtension == "docx" || item.fileExtension == "pdf" ||
                item.fileExtension == "md" || item.fileExtension == "rtf") {
                item.fileKind = FileSystemListItem::KindDocument;
            } else if (item.fileExtension == "cpp" || item.fileExtension == "c" ||
                       item.fileExtension == "h" || item.fileExtension == "hpp" ||
                       item.fileExtension == "py" || item.fileExtension == "java" ||
                       item.fileExtension == "js" || item.fileExtension == "html" ||
                       item.fileExtension == "css" || item.fileExtension == "sh") {
                item.fileKind = FileSystemListItem::KindCode;
            } else if (item.fileExtension == "png" || item.fileExtension == "jpg" ||
                       item.fileExtension == "jpeg" || item.fileExtension == "gif" ||
                       item.fileExtension == "bmp" || item.fileExtension == "svg" ||
                       item.fileExtension == "webp") {
                item.fileKind = FileSystemListItem::KindImage;
            } else if (item.fileExtension == "mp3" || item.fileExtension == "wav" ||
                       item.fileExtension == "flac" || item.fileExtension == "ogg" ||
                       item.fileExtension == "m3u") {
                item.fileKind = FileSystemListItem::KindAudio;
            } else if (item.fileExtension == "mp4" || item.fileExtension == "avi" ||
                       item.fileExtension == "mkv" || item.fileExtension == "mov" ||
                       item.fileExtension == "wmv") {
                item.fileKind = FileSystemListItem::KindVideo;
            } else if (item.fileExtension == "zip" || item.fileExtension == "tar" ||
                       item.fileExtension == "gz" || item.fileExtension == "rar" ||
                       item.fileExtension == "7z" || item.fileExtension == "tar.gz") {
                item.fileKind = FileSystemListItem::KindArchive;
            } else if (item.fileExtension == "exe" || item.fileExtension == "bat" ||
                       item.fileExtension == "cmd" || item.fileExtension == "msi") {
                item.fileKind = FileSystemListItem::KindExecutable;
            } else if (item.fileExtension == "conf" || item.fileExtension == "config" ||
                       item.fileExtension == "ini" || item.fileExtension == "log" ||
                       item.fileExtension == "xml" || item.fileExtension == "json" ||
                       item.fileExtension == "yaml" || item.fileExtension == "yml") {
                item.fileKind = FileSystemListItem::KindConfig;
            } else {
                item.fileKind = FileSystemListItem::KindOther;
            }
        }

        m_items.append(item);
    }

    endResetModel();
}

void FileSystemListModel::setDirectory(uint32_t inode) {
    m_currentInode = inode;
    refresh();
}

QVector<FileSystemListItem> FileSystemListModel::getItems() const {
    return m_items;
}

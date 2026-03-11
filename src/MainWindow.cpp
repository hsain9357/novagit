#include "MainWindow.h"
#include "CommitItemWidget.h"
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QInputDialog>

#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    gitManager = new GitManager(this);
    aiHandler = new AIHandler(this);
    connect(aiHandler, &AIHandler::messageGenerated, this, &MainWindow::onAIMessageGenerated);
    connect(aiHandler, &AIHandler::errorOccurred, this, &MainWindow::onAIError);
    setupUi();
    loadSettings();
    if (!gitManager->repositoryPath().isEmpty()) {
        updateRecentFolders(gitManager->repositoryPath());
    }
    refreshStatus();
}

MainWindow::~MainWindow() {
    saveSettings();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == commitMessageEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return && (keyEvent->modifiers() & Qt::ControlModifier)) {
            commitChanges();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Left Sidebar
    QWidget *sidebar = new QWidget();
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
    sidebarLayout->setSpacing(8);

    QPushButton *openBtn = new QPushButton("Open Folder");
    openBtn->setObjectName("secondaryBtn");
    sidebarLayout->addWidget(openBtn);

    QLabel *recentLabel = new QLabel("Recent Repositories");
    sidebarLayout->addWidget(recentLabel);
    recentFoldersCombo = new QComboBox();
    recentFoldersCombo->setPlaceholderText("Select repository...");
    sidebarLayout->addWidget(recentFoldersCombo);

    // Sidebar Splitter for sections
    QSplitter *sidebarSplitter = new QSplitter(Qt::Vertical);
    sidebarSplitter->setStyleSheet("QSplitter::handle { background: transparent; }");

    // Staged Section
    QWidget *stagedContainer = new QWidget();
    QVBoxLayout *stagedLayout = new QVBoxLayout(stagedContainer);
    stagedLayout->setContentsMargins(0, 0, 0, 0);
    stagedLayout->setSpacing(4);
    QLabel *stagedLabel = new QLabel("Staged Changes");
    stagedLayout->addWidget(stagedLabel);
    stagedList = new QListWidget();
    stagedList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    stagedLayout->addWidget(stagedList);
    sidebarSplitter->addWidget(stagedContainer);

    // Unstaged Section
    QWidget *unstagedContainer = new QWidget();
    QVBoxLayout *unstagedLayout = new QVBoxLayout(unstagedContainer);
    unstagedLayout->setContentsMargins(0, 0, 0, 0);
    unstagedLayout->setSpacing(4);
    QLabel *unstagedLabel = new QLabel("Changes");
    unstagedLayout->addWidget(unstagedLabel);
    unstagedList = new QListWidget();
    unstagedList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    unstagedLayout->addWidget(unstagedList);
    sidebarSplitter->addWidget(unstagedContainer);

    // Log Section
    QWidget *logContainer = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout(logContainer);
    logLayout->setContentsMargins(0, 0, 0, 0);
    logLayout->setSpacing(4);
    QLabel *logLabel = new QLabel("Git History");
    logLayout->addWidget(logLabel);
    logList = new QListWidget();
    logList->setSelectionMode(QAbstractItemView::NoSelection);
    logList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    logList->setStyleSheet("QListWidget { background-color: #1e1e1e; } QListWidget::item { border-bottom: 1px solid #2d2d2d; }");
    logLayout->addWidget(logList);
    sidebarSplitter->addWidget(logContainer);

    sidebarSplitter->setStretchFactor(0, 1);
    sidebarSplitter->setStretchFactor(1, 1);
    sidebarSplitter->setStretchFactor(2, 2);

    sidebarLayout->addWidget(sidebarSplitter, 1);

    commitMessageEdit = new QTextEdit();
    commitMessageEdit->setPlaceholderText("Commit message (Ctrl+Enter to commit)");
    commitMessageEdit->setMaximumHeight(80);
    commitMessageEdit->installEventFilter(this);
    sidebarLayout->addWidget(commitMessageEdit);

    QHBoxLayout *commitBtnLayout = new QHBoxLayout();
    generateBtn = new QPushButton("Generate Message");
    generateBtn->setObjectName("secondaryBtn");
    QPushButton *commitBtn = new QPushButton("Commit");
    commitBtn->setObjectName("primaryBtn");
    commitBtnLayout->addWidget(generateBtn);
    commitBtnLayout->addWidget(commitBtn);
    sidebarLayout->addLayout(commitBtnLayout);

    QHBoxLayout *syncLayout = new QHBoxLayout();
    QPushButton *pullBtn = new QPushButton("Pull");
    QPushButton *pushBtn = new QPushButton("Push");
    syncLayout->addWidget(pullBtn);
    syncLayout->addWidget(pushBtn);
    sidebarLayout->addLayout(syncLayout);

    // Right Content
    QWidget *rightContainer = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    diffTabBar = new QTabBar();
    diffTabBar->setTabsClosable(true);
    diffTabBar->setMovable(true);
    diffTabBar->setStyleSheet("QTabBar::tab { background: #2d2d2d; color: #aaa; padding: 8px 12px; border: 1px solid #1e1e1e; }"
                              "QTabBar::tab:selected { background: #1e1e1e; color: #fff; border-bottom: 2px solid #007acc; }");
    rightLayout->addWidget(diffTabBar);

    diffView = new DiffView();
    rightLayout->addWidget(diffView);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(sidebar);
    splitter->addWidget(rightContainer);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    // Connect signals
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::openFolder);
    connect(recentFoldersCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRecentFolderSelected);
    connect(diffTabBar, &QTabBar::currentChanged, this, &MainWindow::onTabChanged);
    connect(diffTabBar, &QTabBar::tabCloseRequested, this, &MainWindow::closeTab);
    connect(unstagedList, &QListWidget::itemClicked, this, &MainWindow::onFileSelected);
    connect(stagedList, &QListWidget::itemClicked, this, &MainWindow::onFileSelected);
    connect(unstagedList, &QListWidget::itemDoubleClicked, this, &MainWindow::stageSelected);
    connect(stagedList, &QListWidget::itemDoubleClicked, this, &MainWindow::unstageSelected);
    connect(generateBtn, &QPushButton::clicked, this, &MainWindow::generateAICommitMessage);
    connect(commitBtn, &QPushButton::clicked, this, &MainWindow::commitChanges);
    connect(pullBtn, &QPushButton::clicked, this, &MainWindow::pullChanges);
    connect(pushBtn, &QPushButton::clicked, this, &MainWindow::pushChanges);
}

void MainWindow::openFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Git Repository", gitManager->repositoryPath());
    if (!dir.isEmpty()) {
        gitManager->setRepositoryPath(dir);
        updateRecentFolders(dir);
        refreshStatus();
        diffView->clear();
    }
}

void MainWindow::onRecentFolderSelected(int index) {
    if (index < 0) return;
    QString dir = recentFoldersCombo->itemText(index);
    if (!dir.isEmpty() && dir != gitManager->repositoryPath()) {
        gitManager->setRepositoryPath(dir);
        refreshStatus();
        diffView->clear();
    }
}

void MainWindow::loadSettings() {
    QSettings settings("GitGUIApp", "GitGUIApp");
    QString lastDir = settings.value("lastFolder").toString();
    if (!lastDir.isEmpty()) {
        gitManager->setRepositoryPath(lastDir);
    }

    QStringList recentFolders = settings.value("recentFolders").toStringList();
    recentFoldersCombo->blockSignals(true);
    recentFoldersCombo->addItems(recentFolders);
    if (!lastDir.isEmpty()) {
        int idx = recentFoldersCombo->findText(lastDir);
        if (idx >= 0) {
            recentFoldersCombo->setCurrentIndex(idx);
        }
    }
    recentFoldersCombo->blockSignals(false);
}

void MainWindow::saveSettings() {
    QSettings settings("GitGUIApp", "GitGUIApp");
    settings.setValue("lastFolder", gitManager->repositoryPath());
    
    QStringList recentFolders;
    for (int i = 0; i < recentFoldersCombo->count(); ++i) {
        recentFolders << recentFoldersCombo->itemText(i);
    }
    settings.setValue("recentFolders", recentFolders);
}

void MainWindow::updateRecentFolders(const QString &path) {
    recentFoldersCombo->blockSignals(true);
    int idx = recentFoldersCombo->findText(path);
    if (idx >= 0) {
        recentFoldersCombo->removeItem(idx);
    }
    recentFoldersCombo->insertItem(0, path);
    recentFoldersCombo->setCurrentIndex(0);
    
    // Keep only last 10
    while (recentFoldersCombo->count() > 10) {
        recentFoldersCombo->removeItem(recentFoldersCombo->count() - 1);
    }
    recentFoldersCombo->blockSignals(false);
}

void MainWindow::refreshStatus() {
    setWindowTitle("Git GUI - " + gitManager->repositoryPath());
    stagedList->clear();
    unstagedList->clear();
    auto statuses = gitManager->getStatus();
    for (const auto &s : statuses) {
        QListWidgetItem *item = new QListWidgetItem(s.filePath);
        item->setData(Qt::UserRole, s.staged);
        if (s.staged) {
            stagedList->addItem(item);
        } else {
            unstagedList->addItem(item);
        }
    }
    refreshLog();
}

void MainWindow::refreshLog() {
    logList->clear();
    auto commits = gitManager->getLog(20);
    for (const auto &commit : commits) {
        QListWidgetItem *item = new QListWidgetItem(logList);
        CommitItemWidget *widget = new CommitItemWidget(commit);
        item->setSizeHint(widget->sizeHint());
        logList->addItem(item);
        logList->setItemWidget(item, widget);
        
        connect(widget, &CommitItemWidget::checkoutRequested, this, &MainWindow::checkoutCommit);
        connect(widget, &CommitItemWidget::resetRequested, this, &MainWindow::resetCommit);
        // connect(widget, &CommitItemWidget::commitSelected, this, &MainWindow::showCommitDiff); // Disable auto-diff
        connect(widget, &CommitItemWidget::fileSelectedInCommit, this, &MainWindow::showFileDiffInCommit);
        connect(widget, &CommitItemWidget::sizeChanged, [item, widget]() {
            item->setSizeHint(widget->sizeHint());
        });
    }
}

void MainWindow::showCommitDiff(const QString &hash) {
    QString diff = gitManager->getCommitDiff(hash);
    diffView->setUnifiedDiff(diff);
}

void MainWindow::showFileDiffInCommit(const QString &hash, const QString &filePath) {
    addOrActivateTab(filePath, hash, false);
}

void MainWindow::checkoutCommit(const QString &hash) {

    if (gitManager->checkout(hash)) {
        refreshStatus();
        diffView->clear();
    }
}

void MainWindow::resetCommit(const QString &hash, bool hard) {
    if (hard) {
        auto result = QMessageBox::question(this, "Reset Hard", 
            "Are you sure you want to perform a hard reset? All uncommitted changes will be lost.",
            QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes) return;
    }
    
    if (gitManager->reset(hash, hard)) {
        refreshStatus();
        diffView->clear();
    }
}

void MainWindow::onFileSelected(QListWidgetItem *item) {
    bool staged = item->data(Qt::UserRole).toBool();
    addOrActivateTab(item->text(), "", staged);
}

void MainWindow::stageSelected() {
    QListWidgetItem *item = unstagedList->currentItem();
    if (item) {
        gitManager->stageFile(item->text());
        refreshStatus();
    }
}

void MainWindow::unstageSelected() {
    QListWidgetItem *item = stagedList->currentItem();
    if (item) {
        gitManager->unstageFile(item->text());
        refreshStatus();
    }
}

void MainWindow::commitChanges() {
    QString msg = commitMessageEdit->toPlainText();
    if (msg.isEmpty()) {
        QMessageBox::warning(this, "Error", "Commit message cannot be empty");
        return;
    }
    gitManager->commit(msg);
    commitMessageEdit->clear();
    refreshStatus();
}

void MainWindow::generateAICommitMessage() {
    QString diff = gitManager->getStagedDiff();
    if (diff.isEmpty()) {
        // If no staged changes, try to stage everything
        gitManager->stageAll();
        refreshStatus();
        diff = gitManager->getStagedDiff();
    }

    if (diff.isEmpty()) {
        QMessageBox::warning(this, "No Changes", "No changes found to stage or generate a message for.");
        return;
    }

    generateBtn->setEnabled(false);
    generateBtn->setText("Generating...");
    aiHandler->generateCommitMessage(diff);
}

void MainWindow::onAIMessageGenerated(const QString &message) {
    commitMessageEdit->setPlainText(message);
    generateBtn->setEnabled(true);
    generateBtn->setText("Generate");
}

void MainWindow::onAIError(const QString &error) {
    generateBtn->setEnabled(true);
    generateBtn->setText("Generate");

    if (error == "API Key required") {
        bool ok;
        QString key = QInputDialog::getText(this, "Gemini API Key",
                                            "Enter your Gemini API key:", QLineEdit::Normal,
                                            "", &ok);
        if (ok && !key.isEmpty()) {
            // Re-call the save logic in AIHandler
            // Actually I should expose a method for this
            // For now let's just use the file directly or add a method to AIHandler
            QFile file(QDir::homePath() + "/.gemini_git_key");
            if (file.open(QFile::WriteOnly | QFile::Text)) {
                file.write(key.toUtf8());
                file.close();
                generateAICommitMessage(); // Retry
            }
        }
    } else {
        QMessageBox::critical(this, "AI Error", error);
    }
}

void MainWindow::pushChanges() {
    gitManager->push();
    refreshStatus();
}

void MainWindow::pullChanges() {
    gitManager->pull();
    refreshStatus();
}

void MainWindow::addOrActivateTab(const QString &filePath, const QString &hash, bool staged) {
    for (int i = 0; i < openTabs.size(); ++i) {
        if (openTabs[i].filePath == filePath && openTabs[i].hash == hash && openTabs[i].staged == staged) {
            diffTabBar->setCurrentIndex(i);
            onTabChanged(i);
            return;
        }
    }

    TabInfo info = {filePath, hash, staged};
    openTabs.append(info);
    QString label = QFileInfo(filePath).fileName();
    if (!hash.isEmpty()) label += " (" + hash.left(7) + ")";
    int index = diffTabBar->addTab(label);
    diffTabBar->setTabToolTip(index, filePath);
    diffTabBar->setCurrentIndex(index);
    onTabChanged(index);
}

void MainWindow::onTabChanged(int index) {
    if (index < 0 || index >= openTabs.size()) {
        diffView->clear();
        return;
    }

    const auto &info = openTabs[index];
    if (info.hash.isEmpty()) {
        QString original = gitManager->getFileContent(info.filePath, info.staged);
        QString working = gitManager->getWorkingFileContent(info.filePath);
        auto hunks = gitManager->getHunks(info.filePath, info.staged);
        diffView->setDiff(original, working, hunks);
    } else {
        QString oldContent = gitManager->getFileContentAtRevision(info.filePath, info.hash + "^");
        QString newContent = gitManager->getFileContentAtRevision(info.filePath, info.hash);
        auto hunks = gitManager->getHunksForCommit(info.hash, info.filePath);
        diffView->setDiff(oldContent, newContent, hunks);
    }
}

void MainWindow::closeTab(int index) {
    if (index < 0 || index >= openTabs.size()) return;
    openTabs.removeAt(index);
    diffTabBar->removeTab(index);
    if (openTabs.isEmpty()) {
        diffView->clear();
    }
}

#include "MainWindow.h"
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    gitManager = new GitManager(this);
    setupUi();
    refreshStatus();
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Left Sidebar
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(300);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);

    sidebarLayout->addWidget(new QLabel("STAGED CHANGES"));
    stagedList = new QListWidget();
    sidebarLayout->addWidget(stagedList);

    sidebarLayout->addWidget(new QLabel("CHANGES"));
    unstagedList = new QListWidget();
    sidebarLayout->addWidget(unstagedList);

    commitMessageEdit = new QLineEdit();
    commitMessageEdit->setPlaceholderText("Message (Ctrl+Enter to commit)");
    sidebarLayout->addWidget(commitMessageEdit);

    QPushButton *commitBtn = new QPushButton("Commit");
    sidebarLayout->addWidget(commitBtn);

    QHBoxLayout *syncLayout = new QHBoxLayout();
    QPushButton *pullBtn = new QPushButton("Pull");
    QPushButton *pushBtn = new QPushButton("Push");
    syncLayout->addWidget(pullBtn);
    syncLayout->addWidget(pushBtn);
    sidebarLayout->addLayout(syncLayout);

    // Right Content
    diffView = new DiffView();

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(sidebar);
    splitter->addWidget(diffView);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    // Connect signals
    connect(unstagedList, &QListWidget::itemClicked, this, &MainWindow::onFileSelected);
    connect(stagedList, &QListWidget::itemClicked, this, &MainWindow::onFileSelected);
    connect(unstagedList, &QListWidget::itemDoubleClicked, this, &MainWindow::stageSelected);
    connect(stagedList, &QListWidget::itemDoubleClicked, this, &MainWindow::unstageSelected);
    connect(commitBtn, &QPushButton::clicked, this, &MainWindow::commitChanges);
    connect(pullBtn, &QPushButton::clicked, this, &MainWindow::pullChanges);
    connect(pushBtn, &QPushButton::clicked, this, &MainWindow::pushChanges);
}

void MainWindow::refreshStatus() {
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
}

void MainWindow::onFileSelected(QListWidgetItem *item) {
    bool staged = item->data(Qt::UserRole).toBool();
    QString diff = gitManager->getDiff(item->text(), staged);
    diffView->setDiff(diff);
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
    QString msg = commitMessageEdit->text();
    if (msg.isEmpty()) {
        QMessageBox::warning(this, "Error", "Commit message cannot be empty");
        return;
    }
    gitManager->commit(msg);
    commitMessageEdit->clear();
    refreshStatus();
}

void MainWindow::pushChanges() {
    gitManager->push();
    refreshStatus();
}

void MainWindow::pullChanges() {
    gitManager->pull();
    refreshStatus();
}

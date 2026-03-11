#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSettings>
#include <QTabBar>
#include "GitManager.h"
#include "DiffView.h"
#include "AIHandler.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void openFolder();
    void onRecentFolderSelected(int index);
    void refreshStatus();
    void refreshLog();
    void onFileSelected(QListWidgetItem *item);
    void showCommitDiff(const QString &hash);
    void showFileDiffInCommit(const QString &hash, const QString &filePath);
    void checkoutCommit(const QString &hash);
    void resetCommit(const QString &hash, bool hard);
    void generateAICommitMessage();
    void onAIMessageGenerated(const QString &message);
    void onAIError(const QString &error);
    void stageSelected();
    void unstageSelected();
    void commitChanges();
    void pushChanges();
    void pullChanges();
    void onTabChanged(int index);
    void closeTab(int index);

private:
    struct TabInfo {
        QString filePath;
        QString hash; // Empty for working/staged
        bool staged;
    };
    QList<TabInfo> openTabs;
    QTabBar *diffTabBar;

    GitManager *gitManager;
    QListWidget *stagedList;
    QListWidget *unstagedList;
    QListWidget *logList;
    DiffView *diffView;
    QTextEdit *commitMessageEdit;
    QPushButton *generateBtn;
    QComboBox *recentFoldersCombo;
    AIHandler *aiHandler;
    
    void setupUi();
    void loadSettings();
    void saveSettings();
    void updateRecentFolders(const QString &path);
    void addOrActivateTab(const QString &filePath, const QString &hash, bool staged);
};

#endif // MAINWINDOW_H

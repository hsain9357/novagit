#include "GitManager.h"
#include <QDebug>
#include <QDir>

GitManager::GitManager(QObject *parent) : QObject(parent) {
    m_repositoryPath = QDir::currentPath();
}

void GitManager::setRepositoryPath(const QString &path) {
    m_repositoryPath = path;
}

QString GitManager::repositoryPath() const {
    return m_repositoryPath;
}

QString GitManager::runGitCommand(const QStringList &arguments) {
    QProcess process;
    process.setWorkingDirectory(m_repositoryPath);
    process.start("git", arguments);
    if (!process.waitForFinished()) {
        return QString();
    }
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

QList<GitFileStatus> GitManager::getStatus() {
    QList<GitFileStatus> fileStatuses;
    QString output = runGitCommand({"status", "--porcelain"});
    if (output.isEmpty()) return fileStatuses;

    QStringList lines = output.split('\n');
    for (const QString &line : lines) {
        if (line.length() < 4) continue;
        
        QString indexStatus = line.mid(0, 1);
        QString workingTreeStatus = line.mid(1, 1);
        QString filePath = line.mid(3);

        // Staged
        if (indexStatus != " " && indexStatus != "?") {
            fileStatuses.append({filePath, indexStatus, true});
        }
        // Unstaged
        if (workingTreeStatus != " " || indexStatus == "?") {
            fileStatuses.append({filePath, workingTreeStatus == " " ? "??" : workingTreeStatus, false});
        }
    }
    return fileStatuses;
}

QString GitManager::getDiff(const QString &filePath, bool staged) {
    QStringList args = {"diff"};
    if (staged) args << "--cached";
    args << "--" << filePath;
    return runGitCommand(args);
}

bool GitManager::stageFile(const QString &filePath) {
    runGitCommand({"add", filePath});
    return true;
}

bool GitManager::unstageFile(const QString &filePath) {
    runGitCommand({"reset", "HEAD", "--", filePath});
    return true;
}

bool GitManager::commit(const QString &message) {
    runGitCommand({"commit", "-m", message});
    return true;
}

bool GitManager::push() {
    runGitCommand({"push"});
    return true;
}

bool GitManager::pull() {
    runGitCommand({"pull"});
    return true;
}

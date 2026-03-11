#include "GitManager.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

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

QString GitManager::getFileContent(const QString &filePath, bool staged) {
    if (staged) {
        // For staged changes, we want the version in the HEAD (the "old" version)
        return runGitCommand({"show", "HEAD:" + filePath});
    } else {
        // For unstaged changes, we want the version in the index (the "old" version)
        return runGitCommand({"show", ":" + filePath});
    }
}

QString GitManager::getWorkingFileContent(const QString &filePath, bool staged) {
    if (staged) {
        // For staged changes, the "new" version is what's in the index
        return runGitCommand({"show", ":" + filePath});
    }
    // For unstaged changes, the "new" version is what's on disk
    QFile file(m_repositoryPath + "/" + filePath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        return QString::fromUtf8(file.readAll());
    }
    return QString();
}
QString GitManager::getFileContentAtRevision(const QString &filePath, const QString &revision) {
    return runGitCommand({"show", revision + ":" + filePath});
}

QList<GitHunk> GitManager::getHunks(const QString &filePath, bool staged) {
    QList<GitHunk> hunks;
    QStringList args = {"diff", "-U0"};
    if (staged) args << "--cached";
    args << "--" << filePath;
    QString output = runGitCommand(args);
    if (output.isEmpty()) return hunks;

    QStringList lines = output.split('\n');
    GitHunk currentHunk;
    bool inHunk = false;

    for (const QString &line : lines) {
        if (line.startsWith("@@")) {
            if (inHunk) hunks.append(currentHunk);
            inHunk = true;
            currentHunk = GitHunk();
            
            // Format: @@ -oldStart,oldLines +newStart,newLines @@
            QRegularExpression re("@@ -(\\d+)(?:,(\\d+))? \\+(\\d+)(?:,(\\d+))? @@");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                currentHunk.oldStart = match.captured(1).toInt();
                currentHunk.oldLines = match.captured(2).isEmpty() ? 1 : match.captured(2).toInt();
                currentHunk.newStart = match.captured(3).toInt();
                currentHunk.newLines = match.captured(4).isEmpty() ? 1 : match.captured(4).toInt();
            }
        } else if (inHunk && (line.startsWith("+") || line.startsWith("-") || line.startsWith(" "))) {
            currentHunk.lines.append(line);
        }
    }
    if (inHunk) hunks.append(currentHunk);
    return hunks;
}

QList<GitHunk> GitManager::getHunksForCommit(const QString &hash, const QString &filePath) {
    QList<GitHunk> hunks;
    // Compare with parent
    QString output = runGitCommand({"diff", "-U0", hash + "^", hash, "--", filePath});
    if (output.isEmpty()) return hunks;

    QStringList lines = output.split('\n');
    GitHunk currentHunk;
    bool inHunk = false;

    for (const QString &line : lines) {
        if (line.startsWith("@@")) {
            if (inHunk) hunks.append(currentHunk);
            inHunk = true;
            currentHunk = GitHunk();
            
            QRegularExpression re("@@ -(\\d+)(?:,(\\d+))? \\+(\\d+)(?:,(\\d+))? @@");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                currentHunk.oldStart = match.captured(1).toInt();
                currentHunk.oldLines = match.captured(2).isEmpty() ? 1 : match.captured(2).toInt();
                currentHunk.newStart = match.captured(3).toInt();
                currentHunk.newLines = match.captured(4).isEmpty() ? 1 : match.captured(4).toInt();
            }
        } else if (inHunk && (line.startsWith("+") || line.startsWith("-") || line.startsWith(" "))) {
            currentHunk.lines.append(line);
        }
    }
    if (inHunk) hunks.append(currentHunk);
    return hunks;
}

QString GitManager::getDiff(const QString &filePath, bool staged) {
    QStringList args = {"diff"};
    if (staged) args << "--cached";
    args << "--" << filePath;
    return runGitCommand(args);
}

QString GitManager::getStagedDiff() {
    return runGitCommand({"diff", "--cached"});
}

QString GitManager::getCommitDiff(const QString &hash) {
    // -U0 for compact diff, or regular diff for full context
    return runGitCommand({"show", hash, "--format="}); // --format= removes the commit message header
}

QString GitManager::getFileDiffInCommit(const QString &hash, const QString &filePath) {
    return runGitCommand({"show", hash, "--format=", "--", filePath});
}

QStringList GitManager::getChangedFiles(const QString &hash) {
    QString output = runGitCommand({"show", "--format=", "--name-only", hash});
    return output.split('\n', Qt::SkipEmptyParts);
}

bool GitManager::stageFile(const QString &filePath) {
    runGitCommand({"add", filePath});
    return true;
}

bool GitManager::unstageFile(const QString &filePath) {
    runGitCommand({"reset", "HEAD", "--", filePath});
    return true;
}

bool GitManager::stageAll() {
    runGitCommand({"add", "."});
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

QList<GitCommit> GitManager::getLog(int limit) {
    QList<GitCommit> commits;
    // Format: hash||author||date||subject||body<END_COMMIT>
    QString format = "%H||%an||%ad||%s||%b<END_COMMIT>";
    QString output = runGitCommand({"log", "-n", QString::number(limit), "--date=short", "--format=" + format});
    
    if (output.isEmpty()) return commits;

    QStringList entries = output.split("<END_COMMIT>", Qt::SkipEmptyParts);
    for (const QString &entry : entries) {
        QString trimmedEntry = entry.trimmed();
        if (trimmedEntry.isEmpty()) continue;
        
        QStringList parts = trimmedEntry.split("||");
        if (parts.size() >= 4) {
            GitCommit commit;
            commit.hash = parts[0];
            commit.author = parts[1];
            commit.date = parts[2];
            commit.subject = parts[3];
            if (parts.size() >= 5) {
                commit.message = parts[4].trimmed();
            }
            commit.changedFiles = getChangedFiles(commit.hash);
            commits.append(commit);
        }
    }
    return commits;
}

bool GitManager::checkout(const QString &ref) {
    runGitCommand({"checkout", ref});
    return true;
}

bool GitManager::reset(const QString &ref, bool hard) {
    QStringList args = {"reset"};
    if (hard) args << "--hard";
    else args << "--soft";
    args << ref;
    runGitCommand(args);
    return true;
}

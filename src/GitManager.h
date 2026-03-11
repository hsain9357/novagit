#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <QObject>
#include <QStringList>
#include <QProcess>

struct GitFileStatus {
    QString filePath;
    QString status; // "M", "A", "D", "??", etc.
    bool staged;
};

struct GitHunk {
    int oldStart, oldLines;
    int newStart, newLines;
    QStringList lines;
};

struct GitCommit {
    QString hash;
    QString author;
    QString date;
    QString message;
    QString subject;
};

class GitManager : public QObject {
    Q_OBJECT
public:
    explicit GitManager(QObject *parent = nullptr);

    void setRepositoryPath(const QString &path);
    QString repositoryPath() const;

    QList<GitFileStatus> getStatus();
    QString getFileContent(const QString &filePath, bool staged); // Get original version from git
    QString getWorkingFileContent(const QString &filePath);       // Get current file content
    QList<GitHunk> getHunks(const QString &filePath, bool staged);
    QString getDiff(const QString &filePath, bool staged);
    QString getStagedDiff();
    QString getCommitDiff(const QString &hash); // New method
    bool stageFile(const QString &filePath);
    bool unstageFile(const QString &filePath);
    bool stageAll();
    bool commit(const QString &message);
    bool push();
    bool pull();
    QList<GitCommit> getLog(int limit = 50);
    bool checkout(const QString &ref);
    bool reset(const QString &ref, bool hard);

private:
    QString runGitCommand(const QStringList &arguments);
    QString m_repositoryPath;
};

#endif // GITMANAGER_H

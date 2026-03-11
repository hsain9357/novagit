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

class GitManager : public QObject {
    Q_OBJECT
public:
    explicit GitManager(QObject *parent = nullptr);

    void setRepositoryPath(const QString &path);
    QString repositoryPath() const;

    QList<GitFileStatus> getStatus();
    QString getFileContent(const QString &filePath, bool staged); // Get original version from git
    QString getWorkingFileContent(const QString &filePath);       // Get current file content
    QString getDiff(const QString &filePath, bool staged);
    bool stageFile(const QString &filePath);
    bool unstageFile(const QString &filePath);
    bool commit(const QString &message);
    bool push();
    bool pull();

private:
    QString runGitCommand(const QStringList &arguments);
    QString m_repositoryPath;
};

#endif // GITMANAGER_H

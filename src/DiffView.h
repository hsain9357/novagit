#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QWidget>
#include <QTextEdit>
#include "GitManager.h"

class DiffView : public QWidget {
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = nullptr);
    void setDiff(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks);
    void clear();

private:
    QTextEdit *leftEdit;
    QTextEdit *rightEdit;
    void applyDiffAlignment(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks);
};

#endif // DIFFVIEW_H

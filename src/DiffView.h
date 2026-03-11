#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QWidget>
#include <QTextEdit>
#include "GitManager.h"
#include "SyntaxHighlighter.h"

class DiffView : public QWidget {
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = nullptr);
    void setDiff(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks);
    void clear();

private:
    QTextEdit *leftEdit;
    QTextEdit *rightEdit;
    SyntaxHighlighter *leftHighlighter;
    SyntaxHighlighter *rightHighlighter;
    
    // Gutter/Line Number support would go here in a more complex implementation
    // For now, let's focus on fixing the alignment logic in the CPP.

    void applyDiffAlignment(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks);
};

#endif // DIFFVIEW_H

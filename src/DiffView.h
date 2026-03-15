#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QWidget>
#include <QTextEdit>
#include "GitManager.h"
#include "SyntaxHighlighter.h"

class QScrollBar;

class DiffMinimap : public QWidget {
    Q_OBJECT
public:
    explicit DiffMinimap(QWidget *parent = nullptr);
    void setMarkers(const QList<QColor> &colors);
    void setScrollBar(QScrollBar *scrollBar);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void scrollTo(int y);
    QList<QColor> m_colors;
    QScrollBar *m_scrollBar = nullptr;
};

class DiffView : public QWidget {
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = nullptr);
    void setDiff(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks);
    void setUnifiedDiff(const QString &diff);
    void clear();
    void scrollToFirstDiff();

private:
    QTextEdit *leftEdit;
    QTextEdit *rightEdit;
    SyntaxHighlighter *leftHighlighter;
    SyntaxHighlighter *rightHighlighter;
    int m_firstDiffLine = -1;
    
    // Gutter/Line Number support would go here in a more complex implementation
    // For now, let's focus on fixing the alignment logic in the CPP.

    void applyDiffAlignment(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks);

    DiffMinimap *m_minimap;
};

#endif // DIFFVIEW_H

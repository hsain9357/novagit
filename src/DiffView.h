#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QTextEdit>

class DiffView : public QTextEdit {
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = nullptr);
    void setDiff(const QString &diffText);
};

#endif // DIFFVIEW_H

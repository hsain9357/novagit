#ifndef DIFFVIEW_H
#define DIFFVIEW_H

#include <QWidget>
#include <QTextEdit>

class DiffView : public QWidget {
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = nullptr);
    void setDiff(const QString &leftContent, const QString &rightContent);
    void clear();

private:
    QTextEdit *leftEdit;
    QTextEdit *rightEdit;
};

#endif // DIFFVIEW_H

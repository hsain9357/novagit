#ifndef COMMITITEMWIDGET_H
#define COMMITITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "GitManager.h"

class CommitItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit CommitItemWidget(const GitCommit &commit, QWidget *parent = nullptr);

signals:
    void checkoutRequested(const QString &hash);
    void sizeChanged();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    GitCommit m_commit;
    bool m_expanded = false;
    
    QWidget *m_mainContainer;
    QWidget *m_detailsContainer;
    
    QLabel *m_subjectLabel;
    QLabel *m_authorLabel;
    QLabel *m_dateLabel;
    QLabel *m_hashLabel;
    QLabel *m_messageLabel;
    QPushButton *m_checkoutBtn;

    void setupUi();
    void toggleExpanded();
};

#endif // COMMITITEMWIDGET_H

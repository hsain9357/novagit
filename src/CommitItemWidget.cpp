#include "CommitItemWidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

CommitItemWidget::CommitItemWidget(const GitCommit &commit, QWidget *parent)
    : QWidget(parent), m_commit(commit) {
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, [this](const QPoint &pos) {
        QMenu menu(this);
        QAction *copyAction = menu.addAction("Copy Commit ID");
        menu.addSeparator();
        QAction *softReset = menu.addAction("Reset Soft (Keep changes)");
        QAction *hardReset = menu.addAction("Reset Hard (Discard changes)");

        QAction *selectedAction = menu.exec(mapToGlobal(pos));
        if (selectedAction == copyAction) {
            QApplication::clipboard()->setText(m_commit.hash);
        } else if (selectedAction == softReset) {
            emit resetRequested(m_commit.hash, false);
        } else if (selectedAction == hardReset) {
            emit resetRequested(m_commit.hash, true);
        }
    });
    setupUi();
}

void CommitItemWidget::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(4);

    m_mainContainer = new QWidget();
    QVBoxLayout *mainVLayout = new QVBoxLayout(m_mainContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(2);

    m_subjectLabel = new QLabel(m_commit.subject);
    m_subjectLabel->setStyleSheet("font-weight: 600; color: #e1e1e1; font-size: 13px;");
    m_subjectLabel->setWordWrap(true);
    m_subjectLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainVLayout->addWidget(m_subjectLabel);

    QHBoxLayout *infoLayout = new QHBoxLayout();
    m_authorLabel = new QLabel(m_commit.author);
    m_authorLabel->setStyleSheet("color: #888; font-size: 11px;");
    m_dateLabel = new QLabel(m_commit.date);
    m_dateLabel->setStyleSheet("color: #888; font-size: 11px;");
    infoLayout->addWidget(m_authorLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_dateLabel);
    mainVLayout->addLayout(infoLayout);

    layout->addWidget(m_mainContainer);

    m_detailsContainer = new QWidget();
    m_detailsContainer->setVisible(false);
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsContainer);
    detailsLayout->setContentsMargins(0, 8, 0, 4);
    detailsLayout->setSpacing(6);

    m_hashLabel = new QLabel("ID: " + m_commit.hash.left(8));
    m_hashLabel->setStyleSheet("color: #007acc; font-family: 'Consolas', 'Monaco', monospace; font-size: 11px;");
    detailsLayout->addWidget(m_hashLabel);

    if (!m_commit.message.isEmpty() && m_commit.message != m_commit.subject) {
        m_messageLabel = new QLabel(m_commit.message);
        m_messageLabel->setWordWrap(true);
        m_messageLabel->setStyleSheet("color: #bbb; font-size: 12px; padding: 4px; background-color: #2d2d2d; border-radius: 4px;");
        detailsLayout->addWidget(m_messageLabel);
    }

    if (!m_commit.changedFiles.isEmpty()) {
        QLabel *filesHeader = new QLabel("CHANGED FILES");
        filesHeader->setStyleSheet("color: #666; font-weight: bold; font-size: 10px; margin-top: 4px;");
        detailsLayout->addWidget(filesHeader);

        for (const QString &file : m_commit.changedFiles) {
            QPushButton *fileBtn = new QPushButton(file);
            fileBtn->setFlat(true);
            fileBtn->setCursor(Qt::PointingHandCursor);
            fileBtn->setStyleSheet("QPushButton { color: #4daafc; text-align: left; padding: 2px 4px; border: none; font-size: 11px; } "
                                 "QPushButton:hover { background-color: #37373d; border-radius: 2px; }");
            connect(fileBtn, &QPushButton::clicked, [this, file]() {
                emit fileSelectedInCommit(m_commit.hash, file);
            });
            detailsLayout->addWidget(fileBtn);
        }
    }

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    m_checkoutBtn = new QPushButton("Checkout Commit");
    m_checkoutBtn->setObjectName("secondaryBtn");
    m_checkoutBtn->setCursor(Qt::PointingHandCursor);
    m_checkoutBtn->setStyleSheet("QPushButton { font-size: 11px; padding: 4px 8px; }");
    connect(m_checkoutBtn, &QPushButton::clicked, [this]() {
        emit checkoutRequested(m_commit.hash);
    });
    actionsLayout->addWidget(m_checkoutBtn);
    actionsLayout->addStretch();
    detailsLayout->addLayout(actionsLayout);

    layout->addWidget(m_detailsContainer);

    setStyleSheet("CommitItemWidget { border-bottom: 1px solid #2d2d2d; } "
                  "CommitItemWidget:hover { background-color: #2a2d2e; }");
}

void CommitItemWidget::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
}

void CommitItemWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
}

void CommitItemWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit commitSelected(m_commit.hash);
        toggleExpanded();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void CommitItemWidget::toggleExpanded() {
    m_expanded = !m_expanded;
    m_detailsContainer->setVisible(m_expanded);
    emit sizeChanged();
}

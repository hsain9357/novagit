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
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(2);

    m_mainContainer = new QWidget();
    QVBoxLayout *mainVLayout = new QVBoxLayout(m_mainContainer);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(2);

    m_subjectLabel = new QLabel(m_commit.subject);
    m_subjectLabel->setStyleSheet("font-weight: bold;");
    m_subjectLabel->setWordWrap(true);
    mainVLayout->addWidget(m_subjectLabel);

    QHBoxLayout *infoLayout = new QHBoxLayout();
    m_authorLabel = new QLabel(m_commit.author);
    m_authorLabel->setStyleSheet("color: #888; font-size: 10px;");
    m_dateLabel = new QLabel(m_commit.date);
    m_dateLabel->setStyleSheet("color: #888; font-size: 10px;");
    infoLayout->addWidget(m_authorLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_dateLabel);
    mainVLayout->addLayout(infoLayout);

    layout->addWidget(m_mainContainer);

    m_detailsContainer = new QWidget();
    m_detailsContainer->setVisible(false);
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsContainer);
    detailsLayout->setContentsMargins(0, 5, 0, 0);
    detailsLayout->setSpacing(5);

    m_hashLabel = new QLabel("Hash: " + m_commit.hash.left(8));
    m_hashLabel->setStyleSheet("color: #aaa; font-family: monospace;");
    detailsLayout->addWidget(m_hashLabel);

    if (!m_commit.message.isEmpty()) {
        m_messageLabel = new QLabel(m_commit.message);
        m_messageLabel->setWordWrap(true);
        m_messageLabel->setStyleSheet("color: #ccc;");
        detailsLayout->addWidget(m_messageLabel);
    }

    m_checkoutBtn = new QPushButton("Checkout");
    m_checkoutBtn->setCursor(Qt::PointingHandCursor);
    connect(m_checkoutBtn, &QPushButton::clicked, [this]() {
        emit checkoutRequested(m_commit.hash);
    });
    detailsLayout->addWidget(m_checkoutBtn);

    layout->addWidget(m_detailsContainer);

    setStyleSheet("CommitItemWidget { border-bottom: 1px solid #333; } "
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

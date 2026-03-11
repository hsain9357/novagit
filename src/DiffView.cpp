#include "DiffView.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

DiffView::DiffView(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);

    leftEdit = new QTextEdit();
    rightEdit = new QTextEdit();

    for (auto edit : {leftEdit, rightEdit}) {
        edit->setReadOnly(true);
        edit->setLineWrapMode(QTextEdit::NoWrap);
        edit->setFont(QFont("Monospace", 10));
        edit->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: none; }");
    }

    layout->addWidget(leftEdit);
    layout->addWidget(rightEdit);

    // Sync scrolling
    connect(leftEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            rightEdit->verticalScrollBar(), &QScrollBar::setValue);
    connect(rightEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            leftEdit->verticalScrollBar(), &QScrollBar::setValue);
}

void DiffView::setDiff(const QString &leftContent, const QString &rightContent) {
    leftEdit->setPlainText(leftContent);
    rightEdit->setPlainText(rightContent);
}

void DiffView::clear() {
    leftEdit->clear();
    rightEdit->clear();
}

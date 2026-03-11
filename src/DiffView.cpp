#include "DiffView.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include <QTextBlock>

DiffView::DiffView(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    leftEdit = new QTextEdit();
    rightEdit = new QTextEdit();

    for (auto edit : {leftEdit, rightEdit}) {
        edit->setReadOnly(true);
        edit->setLineWrapMode(QTextEdit::NoWrap);
        edit->setFont(QFont("Monospace", 10));
        edit->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: none; }");
        edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    rightEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    layout->addWidget(leftEdit);
    layout->addWidget(rightEdit);

    // Sync scrolling
    connect(rightEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            leftEdit->verticalScrollBar(), &QScrollBar::setValue);
    connect(leftEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            rightEdit->verticalScrollBar(), &QScrollBar::setValue);
}

void DiffView::setDiff(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks) {
    leftEdit->setPlainText(leftContent);
    rightEdit->setPlainText(rightContent);
    applyHighlights(hunks);
}

void DiffView::applyHighlights(const QList<GitHunk> &hunks) {
    QList<QTextEdit::ExtraSelection> leftSelections;
    QList<QTextEdit::ExtraSelection> rightSelections;

    QColor delColor(255, 0, 0, 40);
    QColor addColor(0, 255, 0, 40);

    for (const auto &hunk : hunks) {
        // Highlight deletions on left
        int currentOld = hunk.oldStart;
        int currentNew = hunk.newStart;
        
        for (const QString &line : hunk.lines) {
            if (line.startsWith("-")) {
                QTextEdit::ExtraSelection sel;
                sel.format.setBackground(delColor);
                sel.format.setProperty(QTextFormat::FullWidthSelection, true);
                
                QTextBlock block = leftEdit->document()->findBlockByLineNumber(currentOld - 1);
                if (block.isValid()) {
                    sel.cursor = QTextCursor(block);
                    leftSelections.append(sel);
                }
                currentOld++;
            } else if (line.startsWith("+")) {
                QTextEdit::ExtraSelection sel;
                sel.format.setBackground(addColor);
                sel.format.setProperty(QTextFormat::FullWidthSelection, true);
                
                QTextBlock block = rightEdit->document()->findBlockByLineNumber(currentNew - 1);
                if (block.isValid()) {
                    sel.cursor = QTextCursor(block);
                    rightSelections.append(sel);
                }
                currentNew++;
            } else {
                currentOld++;
                currentNew++;
            }
        }
    }
    leftEdit->setExtraSelections(leftSelections);
    rightEdit->setExtraSelections(rightSelections);
}

void DiffView::clear() {
    leftEdit->clear();
    rightEdit->clear();
}

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

    connect(rightEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            leftEdit->verticalScrollBar(), &QScrollBar::setValue);
    connect(leftEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            rightEdit->verticalScrollBar(), &QScrollBar::setValue);
}

void DiffView::setDiff(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks) {
    applyDiffAlignment(leftContent, rightContent, hunks);
}

void DiffView::applyDiffAlignment(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks) {
    QStringList leftLines = leftContent.split('\n');
    QStringList rightLines = rightContent.split('\n');

    struct DisplayLine {
        QString text;
        bool isPlaceholder = false;
        QColor bgColor = Qt::transparent;
        QList<QPair<int, int>> wordHighlights;
    };

    QList<DisplayLine> leftDisplay, rightDisplay;
    int leftIdx = 0, rightIdx = 0;

    auto getIntraLineDiff = [](const QString &oldL, const QString &newL) {
        if (oldL.trimmed() == newL.trimmed()) return qMakePair(QList<QPair<int, int>>(), QList<QPair<int, int>>());
        int prefix = 0;
        while (prefix < oldL.length() && prefix < newL.length() && oldL[prefix] == newL[prefix]) prefix++;
        int oldS = oldL.length() - 1, newS = newL.length() - 1;
        while (oldS >= prefix && newS >= prefix && oldL[oldS] == newL[newS]) { oldS--; newS--; }
        QList<QPair<int, int>> oh, nh;
        if (oldS >= prefix) oh.append({prefix, oldS - prefix + 1});
        if (newS >= prefix) nh.append({prefix, newS - prefix + 1});
        return qMakePair(oh, nh);
    };

    for (const auto &hunk : hunks) {
        while (leftIdx < hunk.oldStart - 1 && leftIdx < leftLines.size() && rightIdx < hunk.newStart - 1 && rightIdx < rightLines.size()) {
            leftDisplay.append({leftLines[leftIdx++], false});
            rightDisplay.append({rightLines[rightIdx++], false});
        }

        QStringList hOld, hNew;
        for (const QString &line : hunk.lines) {
            if (line.startsWith("-")) hOld.append(line.mid(1));
            else if (line.startsWith("+")) hNew.append(line.mid(1));
        }

        int maxL = qMax(hOld.size(), hNew.size());
        QColor dCol(255, 0, 0, 40), aCol(0, 255, 0, 40), pCol(45, 45, 45, 120);

        for (int i = 0; i < maxL; ++i) {
            QList<QPair<int, int>> lW, rW;
            bool isEdit = false;
            if (i < hOld.size() && i < hNew.size()) {
                if (hOld[i] != hNew[i]) {
                    auto d = getIntraLineDiff(hOld[i], hNew[i]);
                    lW = d.first; rW = d.second;
                    isEdit = true;
                }
            } else {
                isEdit = true; // Pure addition or subtraction
            }

            if (i < hOld.size()) {
                leftDisplay.append({hOld[i], false, isEdit ? dCol : Qt::transparent, lW});
            } else {
                leftDisplay.append({"", true, pCol});
            }

            if (i < hNew.size()) {
                rightDisplay.append({hNew[i], false, isEdit ? aCol : Qt::transparent, rW});
            } else {
                rightDisplay.append({"", true, pCol});
            }
        }
        leftIdx += hOld.size();
        rightIdx += hNew.size();
    }

    while (leftIdx < leftLines.size() || rightIdx < rightLines.size()) {
        QString l = (leftIdx < leftLines.size()) ? leftLines[leftIdx++] : "";
        QString r = (rightIdx < rightLines.size()) ? rightLines[rightIdx++] : "";
        leftDisplay.append({l, leftIdx > leftLines.size()});
        rightDisplay.append({r, rightIdx > rightLines.size()});
    }

    leftEdit->clear();
    rightEdit->clear();
    QColor dW(255, 0, 0, 100), aW(0, 255, 0, 100);

    auto render = [](QTextEdit *ed, const QList<DisplayLine> &disp, QColor wCol) {
        QTextCursor cur(ed->document());
        QList<QTextEdit::ExtraSelection> sels;
        for (int i = 0; i < disp.size(); ++i) {
            const auto &dl = disp[i];
            QTextCharFormat fmt;
            if (dl.isPlaceholder) fmt.setBackground(dl.bgColor);
            int start = cur.position();
            cur.insertText(dl.text, fmt);
            for (const auto &w : dl.wordHighlights) {
                QTextEdit::ExtraSelection s;
                s.format.setBackground(wCol);
                QTextCursor wc = cur;
                wc.setPosition(start + w.first);
                wc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, w.second);
                s.cursor = wc;
                sels.append(s);
            }
            // Full Line Highlight
            if (!dl.isPlaceholder && dl.bgColor != Qt::transparent) {
                QTextEdit::ExtraSelection lineSel;
                lineSel.format.setBackground(dl.bgColor);
                lineSel.format.setProperty(QTextFormat::FullWidthSelection, true);
                
                // CRITICAL: Ensure we only highlight the actual line, not the placeholder
                QTextBlock block = ed->document()->findBlockByLineNumber(i);
                if (block.isValid()) {
                    lineSel.cursor = QTextCursor(block);
                    sels.append(lineSel);
                }
            }
            cur.insertBlock();
        }
        ed->setExtraSelections(sels);
    };

    render(leftEdit, leftDisplay, dW);
    render(rightEdit, rightDisplay, aW);
}

void DiffView::clear() {
    leftEdit->clear();
    rightEdit->clear();
}

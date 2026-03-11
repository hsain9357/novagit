#include "DiffView.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QVector>

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
        while (oldS >= prefix && newS >= prefix && oldL[oldS] == newL[oldS]) { oldS--; newS--; }
        QList<QPair<int, int>> oh, nh;
        if (oldS >= prefix) oh.append({prefix, oldS - prefix + 1});
        if (newS >= prefix) nh.append({prefix, newS - prefix + 1});
        return qMakePair(oh, nh);
    };

    QColor dCol(255, 0, 0, 40), aCol(0, 255, 0, 40), pCol(45, 45, 45, 120);

    for (const auto &hunk : hunks) {
        // Sync context before hunk
        while (leftIdx < hunk.oldStart - 1 && leftIdx < leftLines.size() && rightIdx < hunk.newStart - 1 && rightIdx < rightLines.size()) {
            leftDisplay.append({leftLines[leftIdx++], false});
            rightDisplay.append({rightLines[rightIdx++], false});
        }

        int hIdx = 0;
        while (hIdx < hunk.lines.size()) {
            if (hunk.lines[hIdx].startsWith(" ")) {
                if (leftIdx < leftLines.size()) leftDisplay.append({leftLines[leftIdx++], false});
                if (rightIdx < rightLines.size()) rightDisplay.append({rightLines[rightIdx++], false});
                hIdx++;
            } else {
                QStringList hOld, hNew;
                while (hIdx < hunk.lines.size() && hunk.lines[hIdx].startsWith("-")) hOld.append(hunk.lines[hIdx++].mid(1));
                while (hIdx < hunk.lines.size() && hunk.lines[hIdx].startsWith("+")) hNew.append(hunk.lines[hIdx++].mid(1));
                
                // Use LCS to align lines within the hunk for "line-perfect" comparison
                int n = hOld.size();
                int m = hNew.size();
                QVector<QVector<int>> dp(n + 1, QVector<int>(m + 1, 0));
                for (int i = 1; i <= n; ++i) {
                    for (int j = 1; j <= m; ++j) {
                        if (hOld[i-1] == hNew[j-1]) dp[i][j] = dp[i-1][j-1] + 1;
                        else dp[i][j] = qMax(dp[i-1][j], dp[i][j-1]);
                    }
                }

                QList<QPair<int, int>> alignment;
                int i = n, j = m;
                while (i > 0 || j > 0) {
                    if (i > 0 && j > 0 && hOld[i-1] == hNew[j-1]) {
                        alignment.prepend({--i, --j});
                    } else if (j > 0 && (i == 0 || dp[i][j-1] >= dp[i-1][j])) {
                        alignment.prepend({-1, --j}); // Insertion
                    } else {
                        alignment.prepend({--i, -1}); // Deletion
                    }
                }

                for (const auto &pair : alignment) {
                    int lI = pair.first;
                    int rI = pair.second;
                    QString lText = (lI != -1) ? hOld[lI] : "";
                    QString rText = (rI != -1) ? hNew[rI] : "";
                    bool hasL = (lI != -1);
                    bool hasR = (rI != -1);
                    
                    // Only highlight if they are different AND both sides have content
                    // If one side is a placeholder, the whole line background is handled by isPlaceholder check later or by different background color
                    bool isDifferent = (hasL && hasR && lText != rText);
                    
                    QList<QPair<int, int>> lW, rW;
                    if (hasL && hasR && isDifferent) {
                        auto d = getIntraLineDiff(lText, rText);
                        lW = d.first; rW = d.second;
                    }

                    if (hasL) {
                        // Use dCol only if it's different or if it's a deletion (no matching R)
                        QColor bg = (hasR && !isDifferent) ? Qt::transparent : dCol;
                        leftDisplay.append({lText, false, bg, lW});
                    } else {
                        leftDisplay.append({"", true, pCol});
                    }

                    if (hasR) {
                        // Use aCol only if it's different or if it's an addition (no matching L)
                        QColor bg = (hasL && !isDifferent) ? Qt::transparent : aCol;
                        rightDisplay.append({rText, false, bg, rW});
                    } else {
                        rightDisplay.append({"", true, pCol});
                    }
                }
                leftIdx += hOld.size();
                rightIdx += hNew.size();
            }
        }
    }

    while (leftIdx < leftLines.size() || rightIdx < rightLines.size()) {
        QString l = (leftIdx < leftLines.size()) ? leftLines[leftIdx++] : "";
        QString r = (rightIdx < rightLines.size()) ? rightLines[rightIdx++] : "";
        leftDisplay.append({l, leftIdx > leftLines.size()});
        rightDisplay.append({r, rightIdx > rightLines.size()});
    }

    leftEdit->clear(); rightEdit->clear();
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
            if (!dl.isPlaceholder && dl.bgColor != Qt::transparent) {
                QTextEdit::ExtraSelection s;
                s.format.setBackground(dl.bgColor);
                s.format.setProperty(QTextFormat::FullWidthSelection, true);
                s.cursor = QTextCursor(ed->document()->findBlockByLineNumber(i));
                sels.append(s);
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

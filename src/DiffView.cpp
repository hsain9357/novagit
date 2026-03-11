#include "DiffView.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QVector>
#include <QPainter>
#include <QPixmap>

DiffView::DiffView(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    leftEdit = new QTextEdit();
    rightEdit = new QTextEdit();

    leftHighlighter = new SyntaxHighlighter(leftEdit->document());
    rightHighlighter = new SyntaxHighlighter(rightEdit->document());

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
    m_firstDiffLine = -1;
    applyDiffAlignment(leftContent, rightContent, hunks);
    scrollToFirstDiff();
}

void DiffView::scrollToFirstDiff() {
    if (m_firstDiffLine != -1) {
        QScrollBar *vb = rightEdit->verticalScrollBar();
        QTextBlock block = rightEdit->document()->findBlockByLineNumber(m_firstDiffLine);
        if (block.isValid()) {
            int pos = rightEdit->cursorRect(QTextCursor(block)).top();
            vb->setValue(vb->value() + pos);
        }
    }
}

void DiffView::applyDiffAlignment(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks) {
    QStringList leftLines = leftContent.split('\n');
    QStringList rightLines = rightContent.split('\n');

    struct DisplayLine {
        QString text;
        int lineNumber = -1;
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

    // VS Code inspired colors
    QColor dCol(255, 0, 0, 40), aCol(0, 255, 0, 40), pCol(45, 45, 45, 120);

    for (const auto &hunk : hunks) {
        // Sync context before hunk
        while (leftIdx < hunk.oldStart - 1 && leftIdx < leftLines.size() && rightIdx < hunk.newStart - 1 && rightIdx < rightLines.size()) {
            leftDisplay.append({leftLines[leftIdx], leftIdx + 1, false});
            rightDisplay.append({rightLines[rightIdx], rightIdx + 1, false});
            leftIdx++; rightIdx++;
        }

        int hIdx = 0;
        int currentLeft = hunk.oldStart - 1;
        int currentRight = hunk.newStart - 1;

        while (hIdx < hunk.lines.size()) {
            if (hunk.lines[hIdx].startsWith(" ")) {
                if (leftIdx < leftLines.size()) {
                    leftDisplay.append({leftLines[leftIdx], leftIdx + 1, false});
                    leftIdx++;
                }
                if (rightIdx < rightLines.size()) {
                    rightDisplay.append({rightLines[rightIdx], rightIdx + 1, false});
                    rightIdx++;
                }
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

                int lOffset = 0, rOffset = 0;
                for (const auto &pair : alignment) {
                    int lI = pair.first;
                    int rI = pair.second;
                    QString lText = (lI != -1) ? hOld[lI] : "";
                    QString rText = (rI != -1) ? hNew[rI] : "";
                    bool hasL = (lI != -1);
                    bool hasR = (rI != -1);
                    
                    bool isDifferent = (hasL && hasR && lText != rText);
                    
                    QList<QPair<int, int>> lW, rW;
                    if (hasL && hasR && isDifferent) {
                        auto d = getIntraLineDiff(lText, rText);
                        lW = d.first; rW = d.second;
                    }

                    if (hasL) {
                        QColor bg = (hasR && !isDifferent) ? Qt::transparent : dCol;
                        if (bg != Qt::transparent && m_firstDiffLine == -1) m_firstDiffLine = leftDisplay.size();
                        leftDisplay.append({lText, leftIdx + lI + 1, false, bg, lW});
                    } else {
                        if (m_firstDiffLine == -1) m_firstDiffLine = leftDisplay.size();
                        leftDisplay.append({"", -1, true, pCol});
                    }

                    if (hasR) {
                        QColor bg = (hasL && !isDifferent) ? Qt::transparent : aCol;
                        if (bg != Qt::transparent && m_firstDiffLine == -1) m_firstDiffLine = rightDisplay.size();
                        rightDisplay.append({rText, rightIdx + rI + 1, false, bg, rW});
                    } else {
                        if (m_firstDiffLine == -1) m_firstDiffLine = rightDisplay.size();
                        rightDisplay.append({"", -1, true, pCol});
                    }
                }
                leftIdx += hOld.size();
                rightIdx += hNew.size();
            }
        }
    }

    while (leftIdx < leftLines.size() || rightIdx < rightLines.size()) {
        if (leftIdx < leftLines.size()) {
            leftDisplay.append({leftLines[leftIdx], leftIdx + 1, false});
            leftIdx++;
        } else {
            leftDisplay.append({"", -1, true});
        }
        if (rightIdx < rightLines.size()) {
            rightDisplay.append({rightLines[rightIdx], rightIdx + 1, false});
            rightIdx++;
        } else {
            rightDisplay.append({"", -1, true});
        }
    }

    leftEdit->clear(); rightEdit->clear();
    QColor dW(255, 0, 0, 100), aW(0, 255, 0, 100);

    auto render = [](QTextEdit *ed, const QList<DisplayLine> &disp, QColor wCol) {
        QTextCursor cur(ed->document());
        QList<QTextEdit::ExtraSelection> sels;

        // Create a diagonal pattern for placeholders like VS Code
        QPixmap pix(8, 8);
        pix.fill(Qt::transparent);
        {
            QPainter painter(&pix);
            painter.setPen(QPen(QColor(128, 128, 128, 60), 1));
            painter.drawLine(0, 8, 8, 0);
        }
        QBrush diagonalBrush(pix);

        for (int i = 0; i < disp.size(); ++i) {
            const auto &dl = disp[i];
            
            // Format for line number
            QTextCharFormat numFmt;
            numFmt.setForeground(QColor(100, 100, 100));
            QString numStr = (dl.lineNumber != -1) ? QString::number(dl.lineNumber).rightJustified(4) + " " : "     ";
            int start = cur.position();
            cur.insertText(numStr, numFmt);
            
            // Format for line text
            QTextCharFormat fmt;
            cur.insertText(dl.text, fmt);

            if (dl.isPlaceholder) {
                // Background for placeholder
                QTextEdit::ExtraSelection s;
                s.format.setBackground(diagonalBrush);
                s.format.setProperty(QTextFormat::FullWidthSelection, true);
                s.cursor = QTextCursor(ed->document()->findBlockByLineNumber(i));
                sels.append(s);
            } else {
                for (const auto &w : dl.wordHighlights) {
                    QTextEdit::ExtraSelection s;
                    s.format.setBackground(wCol);
                    QTextCursor wc = cur;
                    // Add offset for line number prefix
                    wc.setPosition(start + 5 + w.first);
                    wc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, w.second);
                    s.cursor = wc;
                    sels.append(s);
                }
                if (dl.bgColor != Qt::transparent) {
                    QTextEdit::ExtraSelection s;
                    s.format.setBackground(dl.bgColor);
                    s.format.setProperty(QTextFormat::FullWidthSelection, true);
                    s.cursor = QTextCursor(ed->document()->findBlockByLineNumber(i));
                    sels.append(s);
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

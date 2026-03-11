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
    applyDiffAlignment(leftContent, rightContent, hunks);
}

void DiffView::applyDiffAlignment(const QString &leftContent, const QString &rightContent, const QList<GitHunk> &hunks) {
    QStringList leftLines = leftContent.split('\n');
    QStringList rightLines = rightContent.split('\n');

    struct DisplayLine {
        QString text;
        bool isPlaceholder = false;
        QColor bgColor = Qt::transparent;
        QList<QPair<int, int>> wordHighlights; // start, length
    };

    QList<DisplayLine> leftDisplay, rightDisplay;
    int leftIdx = 0, rightIdx = 0;

    auto getIntraLineDiff = [](const QString &oldLine, const QString &newLine) {
        // Simple word-level diff: find common prefix and suffix
        int prefix = 0;
        while (prefix < oldLine.length() && prefix < newLine.length() && oldLine[prefix] == newLine[prefix]) {
            prefix++;
        }
        int oldSuffix = oldLine.length() - 1;
        int newSuffix = newLine.length() - 1;
        while (oldSuffix >= prefix && newSuffix >= prefix && oldLine[oldSuffix] == newLine[newSuffix]) {
            oldSuffix--;
            newSuffix--;
        }
        
        QList<QPair<int, int>> oldHighlights, newHighlights;
        if (oldSuffix >= prefix) {
            oldHighlights.append({prefix, oldSuffix - prefix + 1});
        }
        if (newSuffix >= prefix) {
            newHighlights.append({prefix, newSuffix - prefix + 1});
        }
        return qMakePair(oldHighlights, newHighlights);
    };

    for (const auto &hunk : hunks) {
        // Fill unchanged lines before hunk
        while (leftIdx < hunk.oldStart - 1 && leftIdx < leftLines.size() && rightIdx < rightLines.size()) {
            leftDisplay.append({leftLines[leftIdx], false});
            rightDisplay.append({rightLines[rightIdx], false});
            leftIdx++;
            rightIdx++;
        }

        // Process hunk
        QStringList hunkOld, hunkNew;
        for (const QString &line : hunk.lines) {
            if (line.startsWith("-")) hunkOld.append(line.mid(1));
            else if (line.startsWith("+")) hunkNew.append(line.mid(1));
        }

        int maxLen = qMax(hunkOld.size(), hunkNew.size());
        QColor delColor(255, 0, 0, 40);
        QColor addColor(0, 255, 0, 40);

        for (int i = 0; i < maxLen; ++i) {

            QList<QPair<int, int>> leftWords, rightWords;
            if (i < hunkOld.size() && i < hunkNew.size()) {
                auto diff = getIntraLineDiff(hunkOld[i], hunkNew[i]);
                leftWords = diff.first;
                rightWords = diff.second;
            }

            if (i < hunkOld.size()) {
                leftDisplay.append({hunkOld[i], false, delColor, leftWords});
                leftIdx++;
            } else {
                leftDisplay.append({"", true, QColor(45, 45, 45, 100)});
            }

            if (i < hunkNew.size()) {
                rightDisplay.append({hunkNew[i], false, addColor, rightWords});
                rightIdx++;
            } else {
                rightDisplay.append({"", true, QColor(45, 45, 45, 100)});
            }
        }
    }

    // Fill remaining lines
    while (leftIdx < leftLines.size() || rightIdx < rightLines.size()) {
        QString lText = (leftIdx < leftLines.size()) ? leftLines[leftIdx++] : "";
        QString rText = (rightIdx < rightLines.size()) ? rightLines[rightIdx++] : "";
        leftDisplay.append({lText, leftIdx > leftLines.size()});
        rightDisplay.append({rText, rightIdx > rightLines.size()});
    }

    // Render
    leftEdit->clear();
    rightEdit->clear();

    QColor delWordHighlight(255, 0, 0, 100);
    QColor addWordHighlight(0, 255, 0, 100);

    auto render = [](QTextEdit *edit, const QList<DisplayLine> &display, QColor wordHighlightColor) {
        QTextCursor cursor(edit->document());
        QList<QTextEdit::ExtraSelection> selections;

        for (int i = 0; i < display.size(); ++i) {
            const auto &dl = display[i];
            QTextCharFormat format;
            if (dl.isPlaceholder) {
                format.setBackground(dl.bgColor);
            }
            
            int startPos = cursor.position();
            cursor.insertText(dl.text, format);
            
            // Apply intra-line word highlights
            for (const auto &highlight : dl.wordHighlights) {
                QTextEdit::ExtraSelection wordSel;
                wordSel.format.setBackground(wordHighlightColor);
                
                QTextCursor wordCursor = cursor;
                wordCursor.setPosition(startPos + highlight.first);
                wordCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, highlight.second);
                wordSel.cursor = wordCursor;
                selections.append(wordSel);
            }

            if (!dl.isPlaceholder && dl.bgColor != Qt::transparent) {
                QTextEdit::ExtraSelection lineSel;
                lineSel.format.setBackground(dl.bgColor);
                lineSel.format.setProperty(QTextFormat::FullWidthSelection, true);
                lineSel.cursor = QTextCursor(edit->document()->findBlockByLineNumber(i));
                selections.append(lineSel);
            }
            cursor.insertBlock();
        }
        edit->setExtraSelections(selections);
    };

    render(leftEdit, leftDisplay, delWordHighlight);
    render(rightEdit, rightDisplay, addWordHighlight);
}

void DiffView::clear() {
    leftEdit->clear();
    rightEdit->clear();
}

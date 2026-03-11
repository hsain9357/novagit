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
    };

    QList<DisplayLine> leftDisplay, rightDisplay;
    int leftIdx = 0, rightIdx = 0;

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
            // Context lines (' ') are handled by the 'before hunk' loop or skipped in -U0
        }

        int maxLen = qMax(hunkOld.size(), hunkNew.size());
        QColor delColor(255, 0, 0, 40);
        QColor addColor(0, 255, 0, 40);

        for (int i = 0; i < maxLen; ++i) {
            if (i < hunkOld.size()) {
                leftDisplay.append({hunkOld[i], false, delColor});
                leftIdx++;
            } else {
                leftDisplay.append({"", true, QColor(45, 45, 45, 100)});
            }

            if (i < hunkNew.size()) {
                rightDisplay.append({hunkNew[i], false, addColor});
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

    auto render = [](QTextEdit *edit, const QList<DisplayLine> &display) {
        QTextCursor cursor(edit->document());
        QList<QTextEdit::ExtraSelection> selections;

        for (int i = 0; i < display.size(); ++i) {
            const auto &dl = display[i];
            QTextCharFormat format;
            if (dl.isPlaceholder) {
                format.setBackground(dl.bgColor);
            }
            cursor.insertText(dl.text, format);

            if (!dl.isPlaceholder && dl.bgColor != Qt::transparent) {
                QTextEdit::ExtraSelection sel;
                sel.format.setBackground(dl.bgColor);
                sel.format.setProperty(QTextFormat::FullWidthSelection, true);
                sel.cursor = QTextCursor(edit->document()->findBlockByLineNumber(i));
                selections.append(sel);
            }
            cursor.insertBlock();
        }
        edit->setExtraSelections(selections);
    };

    render(leftEdit, leftDisplay);
    render(rightEdit, rightDisplay);
}

void DiffView::clear() {
    leftEdit->clear();
    rightEdit->clear();
}

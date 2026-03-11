#include "DiffView.h"
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class DiffHighlighter : public QSyntaxHighlighter {
public:
    explicit DiffHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {}

protected:
    void highlightBlock(const QString &text) override {
        QTextCharFormat format;
        if (text.startsWith("+")) {
            format.setForeground(Qt::darkGreen);
            format.setBackground(QColor(230, 255, 230));
            setFormat(0, text.length(), format);
        } else if (text.startsWith("-")) {
            format.setForeground(Qt::darkRed);
            format.setBackground(QColor(255, 230, 230));
            setFormat(0, text.length(), format);
        } else if (text.startsWith("@@")) {
            format.setForeground(Qt::blue);
            setFormat(0, text.length(), format);
        }
    }
};

DiffView::DiffView(QWidget *parent) : QTextEdit(parent) {
    setReadOnly(true);
    setLineWrapMode(QTextEdit::NoWrap);
    QFont font("Monospace", 10);
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    new DiffHighlighter(document());
}

void DiffView::setDiff(const QString &diffText) {
    setPlainText(diffText);
}

#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    keywordFormat.setForeground(QColor("#569cd6"));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b"
                    << "\\bif\\b" << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b"
                    << "\\breturn\\b" << "\\bswitch\\b" << "\\bcase\\b" << "\\bdefault\\b"
                    << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bnew\\b" << "\\bdelete\\b"
                    << "\\bthis\\b" << "\\btry\\b" << "\\bcatch\\b" << "\\bthrow\\b"
                    << "\\bexport\\b" << "\\bimport\\b" << "\\bfrom\\b" << "\\bas\\b"
                    << "\\blet\\b" << "\\bvar\\b" << "\\bfunction\\b" << "\\basync\\b" << "\\bawait\\b";

    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(QColor("#4ec9b0"));
    rule.pattern = QRegularExpression("\\b[A-Z][A-Za-z0-9_]*\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(QColor("#ce9178"));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    
    rule.pattern = QRegularExpression("'.*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(QColor("#dcdcaa"));
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(QColor("#6a9955"));
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(QColor("#6a9955"));
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Skip the line number prefix (first 5 characters)
    int offset = 5;
    if (text.length() <= offset) return;

    QString content = text.mid(offset);

    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(content);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart() + offset, match.capturedLength(), rule.format);
        }
    }
}

#include "AIHandler.h"
#include <QInputDialog>

AIHandler::AIHandler(QObject *parent) : QObject(parent) {
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &AIHandler::onReplyFinished);
}

QString AIHandler::getApiKey() {
    QString configPath = QDir::homePath() + "/.gemini_git_key";
    QFile file(configPath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        return QString::fromUtf8(file.readAll()).trimmed();
    }
    return QString();
}

void AIHandler::saveApiKey(const QString &key) {
    QString configPath = QDir::homePath() + "/.gemini_git_key";
    QFile file(configPath);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(key.toUtf8());
        file.close();
    }
}

void AIHandler::generateCommitMessage(const QString &diff) {
    QString apiKey = getApiKey();
    if (apiKey.isEmpty()) {
        emit errorOccurred("API Key required");
        return;
    }

    QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-3-flash-preview:generateContent?key=" + apiKey);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString prompt = "Analyze the provided git diff and generate a professional commit message.\n\n"
                     "Guidelines:\n"
                     "1. Use the Conventional Commits format: <type>(<scope>): <summary>\n"
                     "2. Choose the most appropriate type: feat, fix, refactor, docs, style, perf, test, chore.\n"
                     "3. Scope should be a brief noun describing the section of the codebase.\n"
                     "4. Summary should be in present tense, lowercase, and no period at the end.\n"
                     "5. Provide a detailed body using bullet points to explain 'what' and 'why' behind the changes.\n"
                     "6. Use backticks for file names, variable names, or code symbols.\n\n"
                     "Diff:\n" + diff;

    QJsonObject root;
    QJsonArray contents;
    QJsonObject content;
    QJsonArray parts;
    QJsonObject part;
    part["text"] = prompt;
    parts.append(part);
    content["parts"] = parts;
    contents.append(content);
    root["contents"] = contents;

    m_networkManager->post(request, QJsonDocument(root).toJson());
}

void AIHandler::onReplyFinished(QNetworkReply *reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Network Error: " + reply->errorString());
        return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject root = doc.object();

    // Extract message: candidates[0].content.parts[0].text
    QJsonArray candidates = root["candidates"].toArray();
    if (candidates.isEmpty()) {
        emit errorOccurred("No candidates in AI response");
        return;
    }

    QJsonObject firstCandidate = candidates[0].toObject();
    QJsonObject content = firstCandidate["content"].toObject();
    QJsonArray parts = content["parts"].toArray();
    if (parts.isEmpty()) {
        emit errorOccurred("No parts in AI response");
        return;
    }

    QString message = parts[0].toObject()["text"].toString();
    // Clean up potential markdown formatting
    message.replace("```", "");
    emit messageGenerated(message.trimmed());
}

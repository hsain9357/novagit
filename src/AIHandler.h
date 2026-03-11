#ifndef AIHANDLER_H
#define AIHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

class AIHandler : public QObject {
    Q_OBJECT
public:
    explicit AIHandler(QObject *parent = nullptr);
    void generateCommitMessage(const QString &diff);

signals:
    void messageGenerated(const QString &message);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
    QString getApiKey();
    void saveApiKey(const QString &key);
};

#endif // AIHANDLER_H

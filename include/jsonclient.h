#ifndef JSONCLIENT_H
#define JSONCLIENT_H

#include <jsonipc_global.h>
#include <QLocalSocket>

class QJsonObject;

/**
 * @brief The JsonClient class
 */
class JSONIPC_SHARED_EXPORT JsonClient
    : public QObject
{
    Q_OBJECT

public:
    JsonClient(QLocalSocket* localSocket = nullptr, QObject* parent = nullptr);
    ~JsonClient() override;

    bool connectToServer(const QString& name, int timeout = 1000);
    bool isConnected() const;

    Q_INVOKABLE void send(const QJsonObject& message);

signals:
    void connected();
    void disconnected();
    void received(const QJsonObject& message);
    void errorOccurred(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket* mLocalSocket{nullptr};
    int mMessageSize{0};

    friend class JsonServer;
};

#endif // JSONCLIENT_H

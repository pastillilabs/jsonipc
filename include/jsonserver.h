#ifndef JSONSERVER_H
#define JSONSERVER_H

#include <jsonclient.h>
#include <jsonipc_global.h>

#include <functional>

#include <QLocalServer>
#include <QVector>

class QJsonObject;

/**
 * @brief The JsonServer class
 */
class JSONIPC_SHARED_EXPORT JsonServer
    : public QObject
{
    Q_OBJECT

public:
    using AcceptHandler = std::function<bool(QLocalSocket&)>;

public:
    explicit JsonServer(QObject* parent = nullptr);

    bool listen(const QString& name);
    void setAcceptHandler(AcceptHandler acceptHandler);
    void setSocketOptions(QLocalServer::SocketOptions options);
    void close();

    bool isEmpty() const;
    bool isListening() const;

    void sendJsonMessageAll(const QJsonObject& message);
    void sendBinaryMessageAll(const QByteArray& message);

signals:
    void newClient(JsonClient& client);

private:
    QLocalServer mLocalServer;
    QVector<JsonClient*> mClients;

    AcceptHandler mAcceptHandler{[](const QLocalSocket&) { return true; }};
};

#endif // JSONSERVER_H

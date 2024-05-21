#pragma once

#include <jsonipc/jsonipc_global.h>

#include <functional>

#include <QLocalServer>
#include <QVector>

class QJsonObject;

namespace JsonIpc {

class Client;

/**
 * @brief The Server class
 */
class JSONIPC_SHARED_EXPORT Server : public QObject {
    Q_OBJECT

public:
    using AcceptHandler = std::function<bool(QLocalSocket&)>;

public:
    explicit Server(QObject* parent = nullptr);

    bool listen(const QString& name);
    void setAcceptHandler(AcceptHandler acceptHandler);
    void setSocketOptions(QLocalServer::SocketOptions options);
    void close();

    bool isEmpty() const;
    bool isListening() const;

    void sendJsonMessageAll(const QJsonObject& message);
    void sendBinaryMessageAll(const QByteArray& message);

signals:
    void newClient(Client& client);

private:
    QLocalServer mLocalServer;
    QVector<Client*> mClients;

    AcceptHandler mAcceptHandler{[](const QLocalSocket&) { return true; }};
};

} // namespace JsonIpc

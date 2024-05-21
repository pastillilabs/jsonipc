#pragma once

#include <jsonipc/jsonipc_global.h>

#include <QLocalSocket>

class QJsonObject;

namespace JsonIpc {

struct Header {
    quint8 mType{0};
    quint8 mUnused{0};
    quint16 mVersion{0};
    quint32 mSize{0};

    static const quint8 MSG_TYPE_JSON = 0;
    static const quint8 MSG_TYPE_BINARY = 1;
};

/**
 * @brief The Client class
 */
class JSONIPC_SHARED_EXPORT Client : public QObject {
    Q_OBJECT

public:
    Client(QLocalSocket* localSocket = nullptr, QObject* parent = nullptr);
    ~Client() override;

    bool connectToServer(const QString& name, int timeout = 1000);
    void disconnectFromServer();

    bool isConnected() const;
    QLocalSocket& socket() const;

    void sendJsonMessage(const QJsonObject& message);
    void sendBinaryMessage(const QByteArray& message);

signals:
    void connected();
    void disconnected();
    void jsonMessageReceived(const QJsonObject& message);
    void binaryMessageReceived(const QByteArray& message);
    void errorOccurred(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket* mLocalSocket{nullptr};
    Header mHeader;
};

} // namespace JsonIpc

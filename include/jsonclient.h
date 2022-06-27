#ifndef JSONCLIENT_H
#define JSONCLIENT_H

#include <jsonipc_global.h>
#include <QLocalSocket>

class QJsonObject;

struct Header {
    quint8 mType{0};
    quint8 mUnused{0};
    quint16 mVersion{0};
    quint32 mSize{0};

    static const quint8 MSG_TYPE_JSON = 0;
    static const quint8 MSG_TYPE_BINARY = 1;
};

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

    Q_INVOKABLE void sendJsonMessage(const QJsonObject& message);
    Q_INVOKABLE void sendBinaryMessage(const QByteArray& message);

signals:
    void connected();
    void disconnected();
    void jsonMessageReceived(const QJsonObject& message);
    void binaryMessageReceived(const QByteArray& message);
    void errorOccurred(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket* mLocalSocket{nullptr};
    Header mHeader;

    friend class JsonServer;
};

#endif // JSONCLIENT_H

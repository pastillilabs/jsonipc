#include "jsonipc/client.h"
#include "jsonipc/logging.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace JsonIpc {

Client::Client(QLocalSocket* localSocket, QObject* parent)
    : QObject{parent}
    , mLocalSocket(localSocket ? localSocket : new QLocalSocket(this)) {
    mLocalSocket->setParent(this);

    connect(mLocalSocket, &QLocalSocket::connected, this, &Client::connected);
    connect(mLocalSocket, &QLocalSocket::disconnected, this, &Client::disconnected);
    connect(mLocalSocket, &QLocalSocket::errorOccurred, this, &Client::errorOccurred);

    connect(mLocalSocket, &QLocalSocket::readyRead, this, [this] {
        bool messageReceived{true};

        while(!mLocalSocket->atEnd() && messageReceived) {
            // Read incoming message header
            if(mHeader.mSize == 0 && mLocalSocket->bytesAvailable() >= static_cast<qint64>(sizeof(Header))) {
                mLocalSocket->read(reinterpret_cast<char*>(&mHeader), static_cast<qint64>(sizeof(Header)));
            }

            // Read & handle message when fully available
            messageReceived = (mLocalSocket->bytesAvailable() >= mHeader.mSize);
            if(messageReceived) {
                const QByteArray byteArray{mLocalSocket->read(mHeader.mSize)};
                const quint32 flags = mHeader.mType;
                mHeader.mSize = 0;
                mHeader.mType = 0;

                if(flags == Header::MSG_TYPE_JSON) {
                    const QJsonObject message = QJsonDocument::fromJson(byteArray).object();
                    qCInfo(category) << "RECEIVE" << message;
                    emit jsonMessageReceived(message);
                }
                else if(flags == Header::MSG_TYPE_BINARY) {
                    qCInfo(category) << "RECEIVE" << byteArray;
                    emit binaryMessageReceived(byteArray);
                }
            }
        }
    });

    qCDebug(category) << "Client created:" << mLocalSocket->socketDescriptor();
}

Client::~Client() {
    mLocalSocket->disconnectFromServer();
}

bool Client::connectToServer(const QString& name, int timeout) {
    qCDebug(category) << "Connecting to server:" << name;
    bool result{false};

    if(!mLocalSocket->isOpen()) {
        mLocalSocket->connectToServer(name);
        result = mLocalSocket->waitForConnected(timeout);
    }

    qCDebug(category) << "Server connection resulted in" << result;

    return result;
}

void Client::disconnectFromServer() {
    qCDebug(category) << "Disconnect from server";
    mLocalSocket->disconnectFromServer();
}

bool Client::isConnected() const {
    return mLocalSocket->isOpen();
}

QLocalSocket& Client::socket() const {
    return *mLocalSocket;
}


void Client::sendJsonMessage(const QJsonObject& message) {
    qCInfo(category) << message;
    const QByteArray byteArray(QJsonDocument(message).toJson(QJsonDocument::Compact));

    Header header;
    header.mType = Header::MSG_TYPE_JSON;
    header.mSize = static_cast<quint32>(byteArray.size());

    mLocalSocket->write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(Header)));
    mLocalSocket->write(byteArray);
    mLocalSocket->flush();
}

void Client::sendBinaryMessage(const QByteArray& message) {
    qCInfo(category) << message;
    Header header;
    header.mType = Header::MSG_TYPE_BINARY;
    header.mSize = static_cast<quint32>(message.size());

    mLocalSocket->write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(Header)));
    mLocalSocket->write(message);
    mLocalSocket->flush();
}

} // namespace JsonIpc

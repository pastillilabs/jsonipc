#include "jsonclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

JsonClient::JsonClient(QLocalSocket* localSocket, QObject* parent)
    : QObject{parent}
    , mLocalSocket(localSocket ? localSocket : new QLocalSocket(this))
{
    mLocalSocket->setParent(this);

    connect(mLocalSocket, &QLocalSocket::connected, this, &JsonClient::connected);
    connect(mLocalSocket, &QLocalSocket::disconnected, this, &JsonClient::disconnected);

#if(QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(mLocalSocket, &QLocalSocket::errorOccurred, this, &JsonClient::errorOccurred);
#else
    connect(mLocalSocket, static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error), this, &JsonClient::errorOccurred);
#endif

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
                    emit jsonMessageReceived(QJsonDocument::fromJson(byteArray).object());
                }
                else if(flags == Header::MSG_TYPE_BINARY) {
                    emit binaryMessageReceived(byteArray);
                }
            }
        }
    });
}

JsonClient::~JsonClient()
{
    mLocalSocket->disconnectFromServer();
}

bool JsonClient::connectToServer(const QString& name, int timeout)
{
    bool result{false};

    if(!mLocalSocket->isOpen()) {
        mLocalSocket->connectToServer(name);
        result = mLocalSocket->waitForConnected(timeout);
    }

    return result;
}

bool JsonClient::isConnected() const
{
    return mLocalSocket->isOpen();
}

void JsonClient::sendJsonMessage(const QJsonObject& message)
{
    const QByteArray byteArray(QJsonDocument(message).toJson(QJsonDocument::Compact));

    Header header;
    header.mType = Header::MSG_TYPE_JSON;
    header.mSize = static_cast<quint32>(byteArray.size());

    mLocalSocket->write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(Header)));
    mLocalSocket->write(byteArray);
    mLocalSocket->flush();
}

void JsonClient::sendBinaryMessage(const QByteArray& message)
{
    Header header;
    header.mType = Header::MSG_TYPE_BINARY;
    header.mSize = static_cast<quint32>(message.size());

    mLocalSocket->write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(Header)));
    mLocalSocket->write(message);
    mLocalSocket->flush();
}

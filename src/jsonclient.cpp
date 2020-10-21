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
            // Read incoming message size
            if(mMessageSize == 0 && mLocalSocket->bytesAvailable() >= static_cast<qint64>(sizeof(mMessageSize))) {
                mLocalSocket->read(reinterpret_cast<char*>(&mMessageSize), static_cast<qint64>(sizeof(mMessageSize)));
            }

            // Read & handle message when fully available
            messageReceived = (mLocalSocket->bytesAvailable() >= mMessageSize);
            if(messageReceived) {
                const QByteArray byteArray{mLocalSocket->read(mMessageSize)};
                mMessageSize = 0;

                emit received(QJsonDocument::fromJson(byteArray).object());
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

void JsonClient::send(const QJsonObject& message)
{
    const QByteArray byteArray(QJsonDocument(message).toJson(QJsonDocument::Compact));
    const int size{byteArray.size()};

    mLocalSocket->write(reinterpret_cast<const char*>(&size), static_cast<qint64>(sizeof(size)));
    mLocalSocket->write(byteArray);
    mLocalSocket->flush();
}

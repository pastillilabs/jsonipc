#include "jsonserver.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

namespace {

const int MAX_RETRIES = 3;

} // namespace

JsonServer::JsonServer(QObject* parent)
    : QObject{parent}
    , mAcceptHandler([](const QLocalSocket&) { return true; })
{
    connect(&mLocalServer, &QLocalServer::newConnection, this, [this] {
        QLocalSocket* localSocket{mLocalServer.nextPendingConnection()};

        if(localSocket && mAcceptHandler(*localSocket)) {
            JsonClient* jsonClient = new JsonClient(localSocket, this);
            mClients.append(jsonClient);

            connect(jsonClient, &JsonClient::disconnected, this, [this, jsonClient] {
                mClients.removeOne(jsonClient);

                jsonClient->deleteLater();
            });

            emit newClient(*jsonClient);
        }
    });
}

bool JsonServer::listen(const QString& name)
{
    bool result{false};

    if(!mLocalServer.isListening()) {
        // Check if name is already in use
        QLocalSocket socket;
        socket.connectToServer(name);
        if(socket.waitForConnected(1000)) {
            socket.disconnectFromServer();

            qCritical() << "Name already in use:" << name;
        }
        else {
            int startCounter(MAX_RETRIES);
            while(startCounter && !mLocalServer.listen(name)) {
                --startCounter;
                QLocalServer::removeServer(name);
            }

            if(startCounter) {
                result = true;
            }
            else {
                qCritical() << "Unable to open server socket, error:" << mLocalServer.errorString();
            }
        }
    }

    return result;
}

void JsonServer::setAcceptHandler(AcceptHandler acceptHandler)
{
    mAcceptHandler = acceptHandler;
}

void JsonServer::setSocketOptions(QLocalServer::SocketOptions options)
{
    mLocalServer.setSocketOptions(options);
}

void JsonServer::close()
{
    mLocalServer.close();
}

bool JsonServer::isEmpty() const
{
    return mClients.isEmpty();
}

bool JsonServer::isListening() const
{
    return mLocalServer.isListening();
}

void JsonServer::sendJsonMessageAll(const QJsonObject& message)
{
    const QByteArray byteArray(QJsonDocument(message).toJson(QJsonDocument::Compact));

    Header header;
    header.mType = Header::MSG_TYPE_JSON;
    header.mSize = static_cast<quint32>(byteArray.size());

    const auto& clients = mClients;
    for(JsonClient* jsonClient : clients) {
        QLocalSocket* socket(jsonClient->mLocalSocket);

        socket->write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(header)));
        socket->write(byteArray);
        socket->flush();
    }
}

void JsonServer::sendBinaryMessageAll(const QByteArray& message)
{
    Header header;
    header.mType = Header::MSG_TYPE_BINARY;
    header.mSize = static_cast<quint32>(message.size());

    const auto& clients = mClients;
    for(JsonClient* jsonClient : clients) {
        QLocalSocket* socket(jsonClient->mLocalSocket);

        socket->write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(header)));
        socket->write(message);
        socket->flush();
    }
}

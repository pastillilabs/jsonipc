#include "jsonipc/server.h"
#include "jsonipc/client.h"
#include "jsonipc/logging.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

namespace {

const int MAX_RETRIES = 3;

} // namespace

namespace JsonIpc {

Server::Server(QObject* parent)
    : QObject{parent}
    , mAcceptHandler([](const QLocalSocket&) { return true; }) {
    connect(&mLocalServer, &QLocalServer::newConnection, this, [this] {
        QLocalSocket* localSocket{mLocalServer.nextPendingConnection()};

        if(localSocket && mAcceptHandler(*localSocket)) {
            Client* client = new Client(localSocket, this);
            mClients.append(client);

            connect(client, &Client::disconnected, this, [this, client] {
                mClients.removeOne(client);

                client->deleteLater();
            });

            emit newClient(*client);
        }
    });

    qCDebug(category) << "Server created";
}

bool Server::listen(const QString& name) {
    qCDebug(category) << "Listen as" << name;
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

    qCDebug(category) << "Listen resulted in" << result;

    return result;
}

void Server::setAcceptHandler(AcceptHandler acceptHandler) {
    mAcceptHandler = acceptHandler;
}

void Server::setSocketOptions(QLocalServer::SocketOptions options) {
    mLocalServer.setSocketOptions(options);
}

void Server::close() {
    mLocalServer.close();
}

bool Server::isEmpty() const {
    return mClients.isEmpty();
}

bool Server::isListening() const {
    return mLocalServer.isListening();
}

void Server::sendJsonMessageAll(const QJsonObject& message) {
    qCInfo(category) << message;
    const QByteArray byteArray(QJsonDocument(message).toJson(QJsonDocument::Compact));

    Header header;
    header.mType = Header::MSG_TYPE_JSON;
    header.mSize = static_cast<quint32>(byteArray.size());

    const auto& clients = mClients;
    for(Client* Client : clients) {
        QLocalSocket& socket(Client->socket());

        socket.write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(header)));
        socket.write(byteArray);
        socket.flush();
    }
}

void Server::sendBinaryMessageAll(const QByteArray& message) {
    qCInfo(category) << message;
    Header header;
    header.mType = Header::MSG_TYPE_BINARY;
    header.mSize = static_cast<quint32>(message.size());

    const auto& clients = mClients;
    for(Client* Client : clients) {
        QLocalSocket& socket(Client->socket());

        socket.write(reinterpret_cast<const char*>(&header), static_cast<qint64>(sizeof(header)));
        socket.write(message);
        socket.flush();
    }
}

} // namespace JsonIpc

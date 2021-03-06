#include "controlworker.h"

#include <QDataStream>
#include <QLocalSocket>

namespace {

constexpr int commandMaxArgs = 16;
constexpr int commandArgMaxSize = 4 * 1024;
constexpr quint32 dataMaxSize = 1 * 1024 * 1024;

bool buildArgsData(QByteArray &buffer, const QVariantList &args, bool &compressed)
{
    const int argsCount = args.count();
    if (argsCount == 0)
        return true;

    if (argsCount > commandMaxArgs)
        return false;

    QByteArray data;
    {
        QDataStream stream(&data,
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QIODevice::WriteOnly
#else
                QDataStream::WriteOnly
#endif
        );

        stream << qint8(argsCount);

        for (const auto &arg : args) {
            stream << arg;
        }
    }

    compressed = (data.size() > 128);
    buffer = compressed ? qCompress(data) : data;

    return true;
}

bool parseArgsData(const QByteArray &buffer, QVariantList &args, bool compressed)
{
    if (buffer.isEmpty())
        return true;

    const QByteArray data = compressed ? qUncompress(buffer) : buffer;

    QDataStream stream(data);

    qint8 argsCount;
    stream >> argsCount;

    if (argsCount > commandMaxArgs)
        return false;

    while (--argsCount >= 0) {
        QVariant arg;
        stream >> arg;

        args.append(arg);
    }

    return true;
}

}

ControlWorker::ControlWorker(QLocalSocket *socket, QObject *parent) :
    QObject(parent), m_isServiceClient(false), m_isClientValidated(false), m_socket(socket)
{
}

int ControlWorker::id() const
{
    return int(socket()->socketDescriptor());
}

bool ControlWorker::isConnected() const
{
    return socket()->state() == QLocalSocket::ConnectedState;
}

QString ControlWorker::errorString() const
{
    return socket()->errorString();
}

void ControlWorker::setupForAsync()
{
    socket()->setParent(this);

    connect(socket(), &QLocalSocket::disconnected, this, &ControlWorker::disconnected);
    connect(socket(), &QLocalSocket::errorOccurred, this,
            [&](QLocalSocket::LocalSocketError socketError) {
                qWarning() << "Client error:" << id() << socketError << errorString();
                close();
            });
    connect(socket(), &QLocalSocket::readyRead, this, &ControlWorker::processRequest);
}

bool ControlWorker::connectToServer(const QString &name)
{
    socket()->connectToServer(name);
    return socket()->waitForConnected(100) && socket()->state() == QLocalSocket::ConnectedState;
}

void ControlWorker::close()
{
    socket()->abort();
    socket()->close();
}

QByteArray ControlWorker::buildCommandData(Control::Command command, const QVariantList &args)
{
    QByteArray data;
    bool compressed = false;
    if (!buildArgsData(data, args, compressed))
        return {};

    RequestHeader request(command, compressed, data.size());

    QByteArray buffer;
    buffer.append((const char *) &request, sizeof(RequestHeader));
    buffer.append(data);

    return buffer;
}

bool ControlWorker::sendCommandData(const QByteArray &commandData)
{
    Q_ASSERT(!commandData.isEmpty());

    const int bytesSent = socket()->write(commandData);
    if (bytesSent != commandData.size()) {
        if (bytesSent < 0) {
            qWarning() << "Send error:" << id() << errorString();
        } else {
            qWarning() << "Sent partial:" << id() << bytesSent << commandData.size();
        }
        return false;
    }

    socket()->flush();

    return true;
}

bool ControlWorker::sendCommand(Control::Command command, const QVariantList &args)
{
    const QByteArray buffer = buildCommandData(command, args);
    if (buffer.isEmpty()) {
        qWarning() << "Bad RPC command to send:" << command << args;
        return false;
    }

    return sendCommandData(buffer);
}

bool ControlWorker::waitForSent(int msecs) const
{
    return socket()->waitForBytesWritten(msecs);
}

bool ControlWorker::waitForRead(int msecs) const
{
    return socket()->waitForReadyRead(msecs);
}

void ControlWorker::processRequest()
{
    do {
        if (!readRequest()) {
            close();
            clearRequest();
            break;
        }
    } while (socket()->bytesAvailable() > 0);
}

void ControlWorker::clearRequest()
{
    m_requestHeader.clear();
    m_requestBuffer.clear();
}

bool ControlWorker::readRequest()
{
    if (m_requestHeader.command() == Control::CommandNone && !readRequestHeader())
        return false;

    const int bytesNeeded = m_requestHeader.dataSize() - m_requestBuffer.size();
    if (bytesNeeded > 0) {
        if (socket()->bytesAvailable() == 0)
            return true; // need more data

        const QByteArray data = socket()->read(bytesNeeded);
        if (data.isEmpty())
            return false;

        m_requestBuffer += data;

        if (data.size() < bytesNeeded)
            return true; // need more data
    }

    QVariantList args;
    if (!parseArgsData(m_requestBuffer, args, m_requestHeader.compressed()))
        return false;

    const Control::Command command = m_requestHeader.command();

    clearRequest();

    // qDebug() << "requestReady>" << id() << command << args;

    emit requestReady(command, args);

    return true;
}

bool ControlWorker::readRequestHeader()
{
    if (socket()->read((char *) &m_requestHeader, sizeof(RequestHeader)) != sizeof(RequestHeader))
        return false;

    if (m_requestHeader.command() == Control::CommandNone
            || m_requestHeader.dataSize() > dataMaxSize)
        return false;

    return true;
}

QVariantList ControlWorker::buildArgs(const QStringList &list)
{
    QVariantList args;

    for (const auto &s : list) {
        if (s.size() > commandArgMaxSize)
            return {};

        args.append(s);
    }

    return args;
}

#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <QObject>
#include <QByteArray>

class LogEntry;

class LogBuffer : public QObject
{
    Q_OBJECT

public:
    explicit LogBuffer(int bufferSize = 0,
                       QObject *parent = nullptr);

signals:

public slots:
    int write(const LogEntry &logEntry);
    int read(LogEntry &logEntry);

private:
    void prepareFor(int len);

private:
    int m_top;
    int m_offset;

    QByteArray m_array;
};

#endif // LOGBUFFER_H
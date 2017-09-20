#ifndef TASKINFO_H
#define TASKINFO_H

#include <QObject>
#include <QDateTime>

class TaskInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int intervalHours READ intervalHours WRITE setIntervalHours NOTIFY intervalHoursChanged)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(TaskInfo::TaskType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QDateTime lastRun READ lastRun WRITE setLastRun NOTIFY lastRunChanged)
    Q_PROPERTY(QDateTime lastSuccess READ lastSuccess WRITE setLastSuccess NOTIFY lastSuccessChanged)

public:
    enum TaskType {
        TypeNone = -1,
        Tasix = 0
    };
    Q_ENUM(TaskType)

    explicit TaskInfo(TaskInfo::TaskType type, QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    int intervalHours() const { return m_intervalHours; }
    void setIntervalHours(int intervalHours);

    QString title() const;

    TaskInfo::TaskType type() const { return m_type; }
    void setType(TaskInfo::TaskType type);

    QDateTime lastRun() const { return m_lastRun; }
    void setLastRun(const QDateTime &lastRun);

    QDateTime lastSuccess() const { return m_lastSuccess; }
    void setLastSuccess(const QDateTime &lastSuccess);

    void rawData(QByteArray &data) const;
    void setRawData(const QByteArray &data);

    static QString typeToString(TaskInfo::TaskType type);
    static TaskInfo::TaskType stringToType(const QString &name);

signals:
    void enabledChanged();
    void intervalHoursChanged();
    void typeChanged();
    void lastRunChanged();
    void lastSuccessChanged();

public slots:

private:
    uint m_enabled          : 1;
    uint m_intervalHours    : 16;

    TaskType m_type;

    QDateTime m_lastRun;
    QDateTime m_lastSuccess;
};

#endif // TASKINFO_H

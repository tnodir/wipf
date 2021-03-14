#ifndef TABLESQLMODEL_H
#define TABLESQLMODEL_H

#include "tableitemmodel.h"

class SqliteDb;

class TableSqlModel : public TableItemModel
{
    Q_OBJECT

public:
    explicit TableSqlModel(QObject *parent = nullptr);

    virtual SqliteDb *sqliteDb() const = 0;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

signals:
    void modelChanged();

public slots:
    void reset();
    void refresh();

protected:
    void invalidateRowCache();
    void updateRowCache(int row) const;

    virtual bool updateTableRow(int row) const = 0;
    virtual TableRow &tableRow() const = 0;

    virtual int doSqlCount() const;

    virtual QString sqlCount() const;
    virtual QString sql() const;
    virtual QString sqlBase() const = 0;
    virtual QString sqlOrder() const;
    virtual QString sqlOrderAsc() const;
    virtual QString sqlOrderColumn() const;
    virtual QString sqlWhere() const;
    virtual QString sqlLimitOffset() const;

    int sortColumn() const { return m_sortColumn; }
    void setSortColumn(int v) { m_sortColumn = v; }

    Qt::SortOrder sortOrder() const { return m_sortOrder; }
    void setSortOrder(Qt::SortOrder v) { m_sortOrder = v; }

private:
    int m_sortColumn = -1;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

    mutable int m_rowCount = -1;
};

#endif // TABLESQLMODEL_H

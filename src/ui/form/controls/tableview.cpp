#include "tableview.h"

#include <QContextMenuEvent>
#include <QMenu>

TableView::TableView(QWidget *parent) :
    QTableView(parent)
{
}

QVector<int> TableView::selectedRows() const
{
    QSet<int> rowsSet;
    for (const auto index : selectedIndexes()) {
        rowsSet.insert(index.row());
    }

    auto rows = rowsSet.values();
    std::sort(rows.begin(), rows.end());
    return rows.toVector();
}

void TableView::currentChanged(const QModelIndex &current,
                               const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);

    emit currentIndexChanged(current);
}

void TableView::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_menu != nullptr) {
        m_menu->popup(event->globalPos());
    }
}

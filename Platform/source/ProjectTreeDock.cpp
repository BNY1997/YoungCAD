#include "ProjectTreeDock.h"

#include <QAction>
#include <QMenu>
#include <QSignalBlocker>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace
{
constexpr int kModelIdRole = Qt::UserRole + 1;
}

ProjectTreeDock::ProjectTreeDock(QWidget* parent)
    : QDockWidget(QStringLiteral("Project Tree"), parent)
{
    setObjectName(QStringLiteral("ProjectTreeDock"));
    setAllowedAreas(Qt::LeftDockWidgetArea);
    setFeatures(QDockWidget::NoDockWidgetFeatures);

    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels({ QStringLiteral("Name"), QStringLiteral("Type") });
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);

    m_rootItem = new QTreeWidgetItem(m_tree, { QStringLiteral("Project"), QStringLiteral("Root") });
    m_rootItem->setExpanded(true);

    setWidget(m_tree);

    connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &ProjectTreeDock::onItemSelectionChanged);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &ProjectTreeDock::onCustomContextMenu);
}

void ProjectTreeDock::addModel(int modelId, const QString& name, const QString& category)
{
    if (m_itemById.contains(modelId))
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem(m_rootItem, { name, category });
    item->setData(0, kModelIdRole, modelId);
    m_itemById.insert(modelId, item);
    m_rootItem->setExpanded(true);
}

void ProjectTreeDock::removeModel(int modelId)
{
    QTreeWidgetItem* item = itemForModel(modelId);
    if (!item)
        return;

    m_itemById.remove(modelId);
    delete item;
}

void ProjectTreeDock::clearModels()
{
    m_itemById.clear();
    m_rootItem->takeChildren();
}

void ProjectTreeDock::setSelectedModel(int modelId)
{
    QSignalBlocker blocker(m_tree);
    if (modelId < 0)
    {
        m_tree->clearSelection();
        return;
    }

    QTreeWidgetItem* item = itemForModel(modelId);
    if (!item)
        return;

    m_tree->setCurrentItem(item);
    item->setSelected(true);
}

int ProjectTreeDock::selectedModelId() const
{
    const auto selectedItems = m_tree->selectedItems();
    if (selectedItems.isEmpty())
        return -1;
    return modelIdFromItem(selectedItems.front());
}

void ProjectTreeDock::onItemSelectionChanged()
{
    emit modelSelectionChanged(selectedModelId());
}

void ProjectTreeDock::onCustomContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = m_tree->itemAt(pos);
    if (!item)
        return;

    const int modelId = modelIdFromItem(item);
    if (modelId < 0)
        return;

    QMenu menu(m_tree);
    QAction* deleteAction = menu.addAction(QStringLiteral("Delete Model"));
    QAction* chosen = menu.exec(m_tree->viewport()->mapToGlobal(pos));
    if (chosen == deleteAction)
        emit modelDeleteRequested(modelId);
}

QTreeWidgetItem* ProjectTreeDock::itemForModel(int modelId) const
{
    return m_itemById.value(modelId, nullptr);
}

int ProjectTreeDock::modelIdFromItem(const QTreeWidgetItem* item) const
{
    if (!item)
        return -1;
    return item->data(0, kModelIdRole).toInt();
}

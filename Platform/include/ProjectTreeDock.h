#pragma once

#include <QDockWidget>
#include <QHash>

class QTreeWidget;
class QTreeWidgetItem;

class ProjectTreeDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ProjectTreeDock(QWidget* parent = nullptr);

    void addModel(int modelId, const QString& name, const QString& category);
    void removeModel(int modelId);
    void clearModels();

    void setSelectedModel(int modelId);
    int selectedModelId() const;

signals:
    void modelSelectionChanged(int modelId);
    void modelDeleteRequested(int modelId);

private slots:
    void onItemSelectionChanged();
    void onCustomContextMenu(const QPoint& pos);

private:
    QTreeWidgetItem* itemForModel(int modelId) const;
    int modelIdFromItem(const QTreeWidgetItem* item) const;

    QTreeWidget* m_tree{ nullptr };
    QTreeWidgetItem* m_rootItem{ nullptr };
    QHash<int, QTreeWidgetItem*> m_itemById;
};

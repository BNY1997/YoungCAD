#include "ShapePool.h"

// ── Constructor ───────────────────────────────────────────────────────────

ShapePool::ShapePool()
    : m_root(new osg::Group)
{
}

// ── Private insert helper ─────────────────────────────────────────────────

int ShapePool::add(ShapeEntry&& entry)
{
    const int id = m_nextId++;
    entry.id = id;
    m_root->addChild(entry.node);
    m_entries.emplace(id, std::move(entry));
    return id;
}

// ── Create ────────────────────────────────────────────────────────────────

int ShapePool::add(const std::string& name,
                   const TopoDS_Shape& shape,
                   osg::ref_ptr<osg::Node> node)
{
    ShapeEntry e;
    e.name  = name;
    e.shape = shape;
    e.node  = std::move(node);
    return add(std::move(e));
}

int ShapePool::addNode(const std::string& name,
                       osg::ref_ptr<osg::Node> node)
{
    ShapeEntry e;
    e.name = name;
    e.node = std::move(node);
    return add(std::move(e));
}

// ── Read ──────────────────────────────────────────────────────────────────

const ShapeEntry* ShapePool::get(int id) const
{
    auto it = m_entries.find(id);
    return (it != m_entries.end()) ? &it->second : nullptr;
}

std::vector<const ShapeEntry*> ShapePool::getAll() const
{
    std::vector<const ShapeEntry*> result;
    result.reserve(m_entries.size());
    for (const auto& [id, entry] : m_entries)
        result.push_back(&entry);
    return result;
}

int ShapePool::findByName(const std::string& name) const
{
    for (const auto& [id, entry] : m_entries)
        if (entry.name == name)
            return id;
    return -1;
}

int  ShapePool::count()   const { return static_cast<int>(m_entries.size()); }
bool ShapePool::isEmpty() const { return m_entries.empty(); }

// ── Update ────────────────────────────────────────────────────────────────

bool ShapePool::updateShape(int id,
                             const TopoDS_Shape& newShape,
                             osg::ref_ptr<osg::Node> newNode)
{
    auto it = m_entries.find(id);
    if (it == m_entries.end())
        return false;

    // Swap the OSG child atomically so the scene stays consistent.
    m_root->replaceChild(it->second.node, newNode);
    it->second.shape = newShape;
    it->second.node  = std::move(newNode);
    return true;
}

bool ShapePool::rename(int id, const std::string& newName)
{
    auto it = m_entries.find(id);
    if (it == m_entries.end())
        return false;
    it->second.name = newName;
    return true;
}

// ── Delete ────────────────────────────────────────────────────────────────

bool ShapePool::remove(int id)
{
    auto it = m_entries.find(id);
    if (it == m_entries.end())
        return false;

    m_root->removeChild(it->second.node);
    m_entries.erase(it);
    return true;
}

void ShapePool::clear()
{
    m_root->removeChildren(0, m_root->getNumChildren());
    m_entries.clear();
}

// ── Scene integration ─────────────────────────────────────────────────────

osg::Group* ShapePool::getRoot() const
{
    return m_root.get();
}

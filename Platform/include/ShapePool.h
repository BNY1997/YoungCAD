#pragma once

#include <map>
#include <string>
#include <vector>

#include <TopoDS_Shape.hxx>

#include <osg/Group>
#include <osg/Node>
#include <osg/ref_ptr>

/*!
 * \brief One record held inside ShapePool.
 *
 * `shape` is the original OCCT geometry.  For objects loaded from mesh files
 * (OBJ / point-cloud TXT) the shape is null (TopoDS_Shape::IsNull() == true).
 * `node` is the corresponding OSG renderable that lives in the scene graph.
 */
struct ShapeEntry
{
    int          id   = 0;
    std::string  name;
    TopoDS_Shape shape;                  ///< OCCT geometry (may be null)
    osg::ref_ptr<osg::Node> node;        ///< OSG scene-graph node
};

/*!
 * \brief In-memory pool that owns all OCCT / OSG objects added to the viewer.
 *
 * The pool maintains an internal osg::Group that aggregates every node.
 * Pass getRoot() to OSGWidget::setSceneData() once during initialisation;
 * after that, add/remove/update operations automatically keep the rendered
 * scene in sync without ever calling setSceneData() again.
 *
 * CRUD summary
 * ─────────────────────────────────────────────────────────────────────────
 *  Create  │ add(name, shape, node)   – OCCT shape + OSG node
 *           │ addNode(name, node)      – OSG-only (mesh file, OBJ…)
 *  Read    │ get(id)                  – single entry by ID
 *           │ getAll()                 – all entries
 *           │ findByName(name)         – first matching ID or -1
 *  Update  │ updateShape(id, …)       – replace geometry and node
 *           │ rename(id, newName)      – change entry name
 *  Delete  │ remove(id)               – remove one entry
 *           │ clear()                  – remove all entries
 * ─────────────────────────────────────────────────────────────────────────
 */
class ShapePool
{
public:
    ShapePool();
    ~ShapePool() = default;

    // Non-copyable, movable
    ShapePool(const ShapePool&)            = delete;
    ShapePool& operator=(const ShapePool&) = delete;
    ShapePool(ShapePool&&)                 = default;
    ShapePool& operator=(ShapePool&&)      = default;

    // ── Create ────────────────────────────────────────────────────────────

    /// Add an OCCT shape together with its OSG representation.
    /// Returns the newly assigned ID (always > 0).
    int add(const std::string& name,
            const TopoDS_Shape& shape,
            osg::ref_ptr<osg::Node> node);

    /// Add a node that has no corresponding OCCT shape (e.g. OBJ / TXT files).
    /// Returns the newly assigned ID (always > 0).
    int addNode(const std::string& name,
                osg::ref_ptr<osg::Node> node);

    // ── Read ──────────────────────────────────────────────────────────────

    /// Returns a pointer to the entry, or nullptr if the ID does not exist.
    const ShapeEntry* get(int id) const;

    /// All entries in ID order.
    std::vector<const ShapeEntry*> getAll() const;

    /// Returns the first ID whose name matches, or -1 if not found.
    int  findByName(const std::string& name) const;

    int  count()   const;
    bool isEmpty() const;

    // ── Update ────────────────────────────────────────────────────────────

    /// Replace the OCCT shape and the OSG node of an existing entry.
    /// The old node is removed from the scene and the new node is inserted.
    /// Returns false if the ID does not exist.
    bool updateShape(int id,
                     const TopoDS_Shape& newShape,
                     osg::ref_ptr<osg::Node> newNode);

    /// Rename an existing entry.  Returns false if the ID does not exist.
    bool rename(int id, const std::string& newName);

    // ── Delete ────────────────────────────────────────────────────────────

    /// Remove a single entry by ID and detach its node from the scene.
    /// Returns false if the ID does not exist.
    bool remove(int id);

    /// Remove all entries and clear the scene graph root.
    void clear();

    // ── Scene integration ─────────────────────────────────────────────────

    /// The osg::Group that aggregates all managed nodes.
    /// Pass this once to OSGWidget::setSceneData() during initialisation.
    osg::Group* getRoot() const;

private:
    int add(ShapeEntry&& entry);   ///< shared internal insert helper

    int m_nextId{ 1 };
    osg::ref_ptr<osg::Group>  m_root;
    std::map<int, ShapeEntry> m_entries;   // ordered by ID
};

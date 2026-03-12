#include "DataExchange.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Array>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/LineWidth>
#include <osg/PolygonOffset>
#include <osg/Drawable>

#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

#include <TopoDS_Edge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>

#include <vector>

// osg::DrawElementsUInt is provided by osg/Geometry

namespace
{
    struct MeshStatistics
    {
        size_t vertexCount = 0;
        size_t triangleCount = 0;
    };

    struct EdgePolylineData
    {
        std::vector<osg::Vec3> points;
    };

    struct EdgeStatistics
    {
        size_t vertexCount = 0;
        std::vector<EdgePolylineData> polylines;
    };

    MeshStatistics collectMeshStatistics(const TopoDS_Shape& shape)
    {
        MeshStatistics stats;

        TopExp_Explorer ex(shape, TopAbs_FACE);
        while (ex.More())
        {
            const TopoDS_Face& face = TopoDS::Face(ex.Current());
            TopLoc_Location loc;
            auto triangulation = BRep_Tool::Triangulation(face, loc);

            if (!triangulation.IsNull())
            {
                stats.vertexCount += static_cast<size_t>(triangulation->NbNodes());
                stats.triangleCount += static_cast<size_t>(triangulation->NbTriangles());
            }

            ex.Next();
        }

        return stats;
    }

    osg::ref_ptr<osg::Geometry> buildFaceGeometry(const TopoDS_Shape& shape, const DataExchangeOptions& options)
    {
        const MeshStatistics stats = collectMeshStatistics(shape);

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
        osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
        osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

        vertices->reserve(stats.vertexCount);
        normals->reserve(stats.vertexCount);
        indices->reserve(stats.triangleCount * 3);

        geometry->setVertexArray(vertices);
        geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

        TopExp_Explorer ex(shape, TopAbs_FACE);
        while (ex.More())
        {
            const TopoDS_Face& face = TopoDS::Face(ex.Current());
            TopLoc_Location loc;
            auto triangulation = BRep_Tool::Triangulation(face, loc);

            if (!triangulation.IsNull())
            {
                const Standard_Integer nbNodes = triangulation->NbNodes();
                const Standard_Integer nbTriangles = triangulation->NbTriangles();
                const gp_Trsf trsf = loc.Transformation();
                const bool reverse = (face.Orientation() == TopAbs_REVERSED);
                const unsigned int offset = static_cast<unsigned int>(vertices->size());

                for (Standard_Integer i = 1; i <= nbNodes; ++i)
                {
                    const gp_Pnt p = triangulation->Node(i).Transformed(trsf);
                    vertices->push_back(osg::Vec3(static_cast<float>(p.X()), static_cast<float>(p.Y()), static_cast<float>(p.Z())));
                    normals->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
                }

                for (Standard_Integer i = 1; i <= nbTriangles; ++i)
                {
                    Standard_Integer n1, n2, n3;
                    triangulation->Triangle(i).Get(n1, n2, n3);

                    if (reverse)
                    {
                        std::swap(n1, n3);
                    }

                    const unsigned int i1 = offset + static_cast<unsigned int>(n1 - 1);
                    const unsigned int i2 = offset + static_cast<unsigned int>(n2 - 1);
                    const unsigned int i3 = offset + static_cast<unsigned int>(n3 - 1);

                    indices->push_back(i1);
                    indices->push_back(i2);
                    indices->push_back(i3);

                    const osg::Vec3& v1 = (*vertices)[i1];
                    const osg::Vec3& v2 = (*vertices)[i2];
                    const osg::Vec3& v3 = (*vertices)[i3];
                    osg::Vec3 normal = (v2 - v1) ^ (v3 - v1);
                    if (normal.length2() > 0.0f)
                    {
                        normal.normalize();
                        if (options.smoothNormals)
                        {
                            (*normals)[i1] += normal;
                            (*normals)[i2] += normal;
                            (*normals)[i3] += normal;
                        }
                        else
                        {
                            (*normals)[i1] = normal;
                            (*normals)[i2] = normal;
                            (*normals)[i3] = normal;
                        }
                    }
                }
            }

            ex.Next();
        }

        for (auto& normal : *normals)
        {
            if (normal.length2() > 0.0f)
            {
                normal.normalize();
            }
            else
            {
                normal.set(0.0f, 0.0f, 1.0f);
            }
        }

        geometry->addPrimitiveSet(indices);

        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
        colors->push_back(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));
        geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

        osg::StateSet* stateSet = geometry->getOrCreateStateSet();
        osg::Material* material = new osg::Material;

        material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
        material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6f, 0.6f, 0.6f, 1.0f));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
        material->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);

        stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
        stateSet->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f), osg::StateAttribute::ON);
        stateSet->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON);
        stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

        return geometry;
    }

    EdgeStatistics collectEdgePolylineData(const TopoDS_Shape& shape, const DataExchangeOptions& options)
    {
        EdgeStatistics stats;

        TopExp_Explorer edgeEx(shape, TopAbs_EDGE);
        while (edgeEx.More())
        {
            const TopoDS_Edge& edge = TopoDS::Edge(edgeEx.Current());

            if (!BRep_Tool::Degenerated(edge))
            {
                BRepAdaptor_Curve curve(edge);
                GCPnts_TangentialDeflection discretizer(curve, options.edgeAngularDeflection, options.edgeCurvatureDeflection);

                if (discretizer.NbPoints() > 1)
                {
                    EdgePolylineData polyline;
                    polyline.points.reserve(static_cast<size_t>(discretizer.NbPoints()));

                    for (Standard_Integer i = 1; i <= discretizer.NbPoints(); ++i)
                    {
                        const gp_Pnt p = discretizer.Value(i);
                        polyline.points.emplace_back(static_cast<float>(p.X()), static_cast<float>(p.Y()), static_cast<float>(p.Z()));
                    }

                    stats.vertexCount += polyline.points.size();
                    stats.polylines.push_back(std::move(polyline));
                }
            }

            edgeEx.Next();
        }

        return stats;
    }

    osg::ref_ptr<osg::Geometry> buildEdgeGeometry(const TopoDS_Shape& shape, const DataExchangeOptions& options)
    {
        const EdgeStatistics stats = collectEdgePolylineData(shape, options);

        osg::ref_ptr<osg::Geometry> edgeGeometry = new osg::Geometry();
        osg::ref_ptr<osg::Vec3Array> edgeVertices = new osg::Vec3Array();
        edgeVertices->reserve(stats.vertexCount);
        edgeGeometry->setVertexArray(edgeVertices);

        osg::ref_ptr<osg::Vec4Array> edgeColors = new osg::Vec4Array();
        edgeColors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        edgeGeometry->setColorArray(edgeColors, osg::Array::BIND_OVERALL);

        for (const auto& polyline : stats.polylines)
        {
            if (polyline.points.size() > 1)
            {
                osg::ref_ptr<osg::DrawElementsUInt> lineStrip = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP);
                const unsigned int startIndex = static_cast<unsigned int>(edgeVertices->size());
                lineStrip->reserve(static_cast<unsigned int>(polyline.points.size()));

                for (size_t i = 0; i < polyline.points.size(); ++i)
                {
                    edgeVertices->push_back(polyline.points[i]);
                    lineStrip->push_back(startIndex + static_cast<unsigned int>(i));
                }

                edgeGeometry->addPrimitiveSet(lineStrip);
            }
        }

        if (!edgeVertices->empty())
        {
            osg::StateSet* edgeState = edgeGeometry->getOrCreateStateSet();
            edgeState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            edgeState->setAttributeAndModes(new osg::LineWidth(2.0f), osg::StateAttribute::ON);
            edgeState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
        }

        return edgeGeometry;
    }
}

osg::ref_ptr<osg::Node> convertTopoDSImageToOSG(const TopoDS_Shape& shape, const DataExchangeOptions& options)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    // Triangulate the shape
    BRepMesh_IncrementalMesh mesh(shape, options.meshDeflection);

    osg::ref_ptr<osg::Geometry> geometry = buildFaceGeometry(shape, options);
    geode->addDrawable(geometry);

    if (options.buildEdges)
    {
        osg::ref_ptr<osg::Geometry> edgeGeometry = buildEdgeGeometry(shape, options);
        if (edgeGeometry.valid() && edgeGeometry->getVertexArray() != nullptr && edgeGeometry->getVertexArray()->getNumElements() > 0)
        {
            geode->addDrawable(edgeGeometry);
        }
    }

    return geode;
}

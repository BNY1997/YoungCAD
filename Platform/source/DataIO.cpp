#include "DataIO.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <DataExchange.h>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <fstream>
#include <sstream>
#include <string>
#include "TriangleAlgorithm.h"

osg::ref_ptr<osg::Node> ObjDataIO::readData(const std::string& path)
{
	return osgDB::readNodeFile(path);
}

bool ObjDataIO::writeData(const osg::ref_ptr<osg::Node>& node, const std::string& path)
{
	return osgDB::writeObjectFile(*node, path);
}

osg::ref_ptr<osg::Node> TriangleMeshDataIO::readData(const std::string& path)
{

    std::ifstream ifs(path);
    if (!ifs.is_open())
        return osg::ref_ptr<osg::Node>();

    std::vector<gp_Pnt> points;
    std::string line;
    points.reserve(1024);

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        double x = 0, y = 0, z = 0;
        // format: x,y,z
        if (!std::getline(ss, token, ',')) continue;
        try { x = std::stod(token); } catch (...) { continue; }
        if (!std::getline(ss, token, ',')) continue;
        try { y = std::stod(token); } catch (...) { continue; }
        if (!std::getline(ss, token, ',')) continue;
        try { z = std::stod(token); } catch (...) { continue; }

        points.emplace_back(x, y, z);
    }

    if (points.size() < 3)
        return osg::ref_ptr<osg::Node>();

    // Run triangulation directly on OCC points (uses XY plane internally)
    std::vector<Triangle> triangles;
    BowyerWatson(points, triangles);

    // create OCCT compound of triangular faces
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);

    for (const auto& tri : triangles) {
        BRepBuilderAPI_MakePolygon poly;
        poly.Add(tri.p1);
        poly.Add(tri.p2);
        poly.Add(tri.p3);
        poly.Close();

        TopoDS_Wire wire = poly.Wire();
        BRepBuilderAPI_MakeFace mf(wire);
        if (!mf.IsDone()) continue;
        TopoDS_Face face = mf.Face();
        builder.Add(compound, face);
    }

    return convertTopoDSImageToOSG(compound, m_options);
}

bool TriangleMeshDataIO::writeData(const osg::ref_ptr<osg::Node>& node, const std::string& path)
{
	return false;
}

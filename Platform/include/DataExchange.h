#ifndef DataExchange_h__
#define DataExchange_h__

#include <osg/ref_ptr>
#include <osg/Node>

#include <TopoDS_Shape.hxx>

struct DataExchangeOptions
{
    double meshDeflection = 0.1;
    bool buildEdges = true;
    bool smoothNormals = true;
    double edgeAngularDeflection = 0.1;
    double edgeCurvatureDeflection = 0.1;
};

osg::ref_ptr<osg::Node> convertTopoDSImageToOSG(const TopoDS_Shape& shape, const DataExchangeOptions& options = DataExchangeOptions());

#endif

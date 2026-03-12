#ifndef DATAIO_H
#define DATAIO_H

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Node>

#include "DataExchange.h"

/// <summary>
/// 此类负责数据的输入输出操作，提供统一的接口来读取和写入各种格式的数据文件。
/// </summary>
class IDataIO
{
public:
	virtual ~IDataIO() = default;
	virtual osg::ref_ptr<osg::Node> readData(const std::string& path) = 0;
	virtual bool writeData(const osg::ref_ptr<osg::Node>& node, const std::string& path) = 0;
};

class ObjDataIO : public IDataIO
{
public:
	virtual osg::ref_ptr<osg::Node> readData(const std::string& path) override;
	virtual bool writeData(const osg::ref_ptr<osg::Node>& node, const std::string& path) override;
};

class TriangleMeshDataIO : public IDataIO
{
public:
	TriangleMeshDataIO() = default;
	explicit TriangleMeshDataIO(const DataExchangeOptions& options) : m_options(options) {}
	void setOptions(const DataExchangeOptions& options) { m_options = options; }
	const DataExchangeOptions& options() const { return m_options; }
	virtual osg::ref_ptr<osg::Node> readData(const std::string& path) override;
	virtual bool writeData(const osg::ref_ptr<osg::Node>& node, const std::string& path) override;

private:
	DataExchangeOptions m_options;
};


#endif
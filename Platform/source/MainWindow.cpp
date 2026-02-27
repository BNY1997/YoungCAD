#include "MainWindow.h"
#include "OSGWidget.h"
#include <osg/Material>

#include <QDebug>
#include <QMdiSubWindow>
#include <QMenuBar>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "BusyTodoDialog.h"
#include <QThread>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Node>
#include <osgUtil/SmoothingVisitor>
#include <TopoDS_Edge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <osg/LineWidth>
#include <osg/PolygonOffset>

osg::Node* convertTopoDSImageToOSG(const TopoDS_Shape& shape)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	geode->addDrawable(geometry);

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
	geometry->setVertexArray(vertices);
	geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

	// Triangulate the shape
	BRepMesh_IncrementalMesh mesh(shape, 0.1);

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

			gp_Trsf trsf = loc.Transformation();
			bool reverse = (face.Orientation() == TopAbs_REVERSED);

			// Because we are adding vertices per face, we just append
			int offset = vertices->size();

			for (int i = 1; i <= nbNodes; ++i)
			{
				gp_Pnt p = triangulation->Node(i).Transformed(trsf);
				vertices->push_back(osg::Vec3(p.X(), p.Y(), p.Z()));
				// Placeholder normal, will be smoothed later or computed from face
				normals->push_back(osg::Vec3(0, 0, 1));
			}

			osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
			for (int i = 1; i <= nbTriangles; ++i)
			{
				const Poly_Triangle& tri = triangulation->Triangle(i);
				Standard_Integer n1, n2, n3;
				tri.Get(n1, n2, n3);

				if (reverse)
					std::swap(n1, n3);

				indices->push_back(offset + n1 - 1);
				indices->push_back(offset + n2 - 1);
				indices->push_back(offset + n3 - 1);
			}
			geometry->addPrimitiveSet(indices);
		}
		ex.Next();
	}

	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();
	colors->push_back(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));
	geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

	// Generate normals
	osgUtil::SmoothingVisitor::smooth(*geometry);

	// Edge Extraction
	osg::ref_ptr<osg::Geometry> edgeGeometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> edgeVertices = new osg::Vec3Array();
	edgeGeometry->setVertexArray(edgeVertices);

	osg::ref_ptr<osg::Vec4Array> edgeColors = new osg::Vec4Array();
	edgeColors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Black color for edges
	edgeGeometry->setColorArray(edgeColors, osg::Array::BIND_OVERALL);

	TopExp_Explorer edgeEx(shape, TopAbs_EDGE);
	while (edgeEx.More())
	{
		const TopoDS_Edge& edge = TopoDS::Edge(edgeEx.Current());

		if (!BRep_Tool::Degenerated(edge))
		{
			BRepAdaptor_Curve curve(edge);
			// Discretize the curve
			// Angular deflection ~ 0.1 radians, Curvature deflection ~ 0.1 units
			GCPnts_TangentialDeflection discretizer(curve, 0.1, 0.1);

			if (discretizer.NbPoints() > 1)
			{
				osg::ref_ptr<osg::DrawElementsUInt> lineStrip = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP);
				int startIndex = edgeVertices->size();

				for (int i = 1; i <= discretizer.NbPoints(); ++i)
				{
					gp_Pnt p = discretizer.Value(i);
					edgeVertices->push_back(osg::Vec3(p.X(), p.Y(), p.Z()));
					lineStrip->push_back(startIndex + i - 1);
				}
				edgeGeometry->addPrimitiveSet(lineStrip);
			}
		}
		edgeEx.Next();
	}

	if (edgeVertices->size() > 0)
	{
		osg::StateSet* edgeState = edgeGeometry->getOrCreateStateSet();
		edgeState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		edgeState->setAttributeAndModes(new osg::LineWidth(2.0f), osg::StateAttribute::ON);
		geode->addDrawable(edgeGeometry);
	}

	osg::StateSet* stateSet = geometry->getOrCreateStateSet();
	osg::Material* material = new osg::Material;

	material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);

	// Set material properties for better realism
	material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
	material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.6f, 0.6f, 0.6f, 1.0f));
	material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	material->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);

	stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
	stateSet->setAttributeAndModes(new osg::PolygonOffset(1.0f, 1.0f), osg::StateAttribute::ON);
	stateSet->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON);
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

	return geode.release();
}

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QMenuBar* menuBar = this->menuBar();
	m_osgWidget = new OSGWidget(this);
	this->setCentralWidget(m_osgWidget);
	connect(ui->actionObj_File, &QAction::triggered, this, &MainWindow::onOpenObjFile);
	connect(ui->action_cube, &QAction::triggered, this, &MainWindow::onCreateOCCCube);
	connect(ui->action_cone, &QAction::triggered, this, &MainWindow::onCreateOCCCone);
	connect(ui->action_sphere, &QAction::triggered, this, &MainWindow::onCreateOCCSphere);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::onOpenObjFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), QString(), tr("OBJ Files (*.obj)"));
	if (!filePath.isEmpty())
	{
		bool success = BusyTodoDialog::executeWithBusyDialog(
			this,
			[&]() {
				readObjModel(filePath.toStdString());
			}, "正在处理数据..."
		);

	}
}

void MainWindow::readObjModel(const std::string& filePath)
{
	osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filePath);
	if (!node)
	{
		QMessageBox::warning(this, tr("Error"), tr("Failed to read STL file: %1").arg(QString::fromStdString(filePath)));
		return;
	}
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}

void MainWindow::onCreateOCCCube()
{
	TopoDS_Shape shape = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape);
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}

void MainWindow::onCreateOCCCone()
{
	// Radius1, Radius2, Height
	TopoDS_Shape shape = BRepPrimAPI_MakeCone(5.0, 0.0, 10.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape);
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}

void MainWindow::onCreateOCCSphere()
{
	TopoDS_Shape shape = BRepPrimAPI_MakeSphere(5.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape);
	if (m_osgWidget)
		m_osgWidget->setSceneData(node);
}



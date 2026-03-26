#include "MainWindow.h"
#include "OSGWidget.h"
#include "ShapePool.h"
#include <osg/Material>

#include <QDebug>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QPushButton>
#include <algorithm>

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
// move conversion implementation to DataExchange module
#include "DataExchange.h"
#include "DataIO.h"

namespace
{
QString buildObjCachePath(const QString& objPath)
{
	const QFileInfo info(objPath);
	const QString absPath = info.absoluteFilePath();
	const QString stamp = QStringLiteral("%1|%2|%3")
		.arg(absPath)
		.arg(info.size())
		.arg(info.lastModified().toMSecsSinceEpoch());

	const QByteArray digest = QCryptographicHash::hash(stamp.toUtf8(), QCryptographicHash::Sha1).toHex();

	QDir cacheDir(QDir::tempPath() + QStringLiteral("/YoungCAD/obj_cache"));
	cacheDir.mkpath(QStringLiteral("."));

	const QString base = info.completeBaseName();
	return cacheDir.filePath(base + QStringLiteral("_") + QString::fromLatin1(digest) + QStringLiteral(".osgb"));
}
}


MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags), ui(new Ui::MainWindow)
{
	m_dataExchangeOptions.meshDeflection = 0.1;
	m_dataExchangeOptions.buildEdges = true;
	m_dataExchangeOptions.smoothNormals = true;
	m_dataExchangeOptions.edgeAngularDeflection = 0.1;
	m_dataExchangeOptions.edgeCurvatureDeflection = 0.1;

	ui->setupUi(this);
	QMenuBar* menuBar = this->menuBar();
	m_osgWidget = new OSGWidget(this);
	this->setCentralWidget(m_osgWidget);
	m_projectTreeDock = new ProjectTreeDock(this);
	this->addDockWidget(Qt::LeftDockWidgetArea, m_projectTreeDock);
	// Bind the pool's root group to the viewer once; all subsequent
	// pool add/remove operations keep the scene graph in sync automatically.
	m_osgWidget->setSceneData(m_shapePool.getRoot());
	connect(m_projectTreeDock, &ProjectTreeDock::modelSelectionChanged, this, &MainWindow::onTreeModelSelected);
	connect(m_projectTreeDock, &ProjectTreeDock::modelDeleteRequested, this, &MainWindow::onDeleteModelRequested);
	connect(m_osgWidget, &OSGWidget::modelPicked, this, &MainWindow::onViewportModelPicked);
	connect(ui->actionObj_File, &QAction::triggered, this, &MainWindow::onOpenObjFile);
	connect(ui->action_cube, &QAction::triggered, this, &MainWindow::onCreateOCCCube);
	connect(ui->action_cone, &QAction::triggered, this, &MainWindow::onCreateOCCCone);
	connect(ui->action_sphere, &QAction::triggered, this, &MainWindow::onCreateOCCSphere);

	// create action for TXT point cloud import and add to existing '打开' menu if available

	connect(ui->action_text, &QAction::triggered, this, &MainWindow::onOpenTxtFile);
	createViewControlPanel();
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
		BusyTodoDialog::executeWithBusyDialog(
			this,
			[&]() {
				readObjModel(filePath.toStdString());
			}, tr("正在加载 OBJ 模型...")
		);

	}
}

void MainWindow::onOpenTxtFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Points TXT"), QString(), tr("Text Files (*.txt *.csv)"));
    if (filePath.isEmpty())
        return;

    // Read and convert in background to avoid blocking UI
    bool success = BusyTodoDialog::executeWithBusyDialog(
        this,
        [&]() {
            TriangleMeshDataIO io(createDataExchangeOptions());
            osg::ref_ptr<osg::Node> node = io.readData(filePath.toStdString());
            if (node) {
                const std::string name = QFileInfo(filePath).baseName().toStdString();
                // pool mutation and repaint must run on the UI thread
                QMetaObject::invokeMethod(this, [this, node, name]() {
					const int modelId = m_shapePool.addNode(name, node);
					tagNodeWithModelId(modelId);
					registerModel(modelId, QString::fromStdString(name), false);
					m_osgWidget->centerView();
                    m_osgWidget->update();
                }, Qt::QueuedConnection);
            }
        }, "正在加载点云并构建三角网..."
    );
}

DataExchangeOptions MainWindow::createDataExchangeOptions() const
{
	return m_dataExchangeOptions;
}

void MainWindow::readObjModel(const std::string& filePath)
{
	const QString qtPath = QString::fromStdString(filePath);
	const QString cachePath = buildObjCachePath(qtPath);
	QElapsedTimer timer;
	timer.start();

	osg::ref_ptr<osg::Node> node;
	bool loadedFromCache = false;
	if (QFileInfo::exists(cachePath))
	{
		node = osgDB::readNodeFile(cachePath.toStdString());
		loadedFromCache = (node != nullptr);
	}

	if (!node)
	{
		node = osgDB::readNodeFile(filePath);
		if (node)
		{
			// Cache binary scene for much faster re-import of unchanged OBJ files.
			osgDB::writeNodeFile(*node, cachePath.toStdString());
		}
	}

	if (!node)
	{
		QMetaObject::invokeMethod(this, [this, qtPath]() {
			QMessageBox::warning(this, tr("Error"), tr("Failed to read OBJ file: %1").arg(qtPath));
		}, Qt::QueuedConnection);
		return;
	}

	qDebug() << "OBJ import ms:" << timer.elapsed() << "source:" << (loadedFromCache ? "cache" : "obj");

	const QString modelName = QFileInfo(qtPath).baseName();
	QMetaObject::invokeMethod(this, [this, node, modelName]() {
		const int modelId = m_shapePool.addNode(modelName.toStdString(), node);
		tagNodeWithModelId(modelId);
		registerModel(modelId, modelName, false);
		if (m_osgWidget)
			m_osgWidget->centerView();
		if (m_osgWidget)
			m_osgWidget->update();
	}, Qt::QueuedConnection);
}

void MainWindow::onCreateOCCCube()
{
	TopoDS_Shape shape = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape, createDataExchangeOptions());
	const int modelId = m_shapePool.add("Cube", shape, node);
	tagNodeWithModelId(modelId);
	registerModel(modelId, QStringLiteral("Cube"), true);
	if (m_osgWidget)
		m_osgWidget->update();
}

void MainWindow::onCreateOCCCone()
{
	// Radius1, Radius2, Height
	TopoDS_Shape shape = BRepPrimAPI_MakeCone(5.0, 0.0, 10.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape, createDataExchangeOptions());
	const int modelId = m_shapePool.add("Cone", shape, node);
	tagNodeWithModelId(modelId);
	registerModel(modelId, QStringLiteral("Cone"), true);
	if (m_osgWidget)
		m_osgWidget->update();
}

void MainWindow::onCreateOCCSphere()
{
	TopoDS_Shape shape = BRepPrimAPI_MakeSphere(5.0).Shape();
	osg::ref_ptr<osg::Node> node = convertTopoDSImageToOSG(shape, createDataExchangeOptions());
	const int modelId = m_shapePool.add("Sphere", shape, node);
	tagNodeWithModelId(modelId);
	registerModel(modelId, QStringLiteral("Sphere"), true);
	if (m_osgWidget)
		m_osgWidget->update();
}

void MainWindow::onTreeModelSelected(int modelId)
{
	if (m_osgWidget)
		m_osgWidget->selectModelById(modelId);
}

void MainWindow::onViewportModelPicked(int modelId)
{
	if (m_projectTreeDock)
		m_projectTreeDock->setSelectedModel(modelId);
}

void MainWindow::onDeleteModelRequested(int modelId)
{
	if (modelId < 0)
		return;

	if (!m_shapePool.remove(modelId))
		return;

	if (m_projectTreeDock)
		m_projectTreeDock->removeModel(modelId);

	if (m_osgWidget)
	{
		if (m_osgWidget->selectedModelId() == modelId)
			m_osgWidget->selectModelById(-1);
		m_osgWidget->update();
	}
}

void MainWindow::registerModel(int modelId, const QString& displayName, bool hasOccShape)
{
	if (!m_projectTreeDock)
		return;

	const QString category = hasOccShape ? QStringLiteral("OCC") : QStringLiteral("Mesh");
	m_projectTreeDock->addModel(modelId, displayName, category);
}

void MainWindow::tagNodeWithModelId(int modelId)
{
	const ShapeEntry* entry = m_shapePool.get(modelId);
	if (!entry || !entry->node)
		return;

	entry->node->setUserValue("modelId", modelId);
	entry->node->setName(entry->name);
}

void MainWindow::createViewControlPanel()
{
	if (!m_osgWidget)
		return;

	auto* panel = new QFrame(m_osgWidget);
	panel->setObjectName(QStringLiteral("viewControlPanel"));
	panel->setStyleSheet(
		"#viewControlPanel {"
		" background-color: rgba(255,255,255,220);"
		" border: 1px solid #c8c8c8;"
		" border-radius: 8px;"
		"}"
		"#viewControlPanel QPushButton {"
		" min-width: 68px;"
		" padding: 4px 8px;"
		"}"
	);

	auto* layout = new QGridLayout(panel);
	layout->setContentsMargins(8, 8, 8, 8);
	layout->setSpacing(6);

	auto addButton = [this, layout, panel](const QString& text, int row, int col, OSGWidget::StandardView view) {
		QPushButton* button = new QPushButton(text, panel);
		layout->addWidget(button, row, col);
		connect(button, &QPushButton::clicked, this, [this, view]() {
			if (m_osgWidget)
				m_osgWidget->setStandardView(view);
		});
	};

	addButton(QStringLiteral("Top"), 0, 0, OSGWidget::StandardView::Top);
	addButton(QStringLiteral("Bottom"), 0, 1, OSGWidget::StandardView::Bottom);
	addButton(QStringLiteral("Front"), 1, 0, OSGWidget::StandardView::Front);
	addButton(QStringLiteral("Back"), 1, 1, OSGWidget::StandardView::Back);
	addButton(QStringLiteral("Right"), 2, 0, OSGWidget::StandardView::Right);
	addButton(QStringLiteral("Left"), 2, 1, OSGWidget::StandardView::Left);

	QPushButton* centerButton = new QPushButton(QStringLiteral("Center"), panel);
	layout->addWidget(centerButton, 3, 0, 1, 2);
	connect(centerButton, &QPushButton::clicked, this, [this]() {
		if (m_osgWidget)
			m_osgWidget->centerView();
	});

	panel->adjustSize();
	panel->show();
	m_viewControlPanel = panel;
	positionViewControlPanel();
}

void MainWindow::positionViewControlPanel()
{
	if (!m_osgWidget || !m_viewControlPanel)
		return;

	const int margin = 12;
	const int x = m_osgWidget->width() - m_viewControlPanel->width() - margin;
	const int y = margin;
	m_viewControlPanel->move(std::max(0, x), std::max(0, y));
	m_viewControlPanel->raise();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);
	positionViewControlPanel();
}



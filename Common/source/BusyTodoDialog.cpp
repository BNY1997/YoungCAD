#include "BusyTodoDialog.h"
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QScreen>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMovie>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEventLoop>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>
#include <QMessageBox>
#include <QDebug>

BusyTodoDialog::BusyTodoDialog(QWidget* parent,
    const QString& message,
    AnimationType animationType)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_message(message)
    , m_animationType(animationType)
{
    // 设置对话框属性
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 初始化UI
    initUI();

    // 设置动画
    switch (m_animationType) {
    case Animation_ProgressBar:
        setupProgressBarAnimation();
        break;
    case Animation_Gif:
        setupGifAnimation();
        break;
    case Animation_RotatingDot:
        setupRotatingDotAnimation();
        break;
    case Animation_Pulse:
        setupPulseAnimation();
        break;
    }

    // 初始化定时器
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout,
        this, &BusyTodoDialog::updateProgressAnimation);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &BusyTodoDialog::onTimeout);

    // 设置样式
    setStyleSheet(R"(
        BusyTodoDialog {
            background-color: rgba(255, 255, 255, 240);
            border-radius: 10px;
            border: 1px solid #e0e0e0;
        }
        QLabel#messageLabel {
            color: #333333;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton#cancelButton {
            background-color: #f5f5f5;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 6px 16px;
            color: #666;
            font-size: 12px;
        }
        QPushButton#cancelButton:hover {
            background-color: #e9e9e9;
            border-color: #ccc;
        }
        QPushButton#cancelButton:pressed {
            background-color: #ddd;
        }
    )");
}

BusyTodoDialog::~BusyTodoDialog()
{
    clearAnimation();
}

void BusyTodoDialog::initUI()
{
    // 主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(30, 30, 30, 30);

    // 动画区域布局
    m_animationLayout = new QHBoxLayout();
    m_animationLayout->setSpacing(15);
    m_animationLayout->setAlignment(Qt::AlignCenter);

    // 消息标签
    m_labelMessage = new QLabel(m_message, this);
    m_labelMessage->setObjectName("messageLabel");
    m_labelMessage->setAlignment(Qt::AlignCenter);

    // 动画标签（用于显示GIF或图标）
    m_labelAnimation = new QLabel(this);
    m_labelAnimation->setAlignment(Qt::AlignCenter);
    m_labelAnimation->setFixedSize(60, 60);

    // 进度条（用于传统进度条模式）
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);  // 不确定模式
    m_progressBar->setFixedWidth(200);
    m_progressBar->setTextVisible(false);

    // 取消按钮
    m_buttonCancel = new QPushButton("取消", this);
    m_buttonCancel->setObjectName("cancelButton");
    m_buttonCancel->setVisible(m_cancelable);
    connect(m_buttonCancel, &QPushButton::clicked,
        this, &BusyTodoDialog::onCancelClicked);

    // 根据动画类型添加组件
    switch (m_animationType) {
    case Animation_ProgressBar:
        m_animationLayout->addWidget(m_progressBar);
        break;
    case Animation_Gif:
    case Animation_Pulse:
        m_animationLayout->addWidget(m_labelAnimation);
        break;
    case Animation_RotatingDot:
        createRotatingDots();
        break;
    }

    // 添加到主布局
    m_mainLayout->addLayout(m_animationLayout);
    m_mainLayout->addWidget(m_labelMessage);
    m_mainLayout->addWidget(m_buttonCancel, 0, Qt::AlignCenter);

    setLayout(m_mainLayout);

    // 设置固定大小
    setFixedSize(300, 200);
}

void BusyTodoDialog::setupProgressBarAnimation()
{
    // 使用传统进度条的不确定模式
    m_progressBar->setRange(0, 0);
    m_progressBar->setVisible(true);

    // 也可以使用自定义绘制
    createCustomProgressBar();
}

void BusyTodoDialog::setupGifAnimation()
{
    clearAnimation();

    // 加载内置GIF资源（需要将GIF文件添加到Qt资源文件）
    m_gifMovie = new QMovie(":/animations/busy.gif", QByteArray(), this);

    if (m_gifMovie->isValid()) {
        m_labelAnimation->setMovie(m_gifMovie);
        m_gifMovie->start();
    }
    else {
        // 如果没有GIF资源，回退到旋转进度条
        qWarning() << "GIF resource not found, falling back to progress bar animation";
        m_animationType = Animation_ProgressBar;
        setupProgressBarAnimation();
    }
}

void BusyTodoDialog::setupRotatingDotAnimation()
{
    clearAnimation();
    m_dotIndex = 0;

    // 创建三个点
    for (int i = 0; i < 3; ++i) {
        QLabel* dot = new QLabel("*", this);
        dot->setStyleSheet(QString("color: %1; font-size: 20px;")
            .arg(i == 0 ? "#3498db" : "#bdc3c7"));
        dot->setAlignment(Qt::AlignCenter);
        m_dotLabels.append(dot);
        m_animationLayout->addWidget(dot);
    }

    // 启动定时器更新点
    m_animationTimer->start(m_animationSpeed);
}

void BusyTodoDialog::setupPulseAnimation()
{
    clearAnimation();

    // 创建脉冲动画
    m_labelAnimation->setFixedSize(40, 40);
    m_labelAnimation->setStyleSheet(
        "border-radius: 20px;"
        "background-color: qradialgradient("
        "cx: 0.5, cy: 0.5, radius: 0.5,"
        "fx: 0.5, fy: 0.5,"
        "stop: 0 rgba(52, 152, 219, 255),"
        "stop: 1 rgba(52, 152, 219, 0));"
    );

    // 缩放动画
    m_rotationAnimation = new QPropertyAnimation(m_labelAnimation, "geometry", this);
    m_rotationAnimation->setDuration(1000);
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->setStartValue(QRect(10, 10, 40, 40));
    m_rotationAnimation->setEndValue(QRect(0, 0, 60, 60));
    m_rotationAnimation->start();
}

void BusyTodoDialog::clearAnimation()
{
    if (m_gifMovie) {
        m_gifMovie->stop();
        delete m_gifMovie;
        m_gifMovie = nullptr;
    }

    if (m_rotationAnimation) {
        m_rotationAnimation->stop();
        delete m_rotationAnimation;
        m_rotationAnimation = nullptr;
    }

    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->stop();
    }

    // 清理点标签
    for (QLabel* dot : m_dotLabels) {
        if (dot) {
            m_animationLayout->removeWidget(dot);
            dot->deleteLater();
        }
    }
    m_dotLabels.clear();
}

void BusyTodoDialog::createCustomProgressBar()
{
    // 使用自定义绘制代替传统进度条
    m_progressBar->setVisible(false);

    // 创建自定义绘制的标签
    m_labelAnimation->setFixedSize(60, 60);
    m_labelAnimation->setStyleSheet("background: transparent;");

    // 启动旋转动画
    m_rotationAnimation = new QPropertyAnimation(this, "rotationAngle");
    m_rotationAnimation->setDuration(1000);
    m_rotationAnimation->setStartValue(0);
    m_rotationAnimation->setEndValue(360);
    m_rotationAnimation->setLoopCount(-1);
    m_rotationAnimation->start();
}

void BusyTodoDialog::createRotatingDots()
{
    // 已经在 setupRotatingDotAnimation 中实现
}

void BusyTodoDialog::setMessage(const QString& message)
{
    m_message = message;
    if (m_labelMessage) {
        m_labelMessage->setText(message);
    }
}

QString BusyTodoDialog::message() const
{
    return m_message;
}

void BusyTodoDialog::setAnimationType(AnimationType type)
{
    if (m_animationType != type) {
        m_animationType = type;
        clearAnimation();

        // 清除原有布局
        QLayoutItem* item;
        while ((item = m_animationLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->setVisible(false);
                m_animationLayout->removeWidget(item->widget());
            }
            delete item;
        }

        // 设置新动画
        switch (type) {
        case Animation_ProgressBar:
            setupProgressBarAnimation();
            break;
        case Animation_Gif:
            setupGifAnimation();
            break;
        case Animation_RotatingDot:
            setupRotatingDotAnimation();
            break;
        case Animation_Pulse:
            setupPulseAnimation();
            break;
        }
    }
}

BusyTodoDialog::AnimationType BusyTodoDialog::animationType() const
{
    return m_animationType;
}

void BusyTodoDialog::setCancelable(bool cancelable)
{
    m_cancelable = cancelable;
    if (m_buttonCancel) {
        m_buttonCancel->setVisible(cancelable);
    }
}

bool BusyTodoDialog::isCancelable() const
{
    return m_cancelable;
}

void BusyTodoDialog::setProgressRange(int minimum, int maximum)
{
    if (m_progressBar) {
        m_progressBar->setRange(minimum, maximum);
        m_progressBar->setTextVisible(maximum > 0);
    }
}

void BusyTodoDialog::setProgressValue(int value)
{
    if (m_progressBar) {
        m_progressBar->setValue(value);
    }
}

void BusyTodoDialog::setAnimationSpeed(int speedMs)
{
    m_animationSpeed = speedMs;
    if (m_animationTimer && m_animationTimer->isActive()) {
        m_animationTimer->start(speedMs);
    }
}

int BusyTodoDialog::animationSpeed() const
{
    return m_animationSpeed;
}

void BusyTodoDialog::showWithDelay(int delayMs)
{
    if (delayMs > 0) {
        connect(m_showTimer, &QTimer::timeout, this, &QWidget::show);
        m_showTimer->start(delayMs);
    }
    else {
        show();
    }
}

void BusyTodoDialog::hideWithFade(int fadeMs)
{
    if (!m_opacityEffect) {
        m_opacityEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(m_opacityEffect);
    }

    m_fadeAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_fadeAnimation->setDuration(fadeMs);
    m_fadeAnimation->setStartValue(1.0);
    m_fadeAnimation->setEndValue(0.0);
    m_fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        hide();
        if (m_opacityEffect) {
            m_opacityEffect->setOpacity(1.0);
        }
        });

    m_fadeAnimation->start();
}

bool BusyTodoDialog::executeWithBusyDialog(QWidget* parent,
    const std::function<void()>& task,
    const QString& message,
    bool cancelable,
    int timeoutMs)
{
    BusyTodoDialog dialog(parent, message, Animation_ProgressBar);
    dialog.setCancelable(cancelable);

    if (timeoutMs > 0) {
        dialog.m_timeoutTimer->start(timeoutMs);
    }

    // 异步执行任务
    QFuture<void> future = QtConcurrent::run(task);
    QFutureWatcher<void> watcher;

    QEventLoop loop;
    QObject::connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);
    QObject::connect(&dialog, &BusyTodoDialog::cancelled, &loop, &QEventLoop::quit);
    QObject::connect(&dialog, &BusyTodoDialog::timeout, &loop, &QEventLoop::quit);

    watcher.setFuture(future);
    dialog.show();

    // 处理事件循环
    loop.exec();

    bool success = future.isFinished() && !future.isCanceled();

    if (dialog.m_userCancelled) {
        future.cancel();
        return false;
    }

    if (dialog.m_timeoutTimer->isActive()) {
        dialog.m_timeoutTimer->stop();
    }

    return success;
}

bool BusyTodoDialog::executeWithCancelableBusyDialog(QWidget* parent,
    const std::function<bool(bool&)>& task,
    const QString& message,
    int timeoutMs)
{
    BusyTodoDialog dialog(parent, message, Animation_ProgressBar);
    dialog.setCancelable(true);

    if (timeoutMs > 0) {
        dialog.m_timeoutTimer->start(timeoutMs);
    }

    bool taskCancelled = false;
    bool taskSuccess = false;

    // 异步执行任务
    QFuture<bool> future = QtConcurrent::run([&task, &taskCancelled]() {
        return task(taskCancelled);
        });

    QFutureWatcher<bool> watcher;
    QEventLoop loop;

    QObject::connect(&watcher, &QFutureWatcher<bool>::finished, &loop, &QEventLoop::quit);
    QObject::connect(&dialog, &BusyTodoDialog::cancelled, &loop, [&taskCancelled, &loop]() {
        taskCancelled = true;
        loop.quit();
        });
    QObject::connect(&dialog, &BusyTodoDialog::timeout, &loop, &QEventLoop::quit);

    watcher.setFuture(future);
    dialog.show();

    loop.exec();

    if (dialog.m_timeoutTimer->isActive()) {
        dialog.m_timeoutTimer->stop();
    }

    if (future.isFinished()) {
        taskSuccess = future.result();
    }

    return !taskCancelled && taskSuccess;
}

void BusyTodoDialog::startAnimation()
{
    switch (m_animationType) {
    case Animation_ProgressBar:
        if (m_rotationAnimation) {
            m_rotationAnimation->start();
        }
        break;
    case Animation_Gif:
        if (m_gifMovie) {
            m_gifMovie->start();
        }
        break;
    case Animation_RotatingDot:
        m_animationTimer->start(m_animationSpeed);
        break;
    case Animation_Pulse:
        if (m_rotationAnimation) {
            m_rotationAnimation->start();
        }
        break;
    }
}

void BusyTodoDialog::stopAnimation()
{
    switch (m_animationType) {
    case Animation_ProgressBar:
        if (m_rotationAnimation) {
            m_rotationAnimation->stop();
        }
        break;
    case Animation_Gif:
        if (m_gifMovie) {
            m_gifMovie->stop();
        }
        break;
    case Animation_RotatingDot:
        m_animationTimer->stop();
        break;
    case Animation_Pulse:
        if (m_rotationAnimation) {
            m_rotationAnimation->stop();
        }
        break;
    }
}

void BusyTodoDialog::updateProgressAnimation()
{
    // 旋转进度条动画
    if (m_animationType == Animation_ProgressBar) {
        update();
    }
    // 旋转点动画
    else if (m_animationType == Animation_RotatingDot) {
        updateRotatingDots();
    }
}

void BusyTodoDialog::onCancelClicked()
{
    m_userCancelled = true;
    emit cancelled();
    reject();
}

void BusyTodoDialog::onTimeout()
{
    emit timeout();
    reject();
    QMessageBox::warning(this, "Overtime", "cancel", QMessageBox::Ok);
}

void BusyTodoDialog::rotateProgressBar()
{
    m_rotationAngle = (m_rotationAngle + 5) % 360;
    update();
}

void BusyTodoDialog::updateRotatingDots()
{
    m_dotIndex = (m_dotIndex + 1) % 3;

    for (int i = 0; i < m_dotLabels.size(); ++i) {
        QString color = (i == m_dotIndex) ? "#3498db" : "#bdc3c7";
        m_dotLabels[i]->setStyleSheet(
            QString("color: %1; font-size: 20px; font-weight: bold;").arg(color));
    }
}

void BusyTodoDialog::setRotationAngle(int angle)
{
    m_rotationAngle = angle;
    update();
}

void BusyTodoDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    startAnimation();

    // 居中显示
    if (parentWidget()) {
        QPoint center = parentWidget()->geometry().center();
        move(center.x() - width() / 2, center.y() - height() / 2);
    }
    else {
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        move(screenGeometry.center() - rect().center());
    }
}

void BusyTodoDialog::hideEvent(QHideEvent* event)
{
    QDialog::hideEvent(event);
    stopAnimation();
}

void BusyTodoDialog::closeEvent(QCloseEvent* event)
{
    if (m_cancelable) {
        event->accept();
    }
    else {
        event->ignore();  // 禁止通过关闭按钮关闭
    }
}

void BusyTodoDialog::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

        // 绘制背景
        QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角矩形背景
    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    painter.fillPath(path, QColor(255, 255, 255, 240));
    painter.strokePath(path, QPen(QColor(224, 224, 224), 1));

    // 如果是自定义进度条模式，绘制旋转进度条
    if (m_animationType == Animation_ProgressBar && !m_progressBar->isVisible()) {
        painter.save();
        painter.translate(width() / 2, 60);
        painter.rotate(m_rotationAngle);

        QPen pen(m_progressColor);
        pen.setWidth(m_progressThickness);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        // 绘制弧形
        int radius = m_progressRadius;
        QRectF rect(-radius, -radius, radius * 2, radius * 2);
        painter.drawArc(rect, 0, 16 * 270);  // 绘制270度的弧

        painter.restore();
    }
}
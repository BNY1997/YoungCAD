#ifndef BUSYTODODIALOG_H
#define BUSYTODODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QMovie>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "Common_Export.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QProgressBar;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
QT_END_NAMESPACE

/**
 * @brief 繁忙状态对话框
 *
 * 用于在程序执行耗时操作时显示，提供多种动画效果：
 * 1. 旋转进度条
 * 2. GIF动画
 * 3. 文本动画
 * 4. 淡入淡出效果
 */
class Common_EXPORT BusyTodoDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int rotationAngle READ rotationAngle WRITE setRotationAngle)

public:
    enum AnimationType {
        Animation_ProgressBar,  // 旋转进度条
        Animation_Gif,          // GIF动画
        Animation_RotatingDot,  // 旋转点
        Animation_Pulse         // 脉冲效果
    };

    /**
     * @brief 构造函数
     * @param parent 父窗口
     * @param message 显示的消息
     * @param animationType 动画类型
     */
    explicit BusyTodoDialog(QWidget* parent = nullptr,
        const QString& message = "处理中，请稍候...",
        AnimationType animationType = Animation_ProgressBar);

    virtual ~BusyTodoDialog();

    // 设置消息文本
    void setMessage(const QString& message);
    QString message() const;

    // 设置动画类型
    void setAnimationType(AnimationType type);
    AnimationType animationType() const;

    // 设置是否显示取消按钮
    void setCancelable(bool cancelable);
    bool isCancelable() const;

    // 设置进度条范围（仅对进度条模式有效）
    void setProgressRange(int minimum, int maximum);
    void setProgressValue(int value);

    // 设置动画速度
    void setAnimationSpeed(int speedMs);  // 毫秒
    int animationSpeed() const;

    // 控制显示/隐藏
    void showWithDelay(int delayMs = 0);  // 延迟显示
    void hideWithFade(int fadeMs = 300);  // 淡出隐藏

    // 静态方法：显示繁忙对话框并执行任务
    static bool executeWithBusyDialog(QWidget* parent,
        const std::function<void()>& task,
        const QString& message = "处理中...",
        bool cancelable = false,
        int timeoutMs = 0);

    // 静态方法：显示繁忙对话框并执行可取消任务
    static bool executeWithCancelableBusyDialog(QWidget* parent,
        const std::function<bool(bool&)>& task,
        const QString& message = "处理中...",
        int timeoutMs = 0);

signals:
    void cancelled();  // 用户取消操作
    void timeout();    // 操作超时

public slots:
    void startAnimation();
    void stopAnimation();
    void updateProgressAnimation();

protected:
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

private slots:
    void onCancelClicked();
    void onTimeout();
    void rotateProgressBar();
    void updateRotatingDots();

private:
    // 旋转角度属性（用于动画）
    int rotationAngle() const { return m_rotationAngle; }
    void setRotationAngle(int angle);

    // 初始化UI
    void initUI();

    // 设置动画
    void setupProgressBarAnimation();
    void setupGifAnimation();
    void setupRotatingDotAnimation();
    void setupPulseAnimation();

    // 清除动画
    void clearAnimation();

    // 创建自定义旋转进度条
    void createCustomProgressBar();

    // 创建旋转点动画
    void createRotatingDots();

private:
    // UI组件
    QLabel* m_labelMessage = nullptr;
    QLabel* m_labelAnimation = nullptr;
    QLabel* m_labelIcon = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QPushButton* m_buttonCancel = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
    QHBoxLayout* m_animationLayout = nullptr;
    QTimer* m_animationTimer = nullptr;
    QTimer* m_showTimer = nullptr;
    QTimer* m_timeoutTimer = nullptr;
    QMovie* m_gifMovie = nullptr;
    QPropertyAnimation* m_rotationAnimation = nullptr;
    QGraphicsOpacityEffect* m_opacityEffect = nullptr;

    // 状态
    AnimationType m_animationType = Animation_ProgressBar;
    bool m_cancelable = false;
    bool m_userCancelled = false;
    QString m_message;

    // 动画相关
    int m_rotationAngle = 0;
    int m_dotIndex = 0;
    int m_animationSpeed = 50;  // 毫秒
    QVector<QLabel*> m_dotLabels;

    // 自定义绘制
    QColor m_progressColor = QColor(41, 128, 185);
    int m_progressRadius = 20;
    int m_progressThickness = 4;

    // 透明度效果
    QPropertyAnimation* m_fadeAnimation = nullptr;
};

#endif // BUSYTODODIALOG_H
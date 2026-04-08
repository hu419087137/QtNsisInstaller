#include "ProgressPage.h"
#include "ui_ProgressPage.h"
#include <QDateTime>
#include <QTextCursor>

const QStringList ProgressPage::kInstallSteps = {
    "正在验证安装包完整性...",
    "正在创建安装目录...",
    "正在解压核心运行库...",
    "正在复制应用程序主程序...",
    "正在安装依赖组件...",
    "正在安装插件模块...",
    "正在复制资源文件...",
    "正在配置系统环境变量...",
    "正在注册文件关联...",
    "正在创建桌面快捷方式...",
    "正在创建开始菜单条目...",
    "正在写入注册表信息...",
    "正在更新系统文件缓存...",
    "正在执行安装后脚本...",
    "正在完成安装...",
};

ProgressPage::ProgressPage(QWidget *parent)
    : QWizardPage(parent)
    , _ui(new Ui::ProgressPage)
    , _timer(new QTimer(this))
    , _currentStep(0)
    , _installFinished(false)
{
    setTitle("正在安装");
    setSubTitle("请稍候，正在将 MyApp 安装到您的计算机...");
    _ui->setupUi(this);
    _ui->progressBar->setRange(0, kInstallSteps.size());

    connect(_timer, &QTimer::timeout, this, &ProgressPage::slotInstallStep);
}

ProgressPage::~ProgressPage()
{
    delete _ui;
}

void ProgressPage::initializePage()
{
    _currentStep    = 0;
    _installFinished = false;
    _ui->progressBar->setValue(0);
    _ui->logBrowser->clear();
    _ui->statusLabel->setText("准备安装...");
    _ui->stepCountLabel->setText(
        QString("0 / %1").arg(kInstallSteps.size()));

    appendLog(QString("[%1] ===== 开始安装 MyApp 1.0.0 =====")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

    // Disable Back & Next buttons while installing
    wizard()->button(QWizard::BackButton)->setEnabled(false);
    wizard()->button(QWizard::NextButton)->setEnabled(false);

    _timer->start(550);
}

bool ProgressPage::isComplete() const
{
    return _installFinished;
}

void ProgressPage::slotInstallStep()
{
    if (_currentStep >= kInstallSteps.size()) {
        _timer->stop();
        _installFinished = true;
        _ui->statusLabel->setText("安装完成！");
        _ui->stepCountLabel->setText(
            QString("%1 / %1").arg(kInstallSteps.size()));
        _ui->progressBar->setValue(kInstallSteps.size());
        appendLog(QString("[%1] ===== MyApp 1.0.0 安装成功！=====")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        wizard()->button(QWizard::NextButton)->setEnabled(true);
        emit completeChanged();
        return;
    }

    const QString &step = kInstallSteps.at(_currentStep);
    _ui->statusLabel->setText(step);
    _ui->stepCountLabel->setText(
        QString("%1 / %2").arg(_currentStep + 1).arg(kInstallSteps.size()));
    _ui->progressBar->setValue(_currentStep + 1);
    appendLog(QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), step));

    ++_currentStep;
}

void ProgressPage::appendLog(const QString &message)
{
    _ui->logBrowser->append(message);
    QTextCursor cursor = _ui->logBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    _ui->logBrowser->setTextCursor(cursor);
}

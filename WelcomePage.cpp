#include "WelcomePage.h"
#include "ui_WelcomePage.h"
#include <QPainter>

WelcomePage::WelcomePage(QWidget *parent)
    : QWizardPage(parent)
    , _ui(new Ui::WelcomePage)
{
    setTitle("欢迎使用 MyApp 安装向导");
    setSubTitle("本向导将引导您完成 MyApp 1.0.0 的安装过程。");
    _ui->setupUi(this);
    setupIcon();
}

WelcomePage::~WelcomePage()
{
    delete _ui;
}

void WelcomePage::setupIcon()
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    // Blue rounded square background
    p.setPen(Qt::NoPen);
    QLinearGradient g(0, 0, 64, 64);
    g.setColorAt(0.0, QColor("#0063B1"));
    g.setColorAt(1.0, QColor("#0091FF"));
    p.setBrush(g);
    p.drawRoundedRect(0, 0, 64, 64, 14, 14);

    // White letter "M"
    p.setPen(Qt::white);
    p.setBrush(Qt::NoBrush);
    QFont font("Segoe UI", 28, QFont::Bold);
    p.setFont(font);
    p.drawText(QRect(0, 0, 64, 64), Qt::AlignCenter, "M");

    _ui->appIconLabel->setPixmap(pixmap);
}

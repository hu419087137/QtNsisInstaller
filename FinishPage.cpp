#include "FinishPage.h"
#include "ui_FinishPage.h"
#include <QPainter>
#include <QProcess>

FinishPage::FinishPage(QWidget *parent)
    : QWizardPage(parent)
    , _ui(new Ui::FinishPage)
{
    setTitle("安装完成");
    setSubTitle("MyApp 1.0.0 已成功安装到您的计算机。");
    _ui->setupUi(this);
    setupIcon();
}

FinishPage::~FinishPage()
{
    delete _ui;
}

bool FinishPage::validatePage()
{
    if (_ui->launchAppCheck->isChecked()) {
        // Launch the application after wizard closes
        const QString installDir = field("installDir").toString();
        const QString exePath    = installDir + "/MyApp.exe";
        QProcess::startDetached(exePath, {});
    }
    return true;
}

void FinishPage::setupIcon()
{
    QPixmap pixmap(52, 52);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    // Green circle background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#0F7B0F"));
    p.drawEllipse(0, 0, 52, 52);

    // White checkmark
    QPen pen(Qt::white, 4.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    QPolygonF check;
    check << QPointF(13, 26) << QPointF(22, 36) << QPointF(39, 16);
    p.drawPolyline(check);

    _ui->successIconLabel->setPixmap(pixmap);
}

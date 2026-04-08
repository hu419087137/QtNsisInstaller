#include "InstallerWizard.h"
#include "WelcomePage.h"
#include "LicensePage.h"
#include "InstallDirPage.h"
#include "ProgressPage.h"
#include "FinishPage.h"
#include <QPainter>
#include <QLinearGradient>

InstallerWizard::InstallerWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("MyApp 1.0.0 安装向导");
    setFixedSize(740, 520);
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::NoBackButtonOnStartPage, true);
    setOption(QWizard::NoBackButtonOnLastPage, true);
    setOption(QWizard::NoCancelButtonOnLastPage, true);
    setOption(QWizard::HaveNextButtonOnLastPage, false);

    setPixmap(QWizard::BannerPixmap, createBannerPixmap());
    setPixmap(QWizard::WatermarkPixmap, createWatermarkPixmap());

    setButtonText(QWizard::BackButton,   "< 上一步");
    setButtonText(QWizard::NextButton,   "下一步 >");
    setButtonText(QWizard::FinishButton, "完  成");
    setButtonText(QWizard::CancelButton, "取  消");
    setButtonText(QWizard::CommitButton, "安  装");

    setPage(E_WelcomePage,    new WelcomePage(this));
    setPage(E_LicensePage,    new LicensePage(this));
    setPage(E_InstallDirPage, new InstallDirPage(this));
    setPage(E_ProgressPage,   new ProgressPage(this));
    setPage(E_FinishPage,     new FinishPage(this));

    setupStyle();
}

QPixmap InstallerWizard::createBannerPixmap()
{
    QPixmap pixmap(580, 60);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient g(0, 0, 580, 0);
    g.setColorAt(0.0, QColor("#004EA8"));
    g.setColorAt(1.0, QColor("#0091FF"));
    p.fillRect(pixmap.rect(), g);
    // subtle circles
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 18));
    p.drawEllipse(460, -10, 120, 80);
    p.drawEllipse(520, 10, 60, 60);
    return pixmap;
}

QPixmap InstallerWizard::createWatermarkPixmap()
{
    QPixmap pixmap(160, 386);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient g(0, 0, 0, 386);
    g.setColorAt(0.0, QColor("#003D82"));
    g.setColorAt(0.5, QColor("#0063B1"));
    g.setColorAt(1.0, QColor("#0091FF"));
    p.fillRect(pixmap.rect(), g);
    // decorative circles
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 20));
    p.drawEllipse(-40, 260, 200, 200);
    p.setBrush(QColor(255, 255, 255, 12));
    p.drawEllipse(20, 80, 120, 120);
    p.setBrush(QColor(255, 255, 255, 10));
    p.drawEllipse(-20, 160, 80, 80);
    return pixmap;
}

void InstallerWizard::setupStyle()
{
    setStyleSheet(R"(
        QWizard {
            background: #FFFFFF;
        }
        QWizardPage {
            background: #FFFFFF;
        }
        QPushButton {
            min-height: 30px;
            min-width: 88px;
            padding: 0 18px;
            border-radius: 4px;
            font-size: 12px;
            font-family: "Segoe UI";
        }
        QPushButton#qt_wizard_next,
        QPushButton#qt_wizard_commit {
            background-color: #0078D4;
            color: white;
            border: none;
            font-weight: 600;
        }
        QPushButton#qt_wizard_next:hover,
        QPushButton#qt_wizard_commit:hover {
            background-color: #106EBE;
        }
        QPushButton#qt_wizard_next:pressed,
        QPushButton#qt_wizard_commit:pressed {
            background-color: #005A9E;
        }
        QPushButton#qt_wizard_next:disabled,
        QPushButton#qt_wizard_commit:disabled {
            background-color: #CCCCCC;
            color: #888888;
        }
        QPushButton#qt_wizard_back {
            background-color: transparent;
            color: #1A1A1A;
            border: 1px solid #C8C8C8;
        }
        QPushButton#qt_wizard_back:hover {
            background-color: #F0F0F0;
            border-color: #AAAAAA;
        }
        QPushButton#qt_wizard_back:disabled {
            color: #AAAAAA;
            border-color: #E0E0E0;
        }
        QPushButton#qt_wizard_cancel {
            background-color: transparent;
            color: #605E5C;
            border: 1px solid #C8C8C8;
        }
        QPushButton#qt_wizard_cancel:hover {
            background-color: #FDF3F2;
            color: #C42B1C;
            border-color: #C42B1C;
        }
        QPushButton#qt_wizard_finish {
            background-color: #0F7B0F;
            color: white;
            border: none;
            font-weight: 600;
        }
        QPushButton#qt_wizard_finish:hover {
            background-color: #0A6B0A;
        }
        QPushButton#qt_wizard_finish:pressed {
            background-color: #085508;
        }
        QGroupBox {
            font-size: 12px;
            font-weight: bold;
            color: #1A1A1A;
            border: 1px solid #E0E0E0;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 6px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            background: #FFFFFF;
        }
        QLineEdit {
            border: 1px solid #C8C8C8;
            border-radius: 4px;
            padding: 5px 8px;
            font-size: 12px;
            background: white;
            selection-background-color: #0078D4;
        }
        QLineEdit:focus {
            border-color: #0078D4;
            outline: none;
        }
        QRadioButton {
            font-size: 12px;
            color: #1A1A1A;
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }
        QCheckBox {
            font-size: 13px;
            color: #1A1A1A;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        QProgressBar {
            border: 1px solid #E0E0E0;
            border-radius: 5px;
            background: #F0F0F0;
            min-height: 22px;
            max-height: 22px;
            text-align: center;
            font-size: 11px;
            color: #1A1A1A;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0063B1, stop:1 #0091FF);
            border-radius: 4px;
        }
        QTextBrowser {
            border: 1px solid #E0E0E0;
            border-radius: 4px;
        }
    )");
}

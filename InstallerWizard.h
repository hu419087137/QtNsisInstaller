#pragma once
#include <QWizard>

class InstallerWizard : public QWizard
{
    Q_OBJECT
public:
    enum PageId {
        E_WelcomePage = 0,
        E_LicensePage,
        E_InstallDirPage,
        E_ProgressPage,
        E_FinishPage
    };

    explicit InstallerWizard(QWidget *parent = nullptr);

private:
    void setupStyle();
    QPixmap createBannerPixmap();
    QPixmap createWatermarkPixmap();
};

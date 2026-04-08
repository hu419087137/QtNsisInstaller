#pragma once
#include <QWizardPage>

QT_BEGIN_NAMESPACE
namespace Ui { class InstallDirPage; }
QT_END_NAMESPACE

class InstallDirPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit InstallDirPage(QWidget *parent = nullptr);
    ~InstallDirPage() override;

    bool isComplete() const override;
    void initializePage() override;

private slots:
    void slotBrowse();
    void slotDirChanged(const QString &dir);

private:
    void updateAvailableSpace(const QString &dir);

    Ui::InstallDirPage *_ui;
};

#pragma once
#include <QWizardPage>

QT_BEGIN_NAMESPACE
namespace Ui { class LicensePage; }
QT_END_NAMESPACE

class LicensePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit LicensePage(QWidget *parent = nullptr);
    ~LicensePage() override;

    bool isComplete() const override;

private slots:
    void slotAcceptanceChanged();

private:
    Ui::LicensePage *_ui;
};

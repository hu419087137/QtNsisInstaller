#pragma once
#include <QWizardPage>

QT_BEGIN_NAMESPACE
namespace Ui { class WelcomePage; }
QT_END_NAMESPACE

class WelcomePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);
    ~WelcomePage() override;

private:
    void setupIcon();

    Ui::WelcomePage *_ui;
};

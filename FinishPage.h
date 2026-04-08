#pragma once
#include <QWizardPage>

QT_BEGIN_NAMESPACE
namespace Ui { class FinishPage; }
QT_END_NAMESPACE

class FinishPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit FinishPage(QWidget *parent = nullptr);
    ~FinishPage() override;

    bool validatePage() override;

private:
    void setupIcon();

    Ui::FinishPage *_ui;
};

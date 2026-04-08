#pragma once
#include <QWizardPage>
#include <QTimer>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class ProgressPage; }
QT_END_NAMESPACE

class ProgressPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ProgressPage(QWidget *parent = nullptr);
    ~ProgressPage() override;

    void initializePage() override;
    bool isComplete() const override;

private slots:
    void slotInstallStep();

private:
    void appendLog(const QString &message);

    Ui::ProgressPage *_ui;
    QTimer           *_timer;
    int               _currentStep;
    bool              _installFinished;

    static const QStringList kInstallSteps;
};

#include "InstallDirPage.h"
#include "ui_InstallDirPage.h"
#include <QFileDialog>
#include <QStorageInfo>
#include <QDir>

InstallDirPage::InstallDirPage(QWidget *parent)
    : QWizardPage(parent)
    , _ui(new Ui::InstallDirPage)
{
    setTitle("选择安装位置");
    setSubTitle("请选择 MyApp 的安装目录，或保留默认路径直接继续。");
    setCommitPage(true);

    _ui->setupUi(this);
    registerField("installDir*", _ui->installDirEdit);

    connect(_ui->browseButton,   &QPushButton::clicked,
            this, &InstallDirPage::slotBrowse);
    connect(_ui->installDirEdit, &QLineEdit::textChanged,
            this, &InstallDirPage::slotDirChanged);
}

InstallDirPage::~InstallDirPage()
{
    delete _ui;
}

void InstallDirPage::initializePage()
{
    updateAvailableSpace(_ui->installDirEdit->text());
}

bool InstallDirPage::isComplete() const
{
    return !_ui->installDirEdit->text().trimmed().isEmpty();
}

void InstallDirPage::slotBrowse()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择安装目录",
        _ui->installDirEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (!dir.isEmpty()) {
        _ui->installDirEdit->setText(QDir::toNativeSeparators(dir + "/MyApp"));
    }
}

void InstallDirPage::slotDirChanged(const QString &dir)
{
    updateAvailableSpace(dir);
    emit completeChanged();
}

void InstallDirPage::updateAvailableSpace(const QString &dir)
{
    // Walk up to the first existing ancestor directory
    QDir d(dir);
    while (!d.exists() && !d.isRoot()) {
        d.cdUp();
    }

    const QStorageInfo storage(d.absolutePath());
    if (storage.isValid()) {
        const double availGb = storage.bytesAvailable() / (1024.0 * 1024.0 * 1024.0);
        _ui->availableSpaceLabel->setText(
            QString("%1 GB").arg(availGb, 0, 'f', 1));

        // Warn if less than 200 MB available
        const bool enoughSpace = storage.bytesAvailable() > 200LL * 1024 * 1024;
        _ui->availableSpaceLabel->setStyleSheet(
            enoughSpace
                ? "font-size: 12px; color: #1A1A1A; font-weight: bold;"
                : "font-size: 12px; color: #C42B1C; font-weight: bold;");
    } else {
        _ui->availableSpaceLabel->setText("未知");
    }
}

#include "InstallerWizard.h"
#include "ui_InstallerWizard.h"
#include "ui_CustomTitleBar.h"
#include "ui_WelcomePage.h"
#include "ui_LicensePage.h"
#include "ui_InstallPathPage.h"
#include "ui_ComponentsPage.h"
#include "ui_InstallProgressPage.h"
#include "ui_FinishDialog.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QShowEvent>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QStorageInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

namespace {

constexpr int kWizardMinWidth     = 650;
constexpr int kWizardMinHeight    = 500;
constexpr int kFinishDialogWidth  = 550;
constexpr int kFinishDialogHeight = 400;

constexpr int kShadowMargin  = 16;
constexpr int kShadowAlpha   = 90;
constexpr int kShadowOffsetY = 4;

bool detectWindowsDarkMode()
{
    return false;
#ifdef Q_OS_WIN
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;
#else
    return false;
#endif
}

/** Returns the root path of the mounted volume with the most free bytes.
 *  Skips read-only and very small volumes (< 1 GiB total).
 *  Falls back to the user's home directory on failure.
 */
QString findBestDriveRoot()
{
    qint64  maxFree  = -1;
    QString bestRoot;

    for (const QStorageInfo &vol : QStorageInfo::mountedVolumes()) {
        if (!vol.isValid() || !vol.isReady() || vol.isReadOnly())
            continue;
        if (vol.bytesTotal() < Q_INT64_C(1073741824)) // < 1 GiB — skip tiny volumes
            continue;
        if (vol.bytesAvailable() > maxFree) {
            maxFree  = vol.bytesAvailable();
            bestRoot = vol.rootPath();
        }
    }

    if (bestRoot.isEmpty())
        return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/";

    if (!bestRoot.endsWith('/') && !bestRoot.endsWith('\\'))
        bestRoot += '/';

    return bestRoot;
}

/** Formats a byte count as a human-readable string (e.g. "23.4 GB"). */
QString formatBytes(qint64 bytes)
{
    if (bytes >= Q_INT64_C(1073741824))
        return QString::number(bytes / 1073741824.0, 'f', 1) + " GB";
    if (bytes >= Q_INT64_C(1048576))
        return QString::number(bytes / 1048576.0, 'f', 1) + " MB";
    if (bytes >= 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    return QString::number(bytes) + " B";
}

void paintDropShadow(QPainter &p, int totalWidth, int totalHeight)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);

    const QRect cr(kShadowMargin, kShadowMargin,
                   totalWidth  - 2 * kShadowMargin,
                   totalHeight - 2 * kShadowMargin - kShadowOffsetY);

    const qreal alphaScale = 2.0 * kShadowAlpha / (kShadowMargin * (kShadowMargin + 1));
    for (int i = kShadowMargin; i >= 1; --i) {
        const int alpha = qRound(alphaScale * (kShadowMargin - i + 1));
        if (alpha <= 0) continue;
        p.setBrush(QColor(0, 0, 0, alpha));
        p.drawRoundedRect(
            QRectF(cr).adjusted(-i, -i + kShadowOffsetY, i, i + kShadowOffsetY),
            8.0 + i * 0.5, 8.0 + i * 0.5);
    }
}

} // namespace

// ===========================================================================
// CustomTitleBar
// ===========================================================================

CustomTitleBar::CustomTitleBar(QWidget *parent)
    : QWidget(parent), ui(new Ui::CustomTitleBar), _dragging(false)
{
    ui->setupUi(this);

    connect(ui->minimizeBtn, &QPushButton::clicked, this, &CustomTitleBar::sigMinimizeClicked);
    connect(ui->closeBtn,    &QPushButton::clicked, this, &CustomTitleBar::sigCloseClicked);
}

CustomTitleBar::~CustomTitleBar()
{
    delete ui;
}

void CustomTitleBar::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = true;
        _dragPos  = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
        event->accept();
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPosition().toPoint() - _dragPos);
        event->accept();
    }
}

void CustomTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = false;
        event->accept();
    }
}

// ===========================================================================
// InstallerWizard
// ===========================================================================

InstallerWizard::InstallerWizard(const QString &configFile,
                                 const QString &archivePath,
                                 const QString &appName,
                                 const QString &licenseFile,
                                 QWidget *parent)
    : QDialog(parent),
    ui(new Ui::InstallerWizard),
    _configFile(configFile),
    _archivePath(archivePath),
    _appName(appName),
    _licenseFile(licenseFile),
    _isDarkMode(detectWindowsDarkMode())
{
    setupWindow();
}

InstallerWizard::~InstallerWizard()
{
    delete ui;
}

void InstallerWizard::setupWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setMinimumSize(kWizardMinWidth  + 2 * kShadowMargin,
                   kWizardMinHeight + 2 * kShadowMargin + kShadowOffsetY);
    setAttribute(Qt::WA_TranslucentBackground, true);
    ui->setupUi(this);
    ui->titleBar->setTitle(tr("%1 Setup").arg(_appName));

    connect(ui->titleBar,  &CustomTitleBar::sigCloseClicked,    this, &InstallerWizard::reject);
    connect(ui->titleBar,  &CustomTitleBar::sigMinimizeClicked, this, &InstallerWizard::showMinimized);
    connect(ui->backBtn,   &QPushButton::clicked,               this, &InstallerWizard::slotBack);
    connect(ui->nextBtn,   &QPushButton::clicked,               this, &InstallerWizard::slotNext);
    connect(ui->cancelBtn, &QPushButton::clicked,               this, &InstallerWizard::reject);

    // Create pages and add to the stack (QStackedWidget takes ownership)
    _welcomePage    = new WelcomePage;
    _welcomePage->setAppName(_appName);
    _licensePage    = new LicensePage(_licenseFile);
    _pathPage       = new InstallPathPage(_appName);
    _componentsPage = new ComponentsPage;
    _progressPage   = new InstallProgressPage;
    _progressPage->setAppName(_appName);

    ui->pageStack->addWidget(_welcomePage);
    ui->pageStack->addWidget(_licensePage);
    ui->pageStack->addWidget(_pathPage);
    ui->pageStack->addWidget(_componentsPage);
    ui->pageStack->addWidget(_progressPage);
    ui->pageStack->setCurrentIndex(0);

    // QWizardPage::completeChanged() is a standard Qt signal — connect it
    // directly even though these pages are not inside a QWizard.
    connect(_licensePage,  &QWizardPage::completeChanged, this, &InstallerWizard::slotPageCompleteChanged);
    connect(_pathPage,     &QWizardPage::completeChanged, this, &InstallerWizard::slotPageCompleteChanged);
    connect(_progressPage, &QWizardPage::completeChanged, this, &InstallerWizard::slotPageCompleteChanged);

    updateNavigation();
    applyTheme();
}

void InstallerWizard::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect geo = screen->geometry();
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }
}

void InstallerWizard::paintEvent(QPaintEvent *e)
{
    // QDialog::paintEvent(e);
    QPainter p(this);
    paintDropShadow(p, width(), height());
}

void InstallerWizard::slotNext()
{
    if (_currentPage == kPageComponents) {
        // Components → Progress: save config first, then start extraction
        saveConfig();
        ++_currentPage;
        ui->pageStack->setCurrentIndex(_currentPage);
        updateNavigation();
        _progressPage->startExtraction(_archivePath, _pathPage->installPath());
    } else if (_currentPage == kPageProgress) {
        // Progress page complete → accept
        done(QDialog::Accepted);
    } else if (_currentPage < kPageCount - 1) {
        ++_currentPage;
        ui->pageStack->setCurrentIndex(_currentPage);
        updateNavigation();
    }
}

void InstallerWizard::slotBack()
{
    if (_currentPage > 0 && _currentPage < kPageProgress) {
        --_currentPage;
        ui->pageStack->setCurrentIndex(_currentPage);
        updateNavigation();
    }
}

void InstallerWizard::slotPageCompleteChanged()
{
    updateNavigation();
}

void InstallerWizard::updateNavigation()
{
    const bool isProgressPage = (_currentPage == kPageProgress);

    // Hide Back and Cancel during extraction — extraction cannot be undone
    ui->backBtn->setVisible(!isProgressPage);
    ui->cancelBtn->setVisible(!isProgressPage);

    if (isProgressPage) {
        ui->nextBtn->setText(tr("Finish"));
        QWizardPage *page = qobject_cast<QWizardPage *>(ui->pageStack->currentWidget());
        ui->nextBtn->setEnabled(page != nullptr && page->isComplete());
    } else {
        ui->backBtn->setEnabled(_currentPage > 0);
        const bool isInstallPage = (_currentPage == kPageComponents);
        ui->nextBtn->setText(isInstallPage ? tr("Install") : tr("Next >"));
        QWizardPage *page = qobject_cast<QWizardPage *>(ui->pageStack->currentWidget());
        ui->nextBtn->setEnabled(page == nullptr || page->isComplete());
    }
}

void InstallerWizard::applyTheme()
{
    if (_isDarkMode)
        ui->installerContainer->setStyleSheet(darkStyleSheet());
    // else
    //     ui->installerContainer->setStyleSheet(whiteStyleSheet());
}

QString InstallerWizard::darkStyleSheet() const
{
    return R"(
QFrame#installerContainer {
    background: #2b2b2b;
    border-radius: 8px;
}
CustomTitleBar QLabel {
    color: #e0e0e0;
    font-size: 11pt;
    font-weight: 500;
}
CustomTitleBar QPushButton {
    background: transparent;
    color: #aaaaaa;
    border: none;
    font-size: 18pt;
    padding-bottom: 6px;
}
CustomTitleBar QPushButton:hover {
    background: rgba(255, 255, 255, 25);
    color: #e0e0e0;
}
CustomTitleBar QPushButton#closeBtn {
    font-size: 20pt;
    padding-bottom: 4px;
}
CustomTitleBar QPushButton#closeBtn:hover {
    background: #e81123;
    color: white;
}
QStackedWidget {
    background: #2b2b2b;
}
QLabel {
    color: #e0e0e0;
}
QLineEdit {
    background: #3c3c3c;
    color: #e0e0e0;
    border: 1px solid #555;
    border-radius: 4px;
    padding: 6px;
}
QTextBrowser {
    background: #3c3c3c;
    color: #e0e0e0;
    border: 1px solid #555;
    border-radius: 4px;
}
QCheckBox {
    color: #e0e0e0;
}
QProgressBar {
    border: 1px solid #555;
    border-radius: 4px;
    text-align: center;
    background: #3c3c3c;
    color: #e0e0e0;
}
QProgressBar::chunk {
    background: #0078d4;
    border-radius: 3px;
}
QPushButton {
    background: #0078d4;
    color: white;
    border: none;
    border-radius: 4px;
    padding: 6px 16px;
}
QPushButton:hover {
    background: #1084d8;
}
QPushButton:disabled {
    background: #555;
    color: #888;
}
QPushButton#backBtn, QPushButton#cancelBtn {
    background: #3c3c3c;
    color: #e0e0e0;
    border: 1px solid #555;
}
QPushButton#backBtn:hover, QPushButton#cancelBtn:hover {
    background: #4a4a4a;
}
QPushButton#backBtn:disabled {
    background: #2f2f2f;
    color: #666;
    border: 1px solid #404040;
}
QFrame#navBar {
    background: #2b2b2b;
    border-top: 1px solid #404040;
    border-bottom-left-radius: 8px;
    border-bottom-right-radius: 8px;
}
    )";
}
QString InstallerWizard::whiteStyleSheet() const
{
    return R"(
CustomTitleBar QPushButton {
    background: transparent;
    color: #aaaaaa;
    border: none;
    font-size: 18pt;
    padding-bottom: 6px;
}
CustomTitleBar QPushButton:hover {
    background: rgba(255, 255, 255, 25);
    color: #e0e0e0;
}
CustomTitleBar QPushButton#closeBtn {
    font-size: 20pt;
    padding-bottom: 4px;
}
CustomTitleBar QPushButton#closeBtn:hover {
    background: #e81123;
    color: white;
}
)";
}

void InstallerWizard::saveConfig()
{
    if (_configFile.isEmpty())
        return;

    // Write integers for boolean fields so NSIS "${If} $x == "1"" works.
    QSettings ini(_configFile, QSettings::IniFormat);
    ini.beginGroup("Setup");
    ini.setValue("InstallPath",           _pathPage->installPath());
    ini.setValue("CreateDesktopShortcut", _componentsPage->desktopShortcut()   ? 1 : 0);
    ini.setValue("CreateStartMenu",       _componentsPage->startMenuShortcut() ? 1 : 0);
    ini.endGroup();
    ini.sync();

#ifdef Q_OS_WIN
    // Pre-write install path to registry so next run reads it back correctly
    // even when invoked standalone (without NSIS writing it post-install).
    if (!_appName.isEmpty()) {
        QSettings reg(QString("HKEY_CURRENT_USER\\Software\\%1").arg(_appName),
                      QSettings::NativeFormat);
        reg.setValue("InstallDir", _pathPage->installPath());
    }
#endif
}

// ===========================================================================
// WelcomePage
// ===========================================================================

WelcomePage::WelcomePage(QWidget *parent)
    : QWizardPage(parent), ui(new Ui::WelcomePage)
{
    ui->setupUi(this);
    ui->bgWidget->setAttribute(Qt::WA_StyledBackground);
    setTitle("");
    setSubTitle("");
}

WelcomePage::~WelcomePage()
{
    delete ui;
}

void WelcomePage::setAppName(const QString &appName)
{
    ui->titleLabel->setText(tr("Welcome to %1 Setup").arg(appName));
}

// ===========================================================================
// LicensePage
// ===========================================================================

LicensePage::LicensePage(const QString &licenseFile, QWidget *parent)
    : QWizardPage(parent), ui(new Ui::LicensePage)
{
    ui->setupUi(this);

    if (!licenseFile.isEmpty()) {
        QFile file(licenseFile);
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray raw = file.readAll();
            // Try UTF-8 first; fall back to the system locale (e.g. GBK on Chinese Windows)
            QString text = QString::fromUtf8(raw);
            if (text.contains(QChar::ReplacementCharacter))
                text = QString::fromLocal8Bit(raw);
            ui->licenseText->setPlainText(text);
        } else {
            ui->licenseText->setPlainText(
                tr("(Cannot open license file: %1)").arg(licenseFile));
        }
    }

    connect(ui->agreeCheckbox, &QCheckBox::toggled, this, &LicensePage::slotAgreeToggled);
}

LicensePage::~LicensePage()
{
    delete ui;
}

bool LicensePage::isComplete() const
{
    return ui->agreeCheckbox->isChecked();
}

void LicensePage::slotAgreeToggled(bool)
{
    emit completeChanged();
}

// ===========================================================================
// InstallPathPage
// ===========================================================================

InstallPathPage::InstallPathPage(const QString &appName, QWidget *parent)
    : QWizardPage(parent), ui(new Ui::InstallPathPage), _appName(appName)
{
    ui->setupUi(this);

    // 1. Try previous install path from registry (written by NSIS after a prior install)
    QString defaultPath;
#ifdef Q_OS_WIN
    const QString regKey =
        QString("HKEY_CURRENT_USER\\Software\\%1").arg(appName);
    QSettings reg(regKey, QSettings::NativeFormat);
    defaultPath = reg.value("InstallDir").toString();
#endif

    // 2. First install: default to the drive with the most free space
    if (defaultPath.isEmpty())
        defaultPath = findBestDriveRoot() + appName;

    ui->pathEdit->setText(QDir::toNativeSeparators(defaultPath));

    // Emit completeChanged whenever the path text changes so InstallerWizard
    // can re-validate the Next button.
    connect(ui->pathEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        emit completeChanged();
        updateDiskSpaceLabel(text);
    });

    updateDiskSpaceLabel(ui->pathEdit->text());

    connect(ui->browseBtn, &QPushButton::clicked, this, &InstallPathPage::slotBrowsePath);
}

InstallPathPage::~InstallPathPage()
{
    delete ui;
}

QString InstallPathPage::installPath() const
{
    return ui->pathEdit->text();
}

bool InstallPathPage::isComplete() const
{
    return !ui->pathEdit->text().trimmed().isEmpty();
}

void InstallPathPage::slotBrowsePath()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Installation Directory"), ui->pathEdit->text());
    if (!dir.isEmpty())
        ui->pathEdit->setText(QDir::toNativeSeparators(dir));
}

void InstallPathPage::updateDiskSpaceLabel(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        ui->diskSpaceLabel->clear();
        return;
    }

    // Walk up to the nearest existing ancestor so QStorageInfo can resolve the drive
    QDir dir(path);
    while (!dir.exists()) {
        if (!dir.cdUp()) break;
    }

    const QStorageInfo info(dir.absolutePath());
    if (!info.isValid() || !info.isReady()) {
        ui->diskSpaceLabel->setText(tr("Unable to retrieve disk information"));
        ui->diskSpaceLabel->setStyleSheet("color: #888888; font-size: 9pt;");
        return;
    }

    const qint64 avail = info.bytesAvailable();
    const QString rootNative = QDir::toNativeSeparators(info.rootPath());
    ui->diskSpaceLabel->setText(
        tr("%1 Available: %2").arg(rootNative, formatBytes(avail)));

    // Color-code by available space
    const char *color;
    if (avail < Q_INT64_C(104857600))       // < 100 MB
        color = "#d32f2f";                  // red
    else if (avail < Q_INT64_C(1073741824)) // < 1 GB
        color = "#f57c00";                  // orange
    else
        color = "#388e3c";                  // green

    ui->diskSpaceLabel->setStyleSheet(
        QString("color: %1; font-size: 9pt;").arg(color));
}

// ===========================================================================
// ComponentsPage
// ===========================================================================

ComponentsPage::ComponentsPage(QWidget *parent)
    : QWizardPage(parent), ui(new Ui::ComponentsPage)
{
    ui->setupUi(this);
}

ComponentsPage::~ComponentsPage()
{
    delete ui;
}

bool ComponentsPage::desktopShortcut() const
{
    return ui->desktopShortcut->isChecked();
}

bool ComponentsPage::startMenuShortcut() const
{
    return ui->startMenuShortcut->isChecked();
}

// ===========================================================================
// InstallProgressPage
// ===========================================================================

InstallProgressPage::InstallProgressPage(QWidget *parent)
    : QWizardPage(parent), ui(new Ui::InstallProgressPage)
{
    ui->setupUi(this);
    ui->bgWidget->setAttribute(Qt::WA_StyledBackground);
    setTitle("");
    setSubTitle("");
}

InstallProgressPage::~InstallProgressPage()
{
    if (_process && _process->state() != QProcess::NotRunning) {
        _process->kill();
        _process->waitForFinished(2000);
    }
    delete ui;
}

void InstallProgressPage::setAppName(const QString &appName)
{
    ui->bannerTitleLabel->setText(tr("Installing %1").arg(appName));
}

bool InstallProgressPage::isComplete() const
{
    return _done && _success;
}

void InstallProgressPage::startExtraction(const QString &archivePath, const QString &destPath)
{
    // If no archive specified (e.g. standalone test), complete immediately
    if (archivePath.isEmpty()) {
        ui->statusLabel->setText(tr("No archive specified — skipping extraction."));
        ui->progressBar->setValue(100);
        _done    = true;
        _success = true;
        emit completeChanged();
        return;
    }

    ui->statusLabel->setText(tr("Starting extraction..."));
    ui->progressBar->setValue(0);
    ui->currentFileLabel->setText("");

    // 7za.exe must reside next to QtInstaller.exe (bundled by NSIS)
    const QString sevenZa = QCoreApplication::applicationDirPath() + "/7za.exe";

    _process = new QProcess(this);
    // Merge stdout+stderr so readyReadStandardOutput delivers everything
    _process->setProcessChannelMode(QProcess::MergedChannels);

    connect(_process, &QProcess::readyReadStandardOutput,
            this, &InstallProgressPage::slotReadOutput);
    connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InstallProgressPage::slotProcessFinished);

    // -bsp1 : redirect progress stream (%) to stdout
    // -y    : assume yes to all prompts (overwrite existing files)
    // -o    : output directory (no space between -o and path)
    const QString outArg = QString("-o") + QDir::toNativeSeparators(destPath);
    _process->start(sevenZa, {
                                 "x", archivePath, outArg, "-y", "-bsp1"
                             });

    if (!_process->waitForStarted(5000)) {
        ui->statusLabel->setText(tr("Error: could not start 7za.exe"));
        _done    = true;
        _success = false;
        emit completeChanged();
    }
}

void InstallProgressPage::slotReadOutput()
{
    _outputBuffer += _process->readAllStandardOutput();

    // Find the last percentage value in the accumulated output.
    // 7za writes progress as "\r  35%" using carriage returns.
    static const QRegularExpression re(R"((\d{1,3})%)");
    QRegularExpressionMatchIterator it = re.globalMatch(QString::fromLocal8Bit(_outputBuffer));

    int lastPercent = -1;
    while (it.hasNext())
        lastPercent = it.next().captured(1).toInt();

    if (lastPercent >= 0 && lastPercent <= 100) {
        ui->progressBar->setValue(lastPercent);
        ui->statusLabel->setText(tr("Extracting... %1%").arg(lastPercent));
    }

    // Show the most recently extracted filename (7-zip logs "- filename" per file)
    const QString text = QString::fromLocal8Bit(_outputBuffer);
    const QStringList lines = text.split('\n');
    for (int i = lines.size() - 1; i >= 0; --i) {
        const QString trimmed = lines[i].trimmed();
        if (trimmed.startsWith("- ")) {
            const QString filename = trimmed.mid(2);
            ui->currentFileLabel->setText(
                filename.length() > 70 ? "..." + filename.right(67) : filename);
            break;
        }
    }

    // Prevent unbounded buffer growth for large archives
    if (_outputBuffer.size() > 8192)
        _outputBuffer = _outputBuffer.right(4096);
}

void InstallProgressPage::slotProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    _done = true;
    if (exitCode == 0 && status == QProcess::NormalExit) {
        _success = true;
        ui->progressBar->setValue(100);
        ui->statusLabel->setText(tr("Installation complete!"));
        ui->currentFileLabel->setText("");
    } else {
        _success = false;
        ui->statusLabel->setText(
            tr("Extraction failed (exit code %1). Check that 7za.exe is present.")
                .arg(exitCode));
    }
    emit completeChanged();
}

// ===========================================================================
// FinishDialog
// ===========================================================================

FinishDialog::FinishDialog(const QString &appPath, QWidget *parent)
    : QDialog(parent),
    _appPath(appPath),
    _isDarkMode(detectWindowsDarkMode()),
    ui(new Ui::FinishDialog)
{
    setupFramelessWindow();
}

FinishDialog::~FinishDialog()
{
    delete ui;
}

void FinishDialog::setupFramelessWindow()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kFinishDialogWidth  + 2 * kShadowMargin,
                 kFinishDialogHeight + 2 * kShadowMargin + kShadowOffsetY);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(kShadowMargin, kShadowMargin,
                                    kShadowMargin, kShadowMargin + kShadowOffsetY);
    outerLayout->setSpacing(0);

    auto *container = new QFrame(this);
    outerLayout->addWidget(container);

    ui->setupUi(container);
    ui->banner->setAttribute(Qt::WA_StyledBackground);

    ui->titleBar->setTitle(tr("Installation Complete"));
    connect(ui->titleBar,  &CustomTitleBar::sigCloseClicked, this, &FinishDialog::accept);
    connect(ui->finishBtn, &QPushButton::clicked,            this, &FinishDialog::slotRunAppClicked);

    applyDarkModeIfNeeded();
}

void FinishDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect geo = screen->geometry();
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }
}

void FinishDialog::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    paintDropShadow(p, width(), height());
}

void FinishDialog::applyDarkModeIfNeeded()
{
    const QString bg       = _isDarkMode ? "#2b2b2b"              : "white";
    const QString text     = _isDarkMode ? "#e0e0e0"              : "#333";
    const QString btnColor = _isDarkMode ? "#aaaaaa"              : "#666666";
    const QString btnHover = _isDarkMode ? "rgba(255,255,255,25)" : "#e5e5e5";

    setStyleSheet(QString(R"(
QFrame#finishContainer { background: %1; border-radius: 8px; }
QLabel { color: %2; }
QCheckBox { color: %2; }
CustomTitleBar QLabel { color: %2; font-size: 11pt; font-weight: 500; }
CustomTitleBar QPushButton { background: transparent; color: %3; border: none; font-size: 18pt; padding-bottom: 6px; }
CustomTitleBar QPushButton:hover { background: %4; color: %2; }
CustomTitleBar QPushButton#closeBtn { font-size: 20pt; padding-bottom: 4px; }
CustomTitleBar QPushButton#closeBtn:hover { background: #e81123; color: white; }
    )").arg(bg, text, btnColor, btnHover));
}

void FinishDialog::slotRunAppClicked()
{
    if (ui->runAppCheckbox->isChecked() && !_appPath.isEmpty()) {
        if (!QProcess::startDetached(_appPath, {})) {
            qWarning("FinishDialog::slotRunAppClicked: failed to launch '%s'",
                     qPrintable(_appPath));
        }
    }
    accept();
}

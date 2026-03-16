#pragma once

#include <QDialog>
#include <QWizardPage>
#include <QMouseEvent>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui {
class CustomTitleBar;
class InstallerWizard;
class WelcomePage;
class LicensePage;
class InstallPathPage;
class ComponentsPage;
class InstallProgressPage;
class FinishDialog;
}
QT_END_NAMESPACE

// ===========================================================================
// CustomTitleBar
// ===========================================================================

/** Draggable custom title bar for frameless windows. */
class CustomTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTitleBar(QWidget *parent = nullptr);
    ~CustomTitleBar();

    void setTitle(const QString &title);

signals:
    void sigCloseClicked();
    void sigMinimizeClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::CustomTitleBar *ui;
    QPoint _dragPos;
    bool   _dragging;
};

// ===========================================================================
// Forward declarations for page classes
// ===========================================================================
class WelcomePage;
class LicensePage;
class InstallPathPage;
class ComponentsPage;
class InstallProgressPage;

// ===========================================================================
// InstallerWizard
// ===========================================================================

/** Frameless installer window using QStackedWidget + manual navigation.
 *
 *  QWizard is intentionally avoided: on Windows 10/11, QWizard::ModernStyle
 *  uses Vista native drawing APIs that produce a black/transparent render
 *  when the host window uses Qt::FramelessWindowHint.
 *
 *  Pages: Welcome(0) → License(1) → InstallPath(2) → Components(3) → Progress(4).
 *  On page 3 ("Install"), settings are saved and .7z extraction begins.
 *  On page 4, Back/Cancel are hidden; "Finish" is enabled when extraction completes.
 */
class InstallerWizard : public QDialog
{
    Q_OBJECT
public:
    explicit InstallerWizard(const QString &configFile,
                             const QString &archivePath,
                             const QString &appName,
                             const QString &licenseFile,
                             QWidget *parent = nullptr);
    ~InstallerWizard();

    QString whiteStyleSheet() const;
protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

private slots:
    void slotNext();
    void slotBack();
    /** Re-evaluates the Next button enabled-state when a page reports change. */
    void slotPageCompleteChanged();

private:
    void    setupWindow();
    void    applyTheme();
    QString darkStyleSheet() const;
    void    saveConfig();
    /** Syncs Back/Next/Cancel button states and labels with _currentPage. */
    void    updateNavigation();

    Ui::InstallerWizard *ui;
    QString              _configFile;
    QString              _archivePath;
    QString              _appName;
    QString              _licenseFile;
    bool                 _isDarkMode;
    int                  _currentPage{0};

    // Page indices
    static constexpr int kPageWelcome    = 0;
    static constexpr int kPageLicense    = 1;
    static constexpr int kPagePath       = 2;
    static constexpr int kPageComponents = 3;
    static constexpr int kPageProgress   = 4;
    static constexpr int kPageCount      = 5;

    WelcomePage         *_welcomePage{nullptr};
    LicensePage         *_licensePage{nullptr};
    InstallPathPage     *_pathPage{nullptr};
    ComponentsPage      *_componentsPage{nullptr};
    InstallProgressPage *_progressPage{nullptr};
};

// ===========================================================================
// WelcomePage
// ===========================================================================

class WelcomePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);
    ~WelcomePage();

    /** Sets the application name displayed in the banner and intro text. */
    void setAppName(const QString &appName);

private:
    Ui::WelcomePage *ui;
};

// ===========================================================================
// LicensePage
// ===========================================================================

/** License agreement page.
 *  If @p licenseFile is non-empty the file is read (UTF-8) and shown in the
 *  text browser; otherwise the UI's default HTML content is kept.
 *  Next is disabled until the checkbox is ticked.
 */
class LicensePage : public QWizardPage
{
    Q_OBJECT
public:
    explicit LicensePage(const QString &licenseFile, QWidget *parent = nullptr);
    ~LicensePage();

    bool isComplete() const override;

private slots:
    void slotAgreeToggled(bool);

private:
    Ui::LicensePage *ui;
};

// ===========================================================================
// InstallPathPage
// ===========================================================================

/** Installation directory selection page.
 *
 *  Default path logic (in order):
 *   1. Previous install path read from HKCU\Software\<appName>\InstallDir
 *   2. Drive with the most free space + "\<appName>"
 */
class InstallPathPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit InstallPathPage(const QString &appName, QWidget *parent = nullptr);
    ~InstallPathPage();

    /** Returns the currently entered installation path. */
    QString installPath() const;
    bool    isComplete() const override;

private slots:
    void slotBrowsePath();

private:
    void    updateDiskSpaceLabel(const QString &path);

    Ui::InstallPathPage *ui;
    QString              _appName;
};

// ===========================================================================
// ComponentsPage
// ===========================================================================

/** Component/shortcut selection page shown before the final install step. */
class ComponentsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit ComponentsPage(QWidget *parent = nullptr);
    ~ComponentsPage();

    bool desktopShortcut() const;
    bool startMenuShortcut() const;

private:
    Ui::ComponentsPage *ui;
};

// ===========================================================================
// InstallProgressPage
// ===========================================================================

/** Progress page: launches 7za.exe to extract the archive and shows progress.
 *
 *  Call startExtraction() when this page becomes active. isComplete() returns
 *  true only after the process exits with code 0.
 */
class InstallProgressPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit InstallProgressPage(QWidget *parent = nullptr);
    ~InstallProgressPage();

    bool isComplete() const override;

    /** Sets the application name displayed in the progress banner. */
    void setAppName(const QString &appName);

    /** Start extraction of @p archivePath into @p destPath using 7za.exe. */
    void startExtraction(const QString &archivePath, const QString &destPath);

private slots:
    void slotReadOutput();
    void slotProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    Ui::InstallProgressPage *ui;
    QProcess   *_process{nullptr};
    QByteArray  _outputBuffer;
    bool        _done{false};
    bool        _success{false};
};

// ===========================================================================
// FinishDialog
// ===========================================================================

/** Frameless completion dialog displayed after a successful installation. */
class FinishDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FinishDialog(const QString &appPath, QWidget *parent = nullptr);
    ~FinishDialog();

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void slotRunAppClicked();

private:
    void setupFramelessWindow();
    void applyDarkModeIfNeeded();

    QString           _appPath;
    bool              _isDarkMode;
    Ui::FinishDialog *ui;
};

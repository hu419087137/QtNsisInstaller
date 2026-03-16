#include "InstallerWizard.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QtInstaller");
    app.setApplicationVersion("1.0.0");
    app.setStyle("Fusion");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption modeOption("mode",
                                  "Run mode: setup (installation wizard) or finish (completion dialog)",
                                  "mode", "setup");
    QCommandLineOption configOption("config", "Configuration file path", "file");
    QCommandLineOption archiveOption("archive",
                                     "Path to the .7z archive to extract during setup", "file");
    QCommandLineOption appNameOption("appname",
                                     "Application name (used for registry key and default install path)",
                                     "name", "MyApplication");
    QCommandLineOption licenseOption("license",
                                     "Path to the license text file (.txt) shown on the License page",
                                     "file");
    QCommandLineOption appPathOption("apppath",
                                     "Application path (for finish dialog)", "path");
    QCommandLineOption langOption("lang",
                                   "Language override, e.g. zh_CN or en. "
                                   "Defaults to the system locale.",
                                   "lang");

    parser.addOption(modeOption);
    parser.addOption(configOption);
    parser.addOption(archiveOption);
    parser.addOption(appNameOption);
    parser.addOption(licenseOption);
    parser.addOption(appPathOption);
    parser.addOption(langOption);
    parser.process(app);

    const QString mode         = parser.value(modeOption);
    const QString configFile   = parser.value(configOption);
    const QString archivePath  = parser.value(archiveOption);
    const QString appName      = parser.value(appNameOption);
    const QString licenseFile  = parser.value(licenseOption);
    const QString appPath      = parser.value(appPathOption);
    const QString langOverride = parser.value(langOption);

    // Load translator before any widget is constructed so that both
    // tr() calls and uic-generated translate() calls pick it up.
    const QString locale = langOverride.isEmpty() ? QLocale::system().name() : langOverride;
    QTranslator translator;
    if (!translator.load(":/i18n/qtinstaller_" + locale + ".qm")) {
        // Fall back to the base language (e.g. "zh" when "zh_TW" is not found)
        const int sep = locale.indexOf('_');
        if (sep > 0)
            translator.load(":/i18n/qtinstaller_" + locale.left(sep) + ".qm");
    }
    app.installTranslator(&translator);

    if (mode == "setup") {
        InstallerWizard wizard(configFile, archivePath, appName, licenseFile);
        return wizard.exec() == QDialog::Accepted ? 0 : 1;
    }
    else if (mode == "finish") {
        FinishDialog dlg(appPath);
        dlg.exec();
        return 0;
    }

    return 0;
}

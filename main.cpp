#include <QApplication>
#include "InstallerWizard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MyApp Installer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MyCompany");

    QFont font("Segoe UI", 9);
    app.setFont(font);

    InstallerWizard wizard;
    wizard.show();

    return app.exec();
}

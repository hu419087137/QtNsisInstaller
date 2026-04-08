#include "LicensePage.h"
#include "ui_LicensePage.h"

static const char *kLicenseText =
    "MIT License\n\n"
    "Copyright (c) 2025 MyCompany. All rights reserved.\n\n"
    "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
    "of this software and associated documentation files (the \"Software\"), to deal\n"
    "in the Software without restriction, including without limitation the rights\n"
    "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
    "copies of the Software, and to permit persons to whom the Software is\n"
    "furnished to do so, subject to the following conditions:\n\n"
    "The above copyright notice and this permission notice shall be included in all\n"
    "copies or substantial portions of the Software.\n\n"
    "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
    "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
    "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
    "SOFTWARE.\n\n"
    "------------------------------------------------------------\n\n"
    "附加使用条款\n\n"
    "1. 授权范围\n"
    "   本软件授予用户在个人计算机上安装和使用 MyApp 的非独占性许可。\n"
    "   本许可不可转让，且仅限个人或企业内部使用。\n\n"
    "2. 限制\n"
    "   您不得对本软件进行反向工程、反编译或分解。\n"
    "   您不得将本软件用于任何非法或未经授权的目的。\n"
    "   未经书面许可，不得对本软件进行再分发。\n\n"
    "3. 隐私政策\n"
    "   本软件不会收集任何个人身份信息。\n"
    "   使用数据仅存储于本地计算机，不会上传至任何服务器。\n\n"
    "4. 免责声明\n"
    "   MyCompany 对因使用本软件导致的任何损失不承担责任。\n"
    "   本软件按"现状"提供，不含任何明示或暗示的保证。\n\n"
    "5. 适用法律\n"
    "   本协议受中华人民共和国法律管辖。\n";

LicensePage::LicensePage(QWidget *parent)
    : QWizardPage(parent)
    , _ui(new Ui::LicensePage)
{
    setTitle("许可协议");
    setSubTitle("请仔细阅读以下许可协议。继续安装即表示您同意协议中的所有条款。");
    _ui->setupUi(this);
    _ui->licenseTextBrowser->setPlainText(kLicenseText);

    connect(_ui->acceptRadio,  &QRadioButton::toggled,
            this, &LicensePage::slotAcceptanceChanged);
    connect(_ui->declineRadio, &QRadioButton::toggled,
            this, &LicensePage::slotAcceptanceChanged);
}

LicensePage::~LicensePage()
{
    delete _ui;
}

bool LicensePage::isComplete() const
{
    return _ui->acceptRadio->isChecked();
}

void LicensePage::slotAcceptanceChanged()
{
    emit completeChanged();
}

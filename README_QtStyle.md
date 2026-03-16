# 使用 Qt 作为 NSIS 安装向导界面

本方案实现了**用 Qt 编写的美观安装界面 + NSIS 后端安装逻辑**的组合。

## 📁 项目结构

```
D:\QtDemo\2026\3\Installer\
├── QtStyle_Installer.nsi          # NSIS 主脚本
├── QtInstaller/                   # Qt 安装向导程序
│   ├── QtInstaller.pro
│   ├── qt_installer_main.cpp
│   ├── qt_installer_wizard.h
│   └── qt_installer_wizard.cpp
├── AppFiles/                      # 你的应用程序文件（需自行准备）
│   ├── MyApp.exe
│   ├── Qt6Core.dll
│   ├── Qt6Gui.dll
│   ├── Qt6Widgets.dll
│   └── platforms/
│       └── qwindows.dll
└── README.md                      # 本文件
```

## 🚀 使用步骤

### 1. 编译 Qt 安装向导

```bash
cd QtInstaller
qmake QtInstaller.pro
make release  # 或在 Qt Creator 中编译
```

编译完成后，将 `QtInstaller.exe` 及其依赖（Qt DLLs、plugins）放到 `QtInstaller/` 目录。

**快速部署 Qt 依赖：**
```bash
windeployqt --release QtInstaller.exe
```

### 2. 准备应用程序文件

将你的 Qt 应用程序及其依赖放到 `AppFiles/` 目录：

```
AppFiles/
├── MyApp.exe                # 主程序
├── Qt6Core.dll              # Qt 核心库
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── platforms/               # Qt 插件（必需！）
│   └── qwindows.dll
├── styles/                  # 可选
│   └── qwindowsvistastyle.dll
└── ... 其他依赖
```

**自动收集依赖：**
```bash
cd AppFiles
windeployqt --release MyApp.exe
```

### 3. 修改 NSIS 脚本

编辑 `QtStyle_Installer.nsi`，修改以下变量：

```nsis
!define APP_NAME      "MyQtApplication"    ; 应用名称
!define APP_VERSION   "1.0.0"              ; 版本号
!define PUBLISHER     "Your Company"       ; 发布者
!define MAIN_EXE      "MyApp.exe"          ; 主程序文件名
```

### 4. 编译 NSIS 安装包

```bash
makensis QtStyle_Installer.nsi
```

或右键点击 `.nsi` 文件 → **Compile NSIS Script**

生成的 `MyApp_Setup.exe` 即为最终安装程序。

## 🎨 工作原理

```
用户双击 MyApp_Setup.exe
         ↓
NSIS 解压 QtInstaller.exe 到临时目录
         ↓
运行 QtInstaller.exe --mode=setup
         ↓
Qt 向导显示美观界面（欢迎页、许可协议、路径选择、组件选择）
         ↓
用户点击"开始安装"，Qt 向导将配置写入 installer_config.ini
         ↓
NSIS 读取配置文件，执行实际安装（复制文件、创建快捷方式、写注册表）
         ↓
运行 QtInstaller.exe --mode=finish
         ↓
Qt 向导显示完成页，提供"立即运行"选项
         ↓
安装完成！
```

## ✨ 特性

### Qt 安装向导界面
- ✅ 现代化 Windows 11 风格
- ✅ 蓝色渐变横幅
- ✅ 许可协议页（可自定义）
- ✅ 自定义安装路径
- ✅ 组件选择（桌面快捷方式、开始菜单）
- ✅ 完成页带"立即运行"选项

### NSIS 后端
- ✅ 静默模式（不显示 NSIS 默认界面）
- ✅ 完整的注册表集成
- ✅ 卸载程序
- ✅ 版本信息（右键属性可见）

## 🔧 自定义

### 修改 Qt 界面样式

编辑 `qt_installer_wizard.cpp` 中的 `setStyleSheet()` 部分：

```cpp
setStyleSheet(R"(
QPushButton {
    background: #ff6b6b;  // 改为红色按钮
    ...
}
)");
```

### 添加自定义页面

1. 在 `qt_installer_wizard.h` 中声明新页面类
2. 在 `qt_installer_wizard.cpp` 中实现
3. 在 `InstallerWizard` 构造函数中添加：
   ```cpp
   addPage(new MyCustomPage(this));
   ```

### 修改许可协议

编辑 `LicensePage::LicensePage()` 中的 HTML 内容：

```cpp
m_licenseText->setHtml(R"(
<h3>您的许可协议标题</h3>
<p>协议内容...</p>
)");
```

## 📦 打包清单

最终安装包需要包含：

```
MyApp_Setup.exe (NSIS 编译生成)
  ├── 内嵌 QtInstaller/ 目录（Qt 向导程序）
  │   ├── QtInstaller.exe
  │   ├── Qt6Core.dll
  │   ├── Qt6Gui.dll
  │   ├── Qt6Widgets.dll
  │   └── platforms/qwindows.dll
  └── 内嵌 AppFiles/ 目录（你的应用）
      ├── MyApp.exe
      └── ... Qt 依赖
```

## ⚠️ 注意事项

1. **Qt 版本兼容性**：确保 `QtInstaller.exe` 和 `MyApp.exe` 使用相同的 Qt 版本
2. **插件目录**：`platforms/qwindows.dll` 是必需的，否则 Qt 程序无法启动
3. **管理员权限**：安装到 `Program Files` 需要管理员权限（`RequestExecutionLevel admin`）
4. **临时文件清理**：NSIS 脚本会自动清理临时解压的 Qt 向导文件

## 🐛 故障排除

### Qt 向导无法启动
- 检查 `platforms/qwindows.dll` 是否存在
- 运行 `windeployqt QtInstaller.exe` 确保依赖完整

### 安装后应用无法运行
- 检查 `AppFiles/` 中的 Qt DLLs 是否完整
- 使用 [Dependency Walker](https://www.dependencywalker.com/) 检查缺失的 DLL

### NSIS 编译错误
- 确保 `QtInstaller/` 和 `AppFiles/` 目录存在
- 检查路径中是否有中文或特殊字符

## 📚 参考资料

- [NSIS 官方文档](https://nsis.sourceforge.io/Docs/)
- [Qt Installer Framework](https://doc.qt.io/qtinstallerframework/)
- [windeployqt 工具](https://doc.qt.io/qt-6/windows-deployment.html)

## 📄 许可证

本示例代码采用 MIT 许可证，可自由修改和商用。

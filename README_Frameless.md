# Qt 无边框安装向导 + Windows 深色模式支持

## ✨ 新增特性

### 1. 无边框窗口设计
- ✅ 完全自定义标题栏
- ✅ 拖动窗口支持
- ✅ 最小化/关闭按钮
- ✅ 圆角窗口 + 阴影效果
- ✅ 现代化 Windows 11 风格

### 2. Windows 深色模式自动检测
- ✅ 自动读取系统主题设置
- ✅ 深色/浅色模式自适应
- ✅ 所有控件样式统一
- ✅ 完美支持 Windows 10/11

### 3. 视觉增强
- ✅ 渐变色横幅（蓝色/绿色）
- ✅ 圆角边框
- ✅ 阴影效果
- ✅ Emoji 图标点缀
- ✅ 平滑动画过渡

## 🎨 界面预览

### 浅色模式
```
┌─────────────────────────────────────────────┐
│ MyQtApplication 安装向导          − × │  ← 自定义标题栏
├─────────────────────────────────────────────┤
│                                             │
│   欢迎安装 MyQtApplication                  │  ← 蓝色渐变横幅
│   版本 1.0.0 · 现代化安装体验               │
│                                             │
├─────────────────────────────────────────────┤
│                                             │
│   本向导将引导您完成安装...                 │
│                                             │
│                                             │
│                                             │
│                    [下一步 >]               │
└─────────────────────────────────────────────┘
```

### 深色模式
```
┌─────────────────────────────────────────────┐
│ MyQtApplication 安装向导          − × │  ← 深色标题栏
├─────────────────────────────────────────────┤
│                                             │
│   欢迎安装 MyQtApplication                  │  ← 蓝色渐变横幅
│   版本 1.0.0 · 现代化安装体验               │
│                                             │
├─────────────────────────────────────────────┤
│  [深色背景 #2b2b2b]                         │
│   浅色文字 #e0e0e0                          │
│                                             │
│   🖥️  创建桌面快捷方式                      │
│   📁  创建开始菜单快捷方式                  │
│                                             │
│                    [开始安装]               │
└─────────────────────────────────────────────┘
```

## 🔧 技术实现

### 无边框窗口
```cpp
// 设置窗口标志
setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
setAttribute(Qt::WA_TranslucentBackground);

// 添加阴影
QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
shadow->setBlurRadius(20);
shadow->setColor(QColor(0, 0, 0, 80));
shadow->setOffset(0, 4);
setGraphicsEffect(shadow);
```

### 深色模式检测
```cpp
bool isWindowsDarkMode()
{
#ifdef Q_OS_WIN
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat
    );
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;
#else
    return false;
#endif
}
```

### 自定义标题栏拖动
```cpp
void CustomTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPosition().toPoint() - m_dragPos);
    }
}
```

## 📦 编译说明

### 依赖库
```pro
QT += core gui widgets

# Windows 深色模式检测需要链接 dwmapi.lib
win32 {
    LIBS += -ldwmapi
}
```

### 编译步骤
```bash
cd QtInstaller
qmake QtInstaller.pro
make release

# 部署 Qt 依赖
windeployqt --release release/QtInstaller.exe
```

## 🎯 使用方法

### 1. 测试深色模式
在 Windows 设置中切换主题：
```
设置 → 个性化 → 颜色 → 选择模式
- 浅色：应用程序使用浅色背景
- 深色：应用程序自动切换深色主题
```

### 2. 运行安装向导
```bash
QtInstaller.exe --mode=setup --config="config.ini"
```

### 3. 显示完成页
```bash
QtInstaller.exe --mode=finish --apppath="C:\Program Files\MyApp\MyApp.exe"
```

## 🎨 自定义样式

### 修改主题色
编辑 `applyDarkModeIfNeeded()` 函数：

```cpp
// 将蓝色改为紫色
QPushButton {
    background: #7c3aed;  // 紫色
    ...
}
QPushButton:hover { background: #8b5cf6; }
```

### 修改横幅渐变
```cpp
banner->setStyleSheet(R"(
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #ff6b6b, stop:1 #feca57);  // 红到黄渐变
)");
```

### 添加动画效果
```cpp
QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry");
anim->setDuration(300);
anim->setStartValue(QRect(0, -100, width(), height()));
anim->setEndValue(QRect(0, 0, width(), height()));
anim->setEasingCurve(QEasingCurve::OutCubic);
anim->start(QAbstractAnimation::DeleteWhenStopped);
```

## 🐛 已知问题

### 1. 窗口拖动到屏幕边缘
**现象**：拖动窗口到屏幕边缘时可能触发 Windows 贴靠功能
**解决**：已禁用，如需启用可移除 `Qt::FramelessWindowHint`

### 2. 高 DPI 缩放
**现象**：在高 DPI 显示器上可能显示模糊
**解决**：在 `main.cpp` 中添加：
```cpp
QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
```

### 3. 深色模式切换
**现象**：运行时切换系统主题不会立即生效
**解决**：需要重启安装程序，或实现主题监听：
```cpp
// 监听注册表变化（高级功能）
QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
// 监听注册表文件变化...
```

## 📊 性能优化

### 1. 阴影效果
阴影使用 `QGraphicsDropShadowEffect`，可能影响性能。如需优化：
```cpp
// 方案 1：减小模糊半径
shadow->setBlurRadius(10);  // 从 20 降到 10

// 方案 2：使用图片阴影（更快）
// 预先生成带阴影的背景图片
```

### 2. 样式表缓存
```cpp
// 避免重复设置样式表
static QString cachedStyleSheet;
if (cachedStyleSheet.isEmpty()) {
    cachedStyleSheet = generateStyleSheet();
}
setStyleSheet(cachedStyleSheet);
```

## 🔐 安全注意事项

1. **注册表读取**：仅读取主题设置，不修改系统配置
2. **权限要求**：不需要管理员权限（除非安装到 Program Files）
3. **数据隐私**：不收集用户数据，配置文件仅存储安装路径

## 📚 参考资料

- [Qt Frameless Window](https://doc.qt.io/qt-6/qwidget.html#windowFlags-prop)
- [Windows Dark Mode API](https://docs.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes)
- [QGraphicsEffect](https://doc.qt.io/qt-6/qgraphicseffect.html)

## 📄 更新日志

### v1.1.0 (2026-03-XX)
- ✅ 添加无边框窗口支持
- ✅ 实现 Windows 深色模式自动检测
- ✅ 优化视觉效果（圆角、阴影、渐变）
- ✅ 添加自定义标题栏拖动
- ✅ 改进按钮悬停效果

### v1.0.0 (2026-03-XX)
- 初始版本
- 基础安装向导功能

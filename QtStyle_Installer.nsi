; ============================================================================
; 使用 Qt 自定义界面的 NSIS 安装脚本
; ============================================================================
; 原理：
;   1. NSIS 启动时先解压 Qt 安装向导（QtInstaller.exe）、7za.exe 和 app.7z
;      到临时目录
;   2. Qt 向导显示安装页面，用户选择路径和组件
;   3. Qt 向导调用 7za.exe 解压 app.7z 到目标目录（进度条实时显示）
;   4. Qt 向导将配置写入 INI 文件
;   5. NSIS 读取 INI 文件，创建快捷方式和注册表项
;   6. NSIS 再次调用 Qt 向导显示完成页
; ============================================================================
!include "LogicLib.nsh"

Unicode True
SetCompressor /SOLID lzma

; ── 应用信息 ────────────────────────────────────────────────────────────
!define APP_NAME      "RoboshopPro"
!define APP_VERSION   "2.4.2.38"
!define PUBLISHER     "Seer Group."
!define MAIN_EXE      "RoboshopPro.exe"
!define QT_WIZARD     "QtInstaller.exe"    ; Qt 安装向导程序
!define CONFIG_FILE   "$TEMP\installer_config.ini"
!define ARCHIVE_FILE  "RoboshopPro_x64.7z"             ; 打包的应用程序压缩包
!define BONJOUR_FILE  "InstallBonjour.7z"             ; bonjour 打包的应用程序压缩包
!define LICENSE_FILE  "SCLicence.txt"                 ; 最终用户许可协议

Name    "${APP_NAME} ${APP_VERSION}"
OutFile "RoboshopPro-${APP_VERSION}_Setup.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
RequestExecutionLevel admin

; ── 版本信息 ────────────────────────────────────────────────────────────
VIProductVersion "${APP_VERSION}"
VIAddVersionKey "ProductName"     "${APP_NAME}"
VIAddVersionKey "ProductVersion"  "${APP_VERSION}"
VIAddVersionKey "CompanyName"     "${PUBLISHER}"
VIAddVersionKey "LegalCopyright"  "${PUBLISHER}"
VIAddVersionKey "FileDescription" "${APP_NAME} 安装程序"
VIAddVersionKey "FileVersion"     "${APP_VERSION}"

; ── 页面配置（隐藏默认 NSIS 界面）────────────────────────────────────
SilentInstall silent    ; 静默模式，不显示 NSIS 默认界面
ShowInstDetails nevershow

; ── 安装文件 ────────────────────────────────────────────────────────────
Section "MainSection" SEC01
  ; === 步骤 1：解压 Qt 安装向导（含 7za.exe）和压缩包到临时目录 ===
  SetOutPath "$TEMP\QtInstaller"
  File /r "QtInstaller\*.*"   ; 包含 QtInstaller.exe、Qt DLLs 以及 7za.exe
  File     "${ARCHIVE_FILE}"      ; 应用程序 .7z 压缩包
  File     "${BONJOUR_FILE}"
  File     "${LICENSE_FILE}"
  ; === 步骤 2：运行 Qt 安装向导（向导内含解压进度条）===
  ; Qt 向导会：
  ;   - 显示欢迎/许可/路径/组件页面
  ;   - 调用 7za.exe 解压 app.7z 到用户选定的目录
  ;   - 将配置写入 CONFIG_FILE
  DetailPrint "正在启动安装向导..."
  ExecWait '"$TEMP\QtInstaller\${QT_WIZARD}" --mode=setup --config="${CONFIG_FILE}" --archive="$TEMP\QtInstaller\${ARCHIVE_FILE}"' $0

  ; 检查用户是否取消
  ${If} $0 != 0
    DetailPrint "用户取消安装"
    RMDir /r "$TEMP\QtInstaller"
    Abort
  ${EndIf}

  ; === 步骤 3：读取 Qt 向导生成的配置文件 ===
  ; Qt 使用 QSettings IniFormat，[Setup] 组，整数布尔值（1/0）
  ReadINIStr $INSTDIR "${CONFIG_FILE}"  "Setup" "InstallPath"
  ReadINIStr $1       "${CONFIG_FILE}"  "Setup" "CreateDesktopShortcut"
  ReadINIStr $2       "${CONFIG_FILE}"  "Setup" "CreateStartMenu"

  ; === 步骤 4：创建快捷方式（文件已由 Qt 向导解压）===
  DetailPrint "正在创建快捷方式..."

  ${If} $1 == "1"
    CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${MAIN_EXE}"
  ${EndIf}

  ${If} $2 == "1"
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${MAIN_EXE}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\卸载.lnk" "$INSTDIR\Uninstall.exe"
  ${EndIf}

  ; 写入卸载程序（从临时目录复制到安装目录）
  CopyFiles "$TEMP\QtInstaller\${QT_WIZARD}" "$INSTDIR\${QT_WIZARD}"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; 注册表
  WriteRegStr HKCU "Software\${APP_NAME}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
              "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
              "DisplayVersion" "${APP_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
              "Publisher" "${PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
              "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
              "DisplayIcon" "$INSTDIR\${MAIN_EXE}"

  ; === 步骤 5：显示 Qt 完成页 ===
  ExecWait '"$TEMP\QtInstaller\${QT_WIZARD}" --mode=finish --apppath="$INSTDIR\${MAIN_EXE}"'

  ; 清理临时文件
  Delete "${CONFIG_FILE}"
  RMDir /r "$TEMP\QtInstaller"
SectionEnd

; ── 卸载 ────────────────────────────────────────────────────────────────
Section "Uninstall"
  RMDir /r "$INSTDIR"
  Delete "$DESKTOP\${APP_NAME}.lnk"
  RMDir /r "$SMPROGRAMS\${APP_NAME}"

  DeleteRegKey HKCU "Software\${APP_NAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd

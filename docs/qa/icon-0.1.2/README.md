# 0.1.2 简化图标验收 — 2026-10-18

用户已在本机 Dock 中查看独立预览 App，并明确同意采用。正式项目、1024×1024 主图与 Dock 预览所用像素文件逐字节一致。

- Xcode 27 / CMake / Ninja 构建成功。
- 正式 App 的 CFBundleShortVersionString 为 0.1.2，图标名为 AppIcon。
- AppIcon.icns 和 Assets.car 编译成功；App 包内 .icon 图像与工程选定主图一致。
- ad-hoc 签名通过 codesign --verify --deep --strict。
- 本机音频工作流自测通过；没有新增或更改音频/UI 业务实现。
- 32/64/128 像素 ICNS 预览保存在 small-sizes.png。
- 已通过 CUA 保存退出旧正式版、关闭临时图标预览 App，然后启动正式 Traka；主窗口正常显示。

临时预览应用仅位于忽略的 output/ 目录，不进入源码发布。此更新只更换图标；不增加权限或网络能力。没有重新测试旧版 macOS 或实体麦克风硬件。

更新：用户反馈正式 Dock 仍显示旧图标。已增加 Source/DockIcon.mm，在启动和启动后 500ms 直接读取当前 AppIcon.icns 设置 NSApplication.applicationIconImage 并刷新自身 Dock tile，不重启系统 Dock、不改全局偏好。已正常退出旧进程、刷新当前应用注册并启动新进程。此修复属于尚未发布的 0.1.2。

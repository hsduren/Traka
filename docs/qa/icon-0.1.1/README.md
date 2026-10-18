# 0.1.1 图标验收 — 2026-10-18

用户选定的完全正面银色调音台图标已接入，原图保留，1024×1024 RGB 主图无预烘焙外轮廓、无额外圆角卡片。AppIcon.icon 包含唯一选定图像层；CMake 目标 Resources 和图标名称均指向 AppIcon。

- Xcode 27 编译工具链 / actool + 项目 CMake/Ninja 构建成功。
- App 包内 AppIcon.icns、Assets.car、CFBundleIconName 和 CFBundleIconFile 均验证通过。
- 原图、工程资产和 App 包内原始图像一致，没有磁带/波形草稿残留。
- ad-hoc 签名通过 codesign --verify --deep --strict。
- 本机音频工作流自测通过；界面控件自测 13 项通过，含许可页分页后的源码入口检查。
- 已通过 CUA 实际启动主窗口。Finder 操作因前台窗口切换而停止，不声称已完成真实 Dock/Finder 截图验收。
- system-icon.png 为 NSWorkspace 针对最终 App 包返回的系统图标渲染（非桌面截图）。已目视检查：满版正面面板，单一系统圆角，无双重外框。
- compiled-sizes.png 为最终 ICNS 的 16/32/64/128/256 像素预览，已目视检查；源目录另有 512/1024 像素资源。

本次在用户明确授权下使用主机，未运行 VM。macOS 15 及更早系统只验证了编译所得 ICNS 回退文件，未在旧系统真机运行。真实麦克风及硬件延迟不在本次图标验证范围内。

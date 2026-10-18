# Traka — P0

[公开源码](https://github.com/hsduren/Traka) · [v0.1.1 源码发布](https://github.com/hsduren/Traka/releases/tag/v0.1.1)

一个基于 JUCE + Tracktion Engine 的本地 macOS 音频工作站。首版只验证：**导入 → 播放 / 录音 → 简单剪辑与混音 → 保存 → 导出**。

## 构建与运行

需要 macOS 13+、Xcode 26 或更新版本（图标编译需要 actool）、CMake、Ninja。可以先安装 `brew install cmake ninja`。当前已在 Apple Silicon 开发；Intel 与 universal 构建尚未验证。

```sh
git clone https://github.com/hsduren/Traka.git
cd Traka
git submodule update --init --depth 1
git -C vendor/tracktion_engine config submodule.modules/juce.url https://github.com/juce-framework/JUCE.git
git -C vendor/tracktion_engine submodule update --init --depth 1
./scripts/build.sh
./scripts/test-vm.sh macos2
```

**默认验收在 macOS 2 VM；用户已明确授权本次 0.1.1 图标更新在主机运行和测试。**测试脚本将构建复制到 VM 的 `~/Applications/Traka-P0-QA/Traka.app`，报告和截图保存到本项目 `docs/qa`。

自测 VM 需要先配置可 SSH 连接的主机别名；`macos2` 是维护者使用的别名，可替换为自己的测试 VM。普通用户可在自己的 Mac 构建并手动运行 `build/Traka_artefacts/Debug/Traka.app`。

也可以生成 Xcode 工程：`cmake -S . -B build-xcode -G Xcode`。

## 使用

1. 点击“示例”体验原创合成的三轨片段，或点击“导入音频” / 拖入音频。非空轨道再次导入会新增一轨。
2. 点击“播放”；空格切换播放/暂停。点击时间标尺定位。
3. 点击片段选中，拖动移动；拖片段左右边缘裁剪。可拖到另一轨。播放头放在片段内部后，点击“分割”。Delete 删除片段，⌘Z 撤销，⇧⌘Z 重做。
4. 点击音轨左侧选择轨道；底部调整音量、声像、轨道名称（回车确认）。M 静音，S 独奏。
5. 录音：先在“音频设置”选择输入设备并勾选输入通道，再在底部选择输入，选定目标音轨，点击“录音”。macOS 首次会询问麦克风权限。“停止”或“完成录音”保留录音。输入监听关闭，建议使用耳机。
6. “保存” / ⌘S 创建 `.traka` 项目及同名 `.traka.media` 音频文件夹。**移动或分享项目时，两者一起移动。**新项目草稿在 `~/Library/Application Support/Traka/Sessions`。应用每 20 秒自动保存，切换项目和退出也会保存。
7. “导出 WAV”输出完整时间线的立体声、48 kHz、24-bit WAV，使用当前静音、独奏、音量与声像设置。输出先写入临时文件，成功后替换目标。

语言选择支持系统默认、英语与简体中文，并在本机持久化。JUCE 原生音频设置文字遵循框架本身的语言。

## P0 边界

无 MIDI、钢琴卷帘、虚拟乐器、插件扫描/托管、音效、变速变调、自动化、云端、账户、网络服务、商店发布或签名公证。循环只支持全曲，不包含自定义循环区间。暂无删除音轨、复杂选区、多片段操作、输出电平表、削波提示与混音归一化。录音只录一条选定轨道，不做多输入同时录制。

这是本地开发 MVP。自动测试验证文件、剪辑和渲染；VM 当前没有输入通道；实际麦克风录音、硬件延迟和不同音频接口尚未验证。示例为本项目生成的音频，不使用 upstream 示例媒体。

## 开源

Traka 自有代码使用 AGPL-3.0-or-later。Tracktion Engine 使用 GPL-3.0-or-later，JUCE 使用 AGPLv3 的开源路径，保留它们的原始声明。依赖以 Git submodule 固定提交；见 [UPSTREAM.md](UPSTREAM.md) 与 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。完整源码按应用版本打标签，并在 GitHub Release 附带包含依赖的源码包；应用底部“开源与许可”显示源码地址、许可权利、无担保声明和完整许可文本。后续更新流程见 [RELEASE.md](RELEASE.md)。当前只发布源码，不提交商店。

## 使用完整源码包

GitHub 默认自动生成的 ZIP/TAR 不包含 Git 子模块。下载 Release 的 `Traka-<version>-source.tar.gz` 已包含 JUCE 和 Tracktion Engine，无需运行子模块命令。解压进入目录后直接运行 `./scripts/build.sh`。源码包内 `SOURCE_MANIFEST.json` 记录应用和依赖的精确提交。

本项目是自由软件，允许按适用许可使用、修改和再分发，不提供任何担保。

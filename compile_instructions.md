# 里程碑 1 & 2：编译 GTK 原生应用指南

为了将我们刚才写的 `main.cpp` 编译成能够在你的 Kindle (ARM 架构) 上运行的二进制文件，我们需要**交叉编译 (Cross-Compilation)** 工具链。

由于你目前使用的是纯 Windows 操作系统，且尚未安装 WSL (Linux 子系统) 或 Docker，我们有以下两种最高效的解决方案来完成编译：

### 方案 A：使用 GitHub Actions 云端编译（推荐，最省事）
这招是“无中生有”：我们直接借用微软的免费服务器来帮你编译。
1. 在你的 GitHub 上新建一个空仓库（例如 `kindlestats-app`）。
2. 把 `main.cpp` 传上去。
3. 编写一个 `.github/workflows/build.yml` 文件，让 GitHub 在后台自动装好 Ubuntu 和 `arm-linux-gnueabi` 编译链，编译完直接把 `hello_kindle` 文件吐给你下载。
如果你选这个，我待会直接把 YAML 配置文件写给你！

### 方案 B：本地安装 WSL 编译（硬核，适合长线开发）
这招适合你打算以后天天调试这套 C++ 代码。
1. 你打开 PowerShell（管理员），运行：`wsl --install`
2. 重启电脑后，进入 Ubuntu，运行：
   `sudo apt update`
   `sudo apt install g++-arm-linux-gnueabi libgtk2.0-dev:armel`
3. 随后，我们在 WSL 里用一行命令就能把它编译出来：
   `arm-linux-gnueabi-g++ main.cpp -o hello_kindle $(pkg-config --cflags --libs gtk+-2.0)`

### 编译完成后（里程碑 2 达成）
只要我们拿到了那个编译好的 `hello_kindle` 文件：
1. 把它拷贝到 `E:\extensions\kindlestats\bin\` 目录下。
2. 我们在 `menu.json` 加上启动按钮。
3. 你在 Kindle 屏幕上点击菜单，就会瞬间弹出一个纯白的全屏界面，显示“Hello Kindle”，并且你摸一下屏幕的任何地方，它就会丝滑地退出！

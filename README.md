# i.MX6ULL 应用工程

包含 LED、蜂鸣器控制接口和从虚拟机获取时间的客户端。
当前 main 只执行一次 GetTime("192.168.87.128", 9000, 5000)，成功后更新开发板系统时间。

## 编译

在 Ubuntu 工程目录运行 `make`。SDK 路径配置在 makefile 的 SDK_ENV 中。
可执行文件为根目录的 main，中间文件位于 output/；它们不纳入 Git。
目标为 ARM i.MX6ULL，不能直接在 x86 Ubuntu 上运行。

## 时间服务器

配套服务器位于虚拟机 /home/hutz/application/code/time-server，属于独立工程，未包含在本仓库。
先启动该服务器，再在开发板以 root 运行 main。当前实现不写入硬件 RTC。

## Git 与 Windows 备份

修改后先执行 git diff，选定文件 git add，再 git commit。
Windows 备份仓库：D:\Imx6ull\backup\git\imx6ull.git。
在 Windows PowerShell 运行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File "D:\Imx6ull\backup\git\Backup-imx6ull.ps1"
```

Windows 主动通过现有 SSH 拉取已提交的分支和标签；未提交修改不会备份。
详细恢复方法见 Windows 同目录 README.md。

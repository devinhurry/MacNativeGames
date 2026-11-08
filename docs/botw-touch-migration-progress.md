> 架构已于 2026-11-07 调整：核心现位于独立 `~/projects/touchnx`，通过适配器接入 nxnative 和本仓库。本文保留早期迁移记录；当前结构与验证见 TouchNX 的 README 和 `docs/validation.md`。本文未列出的后续迁入内容，不能据此判断为尚未实现。

# BOTW 触控迁移进度

执行日期：2026-11-07。分支：`feat/botw-touch-migration`。

本文件记录实际完成程度，任务范围仍以 [执行清单](botw-touch-launcher-migration.md) 为准。

## 已迁入并验证的基础

- 触控核心：布局、按钮外观、摇杆曲线、方向命中、触控板手势和竖屏几何。三项原有核心检查在新目录独立编译运行。
- 游戏回调基础：BOTW 1.9.0 的 17 个原有观察位置、原指令和相关对象/调用位置检查。观察回调有独立类型，支持只读、显式修改和只改返回值三种原有接口。
- 寄存器和调用栈：沿用本项目原有的宿主栈和线程实现，补上旧触控需要的保存栈指针。真实 ARM64 检查覆盖两条线程、嵌套观察、SVC、宿主导入、浮点状态和返回值。
- 加载表：保留 NSO 原始构建号，读取时检查长度和内容；旧表的保留字段为零时仍可读取。错误回调类型、越界编号和损坏构建号有失败检查。
- 补丁冲突登记：登记全部 17 个位置，包括含相机参数的两处编号表达式。

菜单、动作、镜头、HUD 和画面区域的五份原实现及其测试也已迁入，原行为代码未重写。当前它们仍以独立检查运行，尚未作为应用触控前端接入；iOS 工程暂不编译这部分目录。

## 来源与行为差异

来源固定为 `8188028655e625120e5291aa7d346532bb9238a9`。逐文件来源和 SHA-256 见
[`ORIGIN.tsv`](../games/totk/runtime/botw_touch/ORIGIN.tsv)。完整复制文件共 24 份，其中 23 份逐字一致。

唯一的完整文件改动是菜单测试：原测试从未保证对齐的字节数组中直接读取 `float`，UBSan 会中止；现改为 `memcpy` 读取同样的四个坐标，断言内容不变。其余接口头和观察回调注册器是按需摘取和适配。

没有迁入旧帧生成、低内存、渲染调度、缓存或旧线程实现。重新生成的 main 包含 17 个观察点，旧性能 CUTS 为 0；应用继续编译并使用目标已有的性能实现。

## 验证记录

所有游戏数据、临时环境和运行日志均在仓库外：
`/Volumes/ExternalSSD/Games/ZeldaWild/macnativegames-touch-migration/`。

| 验证 | 实际结果 |
| --- | --- |
| 迁移前 `make check` | 1,537 项通过，169 项按环境跳过；后续 C 检查通过 |
| 迁移后 `make check` | 1,542 项通过，169 项按环境跳过；后续 C 检查通过 |
| 原有触控核心、菜单、动作、HUD、镜头、画面区域检查 | 全部通过，含 UBSan |
| 新的真实 ARM64 回调/加载表检查 | 通过 |
| BOTW 1.9.0 Mac 原生程序编译 | 通过：`make -C games/totk -j4 build/totk-native-botw190` |
| 新增回调 C 文件及门汇编的 iOS ARM64 编译 | 通过；这不是完整 IPA 编译 |
| 玩家原始 ExeFS | 五个文件全部匹配目标已有校验值 |
| 三个模块重新改写并检查 | 38,110 个结构位置、37,915 个指令等价位置、116,055 次尝试通过 |
| 指令错误检测 | 600 个故意修改的样例全部被检出 |
| 现有原生调用准备机制 | 536 个差分用例、四个错误样例通过；四线程 20,000 次真实调用通过 |
| 新原生程序加载重新生成的三个模块 | 四线程 4,000 次 main → SDK → 宿主计时调用通过；没有运行游戏入口或构造器 |

新增观察点会改变 main 改写产物，因此在原始游戏校验保持不变的前提下，完成上述结构、指令和错误样例检查后，将 `expected-v1.9.0.json` 的 **main 改写产物**摘要更新为：

```
26823ec24bbe7bb8d601e9f0535575939263756da6ee761ef84b1ccdb390ebcf
```

这不是跳过校验：原始 ExeFS、SDK/subsdk0、RomFS、DLC、字体的校验值均未改变。详细模块检查报告位于日志目录的
`validation/totk/analysis/botw190-port/phase12-validation.json`。

复查底层检查：

```sh
python3.14 -m unittest discover -s tests -p test_botw_touch_core.py -v
python3.14 -m unittest discover -s tests -p test_botw_observation.py -v
python3.14 -m unittest discover -s tests -p test_patch_sites.py -v
```

完整检查使用 Python 3.14，并安装 `games/totk/requirements.txt`；指令等价验证另需 `requirements-dev.txt` 和 LLVM。原有验证脚本的 LLVM 默认目录未在本机安装，本次显式使用已安装的 `LLVM_BIN=/opt/homebrew/opt/llvm/bin`。

## 仍需接通

1. 把真实模块地址、内存范围、游戏帧和分开的实体/触控输入接到原有菜单、动作与镜头代码。
2. 接通 Mac 鼠标预览、iOS 手势和生命周期；随后做游戏中的原生菜单和镜头验证。
3. 完成 HUD、符文图标、滤镜/LUT、竖屏及 Duo/Seated 的绘制接入。
4. 迁移共享设置、布局编辑与旧布局导入；迁移存档库、旧存档导入和新版云同步。
5. 接通两端启动器，完成 IPA、图形包、含 DLC 的 RomFS 包三个独立产物及导入。
6. 完整应用构建、Mac 游戏验收和 iPhone/iPad 真机操作验证。

当前没有生成可交付的迁移版 IPA，也没有修改或提交旧 nxnative 工作区。

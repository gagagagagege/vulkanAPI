# Vulkan API 原型

从零手写的 Vulkan 渲染管线，不依赖任何教程框架，Vulkan 资源全部用 `vk::raii` 句柄管理。
这一步的产出（`Buffer` / `Image` 等资源抽象）后续会并入
[Fish](https://github.com/gagagagagege/Fish) 引擎，作为它的 Vulkan 后端。

## 目前跑通了什么

一个带贴图的旋转四边形，完整链路：

- Vulkan 1.3 实例 / 物理设备选择 / 逻辑设备与队列创建
- 校验层 + 调试回调
- Swapchain 创建与窗口尺寸变化时的重建
- **动态渲染**（`dynamicRendering`），不使用 render pass / framebuffer
- 着色器用 **Slang** 编写，编译为 SPIR-V
- 顶点 / 索引缓冲，经 staging buffer 上传到 device local 内存
- 纹理上传（图像布局转换 → 拷贝 → 采样器 → 描述符集）
- UBO 每帧更新，**每个 frame-in-flight 独占一块缓冲**，避免 CPU 写入时 GPU 仍在读取
- 帧同步：信号量 + fence

## 环境要求

| 依赖 | 说明 |
|---|---|
| **Vulkan SDK 1.3+** | 提供头文件、loader；`stb_image.h` 取自 SDK 的 `third_party/` |
| GLFW 3.5 | 随仓库提供 |
| glm | 随仓库提供（已剔除文档与测试目录） |
| CMake 3.20+ | |
| C++20 编译器 | 目前仅在 MSVC 上验证过 |

> GPU 需支持 Vulkan 1.3，且具备 `dynamicRendering` / `synchronization2` /
> `extendedDynamicState` / `samplerAnisotropy`。程序启动时会校验这些特性，不满足则直接退出。

## 构建与运行

```bash
cmake -B build -S .
cmake --build build --config Debug
```

Vulkan SDK 不在默认路径时：

```bash
cmake -B build -S . -DVulkan_ROOT=/path/to/VulkanSDK
```

运行 —— **工作目录必须是可执行文件所在目录**，程序按相对路径读取 `shaders/` 和 `textures/`，
CMake 已配置在构建后自动拷贝：

```bash
cd build/Debug && ./VulkanProject.exe    # Windows
./build/VulkanProject                    # Linux / macOS
```

## 目录结构

```
src/
├── vkContext.h/.cpp         队列族选择、物理设备打分、逻辑设备与队列
├── Buffer.h/.cpp            GPU 缓冲封装（顶点 / 索引 / UBO / staging），含 map/unmap
├── Image.h/.cpp             图像 + view + sampler 的持有者
├── SwapChain.h/.cpp         交换链 + image + view + present 信号量；重建的边界就是它
├── Pipeline.h/.cpp          图形管线与它的 pipelineLayout（含顶点输入布局）
├── CommandPool.h/.cpp       命令池；另外放命令缓冲录制与一次性提交命令
├── frameData.h/.cpp         FrameData（每帧一份的资源）与 Frames（N 份 + 当前下标）
├── DescriptorAllocator.h/.cpp  描述符 layout + pool，以及 UBO 每帧的 MVP 写入
├── texture.h/.cpp           纹理加载、图像布局转换、采样器
├── shader.h/.cpp            SPIR-V 加载
├── ValidationLayers.h/.cpp  校验层与调试回调
├── surface.h/.cpp           Win32 窗口表面
└── TriangleApp.h/.cpp       应用主体、初始化流程与主循环
```

## 几个设计要点

- **`Buffer` 自持句柄与内存。** 构造时分配 `VkBuffer` + `VkDeviceMemory` 并绑定，
  析构时自动释放。内存声明在缓冲之前，保证析构顺序正确。
- **`Buffer` 只可移动，不可拷贝。** 提供默认构造作为「空状态」，便于「先声明、后初始化」的成员写法。
- **构造函数接收 `vkContext*`** 而非一堆裸句柄 —— 设备、物理设备、队列都从它取，
  调用点因此不需要反复透传。
- **`map()` 缓存映射指针**，重复调用返回同一地址，调用方无需自己保存。
- **`uniformBuffer` 全部是静态函数**，不持有状态，只做 UBO 与描述符的搭建。

## 已知问题

- `renderFinishedSemaphores` 按 frame-in-flight 分配，在 present 尚未完成时可能被重新 signal。
  校验层会给出相应警告。当前绘制负载极小，无实际影响；
  计划在做批量渲染时改为每个 swapchain image 配一个信号量。
- `vkContext` 的名字与实际职责略有出入（还包含队列与物理设备），
  并入 Fish 时会重新组织。

## 说明

早期原型的 `Buffer` 抽象是在一个独立沙盒里迭代出来的，所以这个仓库的
`BUFFER_REFACTOR_TODO.md` 记录了那一轮重构的设计取舍与踩过的坑，供后续并入引擎时参考。

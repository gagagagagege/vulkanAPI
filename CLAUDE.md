# 协作须知

> 建立于 2026-09-17,总结自 Image / FrameData 两次重构。
> 本文件每次会话开始时自动加载。

## 一、怎么跟我说话

### 先给事实,不要方案

问"现在长什么样",答**调用点清单 + 行号 + 现状**。不要给"方案 A / 方案 B / 取舍矩阵"
—— 取舍是你要自己判断的部分(见第二节)。

### 我复述理解时,请直接挑错

这一轮最有价值的几次互动都是"你这句不成立":

- "presentComplete 每帧开始前检测可用" → **它从没被检测过,也检测不了**
  (`VkSemaphore` 没有查询状态的 API)。它的"可用"是从 fence 的检测结果推出来的。
- "renderFinished 没有检测" → 不是没检测,是**没有任何检测手段**。
  这个区别决定修法只能是"换信息源",不能是"补上检测"。

**含糊就别顺着我往下走。** 顺着走的结果是设计建立在错模型上,而且要到能跑的时候才暴露。

### 注释写短

默认写得比我要的长,我会手动删。**能一段说清就别写两段**,不要复述代码在做什么,
只写"为什么这么写 / 换成别的会怎样"。

## 二、动手之前

### 结构分叉先定死

"FrameData 是 5 个容器还是 5 个单独对象"这种问题问一句能省整轮返工。**有歧义先问,别猜着写。**

### 抽象切在哪层是我自己的事

`PLAN.md` 第九节的约定继续生效:

| | |
|---|---|
| Vulkan API 事实、验证层报错含义、某个 flag 的作用、GL/VK 语义差异 | ✅ 直接答 |
| 抽象切在哪层、接口什么形状、为什么这样取舍、先做哪个 | ❌ 只摆事实和代价,我自己想 |

判据:**答案可查证的就答,是取舍的就我自己想。**

### 超出指令的改动要单独标出来

删死成员、补文件末尾换行这类顺手做的清理,**在交付时单独列出来**,别混在改动里。

## 三、这个仓库的机制

### 构建

```bash
cmake -S . -B build          # 增删 src/ 下的文件后【必须】重跑
cmake --build build --config Debug
```

`CMakeLists.txt` 用的是 `file(GLOB_RECURSE src/*.cpp)`,**没有 `CONFIGURE_DEPENDS`**。
不重跑 configure 会得到"无法打开源文件"这类误导性报错。

### 运行

```bash
cd build/Debug && ./VulkanProject.exe
```

**工作目录必须是 `build/Debug`** —— 程序读 `textures/texture.jpg` 和 `shaders/slang.spv`,
CMake 构建时拷到这里。

### 构建时的已知噪音

`'pwsh.exe' 不是内部或外部命令` —— 某 post-build 步骤调 PowerShell 7,机器上没装。
exe 和资源都正常产出,**不用管,也不要花时间去修**。

### 验证要到什么程度

"没报错"不算验证。至少三条:

1. 构建 0 错误 0 警告
2. 跑十几秒无验证层输出
3. **证明它真在干活** —— 测 CPU 时间(8 秒墙钟烧 14 秒 CPU = 渲染循环全速跑,
   不是卡在 fence 上死等)。`exit=124` 只说明被 timeout 杀了,不说明在渲染

**画面本身我确认不了。** 每次改完渲染路径,要明确说"我没亲眼看过"并让我开窗口。

## 四、文档体系

| 文件 | 用途 |
|---|---|
| `PLAN.md` | 总计划。里程碑、逐周表、协作约定。有变动**直接改这个文件** |
| `BUFFER_REFACTOR_TODO.md` | Buffer 重构的完成记录 + 遗留项。新的重构**照它的格式另起 `XXX_REFACTOR_TODO.md`** |
| `diary.txt` | 我自己的理解 |
| `README.md` | 对外 |

### diary.txt 的规矩(PLAN 第九节)

> 写你自己的理解,**不要贴 AI 对话**。

让我往里写时,我写**事实**(定义、行号、推导链),并明确说"这是事实记录,不是你的理解"。
**推理那部分你要自己重写一遍** —— 写不顺的地方就是没通的地方。

## 五、代码风格

- 注释、提交信息用中文
- 缩进用 tab
- 提交信息:`type: 中文短标题` + 空行 + 详细正文,**正文讲"为什么"**(参考 `7adbcf2`)
- Vulkan 资源一律 `vk::raii`;裸句柄只在确实只依赖单个对象时用
- 类名首字母大写(`Buffer` / `Image` / `FrameData`),函数名小写开头

## 六、`vk::raii` 的坑(本轮新查证,别重新推导)

1. **复数类型是 `std::vector` 的子类,没有析构函数。**
   `vk::raii::CommandBuffers` / `vk::raii::DescriptorSets` 都是。
   "分配 1 个再 `std::move(...front())`"是安全套路 —— 被搬走的元素在 vector 里变成
   空句柄,临时对象析构时跳过它。
2. **`vk::raii::DescriptorSet` 的析构会真的调 `vkFreeDescriptorSets`。**
   `vulkan_raii.hpp` 里 `~DescriptorSet()` → `clear()`,而 `clear()` 里就是
   `vkFreeDescriptorSets(device, pool, 1, &set)`(头文件 :9702 / :9747 / :9751)。
   **所以 set 必须早于它所属的 `descriptorPool` 释放** —— 池先死,这条路就踩空句柄。
3. **`vk::raii::CommandBuffer` 的析构会 `vkFreeCommandBuffers`。**
   所以**命令缓冲必须早于 `commandPool` 释放** —— 持有它的成员(如 `Frames`)
   在 `TriangleApp` 里的声明位置是被约束的。
4. `vk::raii::CommandBuffer` 没有默认构造函数,但**有 `nullptr_t` 构造**,
   所以 `vk::raii::CommandBuffer x = nullptr;` 可以。

> `BUFFER_REFACTOR_TODO.md` 第八节还有 3 条更早的教训(编辑器没保存导致读旧文件、
> 改名时词边界搜索漏掉限定用法、间歇性问题必须多采样),继续有效。

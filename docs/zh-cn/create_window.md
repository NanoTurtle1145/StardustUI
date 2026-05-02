# 创建窗口

这一页说明 StardustUI 当前版本的 `Window` API。

## 头文件

```cpp
#include "../../includes/window.hpp"
#include "../../includes/components/lable.hpp"
```

## 最小示例

```cpp
Window window("Hello, World!", 400, 300);

Lable hello_label("Hello, World!", 24, 0x000000FF);
hello_label.set_pos(100, 100);

window.addComponent(hello_label);
window.show();
```

## 构造函数

```cpp
Window(const char* title, int width, int height);
```

| 参数 | 含义 |
| --- | --- |
| `title` | 窗口标题 |
| `width` | 窗口宽度 |
| `height` | 窗口高度 |

示例：

```cpp
Window window("Demo", 800, 600);
```

## 主要方法

```cpp
void show();
void hide();
int getWidth();
int getHeight();
const char* getTitle();
void error(const char* msg);
```

- `show()` 会创建原生窗口，进入事件循环，并在窗口关闭前持续运行。
- `hide()` 会在句柄存在时关闭当前窗口。
- `getWidth()`、`getHeight()`、`getTitle()` 返回构造时传入的值。
- `error()` 会把错误信息交给当前平台后端输出。

## 添加控件

当前 `Window` 提供两个重载：

```cpp
void addComponent(base_component& component);
void addComponent(base_component* component);
```

这两个接口最终都会在窗口内部保存控件指针，所以控件对象本身必须在窗口事件循环期间一直有效。

安全写法：

```cpp
Window window("Demo", 400, 300);
Lable label("Text", 24, 0x000000FF);

window.addComponent(label);
window.show();
```

不要这样写：

```cpp
Window window("Demo", 400, 300);
window.addComponent(Lable("Text", 24, 0x000000FF)); // 临时对象
```

## 鼠标与悬停

`Window::show()` 已经集成了平台事件循环，并会通过 `handle_message(...)` 更新控件悬停状态。

只要控件实现了 `contains(int x, int y)`，鼠标移动到控件范围内时，对应的 hover 样式就会自动生效。

## 相关文档

- [快速开始](./quickstart.md)
- [样式系统](./style.md)
- [文档目录](./docs.md)

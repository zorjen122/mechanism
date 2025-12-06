# RDC 架构文档 - COM/ActiveX 与 Qt 集成

## 概述

本项目通过 Qt 的 ActiveX 框架（QAxContainer）封装 Windows 的 MsTscAx COM 组件，实现远程桌面连接功能。这份文档详细说明了 COM、ActiveX 和 Qt 之间的集成方式。

---

## 1. COM 基础

### 1.1 什么是 COM？

COM（Component Object Model）是 Microsoft 的组件对象模型，是一种跨语言、跨进程的二进制标准。

**核心概念：**
- **接口（Interface）**：定义对象的行为，以 `I` 开头（如 `IMsRdpClient10`）
- **CLSID**：类标识符，唯一标识一个 COM 类（如 `{7390F3D8-0439-4C05-91E3-CF5CB290C3D0}`）
- **IUnknown**：所有 COM 接口的基接口，提供引用计数和接口查询

### 1.2 MsTscAx COM 组件

MsTscAx 是 Windows 内置的远程桌面客户端 COM 组件。

**关键接口层次：**
```
IMsTscAx (基础接口)
  ├─ IMsRdpClient
  │   ├─ IMsRdpClient2
  │   │   ├─ IMsRdpClient3
  │   │   │   └─ ... (逐代增强)
  │   │   │       └─ IMsRdpClient10 (最新，Windows 10+)
```

**CLSID：** `{7390F3D8-0439-4C05-91E3-CF5CB290C3D0}`

---

## 2. ActiveX 控件

### 2.1 什么是 ActiveX？

ActiveX 是基于 COM 的可视化组件技术，可以嵌入到容器应用程序中（如浏览器、Qt 应用）。

**特点：**
- 继承自 COM，具有 COM 的所有特性
- 提供可视化界面（窗口）
- 支持事件通知（如连接成功、断开等）
- 可以通过属性和方法进行控制

### 2.2 MsTscAx ActiveX 控件

MsTscAx 既是 COM 组件，也是 ActiveX 控件。

**主要功能：**
- 显示远程桌面会话窗口
- 处理用户输入（键盘、鼠标）
- 接收远程桌面图像
- 提供连接配置接口

---

## 3. Qt ActiveX 框架（QAxContainer）

### 3.1 Qt 的 ActiveX 支持

Qt 通过 `QAxContainer` 模块提供 ActiveX 集成，包含两个核心类：

| 类名 | 用途 | 继承关系 |
|------|------|----------|
| `QAxObject` | 封装无界面的 COM 对象 | `QObject` + `QAxBase` |
| `QAxWidget` | 封装有界面的 ActiveX 控件 | `QWidget` + `QAxBase` |

**QAxBase** 是基类，提供 COM 调用的核心功能。

### 3.2 项目配置

在 `.vcxproj` 中添加模块：
```xml
<QtModules>quick;quickcontrols2;axcontainer;widgets</QtModules>
```

**注意：**
- `axcontainer`：提供 ActiveX 支持
- `widgets`：QAxWidget 需要完整的 QApplication（不能用 QGuiApplication）

---

## 4. RDC 项目中的实现

### 4.1 创建 ActiveX 控件

**代码位置：** `RdpClient::initializeControl()`

```cpp
// 通过 CLSID 创建 MsTscAx ActiveX 控件
m_axWidget = new QAxWidget("7390F3D8-0439-4C05-91E3-CF5CB290C3D0");
```

**关键点：**
1. 使用 CLSID 字符串创建控件
2. `QAxWidget` 自动处理 COM 初始化和窗口创建
3. 控件创建后可以嵌入到 Qt 窗口中

### 4.2 查询 COM 接口

```cpp
// 尝试获取 IMsRdpClient10 接口
m_rdpClient = m_axWidget->querySubObject("IMsRdpClient10");

if (!m_rdpClient) {
    // 降级到 IMsRdpClient9
    m_rdpClient = m_axWidget->querySubObject("IMsRdpClient9");
}
```

**说明：**
- `querySubObject()` 相当于 COM 的 `QueryInterface()`
- 返回 `QAxObject*` 指针，代表特定接口
- 如果接口不存在，返回 `nullptr`
- 可以直接使用 `m_axWidget` 作为基础接口

### 4.3 设置属性

**COM 方式 vs Qt 方式：**

| 操作 | COM/C++ | Qt/QAxBase |
|------|---------|------------|
| 设置属性 | `pRdp->put_Server(L"192.168.1.1")` | `rdp->setProperty("Server", "192.168.1.1")` |
| 获取属性 | `pRdp->get_Server(&bstrServer)` | `rdp->property("Server")` |

**项目实现：**

```cpp
void RdpClient::setRdpProperty(const char *name, const QVariant &value) {
    if (m_rdpClient) {
        m_rdpClient->setProperty(name, value);  // QAxObject 有 setProperty
    } else if (m_axWidget) {
        m_axWidget->setProperty(name, value);   // QAxWidget 也有 setProperty
    }
}
```

**重要差异：**
- `QAxBase` 本身**没有** `setProperty` 方法
- `QAxObject` 和 `QAxWidget` 从各自的父类（`QObject`/`QWidget`）继承了 `setProperty`
- 不能直接对 `QAxBase*` 调用 `setProperty`，需要转换为具体类型

### 4.4 调用方法

**COM 方式 vs Qt 方式：**

| 操作 | COM/C++ | Qt/QAxBase |
|------|---------|------------|
| 调用方法 | `pRdp->Connect()` | `rdp->dynamicCall("Connect()")` |
| 带参数 | `pRdp->SetVirtualChannelOptions(name, options)` | `rdp->dynamicCall("SetVirtualChannelOptions(QString, int)", name, options)` |

**项目实现：**

```cpp
// 发起连接
rdpControl->dynamicCall("Connect()");

// 断开连接
rdpControl->dynamicCall("Disconnect()");
```

**说明：**
- `dynamicCall()` 是 `QAxBase` 的方法，所有 QAx 类都可用
- 方法名区分大小写
- 参数通过 Qt 的元对象系统自动转换

### 4.5 处理事件（信号）

**COM 事件 -> Qt 信号：**

ActiveX 控件的事件会自动转换为 Qt 信号。

```cpp
// 连接 ActiveX 事件到 Qt 槽
QObject::connect(m_axWidget, SIGNAL(OnConnected()), 
                 this, SLOT(onConnected()));
QObject::connect(m_axWidget, SIGNAL(OnDisconnected(int)), 
                 this, SLOT(onDisconnected()));
QObject::connect(m_axWidget, SIGNAL(OnFatalError(int)), 
                 this, SLOT(onFatalError(int)));
```

**事件映射：**

| ActiveX 事件 | Qt 信号 | 说明 |
|-------------|---------|------|
| `OnConnected` | `OnConnected()` | 连接建立 |
| `OnDisconnected` | `OnDisconnected(int)` | 连接断开 |
| `OnLoginComplete` | `OnLoginComplete()` | 登录完成 |
| `OnFatalError` | `OnFatalError(int)` | 致命错误 |

**注意：**
- 必须使用 `SIGNAL()` 和 `SLOT()` 宏（旧式语法）
- 事件名称与 COM 接口定义一致
- 参数类型自动转换

### 4.6 访问子对象

```cpp
// 获取 AdvancedSettings 子对象
QAxObject *advancedSettings = rdpControl->querySubObject("AdvancedSettings9");

// 设置高级选项
advancedSettings->setProperty("RDPPort", 3389);
advancedSettings->setProperty("Compress", 1);
```

**说明：**
- `querySubObject()` 可以访问 COM 对象的属性对象
- 返回的是新的 `QAxObject*`，需要手动管理生命周期
- 子对象也支持 `setProperty()` 和 `dynamicCall()`

---

## 5. 关键差异和注意事项

### 5.1 类型系统差异

| 方面 | COM | Qt |
|------|-----|-----|
| 字符串 | `BSTR` (宽字符) | `QString` (UTF-16) |
| 整数 | `LONG`, `SHORT` | `int`, `short` |
| 布尔 | `VARIANT_BOOL` (-1/0) | `bool` (true/false) |
| 变体 | `VARIANT` | `QVariant` |

**Qt 自动转换：**
- `QVariant` 会自动转换为 `VARIANT`
- `QString` 会自动转换为 `BSTR`
- 基本类型自动映射

### 5.2 内存管理

**COM 规则：**
- 引用计数：`AddRef()` / `Release()`
- 调用者负责释放返回的接口指针

**Qt 处理：**
- `QAxWidget` 和 `QAxObject` 自动管理 COM 引用计数
- 使用 `delete` 或 `deleteLater()` 删除 Qt 对象即可
- **不要**手动调用 `Release()`

**项目实践：**
```cpp
// ✅ 正确：使用 deleteLater() 延迟删除
m_axWidget->deleteLater();

// ❌ 错误：立即 delete 可能导致崩溃
delete m_axWidget;

// ❌ 错误：不要手动管理 COM 引用计数
// m_axWidget->Release();  // 永远不要这样做
```

### 5.3 线程安全

**COM 线程模型：**
- STA（Single-Threaded Apartment）：对象只能在创建线程访问
- MTA（Multi-Threaded Apartment）：对象可以跨线程访问

**MsTscAx 是 STA 对象：**
- 必须在主线程（UI 线程）创建和使用
- 不能从其他线程调用方法
- Qt 的信号槽机制自动处理线程切换

### 5.4 异常处理

**COM 错误：**
- 返回 `HRESULT` 错误码
- 可能抛出 COM 异常

**Qt 处理：**
```cpp
try {
    rdpControl->dynamicCall("Connect()");
} catch (...) {
    // QAxBase 会将 COM 异常转换为 C++ 异常
    qWarning() << "COM call failed";
}
```

### 5.5 属性访问的陷阱

**问题：** `QAxBase` 没有 `setProperty` 方法

```cpp
// ❌ 编译错误
QAxBase *base = getRdpControl();
base->setProperty("Server", "192.168.1.1");  // QAxBase 没有此方法

// ✅ 解决方案 1：使用具体类型
if (m_rdpClient) {
    m_rdpClient->setProperty("Server", "192.168.1.1");  // QAxObject 有
}

// ✅ 解决方案 2：使用辅助方法
void setRdpProperty(const char *name, const QVariant &value) {
    if (m_rdpClient) {
        m_rdpClient->setProperty(name, value);
    } else if (m_axWidget) {
        m_axWidget->setProperty(name, value);
    }
}
```

---

## 6. RDP 会话建立流程

### 6.1 完整流程图

```
用户点击"启动" 
    ↓
配置连接参数（IP、端口、用户名等）
    ↓
点击 OK
    ↓
RdpClient::connectToServer()
    ↓
[首次连接] initializeControl()
    ├─ 创建 QAxWidget (MsTscAx)
    ├─ 查询 IMsRdpClient10/9 接口
    └─ 连接 ActiveX 事件信号
    ↓
configureClient()
    ├─ 设置 Server、Port、UserName
    ├─ 设置分辨率、色彩深度
    ├─ 配置音频、剪贴板、打印机
    └─ 设置高级选项（压缩、缓存等）
    ↓
创建 RdpWindow 窗口
    ├─ 将 QAxWidget 嵌入窗口
    └─ 显示窗口
    ↓
调用 Connect() 方法
    ↓
[ActiveX 控件显示密码输入界面]
    ↓
用户输入密码
    ↓
[触发 OnConnected 事件]
    ↓
RdpClient::onConnected()
    ├─ 设置 m_connected = true
    └─ 发出 connectionSuccess 信号
    ↓
显示远程桌面画面
```

### 6.2 关键代码路径

**1. 初始化控件：**
```cpp
// RdpClient.cpp:initializeControl()
m_axWidget = new QAxWidget("7390F3D8-0439-4C05-91E3-CF5CB290C3D0");
m_rdpClient = m_axWidget->querySubObject("IMsRdpClient10");
```

**2. 配置参数：**
```cpp
// RdpClient.cpp:configureClient()
setRdpProperty("Server", m_server);
setRdpProperty("UserName", m_username);
setRdpProperty("DesktopWidth", m_desktopWidth);
setRdpProperty("DesktopHeight", m_desktopHeight);
setRdpProperty("ColorDepth", m_colorDepth);
```

**3. 显示窗口：**
```cpp
// RdpClient.cpp:connectToServer()
m_rdpWindow = new RdpWindow();
m_rdpWindow->setRdpWidget(m_axWidget);  // 嵌入 ActiveX 控件
m_rdpWindow->show();
```

**4. 发起连接：**
```cpp
// RdpClient.cpp:connectToServer()
rdpControl->dynamicCall("Connect()");
```

**5. 处理事件：**
```cpp
// RdpClient.cpp:onConnected()
void RdpClient::onConnected() {
    m_connected = true;
    emit connectionSuccess();  // 通知 QML 层
}
```

---

## 7. 调试技巧

### 7.1 查看 COM 接口信息

使用 OLE/COM Object Viewer（oleview.exe）：
1. 运行 `oleview.exe`
2. 搜索 "MsTscAx"
3. 查看接口定义、方法、属性、事件

### 7.2 Qt 调试输出

```cpp
// 查看 ActiveX 控件支持的信号
qDebug() << m_axWidget->signalsOnObject();

// 查看支持的属性
qDebug() << m_axWidget->propertyNames();

// 查看支持的方法
qDebug() << m_axWidget->methodNames();
```

### 7.3 常见错误

| 错误 | 原因 | 解决方案 |
|------|------|----------|
| "QAxBase::setProperty not found" | QAxBase 没有此方法 | 使用 QAxObject/QAxWidget 指针 |
| "Failed to create control" | CLSID 错误或未注册 | 检查 CLSID，确认系统支持 |
| 崩溃在析构函数 | 删除顺序错误 | 使用 deleteLater()，先断开连接 |
| 密码界面不显示 | 控件未嵌入窗口 | 确保 QAxWidget 有父窗口并显示 |

---

## 8. 最佳实践

### 8.1 延迟初始化

```cpp
// ✅ 好：在需要时才创建
bool RdpClient::connectToServer() {
    if (!m_axWidget) {
        initializeControl();
    }
    // ...
}

// ❌ 差：在构造函数中创建
RdpClient::RdpClient() {
    initializeControl();  // 可能导致 QML 加载时崩溃
}
```

### 8.2 异常安全

```cpp
// ✅ 好：所有 COM 调用都包裹在 try-catch 中
try {
    rdpControl->dynamicCall("Connect()");
} catch (...) {
    emit connectionError("连接失败");
    return false;
}
```

### 8.3 资源清理

```cpp
// ✅ 好：正确的清理顺序
RdpClient::~RdpClient() {
    // 1. 断开连接
    if (m_connected) {
        rdpControl->dynamicCall("Disconnect()");
    }
    
    // 2. 移除控件
    if (m_rdpWindow) {
        m_rdpWindow->setRdpWidget(nullptr);
    }
    
    // 3. 删除窗口
    if (m_rdpWindow) {
        m_rdpWindow->deleteLater();
    }
    
    // 4. 清除并删除控件
    if (m_axWidget) {
        m_axWidget->clear();
        m_axWidget->deleteLater();
    }
}
```

### 8.4 信号连接

```cpp
// ✅ 好：使用 QObject::connect 明确指定
QObject::connect(m_axWidget, SIGNAL(OnConnected()), 
                 this, SLOT(onConnected()));

// ❌ 差：可能与 RdpClient::connectToServer() 冲突
connect(m_axWidget, SIGNAL(OnConnected()), 
        this, SLOT(onConnected()));
```

---

## 9. 扩展方向

### 9.1 已实现功能
- ✅ 基本连接（IP、端口、用户名）
- ✅ 显示设置（分辨率、色彩深度、全屏）
- ✅ 本地资源（音频、剪贴板、打印机）
- ✅ 窗口显示和断开连接

### 9.2 可扩展功能

**1. 密码管理：**
```cpp
// 设置密码（需要明文，不安全）
rdpControl->setProperty("ClearTextPassword", password);

// 或使用凭据管理器
```

**2. 驱动器重定向：**
```cpp
QAxObject *advSettings = rdpControl->querySubObject("AdvancedSettings");
advSettings->setProperty("RedirectDrives", true);
```

**3. 智能卡重定向：**
```cpp
advSettings->setProperty("RedirectSmartCards", true);
```

**4. 多显示器支持：**
```cpp
rdpControl->setProperty("UseMultimon", true);
```

**5. 连接配置文件：**
- 保存/加载 .rdp 文件
- 使用 `LoadRdpFile()` 和 `SaveRdpFile()` 方法

---

## 10. 参考资料

### 10.1 官方文档
- [IMsRdpClient10 Interface](https://learn.microsoft.com/en-us/windows/win32/termserv/imsrdpclient10)
- [MsTscAx Class](https://learn.microsoft.com/en-us/windows/win32/termserv/mstscax-class)
- [Qt ActiveX Framework](https://doc.qt.io/qt-5/activeqt-index.html)

### 10.2 项目文件
- `RdpClient.h/cpp` - RDP 客户端封装
- `RdpWindow.h/cpp` - 窗口容器
- `ConnectionDialog.qml` - 连接配置界面
- `README.md` - 项目说明
- `TROUBLESHOOTING.md` - 故障排除

---

## 总结

本项目成功地将 Windows COM/ActiveX 技术与 Qt 框架集成，实现了跨技术栈的远程桌面客户端。关键要点：

1. **理解 COM 基础**：接口、CLSID、引用计数
2. **掌握 QAxContainer**：QAxWidget 和 QAxObject 的使用
3. **注意类型差异**：QAxBase 的限制，使用具体类型
4. **正确管理生命周期**：延迟初始化、异常处理、资源清理
5. **利用 Qt 优势**：信号槽机制、自动类型转换、跨平台抽象

通过这种方式，我们可以在现代 Qt/QML 应用中复用 Windows 的成熟 COM 组件，快速实现复杂功能。

# 故障排除指南

## 问题：qlogging.cpp 文件未找到

**原因：** 这是 Qt 调试库的源文件查找提示，不影响程序运行。

**解决方案：**
1. 点击"取消"或"继续"忽略此提示
2. 或在 Visual Studio 中：工具 -> 选项 -> 调试 -> 常规 -> 启用"仅我的代码"

## 问题：Qt5Core.dll 异常退出

**原因：** ActiveX 控件初始化失败，可能是：
- 系统未注册 MsTscAx 控件
- QGuiApplication 不支持 QWidget/ActiveX

**已修复：**
- 改用 QApplication 替代 QGuiApplication
- 延迟初始化 ActiveX 控件（在连接时才创建）
- 添加异常处理和错误提示

## 当前架构

### 延迟初始化
- ActiveX 控件不在构造函数中创建
- 首次调用 `connectToServer()` 时才初始化
- 避免 QML 加载时的初始化问题

### RDP 窗口显示
- 使用独立的 QWidget 窗口（RdpWindow）承载 ActiveX 控件
- 连接时自动显示窗口，用户可以看到密码输入界面
- 窗口包含工具栏和断开连接按钮

### 错误处理
- 所有 ActiveX 操作都包含 try-catch
- 通过信号 `connectionError` 通知 QML 层
- 详细的调试日志输出

### 资源清理
- 使用 `deleteLater()` 延迟删除 Qt 对象，避免立即删除导致的崩溃
- 析构顺序：先断开连接 -> 移除控件 -> 删除窗口 -> 删除 ActiveX 控件
- RdpWindow 不拥有 QAxWidget，只负责显示
- 使用 `clear()` 清除 ActiveX 控件内容后再删除

## 测试步骤

1. 编译并运行程序
2. 如果出现 qlogging.cpp 提示，点击"取消"
3. 程序窗口应该正常显示
4. 点击菜单 [连接] -> [启动]
5. 输入远程主机信息
6. 点击 OK，此时才会初始化 ActiveX 控件

## 系统要求

- Windows 10/11
- Qt 5.15.2
- Visual Studio 2019/2022
- 已安装远程桌面客户端（Windows 自带）

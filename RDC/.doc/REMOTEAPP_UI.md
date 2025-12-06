# RemoteApp UI 实现说明

## 概述
已完成 RDC 项目的 RemoteApp 功能 UI 部分实现。用户现在可以通过主界面菜单栏访问"启动应用"选项来配置 RemoteApp 连接。

## 已实现的功能

### 1. 新增文件
- **RemoteAppDialog.qml**: RemoteApp 配置对话框，参考 ConnectionDialog 的设计风格

### 2. 修改的文件
- **main.qml**: 
  - 在"连接"菜单中添加了"启动应用(&A)"菜单项
  - 集成了 RemoteAppDialog 组件
  - 添加了 RemoteApp 配置的事件处理（目前仅打印日志）

- **qml.qrc**: 
  - 添加了 RemoteAppDialog.qml 到资源文件

## UI 界面说明

### RemoteApp 配置对话框包含以下参数：

#### 连接设置组
- **计算机(C)**: 远程服务器的 IP 地址或主机名
- **端口(P)**: RDP 连接端口（默认 3389）
- **用户名(U)**: 登录用户名

#### 应用程序设置组
根据 ITSRemoteProgram::ServerStartProgram 方法的参数设计：

1. **可执行文件路径(E)** - `bstrExecutablePath`
   - 远程服务器上应用程序的完整路径
   - 示例: `C:\Windows\System32\notepad.exe`
   - **必填项**

2. **文件路径(F)** - `bstrFilePath`
   - 要用应用程序打开的文件路径
   - 示例: `C:\Documents\file.txt`
   - 可选

3. **工作目录(W)** - `bstrWorkingDirectory`
   - 应用程序的工作目录
   - 示例: `C:\Users\Public`
   - 可选
   - 包含复选框: "在服务器上展开环境变量" - `vbExpandEnvVarInWorkingDirectoryOnServer`

4. **命令行参数(A)** - `bstrArguments`
   - 传递给应用程序的命令行参数
   - 示例: `/p /s`
   - 可选
   - 包含复选框: "在服务器上展开环境变量" - `vbExpandEnvVarInArgumentsOnServer`

## 使用方法

1. 启动 RDC 应用程序
2. 点击菜单栏 **[连接] -> [启动应用]**
3. 在弹出的对话框中填写：
   - 连接信息（服务器地址、端口、用户名）
   - 应用程序信息（可执行文件路径等）
4. 点击 **OK** 确认配置

## 后续工作

UI 部分已完成，接下来需要实现功能部分：

1. **RdpClient 类扩展**
   - 添加 RemoteApp 模式支持
   - 实现 `ServerStartProgram` 方法的调用
   - 添加相关属性和信号

2. **连接逻辑**
   - 区分远程桌面模式和 RemoteApp 模式
   - 配置 RDP 客户端为 RemoteApp 模式
   - 处理 RemoteApp 特有的连接参数

3. **错误处理**
   - RemoteApp 特定的错误处理
   - 参数验证

## 参考文档
- [ITSRemoteProgram::ServerStartProgram - Win32 apps | Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/termserv/itsremoteprogram-serverstartprogram)

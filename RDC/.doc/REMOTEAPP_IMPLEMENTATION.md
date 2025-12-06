# RemoteApp 功能实现说明

## 概述
已完成 RDC 项目的 RemoteApp 功能实现，包括 UI 界面和后端功能。用户现在可以通过主界面菜单栏的"启动应用"选项来配置并启动远程应用程序。

## 实现架构

### 1. 核心流程
RemoteApp 的启动流程遵循以下步骤：

1. **用户配置**: 通过 RemoteAppDialog 配置连接信息和应用参数
2. **初始化连接**: 创建 RDP ActiveX 控件并配置基本连接参数
3. **配置 RemoteApp 模式**: 
   - 获取 `RemoteProgram2` 或 `RemoteProgram` 接口
   - 设置 `RemoteProgramMode = VARIANT_TRUE`
4. **建立连接**: 调用 `Connect()` 方法连接到远程服务器
5. **等待登录完成**: 监听 `OnLoginComplete` 事件
6. **启动应用**: 在登录完成后调用 `ServerStartProgram` 方法

### 2. 代码修改

#### RdpClient.h 新增内容

**属性 (Properties)**:
```cpp
// RemoteApp 模式开关
Q_PROPERTY(bool remoteAppMode ...)

// ServerStartProgram 方法的 6 个参数
Q_PROPERTY(QString executablePath ...)           // bstrExecutablePath
Q_PROPERTY(QString filePath ...)                 // bstrFilePath
Q_PROPERTY(QString workingDirectory ...)         // bstrWorkingDirectory
Q_PROPERTY(bool expandEnvVarInWorkingDirectory ...) // vbExpandEnvVarInWorkingDirectoryOnServer
Q_PROPERTY(QString arguments ...)                // bstrArguments
Q_PROPERTY(bool expandEnvVarInArguments ...)     // vbExpandEnvVarInArgumentsOnServer
```

**信号 (Signals)**:
```cpp
void remoteAppStarted();              // RemoteApp 成功启动
void remoteAppError(const QString &error);  // RemoteApp 错误
```

**私有方法**:
```cpp
void configureRemoteApp();  // 配置 RemoteApp 模式
void startRemoteApp();      // 启动远程应用
```

**私有成员**:
```cpp
QAxObject *m_remoteProgram;  // RemoteProgram 接口对象
```

#### RdpClient.cpp 核心实现

**configureRemoteApp() 方法**:
```cpp
void RdpClient::configureRemoteApp() {
  // 1. 获取 RemoteProgram2 接口（或降级到 RemoteProgram）
  m_remoteProgram = rdpControl->querySubObject("RemoteProgram2");
  
  // 2. 设置 RemoteProgramMode = VARIANT_TRUE
  m_remoteProgram->setProperty("RemoteProgramMode", true);
}
```

**startRemoteApp() 方法**:
```cpp
void RdpClient::startRemoteApp() {
  // 调用 ServerStartProgram 方法，传递 6 个参数
  m_remoteProgram->dynamicCall(
    "ServerStartProgram(QString, QString, QString, bool, QString, bool)",
    m_executablePath,
    m_filePath,
    m_workingDirectory,
    m_expandEnvVarInWorkingDirectory,
    m_arguments,
    m_expandEnvVarInArguments
  );
}
```

**onLoginComplete() 修改**:
```cpp
void RdpClient::onLoginComplete() { 
  qDebug() << "RDP Login completed";
  
  // 如果是 RemoteApp 模式，在登录完成后启动应用
  if (m_remoteAppMode) {
    startRemoteApp();
  }
}
```

#### main.qml 集成

**RemoteApp 对话框处理**:
```qml
RemoteAppDialog {
    id: remoteAppDialog
    onAccepted: {
        // 配置连接信息
        rdpClient.server = ip
        rdpClient.port = port
        rdpClient.username = username
        
        // 配置 RemoteApp 参数
        rdpClient.remoteAppMode = true
        rdpClient.executablePath = executablePath
        rdpClient.filePath = filePath
        rdpClient.workingDirectory = workingDirectory
        rdpClient.expandEnvVarInWorkingDirectory = expandEnvVarInWorkingDirectory
        rdpClient.arguments = arguments
        rdpClient.expandEnvVarInArguments = expandEnvVarInArguments
        
        // 发起连接
        rdpClient.connectToServer()
    }
}
```

**信号处理**:
```qml
RdpClient {
    onRemoteAppStarted: {
        statusText.text = "RemoteApp 已启动: " + executablePath
    }
    
    onRemoteAppError: {
        // 显示错误信息
    }
}
```

## 使用方法

### 启动 RemoteApp 应用

1. 启动 RDC 客户端
2. 点击菜单 **[连接] -> [启动应用]**
3. 填写配置信息：

   **连接设置**:
   - 计算机: 远程服务器地址（如 `192.168.1.100`）
   - 端口: RDP 端口（默认 `3389`）
   - 用户名: 登录用户名

   **应用程序设置**:
   - 可执行文件路径: 远程服务器上的程序路径（必填）
     - 示例: `C:\Windows\System32\notepad.exe`
   - 文件路径: 要打开的文件（可选）
     - 示例: `C:\Users\Public\Documents\test.txt`
   - 工作目录: 应用的工作目录（可选）
     - 示例: `C:\Users\Public`
   - 命令行参数: 传递给应用的参数（可选）
     - 示例: `/p /s`

4. 点击 **OK** 启动连接
5. 输入密码（如需要）
6. 等待应用启动

### 示例配置

**启动记事本**:
- 可执行文件路径: `C:\Windows\System32\notepad.exe`
- 其他参数留空

**启动记事本并打开文件**:
- 可执行文件路径: `C:\Windows\System32\notepad.exe`
- 文件路径: `C:\Users\Public\test.txt`

**启动 Word 并打开文档**:
- 可执行文件路径: `C:\Program Files\Microsoft Office\root\Office16\WINWORD.EXE`
- 文件路径: `C:\Users\Public\Documents\report.docx`

**使用环境变量**:
- 可执行文件路径: `C:\Windows\System32\notepad.exe`
- 工作目录: `%USERPROFILE%\Documents`
- 勾选"在服务器上展开环境变量"

## 技术细节

### COM 接口调用

RemoteApp 功能基于 Windows RDP ActiveX 控件的 `ITSRemoteProgram` 接口：

```cpp
// 接口定义
interface ITSRemoteProgram : IDispatch {
  HRESULT ServerStartProgram(
    [in] BSTR         bstrExecutablePath,
    [in] BSTR         bstrFilePath,
    [in] BSTR         bstrWorkingDirectory,
    [in] VARIANT_BOOL vbExpandEnvVarInWorkingDirectoryOnServer,
    [in] BSTR         bstrArguments,
    [in] VARIANT_BOOL vbExpandEnvVarInArgumentsOnServer
  );
};
```

### 时序要求

**关键**: `ServerStartProgram` 必须在 `OnLoginComplete` 事件触发后调用，否则会失败。

时序图：
```
用户点击连接
    ↓
初始化 ActiveX 控件
    ↓
配置 RemoteProgramMode = true
    ↓
调用 Connect()
    ↓
OnConnected 事件
    ↓
OnLoginComplete 事件  ← 在这里调用 ServerStartProgram
    ↓
RemoteApp 窗口显示
```

### 错误处理

实现了完整的错误处理机制：

1. **配置阶段错误**:
   - RemoteProgram 接口获取失败
   - RemoteProgramMode 设置失败

2. **启动阶段错误**:
   - 可执行文件路径为空
   - ServerStartProgram 调用失败

3. **错误信号**:
   - `remoteAppError(QString)` 信号通知 QML 层
   - 在 UI 上显示错误对话框

## 与远程桌面模式的区别

| 特性 | 远程桌面模式 | RemoteApp 模式 |
|------|-------------|----------------|
| 显示内容 | 完整的远程桌面 | 单个应用程序窗口 |
| RemoteProgramMode | false | true |
| 启动方式 | Connect() | Connect() + ServerStartProgram() |
| 窗口集成 | 全屏或窗口化桌面 | 应用窗口无缝集成到本地桌面 |
| 用户体验 | 看到完整的远程桌面 | 像本地应用一样使用 |

## 测试建议

1. **基本功能测试**:
   - 启动简单应用（如记事本）
   - 启动带文件参数的应用
   - 启动带命令行参数的应用

2. **环境变量测试**:
   - 使用 `%USERPROFILE%` 等环境变量
   - 测试环境变量展开功能

3. **错误处理测试**:
   - 输入不存在的可执行文件路径
   - 输入无效的文件路径
   - 测试网络断开情况

4. **兼容性测试**:
   - 不同版本的 Windows Server
   - 不同的 RDP 客户端版本
   - 不同的应用程序类型

## 参考文档

- [ITSRemoteProgram::ServerStartProgram - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/termserv/itsremoteprogram-serverstartprogram)
- [Remote Desktop Services API Reference](https://learn.microsoft.com/en-us/windows/win32/termserv/terminal-services-api-reference)
- [RemoteApp and Desktop Connections](https://learn.microsoft.com/en-us/windows-server/remote/remote-desktop-services/clients/remote-desktop-app-compare)

## 已知限制

1. 需要远程服务器支持 RemoteApp 功能（Windows Server 2008 R2 及以上）
2. 某些应用程序可能不支持 RemoteApp 模式
3. 文件路径必须是远程服务器上的路径，不是本地路径
4. 需要适当的服务器端配置和权限

## 后续优化建议

1. **UI 改进**:
   - 添加常用应用的快捷配置
   - 保存和加载配置文件
   - 应用程序图标显示

2. **功能增强**:
   - 支持多个 RemoteApp 同时运行
   - 文件拖放支持
   - 剪贴板增强

3. **用户体验**:
   - 连接状态更详细的反馈
   - 启动进度指示
   - 自动重连机制

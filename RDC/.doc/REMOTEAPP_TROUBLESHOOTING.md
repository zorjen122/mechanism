# RemoteApp 故障排查指南

## 问题 1: "无法获取RemoteProgram接口"

### 错误信息
```
QAxBase: Error calling IDispatch member RemoteProgram2: Exception thrown by server
Code       : -2147467259
RemoteProgram2 not available, trying RemoteProgram
QAxBase: Error calling IDispatch member RemoteProgram: Exception thrown by server
Failed to get RemoteProgram interface
```

### 可能原因

1. **RDP ActiveX 控件版本过旧**
   - RemoteApp 功能需要 RDP 7.0 或更高版本
   - Windows 7 及以上系统通常已包含

2. **服务器不支持 RemoteApp**
   - 需要 Windows Server 2008 R2 或更高版本
   - 需要安装并配置远程桌面服务 (RDS)
   - 需要启用 RemoteApp 功能

3. **接口获取方式不正确**
   - RemoteProgram 接口的获取方式在不同版本中可能不同

### 解决方案

#### 方案 1: 检查 RDP 客户端版本

在命令提示符中运行：
```cmd
mstsc /?
```

查看版本信息，确保是 RDP 7.0 或更高版本。

#### 方案 2: 检查服务器配置

在远程服务器上：

1. 打开"服务器管理器"
2. 检查"远程桌面服务"角色是否已安装
3. 确认 RemoteApp 功能已启用

PowerShell 检查命令：
```powershell
Get-WindowsFeature -Name RDS-RD-Server
Get-RDRemoteApp
```

#### 方案 3: 使用不同的接口获取方式

代码已更新为尝试多种方式：
1. `GetRemoteProgram2()` 方法
2. `GetRemoteProgram()` 方法
3. `RemoteProgram2` 属性
4. `RemoteProgram` 属性

#### 方案 4: 验证基本 RDP 连接

先测试普通远程桌面连接是否正常：
1. 使用"启动"菜单（而不是"启动应用"）
2. 确认能够成功连接到远程桌面
3. 如果基本连接失败，先解决连接问题

## 问题 2: 连接成功但应用未启动

### 症状
- 连接建立成功
- 显示"RDP Connected successfully"
- 但没有应用窗口出现

### 可能原因

1. **ServerStartProgram 调用时机错误**
   - 必须在 `OnLoginComplete` 事件后调用
   - 当前代码已正确实现

2. **可执行文件路径错误**
   - 路径必须是远程服务器上的路径
   - 路径必须存在且可访问

3. **权限问题**
   - 用户没有执行该程序的权限
   - 程序需要管理员权限但未提供

### 解决方案

#### 检查路径
确保使用远程服务器上的路径：
```
正确: C:\Windows\System32\notepad.exe
错误: D:\MyApp\app.exe (如果远程服务器没有 D 盘)
```

#### 测试简单应用
先测试系统自带的应用：
```
记事本: C:\Windows\System32\notepad.exe
计算器: C:\Windows\System32\calc.exe
CMD: C:\Windows\System32\cmd.exe
```

#### 检查日志
查看详细日志输出：
```
========== Starting RemoteApp ==========
  Executable: C:\Windows\System32\notepad.exe
  File: 
  WorkDir: 
  ExpandWorkDir: false
  Arguments: 
  ExpandArgs: false
========================================
```

## 问题 3: "RemoteProgram接口不可用"

### 症状
在 `onLoginComplete` 时显示此错误

### 原因
`configureRemoteApp()` 未能成功获取接口

### 解决方案

1. **检查 configureRemoteApp 的调用时机**
   - 应该在 `connectToServer()` 中，Connect() 之前调用
   - 当前代码已正确实现

2. **添加更多调试信息**
   ```cpp
   qDebug() << "m_remoteProgram pointer:" << m_remoteProgram;
   qDebug() << "m_remoteAppMode:" << m_remoteAppMode;
   ```

## 问题 4: COM 异常

### 错误信息
```
QAxBase: Error calling IDispatch member: Exception thrown by server
Code: -2147467259
```

### 错误代码含义

| 错误代码 | 含义 | 可能原因 |
|---------|------|---------|
| -2147467259 (0x80004005) | E_FAIL | 一般性失败 |
| -2147024809 (0x80070057) | E_INVALIDARG | 参数无效 |
| -2147024891 (0x80070005) | E_ACCESSDENIED | 访问被拒绝 |
| -2147024894 (0x80070002) | E_FILENOTFOUND | 文件未找到 |

### 解决方案

根据错误代码采取相应措施：

**E_FAIL (0x80004005)**:
- 检查服务器是否支持 RemoteApp
- 检查 RDP 版本兼容性
- 尝试重启 RDP 服务

**E_INVALIDARG (0x80070057)**:
- 检查传递的参数是否正确
- 确保路径格式正确
- 检查布尔值参数

**E_ACCESSDENIED (0x80070005)**:
- 检查用户权限
- 尝试使用管理员账户
- 检查服务器端策略设置

## 问题 5: 参数传递问题

### 症状
应用启动但参数未生效

### 调试方法

1. **启动 CMD 查看参数**
   ```
   可执行文件: C:\Windows\System32\cmd.exe
   参数: /k echo %1 %2 %3
   ```

2. **检查环境变量展开**
   ```
   工作目录: %USERPROFILE%\Documents
   ☑ 在服务器上展开环境变量
   ```

3. **查看日志输出**
   ```
   Arguments: /k echo test
   ExpandArgs: false
   ```

## 调试技巧

### 1. 启用详细日志

在代码中已添加详细日志，运行时查看控制台输出。

### 2. 使用 Process Monitor

在远程服务器上运行 Process Monitor (procmon.exe)：
1. 过滤进程名称
2. 查看进程启动参数
3. 检查文件访问

### 3. 检查事件查看器

在远程服务器上：
1. 打开"事件查看器"
2. 查看"Windows 日志" -> "应用程序"
3. 查找 RemoteApp 相关错误

### 4. 测试矩阵

| 测试项 | 配置 | 预期结果 | 实际结果 |
|-------|------|---------|---------|
| 基本连接 | 远程桌面模式 | 成功 | |
| 简单应用 | notepad.exe | 启动 | |
| 带文件 | notepad.exe + file.txt | 打开文件 | |
| 带参数 | cmd.exe /k echo | 显示文本 | |
| 环境变量 | %USERPROFILE% | 展开 | |

## 常见配置错误

### 错误 1: 使用本地路径
```
❌ 错误: D:\MyApp\app.exe (本地路径)
✅ 正确: C:\Program Files\MyApp\app.exe (远程路径)
```

### 错误 2: 路径分隔符
```
❌ 错误: C:/Windows/System32/notepad.exe (Unix 风格)
✅ 正确: C:\Windows\System32\notepad.exe (Windows 风格)
```

### 错误 3: 引号使用
```
❌ 错误: "C:\Program Files\app.exe" (路径中包含引号)
✅ 正确: C:\Program Files\app.exe (不需要引号)
```

### 错误 4: 环境变量格式
```
❌ 错误: $USERPROFILE (Unix 风格)
❌ 错误: ${USERPROFILE} (Bash 风格)
✅ 正确: %USERPROFILE% (Windows 风格)
```

## 服务器端配置检查清单

- [ ] Windows Server 2008 R2 或更高版本
- [ ] 已安装远程桌面服务角色
- [ ] 已启用 RemoteApp 功能
- [ ] 用户有远程登录权限
- [ ] 用户有执行目标程序的权限
- [ ] 防火墙允许 RDP 连接 (端口 3389)
- [ ] 网络连接正常

## 客户端配置检查清单

- [ ] Windows 7 或更高版本
- [ ] RDP 客户端版本 7.0 或更高
- [ ] Qt 5.15.2 或兼容版本
- [ ] QAxContainer 模块已安装
- [ ] 网络可以访问服务器

## 获取帮助

如果以上方法都无法解决问题：

1. **收集信息**:
   - 完整的错误日志
   - 客户端和服务器版本信息
   - 测试配置详情

2. **检查文档**:
   - Microsoft RemoteApp 官方文档
   - Qt QAxObject 文档
   - RDP ActiveX 控件文档

3. **社区支持**:
   - Qt 论坛
   - Stack Overflow
   - Microsoft 技术社区

## 参考资源

- [RemoteApp 部署指南](https://learn.microsoft.com/en-us/windows-server/remote/remote-desktop-services/rds-deploy-infrastructure)
- [ITSRemoteProgram 接口文档](https://learn.microsoft.com/en-us/windows/win32/termserv/itsremoteprogram)
- [QAxObject 类文档](https://doc.qt.io/qt-5/qaxobject.html)
- [RDP 错误代码参考](https://learn.microsoft.com/en-us/troubleshoot/windows-server/remote/understanding-remote-desktop-protocol-error-codes)

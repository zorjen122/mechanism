# RDP Client Control v11 RemoteApp 修复

## 问题发现

### 错误信息
```
QAxBase::dynamicCallHelper: ServerStartProgram(QString,QString,QString,bool,QString,bool): 
No such property in 7390F3D8-0439-4C05-91E3-CF5CB290C3D0 
[Microsoft RDP Client Control (redistributable) - version 11]

Candidates are:
SecuredSettings
SecuredSettings2
SecuredSettings3
SecuredSettingsEnabled
ServerStart          ← 注意这个！
Connected
```

### 关键发现

1. **方法名不同**: RDP v11 有 `ServerStart` 而不是 `ServerStartProgram`
2. **需要 RemoteProgram 对象**: 不能直接在 m_axWidget 上调用
3. **参数可能不同**: `ServerStart` 可能只接受可执行文件路径

## 解决方案

### 1. 必须获取 RemoteProgram 对象

```cpp
// 错误做法：直接在 m_axWidget 上调用
m_axWidget->dynamicCall("ServerStartProgram(...)", ...);  // ❌ 失败

// 正确做法：先获取 RemoteProgram
m_remoteProgram = m_axWidget->querySubObject("GetRemoteProgram()");
m_remoteProgram->dynamicCall("ServerStartProgram(...)", ...);  // ✅ 可能成功
```

### 2. 尝试多种方法名

```cpp
// 方法1: 标准的 ServerStartProgram（6个参数）
m_remoteProgram->dynamicCall(
  "ServerStartProgram(QString, QString, QString, bool, QString, bool)",
  executablePath, filePath, workingDir, expandWorkDir, args, expandArgs
);

// 方法2: 简化的 ServerStart（1个参数）
m_remoteProgram->dynamicCall("ServerStart(QString)", executablePath);
```

### 3. 完整的调用流程

```cpp
void RdpClient::startRemoteApp() {
  // 步骤1: 确保有 RemoteProgram 对象
  if (!m_remoteProgram) {
    m_remoteProgram = m_axWidget->querySubObject("GetRemoteProgram()");
  }
  
  if (!m_remoteProgram) {
    // 无法获取 RemoteProgram，RemoteApp 不可用
    return;
  }
  
  // 步骤2: 尝试调用 ServerStartProgram
  try {
    m_remoteProgram->dynamicCall("ServerStartProgram(...)", ...);
  } catch (...) {
    // 步骤3: 如果失败，尝试 ServerStart
    m_remoteProgram->dynamicCall("ServerStart(QString)", executablePath);
  }
}
```

## RDP v11 的特殊性

### 接口差异

| 特性 | RDP 7.x/8.x | RDP 11.x |
|------|-------------|----------|
| RemoteProgram 获取 | `querySubObject("RemoteProgram")` | `querySubObject("GetRemoteProgram()")` |
| 方法名 | `ServerStartProgram` | `ServerStart` 或 `ServerStartProgram` |
| 参数数量 | 6个参数 | 可能只支持1个参数 |
| 直接调用 | 可能支持 | 不支持，必须通过 RemoteProgram |

### 可用的候选方法

根据错误信息，RDP v11 控件有以下属性/方法：
- `SecuredSettings`
- `SecuredSettings2`
- `SecuredSettings3`
- `SecuredSettingsEnabled`
- **`ServerStart`** ← 这个可能是我们需要的
- `Connected`

## 测试方法

### 测试 1: 验证 RemoteProgram 对象

```cpp
QAxObject *remoteProgram = m_axWidget->querySubObject("GetRemoteProgram()");
if (remoteProgram) {
  qDebug() << "RemoteProgram obtained successfully";
  
  // 列出所有可用方法
  // 使用 COM 类型库查看器或 OleView
} else {
  qDebug() << "Failed to get RemoteProgram";
}
```

### 测试 2: 尝试不同的方法签名

```cpp
// 测试 1: 完整参数
remoteProgram->dynamicCall(
  "ServerStartProgram(QString, QString, QString, bool, QString, bool)",
  "C:\\Windows\\System32\\notepad.exe", "", "", false, "", false
);

// 测试 2: 简化参数
remoteProgram->dynamicCall(
  "ServerStart(QString)", 
  "C:\\Windows\\System32\\notepad.exe"
);

// 测试 3: 使用 BSTR
remoteProgram->dynamicCall(
  "ServerStartProgram(BSTR, BSTR, BSTR, VARIANT_BOOL, BSTR, VARIANT_BOOL)",
  ...
);
```

### 测试 3: 使用 OleView 查看接口

1. 运行 `oleview.exe`
2. 找到 CLSID `{7390F3D8-0439-4C05-91E3-CF5CB290C3D0}`
3. 查看 `ITSRemoteProgram` 接口
4. 确认方法名和参数

## 调试日志分析

### 成功的日志应该是：

```
========== Starting RemoteApp ==========
Executable: "C:\Windows\System32\notepad.exe"
...
========================================
RemoteProgram not available, trying to get it now...
Success: Got RemoteProgram via GetRemoteProgram()
Using RemoteProgram object: QAxObject(...)
Attempt 1: ServerStartProgram(QString, ...)...
Success: ServerStartProgram called!
========== RemoteApp Started ==========
```

### 当前的问题日志：

```
Using object for ServerStartProgram: 0x262aeaaf130
Object type: m_axWidget                          ← 问题：使用了 m_axWidget
Attempt 1: Standard ServerStartProgram call...
QAxBase::dynamicCallHelper: ServerStartProgram(...): No such property
                                                     ← 问题：方法不存在
```

## 可能的原因

### 1. 没有获取 RemoteProgram 对象

**问题**: 直接在 m_axWidget 上调用  
**解决**: 必须先 `querySubObject("GetRemoteProgram()")`

### 2. 方法名错误

**问题**: RDP v11 可能使用不同的方法名  
**解决**: 尝试 `ServerStart` 而不是 `ServerStartProgram`

### 3. 参数类型不匹配

**问题**: QString 可能不被接受  
**解决**: 尝试 BSTR 或其他类型

## 推荐的实现

```cpp
void RdpClient::startRemoteApp() {
  // 1. 获取 RemoteProgram（关键！）
  if (!m_remoteProgram) {
    m_remoteProgram = m_axWidget->querySubObject("GetRemoteProgram()");
    if (!m_remoteProgram) {
      emit remoteAppError("无法获取RemoteProgram对象");
      return;
    }
  }
  
  // 2. 尝试完整的 ServerStartProgram
  bool success = false;
  try {
    m_remoteProgram->dynamicCall(
      "ServerStartProgram(QString, QString, QString, bool, QString, bool)",
      m_executablePath, m_filePath, m_workingDirectory,
      m_expandEnvVarInWorkingDirectory, m_arguments, m_expandEnvVarInArguments
    );
    success = true;
  } catch (...) {
    qWarning() << "ServerStartProgram failed";
  }
  
  // 3. 降级到简化的 ServerStart
  if (!success) {
    try {
      m_remoteProgram->dynamicCall("ServerStart(QString)", m_executablePath);
      success = true;
    } catch (...) {
      qWarning() << "ServerStart also failed";
    }
  }
  
  if (success) {
    emit remoteAppStarted();
  } else {
    emit remoteAppError("所有启动方法都失败了");
  }
}
```

## 备选方案：使用 .rdp 文件

如果 ActiveX 方式完全不可行：

```cpp
void RdpClient::startRemoteAppViaRdpFile() {
  QString rdpContent = QString(
    "full address:s:%1:%2\n"
    "username:s:%3\n"
    "remoteapplicationmode:i:1\n"
    "remoteapplicationname:s:MyApp\n"
    "remoteapplicationprogram:s:%4\n"
    "remoteapplicationcmdline:s:%5\n"
  ).arg(m_server).arg(m_port).arg(m_username)
   .arg(m_executablePath).arg(m_arguments);
  
  QString tempFile = QDir::temp().filePath("remoteapp.rdp");
  QFile file(tempFile);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    file.write(rdpContent.toUtf8());
    file.close();
    QProcess::startDetached("mstsc.exe", QStringList() << tempFile);
  }
}
```

## 下一步行动

1. **重新编译** - 使用修复后的代码
2. **查看日志** - 确认是否成功获取 RemoteProgram
3. **测试调用** - 查看哪种方法签名有效
4. **如果仍失败** - 考虑使用 .rdp 文件方案

## 参考资源

- [ITSRemoteProgram Interface](https://learn.microsoft.com/en-us/windows/win32/termserv/itsremoteprogram)
- [RDP File Settings](https://learn.microsoft.com/en-us/windows-server/remote/remote-desktop-services/clients/rdp-files)
- [QAxObject Documentation](https://doc.qt.io/qt-5/qaxobject.html)

---

**更新日期**: 2025-12-05  
**问题**: ServerStartProgram 方法不存在  
**解决**: 必须通过 RemoteProgram 对象调用，可能需要使用 ServerStart

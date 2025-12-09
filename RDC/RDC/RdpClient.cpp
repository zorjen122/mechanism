#include "RdpClient.h"
#include "RdpWindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>

RdpClient::RdpClient(QObject *parent)
    : QObject(parent), m_axWidget(nullptr), m_rdpClient(nullptr),
      m_rdpWindow(nullptr), m_port(3389), m_desktopWidth(1920),
      m_desktopHeight(1080), m_colorDepth(32),
      m_fullScreenTitle("VirWork Client"), m_fullScreen(false),
      m_enableSound(true), m_enableClipboard(true), m_enablePrinter(false),
      m_connected(false), m_remoteAppMode(false),
      m_expandEnvVarInWorkingDirectory(false), m_expandEnvVarInArguments(false),
      m_remoteProgram(nullptr) {
  // 不在构造函数中初始化 ActiveX 控件，避免在 QML 加载时出错
  // initializeControl();
}

RdpClient::~RdpClient() {
  qDebug() << "RdpClient destructor called";

  // 先断开连接
  if (m_connected) {
    try {
      if (m_axWidget) {
        QAxBase *rdpControl = getRdpControl();
        rdpControl->dynamicCall("Disconnect()");
      }
      m_connected = false;
    } catch (...) {
      qWarning() << "Exception during disconnect in destructor";
    }
  }

  // 从窗口中移除控件
  if (m_rdpWindow && m_axWidget) {
    m_rdpWindow->setRdpWidget(nullptr);
  }

  // 删除窗口
  if (m_rdpWindow) {
    m_rdpWindow->close();
    m_rdpWindow->deleteLater();
    m_rdpWindow = nullptr;
  }

  // 清理 RemoteProgram 对象
  if (m_remoteProgram) {
    delete m_remoteProgram;
    m_remoteProgram = nullptr;
  }

  // 最后删除 ActiveX 控件
  if (m_axWidget) {
    m_axWidget->clear(); // 清除 ActiveX 控件内容
    m_axWidget->deleteLater();
    m_axWidget = nullptr;
  }

  m_rdpClient = nullptr;
  qDebug() << "RdpClient destructor finished";
}

void RdpClient::initializeControl() {
  if (m_axWidget) {
    qDebug() << "RDP Control already initialized";
    return;
  }

  qDebug() << "Initializing RDP ActiveX control...";

  try {
    // 创建 MsTscAx ActiveX 控件
    // CLSID: {7390F3D8-0439-4C05-91E3-CF5CB290C3D0}
    m_axWidget = new QAxWidget("MsTscAx.MsTscAx");

    if (!m_axWidget) {
      qCritical() << "Failed to create RDP ActiveX control";
      emit connectionError(QString::fromUtf8("无法创建RDP控件"));
      return;
    }
    
    if (!m_rdpClient) {
      qWarning() << "Using QAxWidget directly for COM calls";
    }
    qDebug() << "QAxWidget interface COM ID: " << m_axWidget->control();
	qDebug() << "RDP Control COM interface ID: " << (m_rdpClient ? m_rdpClient->control() : "Null");

    // 连接信号
    QObject::connect(m_axWidget, SIGNAL(OnConnected()), this,
                     SLOT(onConnected()));
    QObject::connect(m_axWidget, SIGNAL(OnDisconnected(int)), this,
                     SLOT(onDisconnected()));
    QObject::connect(m_axWidget, SIGNAL(OnLoginComplete()), this,
                     SLOT(onLoginComplete()));
    QObject::connect(m_axWidget, SIGNAL(OnFatalError(int)), this,
                     SLOT(onFatalError(int)));
    
    // 连接 RemoteApp 相关信号 - 使用正确的枚举类型
    bool remoteProgramConnected = QObject::connect(m_axWidget, 
        SIGNAL(OnRemoteProgramResult(QString, __MIDL___MIDL_itf_mstsax_0000_0059_0001, bool)), 
        this, 
        SLOT(onRemoteProgramResult(QString, QVariant, bool)));
    
    if (remoteProgramConnected) {
        qDebug() << "OnRemoteProgramResult signal connected successfully";
    } else {
        qWarning() << "Failed to connect OnRemoteProgramResult signal";
    }

    qDebug() << "RDP Control initialized successfully";
  } catch (...) {
    qCritical() << "Exception occurred while initializing RDP control";
    emit connectionError(QString::fromUtf8("初始化RDP控件时发生异常"));
    if (m_axWidget) {
      delete m_axWidget;
      m_axWidget = nullptr;
    }
    m_rdpClient = nullptr;
  }
}

QAxBase *RdpClient::getRdpControl() {
  // 优先使用 m_rdpClient，否则使用 m_axWidget
  if (m_rdpClient) {
    return m_rdpClient;
  }
  return m_axWidget;
}

void RdpClient::setRdpProperty(const char *name, const QVariant &value) {
  // 辅助方法：设置 RDP 属性
  if (m_rdpClient) {
    m_rdpClient->setProperty(name, value);
  } else if (m_axWidget) {
    m_axWidget->setProperty(name, value);
  }
}

void RdpClient::configureClient() {
  if (!m_axWidget) {
    qCritical() << "RDP control not initialized";
    return;
  }

  try {
    QAxBase *rdpControl = getRdpControl();

    // 设置服务器地址
    setRdpProperty("Server", m_server);
    qDebug() << "Server set to:" << m_server;

    // 设置服务器端口
    QAxObject *advancedSettings =
        rdpControl->querySubObject("AdvancedSettings9");
    if (!advancedSettings) {
      advancedSettings = rdpControl->querySubObject("AdvancedSettings");
    }

    if (!advancedSettings) {
		qDebug() << "Failed to get AdvancedSettings object.";
        return;
    }

    advancedSettings->setProperty("RDPPort", m_port);
    qDebug() << "Port set to:" << m_port;

    // 设置用户名
    setRdpProperty("UserName", m_username);
    qDebug() << "Username set to:" << m_username;

    // 设置桌面分辨率
    setRdpProperty("DesktopWidth", m_desktopWidth);
    setRdpProperty("DesktopHeight", m_desktopHeight);
    qDebug() << "Resolution set to:" << m_desktopWidth << "x"
             << m_desktopHeight;

    // 设置色彩深度
    setRdpProperty("ColorDepth", m_colorDepth);
    qDebug() << "Color depth set to:" << m_colorDepth;

    // 设置全屏标题
    setRdpProperty("FullScreenTitle", m_fullScreenTitle);
    qDebug() << "Full screen title set to:" << m_fullScreenTitle;

    // 设置全屏模式
    setRdpProperty("FullScreen", m_fullScreen);
    qDebug() << "Full screen mode:" << m_fullScreen;

    // 其他常用设置
    if (advancedSettings) {
      // 启用压缩
      advancedSettings->setProperty("Compress", 1);
      // 位图缓存
      advancedSettings->setProperty("BitmapPeristence", 1);
      // 允许桌面组合
      advancedSettings->setProperty("allowDesktopComposition", true);

      // 音频设置 (0=本地播放, 1=远程播放, 2=不播放)
      advancedSettings->setProperty("AudioRedirectionMode",
                                    m_enableSound ? 0 : 2);
      qDebug() << "Audio enabled:" << m_enableSound;

      // 剪贴板重定向
      advancedSettings->setProperty("RedirectClipboard", m_enableClipboard);
      qDebug() << "Clipboard enabled:" << m_enableClipboard;

      // 打印机重定向
      advancedSettings->setProperty("RedirectPrinters", m_enablePrinter);
      qDebug() << "Printer enabled:" << m_enablePrinter;
    }

  } catch (...) {
    qCritical() << "Exception occurred while configuring RDP client";
  }
}

bool RdpClient::connectToServer() {
  if (m_server.isEmpty()) {
    emit connectionError(QString::fromUtf8("服务器地址不能为空"));
    return false;
  }

  if (m_username.isEmpty()) {
    emit connectionError(QString::fromUtf8("用户名不能为空"));
    return false;
  }

  // 延迟初始化 ActiveX 控件
  if (!m_axWidget) {
    initializeControl();
  }

  if (!m_axWidget) {
    emit connectionError(QString::fromUtf8("RDP控件未初始化"));
    return false;
  }

  configureClient();
  
  // 如果是 RemoteApp 模式，配置 RemoteApp
  if (m_remoteAppMode) {
      qDebug() << "Connecting in RemoteApp mode";
      configureRemoteApp();
      // 如果是 RemoteApp 模式，在登录完成后启动应用
      if (!m_remoteProgram) {
          qCritical() << "Cannot get RemoteProgram object - RemoteApp not supported";
          emit remoteAppError(QString::fromUtf8(
              "无法获取RemoteProgram对象\n\n"
              "此RDP客户端版本可能不支持RemoteApp功能。\n"
              "建议使用 .rdp 文件方式或 mstsc.exe。"
          ));
          return false;
      }
      
      // 注意：不在这里启动 RemoteApp，而是在 onLoginComplete() 中启动
  } else {
      // 桌面模式：确保 RemoteApp 模式被禁用
      qDebug() << "Connecting in Desktop mode";
      QAxBase* rdpControl = getRdpControl();
      if (rdpControl) {
          QAxObject* remoteProgram = rdpControl->querySubObject("RemoteProgram2");
          if (!remoteProgram) {
              remoteProgram = rdpControl->querySubObject("RemoteProgram");
          }
          if (remoteProgram) {
              try {
                  remoteProgram->setProperty("RemoteProgramMode", false);
                  qDebug() << "RemoteProgramMode disabled for Desktop mode";
                  delete remoteProgram;
              } catch (...) {
                  qWarning() << "Exception while disabling RemoteApp mode";
              }
          }
      }
  }

  // 创建并显示 RDP 窗口
  if (!m_rdpWindow) {
    m_rdpWindow = new RdpWindow();
    m_rdpWindow->setServerName(m_server);
    m_rdpWindow->setRdpWidget(m_axWidget);

    // 连接断开信号
    QObject::connect(m_rdpWindow, &RdpWindow::disconnectRequested, this,
                     &RdpClient::disconnectFromServer);
  }

  // 显示窗口
  m_rdpWindow->show();
  m_rdpWindow->raise();
  m_rdpWindow->activateWindow();

  try {
    QAxBase *rdpControl = getRdpControl();

    // 发起连接
    rdpControl->dynamicCall("Connect()");
    qDebug() << "RDP connection initiated to" << m_server;
    return true;
  } catch (...) {
    emit connectionError(QString::fromUtf8("连接失败：无法调用Connect方法"));
    return false;
  }
}

void RdpClient::disconnectFromServer() {
  qDebug() << "Disconnecting from server...";

  if (m_axWidget) {
    try {
      QAxBase *rdpControl = getRdpControl();
      if (rdpControl && m_connected) {
        rdpControl->dynamicCall("Disconnect()");
        qDebug() << "RDP disconnected";
      }
    } catch (...) {
      qWarning() << "Exception occurred while disconnecting";
    }
  }

  m_connected = false;

  // 关闭窗口
  if (m_rdpWindow) {
    m_rdpWindow->hide();
  }
}

QWidget *RdpClient::getWidget() { return m_axWidget; }

// Property setters
void RdpClient::setServer(const QString &server) {
  if (m_server != server) {
    m_server = server;
    emit serverChanged();
  }
}

void RdpClient::setUsername(const QString &username) {
  if (m_username != username) {
    m_username = username;
    emit usernameChanged();
  }
}

void RdpClient::setPort(int port) {
  if (m_port != port) {
    m_port = port;
    emit portChanged();
  }
}

void RdpClient::setDesktopWidth(int width) {
  if (m_desktopWidth != width) {
    m_desktopWidth = width;
    emit desktopWidthChanged();
  }
}

void RdpClient::setDesktopHeight(int height) {
  if (m_desktopHeight != height) {
    m_desktopHeight = height;
    emit desktopHeightChanged();
  }
}

void RdpClient::setColorDepth(int depth) {
  if (m_colorDepth != depth) {
    m_colorDepth = depth;
    emit colorDepthChanged();
  }
}

void RdpClient::setFullScreenTitle(const QString &title) {
  if (m_fullScreenTitle != title) {
    m_fullScreenTitle = title;
    emit fullScreenTitleChanged();
  }
}

void RdpClient::setFullScreen(bool fullScreen) {
  if (m_fullScreen != fullScreen) {
    m_fullScreen = fullScreen;
    emit fullScreenChanged();
  }
}

void RdpClient::setEnableSound(bool enable) {
  if (m_enableSound != enable) {
    m_enableSound = enable;
    emit enableSoundChanged();
  }
}

void RdpClient::setEnableClipboard(bool enable) {
  if (m_enableClipboard != enable) {
    m_enableClipboard = enable;
    emit enableClipboardChanged();
  }
}

void RdpClient::setEnablePrinter(bool enable) {
  if (m_enablePrinter != enable) {
    m_enablePrinter = enable;
    emit enablePrinterChanged();
  }
}

// Slots for RDP events
void RdpClient::onConnected() {
  m_connected = true;
  emit connectedChanged();
  emit connectionSuccess();
  qDebug() << "RDP Connected successfully";
}

void RdpClient::onDisconnected() {
  m_connected = false;
  emit connectedChanged();
  qDebug() << "RDP Disconnected";
  
  // 断开连接后清理 RemoteApp 状态
  if (m_remoteProgram) {
    try {
      // 禁用 RemoteApp 模式
      m_remoteProgram->setProperty("RemoteProgramMode", false);
    } catch (...) {
      qWarning() << "Exception while disabling RemoteApp mode";
    }
    delete m_remoteProgram;
    m_remoteProgram = nullptr;
  }
}

void RdpClient::onLoginComplete() { 
  qDebug() << "RDP Login completed";
  
  // 如果是 RemoteApp 模式，在登录完成后启动应用
  if (m_remoteAppMode && m_remoteProgram) {
    qDebug() << "Login complete, starting RemoteApp...";
    startRemoteApp();
  }
}

void RdpClient::onFatalError(int errorCode) {
  m_connected = false;
  emit connectedChanged();
  emit connectionError(
      QString::fromUtf8("致命错误，错误代码: %1").arg(errorCode));
  qCritical() << "RDP Fatal error:" << errorCode;
}

// RemoteApp configuration
void RdpClient::configureRemoteApp() {
    QAxBase* rdpControl = getRdpControl();
    if (!rdpControl || !m_remoteAppMode)
        return;
    
    qDebug() << "========== Configuring RemoteApp ==========";
    try
    {
        // 尝试获取 RemoteProgram2 接口（更新版本）
        m_remoteProgram = rdpControl->querySubObject("RemoteProgram2");
        if (!m_remoteProgram)
        {
            qDebug() << "Failed to get RemoteProgram2 interface, trying RemoteProgram...";
            m_remoteProgram = rdpControl->querySubObject("RemoteProgram");
            if (!m_remoteProgram)
            {
                qCritical() << "Failed to get RemoteProgram interface";
                emit remoteAppError(QString::fromUtf8(
                    "无法获取 RemoteProgram 接口。\n\n"
                    "此 RDP 客户端版本可能不支持 RemoteApp 功能。\n"
                    "建议使用 .rdp 文件方式或 mstsc.exe。"
                ));
                return;
            }
        }
        
        qDebug() << "RemoteProgram interface obtained successfully";
        
        // 启用 RemoteApp 模式
        m_remoteProgram->setProperty("RemoteProgramMode", true);
        
        // 验证模式是否设置成功
        bool modeSet = m_remoteProgram->property("RemoteProgramMode").toBool();
        qDebug() << "RemoteProgramMode set to:" << modeSet;
        
        if (!modeSet) {
            qCritical() << "Failed to enable RemoteProgramMode";
            emit remoteAppError(QString::fromUtf8("无法启用 RemoteApp 模式"));
        }

    } catch(...) {
        qCritical() << "Exception occurred while configuring RemoteApp";
        emit remoteAppError(QString::fromUtf8(
            "配置 RemoteApp 时发生异常。\n\n"
            "此 RDP 客户端版本可能不支持 RemoteApp 功能。\n"
            "建议使用 .rdp 文件方式或 mstsc.exe。"
        ));
	}
}

// Start RemoteApp after login
void RdpClient::startRemoteApp() {
  if (!m_remoteProgram) {
    qCritical() << "RemoteProgram object not initialized";
    emit remoteAppError(QString::fromUtf8("RemoteProgram对象未初始化"));
    return;
  }
  
  if (m_executablePath.isEmpty()) {
    qCritical() << "Executable path is empty";
    emit remoteAppError(QString::fromUtf8("可执行文件路径不能为空"));
    return;
  }
  
  qDebug() << "========== Starting RemoteApp ==========";
  qDebug() << "  Executable:" << m_executablePath;
  qDebug() << "  File:" << m_filePath;
  qDebug() << "  WorkDir:" << m_workingDirectory;
  qDebug() << "  ExpandWorkDir:" << m_expandEnvVarInWorkingDirectory;
  qDebug() << "  Arguments:" << m_arguments;
  qDebug() << "  ExpandArgs:" << m_expandEnvVarInArguments;
  qDebug() << "========================================";
  
  try {
    // 使用 ServerStartProgram 方法启动 RemoteApp
    // 参数：可执行文件路径、文件路径、工作目录、是否展开工作目录环境变量、参数、是否展开参数环境变量
    m_remoteProgram->dynamicCall(
        "ServerStartProgram(QString, QString, QString, bool, QString, bool)",
        m_executablePath,
        m_filePath,
        m_workingDirectory,
        m_expandEnvVarInWorkingDirectory,
        m_arguments,
        m_expandEnvVarInArguments
    );
    
    qDebug() << "ServerStartProgram called successfully";
    emit remoteAppStarted();
    
  } catch (...) {
    qCritical() << "Exception occurred while starting RemoteApp";
    emit remoteAppError(QString::fromUtf8("启动RemoteApp时发生异常"));
  }
}

// RemoteApp property setters
void RdpClient::setRemoteAppMode(bool enable) {
  if (m_remoteAppMode != enable) {
    m_remoteAppMode = enable;
    emit remoteAppModeChanged();
  }
}

void RdpClient::setExecutablePath(const QString &path) {
  if (m_executablePath != path) {
    m_executablePath = path;
    emit executablePathChanged();
  }
}

void RdpClient::setFilePath(const QString &path) {
  if (m_filePath != path) {
    m_filePath = path;
    emit filePathChanged();
  }
}

void RdpClient::setWorkingDirectory(const QString &dir) {
  if (m_workingDirectory != dir) {
    m_workingDirectory = dir;
    emit workingDirectoryChanged();
  }
}

void RdpClient::setExpandEnvVarInWorkingDirectory(bool expand) {
  if (m_expandEnvVarInWorkingDirectory != expand) {
    m_expandEnvVarInWorkingDirectory = expand;
    emit expandEnvVarInWorkingDirectoryChanged();
  }
}

void RdpClient::setArguments(const QString &args) {
  if (m_arguments != args) {
    m_arguments = args;
    emit argumentsChanged();
  }
}

void RdpClient::setExpandEnvVarInArguments(bool expand) {
  if (m_expandEnvVarInArguments != expand) {
    m_expandEnvVarInArguments = expand;
    emit expandEnvVarInArgumentsChanged();
  }
}


void RdpClient::__demo__() {
    qDebug() << "init";
    initializeControl();

    bool ok = connectToServer();
}
void RdpClient::onRemoteProgramResult(QString bstrExecutablePath, QVariant errorVariant, bool isExecutable) {
    int error = errorVariant.toInt();
    qDebug() << "========== RemoteApp 启动结果 ==========";
    qDebug() << "程序路径:" << bstrExecutablePath;
    qDebug() << "是否可执行:" << isExecutable;
    qDebug() << "原始错误码(Int):" << error;

    // 根据微软文档解析错误码 (RailResult 枚举)
    switch (error) {
    case 0: qDebug() << "结果: 成功 (RAIL_RESULT_NO_ERROR)"; break;
    case 1: qDebug() << "结果: 错误 - 参数校验失败"; break;
    case 2: qDebug() << "结果: 错误 - 无效的执行路径"; break;
    case 3: qDebug() << "结果: 错误 - 文件未找到 (通常是路径不对)"; break;
    case 4: qDebug() << "结果: 错误 - 内部错误"; break;
    case 5: qDebug() << "结果: 错误 - 拒绝访问 (通常是服务端注册表没开白名单)"; break;
    default: qDebug() << "结果: 未知错误"; break;
    }
}
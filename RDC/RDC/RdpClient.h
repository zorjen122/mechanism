#ifndef RDPCLIENT_H
#define RDPCLIENT_H

#include <QAxObject>
#include <QAxWidget>
#include <QObject>
#include <QWidget>

class RdpWindow;

class RdpClient : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
  Q_PROPERTY(
      QString username READ username WRITE setUsername NOTIFY usernameChanged)
  Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
  Q_PROPERTY(int desktopWidth READ desktopWidth WRITE setDesktopWidth NOTIFY
                 desktopWidthChanged)
  Q_PROPERTY(int desktopHeight READ desktopHeight WRITE setDesktopHeight NOTIFY
                 desktopHeightChanged)
  Q_PROPERTY(int colorDepth READ colorDepth WRITE setColorDepth NOTIFY
                 colorDepthChanged)
  Q_PROPERTY(QString fullScreenTitle READ fullScreenTitle WRITE
                 setFullScreenTitle NOTIFY fullScreenTitleChanged)
  Q_PROPERTY(bool fullScreen READ fullScreen WRITE setFullScreen NOTIFY
                 fullScreenChanged)
  Q_PROPERTY(bool enableSound READ enableSound WRITE setEnableSound NOTIFY
                 enableSoundChanged)
  Q_PROPERTY(bool enableClipboard READ enableClipboard WRITE setEnableClipboard
                 NOTIFY enableClipboardChanged)
  Q_PROPERTY(bool enablePrinter READ enablePrinter WRITE setEnablePrinter NOTIFY
                 enablePrinterChanged)
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
  
  // RemoteApp properties
  Q_PROPERTY(bool remoteAppMode READ remoteAppMode WRITE setRemoteAppMode NOTIFY
                 remoteAppModeChanged)
  Q_PROPERTY(QString executablePath READ executablePath WRITE setExecutablePath NOTIFY
                 executablePathChanged)
  Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY
                 filePathChanged)
  Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory NOTIFY
                 workingDirectoryChanged)
  Q_PROPERTY(bool expandEnvVarInWorkingDirectory READ expandEnvVarInWorkingDirectory 
                 WRITE setExpandEnvVarInWorkingDirectory NOTIFY
                 expandEnvVarInWorkingDirectoryChanged)
  Q_PROPERTY(QString arguments READ arguments WRITE setArguments NOTIFY
                 argumentsChanged)
  Q_PROPERTY(bool expandEnvVarInArguments READ expandEnvVarInArguments 
                 WRITE setExpandEnvVarInArguments NOTIFY
                 expandEnvVarInArgumentsChanged)

public:
  explicit RdpClient(QObject *parent = nullptr);
  ~RdpClient();

  // Property getters
  QString server() const { return m_server; }
  QString username() const { return m_username; }
  int port() const { return m_port; }
  int desktopWidth() const { return m_desktopWidth; }
  int desktopHeight() const { return m_desktopHeight; }
  int colorDepth() const { return m_colorDepth; }
  QString fullScreenTitle() const { return m_fullScreenTitle; }
  bool fullScreen() const { return m_fullScreen; }
  bool enableSound() const { return m_enableSound; }
  bool enableClipboard() const { return m_enableClipboard; }
  bool enablePrinter() const { return m_enablePrinter; }
  bool connected() const { return m_connected; }
  
  // RemoteApp getters
  bool remoteAppMode() const { return m_remoteAppMode; }
  QString executablePath() const { return m_executablePath; }
  QString filePath() const { return m_filePath; }
  QString workingDirectory() const { return m_workingDirectory; }
  bool expandEnvVarInWorkingDirectory() const { return m_expandEnvVarInWorkingDirectory; }
  QString arguments() const { return m_arguments; }
  bool expandEnvVarInArguments() const { return m_expandEnvVarInArguments; }

  // Property setters
  void setServer(const QString &server);
  void setUsername(const QString &username);
  void setPort(int port);
  void setDesktopWidth(int width);
  void setDesktopHeight(int height);
  void setColorDepth(int depth);
  void setFullScreenTitle(const QString &title);
  void setFullScreen(bool fullScreen);
  void setEnableSound(bool enable);
  void setEnableClipboard(bool enable);
  void setEnablePrinter(bool enable);
  
  // RemoteApp setters
  void setRemoteAppMode(bool enable);
  void setExecutablePath(const QString &path);
  void setFilePath(const QString &path);
  void setWorkingDirectory(const QString &dir);
  void setExpandEnvVarInWorkingDirectory(bool expand);
  void setArguments(const QString &args);
  void setExpandEnvVarInArguments(bool expand);

public slots:
  // RDP操作
  bool connectToServer();
  void disconnectFromServer();
  QWidget *getWidget();

signals:
  void serverChanged();
  void usernameChanged();
  void portChanged();
  void desktopWidthChanged();
  void desktopHeightChanged();
  void colorDepthChanged();
  void fullScreenTitleChanged();
  void fullScreenChanged();
  void enableSoundChanged();
  void enableClipboardChanged();
  void enablePrinterChanged();
  void connectedChanged();
  void connectionError(const QString &error);
  void connectionSuccess();
  
  // RemoteApp signals
  void remoteAppModeChanged();
  void executablePathChanged();
  void filePathChanged();
  void workingDirectoryChanged();
  void expandEnvVarInWorkingDirectoryChanged();
  void argumentsChanged();
  void expandEnvVarInArgumentsChanged();
  void remoteAppStarted();
  void remoteAppError(const QString &error);

private slots:
  void onConnected();
  void onDisconnected();
  void onLoginComplete();
  void onFatalError(int errorCode);

private:
  void initializeControl();
  void configureClient();
  void configureRemoteApp();
  void startRemoteApp();
  QAxBase *getRdpControl();
  void setRdpProperty(const char *name, const QVariant &value);

  QAxWidget *m_axWidget;
  QAxObject *m_rdpClient;
  RdpWindow *m_rdpWindow;

  QString m_server;
  QString m_username;
  int m_port;
  int m_desktopWidth;
  int m_desktopHeight;
  int m_colorDepth;
  QString m_fullScreenTitle;
  bool m_fullScreen;
  bool m_enableSound;
  bool m_enableClipboard;
  bool m_enablePrinter;
  bool m_connected;
  
  // RemoteApp members
  bool m_remoteAppMode;
  QString m_executablePath;
  QString m_filePath;
  QString m_workingDirectory;
  bool m_expandEnvVarInWorkingDirectory;
  QString m_arguments;
  bool m_expandEnvVarInArguments;
  QAxObject *m_remoteProgram;  // RemoteProgram 接口对象
};

#endif // RDPCLIENT_H

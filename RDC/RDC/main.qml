import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import RDC 1.0

Window {
    id: mainWindow
    visible: true
    width: 800
    height: 600
    title: "RDC - 远程桌面客户端"

    // RDP 客户端
    RdpClient {
        id: rdpClient
        
        onConnectionSuccess: {
            if (remoteAppMode) {
                statusText.text = "已连接到: " + server + "，等待启动应用..."
            } else {
                statusText.text = "已连接到: " + server
            }
            statusText.color = "green"
        }
        
        onConnectionError: {
            statusText.text = "连接错误: " + error
            statusText.color = "red"
            errorDialog.text = error
            errorDialog.open()
        }
        
        onConnectedChanged: {
            if (!connected) {
                statusText.text = "未连接"
                statusText.color = "#666666"
            }
        }
        
        onRemoteAppStarted: {
            statusText.text = "RemoteApp 已启动: " + executablePath
            statusText.color = "green"
        }
        
        onRemoteAppError: {
            statusText.text = "RemoteApp 错误: " + error
            statusText.color = "red"
            errorDialog.text = "RemoteApp 错误: " + error
            errorDialog.open()
        }
    }

    // 连接配置对话框
    ConnectionDialog {
        id: connectionDialog
        onAccepted: {
            console.log("连接配置: IP=" + ip + ", 端口=" + port + ", 用户名=" + username)
            console.log("分辨率=" + desktopWidth + "x" + desktopHeight + ", 色彩深度=" + colorDepth)
            console.log("全屏=" + fullScreen + ", 音频=" + enableSound + ", 剪贴板=" + enableClipboard)
            
            // 配置 RDP 客户端 - 基本设置
            rdpClient.server = ip
            rdpClient.port = port
            rdpClient.username = username
            
            // 配置 RDP 客户端 - 显示设置
            rdpClient.desktopWidth = desktopWidth
            rdpClient.desktopHeight = desktopHeight
            rdpClient.colorDepth = colorDepth
            rdpClient.fullScreen = fullScreen
            
            // 配置 RDP 客户端 - 本地资源
            rdpClient.enableSound = enableSound
            rdpClient.enableClipboard = enableClipboard
            rdpClient.enablePrinter = enablePrinter
            
            // 发起连接
            if (rdpClient.connectToServer()) {
                statusText.text = "正在连接到 " + ip + "..."
                statusText.color = "blue"
            }
        }
    }
    
    // RemoteApp 配置对话框
    RemoteAppDialog {
        id: remoteAppDialog
        onAccepted: {
            console.log("RemoteApp配置: IP=" + ip + ", 端口=" + port + ", 用户名=" + username)
            console.log("可执行文件=" + executablePath)
            console.log("文件路径=" + filePath)
            console.log("工作目录=" + workingDirectory + ", 展开环境变量=" + expandEnvVarInWorkingDirectory)
            console.log("参数=" + appArguments + ", 展开环境变量=" + expandEnvVarInArguments)
            
            // 配置 RDP 客户端 - 基本设置
            rdpClient.server = ip
            rdpClient.port = port
            rdpClient.username = username
            
            // 配置 RemoteApp 模式
            rdpClient.remoteAppMode = true
            rdpClient.executablePath = executablePath
            rdpClient.filePath = filePath
            rdpClient.workingDirectory = workingDirectory
            rdpClient.expandEnvVarInWorkingDirectory = expandEnvVarInWorkingDirectory
            rdpClient.arguments = appArguments
            rdpClient.expandEnvVarInArguments = expandEnvVarInArguments
            
            // 发起连接
            if (rdpClient.connectToServer()) {
                statusText.text = "正在连接到 " + ip + " 并启动应用..."
                statusText.color = "blue"
            }
        }
    }
    
    // 错误对话框
    Dialog {
        id: errorDialog
        title: "连接错误"
        modal: true
        standardButtons: Dialog.Ok
        property alias text: errorText.text
        
        Text {
            id: errorText
            wrapMode: Text.WordWrap
        }
    }

    // 主界面布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 菜单栏
        MenuBar {
            Layout.fillWidth: true
            
            Menu {
                title: "连接(&C)"
                
                MenuItem {
                    text: "启动(&S)"
                    onTriggered: {
                        connectionDialog.open()
                    }
                }
                
                MenuItem {
                    text: "启动应用(&A)"
                    onTriggered: {
                        remoteAppDialog.open()
                    }
                }
                
                MenuSeparator {}
                
                MenuItem {
                    text: "退出(&X)"
                    onTriggered: {
                        Qt.quit()
                    }
                }
            }
            
            Menu {
                title: "帮助(&H)"
                
                MenuItem {
                    text: "关于(&A)"
                    onTriggered: {
                        // TODO: 显示关于对话框
                    }
                }
            }
        }

        // 主内容区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f0f0f0"

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    font.pointSize: 16
                    color: "#666666"
                    text: "请点击菜单 [连接] -> [启动] 开始远程桌面连接"
                }

                Text {
                    id: statusText
                    Layout.alignment: Qt.AlignHCenter
                    font.pointSize: 12
                    color: "#666666"
                    text: "未连接"
                }
            }
        }
    }
}

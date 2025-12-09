import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: dialog
    title: "启动远程应用"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    width: 500
    height: 600
    
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    
    // 对外暴露的属性 - 连接设置
    property string ip: "192.168.10.9"
    property int port: 8848
    property string username: "Administrator"
    
    // RemoteApp 参数
    property string executablePath: "C:\\Windows\\explorer.exe"
    property string filePath: ""
    property string workingDirectory: ""
    property bool expandEnvVarInWorkingDirectory: false
    property string appArguments: ""
    property bool expandEnvVarInArguments: false
    
    onAccepted: {
        // 连接设置
        ip = ipField.text
        port = parseInt(portField.text)
        username = usernameField.text
        
        // RemoteApp 参数
        executablePath = executablePathField.text
        filePath = filePathField.text
        workingDirectory = workingDirectoryField.text
        expandEnvVarInWorkingDirectory = expandWorkDirCheck.checked
        appArguments = argumentsField.text
        expandEnvVarInArguments = expandArgsCheck.checked
    }
    
    onAboutToShow: {
        // 打开对话框时重置或保留上次的值
        ipField.text = ip || ""
        portField.text = port.toString() || "3389"
        usernameField.text = username || ""
        
        executablePathField.text = executablePath || ""
        filePathField.text = filePath || ""
        workingDirectoryField.text = workingDirectory || ""
        expandWorkDirCheck.checked = expandEnvVarInWorkingDirectory
        argumentsField.text = appArguments || ""
        expandArgsCheck.checked = expandEnvVarInArguments
        
        ipField.forceActiveFocus()
    }
    
    ScrollView {
        anchors.fill: parent
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: 12
            
            // 连接设置组
            GroupBox {
                Layout.fillWidth: true
                title: "连接设置"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    // IP地址输入
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Label {
                            text: "计算机(C):"
                            Layout.preferredWidth: 120
                        }
                        
                        TextField {
                            id: ipField
                            Layout.fillWidth: true
                            placeholderText: "输入IP地址或主机名"
                            selectByMouse: true
                        }
                    }
                    
                    // 端口输入
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Label {
                            text: "端口(P):"
                            Layout.preferredWidth: 120
                        }
                        
                        TextField {
                            id: portField
                            Layout.fillWidth: true
                            text: "3389"
                            placeholderText: "默认: 3389"
                            selectByMouse: true
                            validator: IntValidator {
                                bottom: 1
                                top: 65535
                            }
                        }
                    }
                    
                    // 用户名输入
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Label {
                            text: "用户名(U):"
                            Layout.preferredWidth: 120
                        }
                        
                        TextField {
                            id: usernameField
                            Layout.fillWidth: true
                            placeholderText: "输入用户名"
                            selectByMouse: true
                        }
                    }
                }
            }
            
            // RemoteApp 设置组
            GroupBox {
                Layout.fillWidth: true
                title: "应用程序设置"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    // 可执行文件路径
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Label {
                            text: "可执行文件路径(E):"
                            font.bold: true
                        }
                        
                        TextField {
                            id: executablePathField
                            Layout.fillWidth: true
                            placeholderText: "例如: C:\\Windows\\System32\\notepad.exe"
                            selectByMouse: true
                        }
                        
                        Label {
                            text: "远程服务器上应用程序的完整路径"
                            font.pointSize: 8
                            color: "#666666"
                        }
                    }
                    
                    // 文件路径
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Label {
                            text: "文件路径(F):"
                        }
                        
                        TextField {
                            id: filePathField
                            Layout.fillWidth: true
                            placeholderText: "例如: C:\\Documents\\file.txt (可选)"
                            selectByMouse: true
                        }
                        
                        Label {
                            text: "要用应用程序打开的文件路径(可选)"
                            font.pointSize: 8
                            color: "#666666"
                        }
                    }
                    
                    // 工作目录
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Label {
                            text: "工作目录(W):"
                        }
                        
                        TextField {
                            id: workingDirectoryField
                            Layout.fillWidth: true
                            placeholderText: "例如: C:\\Users\\Public (可选)"
                            selectByMouse: true
                        }
                        
                        CheckBox {
                            id: expandWorkDirCheck
                            text: "在服务器上展开环境变量"
                            checked: false
                        }
                        
                        Label {
                            text: "应用程序的工作目录(可选)"
                            font.pointSize: 8
                            color: "#666666"
                        }
                    }
                    
                    // 命令行参数
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        
                        Label {
                            text: "命令行参数(A):"
                        }
                        
                        TextField {
                            id: argumentsField
                            Layout.fillWidth: true
                            placeholderText: "例如: /p /s (可选)"
                            selectByMouse: true
                        }
                        
                        CheckBox {
                            id: expandArgsCheck
                            text: "在服务器上展开环境变量"
                            checked: false
                        }
                        
                        Label {
                            text: "传递给应用程序的命令行参数(可选)"
                            font.pointSize: 8
                            color: "#666666"
                        }
                    }
                }
            }
            
            // 提示信息
            Label {
                Layout.fillWidth: true
                Layout.topMargin: 5
                text: "提示: 密码将在连接时输入。所有路径均为远程服务器上的路径。"
                font.pointSize: 8
                color: "#666666"
                wrapMode: Text.WordWrap
            }
        }
    }
}

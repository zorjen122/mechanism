import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: dialog
    title: "远程桌面连接"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    width: 500
    height: 550
    
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    
    // 对外暴露的属性
    property string ip: ""
    property int port: 8848
    property string username: ""
    property int desktopWidth: 1920
    property int desktopHeight: 1080
    property int colorDepth: 32
    property bool fullScreen: false
    property bool enableSound: true
    property bool enableClipboard: true
    property bool enablePrinter: false
    
    onAccepted: {
        ip = ipField.text
        port = parseInt(portField.text)
        username = usernameField.text
        desktopWidth = parseInt(widthField.text)
        desktopHeight = parseInt(heightField.text)
        colorDepth = parseInt(colorDepthCombo.currentValue)
        fullScreen = fullScreenCheck.checked
        enableSound = soundCheck.checked
        enableClipboard = clipboardCheck.checked
        enablePrinter = printerCheck.checked
    }
    
    onAboutToShow: {
        // 打开对话框时重置或保留上次的值
        ipField.text = ip || "192.168.10.9"
        portField.text = port.toString() || "8899"
        usernameField.text = username || "Administrator"
        widthField.text = desktopWidth.toString() || "1920"
        heightField.text = desktopHeight.toString() || "1080"
        
        // 设置色彩深度下拉框
        for (var i = 0; i < colorDepthCombo.model.length; i++) {
            if (colorDepthCombo.model[i].value === colorDepth) {
                colorDepthCombo.currentIndex = i
                break
            }
        }
        
        fullScreenCheck.checked = fullScreen
        soundCheck.checked = enableSound
        clipboardCheck.checked = enableClipboard
        printerCheck.checked = enablePrinter
        
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
                            Layout.preferredWidth: 100
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
                            Layout.preferredWidth: 100
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
                            Layout.preferredWidth: 100
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
            
            // 显示设置组
            GroupBox {
                Layout.fillWidth: true
                title: "显示设置"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    
                    // 分辨率设置
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Label {
                            text: "分辨率:"
                            Layout.preferredWidth: 100
                        }
                        
                        TextField {
                            id: widthField
                            Layout.preferredWidth: 80
                            text: "1920"
                            selectByMouse: true
                            validator: IntValidator {
                                bottom: 640
                                top: 7680
                            }
                        }
                        
                        Label {
                            text: "×"
                        }
                        
                        TextField {
                            id: heightField
                            Layout.preferredWidth: 80
                            text: "1080"
                            selectByMouse: true
                            validator: IntValidator {
                                bottom: 480
                                top: 4320
                            }
                        }
                        
                        Item {
                            Layout.fillWidth: true
                        }
                    }
                    
                    // 色彩深度
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        Label {
                            text: "色彩深度:"
                            Layout.preferredWidth: 100
                        }
                        
                        ComboBox {
                            id: colorDepthCombo
                            Layout.fillWidth: true
                            model: [
                                { text: "增强色 (15位)", value: 15 },
                                { text: "高彩色 (16位)", value: 16 },
                                { text: "真彩色 (24位)", value: 24 },
                                { text: "最高质量 (32位)", value: 32 }
                            ]
                            textRole: "text"
                            valueRole: "value"
                            currentIndex: 3
                        }
                    }
                    
                    // 全屏模式
                    CheckBox {
                        id: fullScreenCheck
                        text: "全屏显示"
                        checked: false
                    }
                }
            }
            
            // 本地资源组
            GroupBox {
                Layout.fillWidth: true
                title: "本地资源"
                
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    
                    CheckBox {
                        id: soundCheck
                        text: "远程音频"
                        checked: true
                    }
                    
                    CheckBox {
                        id: clipboardCheck
                        text: "剪贴板"
                        checked: true
                    }
                    
                    CheckBox {
                        id: printerCheck
                        text: "打印机"
                        checked: false
                    }
                }
            }
            
            // 提示信息
            Label {
                Layout.fillWidth: true
                Layout.topMargin: 5
                text: "提示: 密码将在连接时输入"
                font.pointSize: 8
                color: "#666666"
                wrapMode: Text.WordWrap
            }
        }
    }
}

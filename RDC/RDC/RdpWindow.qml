import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import RDC 1.0

Window {
    id: rdpWindow
    title: "远程桌面 - " + serverName
    width: 1024
    height: 768
    visible: false
    
    property string serverName: ""
    property var rdpWidget: null
    
    signal closeRequested()
    
    onClosing: {
        closeRequested()
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // 工具栏
        ToolBar {
            Layout.fillWidth: true
            
            RowLayout {
                anchors.fill: parent
                
                Label {
                    text: "连接到: " + serverName
                    font.bold: true
                }
                
                Item {
                    Layout.fillWidth: true
                }
                
                ToolButton {
                    text: "断开连接"
                    onClicked: {
                        closeRequested()
                    }
                }
            }
        }
        
        // RDP 控件容器
        Rectangle {
            id: rdpContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000000"
            
            Text {
                anchors.centerIn: parent
                text: "正在加载远程桌面控件..."
                color: "white"
                font.pointSize: 12
                visible: rdpWidget === null
            }
        }
    }
    
    function embedRdpWidget(widget) {
        if (widget) {
            rdpWidget = widget
            // 注意：QAxWidget 需要通过 C++ 代码嵌入
            console.log("RDP widget ready to embed")
        }
    }
}

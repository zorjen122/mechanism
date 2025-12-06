#include "RdpWindow.h"
#include <QDebug>
#include <QHBoxLayout>


RdpWindow::RdpWindow(QWidget *parent) : QWidget(parent), m_rdpWidget(nullptr) {
  setupUI();
  setWindowTitle("远程桌面连接");
  resize(1024, 768);
}

RdpWindow::~RdpWindow() {
  qDebug() << "RdpWindow destructor called";
  // 移除控件但不删除（由 RdpClient 管理）
  if (m_rdpWidget) {
    m_containerLayout->removeWidget(m_rdpWidget);
    m_rdpWidget->setParent(nullptr);
    m_rdpWidget = nullptr;
  }
  qDebug() << "RdpWindow destructor finished";
}

void RdpWindow::setupUI() {
  m_mainLayout = new QVBoxLayout(this);
  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  m_mainLayout->setSpacing(0);

  // 创建工具栏
  m_toolBar = new QWidget(this);
  m_toolBar->setStyleSheet("QWidget { background-color: #f0f0f0; }");
  m_toolBar->setFixedHeight(40); // 固定高度，不拉伸

  QHBoxLayout *toolBarLayout = new QHBoxLayout(m_toolBar);

  m_serverLabel = new QLabel("连接到: ", m_toolBar);
  m_serverLabel->setStyleSheet("QLabel { font-weight: bold; padding: 5px; }");
  toolBarLayout->addWidget(m_serverLabel);

  toolBarLayout->addStretch();

  m_disconnectButton = new QPushButton("断开连接", m_toolBar);
  connect(m_disconnectButton, &QPushButton::clicked, this,
          &RdpWindow::disconnectRequested);
  toolBarLayout->addWidget(m_disconnectButton);

  m_mainLayout->addWidget(m_toolBar, 0); // 0 = 不拉伸

  // 创建 RDP 控件容器
  m_rdpContainer = new QWidget(this);
  m_rdpContainer->setStyleSheet("QWidget { background-color: #000000; }");
  m_containerLayout = new QVBoxLayout(m_rdpContainer);
  m_containerLayout->setContentsMargins(0, 0, 0, 0);

  m_mainLayout->addWidget(m_rdpContainer, 1); // 1 = 占据剩余空间
}

void RdpWindow::setRdpWidget(QAxWidget *widget) {
  // 移除旧的控件
  if (m_rdpWidget) {
    m_containerLayout->removeWidget(m_rdpWidget);
    m_rdpWidget->setParent(nullptr); // 解除父子关系
    m_rdpWidget = nullptr;
  }

  // 设置新的控件
  m_rdpWidget = widget;

  if (m_rdpWidget) {
    m_rdpWidget->setParent(m_rdpContainer);
    m_containerLayout->addWidget(m_rdpWidget);
    m_rdpWidget->show();
  }
}

void RdpWindow::setServerName(const QString &name) {
  m_serverName = name;
  m_serverLabel->setText(QString::fromUtf8("连接到: %1").arg(name));
  setWindowTitle(QString::fromUtf8("远程桌面 - %1").arg(name));
}

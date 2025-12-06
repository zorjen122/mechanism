#ifndef RDPWINDOW_H
#define RDPWINDOW_H

#include <QAxWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>


class RdpWindow : public QWidget {
  Q_OBJECT

public:
  explicit RdpWindow(QWidget *parent = nullptr);
  ~RdpWindow();

  void setRdpWidget(QAxWidget *widget);
  void setServerName(const QString &name);

signals:
  void disconnectRequested();

private:
  QVBoxLayout *m_mainLayout;
  QWidget *m_toolBar;
  QLabel *m_serverLabel;
  QPushButton *m_disconnectButton;
  QWidget *m_rdpContainer;
  QVBoxLayout *m_containerLayout;
  QAxWidget *m_rdpWidget;
  QString m_serverName;

  void setupUI();
};

#endif // RDPWINDOW_H

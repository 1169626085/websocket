#include "loginlog.h"
#include "ui_loginlog.h"

#include <QPushButton>

Loginlog::Loginlog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Loginlog)
{
    ui->setupUi(this);

    connect(ui->showRegisterButton, &QPushButton::clicked,
            this, &Loginlog::showRegisterRequested);
    initHttpHandlers();
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_login_mod_finish,this,&Loginlog::slot_login_mod_finish);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed, this, &Loginlog::slot_login_failed);
}

Loginlog::~Loginlog()
{
    delete ui;
}

void Loginlog::showTip(QString str, bool b_ok)
{
    if(b_ok){
        ui->titleLabel->setProperty("state","normal");
    }else{
        ui->titleLabel->setProperty("state","err");
    }
    ui->titleLabel->setText(str);

    repolish(ui->titleLabel);
}

void Loginlog::on_loginButton_clicked()
{
    qDebug()<<"login btn clicked";
    if(checkUserValid() == false){
        return;
    }

    if(checkPwdValid() == false){
        return ;
    }
    auto user = ui->accountLineEdit->text();
    auto pwd = ui->passwordLineEdit->text();
    //发送http请求登录
    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] =pwd;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(QString(gate_url_prefix)+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}

void Loginlog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());
    return;
}

void Loginlog::slot_tcp_con_finish(bool bsuccess)
{
    if(bsuccess){
        showTip(tr("聊天服务连接成功，正在登录..."),true);
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给chat server
        TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, jsonString);

    }else{
        showTip(tr("网络异常"),false);

    }


}
bool Loginlog::checkUserValid(){

    auto user = ui->accountLineEdit->text();
    if(user.isEmpty()){
        qDebug() << "User empty " ;
        return false;
    }

    return true;
}

bool Loginlog::checkPwdValid(){
    auto pwd = ui->passwordLineEdit->text();
    // if(pwd.length() < 6 || pwd.length() > 15){
    //     qDebug() << "Pass length invalid";
    //     return false;
    // }

    return true;
}

void Loginlog::initHttpHandlers()
{
  //注册获取登入回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER,[this](QJsonObject jsonObj){
      int error = jsonObj["error"].toInt();
      if(error != ErrorCodes::SUCCESS){
          showTip(tr("参数错误"),false);
          return;
      }
      auto user = jsonObj["user"].toString();
      ServerInfo si;
      si.Uid = jsonObj["uid"].toInt();
      si.Host = jsonObj["host"].toString();
      si.Port = jsonObj["port"].toString();
      si.Token = jsonObj["token"].toString();

      _uid = si.Uid;
      _token = si.Token;
      qDebug()<< "user is " << user << " uid is " << si.Uid <<" host is "
               << si.Host << " Port is " << si.Port << " Token is " << si.Token;
      emit sig_connect_tcp(si);
      showTip(tr("登录成功"), true);

  });
}


void Loginlog::slot_login_failed(int err)
{
    QString result = QString("登录失败, err is %1")
                         .arg(err);
    showTip(result,false);

}

#include "registerlog.h"
#include "ui_registerlog.h"

#include <QPushButton>

Registerlog::Registerlog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Registerlog)
{
    ui->setupUi(this);
    initHttpHandlers();
    ui->titleLabel->setProperty("state","normal");
    repolish(ui->titleLabel);

    connect(ui->backToLoginButton, &QPushButton::clicked,
            this, &Registerlog::backToLoginRequested);
    connect(HttpMgr::GetInstance().get(),&HttpMgr::sig_reg_mod_finish,this,&Registerlog::slot_reg_mod_finish);
}

Registerlog::~Registerlog()
{
    delete ui;
}

void Registerlog::showTip(QString str,bool b_ok)
{
    if(b_ok){
        ui->titleLabel->setProperty("state","normal");
    }else{
        ui->titleLabel->setProperty("state","err");
    }
    ui->titleLabel->setText(str);

    repolish(ui->titleLabel);
}

void Registerlog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{    if(err != ErrorCodes::SUCCESS){
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
    QJsonObject jsonObj = jsonDoc.object();
    //调用对应的逻辑
    _handlers[id](jsonDoc.object());
    return;

}

void Registerlog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,[this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送到邮箱，注意查收"), true);
        qDebug()<< "email is " << email ;
    });

    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug()<< "email is " << email ;
    });
}

void Registerlog::on_registerButton_clicked()
{
    if(ui->accountLineEdit->text() == ""){
        showTip(tr("用户名不能为空"), false);
        return;
    }

    if(ui->emailLineEdit->text() == ""){
        showTip(tr("邮箱不能为空"), false);
        return;
    }

    if(ui->passwordLineEdit->text() == ""){
        showTip(tr("密码不能为空"), false);
        return;
    }

    if(ui->confirmPasswordLineEdit->text() == ""){
        showTip(tr("确认密码不能为空"), false);
        return;
    }

    if(ui->confirmPasswordLineEdit->text() != ui->passwordLineEdit->text()){
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }

    if(ui->varifyCodeLineEdit->text() == ""){
        showTip(tr("验证码不能为空"), false);
        return;
    }

    //day11 发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->accountLineEdit->text();
    json_obj["email"] = ui->emailLineEdit->text();
    json_obj["passwd"] = ui->passwordLineEdit->text();
    json_obj["confirm"] = ui->confirmPasswordLineEdit->text();
    json_obj["varifycode"] = ui->varifyCodeLineEdit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl("http://localhost:8080/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
}

void Registerlog::on_getVarifyCodeButton_clicked()
{
    auto email=ui->emailLineEdit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match =regex.match(email).hasMatch();
    if(match){
        QJsonObject json_obj;
        json_obj["email"]=email;
        HttpMgr::GetInstance()->PostHttpReq(QUrl("http://localhost:8080/get_varifycode"),
                json_obj,ReqId::ID_GET_VARIFY_CODE,Modules::REGISTERMOD);
    }else{
        //邮箱不正确
        showTip(tr("邮箱不正确"),false);

    }

}

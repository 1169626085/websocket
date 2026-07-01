#include "tcpmgr.h"
#include <QJsonArray>

TcpMgr::TcpMgr():_host(""),_port(0),_b_recv_pending(false),_message_id(0),_message_len(0) {
    qRegisterMetaType<std::shared_ptr<SearchInfo>>("std::shared_ptr<SearchInfo>");
    qRegisterMetaType<QVector<std::shared_ptr<SearchInfo>>>("QVector<std::shared_ptr<SearchInfo>>");
    qRegisterMetaType<std::shared_ptr<FriendApplyInfo>>("std::shared_ptr<FriendApplyInfo>");

    QObject::connect(&_socket,&QTcpSocket::connected,[&](){
    qDebug()<<"Connected to server!";
    emit sig_con_success(true);
    });
    QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {
           // 当有数据可读时，读取所有数据
           // 读取所有数据并追加到缓冲区
        _buffer.append(_socket.readAll());

        forever{
            if(!_b_recv_pending){
                if(_buffer.size() < static_cast<int> (sizeof(quint16) + sizeof(quint32))){
                    return;
                }
                QDataStream stream(_buffer.left(sizeof(quint16) + sizeof(quint32)));
                stream.setByteOrder(QDataStream::BigEndian);
                stream.setVersion(QDataStream::Qt_5_0);
                quint32 messageLen = 0;
                stream >> _message_id >> messageLen;
                _message_len = messageLen;

                //将buffer 中的前四个字节移除
                _buffer = _buffer.mid(sizeof(quint16) + sizeof(quint32));

                // 输出读取的数据
                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_len;
            }

            //
            if(_buffer.size() < static_cast<int>(_message_len)){
                _b_recv_pending = true;
                return;
            }

            _b_recv_pending = false;
            // 读取消息体
            QByteArray messageBody = _buffer.mid(0, static_cast<int>(_message_len));
            qDebug() << "receive body msg is " << messageBody ;
            handleMSg(static_cast<ReqId>(_message_id), static_cast<int>(_message_len), messageBody);

            _buffer = _buffer.mid(static_cast<int>(_message_len));
        }
        });
        QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
           qDebug() << "Error:" << _socket.errorString();
           emit sig_con_success(false);
       });

    QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server.";
    });

    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
    //连接tcp管理者发出的登陆失败信号
    // 直接使用返回的指针

    initHandlers();

}

void TcpMgr::initHandlers()
{
    _handlers.insert(ID_CHAT_LOGIN_RSP,[this](ReqId id,int len,QByteArray data){
        Q_UNUSED(len);
        qDebug()<< "handle id is"<<id<<"data is"<<data;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        //检查是否转换成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }
        QJsonObject jsonObj=jsonDoc.object();
        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }
        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }
        UserMgr::GetInstance()->SetUid(jsonObj["uid"].toInt());
        UserMgr::GetInstance()->SetName(jsonObj["name"].toString());
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
        emit sig_swich_chatdlg();

    });
    _handlers.insert(ID_SEARCH_USER_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug()<< "handle id is "<< id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }

        QVector<std::shared_ptr<SearchInfo>> results;
        if (jsonObj.contains("users") && jsonObj["users"].isArray()) {
            const auto users = jsonObj["users"].toArray();
            for (const auto& item : users) {
                const auto obj = item.toObject();
                auto search_info = std::make_shared<SearchInfo>(obj["uid"].toInt(),
                                                                obj["name"].toString(),
                                                                obj["email"].toString(),
                                                                obj["nick"].toString(),
                                                                obj["desc"].toString(),
                                                                obj["sex"].toInt(),
                                                                obj["icon"].toString(),
                                                                obj["is_friend"].toBool());
                results.push_back(search_info);
            }
        } else if (jsonObj.contains("uid")) {
            auto search_info = std::make_shared<SearchInfo>(jsonObj["uid"].toInt(),
                                                            jsonObj["name"].toString(),
                                                            jsonObj["email"].toString(),
                                                            jsonObj["nick"].toString(),
                                                            jsonObj["desc"].toString(),
                                                            jsonObj["sex"].toInt(),
                                                            jsonObj["icon"].toString(),
                                                            jsonObj["is_friend"].toBool());
            results.push_back(search_info);
        }

        emit sig_user_search(results);
    });

    _handlers.insert(ID_ADD_FRIEND_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            emit sig_add_friend_result(ErrorCodes::ERR_JSON, 0, QString());
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        emit sig_add_friend_result(jsonObj["error"].toInt(),
                                   jsonObj["to_uid"].toInt(),
                                   jsonObj["to_name"].toString());
    });

    _handlers.insert(ID_NOTIFY_ADD_FRIEND_REQ, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        auto apply = std::make_shared<FriendApplyInfo>(jsonObj["from_uid"].toInt(),
                                                       jsonObj["to_uid"].toInt(),
                                                       jsonObj["from_name"].toString(),
                                                       jsonObj["message"].toString());
        emit sig_friend_apply_notify(apply);
    });

    _handlers.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            emit sig_auth_friend_result(ErrorCodes::ERR_JSON, 0);
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        emit sig_auth_friend_result(jsonObj["error"].toInt(), jsonObj["from_uid"].toInt());
    });

    _handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data){
        Q_UNUSED(id);
        Q_UNUSED(len);
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            return;
        }
        QJsonObject jsonObj = jsonDoc.object();
        emit sig_friend_auth_notify(jsonObj["from_uid"].toInt(),
                                    jsonObj["to_uid"].toInt(),
                                    jsonObj["from_name"].toString());
    });
}

void TcpMgr::handleMSg(ReqId id, int len, QByteArray data)
{
    auto find_iter = _handlers.find(id);
    if(find_iter == _handlers.end()){
        qDebug()<< "not found id ["<< id << "] to handle";
        return ;
    }

    find_iter.value()(id,len,data);

}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";
    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toUInt());
    _socket.connectToHost(si.Host, _port);

}
void TcpMgr::slot_send_data(ReqId reqId, QString data)
{
    uint16_t id = reqId;

    // 将字符串转换为UTF-8编码的字节数组
    QByteArray dataBytes = data.toUtf8();

    // 计算长度（使用网络字节序转换）
    quint32 len = static_cast<quint32>(dataBytes.size());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    // 添加字符串数据
    block.append(data.toUtf8());

    // 发送数据
    _socket.write(block);
}

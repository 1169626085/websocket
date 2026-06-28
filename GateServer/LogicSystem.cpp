#include "LogicSystem.h"
#include "HttpConnection.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
    auto iter = get_handlers_.find(path);
    if (iter == get_handlers_.end()) {
        return false;
    }

    iter->second(con);
    return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    if (post_handlers_.find(path) == post_handlers_.end()) {
        return false;
    }

    post_handlers_[path](con);
    return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
    get_handlers_.insert(std::make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
    post_handlers_.insert(make_pair(url,handler));
}

LogicSystem::LogicSystem()
{
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req";
        int i = 0;
        for (auto& elem : connection->get_params_) {
            i++;
            beast::ostream(connection->response_.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(connection->response_.body()) << ", " <<  " value is " << elem.second << std::endl;
        }
    });





    RegPost("/get_varifycode",[](std::shared_ptr<HttpConnection> connection){
        auto body_str=boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is" << body_str << std::endl;
        connection->response_.set(http::field::content_type,"text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str,src_root);
        if(!parse_success){
            std::cout << "Failed to parse Json data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->response_.body())<< jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
        std::cout<< "email is " << email << std::endl;
        root["error"]=0;
        root["email"]=src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    });




    RegPost("/user_register",[](std::shared_ptr<HttpConnection> connection){
        auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
        std::cout << "receive body is" << body_str <<std::endl;
        connection->response_.set(http::field::content_type,"text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
        std::cout << "Failed to parse JSON data!" << std::endl;
        root["error"] = ErrorCodes::Error_Json;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }

    
    auto email = src_root["email"].asString();
    auto name = src_root["user"].asString();
    auto pwd = src_root["passwd"].asString();
    auto confirm = src_root["confirm"].asString();

    if (pwd != confirm) {
        std::cout << "password err " << std::endl;
        root["error"] = ErrorCodes::PasswdErr;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }
    //先查找redis中email对应的验证码是否合理
    std::string varify_code;
    bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
    if (!b_get_varify) {
        std::cout << " get varify code expired" << std::endl;
        root["error"] = ErrorCodes::VarifyExpired;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }
    if (varify_code != src_root["varifycode"].asString()) {
        std::cout << " varify code error" << std::endl;
        root["error"] = ErrorCodes::VarifyCodeErr;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }
    int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
    if (uid == 0 || uid == -1) {
        std::cout << " user or email exist" << std::endl;
        root["error"] = ErrorCodes::UserExist;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }
    root["error"] = 0;
    root["uid"] = uid;
    root["email"] = email;
    root ["user"]= name;
    root["passwd"] = pwd;
    root["confirm"] = confirm;
    root["varifycode"] = src_root["varifycode"].asString();
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return true;
        
    });

    RegPost("/user_login",[](std::shared_ptr<HttpConnection> connection){
            auto body_str = boost::beast::buffers_to_string(connection->request_.body().data());
    std::cout << "receive body is " << body_str << std::endl;
    connection->response_.set(http::field::content_type, "text/json");
    Json::Value root;
    Json::Reader reader;
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
        std::cout << "Failed to parse JSON data!" << std::endl;
        root["error"] = ErrorCodes::Error_Json;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }
    auto name = src_root["user"].asString();
    auto pwd = src_root["passwd"].asString();
    UserInfo userInfo;
    //查询数据库判断用户名和密码是否匹配
    bool pwd_valid = MysqlMgr::GetInstance()->CheckPwd(name, pwd, userInfo);
    if (!pwd_valid) {
        std::cout << " user pwd not match" << std::endl;
        root["error"] = ErrorCodes::PasswdInvalid;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }
        //查询StatusServer找到合适的连接
    auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
    if (reply.error()) {
        std::cout << " grpc get chat server failed, error is " << reply.error()<< std::endl;
        root["error"] = ErrorCodes::RPCGetFailed;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->response_.body()) << jsonstr;
        return true;
    }

    std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
    root["error"] = 0;
    root["user"] = name;
    root["uid"] = userInfo.uid;
    root["token"] = reply.token();
    root["host"] = reply.host();
    root["port"]=reply.port();
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return true;
    });
}

LogicSystem::~LogicSystem() = default;

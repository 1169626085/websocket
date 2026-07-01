#pragma once
#include "const.h"
#include <array>

class CServer;

class MsgNode {
public:
    MsgNode(int total_len, int msg_id = 0)
        : _total_len(total_len), _cur_len(0), _msg_id(msg_id), _data(new char[total_len + 1]()) {}
    virtual ~MsgNode() { delete[] _data; }
    void Clear() {
        ::memset(_data, 0, _total_len + 1);
        _cur_len = 0;
    }

    int _total_len;
    int _cur_len;
    int _msg_id;
    char* _data;
};

class RecvNode : public MsgNode {
public:
    RecvNode(int total_len, int msg_id) : MsgNode(total_len, msg_id) {}
};

class CSession: public std::enable_shared_from_this<CSession>{
public:
    CSession(boost::asio::io_context& io_context, CServer* server);
    void Start();
    void Send(const std::string& msg, short msg_id);
    void Close();
    tcp::socket& GetSocket();
    int GetUuid() const;
    void AsyncReadHead(int total_len);
    void asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void asyncReadLen(std::size_t read_len, std::size_t total_len, 
    std::function<void(const boost::system::error_code&, std::size_t)> handler);
    void AsyncReadBody(int total_len);
    void SetUserId(int uid);
    int GetUserId() const;

private:
    tcp::socket _socket;
    CServer* _server;
    int _uuid;
    int _user_id;
    bool _closed;
    char _data[MAX_LENGTH + 1];
    std::shared_ptr<MsgNode> _recv_head_node;
    std::shared_ptr<RecvNode> _recv_msg_node;
};

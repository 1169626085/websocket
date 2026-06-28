#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"

static std::atomic<int> session_uuid{1};

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
    : _socket(io_context),
      _server(server),
      _uuid(session_uuid++),
      _closed(false),
      _recv_head_node(std::make_shared<MsgNode>(HEAD_TOTAL_LEN))
{
}

void CSession::Start()
{
     AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::AsyncReadHead(int total_len)
{
    auto self = shared_from_this();
    asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
        try {
            if (ec) {
                std::cout << "handle read failed, error is " << ec.message() << endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            if (bytes_transfered < HEAD_TOTAL_LEN) {
                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
                    << HEAD_TOTAL_LEN << "]" << endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            _recv_head_node->Clear();
            memcpy(_recv_head_node->_data, _data, bytes_transfered);

            //获取头部MSGID数据
            short msg_id = 0;
            memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
            //网络字节序转化为本地字节序
            msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
            std::cout << "msg_id is " << msg_id << endl;
            //id非法
            if (msg_id > MAX_LENGTH) {
                std::cout << "invalid msg_id is " << msg_id << endl;
                _server->ClearSession(_uuid);
                return;
            }
            int msg_len = 0;
            memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
            //网络字节序转化为本地字节序
            msg_len = boost::asio::detail::socket_ops::network_to_host_long(msg_len);
            std::cout << "msg_len is " << msg_len << endl;

            //id非法
            if (msg_len > MAX_LENGTH) {
                std::cout << "invalid data length is " << msg_len << endl;
                _server->ClearSession(_uuid);
                return;
            }

            _recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
            AsyncReadBody(msg_len);
        }
        catch (std::exception& e) {
            std::cout << "Exception code is " << e.what() << endl;
        }
        });
}

void CSession::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code &, std::size_t)> handler)
{
    ::memset(_data, 0, MAX_LENGTH);
    asyncReadLen(0, maxLength, handler);
}

//读取指定字节数
void CSession::asyncReadLen(std::size_t read_len, std::size_t total_len, 
    std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
    auto self = shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data + read_len, total_len-read_len),
        [read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
            if (ec) {
                // 出现错误，调用回调函数
                handler(ec, read_len + bytesTransfered);
                return;
            }

            if (read_len + bytesTransfered >= total_len) {
                //长度够了就调用回调函数
                handler(ec, read_len + bytesTransfered);
                return;
            }

            // 没有错误，且长度不足则继续读取
            self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
    });
}

void CSession::AsyncReadBody(int total_len)
{
    auto self = shared_from_this();
    asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
        try {
            if (ec) {
                std::cout << "handle read failed, error is " << ec.message() << endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            if (bytes_transfered < total_len) {
                std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
                    << total_len<<"]" << endl;
                Close();
                _server->ClearSession(_uuid);
                return;
            }

            memcpy(_recv_msg_node->_data , _data , bytes_transfered);
            _recv_msg_node->_cur_len += bytes_transfered;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            cout << "receive data is " << _recv_msg_node->_data << endl;
            //此处将消息投递到逻辑队列中
            LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
            //继续监听头部接受事件
            AsyncReadHead(HEAD_TOTAL_LEN);
        }
        catch (std::exception& e) {
            std::cout << "Exception code is " << e.what() << endl;
        }
        });
}

void CSession::Send(const std::string& msg, short msg_id)
{
    short net_msg_id = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    int msg_len = static_cast<int>(msg.size());
    int net_msg_len = boost::asio::detail::socket_ops::host_to_network_long(msg_len);
    auto send_data = std::make_shared<std::string>();
    send_data->resize(HEAD_TOTAL_LEN + msg.size());
    memcpy(&(*send_data)[0], &net_msg_id, HEAD_ID_LEN);
    memcpy(&(*send_data)[HEAD_ID_LEN], &net_msg_len, HEAD_DATA_LEN);
    memcpy(&(*send_data)[HEAD_TOTAL_LEN], msg.data(), msg.size());
    boost::asio::async_write(_socket, boost::asio::buffer(*send_data),
        [self = shared_from_this(), send_data](const boost::system::error_code&, std::size_t) {});
}

void CSession::Close()
{
    if (_closed) {
        return;
    }
    _closed = true;
    boost::system::error_code ec;
    _socket.close(ec);
}

tcp::socket& CSession::GetSocket()
{
    return _socket;
}

int CSession::GetUuid() const
{
    return _uuid;
}

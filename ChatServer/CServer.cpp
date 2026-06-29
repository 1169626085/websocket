#include "CServer.h"
#include "AsioIOServicePool.h"


CServer::CServer(boost::asio::io_context & io, unsigned short port):_io_context(io),_port(port),_acceptor(io,boost::asio::ip::tcp::endpoint(
    boost::asio::ip::tcp::v4(),port))
{
    cout << "Server start success, listen on port : " << _port << endl;
    StartAccept();
}

void CServer::StartAccept() {
    auto &io_context = AsioIoServicePool::GetInstance()->GetIOService();
    shared_ptr<CSession> new_session = make_shared<CSession>(io_context, this);
    _acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, placeholders::_1));
}


void CServer::HandleAccept(shared_ptr<CSession> new_session, const boost::system::error_code &error)
{
    if(!error){
        new_session ->Start();
        lock_guard<mutex> lock(_mutex);
        _sessions.insert(make_pair(new_session->GetUuid(), new_session));
    }
    else {
        cout << "session accept failed, error is " << error.what() << endl;
    }
      StartAccept();
}

CServer::~CServer() = default;

void CServer::ClearSession(int uuid)
{
    lock_guard<mutex> lock(_mutex);
    _sessions.erase(uuid);
}
void CServer::ClearSession(std::string session_id)
{
    if(_sessions.find(session_id)!=_sessions.end()){
        UserMgr::GetInstance()->RmvUserSession(_sessions[session_id]->GetUserId());
    }
    {
        lock_guard<mutex> lock(_mutex);
        _sessions.erase(session_id);
    }
}
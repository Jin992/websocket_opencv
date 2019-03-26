#include <set>+
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class base_server {
public:
    base_server() : is_connected(false) {
        m_server.init_asio();
        m_server.set_open_handler(bind(&base_server::on_open,this,::_1));
        m_server.set_close_handler(bind(&base_server::on_close,this,::_1));
        m_server.set_validate_handler(bind(&base_server::on_validate,this,::_1));
        m_server.set_message_handler(bind(&base_server::on_message,this, ::_1,::_2));
    }

    void on_open(connection_hdl hdl) {
        is_connected = true;
        for (int i = 0; i < 15; i++) {
            m_server.send(hdl, &i, 4, websocketpp::frame::opcode::binary);
        }
    }

    void on_close(connection_hdl hdl) {
        is_connected = false;
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {

    }

    bool on_validate(connection_hdl hdl) {
        if (is_connected) {
            return false;
        }
        return true;
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }
private:
    server m_server;
    bool is_connected;

};

int main() {
    base_server server;
    server.run(9002);
}

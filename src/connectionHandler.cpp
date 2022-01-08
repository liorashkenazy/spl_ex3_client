#include <connectionHandler.h>
#include <messageEncoderDecoder.h>

using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
 
ConnectionHandler::ConnectionHandler(string host, short port): host_(host), port_(port), io_service_(),
                                                                socket_(io_service_), should_terminate(false),
                                                                send_buffer(), receive_buffer(){}
    
ConnectionHandler::~ConnectionHandler() {
    close();
}
 
bool ConnectionHandler::connect() {
    std::cout << "Starting connect to " 
        << host_ << ":" << port_ << std::endl;
    try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp ) {
			tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);			
        }
		if(error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
	boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp ) {
			tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
		if (error)
			throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "send failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}
 
bool ConnectionHandler::getMsg(std::string& line) {
    receive_buffer.clear();
    if (!getNextMessage(';')) {
        return false;
    }
    messageEncoderDecoder::decode(receive_buffer.data(), receive_buffer.size(), line, &should_terminate);
    return true;
}

bool ConnectionHandler::sendMsg(std::string& line) {
    send_buffer.clear();
    // If this is not a valid action, don't send anything to the server
    if (!messageEncoderDecoder::encode(line, send_buffer)) {
        return true;
    }
    return sendBytes(send_buffer.data(), send_buffer.size());
}
 
bool ConnectionHandler::getNextMessage(char delimiter) {
    // Stop when we encounter the null character. 
    // Notice that the null character is not appended to the frame string.
    char ch;
    do {
        if (!getBytes(&ch, 1)) {
            return false;
        }
        receive_buffer.push_back(ch);
    } while (delimiter != ch);
    return true;
}

// Close down the connection properly.
void ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}

bool ConnectionHandler::shouldTerminate() {
    return should_terminate;
}

#include <stdlib.h>
#include <connectionHandler.h>
#include <thread>

void receiveForever(ConnectionHandler *conn) {
    std::cout << "Starting to receive messages" << std::endl;
    while (!conn->shouldTerminate()) {
        std::string answer;
        if (!conn->getMsg(answer)) {
            break;
        }
        std::cout << "Reply: " << answer << std::endl;
    }
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    std::thread receiver_thread(receiveForever, &connectionHandler);

	//From here we will see the rest of the ehco client implementation:
    while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
		std::string message(buf);
        if (!connectionHandler.sendMsg(message)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }
        if (message.substr(0, message.find(' ')).compare("LOGOUT") == 0) {
            break;
        }
    }

    receiver_thread.join();
    return 0;
}

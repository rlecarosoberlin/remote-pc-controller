#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <termios.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Turn off echo and canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

class NetworkTest {
private:
    int server_socket;
    int client_socket;
    bool is_server;
    std::string server_ip;
    int port;

public:
    NetworkTest() : server_socket(-1), client_socket(-1), is_server(false), port(8080) {}

    bool startServer() {
        server_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_socket == -1) {
            std::cerr << "Failed to create server socket" << std::endl;
            return false;
        }

        // Set socket option to reuse address
        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            return false;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to bind server socket" << std::endl;
            return false;
        }

        if (listen(server_socket, 1) < 0) {
            std::cerr << "Failed to listen on server socket" << std::endl;
            return false;
        }

        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Waiting for client connection..." << std::endl;

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            std::cerr << "Failed to accept client connection" << std::endl;
            return false;
        }

        std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << std::endl;
        is_server = true;
        return true;
    }

    bool connectToServer(const std::string& ip) {
        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_socket == -1) {
            std::cerr << "Failed to create client socket" << std::endl;
            return false;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            return false;
        }

        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }

        std::cout << "Connected to server at " << ip << ":" << port << std::endl;
        is_server = false;
        return true;
    }

    void sendMessage(const std::string& message) {
        int socket_to_use = client_socket;
        if (send(socket_to_use, message.c_str(), message.length(), 0) < 0) {
            std::cerr << "Failed to send message" << std::endl;
        } else {
            std::cout << "Sent: " << message << std::endl;
        }
    }

    std::string receiveMessage() {
        char buffer[1024] = {0};
        int socket_to_use = is_server ? client_socket : client_socket;
        int bytes_received = recv(socket_to_use, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            std::string message(buffer);
            std::cout << "Received: " << message << std::endl;
            return message;
        } else if (bytes_received == 0) {
            std::cout << "Connection closed by peer" << std::endl;
            return "";
        } else {
            std::cerr << "Failed to receive message" << std::endl;
            return "";
        }
    }

    void runServer() {
        if (!startServer()) {
            return;
        }

        std::cout << "\nServer is running. Type messages to send to client (type 'quit' to exit):" << std::endl;
        
        // Start receiving thread
        // Create a thread using networkTest class as a lambda function
        std::thread receive_thread([this]() {
            while (true) {
                std::string received = receiveMessage();
                if (received.empty()) break;
            }
        });

        // Send messages in main thread
        enableRawMode();
        char c;
        while (true) {
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if (n == 1) {
                if (c == 3) break; // Ctrl-C to quit
                sendMessage(std::string(1, c));
            }
        }
        disableRawMode();

        receive_thread.join();
        cleanup();
    }

    void runClient(const std::string& server_ip) {
        if (!connectToServer(server_ip)) {
            return;
        }

        std::cout << "\nClient is running. Type messages to send to server (type 'quit' to exit):" << std::endl;
        
        // Start receiving thread
        std::thread receive_thread([this]() {
            while (true) {
                std::string received = receiveMessage();
                if (received.empty()) break;
            }
        });

        // Send messages in main thread
        enableRawMode();
        char c;
        while (true) {
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if (n == 1) {
                if (c == 3) break; // Ctrl-C to quit
                sendMessage(std::string(1, c));
            }
        }
        disableRawMode();

        receive_thread.join();
        cleanup();
    }

    void cleanup() {
        if (client_socket != -1) {
            close(client_socket);
        }
        if (server_socket != -1) {
            close(server_socket);
        }
    }

    ~NetworkTest() {
        cleanup();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  To run as server: " << argv[0] << " server" << std::endl;
        std::cout << "  To run as client: " << argv[0] << " client <server_ip>" << std::endl;
        std::cout << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "  Server: " << argv[0] << " server" << std::endl;
        std::cout << "  Client: " << argv[0] << " client 192.168.1.100" << std::endl;
        return 1;
    }

    NetworkTest network_test;
    std::string mode = argv[1];

    if (mode == "server") {
        network_test.runServer();
    } else if (mode == "client") {
        if (argc < 3) {
            std::cerr << "Client mode requires server IP address" << std::endl;
            return 1;
        }
        std::string server_ip = argv[2];
        network_test.runClient(server_ip);
    } else {
        std::cerr << "Invalid mode. Use 'server' or 'client'" << std::endl;
        return 1;
    }

    return 0;
}

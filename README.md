# Network Test Application

A simple C++ network test application that allows you to send messages between two machines over a TCP connection. This is a great starting point for learning network programming basics.

## Features

- **Server Mode**: Listens for incoming connections on port 8080
- **Client Mode**: Connects to a server at a specified IP address
- **Bidirectional Communication**: Send and receive messages simultaneously
- **Real-time Messaging**: Uses threads to handle sending and receiving concurrently

## Prerequisites

- Linux system (tested on Arch Linux)
- GCC compiler with C++11 support
- Make utility

## Compilation

Compile the application using the provided Makefile:

```bash
make
```

This will create an executable called `network_test`.

## Usage

### Running as Server

On the machine that will act as the server:

```bash
./network_test server
```

Or using make:

```bash
make run-server
```

The server will:
1. Start listening on port 8080
2. Wait for a client to connect
3. Display the client's IP address when connected
4. Allow you to send messages to the client

### Running as Client

On the machine that will act as the client:

```bash
./network_test client <server_ip>
```

Or using make:

```bash
make run-client SERVER_IP=192.168.1.100
```

Replace `192.168.1.100` with the actual IP address of your server machine.

### Finding Your IP Address

To find your machine's IP address, use:

```bash
ip addr show
```

Or:

```bash
hostname -I
```

Look for your local network IP (usually starts with `192.168.` or `10.0.`).

## How to Test

1. **On Machine A (Server)**:
   ```bash
   make
   ./network_test server
   ```

2. **On Machine B (Client)**:
   ```bash
   make
   ./network_test client <Machine_A_IP>
   ```

3. **Test Communication**:
   - Type messages on either machine and press Enter
   - Messages will be sent to the other machine
   - Type `quit` to exit

## Example Session

**Server side:**
```
Server started on port 8080
Waiting for client connection...
Client connected from 192.168.1.101

Server is running. Type messages to send to client (type 'quit' to exit):
Hello from server!
Sent: Hello from server!
Received: Hello from client!
```

**Client side:**
```
Connected to server at 192.168.1.100:8080

Client is running. Type messages to send to server (type 'quit' to exit):
Hello from client!
Sent: Hello from client!
Received: Hello from server!
```

## Troubleshooting

### Connection Refused
- Make sure the server is running before starting the client
- Check if port 8080 is not blocked by firewall
- Verify the IP address is correct

### Permission Denied
- Make sure the executable has proper permissions:
  ```bash
  chmod +x network_test
  ```

### Compilation Errors
- Ensure you have GCC installed:
  ```bash
  gcc --version
  ```
- Install if needed:
  ```bash
  sudo pacman -S gcc  # For Arch Linux
  ```

## Next Steps

This basic network test provides the foundation for your remote PC controller project. Once you're comfortable with this, you can:

1. **Add Screen Sharing**: Implement screen capture and video streaming
2. **Add Input Handling**: Capture mouse and keyboard events
3. **Improve Protocol**: Add message framing, compression, and error handling
4. **Add Security**: Implement encryption and authentication
5. **Add GUI**: Create a graphical interface for easier use

## Code Structure

The application uses:
- **TCP Sockets**: For reliable, ordered communication
- **Threading**: For concurrent send/receive operations
- **C++11**: For modern C++ features like lambda functions
- **POSIX Socket API**: For cross-platform network programming

## Port Configuration

The default port is 8080. To change it, modify the `port` variable in the `NetworkTest` constructor in `main.cpp`. 
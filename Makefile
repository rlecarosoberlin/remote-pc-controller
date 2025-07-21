CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
TARGET = network_test
SOURCE = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

run-server: $(TARGET)
	./$(TARGET) server

run-client: $(TARGET)
	@echo "Usage: make run-client SERVER_IP=<ip_address>"
	@echo "Example: make run-client SERVER_IP=192.168.1.100"
	@if [ -z "$(SERVER_IP)" ]; then \
		echo "Error: SERVER_IP not specified"; \
		exit 1; \
	fi
	./$(TARGET) client $(SERVER_IP)

.PHONY: all clean run-server run-client 
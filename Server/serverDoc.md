# Simple Windows Chat Server

This project implements a basic multi-client chat server using C++ and the Windows Sockets API (Winsock). The server listens for incoming TCP connections, accepts multiple clients simultaneously using **multithreading**, and broadcasts messages received from one client to all other connected clients.

## Features

*   **Multi-client Concurrency:** Handles multiple client connections simultaneously. Each client connection is managed by a dedicated thread, allowing the main server thread to continue listening for new connections without blocking.
*   **Message Broadcasting:** Relays messages from any client to all other connected clients.
*   **TCP/IP Communication:** Uses the standard TCP protocol for reliable data transmission.
*   **Winsock API:** Built using the native Windows Sockets 2 API.

## Included Headers

The following header files are used in `server.cpp`:

*   **`<iostream>`:** Provides standard input/output stream objects like `cout` for printing messages to the console.
*   **`<WinSock2.h>`:** The main header file for using the Windows Sockets 2 API. It contains core socket functions (`socket`, `bind`, `listen`, `accept`, `send`, `recv`, `closesocket`), data structures (`SOCKET`, `sockaddr_in`), and initialization functions (`WSAStartup`, `WSACleanup`).
*   **`<WS2tcpip.h>`:** Provides definitions for TCP/IP specific functionality, including functions like `InetPton` (used for converting IP addresses from text to binary format) and structures used with newer address-family independent functions.
*   **`<tchar.h>`:** (Optional in this specific code but often included in older Windows examples) Provides macros for generic-text mappings, allowing code to be compiled for either Unicode (`wchar_t`) or ANSI (`char`) character sets.
*   **`<thread>`:** Part of the C++ Standard Library, used here to create and manage separate threads of execution for handling each client connection concurrently.
*   **`<vector>`:** Part of the C++ Standard Library, used to create a dynamic array (`std::vector<SOCKET>`) to store the socket descriptors of all currently connected clients.

   
 > For detailed overview and instruction for the project kindly go though `Mircosoft Doc` for the winSock2 library. [*click_here*](https://learn.microsoft.com/en-us/windows/win32/winsock/winsock-server-application)
## Socket Programming Concepts (Winsock)

This server demonstrates the fundamental steps required for TCP server-side socket programming on Windows:

### 1. Initialize Winsock

Before using any Winsock functions, the Winsock library must be initialized. `WSAStartup` is called to specify the version of Winsock requested and to retrieve details about the implementation. We also need to link the `ws2_32.lib` library.

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\server.cpp
// Link the Winsock library
#pragma comment(lib, "ws2_32.lib")

// Function to initialize Winsock
bool Initialize() {
    WSADATA data;
    // Request Winsock version 2.2
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// In main():
int main() {
    if (!Initialize()) {
        cout << "Winsock initialization failed" << endl;
        return 1; // Exit if initialization fails
    }
    // ... rest of the code
}
```

### 2. Create the Socket

A socket is an endpoint for communication. We create a socket using the `socket()` function, specifying the address family (IPv4), socket type (TCP stream), and protocol (0 lets the service provider choose).

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\server.cpp
// In main():
SOCKET listenSocket = socket(AF_INET,     // Address family: IPv4
                             SOCK_STREAM, // Socket type: TCP
                             0);          // Protocol: Default (TCP for SOCK_STREAM)

if (listenSocket == INVALID_SOCKET) {
    cout << "Socket creation failed: " << WSAGetLastError() << endl;
    WSACleanup(); // Clean up Winsock
    return 1;
}
```

### 3. Bind the Socket to an IP Address and Port

Binding assigns a local address (IP address and port number) to the socket. The server needs a specific port to listen on. We use `0.0.0.0` to listen on all available network interfaces on the machine and port `12345`. `htons()` converts the port number to network byte order.

```cpp
// In main():
int portNum = 12345;
sockaddr_in serveraddr;
serveraddr.sin_family = AF_INET;    // IPv4
serveraddr.sin_port = htons(portNum); // Port number in network byte order

// Setting up the address structure

if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
    cout << "setting address structure failed" << endl;
    closesocket(listenSocket);
    WSACleanup();
    return 1;
}

if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
    cout << "Bind failed: " << WSAGetLastError() << endl;
    closesocket(listenSocket);
    WSACleanup();
    return 1;
}
```

### 4. Listen for Incoming Connections

The `listen()` function puts the socket into a state where it can accept incoming connections. `SOMAXCONN` is a system-defined constant for the maximum reasonable backlog of pending connections.

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\server.cpp
// In main():
if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    cout << "Listen failed: " << WSAGetLastError() << endl;
    closesocket(listenSocket);
    WSACleanup();
    return 1;
}

cout << "Server has started listening on port " << portNum << endl;
```

### 5. Accept Client Connections & Launch Threads

The `accept()` function waits for a client. When one connects, a new socket (`clientSocket`) is created for that specific connection. To handle this client without blocking the main loop (which needs to keep listening for *other* clients), a new `std::thread` is created. This thread executes the `InteractWithClient` function, passing it the new `clientSocket` and a reference to the shared `clients` vector. The thread is then `detach()`ed, allowing the main loop to immediately call `accept()` again.

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\server.cpp
// In main():
vector<SOCKET> clients; // Store connected client sockets
while (1) {
    // Accept blocks until a client connects
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Accept failed: " << WSAGetLastError() << endl;
        continue; // Log error and continue listening
    }

    // Add the new client to the list (Needs thread-safety in production)
    clients.push_back(clientSocket);

    // Create a new thread to handle this client independently
    // Pass clientSocket by value, clients vector by reference wrapper
    thread tl(InteractWithClient, clientSocket, ref(clients));
    tl.detach(); // Let the thread run independently from the main loop
}
```

*Note: Accessing the shared `clients` vector from multiple threads (adding in `main`, iterating/removing in `InteractWithClient`) requires proper synchronization (e.g., using `std::mutex`) in a production environment to prevent race conditions. This example omits mutexes for simplicity.*

### 6. Send and Receive Data (Client Thread)

This function runs in a separate thread for each connected client. It continuously waits to `recv()` data on its dedicated `clientSocket`. When data arrives, it broadcasts the message using `send()` to all *other* sockets stored in the shared `clients` vector. If `recv()` indicates a disconnect, the thread cleans up by removing its socket from the vector and closing the socket.

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\server.cpp
void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients) { // Runs in its own thread
    cout << "Client connected." << endl;
    char buffer[4096];

    while (1) {
        int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesrecvd <= 0) {
            cout << "Client disconnected or error: " << WSAGetLastError() << endl;
            break; // Exit the loop for this client thread
        }

        string message(buffer, bytesrecvd);
        cout << "Message from client: " << message << endl;

        // Broadcast the message (Needs thread-safety for 'clients' access)
        for (auto client : clients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.length(), 0);
            }
        }
    }

    // Cleanup for the disconnected client (Needs thread-safety for 'clients' access)
    auto it = find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }
    closesocket(clientSocket);
    cout << "Client socket closed." << endl;
} // Thread terminates here
```

### 7. Close Sockets and Cleanup

When communication is finished (client disconnects or server shuts down), sockets must be closed using `closesocket()`. Finally, `WSACleanup()` is called to release resources used by Winsock.

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\server.cpp
// In InteractWithClient (when client disconnects):
closesocket(clientSocket);

// In main (on server shutdown - though this example runs indefinitely):
// ... (loop termination logic would go here)
closesocket(listenSocket); // Close the listening socket
WSACleanup(); // Final Winsock cleanup
return 0;
```

## How to Build and Run

### Using Visual Studio [Recommended]

1.  Open the `Server.sln` file in Visual Studio.
2.  Select a build configuration (e.g., Debug | x64).
3.  Build the solution (Build > Build Solution or F7).
4.  Run the server (Debug > Start Without Debugging or Ctrl+F5).
 1.  If the windows firewall asks permission kindly grant it.

### Using VS Code (with MSVC Compiler)

1.  **Prerequisites:**
    *   Install VS Code.
    *   Install the Microsoft C/C++ extension.
    *   Install Visual Studio Build Tools (making sure `cl.exe` is in your PATH or run VS Code from the Developer Command Prompt).
2.  **Configure Tasks:** Create a `.vscode/tasks.json` file (see previous response for an example) to define how to compile `server.cpp` using `cl.exe` and link `ws2_32.lib`.
3.  **Configure Launch:** Create a `.vscode/launch.json` file (see previous response for an example) to define how to run the compiled `Server.exe`.
4.  **Build:** Press `Ctrl+Shift+B`.
5.  **Run:** Press `F5` (or use the Run and Debug panel).

The server will start listening on port 12345. You can connect to it using a TCP client application (like Telnet, PuTTY, or a custom client).
 

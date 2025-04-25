# Simple Windows Chat Client

This project implements a basic chat client using C++ and the Windows Sockets API (Winsock). The client connects to the chat server, allows the user to enter a chat name, and then uses separate threads to simultaneously send user input to the server and receive messages broadcast by the server.

## Features

*   **Connects to Server:** Establishes a TCP connection to the specified server IP address and port.
*   **User Identification:** Prompts the user for a chat name to prepend to outgoing messages.
*   **Concurrent Send/Receive:** Uses two threads: one dedicated to reading user input and sending messages, the other dedicated to receiving and displaying messages from the server.
*   **TCP/IP Communication:** Uses the standard TCP protocol for reliable data transmission.
*   **Winsock API:** Built using the native Windows Sockets 2 API.
*   **Graceful Exit:** Allows the user to type "quit" to close the connection and exit the application.

## Included Headers

The following header files are used in `1.cpp`:

*   **`<iostream>`:** Provides standard input/output stream objects like `cin` and `cout` for user interaction and displaying messages.
*   **`<WinSock2.h>`:** The main header file for using the Windows Sockets 2 API. Contains core socket functions (`socket`, `connect`, `send`, `recv`, `closesocket`), data structures (`SOCKET`, `sockaddr_in`), and initialization functions (`WSAStartup`, `WSACleanup`).
*   **`<WS2tcpip.h>`:** Provides definitions for TCP/IP specific functionality, including the `inet_pton` function used here to convert the server's IP address string into the binary format required by the `sockaddr_in` structure.
*   **`<thread>`:** Part of the C++ Standard Library, used to create and manage separate threads for sending and receiving messages concurrently.
*   **`<stdio.h>`:** (Included but `iostream` and `<string>` are primarily used for I/O) Standard C input/output library. `getline` from `<string>` is used for reading full lines.
*   **`<string>`:** Part of the C++ Standard Library, used for easy manipulation of text strings (chat name, messages).


> **Note:** For detailed overview and instruction for the project kindly go though `Mircosoft Doc` for the winSock2 library. [*click_here*](https://learn.microsoft.com/en-us/windows/win32/winsock/winsock-server-application)
## Socket Programming Concepts (Winsock Client)

This client demonstrates the fundamental steps required for TCP client-side socket programming on Windows:

### 1. Initialize Winsock

Similar to the server, the client must initialize the Winsock library before making any socket calls.

```cpp
#pragma comment(lib,"ws2_32.lib") // Link the Winsock library

bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// In main():
int main() {
    if (!Initialize()) {
        cout << "Initialization is failed" << endl;
        return 1;
    }
    // ... rest of the code
}
```

### 2. Create the Socket

A socket is created for the client to use for communication. The parameters (`AF_INET`, `SOCK_STREAM`, `0`) match the server's listening socket for IPv4 TCP communication.

```cpp
// In main():
SOCKET s;
s = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP
if (s == INVALID_SOCKET) {
    cout << "invalid socket created" << endl;
    WSACleanup(); // Clean up Winsock
    return 1;
}
```

### 3. Connect to the Server

The client needs the server's IP address and port number to connect. A `sockaddr_in` structure is filled with this information. `inet_pton` converts the string IP address ("127.0.0.1" for localhost) to the required binary format. `htons` converts the port number to network byte order. The `connect()` function attempts to establish a connection to the server at the specified address and port using the created socket.

```cpp``
int port = 12345; // Must match the server's listening port
string serveraddress = "127.0.0.1"; // IP address of the server (localhost)
sockaddr_in serveraddr;
serveraddr.sin_family = AF_INET;
serveraddr.sin_port = htons(port);
// Convert string IP to binary format
inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));

// Attempt to connect
if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
    cout << "Not able to connect to server. Error: " << WSAGetLastError() << endl;
    closesocket(s);
    WSACleanup();
    return 1;
}

cout << "Successfully connected to server.." << endl;
```

### 4. Send and Receive Data (Using Threads)

To allow the user to type messages and send them *while simultaneously* receiving messages from other clients via the server, two separate threads are created:

*   **`SendMsg` Thread:** Prompts the user for a name, then enters a loop reading lines from `cin`. Each line is prepended with the name and sent to the server using `send()`. If the user types "quit", the loop breaks.
*   **`ReceiveMessage` Thread:** Enters a loop that continuously calls `recv()` to wait for data from the server. When data arrives, it's printed to the console. If `recv()` indicates the server disconnected, the loop breaks.

```cpp
// filepath: d:\Coding\Devlopment\WinChatApp - Copy\Server\1.cpp
void SendMsg(SOCKET s) {
    cout << "Enter your chat name:" << endl;
    string name;
    getline(cin, name); // Read the chat name
    string message;
    while (1) {
        getline(cin, message); // Read message line from user
        string msg = name + ":" + message;
        int bytesent = send(s, msg.c_str(), msg.length(), 0);
        if (bytesent == SOCKET_ERROR) {
            cout << "Error sending message.." << endl;
            break;
        }
        if (message == "quit") { // Check for quit command
            cout << "Stopping the application.." << endl;
            break;
        }
    }
    closesocket(s); // Close socket when sending stops
    WSACleanup();   // Cleanup Winsock (might be called twice if receiver also cleans up)
}

void ReceiveMessage(SOCKET s) {
    char buffer[4096];
    int recvlength;
    string msg = "";
    while (1) {
        recvlength = recv(s, buffer, sizeof(buffer), 0); // Wait for message
        if (recvlength <= 0) { // Server disconnected or error
            cout << "Disconnected from server" << endl;
            break;
        }
        else {
            msg = string(buffer, recvlength); // Convert buffer to string
            cout << msg << endl;              // Display received message
        }
    }
    closesocket(s); // Close socket when receiving stops
    WSACleanup();   // Cleanup Winsock
}

// In main():
// Create and start the threads
thread senderthread(SendMsg, s);
thread receiverthread(ReceiveMessage, s);

// Wait for threads to complete (sender thread exits on "quit" or error)
senderthread.join();
// Receiver thread might exit if server disconnects or send thread closes socket
receiverthread.join();
```

*Note: Both threads call `closesocket` and `WSACleanup`. In a more robust application, cleanup logic should be carefully managed to avoid closing the same socket twice or calling `WSACleanup` prematurely.*

### 5. Close the Socket and Cleanup

Closing the socket (`closesocket()`) and cleaning up Winsock (`WSACleanup()`) are handled within the `SendMsg` and `ReceiveMessage` threads when their respective loops terminate (e.g., user types "quit", server disconnects, or an error occurs). The `main` function uses `join()` to wait for these threads to finish before the program exits.

## How to Build and Run

### Using Visual Studio [Reccomended]

1.  Create a new C++ Console App project in Visual Studio.
2.  Replace the content of the main `.cpp` file with the code from `1.cpp`.
3.  Build the solution (Build > Build Solution or F7).
4.  Run the client (Debug > Start Without Debugging or Ctrl+F5). *Make sure the server is running first.*

### Using VS Code (with MSVC Compiler)
*I did not used this method so I am not sure if the linking and build will work fine or not.  Below is the method generated by chatgpt*
1.  **Prerequisites:**
    *   Install VS Code.
    *   Install the Microsoft C/C++ extension.
    *   Install Visual Studio Build Tools (making sure `cl.exe` is in your PATH or run VS Code from the Developer Command Prompt).
2.  **Configure Tasks:** Create a `.vscode/tasks.json` file to compile `1.cpp` using `cl.exe` and link `ws2_32.lib`.
    ```json
    // Example tasks.json for client
    {
        "version": "2.0.0",
        "tasks": [
            {
                "label": "Build Client (MSVC)",
                "type": "shell",
                "command": "cl.exe",
                "args": [
                    "/EHsc",
                    "/Zi",
                    "/Fe:",
                    "${workspaceFolder}\\Client.exe", // Output executable name
                    "${workspaceFolder}\\1.cpp",    // Input source file
                    "ws2_32.lib"
                ],
                "group": {
                    "kind": "build",
                    "isDefault": true
                },
                "presentation": { "reveal": "always", "panel": "new" },
                "problemMatcher": ["$msCompile"]
            }
        ]
    }
    ```
3.  **Configure Launch:** Create a `.vscode/launch.json` file to run the compiled `Client.exe`.
    ```json
    // Example launch.json for client
    {
        "version": "0.2.0",
        "configurations": [
            {
                "name": "Launch Client (MSVC)",
                "type": "cppvsdbg",
                "request": "launch",
                "program": "${workspaceFolder}\\Client.exe", // Path to executable
                "args": [],
                "stopAtEntry": false,
                "cwd": "${workspaceFolder}",
                "environment": [],
                "console": "integratedTerminal",
                "preLaunchTask": "Build Client (MSVC)" // Build before launching
            }
        ]
    }
    ```
4.  **Build:** Press `Ctrl+Shift+B`.
5.  **Run:** Press `F5` (or use the Run and Debug panel). *Make sure the server is running first.*

Connect to the running server (default is `127.0.0.1` port `12345`). Enter your name, and start chatting. Type `quit` to disconnect and exit.

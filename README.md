# WinChatApp: Multi-threaded TCP Chat System

## Project Overview

A high-performance, multi-threaded chat application implemented using Windows Sockets API (Winsock2) and modern C++ concurrency features. The system consists of a server component that supports multiple simultaneous client connections, with each client handled by a dedicated thread for maximum responsiveness and throughput.

## Technical Architecture

### Server Architecture

- **Connection Management**: Implements the accept-dispatch pattern with a primary thread dedicated to accepting incoming TCP connections while delegating client communication to worker threads.
  
- **Concurrency Model**: Employs a 1:1 threading model where each client connection receives a dedicated thread via `std::thread`, allowing for true parallel processing of messages on multi-core systems.
  
- **Dynamic Client Tracking**: Maintains a thread-safe vector of active socket descriptors to enable efficient broadcasting and connection management.

- **Non-blocking I/O Processing**: Server maintains responsiveness by delegating blocking I/O operations to worker threads, ensuring the main accept loop remains available to handle new connections.

### Client Architecture

- **Dual-threaded Design**: Implements separate execution paths for:
  - User input processing and message transmission
  - Network message reception and display
  
- **Thread Synchronization**: Properly handles thread termination with join semantics to prevent resource leaks.

- **User Experience**: Provides name identification for chat participants and supports graceful application termination via "quit" command.

## Implementation Details

### Key Technical Features

- **Network Protocol**: TCP/IP with socket-level error handling and proper connection lifecycle management
- **Thread Management**: Thread detachment for server worker threads to allow independent lifecycle management
- **Buffer Management**: Fixed-size buffer handling with proper boundary checking for network communication
- **Message Broadcasting**: O(n) broadcasting algorithm that excludes message origin from broadcast targets

### Technical Decisions

- Selected the Windows-native Winsock2 API over cross-platform alternatives to minimize abstraction overhead and achieve maximum performance on the target platform.
  
- Implemented a thread-per-connection model rather than thread pooling or asynchronous I/O for simplicity and direct mapping of client state to thread context.

- Used standard C++ library components (`std::thread`, `std::vector`) to leverage modern language features while maintaining compatibility with the Windows-specific networking stack.

## Development Environment

- **IDE**: Microsoft Visual Studio 2022
- **Language**: C++17
- **System Requirements**: Windows 10/11 with TCP/IP networking support
- **Core Libraries**:
  - Winsock2 (`ws2_32.lib`)
  - C++ Standard Library (threads, containers)  
## Project Structure

```
Windows-Chat-App/
│
├── Server/
│   ├── Server.cpp         # Main server implementation
│   ├── Server.vcxproj     # Project file
│   └── ...
│
├── Client1/
│   ├── Client1.cpp        # Standard chat client implementation
│   ├── Client1.vcxproj    # Project file
│   └── ...
│
├── Client2/
│   ├── Client2.cpp        # Alternative client implementation
    ├── Client2.vcxproj    # Project file
    └── ...
```

## Building the Project

This project is built using Visual Studio's native toolchain. Using Visual Studio is strongly recommended to avoid library linking complexities.

### Recommended Build Procedure

1. Open `Server.sln` in Visual Studio 2022
2. Select preferred build configuration (Debug/Release)
3. Build → Build Solution (F7)
4. Output executables will be placed in the respective Debug/Release folders

## Execution Instructions

1. Launch the server executable first
2. Launch one or more client instances 
3. Enter a user name at the client prompt
4. Exchange messages between connected clients
5. Type "quit" in any client to gracefully disconnect

## Demo Video 
![Click Here](/winChatApp.gif)

## Technical References

- [Windows Sockets 2 Reference](https://docs.microsoft.com/en-us/windows/win32/api/_winsock/)
- [Socket Connection Example](https://docs.microsoft.com/en-us/windows/win32/winsock/accepting-a-connection)
- [Thread Management in C++](https://en.cppreference.com/w/cpp/thread/thread)

## Future Enhancements

- Implement proper thread synchronization for shared resources using mutex
- Add support for private messaging between clients
- Implement a more robust protocol with message types and error handling
- Create a GUI front-end using Windows native controls or a cross-platform framework


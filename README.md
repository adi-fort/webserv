<div align="center">

# 🌐 WEBSERV

### A HTTP/1.1 Web Server Built from Scratch in C++

![C++](https://img.shields.io/badge/C++-98-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![HTTP](https://img.shields.io/badge/HTTP-1.1-4285F4?style=for-the-badge)
![42](https://img.shields.io/badge/42-Common_Core-000000?style=for-the-badge)

**An event-driven HTTP server built using low-level Unix system calls.**

*Networking · Non-blocking I/O · HTTP Protocol · CGI · Systems Programming*

</div>

---

## 📖 About the Project

**Webserv** is a custom HTTP/1.1 server developed as part of the **42 School Common Core curriculum**.

The objective is to understand how web servers work internally by implementing one from scratch, without relying on existing HTTP server libraries or frameworks.

Written in C++, the project focuses on low-level networking, event-driven architecture, HTTP request processing, and Unix system programming.

The server uses non-blocking sockets and `poll()` to manage multiple client connections within a single event loop.

---

## ✨ Features

| Feature | Description |
|:---|:---|
| **HTTP/1.1** | HTTP request parsing and response generation |
| **HTTP Methods** | Support for `GET`, `POST`, and `DELETE` |
| **Non-blocking I/O** | Event-driven connection handling using `poll()` |
| **Multiple Servers** | Configurable virtual servers and listening ports |
| **Routing** | Location-based request handling |
| **Static Files** | HTML, CSS, and other static resources |
| **CGI Execution** | Dynamic content through CGI scripts |
| **File Uploads** | Upload and storage of files |
| **Directory Listing** | Configurable automatic directory indexing |
| **Error Handling** | HTTP status codes and custom error pages |
| **Redirections** | Configurable HTTP redirects |
| **Body Size Limits** | Configurable maximum request body size |

---

## 🏗️ Architecture

The server follows an event-driven architecture designed to handle multiple client connections efficiently.

### Core Components

**🔹 Server & Connection Management**

- Creates and configures TCP listening sockets.
- Accepts incoming client connections.
- Uses non-blocking sockets to avoid blocking the server.
- Monitors file descriptors through `poll()`.
- Manages the lifecycle of client connections.

**🔹 HTTP Request Processing**

- Reads incoming HTTP requests.
- Parses request methods, URIs, headers, and bodies.
- Validates requests and handles malformed input.
- Determines the appropriate response based on server configuration.

**🔹 Configuration Parser**

- Loads configuration files.
- Supports multiple server configurations.
- Handles location-specific directives.
- Configures routing, error pages, body limits, and CGI execution.

**🔹 Request Handler**

- Serves static resources.
- Processes supported HTTP methods.
- Handles file uploads and deletions.
- Generates HTTP responses and status codes.
- Manages redirects and error pages.

**🔹 CGI Handler**

- Executes external scripts using Unix process management.
- Uses `fork()`, `execve()`, and pipes.
- Passes request information to CGI processes.
- Collects script output and generates HTTP responses.

---

## ⚡ Event-Driven Networking

One of the central challenges of Webserv is handling multiple simultaneous connections without creating a dedicated thread for each client.

The server relies on **I/O multiplexing** through `poll()`.

This allows a single event loop to monitor multiple file descriptors and react when sockets become ready for reading or writing.

### Key Concepts

- TCP sockets
- Non-blocking file descriptors
- I/O multiplexing
- Connection lifecycle management
- Partial reads and writes
- Network error handling

This architecture provides practical experience with the foundations of high-performance network servers.

---

## 🚀 Getting Started

### Requirements

- Unix-like operating system
- C++ compiler
- GNU Make
- POSIX networking and process APIs

### Compilation

Clone the repository:

```bash
git clone https://github.com/adi-fort/webserv.git
cd webserv
```

Compile the project:

```bash
make
```

Additional Makefile commands:

```bash
make clean     # Remove object files
make fclean    # Remove object files and executable
make re        # Recompile the entire project
```

### Execution

Start the server using a configuration file:

```bash
./webserv config.conf
```

Or run it with the default configuration:

```bash
./webserv
```

Once the server is running, open a browser and navigate to the configured host and port.

> **Note:** The original configuration files were developed in the 42 environment and may contain machine-specific paths. Adjust them to match your local filesystem before running the server.

---

## ⚙️ Configuration

Webserv uses configuration files to define server behaviour.

Configuration options include:

| Directive Category | Purpose |
|:---|:---|
| Host and Port | Define listening addresses and ports |
| Server Name | Configure virtual server identification |
| Root Directory | Set the document root |
| Allowed Methods | Restrict HTTP methods for specific routes |
| Index Files | Define default pages |
| Error Pages | Configure custom HTTP error responses |
| Body Size | Limit incoming request body size |
| CGI | Configure CGI execution |
| Autoindex | Enable or disable directory listings |
| Redirections | Define HTTP redirects |

The repository includes several configuration files for testing different server behaviours.

---

## 🧪 Testing

The project was developed and evaluated within the **42 School environment**.

Testing covers the main server capabilities, including:

- HTTP request handling
- Multiple client connections
- GET, POST, and DELETE requests
- Static file serving
- CGI execution
- File uploads
- Server configuration
- Error handling

The repository also contains test resources, HTML pages, and CGI scripts used during development.

---

## 🧠 What I Learned

Developing Webserv provided hands-on experience with several fundamental areas of software engineering.

### Network Programming
Understanding TCP connections, socket programming, and communication between clients and servers.

### Operating Systems
Working directly with Unix system calls, file descriptors, processes, and inter-process communication.

### HTTP Protocol
Understanding HTTP message structure, request parsing, response generation, and protocol behaviour.

### Event-Driven Programming
Designing a server around asynchronous I/O readiness rather than sequential blocking operations.

### Software Architecture
Organizing a complex program into distinct components responsible for configuration, networking, parsing, and request handling.

### Debugging & Problem Solving
Investigating networking issues, handling edge cases, and managing complex interactions between system components.

---

## 🛠️ Technologies & Concepts

<div align="center">

![C++](https://img.shields.io/badge/C++-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![HTTP](https://img.shields.io/badge/HTTP-1.1-blue?style=flat-square)
![TCP/IP](https://img.shields.io/badge/TCP%2FIP-Networking-green?style=flat-square)
![POSIX](https://img.shields.io/badge/POSIX-System_Calls-orange?style=flat-square)
![Make](https://img.shields.io/badge/GNU-Make-A42E2B?style=flat-square&logo=gnu&logoColor=white)

</div>

---

## 🎓 42 School

This project is part of the **42 Common Core**, a peer-to-peer, project-based software engineering curriculum.

Webserv focuses on understanding the underlying mechanisms of HTTP servers and developing practical skills in systems programming and networking.

**Project:** Webserv  
**School:** 42 Roma  
**Status:** Completed & Evaluated

---

<div align="center">

### Built from scratch. Driven by curiosity.

**[GitHub Profile](https://github.com/adi-fort)**

<sub>42 Common Core | Systems Programming & Networking</sub>

</div>

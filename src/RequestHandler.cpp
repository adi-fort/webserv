#include "RequestHandler.hpp"
#include "ServerConfig.hpp"
#include "RouteConfig.hpp"
#include "Server.hpp"
#include "FileInfo.hpp"

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <cstdlib> 
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <algorithm> 
#include <filesystem> 


RequestHandler::~RequestHandler() {}

RequestHandler::RequestHandler(const std::map<std::string, ServerConfig>& server_configs) : server_configs(server_configs) {}


void RequestHandler::logServerConfig() const 
{
    	for (const auto& [key, server] : server_configs) 
	{
        	for (const auto& [route, config] : server.routes) 
		{
            		std::cout << " - Route " << route << " has upload_path: " << config.upload_path << "\n";
        	}
    	}
}


const ServerConfig* RequestHandler::findServerConfig(const std::string& host, int port) const 
{
    	std::string normalized_host = host;
    	if (host == "localhost") 
	{
        	normalized_host = "127.0.0.1";
    	}

    	std::string search_key = normalized_host;
    	if (normalized_host.find(":") == std::string::npos) 
	{
        	search_key += ":" + std::to_string(port);
	}

    	auto it = server_configs.find(search_key);
    	if (it != server_configs.end()) 
        	return &it->second;

    	return nullptr;
}

const ServerConfig* RequestHandler::getCurrentServerConfig() 
{
    std::string client_host = "127.0.0.1";
    int client_port = 80;

    auto it = currentRequest.headers.find("Host");
    if (it != currentRequest.headers.end()) {
        std::string hostHeader = it->second;

        size_t colonPos = hostHeader.find(":");
        if (colonPos != std::string::npos) {
            client_host = hostHeader.substr(0, colonPos);
            client_port = std::stoi(hostHeader.substr(colonPos + 1));
        } else {
            client_host = hostHeader;
        }
    }

    std::cout << "DEBUG: Current request -> Host: " << client_host << " | Port: " << client_port << std::endl;

    return findServerConfig(client_host, client_port);
}



const RouteConfig* RequestHandler::findRouteConfig(const ServerConfig& server, const std::string& path) const 
{
    	auto it = server.routes.find(path);
    	if (it != server.routes.end()) 
        	return &it->second;

    	return nullptr;
}



bool isFileExists(const std::string& path) 
{
    	struct stat buffer;
    	return (stat(path.c_str(), &buffer) == 0);
}




std::string RequestHandler::handleRequest(const HttpRequest& request, int client_fd) 
{
	
	this->currentRequest = request;

	const ServerConfig* serverConfig = findServerConfig(request.host, request.port);
	if (serverConfig == nullptr) 
    		return generateErrorResponse(502);
	if (request.path.empty()) 
        	return generateErrorResponse(500);

    	
	const RouteConfig* routeConfig = findRouteConfig(*serverConfig, request.path);

    	if (routeConfig) 
    	{
        	if (!routeConfig->http_redirection.empty()) 
        	{
            		return "HTTP/1.1 301 Moved Permanently\r\n"
                   	"Location: " + routeConfig->http_redirection + "\r\n"
                   	"Content-Length: 0\r\n\r\n";
        	}
    	}	
		
	std::string response;
	
    	auto routeIt = serverConfig->routes.find(request.path);
    	if (routeIt != serverConfig->routes.end()) 
	{
        	const std::vector<std::string>& allowedMethods = routeIt->second.methods;
 
        	if (std::find(allowedMethods.begin(), allowedMethods.end(), request.method) == allowedMethods.end()) 
		{
            		return generateErrorResponse(405);
        	}

    	}
	
	/*if (request.method == "POST" && request.headers.find("Content-Type") != request.headers.end()) 
    	{
        	if (request.headers.at("Content-Type") == "application/x-www-form-urlencoded") 
        	{
            		std::string body(request.body.begin(), request.body.end());
            		if (body.find("_method=DELETE") != std::string::npos) 
            		{
                		return handleDeleteRequest(request);
            		}
        	}
    	}*/
	/*if (request.method == "POST" && request.headers.find("Content-Type") != request.headers.end()) 
	{
    		if (request.headers.at("Content-Type") == "application/x-www-form-urlencoded") 
    		{
        		std::string body(request.body.begin(), request.body.end());
        		if (body.find("_method=DELETE") != std::string::npos) 
        		{
            			auto routeIt = serverConfig->routes.find(request.path);
            			if (routeIt != serverConfig->routes.end()) 
            			{
                			const std::vector<std::string>& allowedMethods = routeIt->second.methods;
                			if (std::find(allowedMethods.begin(), allowedMethods.end(), "DELETE") == allowedMethods.end()) 
                			{
                    				std::cout << "DEBUG: DELETE is not allowed on " << request.path << std::endl;
                    				return generateErrorResponse(405);
                			}
            			}
			
            			return handleDeleteRequest(request);
        		}	
    		}
	}*/
	
	if (request.method == "POST" && request.headers.find("Content-Type") != request.headers.end()) 
{
    if (request.headers.at("Content-Type") == "application/x-www-form-urlencoded") 
    {
        std::string body(request.body.begin(), request.body.end());

        if (body.find("_method=DELETE") != std::string::npos) 
        {
            std::cout << "DEBUG: _method=DELETE detected in POST request for " << request.path << std::endl;

            if (request.path.find("/uploads/") == 0) 
            {
                auto routeIt = serverConfig->routes.find("/upload");
                if (routeIt == serverConfig->routes.end()) 
                {
                    std::cout << "DEBUG: /upload route not found in config\n";
                    return generateErrorResponse(404);
                }

                const std::vector<std::string>& allowedMethods = routeIt->second.methods;
                if (std::find(allowedMethods.begin(), allowedMethods.end(), "DELETE") == allowedMethods.end()) 
                {
                    std::cout << "DEBUG: DELETE is not allowed on /uploads/\n";
                    return generateErrorResponse(405);
                }

                return handleDeleteRequest(request);
            }
        }
    }
}


	
	if (request.method == "POST" && request.body.size() > 10 * 1024 * 1024) 
	{
    		std::cout << "DEBUG: Body too large! Rejecting request." << std::endl;
		return generateErrorResponse(413);
	}
	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
	{
		response = generateErrorResponse(403);
	}
	else if (request.path == "/upload" && request.method == "POST") 
	{	
		response = handleUploadRequest(client_fd, request);
	}
	else if (request.path == "/upload" && request.method == "DELETE") 
	{
    		auto routeIt = serverConfig->routes.find("/upload");
    		if (routeIt != serverConfig->routes.end()) 
    		{
        		const std::vector<std::string>& allowedMethods = routeIt->second.methods;
        		if (std::find(allowedMethods.begin(), allowedMethods.end(), "DELETE") == allowedMethods.end()) 
        		{
            			return generateErrorResponse(405);
        		}
    		}
		response = handleDeleteRequest(request);
	}
	else if (request.path == "/upload.html" && request.method == "GET") 
	{
        	response = serveStaticFile("/upload.html");
    	} 
	else if (request.path == "/uploads" && request.method == "GET") 
	{
        	response = listUploadedFiles();
    	} 
	else if (request.path.find("/testCGI/") == 0) 
	{
        	response = handleCgiRequest(request);
    	} 
	else if (request.method == "GET") 
	{
		response = serveStaticFile(request.path);
    	} 
	else if (request.method == "POST") 
	{
        	response = handlePostRequest(request);
    	} 
/*	else if (request.method == "DELETE") 
	{
        	response = handleDeleteRequest(request);
    	} 
*/	

/*else if (request.method == "DELETE") 
{
    auto routeIt = serverConfig->routes.find(request.path);
    if (routeIt != serverConfig->routes.end()) 
    {
        const std::vector<std::string>& allowedMethods = routeIt->second.methods;
        if (std::find(allowedMethods.begin(), allowedMethods.end(), "DELETE") == allowedMethods.end()) 
        {
            std::cout << "DEBUG: DELETE is not allowed on " << request.path << std::endl;
            return generateErrorResponse(405);
        }
    }
    response = handleDeleteRequest(request);
}*/
	else if (request.method == "DELETE") 
{
    std::cout << "DEBUG: DELETE request received for path: " << request.path << std::endl;

    if (request.path.find("/uploads/") == 0) 
    {
        auto routeIt = serverConfig->routes.find("/upload");
        if (routeIt == serverConfig->routes.end()) 
        {
            std::cout << "DEBUG: /upload route not found in config\n";
            return generateErrorResponse(404);
        }

        const std::vector<std::string>& allowedMethods = routeIt->second.methods;
        if (std::find(allowedMethods.begin(), allowedMethods.end(), "DELETE") == allowedMethods.end()) 
        {
            std::cout << "DEBUG: DELETE is not allowed on /uploads/\n";
            return generateErrorResponse(405);
        }

        return handleDeleteRequest(request);
    }

    return generateErrorResponse(403);
}



	else 
	{
        	response = generateErrorResponse(405);
    	}

    	if (response.empty()) 
	{
        	std::cerr << "ERROR: Response is empty for path: " << request.path << std::endl;
        	response = generateErrorResponse(500);
    	}

	return response;
}


std::string RequestHandler::decodeUrl(const std::string &url)
{
    	std::string result;
    	char hex[3] = {0};
    	for (size_t i = 0; i < url.length(); i++) 
    	{
        	if (url[i] == '%') 
        	{
            		if (i + 2 < url.length()) 
            		{
                		hex[0] = url[i + 1];
                		hex[1] = url[i + 2];
                		result += static_cast<char>(strtol(hex, nullptr, 16));
                		i += 2;
            		}
        	} 
        	else if (url[i] == '+') 
        	{
            		result += ' ';
        	} 
        	else 
        	{
            		result += url[i];
        	}
    	}
    	return result;
}



std::string RequestHandler::handleCgiRequest(const HttpRequest& request) 
{
    	const ServerConfig* serverConfig = findServerConfig(request.host, request.port);
	if (!serverConfig) 
		return generateErrorResponse(502);

	
	std::string path = request.path;
    	if (path.find("/testCGI/") == 0)
        	path = path.substr(9);  
    
    	//std::string script_path = "/var/www/cgi-bin/" + path;
	std::string script_path = serverConfig->routes.at("/testCGI/").root + "/" + path;

    	if (access(script_path.c_str(), F_OK) == -1)
        	return generateErrorResponse(404);
    
    	if (access(script_path.c_str(), X_OK) == -1)
        	return generateErrorResponse(403);

    	int pipe_stdout[2];
    	int pipe_stdin[2];

    	if (pipe(pipe_stdout) == -1 || pipe(pipe_stdin) == -1)
        	return generateErrorResponse(500);

    	pid_t pid = fork();
    	if (pid < 0) 
	{
        	return generateErrorResponse(500);
    	}
    	else if (pid == 0) 
	{
        	close(pipe_stdout[0]);
        	dup2(pipe_stdout[1], STDOUT_FILENO);
        	close(pipe_stdout[1]); 

        	close(pipe_stdin[1]);
        	dup2(pipe_stdin[0], STDIN_FILENO);
        	close(pipe_stdin[0]);

        	char *args[] = { const_cast<char*>(script_path.c_str()), NULL };
	
		std::vector<std::string> env_strings;
		env_strings.push_back("REQUEST_METHOD=" + request.method);
		env_strings.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));
		env_strings.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");

		std::vector<char*> envp;
		for (size_t i = 0; i < env_strings.size(); ++i) 
		{
    			envp.push_back(const_cast<char*>(env_strings[i].c_str()));
		}	
		envp.push_back(NULL);

		alarm(5);
		execve(script_path.c_str(), args, envp.data());
	
        	exit(1);
    	}
    	else 
	{
        	close(pipe_stdout[1]);
        	close(pipe_stdin[0]);

        	if (request.method == "POST" && !request.body.empty()) 
		{
			ssize_t written = write(pipe_stdin[1], request.body.data(), request.body.size());
			if (written < 0)
			{
				return generateErrorResponse(500);
			}
		}
        	close(pipe_stdin[1]);

        	char 		buffer[4096];
        	std::string 	output;
        	ssize_t 	bytes_read;
        	fd_set		readfds;		
		struct timeval	timeout;
		
		FD_ZERO(&readfds);
		FD_SET(pipe_stdout[0], &readfds);
		
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		
		int ready = select(pipe_stdout[0] + 1, &readfds, NULL, NULL, &timeout);
        	if (ready == 0) 
        	{
            		kill(pid, SIGKILL);
            		return generateErrorResponse(504);
        	}

        	while ((bytes_read = read(pipe_stdout[0], buffer, sizeof(buffer))) > 0)
            		output.append(buffer, bytes_read);
        	close(pipe_stdout[0]);

       		int status;
        	waitpid(pid, &status, 0);

        	if (status != 0)
            		return generateErrorResponse(500);

        	std::string response = "HTTP/1.1 200 OK\r\n";
        	response += "Content-Length: " + std::to_string(output.size()) + "\r\n";
        	response += "Content-Type: text/html\r\n\r\n";
        	response += output;

        	return response;    	
	}
}


typedef std::vector<unsigned char> ByteVector;

ByteVector::const_iterator findInVector(const ByteVector& haystack, const std::string& needle, ByteVector::const_iterator start) 
{
    	return std::search(start, haystack.end(), needle.begin(), needle.end());
}

ByteVector extractSubVector(const ByteVector& vec, ByteVector::const_iterator start, ByteVector::const_iterator end) 
{
    	return ByteVector(start, end);
}


std::string RequestHandler::handlePostRequest(const HttpRequest& request) 
{
    	if (request.body.empty()) 
    	{
        	std::cerr << "Request body is empty!" << std::endl;
        	return generateErrorResponse(400);
    	}

    	std::string content_disposition;
    	ByteVector::const_iterator content_disposition_pos = findInVector(request.body, "Content-Disposition: form-data;", request.body.begin());
    	if (content_disposition_pos != request.body.end()) 
    	{
        	ByteVector::const_iterator start_pos = findInVector(request.body, "filename=\"", content_disposition_pos);
        	if (start_pos != request.body.end()) 
		{
            		start_pos += std::strlen("filename=\"");
            		ByteVector::const_iterator end_pos = std::find(start_pos, request.body.end(), '\"');
            		if (end_pos != request.body.end()) 
			{
                		ByteVector content_disposition_vec = extractSubVector(request.body, start_pos, end_pos);
                		content_disposition = std::string(content_disposition_vec.begin(), content_disposition_vec.end());
            		}
        	}
    	}

    	if (content_disposition.empty()) 
        	return generateErrorResponse(400);

    	std::string file_name = content_disposition;
    	std::string file_path = "uploads/" + file_name;

    	std::ofstream out_file(file_path.c_str(), std::ios::binary);
    	if (out_file.is_open()) 
    	{
        	out_file.write(reinterpret_cast<const char*>(request.body.data()), request.body.size());
        	out_file.close();
    	} 
    	else 
        {	
		return generateErrorResponse(500);
	}
    	
	std::string response = "HTTP/1.1 200 OK\r\n";
    	response += "Content-Length: " + std::to_string(file_name.size()) + "\r\n";
    	response += "Content-Type: text/plain\r\n\r\n";
    	response += "Received file: " + file_name;

    	return response;
}


ByteVector::const_iterator findBoundary(const ByteVector& body, const std::string& boundary) 
{
    	return std::search(body.begin(), body.end(), boundary.begin(), boundary.end());
}


FileInfo extractFileInfo(const ByteVector& body, const std::string& boundary) 
{
    	FileInfo fileInfo;

    	ByteVector::const_iterator boundary_pos = findBoundary(body, boundary);
    	if (boundary_pos == body.end()) 
	{
        	throw std::runtime_error("Boundary not found in body!");
    	}

    	ByteVector::const_iterator filename_pos = findBoundary(body, "filename=\"");
    	if (filename_pos == body.end()) 
	{
        	throw std::runtime_error("Filename not found!");
    	}
    	filename_pos += 10;

    	ByteVector::const_iterator filename_end = std::find(filename_pos, body.end(), '\"');
    	fileInfo.filename = std::string(filename_pos, filename_end);

    	ByteVector::const_iterator content_start = findBoundary(body, "\r\n\r\n");
    	if (content_start == body.end()) 
	{
        	throw std::runtime_error("File start offset not found!");
    	}
    	content_start += 4;

    	ByteVector::const_iterator content_end = findBoundary(body, boundary);
	if (content_end == body.end()) 
	{
        	throw std::runtime_error("File end offset not found!!");
    	}
    	
	content_end -= 2;

    	fileInfo.file_start = content_start;
    	fileInfo.file_end = content_end;

    	return fileInfo;
}


std::string RequestHandler::handleUploadRequest(int client_fd, const HttpRequest& request) 
{
    	
	const ServerConfig* serverConfig = findServerConfig(request.host, request.port);
	if (!serverConfig) 
		return generateErrorResponse(502);

	try 
	{
        	if (request.method != "POST")
            		return generateErrorResponse(405);

        	auto it = request.headers.find("Content-Type");
        	if (it == request.headers.end()) 
            		return generateErrorResponse(400);
        
       		size_t boundary_pos = it->second.find("boundary=");
        	if (boundary_pos == std::string::npos) 
            		return generateErrorResponse(400);
        
        	std::string boundary = "--" + it->second.substr(boundary_pos + 9);

        	ByteVector FiletoDisplay;

        	ByteVector::const_iterator file_start = std::search(request.body.begin(), request.body.end(), "\r\n\r\n", "\r\n\r\n" + 4);
        	
		if (file_start == request.body.end()) 
		{
            		throw std::runtime_error("Error file start not found!");
        	}
        	file_start += 4;

        	ByteVector::const_iterator file_end = std::search(file_start, request.body.end(), boundary.begin(), boundary.end());
        	if (file_end == request.body.end()) 
            		throw std::runtime_error("Error: file end not found!");
        	file_end -= 2;  

        	FiletoDisplay.assign(file_start, file_end);

        	//std::string upload_dir = "/home/andrea/Scrivania/lastweb-main/uploads";
		std::string upload_dir = serverConfig->routes.at("/upload").upload_path;

		if (!std::filesystem::exists(upload_dir)) 
		{
            		std::filesystem::create_directory(upload_dir);
        	}
	
		const FileInfo& fileInfo = request.files[0];
		std::string filename = fileInfo.filename;
	
        	std::string file_path = upload_dir + "/" + filename;
        	std::ofstream out_file(file_path, std::ios::binary);
        	if (!out_file.is_open()) 
		{
            		return generateErrorResponse(500);
        	}

        	out_file.write(reinterpret_cast<const char*>(FiletoDisplay.data()), FiletoDisplay.size());
        	out_file.close();

       		std::string response_body = "<html><body><h2>File uploaded successfully!</h2></body></html>";


		std::string response = "HTTP/1.1 200 OK\r\n";
		response += "Content-Length: " + std::to_string(response_body.size()) + "\r\n";
		response += "Content-Type: text/html\r\n";
		response += "\r\n";
		response += response_body;
        
		return response;

    	} 
	catch (const std::exception& e) 
	{
        	std::cerr << "ERROR: " << e.what() << std::endl;
        	return generateErrorResponse(400);
    	}
}


std::string RequestHandler::listUploadedFiles() 
{
    	std::string response = "HTTP/1.1 200 OK\r\n";
    	response += "Content-Type: text/html\r\n";

    	std::string body = "<html><body><h1>Uploaded Files</h1>";

    	std::vector<std::string> files = getUploadedFiles();
    	for (const auto& file : files) 
    	{
        	body += "<a href='/uploads/" + file + "'>" + file + "</a><br>";
        	body += "<form action='/uploads/" + file + "' method='POST'>";
        	body += "<input type='hidden' name='_method' value='DELETE'>";
        	body += "<input type='submit' value='Delete this file'>";
        	body += "</form><br>";
    	}

    	body += "</body></html>";

    	response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    	response += "\r\n";
    	response += body;

    	return response;
}


std::vector<std::string> RequestHandler::getUploadedFiles() 
{
    	std::vector<std::string> files;
    	for (const auto& entry : std::filesystem::directory_iterator("uploads")) 
	{
        	files.push_back(entry.path().filename().string());
    	}
    	return files;
}

std::string RequestHandler::handleDeleteRequest(const HttpRequest& request) 
{
    	const ServerConfig* serverConfig = findServerConfig(request.host, request.port);
	if (!serverConfig) 
		return generateErrorResponse(502);

	std::string file_path = request.path;

    	if (file_path.find("/uploads/") != 0) 
    	{
        	return "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n";
    	}

    	std::string filename = file_path.substr(9);
    	filename = decodeUrl(filename);

    	//std::string full_path = "/home/andrea/Scrivania/lastweb-main/uploads/" + filename;
	std::string full_path = serverConfig->routes.at("/upload").upload_path + "/" + filename;

	
    	if (!std::filesystem::exists(full_path)) 
    	{
        	std::cerr << "ERROR: File to delete not found: " << full_path << std::endl;
        	return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    	}

    	if (!std::filesystem::remove(full_path)) 
    	{
        	std::cerr << "ERROR: Failed to remove file: " << full_path << std::endl;
        	return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
    	}

    	std::string message = "File deleted successfully";
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Length: " + std::to_string(message.size()) + "\r\n";
	response += "Content-Type: text/plain\r\n\r\n";
	response += message;
    	return response;
}


std::string RequestHandler::serveStaticFile(const std::string& path) 
{
	const ServerConfig* serverConfig = findServerConfig("127.0.0.1", 8080);
	if (!serverConfig) 
		return generateErrorResponse(502);

	const RouteConfig* routeConfig = findRouteConfig(*serverConfig, path);

	
	std::string basePath;
	struct stat buffer;
	
	std::cout << path << std::endl;
	
	/*if (path.find("/uploads", 0) == 0)
		basePath = "/home/andrea/Scrivania/lastweb-main";
	else
	{
		basePath = "/home/andrea/Scrivania/lastweb-main/static";
        }
	
	std::string fullPath = basePath + path;
	*/
	
	std::cout << "DEBUG: Requested path: " << path << std::endl;
std::cout << "DEBUG: Using routeConfig? " << (routeConfig ? "Yes" : "No") << std::endl;
std::cout << "DEBUG: routeConfig->root: " << (routeConfig ? routeConfig->root : "N/A") << std::endl;
std::cout << "DEBUG: serverConfig->static_directory: " << serverConfig->static_directory << std::endl;

	
	std::string fullPath = routeConfig ? routeConfig->root + path : serverConfig->static_directory + path;

	if (stat(fullPath.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode)) 
	{
        	std::cout << "DEBUG: Requested path is a directory: " << fullPath << std::endl;
        	fullPath += "/index.html";
    	}
	
	std::cout << "DEBUG: Attempting to serve file at: " << fullPath << std::endl;
	  	
	if (stat(fullPath.c_str(), &buffer) != 0) 
	{
        	std::cerr << "ERROR: File not found: " << fullPath << std::endl;
        	return (generateErrorResponse(404));
		//return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    	}

	std::ifstream file(fullPath, std::ios::binary);
	if (!file.is_open()) 
	{
        	std::cerr << "ERROR: Impossible to open file: " << fullPath << std::endl;
        	return generateErrorResponse(404);
    	}
  
	std::vector<char> file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	
	std::string content_type = "text/html";
    	if (path.find(".css") != std::string::npos) content_type = "text/css";
    	else if (path.find(".js") != std::string::npos) content_type = "application/javascript";
    	else if (path.find(".png") != std::string::npos) content_type = "image/png";
    	else if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) content_type = "image/jpeg";
    	else if (path.find(".gif") != std::string::npos) content_type = "image/gif";
    	else if (path.find(".json") != std::string::npos) content_type = "application/json";
    	else if (path.find(".txt") != std::string::npos) content_type = "text/plain";

 	std::ostringstream response;
    	response << "HTTP/1.1 200 OK\r\n";
    	response << "Content-Length: " << file_content.size() << "\r\n";
    	response << "Content-Type: " << content_type << "\r\n";
    	response << "Connection: keep-alive\r\n";
    	response << "\r\n";
    	response.write(file_content.data(), file_content.size());

    	return response.str();
}

/*
std::string RequestHandler::generateErrorResponse(int errorCode) 
{

    	const ServerConfig* serverConfig = getCurrentServerConfig();
    	if (!serverConfig) 
	{
        	return "HTTP/1.1 500 Internal Server Error\r\n"
               		"Content-Type: text/html\r\n"
               		"Content-Length: 48\r\n\r\n"
               		"<html><body style='background:black;color:white;text-align:center;'><h1>500 Internal Server Error</h1></body></html>";    
	}
	
    	auto it = serverConfig->error_pages.find(errorCode);
    if (it == serverConfig->error_pages.end()) {
        return "HTTP/1.1 " + std::to_string(errorCode) + " Error\r\n"
               "Content-Length: 0\r\n"
               "Content-Type: text/plain\r\n\r\n";
    }

    std::string file_path = it->second;
    std::cout << "DEBUG: Pagina errore trovata: " << file_path << std::endl;

    std::ifstream file(file_path.c_str());
    if (!file.is_open()) {
        std::cerr << "ERROR: Custom error page not found: " << file_path << std::endl;
	return "HTTP/1.1 " + std::to_string(errorCode) + " Error\r\n"
               "Content-Length: 0\r\n"
               "Content-Type: text/plain\r\n\r\nDEFAULT ERROR PAGE";
    }

    std::ostringstream file_content;
    file_content << file.rdbuf();

    std::string response = "HTTP/1.1 " + std::to_string(errorCode) + " Error\r\n";
    response += "Content-Length: " + std::to_string(file_content.str().size()) + "\r\n";
    response += "Content-Type: text/html\r\n\r\n";
    response += file_content.str();

    return response;
}
*/

std::string RequestHandler::generateErrorResponse(int errorCode) {
    const ServerConfig* serverConfig = getCurrentServerConfig();
    if (!serverConfig) {
        return "HTTP/1.1 500 Internal Server Error\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: 48\r\n\r\n"
               "<html><body style='background:black;color:white;text-align:center;'><h1>500 Internal Server Error</h1></body></html>";
    }

    auto it = serverConfig->error_pages.find(errorCode);
    std::string file_path;

    if (it != serverConfig->error_pages.end()) {
        file_path = it->second;
        std::cout << "DEBUG: Using custom error page: " << file_path << std::endl;
    } else {
        std::cerr << "WARNING: No custom error page for error " << errorCode << ". Using default error fallback." << std::endl;
        file_path = "default_error.html"; 
    }

    std::ifstream file(file_path.c_str());
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open error page: " << file_path << ". Sending fallback HTML response.\n";

        std::string fallbackHTML =
            "<html><body style='background:black;color:white;text-align:center;'>"
            "<h1>" + std::to_string(errorCode) + " ERROR</h1></body></html>";

        return "HTTP/1.1 " + std::to_string(errorCode) + " Error\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(fallbackHTML.size()) + "\r\n\r\n" +
               fallbackHTML;
    }

    std::ostringstream file_content;
    file_content << file.rdbuf();
    file.close();

    std::string response = "HTTP/1.1 " + std::to_string(errorCode) + " Error\r\n";
    response += "Content-Length: " + std::to_string(file_content.str().size()) + "\r\n";
    response += "Content-Type: text/html\r\n\r\n";
    response += file_content.str();

    return response;
}


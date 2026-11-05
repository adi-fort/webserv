#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include "RouteConfig.hpp"

struct ServerConfig 
{
    	std::string 				host;
    
	int 					port;
    
	std::string 				server_name;
    
	size_t 					client_body_size;

	std::string				static_directory;
    
	std::vector<std::string>		allowed_methods;

	std::map<std::string, RouteConfig>	routes;
	
	std::map<int, std::string>		error_pages;
};

#endif


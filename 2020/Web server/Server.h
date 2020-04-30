#ifndef SERVER_H
#define SERVER_H

// Libraries
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Including headers 
#include "Utility.h"
#include "Dictionary.h"

// Defines
static const char *SERVER_NAME = "C web server project";
static const char *BACKEND_SERVER = "https://github.com/ItsOKayCZ/C";
#define DEFAULT_REQUEST_LENGTH 30000
#define DEBUG 1

// HTML status codes
#define OK 200
#define BAD_REQUEST 400

#define NOT_FOUND 404
static const char notFoundFile[] = "/notFound.html";

// Request indexes
#define HTTP_METHOD 	0
#define HTTP_URL		1
#define HTTP_VERSION 	2

// All of the allowed methods for this web server
static const char *allowedMethods[] = {"GET", "POST"};

// All of the MIME types
static const char *MIMETypesInit[] = {
	"html=text/html; charset=utf-8",
	"css=text/css",
	"js=text/javascript"
};
static Dictionary **MIMETypes;
#define DEFAULT_MIME_TYPE "text/html; charset=utf-8"
//#define DEFAULT_MIME_TYPE "text/plain" // The default MIME type

// Add of the status codes
static const char *StatusCodesInit[] = {
	"200=OK",
	"400=Bad Request",
	"404=Not Found"
};
static Dictionary **statusCodes;

/**
 * The main server struct for initializing the server
 * fd				-> The server fd
 * address			-> The ip and port configuration
 * requestLength	-> The maximum amount of bytes accepted from the client in a request
 */
typedef struct{
	int fd;
	struct sockaddr_in address;
	int requestLength;
} Server;

/**
 * The HTTP request
 * method		-> The HTTP method (GET, POST)
 * version		-> The HTTP version
 * path			-> The requested path
 * code			-> Will be set with a HTML status code
 */
typedef struct{
	char *method;
	char *version;
	char *path;
	Dictionary **query;
	Dictionary **headers;
	int code;
} HTTPRequest;

/**
 * The HTTP response
 * headers		-> The headers that will be sent back
 * content		-> The web page
 * statusCode	-> The status code of the response
 * socket		-> The socket of the connected client
 */
typedef struct{
	Dictionary **headers;
	char *content;
	int statusCode;
	char *version;
	int socket;
} HTTPResponse;

// Functions
int server_getFile(char **content, char *path);
void server_sendResponse(HTTPResponse *response);
char *server_getMIMEType(char *path);
static HTTPResponse *server_initHTTPResponse(int socket, HTTPRequest *request);
void server_freeHTTPResponse(HTTPResponse *response);
void server_freeHTTPRequest(HTTPRequest *request);
static HTTPRequest *server_setError(HTTPRequest *request, int statusCode);
static HTTPRequest *server_parseRequest(char *request);
int server_listen(Server *server, void (*callback)(HTTPRequest *request, HTTPResponse *response));
int server_create(Server *server, const int PORT);

#endif

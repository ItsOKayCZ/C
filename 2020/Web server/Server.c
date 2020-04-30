#include "Server.h"

/**
 * Gets the contents of the file requested
 * @param content	-> Where the file content will be saved
 * @param path		-> The path to the file
 * @return Returns the OK code or NOT FOUND code
 */
int server_getFile(char **content, char *path){
	FILE *file;

	file = fopen(path + 1, "r");
	if(file == NULL){
		perror("Error while opening file.\n");
		return NOT_FOUND;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	*content = malloc(fileSize + 1);
	fread(*content, 1, fileSize, file);
	fclose(file);

	(*content)[fileSize] = 0;

	return OK;
}

/**
 * Send a response to the user
 * @param response	-> The response parameters
 */
void server_sendResponse(HTTPResponse *response){
	/*
	 * 200 OK
	 * Access-Control-Allow-Origin: *
	 * Connection: close
	 * Content-Type: text/html; charset=utf-8
	 * Server: Apache
	 * X-Backend-Server: developer2.webapp.scl3.mozilla.com
	*/

	char *strResponse;
	char strCode[2];
	sprintf(strCode, "%d", response->statusCode);

	asprintf(&strResponse, "%s %d %s\n", response->version, response->statusCode, dictionary_findValue(statusCodes, strCode));

	for(int i = 0; i < dictionary_size(response->headers); i++){
		strResponse = realloc(strResponse, strlen(strResponse) + strlen(response->headers[i]->key) + strlen(response->headers[i]->value) + 4);
		strcat(strResponse, response->headers[i]->key);
		strcat(strResponse, ": ");
		strcat(strResponse, response->headers[i]->value);
		strcat(strResponse, "\n");
	}

	strResponse = realloc(strResponse, strlen(strResponse) + strlen(response->content) + 3);
	strcat(strResponse, "\n");
	strcat(strResponse, response->content);
	strcat(strResponse, "\n");

	if(DEBUG == 1){
		printf("Raw response: \n%s\n", strResponse);
	}
	write(response->socket, strResponse, strlen(strResponse));
	
	free(strResponse);
}

/**
 * Gets the MIME type from the url path
 * @param path	-> The url path
 * @return Returns the MIME type
 */
char *server_getMIMEType(char *path){
	char *extension;
	if(strcmp(path, "/") == 0){
		extension = strdup("html");
	} else {
		char **temp = str_split(path, ".");
		extension = strdup(temp[str_len(temp) - 1]); 
		str_free(temp);
	} 

	char *MIMEType = dictionary_findValue(MIMETypes, extension);
	if(MIMEType == NULL){
		MIMEType = DEFAULT_MIME_TYPE;
	}

	free(extension);
	return MIMEType;
}

/**
 * Inits and sets all the elements of the HTTPResponse struct
 * @param response	-> The HTTPResponse struct
 * @param socket	-> The socket of the connected client
 * @return Returns the HTTPResponse struct with the headers set
 */
static HTTPResponse *server_initHTTPResponse(int socket, HTTPRequest *request){
	/*
	 * 200 OK
	 * Access-Control-Allow-Origin: *
	 * Connection: close
	 * Content-Type: text/html; charset=utf-8
	 * Server: Apache
	 * X-Backend-Server: developer2.webapp.scl3.mozilla.com
	*/

	HTTPResponse *httpResponse = malloc(sizeof(HTTPResponse));
	httpResponse->socket = socket;

	// Setting status code
	httpResponse->statusCode = request->code;

	// Setting the HTTP version
	httpResponse->version = strdup(request->version);

	// Setting headers
	httpResponse->headers = dictionary_init(NULL, NULL);
	dictionary_add(&httpResponse->headers, "Access-Control-Allow-Origin", "*");
	dictionary_add(&httpResponse->headers, "Connection", "close");
	dictionary_add(&httpResponse->headers, "Server", (char *)SERVER_NAME);
	dictionary_add(&httpResponse->headers, "X-Backend-Server", (char *)BACKEND_SERVER);

	// Setting Content-Type
	char *MIMEType = server_getMIMEType(request->path); 
	dictionary_add(&httpResponse->headers, "Content-type", MIMEType);

	if(DEBUG == 1){
		printf("Response headers:\n");
		for(int i = 0; i < dictionary_size(httpResponse->headers); i++){
			printf("%s: %s\n", httpResponse->headers[i]->key, httpResponse->headers[i]->value);
		}
		printf("\n");
	}

	char *path = strdup(request->path);
	if(strcmp(path, "/") == 0){
		free(path);
		path = strdup("/index.html");
	}
	int code = server_getFile(&httpResponse->content, path);
	if(code == NOT_FOUND){
		code = server_getFile(&httpResponse->content, (char *)notFoundFile);
		httpResponse->statusCode = code;
	}
	
	free(path);
	return httpResponse;
}

/**
 * Frees the memory of the HTTPResponse struct
 * @param response	-> The HTTPResponse struct to be freed
 */
void server_freeHTTPResponse(HTTPResponse *response){
	dictionary_free(response->headers);
	free(response->content);
	response->statusCode = 0;
	free(response);
}

/**
 * Frees the memory of the HTTPRequest struct
 * @param request	-> The HTTPRequest struct to be freed
 */
void server_freeHTTPRequest(HTTPRequest *request){
	free(request->method);
	free(request->version);
	free(request->path);
	dictionary_free(request->query);
	dictionary_free(request->headers);
	request->code = 0;
}

/**
 * Sets the error code of the HTTPRequest
 * @param request		-> The HTTPRequest struct of the request
 * @param statusCode	-> The statusCode to set it to
 * @return Returns the 
 */
static HTTPRequest *server_setError(HTTPRequest *request, int statusCode){
	server_freeHTTPRequest(request);
	request->code = statusCode;

	return request;
}

/**
 * Parses the request into the HTTRequest struct
 * @param request	-> The request string
 * @return Returns the HTTPRequest without error code, if no occures
 */
static HTTPRequest *server_parseRequest(char *request){
	printf("Parsing request: \n%s\n", request);

	/*
	 * Request:
	 * GET /index.html?query=Testing&query2=Testing2 HTTP/1.1
	 * Host: localhost:8080
	 * User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0
	 * Accept: text/html,application/xhtml+xml,application/xml;q=0.9,`star`/`star`;q=0.8
	 * Accept-Language: en-GB,en;q=0.5
	 * Accept-Encoding: gzip, deflate
	 * DNT: 1
	 * Connection: keep-alive
	 * Upgrade-Insecure-Requests: 1
	 *
	 * typdef struct {
	 *  char *method;
	 *  char *version;
	 *  char *path
	 *  Hashmap **parameters;
	 *	Hashmap **headers;
	 * } httpRequest;
	 *
	 * Structure:
	 * method = GET
	 * version = HTTP/1.1
	 * path = index.html 
	 * query = {
	 * 	query: Testing,
	 * 	query2: Testing2
	 * }
	 * headers = {
	 *  Host: localhost:8080,
	 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:68.0) Gecko/20100101 Firefox/68.0
	 *  Accept: text/html,application/xhtml+xml,application/xml;q=0.9,`star`/`star`;q=0.8
	 *  Accept-Language: en-GB,en;q=0.5
	 *  Accept-Encoding: gzip, deflate
	 *  DNT: 1
	 *  Connection: keep-alive
	 *  Upgrade-Insecure-Requests: 1	 
	 *}
	 */

	request = str_removeChar(request, '\r');
	char **requestLines = str_split(request, "\n");

	static HTTPRequest httpRequest;
	if(str_len(requestLines) <= 0){
		printf("Bad request\n");
		return server_setError(&httpRequest, BAD_REQUEST);
	}

	// First line
	char **requestInfo = str_split(requestLines[0], " ");

	// Checking if the request method is allowed
	int allowed = 1;
	for(int i = 0; i < str_len((char **)allowedMethods); i++){
		if(strcmp(allowedMethods[i], requestInfo[HTTP_METHOD]) == 0){
			allowed = 0;
			break;
		}
	}

	if(allowed){
		printf("Not allowed method\n");
		return server_setError(&httpRequest, BAD_REQUEST);
	}

	// Setting method
	httpRequest.method = (char *)malloc(strlen(requestInfo[HTTP_METHOD]) + 1);
	strcpy(httpRequest.method, requestInfo[HTTP_METHOD]);

	// Setting version
	httpRequest.version = (char *)malloc(strlen(requestInfo[HTTP_VERSION]) + 1);
	strcpy(httpRequest.version, requestInfo[HTTP_VERSION]);


	char **url = str_split(requestInfo[HTTP_URL], "?");
	
	// Setting path
	httpRequest.path = (char *)malloc(strlen(url[0]) + 1);
	strcpy(httpRequest.path, url[0]);

	// Setting query
	if(str_len(url) > 1){
		char **query = str_split(url[1], "&");
		httpRequest.query = dictionary_init(query, "=");
	} else {
		httpRequest.query = NULL;
	}

	// Setting headers
	char **headers = requestLines + 1;
	httpRequest.headers = dictionary_init(headers, ":");

	// Debugging request info
	if(DEBUG == 1){
		printf("Method: %s\n", httpRequest.method);
		printf("Version: %s\n", httpRequest.version);
		printf("Path: %s\n", httpRequest.path);
		printf("Query (Size: %d): \n", dictionary_size(httpRequest.query));
		for(int i = 0; i < dictionary_size(httpRequest.query); i++){
			printf("\t%s: %s\n", httpRequest.query[i]->key, httpRequest.query[i]->value);
		}
		printf("Headers (Size: %d): \n", dictionary_size(httpRequest.headers));
		for(int i = 0; i < dictionary_size(httpRequest.headers); i++){
			printf("\t%s: %s\n", httpRequest.headers[i]->key, httpRequest.headers[i]->value);
		}
		printf("\n");
	}

	free(request);
	str_free(url);
	str_free(requestInfo);
	str_free(requestLines);

	httpRequest.code = OK;
	return &httpRequest;
}

/**
 * Starts the listening of the server
 * @param server	-> The server struct
 * @param callback	-> The callback called after a request is parsed
 * @return Returns -1 if an error occures
 */
int server_listen(Server *server, void (*callback)(HTTPRequest *request, HTTPResponse *response)){
	long valread;
	int addressLength = sizeof(server->address);

	// Binding
	if(bind(server->fd, (struct sockaddr *)&server->address, addressLength) < 0){
		perror("Error in binding socket");
		return -1;
	} 

	if(listen(server->fd, 10) < 0){
		perror("Error in starting server");
		return -1;
	}

	while(1){
		printf("--------------Waiting for connection--------------\n");

		int newSocket = accept(server->fd, (struct sockaddr *)&server->address, (socklen_t *)&addressLength);
		if(newSocket < 0){
			perror("Error in accepting connection");
			return -1;
		}
		printf("---------------Connection accepted----------------\n");

		char *buffer = (char *)malloc(server->requestLength + 2);
		valread = read(newSocket, buffer, server->requestLength);
		printf("Got request of %ld bytes\n", valread);

		HTTPRequest *request = server_parseRequest(buffer);
		HTTPResponse *response = server_initHTTPResponse(newSocket, request);
		callback(request, response);

		free(buffer);

		server_freeHTTPRequest(request);
		server_freeHTTPResponse(response);
		close(newSocket);
	}

	return 0;
}

/**
 * Creates the socket and sets a address
 * @param server	-> The server struct
 * @param PORT		-> The server port
 * @return Returns a -1 if an error occures :)
 */
int server_create(Server *server, const int PORT){

	server->fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server->fd == 0){
		perror("Error in creating socket");
		return -1;
	}

	server->address.sin_family = AF_INET;
	server->address.sin_addr.s_addr = INADDR_ANY;
	server->address.sin_port = htons(PORT);
	memset(server->address.sin_zero, '\0', sizeof(server->address.sin_zero));

	server->requestLength = DEFAULT_REQUEST_LENGTH;

	int option = 1;
	setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	// Initializing MIME types
	MIMETypes = dictionary_init((char **)MIMETypesInit, "=");
	// Initializing Status codes
	statusCodes = dictionary_init((char **)StatusCodesInit, "=");
	
	return 0;
}

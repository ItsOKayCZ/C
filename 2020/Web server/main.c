#include <stdio.h>

#include "Server.h"

#define PORT 8080

void onRequest(HTTPRequest *request, HTTPResponse *response);

int main(int argc, char *argv[]){

	printf("Creating server...\n");
	Server server;
	server_create(&server, PORT);
	printf("Server listening on port %d\n", PORT);
	server_listen(&server, onRequest);

	return 0;
}

/**
 * Is called when a request is issued
 * @param request	-> The request
 * @param response	-> The response
 */
void onRequest(HTTPRequest *request, HTTPResponse *response){

	printf("Sending response\n");
	server_sendResponse(response);
}

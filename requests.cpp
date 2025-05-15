#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(const char *host, const char *url, char *query_params,
							char *cookie, char *jwt)
{
	char *message = (char *) calloc(BUFLEN, sizeof(char));
	char *line = (char *) calloc(LINELEN, sizeof(char));

	if (query_params) {
		sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
	} else {
		sprintf(line, "GET %s HTTP/1.1", url);
	}

	compute_message(message, line);

	memset(line, 0, LINELEN);
	if (host) {
		sprintf(line, "Host: %s", host);
		compute_message(message, line);
	}
	
	if (cookie) {
	   sprintf(line, "Cookie: session=%s", cookie);
	   compute_message(message, line);
	}

	if (jwt) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}
	
	compute_message(message, "");
	free(line);
	return message;
}

char *compute_put_request(const char *host, const char *url, const char* content_type, char **body_data,
							int body_data_fields_count, char* cookie, char *jwt)
{
	char *message = (char *) calloc(BUFLEN, sizeof(char));
	char *line = (char *) calloc(LINELEN, sizeof(char));
	char *body_data_buffer = (char *) calloc(LINELEN, sizeof(char));

	sprintf(line, "PUT %s HTTP/1.1", url);
	compute_message(message, line);

	if (host) {
		sprintf(line, "Host: %s", host);
		compute_message(message, line);
	}

	uint32_t content_length = 0;
	for (int i = 0; i < body_data_fields_count; i++) {

		if (content_length + strlen(body_data[i]) + 1 > LINELEN)
			break;

		content_length += strlen(body_data[i]);
		strcat(body_data_buffer, body_data[i]);

		if (i < body_data_fields_count - 1) {
			strcat(body_data_buffer, "&");
			content_length += 1;
		}
	}
	
	sprintf(line, "Content-type: %s", content_type);
	compute_message(message, line);

	sprintf(line, "Content-length: %lu", strlen(body_data_buffer));
	compute_message(message, line);
	
	if (cookie) {
		sprintf(line, "Cookie: session=%s", cookie);
		compute_message(message, line);
	}

	if (jwt) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}

	compute_message(message, "");

	memset(line, 0, LINELEN);
	strcat(message, body_data_buffer);

	free(line);
	free(body_data_buffer);
	return message;
}

char *compute_post_request(const char *host, const char *url, const char* content_type, char **body_data,
							int body_data_fields_count, char *cookie, char *jwt)
{
	char *message = (char *) calloc(BUFLEN, sizeof(char));
	char *line = (char *) calloc(LINELEN, sizeof(char));
	char *body_data_buffer = (char *) calloc(LINELEN, sizeof(char));

	sprintf(line, "POST %s HTTP/1.1", url);
	compute_message(message, line);

	if (host) {
		sprintf(line, "Host: %s", host);
		compute_message(message, line);
	}

	uint32_t content_length = 0;
	for (int i = 0; i < body_data_fields_count; i++) {

		if (content_length + strlen(body_data[i]) + 1 > LINELEN)
			break;

		content_length += strlen(body_data[i]);
		strcat(body_data_buffer, body_data[i]);

		if (i < body_data_fields_count - 1) {
			strcat(body_data_buffer, "&");
			content_length += 1;
		}
	}
	
	sprintf(line, "Content-type: %s", content_type);
	compute_message(message, line);

	sprintf(line, "Content-length: %lu", strlen(body_data_buffer));
	compute_message(message, line);

	if (cookie) {
		sprintf(line, "Cookie: session=%s", cookie);
		compute_message(message, line);
	}

	if (jwt) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}

	compute_message(message, "");
	memset(line, 0, LINELEN);
	strcat(message, body_data_buffer);

	free(line);
	free(body_data_buffer);
	return message;
}

char *compute_delete_request(const char *host, const char *url, char *query_params,
							char *cookie, char *jwt) {
	
	char *message = (char *) calloc(BUFLEN, sizeof(char));
	char *line = (char *) calloc(LINELEN, sizeof(char));

	if (query_params) {
		sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
	} else {
		sprintf(line, "DELETE %s HTTP/1.1", url);
	}

	compute_message(message, line);

	memset(line, 0, LINELEN);
	if (host) {
		sprintf(line, "Host: %s", host);
		compute_message(message, line);
	}
	
	if (cookie) {
	   sprintf(line, "Cookie: session=%s", cookie);
	   compute_message(message, line);
	}

	if (jwt) {
		sprintf(line, "Authorization: Bearer %s", jwt);
		compute_message(message, line);
	}

	compute_message(message, "");
	free(line);
	return message;
}
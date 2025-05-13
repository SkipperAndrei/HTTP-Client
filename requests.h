#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(const char *host, const char *url, char *query_params,
							char *cookie);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(const char *host, const char *url, const char* content_type, char **body_data,
							int body_data_fields_count, char* cookie);

// computes and returns a DELETE request string (query_params and cookie are nullable)
char *compute_delete_request(const char *host, const char *url, char *query_params,
							char *cookie);

#endif

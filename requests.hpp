#ifndef _REQUESTS_
#define _REQUESTS_

#include <bits/stdc++.h>
using namespace std;

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
                        char **cookies, int cookies_count, bool access_library);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, 
            vector<pair<string, string>> body_data, int body_data_fields_count, 
            char **cookies, int cookies_count, bool access_library);

// computes and returns a DELETE request string (cookies can be NULL if not needed)
char *compute_delete_request(char *host, char *url, char* content_type, 
            vector<pair<string, string>> body_data, int body_data_fields_count, 
            char **cookies, int cookies_count, bool access_library);

#endif
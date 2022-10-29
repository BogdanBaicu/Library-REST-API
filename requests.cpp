#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "requests.hpp"

char *compute_get_request(char *host, char *url, char *query_params,
                        char **cookies, int cookies_count, bool access_library)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s/%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);
    
    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        // daca este autorizarea JWT
        if(access_library) {
            sprintf(line, "Authorization: Bearer %s", cookies[0]);
            strcat(message, line);
        } else{
            sprintf(line, "Cookie: ");
            strcat(message, line);

            for (int i = 0; i < cookies_count; i++) {
                sprintf(line, "%s; ", cookies[i]);
                strcat(message, line);
            }
        }
        compute_message(message, "");
    }
    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, 
            vector<pair<string, string>> body_data, int body_data_fields_count, 
            char **cookies, int cookies_count, bool access_library)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char*) calloc(LINELEN, sizeof(char));
    int i;

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1",  url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    strcpy(body_data_buffer, "");

    // Step 3: add the payload
    if(body_data_fields_count > 0){
        strcat(body_data_buffer,"{");
        
        for(i = 0; i < body_data_fields_count; i++){
            string aux = "\"" + body_data[i].first + "\": " +
                            "\"" + body_data[i].second + "\"";
            strcat(body_data_buffer, &aux[0]);

            if(i != body_data_fields_count - 1)
                strcat(body_data_buffer, ",");
        }
        strcat(body_data_buffer, "}\r\n");
    }

    // Step 4: add necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    compute_message(message, line);

    // Step 5 (optional): add cookies
    if (cookies != NULL) {
        if(access_library){
            sprintf(line, "Authorization: Bearer %s", cookies[0]);
            strcat(message, line);
        } else {
            sprintf(line, "Cookie: ");
            strcat(message, line);

            for(i = 0; i < cookies_count; i++) {
                sprintf(line, "%s; ", cookies[i]);
                strcat(message, line);
            }
        }
        compute_message(message,"");   
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}


char *compute_delete_request(char *host, char *url, char* content_type, 
            vector<pair<string, string>> body_data, int body_data_fields_count, 
            char **cookies, int cookies_count, bool access_library)
{
    char *message = (char*) calloc(BUFLEN, sizeof(char));
    char *line = (char*) calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char*) calloc(LINELEN, sizeof(char));
    int i;

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    strcpy(body_data_buffer, "");

    // Step 3: add the payload
    if(body_data_fields_count > 0){
        strcat(body_data_buffer,"{");
        
        for(i = 0; i < body_data_fields_count; i++){
            string aux = "\"" + body_data[i].first + "\": " +
                            "\"" + body_data[i].second + "\"";
            strcat(body_data_buffer, &aux[0]);

            if(i != body_data_fields_count - 1)
                strcat(body_data_buffer, ",");
        }
        strcat(body_data_buffer, "}\r\n");
    }

    // Step 4: add necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    compute_message(message, line);

    // Step 5 (optional): add cookies
    if (cookies != NULL) {
        if(access_library){
            sprintf(line, "Authorization: Bearer %s", cookies[0]);
            strcat(message, line);
        } else {
            sprintf(line, "Cookie: ");
            strcat(message, line);

            for(i = 0; i < cookies_count; i++) {
                sprintf(line, "%s; ", cookies[i]);
                strcat(message, line);
            }
        }
        compute_message(message,"");   
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
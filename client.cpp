#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/socket.h>
#include "helpers.hpp"
#include "requests.hpp"

using namespace std;
int sockfd;
char *message;
char *response;
string cookie;
string access_library;

// realieaza conexiunea prin socket
void connect_socket(){
    sockfd = open_connection((char*)HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        exit_error((char*)"EROARE: Nu s-a putut deschide socketul\n");
}

// functie ce afla response code-ul din raspunsul serverului
int get_response_code(char *response){
    char response_code[strlen(response) + 1];
    // copiez in response code mesajul de la al 9-lea caracter
    strcpy(response_code, (response + 9));

    // daca am mai mult de 4 caractere, adaug caracterul de sfarsit de text
    if(strlen(response_code) >= 4)
        response_code[4] ='\0';
    
    // realizez conversie la int
    return atoi(response_code);
}

// functie pentru parsarea cookie urilor
char* get_connection_cookie(char *response){
    char *token = strstr(response, "Set-Cookie");
    // folosesc un token pe care il duc la prima aparitie a lui "Set-Cookie"
    if(token!=NULL && strlen(token) > 12){
        // daca am tokenul si este suficient de lung, il deplasez 12 pozitii
        token = token +12;
        // trunchez tokenul
        int pozitie;
        pozitie = strstr(token,";") -token;
        token[pozitie] = '\0';

    }
    // daca tokenul corespunde, il returnez
    if(token != NULL && strlen(token) >= 12)
        return token;
    return (char*) "";
}

// functie pentru parsarea JWT
char* get_access_token(char* response) {
    char *token = strstr(response, "token");
    // folosesc un token pe care il duc la prima aparitie a lui "token"
    if (token != NULL && strlen(token) > 8) {
        // daca am tokenul si este suficient de lung, il deplasez 8 pozitii
        token = token + 8;
        // trunchez tokenul
        token[strlen(token) - 2] = '\0';
    }
    // daca tokenul corespunde, il returnez
    if(token != NULL && strlen(token) >= 8)
        return token;
    return (char*) "";
}

// functie pentru parsarea payload ului
char* get_payload(char *response){
    char *token = strstr(response, "[");
    // folosesc un token pe care il duc la prima aparitie a lui "["
    if (token != NULL && strlen(token) > 1) {
        // daca am tokenul si este suficient de lung, il deplasez 1 pozitie
        token = token + 1;
        int pozitie = strstr(token, "]") - token;
        // trunchez tokenul
        token[pozitie] = '\0';
    }
    // daca tokenul corespunde, il returnez
    if(token != NULL && strlen(token) >= 1)
        return token;
    return (char*) "";
}

// functie pentru inregistrarea utilizatorului
void register_user(string username, string password) {
    // vectorul de perechi ce contine credentialele de autentificare
    vector<pair<string, string>> payload = {{"username", username},
                                            {"password", password}};

    int response_code;
    // ma conectez la server
    connect_socket();
    // compun si trimit requestul
    message = compute_post_request((char*)HOST, (char*)CALE_REGISTER, 
                (char*) PAYLOAD_TYPE, payload, payload.size(), NULL, 0, false);
    
    send_to_server(sockfd, message);

    // primesc raspunsul
    response = receive_from_server(sockfd);

    // analizez rapsunsul in functie de codul de eroare
    if (strlen(response)){
        response_code = get_response_code(response);
        if(200 <= response_code && response_code <300)
            cout << "CORECT: Utilizatorul a fost inregistrat cu succes (" << 
                    response_code << ")\n";
        else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    } else
        cout << "CORECT: Utilizatorul a fost inregistrat cu succes\n";

    close(sockfd);    
}

// functie pentru logarea utilizatorului
void login_user(string username, string password){
    // vectorul de perechi ce contine credentialele de autentificare
    vector<pair<string, string>> payload = {{"username", username},
                                            {"password", password}};

    int response_code;
    // ma conectez la server
    connect_socket();
    // compun si trimit requestul
    message = compute_post_request((char*)HOST, (char*)CALE_LOGIN, 
                (char*) PAYLOAD_TYPE, payload, payload.size(), NULL, 0, false);

    send_to_server(sockfd, message);
    // primesc raspunsul
    response = receive_from_server(sockfd);
    // analizez rapsunsul in functie de codul de eroare si daca este cazul
    // actualizez cookie-urile si JWT
    if (strlen(response)){
        response_code = get_response_code(response);
        if(response_code == 200){
            cout << "CORECT: Utilizatorul s-a conectat cu succes (" << 
                    response_code << ")\n";
            cookie = get_connection_cookie(response);
            access_library = "";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    }

    close(sockfd); 
}

// functie pentru logoutlui utilizatorului
void logout_user(){
    char **cookies = (char**) malloc(sizeof(char*));
    int response_code;

    cookies[0] = strdup(cookie.c_str());
    // ma conectez la server
    connect_socket();
    // compun si trimit requestul
    message = compute_get_request((char*)HOST, (char*) CALE_LOGOUT,
                                NULL, cookies, 1, false);

    send_to_server(sockfd, message);
    // primesc mesajul si il analizez in functie de response code
    response = receive_from_server(sockfd);

    if (strlen(response)) {
        response_code = get_response_code(response);

        if (response_code == 200) {
            // sterg cookies si JWT
            access_library = "";
            cookie = "";
            cout << "CORECT: Utilizatorul s-a deconectat\n";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    } else {
        access_library = "";
        cookie = "";
    }

    close(sockfd);
}

// functie pentru accesarea bibliotecii
void enter_library(){
    // actualizez cookies
    char **cookies = (char**) malloc(sizeof(char*));
    int response_code;

    cookies[0] = strdup(cookie.c_str());
    // ma conectez la server
    connect_socket();
    // compun si transmit requestul
    message = compute_get_request((char*)HOST, (char*) CALE_ACCES_BIBLIOTECA, NULL, 
                cookies, 1, false);
    
    send_to_server(sockfd, message);

    // primesc si analizez raspunsul
    response =  receive_from_server(sockfd);

    if (strlen(response)){
        response_code = get_response_code(response);
        if(response_code == 200){
            // actualizez autorizarea JWT
            access_library = get_access_token(response);
            cout << "CORACT: S-a putut accesa biblioteca\n";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    }

    close(sockfd);
}

// functie pentru adaugarea unei carti
void add_book(){
    string title, author, genre, publisher, page_count;
    int response_code;
    // actualizez cookie-urile
    char **cookies = (char**) malloc(sizeof(char*));

    cookies[0] = strdup(access_library.c_str());
    cin.get();
    cout << "title=";
    getline(cin, title);
    cout << "author=";
    getline(cin, author);
    cout << "genre=";
    getline(cin, genre);
    cout << "publisher=";
    getline(cin, publisher);
    cout << "page_count=";
    getline(cin, page_count);

    if(title.size() < 1)
        title = "";
    if(author.size() < 1)
        author = "";
    if(genre.size() < 1)
        genre = "";
    if(publisher.size() < 1)
        publisher = "";
    if(page_count.size() < 1)
        page_count = "";

    // vectorul cu perechi de date cu informatii despre carte
    vector<pair<string,string>> payload = {
        {"title", title},
        {"author", author},
        {"genre", genre},
        {"publisher", publisher},
        {"page_count", page_count}
    };

    // ma conectez la server
    connect_socket();
    // compun si transmit mesajul
    message = compute_post_request((char*) HOST, (char*) CALE_CARTI, 
                    (char*) PAYLOAD_TYPE, payload, payload.size(), cookies, 1, true);
    
    send_to_server(sockfd, message);
    // primesc si analizez raspunsul in functie de response code
    response = receive_from_server(sockfd);

    if (strlen(response)) {
        response_code = get_response_code(response);

        if (response_code == 200) {
           cout << "CORECT: Cartea a fost adaugata\n";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    } 

    close(sockfd);
}

// functie pentru afisarea cartilor
void get_books(){
    // actualizez cookie urile
    char **cookies = (char**) malloc(sizeof(char*));
    int response_code;

    cookies[0] = strdup(access_library.c_str());
    // ma conectez la server
    connect_socket();
    // compun si transmi mesajul
    message = compute_get_request((char*)HOST, (char*) CALE_CARTI, NULL, cookies, 1, true);

    send_to_server(sockfd, message);
    // primesc si analizez raspunsul
    response = receive_from_server(sockfd);

    if (strlen(response)) {
        response_code = get_response_code(response);

        if (response_code == 200) {
            // iau payload-ul si il parsez
            get_payload(response);
            if(strlen(response)){
                cout << "CORECT: S-a putut extrage lista de carti\n";
                cout << "[" << response << "]\n";
            } else 
                cout << "CORECT: Lista de carti este goala\n";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    } 

    close(sockfd);
}

// functie pentru afisarea informatiilor unei carti
void get_book(string id){
    // actualizez cookie urile
    char **cookies = (char**) malloc(sizeof(char*));
    int response_code;

    cookies[0] = strdup(access_library.c_str());

    // ma conectez la server
    connect_socket();
    // compun si transmit mesajul
    message = compute_get_request((char*)HOST, (char*)CALE_CARTI,
                                (char *)id.c_str(), cookies, 1, true);

    send_to_server(sockfd, message);
    // primesc si analizez raspusnul in functie de response code
    response = receive_from_server(sockfd);

    if (strlen(response)) {
        response_code = get_response_code(response);

        if (response_code == 200) {
            // afisez payload ul
            response = get_payload(response);
            cout << "CORECT: Cartea a fost gasita\n";
            cout << response <<"\n";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    } 
    close(sockfd);
}

// functie pentru stergerea unei carti
void delete_book(string id){
    // actalizez cookie urile
    char **cookies = (char**) malloc(sizeof(char*));
    int response_code;
    string route = string(CALE_CARTI) + "/" +id;

    cookies[0] = strdup(access_library.c_str());
    // ma conectez la serve
    connect_socket();
    // compun si transmit mesjaul
    message = compute_delete_request((char*) HOST, (char *) route.c_str(), (char*)PAYLOAD_TYPE,
                                {}, 0, cookies, 1, true);

    send_to_server(sockfd, message);
    // primesc raspunsul si il analizez in functie de response code
    response = receive_from_server(sockfd);

    if (strlen(response)) {
        response_code = get_response_code(response);

        if (response_code == 200) {
            cout << "CORECT: Cartea a fost stearsa\n";
        } else if(response_code == 400)
            cout << "EROARE: Bad request (error code 400)\n";
        else if(response_code == 404)
            cout << "EROARE: Error 404 not found\n";
        else if(response_code == 429)
            cout << "EROARE: Prea multe incercari, incercati mai tarziu (" <<
            "error code 429)\n";
        else 
            cout << "EROARE: Error code " << response_code <<"\n";
    } 
    close(sockfd);
}

int main(){
    while (1){
        char *comanda = (char*) malloc(COMMANDLEN * sizeof(char));
        fscanf(stdin, "%s", comanda);
        
        // tratez comanda primita
        if(strcmp(comanda, "exit") == 0){
            close(sockfd);
            return 0;
        } else if(strcmp(comanda, "register") == 0){
            cin.get();
            string username;
            string password;
            cout << "username=";
            getline(cin, username);
            cout << "password=";
            getline(cin, password);

            register_user(username, password);
        } else if (strcmp(comanda, "login") == 0){
            cin.get();
            string username;
            string password;
            cout << "username=";
            getline(cin, username);
            cout << "password=";
            getline(cin, password);
            login_user(username, password);
        } else if (strcmp(comanda, "logout") == 0){
            logout_user();
        } else if (strcmp(comanda, "enter_library") == 0){
            enter_library();
        } else if (strcmp(comanda, "add_book") == 0){
            add_book();
        } else if (strcmp(comanda, "get_book") == 0){
            string id;
            cin.get();
            cout << "id=";
            getline(cin, id);
            get_book(id);
        } else if (strcmp(comanda, "get_books") == 0){
            get_books();
        } else if (strcmp(comanda, "delete_book") == 0){
            string id;
            cin.get();
            cout << "id=";
            getline(cin, id);
            delete_book(id);
        } else
            cout << "Comanda necunoscuta\n";
    }
    
    return 0;
}
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "rpc_authentication.h"

using namespace std;

#define DELIM ','

// the structure describing a user operation
typedef struct operation {
    string user_id;
    string action;
    string param;
} operation_t;

// map that keeps the users and their request authentication tokens
unordered_map <string, string> user_auth_req_tokens;

// map that keeps the users and their access tokens
unordered_map <string, string> user_access_tokens;

// map that keeps the users and their refresh tokens
unordered_map <string, string> user_refresh_tokens;

// map that keeps the users and their autorefresh option
unordered_map <string, int> user_autorefresh_token;

// map that keeps the access tokens and their refresh tokens
unordered_map <string, int> token_valability;

// function used to parse an operation from the input and return it as a structure
operation_t parse_operation(string line) {
    operation_t op;

    // split the line by ","
    int pos = 0;
    string token;

    pos = line.find(DELIM);
    op.user_id = line.substr(0, pos);
    line.erase(0, pos + 1);

    pos = line.find(DELIM);
    op.action = line.substr(0, pos);
    line.erase(0, pos + 1);

    op.param = line;

    return op;
}

// function used to read the input file and return a vector of operations
vector<operation_t> read_input(string filename) {
    ifstream infile(filename.c_str());
    string line;
    vector<operation_t> operations;

    while (getline(infile, line)) {
        operation_t op = parse_operation(line);
        operations.push_back(op);
    }

    infile.close();
    return operations;
}

// function used for request entries
void execute_request(CLIENT *clnt, string user_id, string param) {
    char *input_user_id = (char *)user_id.c_str();

    if (param == "0") {
        user_autorefresh_token[user_id] = 0;
    } else if (param == "1") {
        user_autorefresh_token[user_id] = 1;
    }

    // request the authorization token
    struct request_authorization_response *authorization_response = request_authorization_1(&input_user_id, clnt);

    if (authorization_response == NULL) {
        clnt_perror(clnt, "request authorization call failed");
    } else if (authorization_response->status == USER_NOT_FOUND) {
        cout << "USER_NOT_FOUND" << endl;
        return;
    }

    // extract the authorization token from the response
    char* auth_req_token = authorization_response->authorization_token;
    user_auth_req_tokens[user_id] = auth_req_token;

    // now the end user has to approve the request token by looking in the permission file
    operation_status_t *status = aprove_request_token_1(&auth_req_token, clnt);
    if (status == NULL) {
        clnt_perror(clnt, "approve request token call failed");
    }

    // create the request struct for requesting the access token
    struct access_token_request access_token_request;
    access_token_request.user_id = input_user_id;
    access_token_request.authorization_token = auth_req_token;
    access_token_request.autorefresh = user_autorefresh_token[user_id];

    struct access_token_response *access_token_response = request_access_token_1(&access_token_request, clnt);

    if (access_token_response == NULL) {
        clnt_perror(clnt, "request access call failed");
    } else if (access_token_response->status == REQUEST_DENIED) {
        // the authorization token was not previously signed
        cout << "REQUEST_DENIED" << endl;
        return;
    }

    string access_token(access_token_response->access_token);
    user_access_tokens[user_id] = access_token;
    token_valability[access_token] = access_token_response->token_expiration;

    string refresh_token(access_token_response->refresh_token);
    user_refresh_tokens[user_id] = refresh_token;

    cout << auth_req_token << " -> " << access_token;
    if (refresh_token != "") {
        cout << "," << refresh_token;
    }
    cout << endl;
}

// function used for operation entries
void execute_operation(CLIENT *clnt, string user_id, string action, string param) {
    char *input_user_id = (char *)user_id.c_str();
    char *input_action = (char *)action.c_str();
    char *input_resource = (char *)param.c_str();

    // check if the access token is still valid
    if (token_valability[user_access_tokens[user_id]] == 0) {
        // the token expired, request a new one if the user has autorefresh enabled
        if (user_autorefresh_token[user_id] == 1) {
            struct refresh_token_request request;
            request.access_token = (char *)user_access_tokens[user_id].c_str();
            request.refresh_token = (char *)user_refresh_tokens[user_id].c_str();

            struct refresh_token_response *response = refresh_token_operation_1(&request, clnt);
            string new_access_token(response->new_access_token);
            string new_refresh_token(response->new_refresh_token);

            user_access_tokens[user_id] = new_access_token;
            user_refresh_tokens[user_id] = new_refresh_token;
            
            // delete the old access token and add the new one
            token_valability.erase(request.access_token);
            token_valability[new_access_token] = response->token_expiration;
        }
    }

    // now that the access token is valid, the operation can be executed
    struct validate_delegated_action_request request;
    request.operation_type = input_action;
    request.accessed_resource = input_resource;
    request.access_token = (char *)user_access_tokens[user_id].c_str();

    struct validate_delegated_action_response *response = validate_delegated_action_1(&request, clnt);

    // decrement the token valability
    token_valability[user_access_tokens[user_id]]--;

    // check the status of the operation
    if (response->status == PERMISSION_DENIED) {
        cout << "PERMISSION_DENIED" << endl;
    } else if (response->status == TOKEN_EXPIRED) {
        cout << "TOKEN_EXPIRED" << endl;
    } else if (response->status == RESOURCE_NOT_FOUND) {
        cout << "RESOURCE_NOT_FOUND" << endl;
    } else if (response->status == OPERATION_NOT_PERMITTED) {
        cout << "OPERATION_NOT_PERMITTED" << endl;
    } else if (response->status == PERMISSION_GRANTED) {
        cout << "PERMISSION_GRANTED" << endl;
    }
}


void authentication_prog_1(string host, vector<operation_t> operations) {
    CLIENT *clnt;
    clnt = clnt_create(host.c_str(), AUTHENTICATION_PROG, AUTHENTICATION_VERS, "tcp");

    if (clnt == NULL) {
        clnt_pcreateerror(host.c_str());
        exit(1);
    }

    for (operation_t op : operations) {
        if (op.action == "REQUEST") {
            execute_request(clnt, op.user_id, op.param);
        } else {
            execute_operation(clnt, op.user_id, op.action, op.param);
        }
    }

    clnt_destroy(clnt);
}


int main(int argc, char* argv[])
{
    string host;
    string filename;
    vector<operation_t> operations;

	if (argc < 3) {
        cout << "Usage: " << argv[0] << " <server_host> <client_in_cvs_file>" << endl;
		return 1;
	}
	host = argv[1];
    filename = argv[2];

    // parse the input file with all the operations
    operations = read_input(filename);

    // execute the operations
    authentication_prog_1(host, operations);

    return 0;
}

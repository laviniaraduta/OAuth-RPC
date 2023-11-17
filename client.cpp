#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "rpc_authentication.h"
#include <unordered_map>

using namespace std;

#define DELIM ','

typedef struct operation {
    string user_id;
    string action;
    string param;
} operation_t;


unordered_map <string, string> user_auth_req_tokens;
unordered_map <string, string> user_access_tokens;
unordered_map <string, string> user_refresh_tokens;
unordered_map <string, int> user_autorefresh_token;
unordered_map <string, int> token_valability;

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

void execute_request(CLIENT *clnt, string user_id, string param) {
    char *input_user_id = (char *)user_id.c_str();

    if (param == "0") {
        user_autorefresh_token[user_id] = 0;
    } else if (param == "1") {
        user_autorefresh_token[user_id] = 1;
    }

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
        cout << " , " << refresh_token;
    }
    cout << endl;
}

void execute_operation(CLIENT *clnt, string user_id, string action, string param) {
    char *input_user_id = (char *)user_id.c_str();
    char *input_action = (char *)action.c_str();
    char *input_resource = (char *)param.c_str();

    struct validate_delegated_action_request request;
    request.operation_type = input_action;
    request.accessed_resource = input_resource;
    request.access_token = (char *)user_access_tokens[user_id].c_str();

    struct validate_delegated_action_response *response = validate_delegated_action_1(&request, clnt);

    if (response->status == PERMISSION_DENIED) {
        cout << "PERMISSION_DENIED" << endl;
        return;
    } else if (response->status == TOKEN_EXPIRED) {
        cout << "TOKEN_EXPIRED" << endl;
        return;
    } else if (response->status == RESOURCE_NOT_FOUND) {
        cout << "RESOURCE_NOT_FOUND" << endl;
        return;
    } else if (response->status == OPERATION_NOT_PERMITTED) {
        cout << "OPERATION_NOT_PERMITTED" << endl;
        return;
    } else if (response->status == PERMISSION_GRANTED) {
        cout << "PERMISSION_GRANTED" << endl;
        return;
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
            // cout << "request" << endl;
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
		printf ("usage: %s server_host cvs_file\n", argv[0]);
		exit (1);
	}
	host = argv[1];
    filename = argv[2];

    operations = read_input(filename);

    authentication_prog_1(host, operations);

    return 0;
}

#include "user_database.h"
#include "rpc_authentication.h"
#include <iostream>

unordered_set <string> approved_tokens;
unordered_map <string, vector<permission_t>> auth_token_permissions;

// function used to sign the authorization tokens and associate them with the permissions
operation_status_t * aprove_request_token_1_svc(char **auth_req_token, struct svc_req *) {
    static operation_status_t response;
    
    vector <permission_t> user_perms = permissions.front();
    permissions.pop();

    string auth_token(*auth_req_token);

    if (user_perms.size() != 0) {
        approved_tokens.insert(auth_token);
        response = SIGNED;

        // associate the request token with the permissions
        auth_token_permissions[auth_token] = user_perms;
        return &response;
    } else {
        response = NOT_SIGNED;
        return &response;
    }
}
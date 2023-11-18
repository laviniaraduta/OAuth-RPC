#include "user_database.h"
#include "rpc_authentication.h"
#include "token.h"
#include <iostream>

unordered_map <string, string> user_tokens;
unordered_map <string, int> access_token_valability;
unordered_map <string, string> access_tokens_refresh_tokens;
unordered_map <string, string> access_req_tokens;

// function used to generate an authorization request token
struct request_authorization_response *request_authorization_1_svc(char **user_id, struct svc_req *rqstp) {
    static struct request_authorization_response *response;
    if (response) {
        free(response);
    }

    response = (struct request_authorization_response *)malloc(sizeof(*response));

    cout << "BEGIN " << *user_id << " AUTHZ" << endl;

    if (user_ids.find(*user_id) != user_ids.end()) {
        response->status = USER_FOUND;
        response->authorization_token = generate_access_token(*user_id);
        cout << "  RequestToken = " << response->authorization_token << endl;

        // add the new user in the user database with the request token
        user_tokens[*user_id] = response->authorization_token;
        return response;
    } else {
        response->status = USER_NOT_FOUND;
        response->authorization_token = strdup("");
        return response;
    }
}

// function used to generate an access token
struct access_token_response *request_access_token_1_svc(struct access_token_request *request, struct svc_req *rqstp) {
    static struct access_token_response *response;

    if (response) {
        free(response);
    }

    response = (struct access_token_response *)malloc(sizeof(*response));
    char *auth_token = request->authorization_token;
    if (approved_tokens.find(auth_token) != approved_tokens.end()) {
        char *access_token = generate_access_token(auth_token);

        // keep the connection between authentication request token and access token
        access_req_tokens[access_token] = auth_token;

        // update the request auth token with the access token
        user_tokens[request->user_id] = access_token;
        cout << "  AccessToken = " << access_token << endl;

        // generate the refresh token only if the user has autorefresh enabled
        if (request->autorefresh) {
            char *refresh_token = generate_access_token(access_token);
            access_tokens_refresh_tokens[access_token] = refresh_token;
            access_token_valability[access_token] = token_valability;
            cout << "  RefreshToken = " << refresh_token << endl;

            response->status = ACCESS_GRANTED;
            response->access_token = access_token;
            response->refresh_token = refresh_token;
            response->token_expiration = token_valability;
            return response;
        } else {
            access_token_valability[access_token] = token_valability;
            response->status = ACCESS_GRANTED;
            response->refresh_token = strdup("");
            response->access_token = access_token;
            response->token_expiration = 0;
            return response;
        }
    } else {
        response->status = REQUEST_DENIED;
        response->access_token = strdup("");
        response->refresh_token = strdup("");
        response->token_expiration = 0;
        return response;
    }
}

struct refresh_token_response * refresh_token_operation_1_svc(struct refresh_token_request *request, struct svc_req *rqstp) {
    static struct refresh_token_response *response;

    if (response) {
        free(response);
    }

    response = (struct refresh_token_response *)malloc(sizeof(*response));

    char *old_access_token = request->access_token;
    char *old_refresh_token = request->refresh_token;

    // traverse the user_tokens map to find the user that has the old access token
    string user_id;
    for (auto it = user_tokens.begin(); it != user_tokens.end(); it++) {
        if (it->second == old_access_token) {
            user_id = it->first;
            break;
        }
    }

    cout << "BEGIN " << user_id << " AUTHZ REFRESH" << endl;

    char *new_access_token = generate_access_token(old_refresh_token);
    char *new_refresh_token = generate_access_token(new_access_token);

    cout << "  AccessToken = " << new_access_token << endl;
    cout << "  RefreshToken = " << new_refresh_token << endl;

    // update the user_tokens map with the new access token
    user_tokens[user_id] = new_access_token;

    // update the access_req_tokens map with the new access token
    access_req_tokens[new_access_token] = access_req_tokens[old_access_token];
    access_req_tokens.erase(old_access_token);

    // update the access_tokens_refresh_tokens map with the new access token
    access_tokens_refresh_tokens[new_access_token] = new_refresh_token;
    access_tokens_refresh_tokens.erase(old_access_token);

    // update the access_tokens_with_autorefresh map with the new access token
    access_token_valability[new_access_token] = token_valability;
    access_token_valability.erase(old_access_token);

    response->new_access_token = new_access_token;
    response->new_refresh_token = new_refresh_token;
    response->token_expiration = token_valability;

    return response;
}
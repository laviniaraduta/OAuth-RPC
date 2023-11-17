#include "user_database.h"
#include "rpc_authentication.h"
#include "token.h"
#include <iostream>

unordered_map <string, string> user_tokens;
unordered_map <string, int> access_tokens_with_autorefresh;
unordered_map <string, string> access_tokens_refresh_tokens;
unordered_map <string, string> access_req_tokens;

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
        response->authorization_token = "";
        return response;
    }
}

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
            access_tokens_with_autorefresh[access_token] = token_valability;
            cout << "  RefreshToken = " << refresh_token << endl;

            response->status = ACCESS_GRANTED;
            response->access_token = access_token;
            response->refresh_token = refresh_token;
            response->token_expiration = token_valability;
            return response;
        } else {
            access_tokens_with_autorefresh[access_token] = token_valability;
            response->status = ACCESS_GRANTED;
            response->refresh_token = "";
            response->access_token = access_token;
            response->token_expiration = 0;
            return response;
        }
    } else {
        response->status = REQUEST_DENIED;
        response->access_token = "";
        response->refresh_token = "";
        response->token_expiration = 0;
        return response;
    }
}

struct refresh_token_response * refresh_token_operation_1_svc(struct refresh_token_request *request, struct svc_req *rqstp) {
    return NULL;
}
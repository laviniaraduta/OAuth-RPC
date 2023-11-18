#include "user_database.h"
#include "rpc_authentication.h"
#include <iostream>

void print_operation_result(string result, string operation, string resource, string token, int token_valability) {
    cout << result << " ("
        << operation << ","
        << resource << ","
        << token << "," 
        << token_valability << ")" << endl;
}

validate_delegated_action_response * validate_delegated_action_1_svc(struct validate_delegated_action_request *req, struct svc_req *) {
    static struct validate_delegated_action_response *response;

    if (response) {
        free(response);
    }

    response = (struct validate_delegated_action_response *)malloc(sizeof(*response));

    string user_operation(req->operation_type);
    string user_accessed_resource(req->accessed_resource);
    string user_access_token(req->access_token);

    // now check if the access token provided is valid (if it matches the one in the database)
    if (access_req_tokens.find(user_access_token) != access_req_tokens.end()) {

        string req_auth_token = access_req_tokens[user_access_token];
        // the access token was found in the database, now check the token valability
        if (access_token_valability[user_access_token] != 0) {
            // the token is not expired
            // now check if the resource exists
            if (resources.find(user_accessed_resource) != resources.end()) {
                // the resource exists in the database
                // now check if the user has the permission to access the resource

                // get the user permissions
                vector <permission_t> user_permissions = auth_token_permissions[req_auth_token];
                bool has_access = false;

                permission_type_t user_permission_request;

                if (user_operation == "READ") {
                    user_permission_request = READ;
                } else if (user_operation == "INSERT") {
                    user_permission_request = INSERT;
                } else if (user_operation == "MODIFY") {
                    user_permission_request = MODIFY;
                } else if (user_operation == "DELETE") {
                    user_permission_request = DELETE;
                } else if (user_operation == "EXECUTE") {
                    user_permission_request = EXECUTE;
                }

                for (int i = 0; i < user_permissions.size(); i++) {
                    if (user_permissions[i].resource == user_accessed_resource) {
                        unordered_set <permission_type_t> resource_permissions = user_permissions[i].resource_permissions;

                        if (resource_permissions.find(user_permission_request) != resource_permissions.end()) {
                            has_access = true;
                            response->status = PERMISSION_GRANTED;
                            access_token_valability[user_access_token]--;
                            if (access_token_valability[user_access_token] == 0) {
                                access_token_valability.erase(user_access_token);
                            }
                            print_operation_result("PERMIT", user_operation, user_accessed_resource, user_access_token, access_token_valability[user_access_token]);
                            return response;
                        }
                    }
                }

                if (!has_access) {
                    response->status = OPERATION_NOT_PERMITTED;
                    access_token_valability[user_access_token]--;
                    if (access_token_valability[user_access_token] == 0) {
                        access_token_valability.erase(user_access_token);
                    }
                    print_operation_result("DENY", user_operation, user_accessed_resource, user_access_token, access_token_valability[user_access_token]);
                    return response;
                }

            } else {
                response->status = RESOURCE_NOT_FOUND;
                access_token_valability[user_access_token]--;
                if (access_token_valability[user_access_token] == 0) {
                    access_token_valability.erase(user_access_token);
                }
                print_operation_result("DENY", user_operation, user_accessed_resource, user_access_token, access_token_valability[user_access_token]);
                return response;
            }
        } else {
            response->status = TOKEN_EXPIRED;
            access_token_valability[user_access_token]--;
            if (access_token_valability[user_access_token] == 0) {
                access_token_valability.erase(user_access_token);
            }
            print_operation_result("DENY", user_operation, user_accessed_resource, "", 0);
            return response;
        }



    } else {
        response->status = PERMISSION_DENIED;
        print_operation_result("DENY", user_operation, user_accessed_resource, user_access_token, access_token_valability[user_access_token]);
        return response;
    }

    return NULL;

}
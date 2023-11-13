struct request_authorization_response {
    int status;
    string authorization_token<>;
};

struct access_token_request {
    string user_id<>;
    string authorization_token<>;
};

struct access_token_response {
    int status;
    string access_token<>;
    string refresh_token<>;
    int token_expiration;
};

struct validate_delegated_action_request {
    string operation_type<>;
    string accessed_resource<>;
    string access_token<>;
};

struct validate_delegated_action_response {
    int status;
};

struct aprove_request_token_response {
    int status;
};

program AUTHENTICATION_PROG {
    version AUTHENTICATION_VERS {
        struct request_authorization_response request_authorization(string) = 1;
        struct access_token_response request_access_token(struct access_token_request) = 2;
        struct validate_delegated_action_response validate_delegated_action(struct validate_delegated_action_request) = 3;
        struct aprove_request_token_response aprove_request_token(string) = 4;
    } = 1;
} = 1;
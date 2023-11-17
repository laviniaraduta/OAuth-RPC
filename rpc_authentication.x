enum operation_status_t {
    USER_FOUND,
	USER_NOT_FOUND,
    SIGNED,
    NOT_SIGNED,
    ACCESS_GRANTED,
	REQUEST_DENIED,
	PERMISSION_DENIED,
	TOKEN_EXPIRED,
	RESOURCE_NOT_FOUND,
	OPERATION_NOT_PERMITTED,
	PERMISSION_GRANTED
};

struct request_authorization_response {
    operation_status_t status;
    string authorization_token<>;
};

struct access_token_request {
    string user_id<>;
    string authorization_token<>;
    int autorefresh;
};

struct access_token_response {
    operation_status_t status;
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
    operation_status_t status;
};

struct refresh_token_request {
    string refresh_token<>;
};

struct refresh_token_response {
    string new_access_token<>;
    string new_refresh_token<>;
    int token_expiration;
};

program AUTHENTICATION_PROG {
    version AUTHENTICATION_VERS {
        struct request_authorization_response request_authorization(string) = 1;
        struct access_token_response request_access_token(struct access_token_request) = 2;
        struct validate_delegated_action_response validate_delegated_action(struct validate_delegated_action_request) = 3;
        operation_status_t aprove_request_token(string) = 4;
        struct refresh_token_response refresh_token_operation(struct refresh_token_request) = 5;
    } = 1;
} = 1;
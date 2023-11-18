/* this enum is used to define the status of the operations */
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

/* authentication server response for the request_authorization procedure */
struct request_authorization_response {
    /* status of the operation - can be USER_FOUND or USER_NOT_FOUND */
    operation_status_t status;

    /* the authorization token that was generated from the user id */
    string authorization_token<>;
};

/* client request for the request_access_token procedure */
struct access_token_request {
    /* the id of the user that is requesting the access token */
    string user_id<>;

    /* the authorization token that was generated previously */
    string authorization_token<>;

    /* 1 if the user has the autorefresh option enabled, 0 otherwise */
    int autorefresh;
};


/* authentication server response for the request_access_token procedure */
struct access_token_response {
    /* status of the operation - can be ACCESS_GRANTED or REQUEST_DENIED */
    operation_status_t status;

    /* the access token that was generated from the authorization request token */
    string access_token<>;

    /* the refresh token that was generated from the access token
       if the user does not have autorefresh, the refresh token will be empty ("") */
    string refresh_token<>;

    /* the number of operations that can be performed with the access token */
    int token_expiration;
};


/* client request for the validate_delegated_action procedure */
struct validate_delegated_action_request {
    /* what type of operation is the user trying to perform */
    string operation_type<>;

    /* the resource that the user is trying to access */
    string accessed_resource<>;

    /* the access token that was generated previously */
    string access_token<>;
};

/* resource server response for the validate_delegated_action procedure */
struct validate_delegated_action_response {
    /* status of the operation - can be PERMISSION_GRANTED/ OPERATION_NOT_PERMITED/
       RESOUCE_NOT_FOUND/ TOKEN_EXPIRED/ PERMISSION_DENIED */
    operation_status_t status;
};

/* client request for refreshing the access and refresh tokens */
struct refresh_token_request {
    /* the old refresh token */
    string refresh_token<>;

    /* the old access token */
    string access_token<>;
};

/* authentication server response for the refresh_token_operation procedure */
struct refresh_token_response {
    /* the new access token generated from the old refresh token */
    string new_access_token<>;

    /* the new refresh token generated from the new access token */
    string new_refresh_token<>;

    /* the number of operations that can be performed with the new access token */
    int token_expiration;
};

program AUTHENTICATION_PROG {
    version AUTHENTICATION_VERS {
        /* authentication server generates the authorization token */ 
        struct request_authorization_response request_authorization(string) = 1;

        /* authentication server generates the access token and the refresh token */
        struct access_token_response request_access_token(struct access_token_request) = 2;

        /* resource server validates the access to the resource */
        struct validate_delegated_action_response validate_delegated_action(struct validate_delegated_action_request) = 3;
        
        /* end user signs the request token */
        operation_status_t aprove_request_token(string) = 4;

        /* authentication server refreshes the access and refresh tokens */
        struct refresh_token_response refresh_token_operation(struct refresh_token_request) = 5;
    } = 1;
} = 1;
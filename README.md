# Raduta Lavinia-Maria 343C1
# SPRC - Homework 1
# OAuth authorization using RPC
## November 2023

## 1. Introduction
This homework presents a client-server application that simulates an OAuth 
authorization system using Remote Procedure Calls (RPC)

## 2. Application components
The application is composed of two main components:
- The client (**client.cpp**):
    - reads the opperations that will be executed from a cvs file (**client.
    in**)
    - executes the operations in order calling the remote procedures
- The server (**server.cpp**)
    - the server implements the remote procedures authorizing the users, manage 
    the resources and approve authorization tokens from the end user
    - the initial information about users, resources and permissions on the 
    resources are read from files and stored in the memory of the server
    - the logic is then splitted in 3 main components:
        - **The authentication server** (authentication_server.cpp) - 
        responsible for:
            - generating authorization tokens
            - generating access tokens
        - **The resource server** (resource_server.cpp) - responsible for:
            - approving/denying the user access to the resources, depending on 
            their permissions
        - **The end user** (end_user.cpp) - responsible for:
            - approving the request access of the users

## 3. Normal Program Flow
```
CLIENT ------------- request authorization -------------> AUTHENTICATION SERVER
AUTHENTICATION SERVER --------------- authorization token -------------> CLIENT
CLIENT ------------------- approve request token --------------------> END USER
END USER --------------------------- SIGNED  --------------------------> CLIENT
CLIENT -------------- request access token -------------> AUTHENTICATION SERVER
AUTHENTICATION SERVER ------- access token (+refresh token) -----------> CLIENT
CLIENT -------------- validate delegated action --------------> RESOURCE SERVER
RESOURCE SERVER --------------- approve/deny action -------------------> CLIENT

................

(when a token expires and the user has autorefresh)
CLIENT ------------ request new access token -----------> AUTHENTICATION SERVER
AUTHENTICATION SERVER -------- new access token + refresh token -------> CLIENT
```
## 4. Procedures
- **Request Authorization** (authentication_server.cpp - request_access_token_1)
    - the server receives from the client an user_id
    - the server searches in its internal user_database for the user_id
    - it generates an authorization token only if the user_id was found

- **Approve request token** (end_user.cpp - aprove_request_token_1)
    - the server receives from the client an authorization request token
    - it gets from the internal approvals database the permissions the user 
    approves
    - if the user approves some permissions, the token is signed (is added to 
    an approved_tokens set) and the status is returned to the client

- **Request access token** (authorization_server.cpp - request_access_token_1)
    - the server receives from the client an authorization token
    - it searches in the approved tokens for the one received. If the search 
    fails, the request is denied and the client receives the REQUEST_DENIED 
    error code
    - if the token was previously signed, the server generates an access token, 
    using the authorization token
    - if the user opted for autorefresh of the access token, a refresh token 
    will also be generated using the access one
    - the generated tokens will also have a valability associated with them
    - the authorization token is updated with the new access token in the users 
    database

 - **Validate delegated action** (resource_server.cpp - 
 validate_delegated_action_1)
    - the server receives from the client the access token, the resource that 
    they want to access and the operation that should be executed on the 
    resource
    - the server checks if the access token is present in the databse, and in 
    case of failure, PERMISSION_DENIED is returned to the client
    - then the server checks the valability of the access token, and in case it 
    is expired it returns TOKEN_EXPIRED. Users that have autorefresh enabled 
    have to refresh the access token prior to executing operations on resources
    - then the server checks if the resource the client wants to access really 
    exists, returning RESOURCE_NOT_FOUND in case of failure
    - finally, the server checks the user permissions for the resouce and 
    decides if the operation is permitted or denied

- **Refresh access token** (authentication_server.cpp - 
refresh_token_operation_1)
    - the valability of access tokens is decremented with each operation on 
    resources, so when it reaches the value of 0, the client must initiate the 
    refresh of the tokens for the users that opted for autorefresh
    - the client sends the old access and refresh tokens to the server
    - the server generates a new access token using the refresh token received 
    and a new refresh token from the access one just generated
    - the new tokens are updated in all the data structures that keep the users 
    database

## 5. User database
- the information about users and their tokens and permissions are stored in 
multiple data structures (**user_database.h**)
    - map <user_id:string, token:string> user_tokens
        - map that keeps the user_id and the current active token 
        (authentication request or access token)
    - map <access_token:string, authorization_token:string> access_req_tokens
        - map that keeps the correlation between the access token and the 
        authorization request token
    - set <authorization_token:string> approved_tokens
        - set that keeps the approved authorization request tokens
    - map <authorization_token:string, user_permissions:vector <permissions>> 
    auth_token_permissions
        - map that keeps the authorization token and the permissions associated 
        with it (permission = resource + resouce permissions)
    - map <access_token:string, valability:int>
        - map that keeps the access token and its availability
    - map <access_token:string, refresh_token:string>
        - map that keeps the access token and the refresh token associated

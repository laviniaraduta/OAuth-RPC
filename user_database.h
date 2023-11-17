#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <queue>

using namespace std;

typedef enum {
    READ,
    INSERT,
    MODIFY,
    DELETE,
    EXECUTE
} permission_type_t;

typedef struct permission {
    string resource;
    unordered_set<permission_type_t> resource_permissions;
} permission_t;

// The database that is loaded at the beginning of the program
extern int no_of_users, no_of_resources, token_valability;
extern unordered_set <string> user_ids;
extern unordered_set <string> resources;
extern queue<vector<permission_t>> permissions;

// Data structures used to keep information about users operations

// map that keeps the user_id and the current used token (authentication request or access token)
extern unordered_map <string, string> user_tokens;

// map that keeps the correlation between the access token and the auth request token
extern unordered_map <string, string> access_req_tokens;

// set that keep the approved authentication request tokens
extern unordered_set <string> approved_tokens;

// map that keeps the authentication token and the permissions that it has
extern unordered_map <string, vector<permission_t>> auth_token_permissions;

// map that keeps the access token and its availability
extern unordered_map <string, int> access_tokens_with_autorefresh;

// map that keeps the access token and the refresh token associated with it
extern unordered_map <string, string> access_tokens_refresh_tokens;

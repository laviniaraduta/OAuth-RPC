#include "user_database.h"
#include "rpc_authentication.h"
#include <iostream>
#include <fstream>
#include "rpc_authentication_svc.c"

using namespace std;

#define DELIM ','

int no_of_users, no_of_resources, token_valability;

// set of user ids
unordered_set <string> user_ids;

// set of resource names
unordered_set <string> resources;

// queue of allowed permissions for each user
queue<vector<permission_t>> permissions;

void read_clients_file(string filename) {
    ifstream infile(filename.c_str());
    string line;

    getline(infile, line);
    no_of_users = stoi(line);

    while (getline(infile, line)) {
        user_ids.insert(line);
    }

    infile.close();
}

void read_resources_file(string filename) {
    ifstream infile(filename.c_str());
    string line;

    getline(infile, line);
    no_of_resources = stoi(line);

    while (getline(infile, line)) {
        resources.insert(line);
    }

    infile.close();
}

// function used to read the approvals file
void read_approvals_file(string filename) {
    ifstream infile(filename.c_str());
    string line;

    while (getline(infile, line)) {
        vector<permission_t> user_permissions;

        if (line == "*,-") {
            // the user does not have any permissions
            permissions.push(user_permissions);
            continue;
        }

        int pos = line.find(DELIM);

        while (pos != string::npos) {
            string resouce_name = line.substr(0, pos);
            line.erase(0, pos + 1);

            pos = line.find(DELIM);
            string permission_type = line.substr(0, pos);
            line.erase(0, pos + 1);

            // each resource has a set of permissions
            unordered_set <permission_type_t> resource_perms;

            for (char c : permission_type) {
                if (c == 'R') {
                    resource_perms.insert(READ);
                } else if (c == 'I') {
                    resource_perms.insert(INSERT);
                } else if (c == 'M') {
                    resource_perms.insert(MODIFY);
                } else if (c == 'D') {
                    resource_perms.insert(DELETE);
                } else if (c == 'X') {
                    resource_perms.insert(EXECUTE);
                }
            }

            permission_t perm;
            perm.resource = resouce_name;
            perm.resource_permissions = resource_perms;
            user_permissions.push_back(perm);

            pos = line.find(DELIM);
        }
        permissions.push(user_permissions);
    }
    infile.close();
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        cout << "Usage: " << argv[0] << " <clients_file> <resouces_file> <approvals_file> <token_valiability>" << endl;
        return 1;
    }

    // Read the clients file
    read_clients_file(argv[1]);

    // Read the resources file
    read_resources_file(argv[2]);

    // Read the approvals file
    read_approvals_file(argv[3]);

    token_valability = stoi(argv[4]);

    // Start the RPC server
    register SVCXPRT *transp;

    pmap_unset (AUTHENTICATION_PROG, AUTHENTICATION_VERS);
    
    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, AUTHENTICATION_PROG, AUTHENTICATION_VERS, authentication_prog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (AUTHENTICATION_PROG, AUTHENTICATION_VERS, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, AUTHENTICATION_PROG, AUTHENTICATION_VERS, authentication_prog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (AUTHENTICATION_PROG, AUTHENTICATION_VERS, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
    return 0;
}
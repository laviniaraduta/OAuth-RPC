#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define DELIM ','

typedef struct operation {
    string user_id;
    string action;
    string param;
} operation_t;


operation_t parse_operation(string line) {
    operation_t op;

    // split the line by ","
    int pos = 0;
    string token;

    pos = line.find(DELIM);
    op.user_id = line.substr(0, pos);
    line.erase(0, pos + 1);

    pos = line.find(DELIM);
    op.action = line.substr(0, pos);
    line.erase(0, pos + 1);

    op.param = line;

    return op;
}

vector<operation_t> read_input(string filename) {
    ifstream infile(filename.c_str());
    string line;
    vector<operation_t> operations;

    while (getline(infile, line)) {
        cout << line << endl;
        operation_t op = parse_operation(line);
        operations.push_back(op);
    }

    infile.close();
    return operations;
}


void execute_operations(string host, vector<operation_t> operations) {
    for (operation_t op : operations) {
        if (op.action == "REQUEST") {
            cout << "request" << endl;
        } else {
            cout << "not request" << endl;
        }
    }
}


int main(int argc, char* argv[])
{
    string host;
    string filename;
    vector<operation_t> operations;

	if (argc < 3) {
		printf ("usage: %s server_host cvs_file\n", argv[0]);
		exit (1);
	}
	host = argv[1];
    filename = argv[2];

    cout << "host: " << host << endl;
    cout << "filename: " << filename << endl;

    operations = read_input(filename);

    execute_operations(host, operations);

    return 0;
}

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <dirent.h>

#include <sys/stat.h>
#include <stdio.h>

using namespace std;

struct stat sb;

bool VERBOSE = false;
bool REMOVE = false;
bool IS_PIPE = false;
bool LIST = false;

bool isDirectory(const char*);
string resolvePath(string path);

map<string, string> getArgs(int argc, char* argv[]);
bool copyFile(istream&, ostream&);

int main(int argc, char* argv[]) {
    string MEMO_PATH = resolvePath("~/.memo/");

    map<string, string> args = getArgs(argc, argv);

    //Create memo save location if it doesn't already exist
    if (!isDirectory(MEMO_PATH.c_str())) {
        if (VERBOSE) {
            cout << "Creating memo directory: " << MEMO_PATH << endl;
        }
        system((string("mkdir ") + MEMO_PATH).c_str());
    }
    
    if (VERBOSE) {
        cout << "Memo (C) Langston Chandler 2018" << endl;
    }

    if (REMOVE) {
        if (remove((MEMO_PATH + args["name"]).c_str()) != 0) {
            cerr << "Could not remove memo. Is the name correct?" << endl;
        }
        else {
            cout << "Memo removed." << endl;
        }
    }
    else if (LIST) {
        DIR *dir = opendir(MEMO_PATH.c_str());
        struct dirent *ent;
        if (dir != nullptr) {
            while ((ent = readdir (dir)) != NULL) {
                if ((ent->d_name)[0] != '.') {
                    cout << ent->d_name << endl;
                }
            }
            closedir (dir);
        } else {
            perror ("");
            return EXIT_FAILURE;
        }
    }
    //If no input, print whatever memo already exists with that name
    else if (!IS_PIPE && args.find("value") == args.end()) {
        ifstream f(MEMO_PATH + args["name"], ios::binary);

        if (VERBOSE) {
            cout << "Printing memo...\n" << endl;
        }

        copyFile(f, cout);

        if (VERBOSE) {
            cout << "\n\nDone." << endl;
        }
        f.close();
    }
    else if (IS_PIPE) {
        ofstream f(MEMO_PATH + args["name"], ios::binary);

        if (VERBOSE) {
            cout << "Writing memo from piped input..." << endl;
        }

        copyFile(cin, f);

        if (VERBOSE) {
            cout << "\nDone." << endl;
        }
        f.close();
    }
    else {
        ofstream f(MEMO_PATH + args["name"], ios::binary);

        if (VERBOSE) {
            cout << "Writing memo from argument input..." << endl;
        }

        f << args["value"];

        if (VERBOSE) {
            cout << "\nDone." << endl;
        }

        f.close();
    }

    return 0;
}

string resolvePath(string path) {
    if (path.length() < 2) return "";
    if (path[0] == '.') return string("pwd") + path.substr(1);
    if (path[0] == '~') return string(getenv("HOME")) + path.substr(1);
}

bool isDirectory(const char* pathName) {
    return stat(pathName, &sb) == 0 && S_ISDIR(sb.st_mode);
}

map<string, string> getArgs(int argc, char* argv[]) {
    map<string, string> args;
    string arg;
    for (int i = 1; i < argc; i++) {
        arg = string(argv[i]);
        if (arg.size() > 1 && arg[0] == '-') {
            switch(arg[1]) {
                case 'r':
                case 'R':
                    REMOVE = true;
                    break;
                case 'l':
                case 'L':
                    LIST = true;
                    break;
                case 'n':
                case 'N':
                    if (i+1 < argc) {
                        args["name"] = string(argv[++i]); 
                    }
                    else {
                        cout << "ERROR: Name identifier must be followed by a name." << endl;
                        exit(-1);
                    }
                    break;
                case 'p':
                case 'P':
                    IS_PIPE = true;
                    break;
                case 'v':
                case 'V':
                    VERBOSE = true;
                    break;
                default:
                    cout << "ERROR: Invalid argument! (" << arg << endl;
                    exit(-1);
            }
        }
        else {
            if (args.find("value") != args.end()) {
                cout << "ERROR: Cannot specify multiple values for a single memo." << endl
                     << "\tDid you forget quotation marks around the memo?" << endl;
                exit(-1);
            }
            args["value"] = string(argv[i]);
        }
        if (LIST) break;
    }
    if (!LIST && args.find("name") == args.end()) {
        cout << "ERROR: Please specify the name of a memo." << endl;
        exit(-1);
    }
    if (VERBOSE) {
        cout << "Detected settings:\n"
             << "\tName: " << args["name"] << endl
             << "\tValue: " << ((args.find("value") != args.end())?args["value"]:"[EMPTY]") << endl
             << "\tVerbose: TRUE" << endl;
    }
    return args;
}

bool copyFile(istream& input, ostream& output) {

    istreambuf_iterator<char> begin_from(input);
    istreambuf_iterator<char> end_from;
    ostreambuf_iterator<char> begin_to(output);

    copy(begin_from, end_from, begin_to);
    return true;
}
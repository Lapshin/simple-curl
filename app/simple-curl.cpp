#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "WebClient.h"


using namespace std;

void print_help()
{
    cout << "Usage: simple-curl [OPTION]... URL" << endl;
    cout << "  -o FILE    HTTP response will be written to FILE" << endl;
}

int main(int argc, char **argv)
{
    char *url = NULL;
    char *filePath = NULL;
    if (argc <= 1)
    {
        print_help();
        return -1;
    }
    for (;;)
    {
        switch (getopt(argc, argv, "-:o:h"))
        {
        case 'o':
            if(filePath != NULL)
            {
                cerr << "Output file specified more then one time" << endl;
                return 2;
            }
            filePath = optarg;
            continue;
        case '?':
        case 'h':
            print_help();
            return 0;
        case 1:
        {
            if(url != NULL)
            {
                cerr << "Only one url must specified" << endl;
                return 2;
            }
            url = optarg;
            continue;
        }
        default:
            break;
        }
        break;
    }

    try
    {
        WebClient wc(url);
        wc.setFilePath(filePath);
        wc.fetch();
        wc.flushResponse();
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
        if (errno != 0)
        {
            cerr << "\t" << "Errno: " << errno << ", " << strerror(errno) << endl;
        }
        return 5;
    }
    return 0;
}

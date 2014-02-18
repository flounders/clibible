#include <iostream>
#include <swmgr.h>
#include <versekey.h>
#include <listkey.h>
#include <swmodule.h>
#include <markupfiltmgr.h>
#include <rawtext.h>
#include <regex.h>
#include <cstdlib>
#include <unistd.h>

// I am simply going to use one variable for
// option flags and I am assigning one bit
// for each option.
#define HELP_FLAG 1
#define SEARCH_FLAG 2
#define MODULE_FLAG 4
#define SEARCH_TYPE_FLAG 8

using sword::SWMgr;
using sword::VerseKey;
using sword::ListKey;
using sword::SWModule;
using sword::SW_POSITION;
using sword::FMT_PLAIN;
using sword::MarkupFilterMgr;
using sword::SWBuf;
using std::cerr;
using std::cout;
using std::endl;

/* search constants
 *                     >= 0 - regex
 *                     -1   - phrase
 *                     -2   - multiword
 *                     -3   - entryAttrib (eg. Word//Lemma/G1234/)
 *                     -4   - Lucene
 */

char printed = 0;

char SEARCH_TYPE=-2;

void usage(char *progName);
void percentUpdate(char percent, void *userData);

int main(int argc, char **argv)
{
    int c;

    char *modArg = NULL;
    char *searchArg = NULL;
    char *verseRange = NULL;

    short optflags;

    VerseKey displayParser;
    VerseKey *searchParser;
    ListKey result;
    SWBuf searchTerm;
    SWMgr library(new MarkupFilterMgr(FMT_PLAIN));
    SWModule *book = NULL;

    while ((c = getopt(argc, argv, "hs:m:t:")) != -1) {
        switch (c) {
        case 'h':
            optflags |= HELP_FLAG;
            break;
        case 's':
            optflags |= SEARCH_FLAG;
            searchArg = optarg;
            break;
        case 'm':
            optflags |= MODULE_FLAG;
            modArg = optarg;
            break;
        case 't':
            optflags |= SEARCH_TYPE_FLAG;
            SEARCH_TYPE = atoi(optarg);
            break;
        case '?':
            usage(argv[0]);
            exit(1);
            break;
        }
    }

    if (optflags & HELP_FLAG) {
        usage(argv[0]);
        exit(0);
    }
    if (optflags & MODULE_FLAG) {
        book = library.getModule(modArg);
    }
    if (optflags & SEARCH_FLAG) {
        searchTerm = searchArg;
    }

    if (optind < argc & searchArg == NULL) {
        int index;
        for (index = optind; index < argc; index++) {
            verseRange = argv[index];
            result = displayParser.parseVerseList(verseRange, displayParser, true);
            for (result = TOP; !result.popError(); result++) {
                if (book) {
                    book->setKey(result);
                    cout << book->getKeyText() << ": " << book->renderText()
                         << endl;
                }
            }
        }
    }
    else if (optind < argc && searchArg != NULL) {
        if (book != NULL) {
            sword::ModMap::iterator it;
            it = library.Modules.find(modArg);
            if (it == library.Modules.end()) {
                cerr << "Could not find module [" << modArg 
                     << "].  Available modules:\n";

                for (it = library.Modules.begin(); it != library.Modules.end(); ++it) {
                    cerr << "[" << (*it).second->getName() << "]\t - "
                         << (*it).second->getDescription() << endl;
                    exit(1);
                }
            }

            book = (*it).second;

            sword::SWKey *k = book->getKey();
            searchParser = SWDYNAMIC_CAST(VerseKey, k);
            if (!searchParser) {
                cerr << "Encountered error with parser.\n";
                exit(1);
            }

            int index = optind;

            result = searchParser->parseVerseList(argv[index], *searchParser, true);
            result.setPersist(true);
            book->setKey(result);
            index++;

            cout << "[0=================================50===============================100]\n ";

            char lineLen = 70;
            ListKey listKey = book->search(searchTerm.c_str(), SEARCH_TYPE,
                                           REG_ICASE, 0, 0, &percentUpdate, &lineLen);
            if (index < argc) {
                result = listKey;
                result.setPersist(true);
                book->setKey(result);
                printed = 0;
                cout << " ";
                listKey = book->search(argv[index], SEARCH_TYPE,
                                       REG_ICASE, 0, 0, &percentUpdate, &lineLen);
                cout << endl;
            }
            cout << endl;
            listKey.sort();
            while (!listKey.popError()) {
                cout << (const char *)listKey << endl;
                listKey++;
            }
        }
    }
    else if (searchArg != NULL) {
        if (book != NULL) {
            cout << "[0=================================50===============================100]\n ";
            char lineLen = 70;
            ListKey listKey = book->search(searchTerm.c_str(), SEARCH_TYPE,
                                           REG_ICASE, 0, 0, &percentUpdate, &lineLen);
            cout << endl;
            listKey.sort();
            while (!listKey.popError()) {
                cout << (const char *)listKey << endl;
                listKey++;
            }
        }
    }    else {
        cout << "Too few arguments.\n";
        usage(argv[0]);
        return 1;
    }

    if (!book) {
        cerr << "Invalid module or no module given.\n";
        return 1;
    }

    return 0;
}

void usage(char *progName)
{
    cout << "Usage: " << progName << " [OPTIONS] [ARGS]\n";
    cout << "Options:\n";
    cout << "\t-h help\n";
    cout << "\t-m module [MOD]\n";
    cout << "\t-s search [SEARCH_PARAMETERS]\n";
    cout << "\t-t search type [SEARCH_TYPE]\n";
    cout << endl;

    cout << "SEARCH_TYPES: >=0 - regex\n";
    cout << "              -1  - phrase\n";
    cout << "              -2  - multiword\n";
    cout << "              -3  - entryAttrib (eg. Word//Lemma/G1234/)\n";
    cout << "              -4  - Lucene\n";
}

void percentUpdate(char percent, void *userData)
{
    char maxHashes = *((char *)userData);

    while ((((float)percent)/100) * maxHashes > printed) {
        cout << "=";
        printed++;
        cout.flush();
    }

    cout.flush();
}

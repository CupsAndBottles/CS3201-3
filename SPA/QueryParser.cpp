#include "QueryParser.h"
#include "Query.h"
#include "Clause.h"
#include "StringPair.h"
#include "Utils.h"
#include "PQLExceptions.h"
#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

QueryParser::QueryParser(void){
}

QueryParser::~QueryParser(void){
}

vector<string> split(string s, char delim, vector<string> elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> QueryParser::tokeniser(string input, char delim){
	vector<string> elems;
    split(input, delim, elems);
    return elems;
}

Query QueryParser::processQuery(string input){
	Utils util;
	Query parsedQuery = Query();
	vector<string> declarationTokens = tokeniser(input, ';');
	int numSynonyms = declarationTokens.size() - 1;
	string selectStatement = declarationTokens.at(numSynonyms);
	declarationTokens.pop_back();
	map<string, string> declarations = map<string, string>();
	for (int i=0; i<numSynonyms; i++){
		string currentDeclaration = declarationTokens.at(i);
		vector<string> declarationPair = tokeniser(currentDeclaration, ' ');
		declarations.insert(declarationPair.at(1), declarationPair.at(0));
	}
	parsedQuery.setDeclarationList(declarations);
	selectStatement = util.sanitise(selectStatement);
	vector<string> tokens = util.explode(selectStatement);
	for (int i=0; i<tokens.size(); i++){
		string current = tokens.at(i);
		if (i==0){
			if (!current.compare(stringconst::STRING_SELECT)){
				throw InvalidSelectException();
			}
		} else if (i==1){
			parsedQuery.addSelectSynonym(current);
		} else {
			if (current.compare(stringconst::STRING_AND) || current.compare(stringconst::STRING_WITH)){
				if (i == tokens.size() -1){
					throw InvalidSentenceException();
				} else {
				
				}
			} else if (current.compare(stringconst::STRING_SUCH)){
				if (i == tokens.size() -1){
					throw InvalidSentenceException();
				} else {
					
				}
			}
		}
	}
}



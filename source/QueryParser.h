#pragma once
#include <string>
#include "Query.h"
using std::string;
class QueryParser
{
private:
	bool checkValidity(Query);
	Query queryCreator(string);
public:
	QueryParser(void);
	~QueryParser(void);
};


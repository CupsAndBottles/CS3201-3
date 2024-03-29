#ifndef TESTWRAPPER_H
#define TESTWRAPPER_H

#include <string>
#include <iostream>
#include <list>
#include "../../SPA/Parser.h"
#include "../../SPA/InvalidCodeException.h"
#include "../../SPA/PQLController.h"


// include your other headers here
#include "AbstractWrapper.h"

class TestWrapper : public AbstractWrapper {
 public:
  // default constructor
  TestWrapper();
  
  // destructor
  ~TestWrapper();
  
  // method for parsing the SIMPLE source
  virtual void parse(std::string filename);
  
  // method for evaluating a query
  virtual void evaluate(std::string query, std::list<std::string>& results);

  Parser* parser;
  PQLController* pqlController;
  int counter;
};

#endif

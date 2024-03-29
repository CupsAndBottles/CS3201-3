
#include <stack>
#include <queue>

#ifndef PDR_HEADER
#define PDR_HEADER

#include <string>
#include <regex>

#include "ConstNode.h"
#include "ParsedData.h"
#include "ProcNode.h"
#include "ProcTable.h"
#include "VarTable.h"
#include "Variable.h"
#include "AssgNode.h"
#include "StmtNode.h"
#include "StmtTable.h"
#include "Statement.h"
#include "WhileNode.h"
#include "StmtLstNode.h"
#include "Procedure.h"
#include "IfNode.h"
#include "CallNode.h"
#include "OpNode.h"
#include "AST.h"
#include "Constant.h"
#include "ConstTable.h"

using namespace std;

class PDR {
	
public:
    

    static PDR* getInstance();
	static void resetInstanceFlag();

	void processParsedData(ParsedData);

	int getCurrNestingLevel();
	int getCurrStmtNumber();
	stack<TNode*> getNodeStack();

private:
	enum Type {ASSIGNMENT, PROCEDURE, PROGRAM, OPERATOR, WHILE};
	enum Flag {USES, MODIFIES};

	int stmtCounter;
	int currNestingLevel;
    static bool instanceFlag;
    static PDR* pdrInstance;

	stack<int> stmtParentNumStack;
	stack<TNode*> nodeStack;

	void processProcedureStmt(ParsedData);
	void processAssignStmt(ParsedData);
	void processIfStmt(ParsedData);
	void processWhileStmt(ParsedData);
	void processCallStmt(ParsedData);
    void processEndProgram();
	
    void addParentSet(set<string>, Flag);
    void addToProcTable(TNode*);
    void addToVarTable(TNode*, Flag);
	void addToConstTable(TNode*);
    
    TNode* breakDownAssignExpression(ParsedData, set<string>&);
    
    bool isInteger(string);

	// constructor
	PDR();

};
#endif

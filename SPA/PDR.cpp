#include "PDR.h"

using namespace std;

bool PDR::instanceFlag = false;
PDR* PDR::pdrInstance = NULL;

PDR::PDR() {
    currNestingLevel = 0;
    stmtCounter = 0;
}

PDR* PDR::getInstance() {
    if(!instanceFlag) {
        pdrInstance = new PDR();
        instanceFlag = true;
    }
    
    return pdrInstance;
}

void PDR::resetInstanceFlag() {
	instanceFlag = false;
}

void PDR::processParsedData(ParsedData data) {
    ParsedData::Type stmtType = data.getType();
    switch(stmtType) {
        case ParsedData::PROCEDURE:
            processProcedureStmt(data);
            break;
        case ParsedData::ASSIGNMENT:
            processAssignStmt(data);
            break;
        case ParsedData::WHILE:
            processWhileStmt(data);
            break;
        case ParsedData::END:
            processEndProgram();
            break;
        default:
            break;
    }
}

void PDR::processProcedureStmt(ParsedData data) {
    // New Procedure
    for(int i = 0; i < currNestingLevel - data.getNestingLevel(); i++) {
        nodeStack.pop();
    }
    
    // If there was a previous procedure, link prev proc to AST
    if(!nodeStack.empty()) {
        ProcNode* previousProc = (ProcNode*)nodeStack.top();
        AST* ast = AST::getInstance();
        ast->addProcNode(previousProc);
        nodeStack.pop();
    }

    ProcNode* procedure = new ProcNode(data.getProcName());
    StmtLstNode* stmtLst = new StmtLstNode();
    procedure->addChild(stmtLst);
    
    nodeStack.push(procedure);
    nodeStack.push(stmtLst);
    
    currNestingLevel = data.getNestingLevel();
    currNestingLevel++;
    
    addToProcTable(procedure);
    
}

void PDR::processAssignStmt(ParsedData data) {
    if(currNestingLevel > data.getNestingLevel()) {
        int diffNestingLevel = currNestingLevel - data.getNestingLevel();
        for(int i = 0; i < diffNestingLevel; i++) {
            stmtParentNumStack.pop();
            nodeStack.pop();
        }
        currNestingLevel = data.getNestingLevel();
    }
    
    set<string> modifies;
    set<string> uses;
    AssgNode* assignNode = new AssgNode(++stmtCounter);
    TNode* assignExpChild = breakDownAssignExpression(data, uses);
    TNode* parentStmtLst = nodeStack.top();
    
    if(parentStmtLst->hasChildren()) {
        vector<TNode*> listOfChildren = parentStmtLst->getChildren();
        long int lastChild = listOfChildren.size() - 1;
        TNode* leftSibling = listOfChildren[lastChild];
        assignNode->linkLeftSibling(leftSibling);
    }
    
    modifies.insert(data.getAssignVar());
    
    // Linking the AST
    assignNode->linkParent(parentStmtLst);
    assignNode->linkExprNode(assignExpChild);
    VarNode* modifiesVar = new VarNode(data.getAssignVar());
    assignNode->linkVarNode(modifiesVar);
    addToVarTable(modifiesVar);
    
    
    // Populating the StmtTable
    StmtTable* stmtTable = StmtTable::getInstance();
    Statement* stmt = new Statement();
    stmt->setType(assignNode->getNodeType());
    stmt->setStmtNum(stmtCounter);
    stmt->setTNodeRef(assignNode);
    stmt->setModifies(modifies);
    stmt->setUses(uses);
	
    
    if(assignNode->hasLeftSibling()) {
        StmtNode* leftSib = (StmtNode*)assignNode->getLeftSibling();
        stmt->setFollowsAfter(leftSib->getStmtNum());
        Statement* leftStmt = stmtTable->getStmtObj(leftSib->getStmtNum());
        leftStmt->setFollowsBefore(assignNode->getStmtNum());
    }
    
    if(!stmtParentNumStack.empty()) {
        int parentStmtNum = stmtParentNumStack.top();
        stmt->setParent(parentStmtNum);
		Statement* parentStmt = stmtTable->getStmtObj(parentStmtNum);
		set<int> children = parentStmt->getChildren();
		children.insert(assignNode->getStmtNum());
		parentStmt->setChildren(children);
    }
	
    stmtTable->addStmt(stmt);
    
}

void PDR::processWhileStmt(ParsedData data) {
    if(currNestingLevel > data.getNestingLevel()) {
        int diffNestingLevel = currNestingLevel - data.getNestingLevel();
        for(int i = 0; i < diffNestingLevel; i++) {
            stmtParentNumStack.pop();
            nodeStack.pop();
        }
    }
    
    WhileNode* whileNode = new WhileNode(++stmtCounter);
    StmtLstNode* stmtLst = new StmtLstNode();
    TNode* parentStmtLst = nodeStack.top();
    
    if(parentStmtLst->hasChildren()) {
        vector<TNode*> listOfChildren = parentStmtLst->getChildren();
        long int lastChild = listOfChildren.size() - 1;
        TNode* leftSibling = listOfChildren[lastChild];
        whileNode->linkLeftSibling(leftSibling);
    }
    
    // Linking the AST
    VarNode* whileVar = new VarNode(data.getWhileVar());
    whileNode->linkParent(parentStmtLst);
    whileNode->linkVarNode(whileVar);
    whileNode->linkStmtLstNode(stmtLst);
    
    stmtParentNumStack.push(stmtCounter);
    nodeStack.push(stmtLst);
    stmtParentNumStack.push(stmtCounter);
    currNestingLevel = data.getNestingLevel() + 1;
    
    // Populating StmtTable
    StmtTable* stmtTable = StmtTable::getInstance();
    Statement* whileStmt = new Statement();
    whileStmt->setType(whileNode->getNodeType());
    whileStmt->setStmtNum(whileNode->getStmtNum());
    whileStmt->setTNodeRef(whileNode);
    
    if(whileNode->hasLeftSibling()) {
        StmtNode* leftSib = (StmtNode*)whileNode->getLeftSibling();
        whileStmt->setFollowsAfter(leftSib->getStmtNum());
        Statement* leftStmt = stmtTable->getStmtObj(leftSib->getStmtNum());
        leftStmt->setFollowsBefore(whileNode->getStmtNum());
    }
    
    if(!stmtParentNumStack.empty()) {
        int parentStmtNum = stmtParentNumStack.top();
        whileStmt->setParent(parentStmtNum);
		Statement* parentStmt = stmtTable->getStmtObj(parentStmtNum);
		set<int> children = parentStmt->getChildren();
		children.insert(whileNode->getStmtNum());
		parentStmt->setChildren(children);
    }
    
    addToVarTable(whileVar);
    stmtTable->addStmt(whileStmt);
}

TNode* PDR::breakDownAssignExpression(ParsedData data, set<string>& usesSet) {
    // Assume expression to be the RPN of the variables and operators
    queue<string> expression = data.getAssignExpression();
    stack<TNode*> rpnNodeStack;
	int numExpressions = expression.size();
    
    if(numExpressions == 1) {
        string exp = expression.front();
        expression.pop();
        
        if(isInteger(exp)) {
            ConstNode* constNode = new ConstNode(exp);
            return constNode;
        } else {
            VarNode* var = new VarNode(exp);
            usesSet.insert(exp);
            return var;
        }
    }
    
    for(size_t i = 0; i < numExpressions; i++) {
        string exp = expression.front();
        expression.pop();
        
        if(exp == "+" || exp == "-" || exp == "*" || exp == "/") {
            OpNode* operat = new OpNode(exp);
            TNode* right = rpnNodeStack.top();
            rpnNodeStack.pop();
            TNode* left = rpnNodeStack.top();
            rpnNodeStack.pop();
            operat->linkLeftNode(left);
            operat->linkRightNode(right);
            rpnNodeStack.push(operat);
        } else {
            if(isInteger(exp)) {
                ConstNode* constNode = new ConstNode(exp);
                rpnNodeStack.push(constNode);
            } else {
                VarNode* var = new VarNode(exp);
                rpnNodeStack.push(var);
                usesSet.insert(exp);
                addToVarTable(var);
            }
        }
    }
    
    TNode* result = NULL;
    while(!rpnNodeStack.empty()) {
        result = rpnNodeStack.top();
        rpnNodeStack.pop();
    }
    
    return result;
}

void PDR::addToProcTable(TNode* procedure) {
    ProcTable* procTable = ProcTable::getInstance();
    Procedure* proc = new Procedure(procedure->getName(), procedure);
    procTable->addProc(proc);
}

void PDR::addToVarTable(TNode* variable) {
    VarTable* varTable = VarTable::getInstance();
    
    if(varTable->contains(variable->getName())) {
        Variable* var = varTable->getVariable(variable->getName());
        var->addTNode(variable);
    } else {
        Variable* var = new Variable(variable->getName());
        varTable->addVariable(var);
    }
}

void PDR::processEndProgram() {
    for(int i = 0; i < currNestingLevel; i++) {
        nodeStack.pop();
    }
    
    ProcNode* procNodeToBeLinked = (ProcNode*)nodeStack.top();
    AST* ast = AST::getInstance();
    ast->addProcNode(procNodeToBeLinked);
    
    nodeStack.pop();
}

bool PDR::isInteger(string exp) {
    regex integer("(\\+|-)?[[:digit:]]+");
    
    if(regex_match(exp, integer)) {
        return true;
    }
    
    return false;
}
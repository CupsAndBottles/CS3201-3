#include "QueryEvaluator.h"
#include "Utils.h"
#include "InsertType.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

QueryEvaluator::QueryEvaluator(void)
{
	stmtTable = StmtTable::getInstance();
	varTable = VarTable::getInstance();
	constTable = ConstTable::getInstance();
	procTable = ProcTable::getInstance();
}

QueryEvaluator::~QueryEvaluator(void)
{
}

Result* QueryEvaluator::evaluateQuery(Query* query) {
	Result *obj = new Result();
	setClauseList(query->getClauseList());
	setSelectList(query->getSelectList());
	evaluateClauses(obj, clauseList);
	return obj;
}

string QueryEvaluator::boolToString(bool b) {
	if (b) {
		return "true";
	} else {
		return "false";
	}
}

unordered_set<string> QueryEvaluator::getValuesToPrint(Result* obj, vector<StringPair> selectList) {
	unordered_set<string> resultSet = unordered_set<string>();
	string syn, type;
	int numOfSyn = selectList.size();
	bool isQueryPassed = obj->isPass();
	
	if (numOfSyn == 1) {
		syn = selectList.at(0).getFirst();
		type = selectList.at(0).getSecond();
		// Case of select boolean
		bool isSelectBoolNoClause = (syn == "BOOLEAN") && 
			(type == stringconst::ARG_BOOLEAN) && (clauseList.empty());
		bool isSelectBoolWithClause = (syn == "BOOLEAN") && 
			(type == stringconst::ARG_BOOLEAN) && (!clauseList.empty());
		
		if (isSelectBoolNoClause) {
			obj->setFail();
			resultSet.insert(boolToString(obj->isPass()));

		} else if (isSelectBoolWithClause) {
			resultSet.insert(boolToString(isQueryPassed));

		} else if (isQueryPassed) {
			// Case of select 1 synonym
			resultSet = printSingleSynValues(*obj, syn);
		}
	/*
		if (syn == "BOOLEAN" && type == stringconst::ARG_BOOLEAN) {
			
		} else if (isQueryPassed) {
			// Case of select 1 synonym
			resultSet = printSingleSynValues(*obj, syn);
		}
		*/
	} else {
		// Case of select tuple
		if (isQueryPassed) {
			resultSet = printTupleSynValues(*obj, selectList);
		}
	}

	return resultSet;
}

unordered_set<string> QueryEvaluator::printSingleSynValues(Result &obj, string syn) {
	unordered_set<string> resultSet = obj.getSyn(syn);
	return resultSet;
}

unordered_set<string> QueryEvaluator::printTupleSynValues(Result &obj, vector<StringPair> selectList) {
	vector<string> synList;
	unordered_set<string> resultSet;
	for (auto iter = selectList.begin(); iter != selectList.end(); ++iter) {
		synList.push_back(iter->getFirst());
	}
	unordered_set<vector<string>> resSet = obj.getMultiSyn(synList);
	for (auto rowIter = resSet.begin(); rowIter != resSet.end(); ++rowIter) {
		string tuple = "";
		vector<string> tupleList = *rowIter;
		for (auto valueIter = tupleList.begin(); valueIter != tupleList.end(); ++valueIter) {
			if (valueIter != tupleList.end() - 1) {
				tuple.append(*valueIter);
				tuple.append(" ");
			} else {
				tuple.append(*valueIter);
			}
		}
		resultSet.insert(tuple);
	}
	return resultSet;
}

void QueryEvaluator::setClauseList(vector<Clause*> clauseList) {
	this->clauseList = clauseList;
}

void QueryEvaluator::setSelectList(vector<StringPair> selectList) {
	this->selectList = selectList;
}

void QueryEvaluator::evaluateClauses(Result* obj, vector<Clause*> clauseList) {
	for (vector<Clause*>::iterator i = clauseList.begin(); i != clauseList.end(); ++i) {
		Clause* c = *i;
		if (c->evaluate(obj) == false) {
			return;
		} 
	}
	string syn = selectList.at(0).getFirst();
	string type = selectList.at(0).getSecond();
	if (syn != "BOOLEAN" && type != stringconst::ARG_BOOLEAN) {
		getRemainingSynValuesFromTable(*obj);
	}

	obj->setPass();
}

void QueryEvaluator::getRemainingSynValuesFromTable(Result &obj) {
	for (vector<StringPair>::iterator i = selectList.begin(); i != selectList.end(); ++i) {
		string syn = i->getFirst();
		if ((!obj.isSynPresent(syn) && obj.isPass() == true) ||
			(!obj.isSynPresent(syn) && clauseList.empty())) {
			string type = i->getSecond();
			unordered_set<string> values = getTableValues(type);
			insertSetValues(syn, values, obj);
		}
	}
}

void QueryEvaluator::insertSetValues(string syn, unordered_set<string> values, Result &obj) {
	SingleSynInsert insert;
	insert.setSyn(syn);
	for (unordered_set<string>::iterator i = values.begin(); i != values.end(); ++i) {
		string val = *i;
		insert.insertValue(val);
	}
	obj.push(insert);
}

unordered_set<string> QueryEvaluator::getTableValues(string type) {
	unordered_set<string> values = unordered_set<string>();
	if (type == stringconst::ARG_ASSIGN) {
		values = getAllAssign();
	} else if (type == stringconst::ARG_STATEMENT || type == stringconst::ARG_PROGLINE) {
		values = getAllStmt();
	} else if (type == stringconst::ARG_WHILE) {
		values = getAllWhile();
	} else if (type == stringconst::ARG_IF) {
		values = getAllIf();
	} else if (type == stringconst::ARG_CALL) {
		values = getAllCall();
	} else if (type == stringconst::ARG_VARIABLE) {
		values = getAllVar();
	} else if (type == stringconst::ARG_PROCEDURE) {
		values = getAllProc();
	} else if (type == stringconst::ARG_CONSTANT) {
		values = getAllConst();
	} else {
		// error: type not recognise
	}
	return values;
}

unordered_set<string> QueryEvaluator::getAllAssign() {
	// get all assignment statements
	unordered_set<Statement*> assignStmtSet = stmtTable->getAssgStmts();
	unordered_set<string> assignStringSet;
	for (unordered_set<Statement*>::iterator i = assignStmtSet.begin(); i != assignStmtSet.end(); ++i) {
		Statement* stmtObj = *i;
		int assignNum = stmtObj->getStmtNum();
		assignStringSet.insert(boost::lexical_cast<string>(assignNum));
	}
	return assignStringSet;
}

unordered_set<string> QueryEvaluator::getAllStmt() {
	// get all statement/ progline
	unordered_set<Statement*> stmtSet = stmtTable->getAllStmts();
	unordered_set<string> stmtStringSet;
	for (unordered_set<Statement*>::iterator i = stmtSet.begin(); i != stmtSet.end(); ++i) {
		Statement* stmtObj = *i;
		int stmtNum = stmtObj->getStmtNum();
		stmtStringSet.insert(boost::lexical_cast<string>(stmtNum));
	}
	return stmtStringSet;
}

unordered_set<string> QueryEvaluator::getAllWhile() {
	// get all whileStmt
	unordered_set<Statement*> whileStmtSet = stmtTable->getWhileStmts();
	unordered_set<string> whileStmtStringSet;
	for (unordered_set<Statement*>::iterator i = whileStmtSet.begin(); i != whileStmtSet.end(); ++i) {
		Statement* stmtObj = *i;
		int whileStmt = stmtObj->getStmtNum();
		whileStmtStringSet.insert(boost::lexical_cast<string>(whileStmt));
	}
	return whileStmtStringSet;
}

unordered_set<string> QueryEvaluator::getAllIf() {
	// get all ifStmt
	unordered_set<Statement*> ifStmtSet = stmtTable->getIfStmts();
	unordered_set<string> ifStmtStringSet;
	for (unordered_set<Statement*>::iterator i = ifStmtSet.begin(); i != ifStmtSet.end(); ++i) {
		Statement* stmtObj = *i;
		int ifStmt = stmtObj->getStmtNum();
		ifStmtStringSet.insert(boost::lexical_cast<string>(ifStmt));
	}
	return ifStmtStringSet;
}

unordered_set<string> QueryEvaluator::getAllCall() {
	// get all callStmt
	unordered_set<Statement*> callStmtSet = stmtTable->getCallStmts();
	unordered_set<string> callStmtStringSet;
	for (unordered_set<Statement*>::iterator i = callStmtSet.begin(); i != callStmtSet.end(); ++i) {
		Statement* stmtObj = *i;
		int callStmt = stmtObj->getStmtNum();
		callStmtStringSet.insert(boost::lexical_cast<string>(callStmt));
	}
	return callStmtStringSet;
}

unordered_set<string> QueryEvaluator::getAllVar() {
	// get all variables
	unordered_set<string> varVector = varTable->getAllVarNames();

	return varVector;
}

unordered_set<string> QueryEvaluator::getAllProc() {
	unordered_set<Procedure*> procSet = procTable->getAllProcs();
	unordered_set<string> procNameSet;
	for (unordered_set<Procedure*>::iterator i = procSet.begin(); i != procSet.end(); ++i) {
		Procedure* procObj = *i;
		string procName = procObj->getProcName();
		procNameSet.insert(procName);
	}
	return procNameSet;
}

unordered_set<string> QueryEvaluator::getAllConst() {
	vector<int> constVec = constTable->getAllConstValues();
	unordered_set<string> constStringSet = intVectorToSet(constVec);
	return constStringSet;
}

unordered_set<string> QueryEvaluator::stringVectorToSet(vector<string> &vec) {
	unordered_set<string> stringSet = unordered_set<string>();
	for (vector<string>::iterator i = vec.begin(); i != vec.end(); ++i) {
		string var = *i;
		stringSet.insert(var);
	}
	return stringSet;
}

unordered_set<string> QueryEvaluator::intVectorToSet(vector<int> &vec) {
	unordered_set<string> stringSet = unordered_set<string>();
	for (vector<int>::iterator i = vec.begin(); i != vec.end(); ++i) {
		int cons = *i;
		stringSet.insert(boost::lexical_cast<string>(cons));
	}
	return stringSet;
}
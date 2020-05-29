#ifndef __MysqlBuilder_h__
#define __MysqlBuilder_h__
#include "MultiSys.h"
#include "TString.h"
#include "TList.h"
#include "Tools.h"
#include <string>

class FiledName
{
	std::string _name;
	FiledName *next;
	FiledName(const char *name) :_name(name), next(nullptr) {};
};

class ConditionExpres
{
public:
protected:
private:
};

//typedef tlib::linear::SimpleList<FiledName> NameSet;
typedef std::string NameSet;
typedef std::string ConditionSet;
typedef std::string FinalSQL;
typedef std::string OrderByExp;
typedef std::string SetFiledSQL;

const static s32 DEFAULT_CONTEXT_LEN = 32;
const static s32 MYSQL_TABLE_NAME_LEN = 64;
const static char MYSQL_ESCAPE_CHAR = '`';
const static char MYSQL_STR_SPLIT = '\'';
const static char MYSQL_END_CHAR = ';';

const static char *MYSQL_SPILT_STR = ", ";
const static s32 MYSQL_ESCAPE_STR_ADD = 1;
const static char *MYSQL_KEYWORD_SELECT = "SELECT ";
const static char *MYSQL_KEYWORD_UPDATE = "UPDATE ";
const static char *MYSQL_KEYWORD_INSERT = "INSERT INTO ";
const static char *MYSQL_KEYWORD_SET = " SET ";
const static char *MYSQL_KEYWORD_FROM = " FROM ";
const static char *MYSQL_KEYWORD_WHERE = " WHERE ";
const static char *MYSQL_KEYWORD_ORDERBY = " ORDER BY ";
const static char *MYSQL_KEYWORD_DESC = " DESC ";
const static char *MYSQL_KEYWORD_ASC = " ASC ";
const static char *MYSQL_KEYWORD_LIMIT = " LIMIT ";
const static char *MYSQL_KEYWORD_EQ = " = ";


typedef s32(*ESCAPE_STR_FUN)(const void *context, s32 len, const char* pszSrc, int nSize, char* pszDest);

#define WHERE_LOGIC_EXP_DEC(H, name)\
	template< class T>\
SQLCommand & H##name(const char *filedName, SYMBOL symbol, T val)\

#define WHERE_LOGIC_EXP_DEF(H, name)\
template< class T>\
inline SQLCommand & SQLCommand::H##name(const char *filedName, SYMBOL symbol, T val)\
{\
	if (!_whereConditions.empty())\
	{\
		_whereConditions.append(" "#H#name" ");\
		AddFiledName(_whereConditions, filedName);\
		AddSymbol(_whereConditions, symbol);\
		AddFiledVal(_whereConditions, val);\
	}\
	return *this;\
}\
template<>\
inline SQLCommand & SQLCommand::H##name<const char *>(const char *filedName, SYMBOL symbol, const char *val)\
{\
	if (!_whereConditions.empty())\
	{\
		_whereConditions.append(" "#H#name" "); \
		AddFiledName(_whereConditions, filedName);\
		AddSymbol(_whereConditions, symbol);\
		AddFiledVal(_whereConditions, val, strlen(val));\
	}\
	return *this;\
}


class SQLCommand
{
public:
	enum SYMBOL
	{
		EQ = 1,
		LESS = 2,
	};
	SQLCommand(void *context, s32 size, ESCAPE_STR_FUN fun, const char *table) : _size(size), _fun(fun) {
		_context = size <= DEFAULT_CONTEXT_LEN ? _defaultContext : malloc(size);
		memcpy(_context, context, size);
		_size = size;
		_table = MYSQL_ESCAPE_CHAR;
		_table << table;
		_table << MYSQL_ESCAPE_CHAR;
	}
	virtual ~SQLCommand() 
	{
		if (_context != _defaultContext)
			free(_context);
	};

	virtual std::string & ToStr() = 0;
	virtual void clear() = 0;

	template< class T>
	SQLCommand & Where(const char *filedName, SYMBOL symbol, T val);
	WHERE_LOGIC_EXP_DEC(A,nd);
	WHERE_LOGIC_EXP_DEC(O,r);
protected:
	void AddFiledName(std::string &src, const char *filedName)
	{
		src.push_back(MYSQL_ESCAPE_CHAR);
		src.append(filedName);
		src.push_back(MYSQL_ESCAPE_CHAR);
	}

	void AddSymbol(std::string &src, SYMBOL symbol);
	template<typename T>
	void AddFiledVal(std::string &src, const T &val) {
		src.append(std::to_string(val));
	}
	void AddFiledVal(std::string &src, const char *val)
	{
		AddFiledVal(src, val, strlen(val));
	}

	void AddFiledVal(std::string &src, const char *val, s32 len)
	{
		char *dst = (char *)alloca(GetEscapeBuffSize(len));
		s32 dstLen = EscapeStr(val, len, dst);
		src.push_back(MYSQL_STR_SPLIT);
		src.append(dst, dstLen);
		src.push_back(MYSQL_STR_SPLIT);
	}

	s32 EscapeStr(const char* pszSrc, int nSize, char* pszDest){return _fun(_context, _size, pszSrc, nSize, pszDest);}
	s32 GetEscapeBuffSize(s32 nSize) { return 2 * nSize + MYSQL_ESCAPE_STR_ADD; };

protected:
	void *_context;
	s32 _size;
	char _defaultContext[DEFAULT_CONTEXT_LEN];
	ESCAPE_STR_FUN _fun;

	tlib::TString<MYSQL_TABLE_NAME_LEN> _table;
	ConditionSet	_whereConditions;
};

template< class T>
inline SQLCommand & SQLCommand::Where(const char *filedName, SYMBOL symbol, T val)
{
	if (_whereConditions.empty())
	{
		AddFiledName(_whereConditions, filedName);
		AddSymbol(_whereConditions, symbol);
		AddFiledVal(_whereConditions, val);
	}
	return *this;
}
template<>
inline SQLCommand & SQLCommand::Where<const char *>(const char *filedName, SYMBOL symbol, const char *val)
{
	if (_whereConditions.empty())
	{
		AddFiledName(_whereConditions, filedName);
		AddSymbol(_whereConditions, symbol);
		AddFiledVal(_whereConditions, val, strlen(val));
	}
	return *this;
}
WHERE_LOGIC_EXP_DEF(A, nd)
WHERE_LOGIC_EXP_DEF(O, r)

class QuerySQLCommand : public SQLCommand
{
public:
	QuerySQLCommand(void *context, s32 size, ESCAPE_STR_FUN fun, const char *table) : SQLCommand(context, size, fun, table){}

	virtual std::string & ToStr();
	virtual void clear();
public:
	QuerySQLCommand & Select(const char *name){
		if (!_selects.empty())
			_selects.append(MYSQL_SPILT_STR);
		AddFiledName(_selects, name);
		return *this;
	}

	template<typename... Args>
	QuerySQLCommand & Select(Args... args)
	{
		s32 a[] = { (Select(args),0)... };
		return *this;
	}

	QuerySQLCommand & OrderBy(const char *name, bool desc = true)
	{
		if (_orderBy.empty())
			_orderBy.append(MYSQL_KEYWORD_ORDERBY);
		else
			_orderBy.append(MYSQL_SPILT_STR);
		AddFiledName(_orderBy, name);
		if (desc)
			_orderBy.append(MYSQL_KEYWORD_DESC);
		else
			_orderBy.append(MYSQL_KEYWORD_ASC);
		return *this;
	}

	template<typename... Args>
	QuerySQLCommand & OrderBy(const char *name, bool second, Args... args)
	{
		OrderBy(name, second);
		OrderBy(args...);
		return *this;
	}

	template<typename... Args>
	QuerySQLCommand & OrderBy(const char *name, Args... args)
	{
		OrderBy(name);
		OrderBy(args...);
		return *this;
	}

	QuerySQLCommand & Limit(s32 count, s32 offset = 0)
	{
		if (_rowCount == 0)
		{
			_rowCount = count;
			_offSet = offset;
		}
		return *this;
	}

protected:
	NameSet	_selects;
	FinalSQL _final;
	OrderByExp _orderBy;
	s32 _rowCount = {0};
	s32 _offSet = {0};
};


class UpdateSQLCommand : public SQLCommand
{
public:
	UpdateSQLCommand(void *context, s32 size, ESCAPE_STR_FUN fun, const char *table) : SQLCommand(context, size, fun, table){}
	virtual std::string & ToStr();
	virtual void clear();

	template<typename T>
	UpdateSQLCommand & SetFiled(const char *name, const T &val)
	{
		if (!_setFiledSQL.empty())
			_setFiledSQL.append(MYSQL_SPILT_STR);

		AddFiledName(_setFiledSQL, name);
		_setFiledSQL.append(MYSQL_KEYWORD_EQ);
		AddFiledVal(_setFiledSQL, val);
		return *this;
	}

	template<typename T, typename... Args>
	UpdateSQLCommand & SetFiled(const char *name, const T &val, Args... args)
	{
		SetFiled(name, val);
		SetFiled(args...);
		return *this;
	}

	UpdateSQLCommand & SetBlobFiled(const char *name, const void *content, s32 len)
	{
		if (!_setFiledSQL.empty())
			_setFiledSQL.append(MYSQL_SPILT_STR);

		AddFiledName(_setFiledSQL, name);
		_setFiledSQL.append(MYSQL_KEYWORD_EQ);
		AddFiledVal(_setFiledSQL, (const char *)content, len);
		return *this;
	}

protected:
private:
	SetFiledSQL	_setFiledSQL;
	FinalSQL _final;
};

class InsertSQLCommand : public SQLCommand
{
public:
	InsertSQLCommand(void *context, s32 size, ESCAPE_STR_FUN fun, const char *table) : SQLCommand(context, size, fun, table) {}
	virtual std::string & ToStr();
	virtual void clear();

	template<typename T>
	InsertSQLCommand & SetFiled(const char *name, const T &val)
	{
		if (!_fileds.empty())
		{
			_fileds.append(MYSQL_SPILT_STR);
			_values.append(MYSQL_SPILT_STR);
		}

		AddFiledName(_fileds, name);
		AddFiledVal(_values, val);
		return *this;
	}

	template<typename T, typename... Args>
	InsertSQLCommand & SetFiled(const char *name, const T &val, Args... args)
	{
		SetFiled(name, val);
		SetFiled(args...);
		return *this;
	}

	InsertSQLCommand & SetBlobFiled(const char *name, const void *content, s32 len)
	{
		if (!_fileds.empty())
		{
			_fileds.append(MYSQL_SPILT_STR);
			_values.append(MYSQL_SPILT_STR);
		}

		AddFiledName(_fileds, name);
		AddFiledVal(_values, (const char *)content, len);
		return *this;
	}
protected:
private:
	FinalSQL _final;
	std::string _fileds;
	std::string _values;
};

class SaveSQLCommand : public SQLCommand
{
public:
	SaveSQLCommand(void *context, s32 size, ESCAPE_STR_FUN fun, const char *table) : SQLCommand(context, size, fun, table) {}
	virtual std::string & ToStr();
	virtual void clear();


	struct FiledNode 
	{
		template<typename T>
		FiledNode(SaveSQLCommand &saveCommand, const char *name, const T &val)
		{
			_filed = name;
			saveCommand.AddFiledVal(_val, val);
		}
		FiledNode(SaveSQLCommand &saveCommand, const char *name, const void *content, s32 len)
		{
			_filed = name;
			saveCommand.AddFiledVal(_val, (const char *)content, len);
		}
		const char *_filed;
		std::string _val;
	};

	template<typename T>
	SaveSQLCommand & SetKey(const char *name, const T &val)
	{
		_keys.emplace_back(*this, name, val);
		return *this;
	}

	template<typename T, typename... Args>
	SaveSQLCommand & SetKey(const char *name, const T &val, Args... args)
	{
		SetKey(name, val);
		SetKey(args...);
		return *this;
	}

	template<typename T>
	SaveSQLCommand & SetFiled(const char *name, const T &val)
	{
		_clumns.emplace_back(*this, name, val);
		return *this;
	}

	template<typename T, typename... Args>
	SaveSQLCommand & SetFiled(const char *name, const T &val, Args... args)
	{
		SetFiled(name, val);
		SetFiled(args...);
		return *this;
	}

	SaveSQLCommand & SetBlobFiled(const char *name, const void *content, s32 len)
	{
		_clumns.emplace_back(*this, name, content, len);
		return *this;
	}
protected:
private:
	FinalSQL _final;
	std::vector<FiledNode> _keys;
	std::vector<FiledNode> _clumns;
};

#endif

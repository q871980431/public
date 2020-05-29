#include "MysqlBuilder.h"
void SQLCommand::AddSymbol(std::string &src, SYMBOL symbol)
{
	switch (symbol)
	{
	case EQ: src.append(" = "); break;
	case LESS: src.append(" < "); break;
	//case EQ: src.append(" == "); break;
	}
}

std::string & QuerySQLCommand::ToStr()
{
	if (_final.empty())
	{
		_final.append(MYSQL_KEYWORD_SELECT);
		_final.append(_selects);
		_final.append(MYSQL_KEYWORD_FROM);
		_final.append(_table.GetString());
		_final.append(MYSQL_KEYWORD_WHERE);
		_final.append(_whereConditions);
		_final.append(_orderBy);
		if (_rowCount != 0)
		{
			_final.append(MYSQL_KEYWORD_LIMIT);
			AddFiledVal(_final, _offSet);
			_final.append(MYSQL_SPILT_STR);
			AddFiledVal(_final, _rowCount);
		}
		_final.push_back(MYSQL_END_CHAR);
	}
	return _final;
}

void QuerySQLCommand::clear()
{
	_selects.clear();
	_final.clear();
	_orderBy.clear();
	_rowCount = 0;
	_offSet = 0;
}

std::string & UpdateSQLCommand::ToStr()
{
	if (_final.empty())
	{
		_final.append(MYSQL_KEYWORD_UPDATE);
		_final.append(_table.GetString());
		_final.append(MYSQL_KEYWORD_SET);
		_final.append(_setFiledSQL.c_str());
		_final.append(MYSQL_KEYWORD_WHERE);
		_final.append(_whereConditions);
		_final.push_back(MYSQL_END_CHAR);
	}

	return _final;
}

void UpdateSQLCommand::clear()
{
	_final.clear();
	_setFiledSQL.clear();
}

std::string & InsertSQLCommand::ToStr()
{
	if (_final.empty())
	{
		_final.append(MYSQL_KEYWORD_INSERT);
		_final.append(_table.GetString());
		_final.append(" ( ");
		_final.append(_fileds.c_str());
		_final.append(" ) VALUES ( ");
		_final.append(_values);
		_final.append(" )");
		_final.push_back(MYSQL_END_CHAR);
	}

	return _final;
}

void InsertSQLCommand::clear()
{
	_final.clear();
	_fileds.clear();
	_values.clear();
}

std::string & SaveSQLCommand::ToStr()
{
	if (_final.empty())
	{
		_final.append(MYSQL_KEYWORD_INSERT);
		_final.append(_table.GetString());
		_final.append(" ( ");
		bool mark = false;
		for (auto &iter : _keys)
		{
			if (mark)
				_final.append(MYSQL_SPILT_STR);
			AddFiledName(_final, iter._filed);
			mark = true;
		}
		for (auto &iter : _clumns)
		{
			if (mark)
				_final.append(MYSQL_SPILT_STR);
			AddFiledName(_final, iter._filed);
			mark = true;
		}
		_final.append(" ) VALUES ( ");
		mark = false;
		for (auto &iter : _keys)
		{
			if (mark)
				_final.append(MYSQL_SPILT_STR);
			_final.append(iter._val.c_str());
			mark = true;
		}
		for (auto &iter : _clumns)
		{
			if (mark)
				_final.append(MYSQL_SPILT_STR);
			_final.append(iter._val.c_str());
			mark = true;
		}
		_final.append(" ) ON DUPLICATE KEY UPDATE ");
		mark = false;
		for (auto &iter : _clumns)
		{
			if (mark)
				_final.append(MYSQL_SPILT_STR);
			AddFiledName(_final, iter._filed);
			_final.append(MYSQL_KEYWORD_EQ);
			_final.append(iter._val);
			mark = true;
		}
		_final.push_back(MYSQL_END_CHAR);
	}

	return _final;
}

void SaveSQLCommand::clear()
{
	_final.clear();
	_keys.clear();
	_clumns.clear();
}
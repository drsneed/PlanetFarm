#include "DbInterface.h"

namespace
{
	void _CreateEntityTable(Db::Connection& connection)
	{
		connection.Execute(
			SQL(
				CREATE TABLE `Entity` (
					`ID`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
					`TileID`	INTEGER NOT NULL,
					`Data`	BLOB )
			)
		);
	}
}

void DbInterface::CreateSaveGameDb(const wchar_t* const filename)
{
	auto connection = Db::Connection(filename);
	_CreateEntityTable(connection);
}
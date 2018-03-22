#include "DbInterface.h"

namespace
{
	void _CreateResourceTable(Db::Connection& connection)
	{
		connection.Execute(
			SQL(
				CREATE TABLE `Resource` (
					`TileID`	INTEGER NOT NULL,
					`ResourceID` INTEGER NOT NULL,
					`Payload`	BLOB )
			)
		);
	}
}

void DbInterface::CreateSaveGameDb(const char* const filename, bool create_test_data)
{
	auto connection = Db::Connection(filename);
	_CreateResourceTable(connection);
	if (create_test_data)
	{
		char payload[6] = { 'S', 'T', 'E', 'A', 'K', '\0' };
		Resource resource{
			0, // rowid
			0, // tileid
			(1 << 4) | ZOOM_MASK, // resourceid
			Payload(payload, 6)
		};
		DbInterface::PutResource(connection, resource);
	}
}

std::vector<ResourceID> DbInterface::QueryResourceIDs(Db::Connection& conn, TileID tile_id)
{
	std::vector<ResourceID> result;

	auto query = SQL(SELECT ResourceID FROM Resource WHERE TileID = ?);
	auto rows = Db::Statement(conn, query, tile_id);
	
	for (auto& row : rows)
	{
		// Resource IDs need the zoom mask to differentiate between tile id
		result.push_back(static_cast<ResourceID>(row.GetInt()));
	}

	return result;
}

void DbInterface::PutResource(Db::Connection& conn, Resource& resource)
{
	if (resource.row_id == 0)
	{
		auto query = SQL(INSERT INTO Resource(TileID, ResourceID, Payload) VALUES(?, ?, ?));
		Db::Statement statement(conn, query);
		statement.Bind(1, resource.tile_id);
		statement.Bind(2, resource.resource_id);
		statement.Bind(3, (const void*)resource.payload.blob, resource.payload.blob_size);
		statement.Execute();
		resource.row_id = conn.RowId();
	}
	else
	{
		auto query = SQL(UPDATE Resource SET Payload=? WHERE rowid=?);
		Db::Statement statement(conn, query);
		statement.Bind(1, (const void*)resource.payload.blob, resource.payload.blob_size);
		statement.Bind(2, resource.row_id);
		statement.Execute();
	}
}

bool DbInterface::GetResource(Db::Connection& conn, TileID tile_id, ResourceID resource_id, Resource& out)
{
	auto query = SQL(SELECT rowid,* FROM Resource WHERE TileID = ? AND ResourceID = ? LIMIT 1);
	Db::Row row;
	Db::Statement statement(conn, query, tile_id, resource_id);

	if (statement.GetSingle(row))
	{
		out.row_id = static_cast<int64_t>(row.GetInt(0));
		out.resource_id = static_cast<ResourceID>(row.GetInt(1));
		out.tile_id = static_cast<TileID>(row.GetInt(2));
		int payload_size;
		auto payload = row.GetBlob(payload_size, 3);
		out.payload = Payload(payload, payload_size);
		return true;
	}

	return false;
}

bool DbInterface::GetResource(Db::Connection& conn, int64_t row_id, Resource& out)
{
	auto query = SQL(SELECT ResourceID, TileID, Payload FROM Resource WHERE rowid = ? LIMIT 1);
	Db::Row row;
	Db::Statement statement(conn, query, row_id);

	if (statement.GetSingle(row))
	{
		out.row_id = row_id;
		out.resource_id = static_cast<ResourceID>(row.GetInt(0));
		out.tile_id = static_cast<TileID>(row.GetInt(1));
		int payload_size;
		auto payload = row.GetBlob(payload_size, 2);
		out.payload = Payload(payload, payload_size);
		return true;
	}
	
	return false;
}
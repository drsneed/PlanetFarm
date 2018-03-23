#include "DbInterface.h"

namespace
{
	void _CreateResourceTable(Db::Connection& connection)
	{
		connection.Execute(
			SQL(
				CREATE TABLE `Resource` (
					`TileID`	INTEGER NOT NULL,
					`Type` INTEGER NOT NULL,
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
		{
			char payload[] = { 'S', 'T', 'E', 'A', 'K', '\0' };
			Resource resource(
				0, // resource id
				Tile(0, 0, 0).GetID(), // tileid
				ResourceType::ResidentialBuilding1,
				payload,
				sizeof(payload)
			);
			DbInterface::PutResource(connection, resource);
		}
		{
			char payload[] = { 'B', 'A', 'N', 'A', 'N', 'A', '\0' };
			Resource resource(
				0, // resource id
				Tile(0, 1, 1).GetID(), // tileid
				ResourceType::ResidentialBuilding1,
				payload,
				sizeof(payload)
			);
			DbInterface::PutResource(connection, resource);
		}
		{
			char payload[] = { 'A', 'P', 'P', 'L', 'E', '\0' };
			Resource resource(
				0, // resource id
				Tile(0, 1, 1).GetID(), // tileid
				ResourceType::ResidentialBuilding1,
				payload,
				sizeof(payload)
			);
			DbInterface::PutResource(connection, resource);
		}
		{
			char payload[] = { 'B', 'U', 'D', ' ', 'L', 'I', 'G', 'H', 'T', '\0' };
			Resource resource(
				0, // resource id
				Tile(0, 1, 1).GetID(), // tileid
				ResourceType::ResidentialBuilding1,
				payload,
				sizeof(payload)
			);
			DbInterface::PutResource(connection, resource);
		}
		{
			char payload[] = { 'F', 'R', 'E', 'N', 'C', 'H', ' ', 'F', 'R', 'I', 'E', 'S', '\0' };
			Resource resource(
				0, // resource id
				Tile(0, 1, 1).GetID(), // tileid
				ResourceType::ResidentialBuilding1,
				payload,
				sizeof(payload)
			);
			DbInterface::PutResource(connection, resource);
		}
		{
			char payload[] = { 'S', 'O', 'D', 'A', '\0' };
			Resource resource(
				0, // resource id
				Tile(1, 3, 2).GetID(), // tileid
				ResourceType::ResidentialBuilding1,
				payload,
				sizeof(payload)
			);
			DbInterface::PutResource(connection, resource);
		}

	}
}

std::vector<ResourceID> DbInterface::QueryResourceIDs(Db::Connection& conn, TileID tile_id)
{
	std::vector<ResourceID> result;

	auto query = SQL(SELECT [rowid] FROM Resource WHERE TileID = ?);
	auto rows = Db::Statement(conn, query, tile_id);
	
	for (auto& row : rows)
	{
		result.push_back(row.GetInt64());
	}

	return result;
}

void DbInterface::PutResource(Db::Connection& conn, Resource& resource)
{
	if (resource.id == 0)
	{
		auto query = SQL(INSERT INTO Resource(TileID, Type, Payload) VALUES(?, ?, ?));
		Db::Statement statement(conn, query);
		statement.Bind(1, resource.tile_id);
		statement.Bind(2, (int)resource.type);
		statement.Bind(3, (const void*)resource.payload.blob, resource.payload.blob_size);
		statement.Execute();
		resource.id = conn.RowId();
	}
	else
	{
		auto query = SQL(UPDATE Resource SET Payload=? WHERE [rowid]=?);
		Db::Statement statement(conn, query);
		statement.Bind(1, (const void*)resource.payload.blob, resource.payload.blob_size);
		statement.Bind(2, resource.id);
		statement.Execute();
	}
}

Resource DbInterface::GetResource(Db::Connection& conn, ResourceID id)
{
	auto query = "SELECT TileID, Type, Payload FROM Resource WHERE [rowid] = ? LIMIT 1";
	Db::Row row;
	Db::Statement statement(conn, query, id);
	Resource resource;
	if (statement.GetSingle(row))
	{
		resource.id = id;
		resource.tile_id = static_cast<TileID>(row.GetInt(0));
		resource.type = static_cast<ResourceType>(row.GetInt(1));
		int payload_size;
		auto payload = row.GetBlob(payload_size, 2);
		resource.payload = Payload(payload, payload_size);
	}
	return resource;
}
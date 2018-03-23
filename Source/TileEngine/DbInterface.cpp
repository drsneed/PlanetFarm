#include "DbInterface.h"

namespace
{
	void _CreateFeatureTable(Db::Connection& connection)
	{
		connection.Execute(
			SQL(
				CREATE TABLE `Feature` (
					`Name`		TEXT NULL,
					`TileID`	INTEGER NOT NULL,
					`Type`		INTEGER NOT NULL,
					`PosX`		REAL NOT NULL,
					`PosY`		REAL NOT NULL,
					`Points`	BLOB NULL)
			)
		);
	}
}

void DbInterface::CreateSaveGameDb(const char* const filename, bool create_test_data)
{
	auto connection = Db::Connection(filename);
	_CreateFeatureTable(connection);
	if (create_test_data)
	{
		{
			Feature feature(
				std::string("Big Island"),
				Tile(0, 0, 0).GetID(), // tileid
				FeatureType::Unknown, // type
				XMFLOAT2(0.5f, 0.5f)
			);
			DbInterface::PutFeature(connection, feature);
		}
		{
			Feature feature(
				std::string("Static Object"),
				Tile(1, 0, 1).GetID(), // tileid
				FeatureType::Unknown, // type
				XMFLOAT2(0.5f, 0.5f)
			);
			DbInterface::PutFeature(connection, feature);
		}
		{
			Feature feature(
				std::string("Dynamic Object 1"),
				Tile(1, 2, 2).GetID(), // tileid
				FeatureType::Unknown, // type
				XMFLOAT2(0.1f, 0.1f),
				{ { 0.125f, 0.223f },{ 0.430f, 0.987f } }
			);
			DbInterface::PutFeature(connection, feature);
		}
		{
			Feature feature(
				std::string("Dynamic Object 2"),
				Tile(1, 2, 2).GetID(), // tileid
				FeatureType::Unknown, // type
				XMFLOAT2(0.9f, 0.9f),
				{ { 0.180f, 0.803f },{ 0.630f, 0.5f } }
			);
			DbInterface::PutFeature(connection, feature);
		}
	}
}

std::vector<FeatureID> DbInterface::GetFeatureIDs(Db::Connection& conn, TileID tile_id)
{
	std::vector<FeatureID> result;

	auto query = SQL(SELECT [rowid] FROM Feature WHERE TileID = ?);
	auto rows = Db::Statement(conn, query, tile_id);
	
	for (auto& row : rows)
	{
		result.push_back(row.GetInt64());
	}

	return result;
}

void DbInterface::PutFeature(Db::Connection& conn, Feature& feature)
{
	if (feature.GetID() == 0)
	{
		auto query = "INSERT INTO Feature(Name, TileID, Type, PosX, PosY, Points) VALUES(?, ?, ?, ?, ?, ?)";
		Db::Statement statement(conn, query);
		statement.Bind(1, feature.GetName());
		statement.Bind(2, feature.GetTileID());
		statement.Bind(3, feature.GetTypeInt());
		auto pos = feature.GetPos();
		statement.Bind(4, pos.x);
		statement.Bind(5, pos.y);
		if (feature.HasPoints())
		{
			auto& points = feature.GetPointsRef();
			statement.Bind(6, static_cast<const void*>(&points[0]), points.size() * sizeof(points[0]));
		}
		else
		{
			statement.Bind(6, static_cast<const void*>(nullptr), 0);
		}
		statement.Execute();
		feature.SetID(conn.RowId());
	}
	else
	{
		auto query = "UPDATE Feature SET Name=?, TileID=?, Type=?, PosX=?, PosY=?, Points=? WHERE [rowid]=?";
		Db::Statement statement(conn, query);
		statement.Bind(1, feature.GetName());
		statement.Bind(2, feature.GetTileID());
		statement.Bind(3, feature.GetTypeInt());
		auto pos = feature.GetPos();
		statement.Bind(4, pos.x);
		statement.Bind(5, pos.y);
		if (feature.HasPoints())
		{
			auto& points = feature.GetPointsRef();
			statement.Bind(6, static_cast<const void*>(&points[0]), points.size() * sizeof(points[0]));
		}
		else
		{
			statement.Bind(6, static_cast<const void*>(nullptr), 0);
		}
		
		statement.Bind(7, feature.GetID());
		statement.Execute();
	}
}

Feature DbInterface::GetFeature(Db::Connection& conn, FeatureID id)
{
	auto query = "SELECT Name, TileID, Type, PosX, PosY, Points FROM Feature WHERE [rowid] = ? LIMIT 1";
	Db::Row row;
	Db::Statement statement(conn, query, id);
	if (statement.GetSingle(row))
	{
		auto name = std::string(row.GetString(0), row.GetStringLength());
		auto tile_id = static_cast<TileID>(row.GetInt(1));
		auto type = static_cast<FeatureType>(row.GetInt(2));
		auto posx = row.GetFloat(3);
		auto posy = row.GetFloat(4);
		auto points = static_cast<const XMFLOAT2*>(row.GetBlob(5));
		auto points_size = row.GetBlobSize(5);
		return Feature(id, name, tile_id, type, XMFLOAT2(posx, posy), points, points_size);
	}
	return Feature();
}
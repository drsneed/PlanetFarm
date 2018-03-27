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
					`Rot`		REAL NOT NULL,
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
		Feature feature(
			std::string("Outline"),
			Tile(0, 0, 0).GetID(), // tileid
			FeatureType::Unknown, // type
			XMFLOAT2(0.5f, 0.5f),
			0.0f, // rot
			{ 
				{ -120.88f, -118.75f },
				{ -117.50f, -117.63f },
				{ -111.88f, -109.63f },
				{ -110.63f, -96.00f },
				{ -95.88f, -85.75f },
				{ -85.63f, -89.25f },
				{ -76.63f, -99.38f },
				{ -72.00f, -110.75f },
				{ -63.00f, -117.63f },
				{ -47.00f, -115.38f },
				{ -41.38f, -107.25f },
				{ -36.88f, -100.38f },
				{ -26.63f, -85.63f },
				{ -13.00f, -74.25f },
				{ 4.00f, -68.63f },
				{ 15.50f, -74.25f },
				{ 25.75f, -85.63f },
				{ 30.25f, -99.25f },
				{ 61.00f, -110.75f },
				{ 86.00f, -110.75f },
				{ 96.25f, -101.75f },
				{ 99.75f, -86.88f },
				{ 99.75f, -64.13f },
				{ 91.75f, -50.38f },
				{ 79.25f, -41.38f },
				{ 79.25f, -30.00f },
				{ 87.25f, -16.38f },
				{ 102.00f, 0.75f },
				{ 98.50f, 28.00f },
				{ 79.25f, 41.50f },
				{ 70.25f, 51.75f },
				{ 69.00f, 65.50f },
				{ 74.75f, 73.50f },
				{ 88.25f, 80.25f },
				{ 98.50f, 88.25f },
				{ 106.50f, 96.25f },
				{ 111.00f, 104.25f },
				{ 116.75f, 115.50f },
				{ 121.25f, 120.25f },
				{ 112.25f, 120.25f },
				{ 103.00f, 116.75f },
				{ 94.00f, 112.25f },
				{ 78.00f, 111.00f },
				{ 71.25f, 112.25f },
				{ 62.25f, 113.25f },
				{ 46.25f, 116.75f },
				{ 30.25f, 116.75f },
				{ 21.25f, 112.25f },
				{ 5.25f, 103.00f },
				{ -7.63f, 98.25f },
				{ -27.00f, 84.75f },
				{ -32.63f, 70.00f },
				{ -33.88f, 56.25f },
				{ -32.63f, 38.00f },
				{ -32.63f, 14.25f },
				{ -38.50f, -3.13f },
				{ -65.75f, -8.75f },
				{ -76.00f, 3.75f },
				{ -78.25f, 25.50f },
				{ -78.25f, 51.50f },
				{ -74.88f, 65.25f },
				{ -77.13f, 85.75f },
				{ -90.75f, 96.00f },
				{ -118.13f, 92.50f },
				{ -120.50f, 67.50f },
				{ -117.00f, 38.75f },
				{ -114.75f, 17.25f },
				{ -113.50f, -0.00f },
				{ -114.63f, -15.88f },
				{ -115.75f, -43.38f },
				{ -118.13f, -58.63f },
				{ -120.50f, -72.38f },
				{ -121.63f, -88.25f },
				{ -121.63f, -105.75f },
				{ -120.88f, -118.75f }
			}
		);
		DbInterface::PutFeature(connection, feature);
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
		auto query = "INSERT INTO Feature(Name, TileID, Type, PosX, PosY, Rot, Points) VALUES(?, ?, ?, ?, ?, ?, ?)";
		Db::Statement statement(conn, query);
		int i = 1;
		statement.Bind(i++, feature.GetName());
		statement.Bind(i++, feature.GetTileID());
		statement.Bind(i++, feature.GetTypeInt());
		auto pos = feature.GetPosition();
		statement.Bind(i++, pos.x);
		statement.Bind(i++, pos.y);
		statement.Bind(i++, feature.GetRotation());
		if (feature.HasPoints())
		{
			auto& points = feature.GetPointsRef();
			statement.Bind(i++, static_cast<const void*>(&points[0]), points.size() * sizeof(points[0]));
		}
		else
		{
			statement.Bind(i++, static_cast<const void*>(nullptr), 0);
		}
		statement.Execute();
		feature.SetID(conn.RowId());
	}
	else
	{
		auto query = "UPDATE Feature SET Name=?, TileID=?, Type=?, PosX=?, PosY=?, Rot=?, Points=? WHERE [rowid]=?";
		Db::Statement statement(conn, query);
		int i = 1;
		statement.Bind(i++, feature.GetName());
		statement.Bind(i++, feature.GetTileID());
		statement.Bind(i++, feature.GetTypeInt());
		auto pos = feature.GetPosition();
		statement.Bind(i++, pos.x);
		statement.Bind(i++, pos.y);
		statement.Bind(i++, feature.GetRotation());
		if (feature.HasPoints())
		{
			auto& points = feature.GetPointsRef();
			statement.Bind(i++, static_cast<const void*>(&points[0]), points.size() * sizeof(points[0]));
		}
		else
		{
			statement.Bind(i++, static_cast<const void*>(nullptr), 0);
		}
		
		statement.Bind(i++, feature.GetID());
		statement.Execute();
	}
}

Feature DbInterface::GetFeature(Db::Connection& conn, FeatureID id)
{
	auto query = "SELECT Name, TileID, Type, PosX, PosY, Rot, Points FROM Feature WHERE [rowid] = ? LIMIT 1";
	Db::Row row;
	Db::Statement statement(conn, query, id);
	if (statement.GetSingle(row))
	{
		int i = 0;
		auto name = row.GetString(i);
		auto name_length = row.GetStringLength(i++);
		auto tile_id = static_cast<TileID>(row.GetInt(i++));
		auto type = static_cast<FeatureType>(row.GetInt(i++));
		auto posx = row.GetFloat(i++);
		auto posy = row.GetFloat(i++);
		auto rot = row.GetFloat(i++);
		auto points = static_cast<const XMFLOAT2*>(row.GetBlob(i));
		auto points_size = row.GetBlobSize(i);
		return Feature(id, std::string(name, name_length), tile_id, type, XMFLOAT2(posx, posy), rot, points, (points_size / sizeof(XMFLOAT2)));
	}
	return Feature();
}
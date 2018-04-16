#include "DbInterface.h"
#include <MapGeneration/LandGenerator.h>
//https://blog.mapbox.com/rendering-big-geodata-on-the-fly-with-geojson-vt-4e4d2a5dd1f2
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
		double max_bounds = static_cast<double>(MAP_WIDTH_MAX_ZOOM);
		LandGenerator generator(time(NULL), { max_bounds, max_bounds }, MAP_WIDTH_MAX_ZOOM / 32.0);
		auto& mesh = generator.GetMesh();
		auto coastlines = generator.GetCoastlines();
		for (int i = 0; i < coastlines.size(); ++i)
		{
			std::vector<XMFLOAT2> vertices(coastlines[i].size() + 1);
			for (int v = 0; v < coastlines[i].size(); ++v)
			{
				vertices[v] = XMFLOAT2(mesh.region_vertices[coastlines[i][v]].x - MAP_ABSOLUTE_CENTER, mesh.region_vertices[coastlines[i][v]].y - MAP_ABSOLUTE_CENTER);
			}
			vertices[coastlines[i].size()] = vertices[0];

			Feature feature(
				std::string("Island"),
				Tile(0, 0, 0).GetID(), // tileid
				FeatureType::Unknown, // type
				XMFLOAT2(0.5f, 0.5f),
				0.0f, // rot
				vertices
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
		if (feature.IsDynamic())
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
		if (feature.IsDynamic())
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
// PlanetFarm.cpp : Defines the entry point for the application.
//

#include <DirectXColors.h>
#include <Core/StdIncludes.h>
#include <Core/Embed.h>
#include <Core/GraphicsWindow.h>
#include <Core/Logger.h>

#include <Core/Font.h>

#include <Game/TextRenderer.h>


#include <Game/MapRenderer.h>
#include <random>
#include <ctime>
#include <Core/ColorConverter.h>
#include <Game/Camera.h>
#include <Game/CameraBehaviorMap.h>
#include <TileEngine/Tile.h>
#include <TileEngine/DbInterface.h>
#include <Game/Map.h>
#include <Core/Db.h>
#include "Shlwapi.h"
#include <bitset>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

const char* db_name = "Data/SaveGame.db";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	OleInitialize(NULL);
	//std::bitset<32> bits((uint32_t)((1 << 4) | (1 << 1)));
	//PRINTF(L"%S\n", bits.to_string().c_str());
	//PRINTF(L"%u\n", (uint32_t)((1 << 4) | (1 << 1)));


	auto window = GraphicsWindow::CreateInstance(L"Planet Farm", 1366, 768, false);
	//window->ShowCursor(false);
	auto font = std::make_shared<Font>("Data\\Cantarell.fnt");
	auto text_renderer = std::make_unique<TextRenderer>(font.get());



	window->Show();
	if (D3DErrorOccurred())
	{
		window->Close();
	}

	auto map_cam = std::make_shared<Camera>(std::make_shared<CameraBehaviorMap>());
	//map_cam->SetPosition(MAP_ABSOLUTE_CENTER, 1000.f, MAP_ABSOLUTE_CENTER);
	map_cam->SetPosition(0.0f, 38.f, 0.0f);
	map_cam->Pitch(90.0f);

	if (!PathFileExistsA(db_name))
	{
		DbInterface::CreateSaveGameDb(db_name, true);
	}

	auto map = std::make_unique<Map>(map_cam, db_name);
	map->SetZoom(0, 0);
	//camera->RotateY(XMConvertToRadians(-135.0f));
	//camera->Pitch(XMConvertToRadians(30.f));


	//const FLOAT bg[4] = { 0.14f, 0.34f, 0.34f, 1.0f };
	const FLOAT bg[4] = { 0.128f, 0.128f, 0.128f, 1.0f };

	//RunTileTest();

	GraphicsWindow::Event windowEvent;
	while (window->IsOpen())
	{
		// Make the window retrieve windows messages
		window->Tick();

		// Handle window events
		while (window->PollEvent(windowEvent))
		{
			if (windowEvent.type == GraphicsWindow::Event::Type::KeyRelease &&
				windowEvent.code == GraphicsWindow::Event::Code::Escape)
			{
				window->Close();
			}
			// Dispatch windowEvent to subsystems
			map_cam->HandleEvent(windowEvent);
			map->HandleEvent(windowEvent);

		}

		auto delta_time = window->GetTimer()->GetDeltaTime();

		// Perform Updating
		map_cam->Tick(delta_time);
		map->Tick(delta_time);


		// Perform Rendering
		if (!window->IsMinimized())
		{
			window->Clear(bg);

			map->RenderScene();

			//map_renderer->DrawGrid(map_cam);


			auto zoom = map->GetZoom();
			auto pos = map_cam->GetPosition();
			auto cursor_pos = window->GetMousePosition();
			auto map_cursor = map->GetCursor();
			text_renderer->PreparePipeline();
			text_renderer->Printf(10.f, 10.f, 0.01f, 0xFFFFFFFF, 0.8f, "CAM: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
			text_renderer->Printf(10.f, 30.f, 0.01f, 0xDBB600FF, 0.8f, "FPS: %.2f", window->GetTimer()->GetFPS());
			text_renderer->Printf(10.f, 50.f, 0.01f, 0xFFFFFFFF, 0.8f, "SCREEN_CURSOR: (%.2f, %.2f)", cursor_pos.x, cursor_pos.y);
			text_renderer->Printf(10.f, 70.f, 0.01f, 0xFFFFFFFF, 0.8f, "MAP_CURSOR: (%.2f, %.2f)", map_cursor.x, map_cursor.y);
			text_renderer->Printf(10.f, 90.f, 0.01f, 0xFFFFFFFF, 0.8f, "ZOOM: %d.%d", zoom.major_part, zoom.minor_part );
			text_renderer->RestorePipeline();

			window->RenderSciterUI();
			window->Present();
		}

		// Perform error checking
		if (D3DErrorOccurred())
		{
			window->Close();
		}
	}

	/*		ID3D11Debug* debug;
	auto device = window->GetDevice();
	device->QueryInterface(IID_PPV_ARGS(&debug));
	debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);*/


	GraphicsWindow::DestroyInstance();

	if (D3DErrorOccurred())
	{
		printf("A D3D Error Occured.\n");
	}
	return 0;

}
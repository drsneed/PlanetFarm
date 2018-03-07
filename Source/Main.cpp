// PlanetFarm.cpp : Defines the entry point for the application.
//

#include "StdIncludes.h"
#include "Embed.h"
#include "GraphicsWindow.h"
#include "Logger.h"
#include <DirectXColors.h>
#include "Font.h"
#include "TextRenderer.h"

#include "Camera.h"
#include "MapRenderer.h"
#include <random>
#include <ctime>
#include "ColorConverter.h"
#include "FirstPersonCamera.h"
#include "Player.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static bool ShowLaunchWindow();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	auto startGame = true;//ShowLaunchWindow();

	if (startGame)
	{
		auto window = GraphicsWindow::CreateInstance(800, 600, false);
		window->ShowCursor(false);
		auto font = std::make_shared<Font>("Data\\Cantarell.fnt");
		auto text_renderer = std::make_unique<TextRenderer>(font.get());



		window->Show();
		if (D3DErrorOccurred())
		{
			window->Close();
		}


		Player me;
		auto worldRenderer = std::make_unique<WorldRenderer>();

		//camera->RotateY(XMConvertToRadians(-135.0f));
		//camera->Pitch(XMConvertToRadians(30.f));


		const FLOAT bg[4] = { 0.14f, 0.34f, 0.34f, 1.0f };

		GraphicsWindow::Event windowEvent;
		while (window->IsOpen())
		{
			// Make the window retrieve windows messages
			window->Tick();
			
			// Handle window events
			while (window->PollEvent(windowEvent))
			{
				if(windowEvent.type == GraphicsWindow::Event::Type::KeyRelease &&
				   windowEvent.code == GraphicsWindow::Event::Code::Escape)
				{
					window->Close();
				}
				// Dispatch windowEvent to subsystems
				me.HandleEvent(windowEvent);


			}

			auto dt = window->GetTimer()->GetDeltaTime();

			// Perform Updating
			me.Update(dt, false);


			// Perform Rendering
			if (!window->IsMinimized())
			{
				window->Clear(bg);
				
				worldRenderer->RenderGrid(me.GetCamera());

				auto pos = me.GetCamera()->GetPosition();
				text_renderer->PreparePipeline();
				text_renderer->Printf(10.f, 10.f, 0.01, 0xFFFFFFFF, 1.0f, "Camera Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
				text_renderer->Printf(10.f, 40.f, 0.01, 0xDBB600FF, 1.0f, "FPS: %.2f", window->GetTimer()->GetFPS());
				text_renderer->RestorePipeline();
				window->Present();
			}

			me.EndFrame();

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


	}
	if (D3DErrorOccurred())
	{
		printf("A D3D Error Occured.\n");
	}
	return 0;

}
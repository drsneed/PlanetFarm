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
#include <Game/FlyCamera.h>


#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	auto window = GraphicsWindow::CreateInstance(800, 600, false);
	//window->ShowCursor(false);
	auto font = std::make_shared<Font>("Data\\Cantarell.fnt");
	auto text_renderer = std::make_unique<TextRenderer>(font.get());



	window->Show();
	if (D3DErrorOccurred())
	{
		window->Close();
	}

	auto worldRenderer = std::make_unique<WorldRenderer>();

	//camera->RotateY(XMConvertToRadians(-135.0f));
	//camera->Pitch(XMConvertToRadians(30.f));
	auto fly_cam = std::make_shared<FlyCamera>();

	const FLOAT bg[4] = { 0.14f, 0.34f, 0.34f, 1.0f };

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
			fly_cam->HandleEvent(windowEvent);


		}

		auto delta_time = window->GetTimer()->GetDeltaTime();

		// Perform Updating
		fly_cam->Tick(delta_time);


		// Perform Rendering
		if (!window->IsMinimized())
		{
			window->Clear(bg);

			//worldRenderer->RenderGrid(std::static_pointer_cast<CameraBase>(fly_cam));

			auto pos = fly_cam->GetPosition();
			/*text_renderer->PreparePipeline();
			text_renderer->Printf(10.f, 10.f, 0.01f, 0xFFFFFFFF, 1.0f, "Camera Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
			text_renderer->Printf(10.f, 40.f, 0.01f, 0xDBB600FF, 1.0f, "FPS: %.2f", window->GetTimer()->GetFPS());
			text_renderer->RestorePipeline();*/
			window->Present();
		}

		fly_cam->GetChanges().Reset();

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
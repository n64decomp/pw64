#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <imgui.h>
#include "controller_host.h"
#include "libultraship/libultra/gbi.h"
#include <algorithm>

#include "ship/Context.h"
#include "ship/resource/ResourceManager.h"
#include "ship/config/Config.h"
#include "ship/config/ConsoleVariable.h"
#include "fast/Fast3dWindow.h"
#include "libultraship/controller/controldeck/ControlDeck.h"

extern "C" {
#include <uv_vector.h>
#include <uv_matrix.h>
	}

static void PackHostMatrix(
    uint32_t (&packed)[16],
    const Mtx4F& matrix) {
	for (int row = 0; row < 4; ++row) {
		for (int pair = 0; pair < 2; ++pair) {
			const int col = pair * 2;

			const uint32_t a = static_cast<uint32_t>(
			                       static_cast<int32_t>(
			                           matrix.m[row][col] * 65536.0f + 0.5f));

			const uint32_t b = static_cast<uint32_t>(
			                       static_cast<int32_t>(
			                           matrix.m[row][col + 1] * 65536.0f + 0.5f));

			const int index = row * 2 + pair;

			packed[index] =
			    (a & 0xFFFF0000u) | (b >> 16);

			packed[index + 8] =
			    (a << 16) | (b & 0x0000FFFFu);
			}
		}
	}

static Gfx* BuildGraphicsTest(int x, int y, float angle) {
	static Gfx commands[64] {};
	static uint16_t colorBuffer[320 * 240] {};

	// Packed N64 identity matrix:
	alignas(8) static uint32_t projectionMatrix[16] {};
	alignas(8) static uint32_t modelMatrix[16] {};

	Mtx4F projection{};
	Mtx4F model{};

// Camera at the origin, looking along negative Z.
// Near plane: 1; far plane: 10.
// Horizontal extent accounts for the 320:240 aspect ratio.
	constexpr float aspect = 320.0f / 240.0f;

	uvMat4SetFrustrum(
	    &projection,
	    -aspect, aspect,
	    -1.0f, 1.0f,
	    1.0f, 10.0f);

// Position the triangle in front of the camera.
	uvMat4SetIdentity(&model);
	uvMat4LocalTranslate(&model, 0.0f, 0.0f, -3.0f);

// Rotate around two axes so its 3D orientation is visible.
	uvMat4RotateAxis(&model, angle, 'y');
	uvMat4RotateAxis(&model, angle * 0.6f, 'x');

	uvMat4Scale(&model, 0.75f, 0.75f, 0.75f);

	PackHostMatrix(projectionMatrix, projection);
	PackHostMatrix(modelMatrix, model);

	static Vp viewport{};
	static Vtx vertices[3] {};

	// A 120x80 viewport centered in the 320x240 logical screen.
	// Viewport X/Y values have two fractional bits.
	viewport.vp.vscale[0] = 160 * 4;
	viewport.vp.vscale[1] = 120 * 4;
	viewport.vp.vscale[2] = G_MAXZ / 2;
	viewport.vp.vscale[3] = 0;

	viewport.vp.vtrans[0] = 160 * 4;
	viewport.vp.vtrans[1] = 120 * 4;
	viewport.vp.vtrans[2] = G_MAXZ / 2;
	viewport.vp.vtrans[3] = 0;

	// Identity matrices mean these are already clip-space positions.
	vertices[0].v.ob[0] = -1;
	vertices[0].v.ob[1] = -1;
	vertices[0].v.ob[2] = 0;

	vertices[1].v.ob[0] = 1;
	vertices[1].v.ob[1] = -1;
	vertices[1].v.ob[2] = 0;

	vertices[2].v.ob[0] = 0;
	vertices[2].v.ob[1] = 1;
	vertices[2].v.ob[2] = 0;

	// Red, green, and blue vertices, all fully opaque.
	vertices[0].v.cn[0] = 255;
	vertices[0].v.cn[1] = 0;
	vertices[0].v.cn[2] = 0;
	vertices[0].v.cn[3] = 255;

	vertices[1].v.cn[0] = 0;
	vertices[1].v.cn[1] = 255;
	vertices[1].v.cn[2] = 0;
	vertices[1].v.cn[3] = 255;

	vertices[2].v.cn[0] = 0;
	vertices[2].v.cn[1] = 0;
	vertices[2].v.cn[2] = 255;
	vertices[2].v.cn[3] = 255;

	Gfx* p = commands;

	gDPPipeSync(p++);
	gDPSetColorImage(
	    p++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, colorBuffer);

	gDPSetScissor(p++, G_SC_NON_INTERLACE, 0, 0, 320, 240);
	gDPSetCycleType(p++, G_CYC_FILL);
	gDPSetRenderMode(p++, G_RM_NOOP, G_RM_NOOP2);

	// Clear the entire background.
	const uint32_t background =
	    GPACK_RGBA5551(16, 24, 48, 1);

	gDPSetFillColor(
	    p++, (background << 16) | background);
	gDPFillRectangle(p++, 0, 0, 319, 239);

	gDPPipeSync(p++);

	// Preserve the controllable orange rectangle.
	const uint32_t rectangle =
	    GPACK_RGBA5551(255, 160, 32, 1);

	gDPSetFillColor(
	    p++, (rectangle << 16) | rectangle);
	gDPFillRectangle(p++, x, y, x + 63, y + 47);

	gDPPipeSync(p++);

	// Switch from rectangle fill mode to triangle rendering.
	gDPSetCycleType(p++, G_CYC_1CYCLE);
	gDPSetRenderMode(p++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
	gDPSetAlphaCompare(p++, G_AC_NONE);
	gDPSetCombineMode(p++, G_CC_SHADE, G_CC_SHADE);

	// No lighting, fog, depth testing, texture generation, or culling.
	gSPClearGeometryMode(p++, 0xFFFFFFFFu);
	gSPSetGeometryMode(p++, G_SHADE | G_SHADING_SMOOTH);
	gSPTexture(p++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);

	gSPViewport(p++, &viewport);

	gSPMatrix(
	    p++, projectionMatrix,
	    G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

	gSPMatrix(
	    p++, modelMatrix,
	    G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

	const Gfx vertexLoad = {
		gsSPVertex(vertices, 3, 0)
		};
	*p++ = vertexLoad;
	gSP1Triangle(p++, 0, 1, 2, 0);

	gDPPipeSync(p++);
	gDPFullSync(p++);
	gSPEndDisplayList(p++);

	return commands;
	}

int main() {
	try {
		const auto assetPath =
		    std::filesystem::absolute("host-assets");

		std::filesystem::create_directories(assetPath);
		std::ofstream(assetPath / "test.txt") << "Pilotwings host test\n";

		auto context =
		    Ship::Context::CreateInstance("Pilotwings 64", "pw64");

		context->Init();

		const nlohmann::json specification = {
				{
				"components", {
						{
							{"type", "Config"},
							{"initArgs", {{"path", "pw64-host.json"}}}
						},
						{
							{"type", "ConsoleVariable"}
						},
						{
							{"type", "ThreadPool"},
							{"initArgs", {{"threadCount", 1}}}
						},
						{
							{"type", "ResourceManager"},
							{
							"initArgs", {
									{"archivePaths", {assetPath.string()}}
								}
							}
						}
					}
				}
			};

		if (!Ship::Context::BuildComponentsFromJson(
		            context, specification)) {
			std::cerr << "Component initialization failed.\n";
			return 1;
			}

		auto resources = context->GetChildren()
		                 .GetFirst<Ship::ResourceManager>();

		if (!resources || !resources->IsInitialized()) {
			std::cerr << "Resource manager is not initialized.\n";
			return 1;
			}

		auto file = resources->LoadFileProcess("test.txt");

		if (!file) {
			std::cerr << "Could not load test.txt.\n";
			return 1;
			}

		std::cout << "LUS resource manager loaded test.txt successfully.\n";

		auto config =
		    context->GetChildren().GetFirst<Ship::Config>();

		auto consoleVariables =
		    context->GetChildren().GetFirst<Ship::ConsoleVariable>();

// Construct the window first; it can find the deck through Context later.
		auto window = std::make_shared<Fast::Fast3dWindow>(
		                  config, consoleVariables);

		auto controlDeck = std::make_shared<LUS::ControlDeck>(
		                       window, consoleVariables);

		const nlohmann::json presentationComponents = {
				{
				"components", {
						{{"type", "Console"}},
						{{"type", "Events"}},
						{{"type", "ControlDeck"}},
						{{"type", "Window"}},
						{{"type", "FileDrop"}}
					}
				}
			};

		const std::unordered_map<
		std::string, std::shared_ptr<Ship::Component>> overrides = {
				{"ControlDeck", controlDeck},
				{"Window", window}
			};

		if (!Ship::Context::BuildComponentsFromJson(
		            context,
		            presentationComponents,
		            nlohmann::json::object(),
		            overrides)) {
			std::cerr << "Window component initialization failed.\n";
			return 1;
			}

// The controller-specific Init needs an initialized window.
		uint8_t controllerBits = 0;
		controlDeck->Init(&controllerBits);

		uvControllerInit();

		OSContPad hostPads[MAXCONTROLLERS] {};
		unsigned int pressFrames = 0;
		unsigned int releaseFrames = 0;

		std::cout << "Window initialized using "
		          << window->GetWindowBackendName() << '\n';

		window->SetRendererUCode(ucode_f3dex2);

		float triangleAngle = 0.0f;

		Vec3F rectanglePosition{};
		rectanglePosition.x = 128.0f;
		rectanglePosition.y = 96.0f;
		auto previousUpdate = std::chrono::steady_clock::now();

		while (window->IsRunning()) {
			window->HandleEvents();

			if (!window->IsRunning()) {
				break;
				}

			if (!window->IsFrameReady()) {
				std::this_thread::sleep_for(
				    std::chrono::milliseconds(1));
				continue;
				}

			for (auto& pad : hostPads) {
				pad = {};
				}

			controlDeck->WriteToPad(hostPads);

			PW64_SetHostPad(
			    hostPads[0].button,
			    hostPads[0].stick_x,
			    hostPads[0].stick_y);

			if (!uvIOUpdate()) {
				std::cerr << "Pilotwings input update failed.\n";
				break;
				}

			const auto now = std::chrono::steady_clock::now();

			float dt =
			    std::chrono::duration<float>(
			        now - previousUpdate).count();

			previousUpdate = now;

			// Clamp before using dt for either animation or movement.
			dt = std::clamp(dt, 0.0f, 0.05f);

			constexpr float twoPi = 6.2831853f;
			constexpr float angularSpeed = 1.0f;

			triangleAngle += angularSpeed * dt;

			if (triangleAngle >= twoPi) {
				triangleAngle -= twoPi;
				}

			constexpr float speed = 100.0f;

			Vec3F input{};
			input.x = uvControllerGetStick(0, 0);
			input.y = -uvControllerGetStick(0, 1);

			if (uvVec3Len(&input) > 1.0f) {
				Vec3F normalized{};
				uvVec3Normal(&normalized, &input);
				uvVec3Copy(&input, &normalized);
				}

			Vec3F displacement{};
			uvVec3Mul(&displacement, &input, speed * dt);

			Vec3F nextPosition{};
			uvVec3Add(
			    &nextPosition,
			    &rectanglePosition,
			    &displacement);

			nextPosition.x =
			    std::clamp(nextPosition.x, 0.0f, 256.0f);
			nextPosition.y =
			    std::clamp(nextPosition.y, 0.0f, 192.0f);

			uvVec3Copy(&rectanglePosition, &nextPosition);

			Gfx* graphicsTest = BuildGraphicsTest(
			                        static_cast<int>(rectanglePosition.x),
			                        static_cast<int>(rectanglePosition.y),
			                        triangleAngle);

			window->DrawAndRunGraphicsCommands(
			    graphicsTest, {}, {});
			} // End while.

		return 0;
		} // End try.
	catch (const std::exception& error) {
		std::cerr << "Initialization error: "
		          << error.what() << '\n';
		return 1;
		} // End catch.
	} // End main.
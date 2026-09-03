#pragma once
#include <chrono>
#include "oxyMeter.h"
#include "Settings.h"

oxygenMenu::oxygenMenu()
{
	auto scaleformManager = RE::BSScaleformManager::GetSingleton();

	inputContext = Context::kNone;
	depthPriority = 0;

	menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);
	menuFlags.set(RE::UI_MENU_FLAGS::kRequiresUpdate);
	menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);

	if (uiMovie) {
		uiMovie->SetMouseCursorCount(0);  // disable input
	}

	scaleformManager->LoadMovieEx(this, MENU_PATH, [](RE::GFxMovieDef* a_def) -> void {
		a_def->SetState(RE::GFxState::StateType::kLog,
			RE::make_gptr<Logger>().get());
	});
}

RE::GPtr<oxygenMenu> oxygenMenu::GetOxygenMenu()
{
	auto ui = RE::UI::GetSingleton();
	return ui ? ui->GetMenu<oxygenMenu>(oxygenMenu::MENU_NAME) : nullptr;
}

struct detail
{
	static float get_total_breath_time()
	{
		const auto gamesetting = RE::GameSettingCollection::GetSingleton();
		return (50.0f * gamesetting->GetSetting("fActorSwimBreathMult")->GetFloat()) + gamesetting->GetSetting("fActorSwimBreathBase")->GetFloat();
	}

	static float get_remaining_breath(RE::AIProcess* a_process)
	{
		auto high = a_process ? a_process->high : nullptr;
		return high ? high->breathTimer : 20.0f;
	}

	static std::optional<double> get_player_breath_pct()
	{
		const auto player = RE::PlayerCharacter::GetSingleton();

		if (!player->IsPointSubmergedMoreThan(player->GetPosition(), player->GetParentCell(), 0.875f) || player->IsInvulnerable() || player->GetActorValue(RE::ActorValue::kWaterBreathing) > 0.0001f) {
			return std::nullopt;
		}

		float totalBreathTime = detail::get_total_breath_time();
		if (totalBreathTime <= 0.0f) {
			return std::nullopt;
		}
		float remainingBreath = detail::get_remaining_breath(player->currentProcess);

		return (remainingBreath / totalBreathTime) * 100.0;
	}
};

void oxygenMenu::Register()
{
	auto ui = RE::UI::GetSingleton();
	if (ui) {
		ui->Register(MENU_NAME, Creator);
		REX::INFO("Menu Registered.");
	}
}

void oxygenMenu::Show()
{
	auto msgQ = RE::UIMessageQueue::GetSingleton();
	if (msgQ) {
		msgQ->AddMessage(oxygenMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}
}

void oxygenMenu::Hide()
{
	auto msgQ = RE::UIMessageQueue::GetSingleton();
	if (msgQ) {
		msgQ->AddMessage(oxygenMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	}
}

// pass the breath meter percents to the scaleform menu using invokes, and tell the menu when to show or hide itself (as in within the scaleform, not the IMenu kHide flag)
void oxygenMenu::Update()
{
	RE::GPtr<RE::IMenu> oxygenMeter = RE::UI::GetSingleton()->GetMenu(oxygenMenu::MENU_NAME);
	if (!oxygenMeter || !oxygenMeter->uiMovie)
		return;

	static bool fadeWhenDrowning{ Settings::GetSingleton()->fadeWhenDrowning };
	static float flashWhenBelow{ Settings::GetSingleton()->flashWhenBelow };
	auto fillPct = detail::get_player_breath_pct();

	if (fillPct) {

		RE::GFxValue MenuEnabled;
		oxygenMeter->uiMovie->GetVariable(&MenuEnabled, "main.MenuEnabled");

		if (!MenuEnabled.GetBool()) {
			ApplyLayout(oxygenMeter);
			ApplyColour(oxygenMeter);
			oxygenMeter->uiMovie->SetVariable("main.MenuEnabled", true);
		}

		SetMenuVisibilityMode(MenuVisibilityMode::kVisible);
		
		const RE::GFxValue currentBreathAmount = *fillPct;

		if (!holding_breath) {
			holding_breath = true;
		}

		if (drowning && fadeWhenDrowning) {
			oxygenMeter->uiMovie->Invoke("main.doFadeOut", nullptr, &currentBreathAmount, 1);
			return;
		}
		
		if (*fillPct <= flashWhenBelow) {
			oxygenMeter->uiMovie->Invoke("main.doFlash", nullptr, nullptr, 0);
		}

		oxygenMeter->uiMovie->Invoke("main.doShow", nullptr, nullptr, 0);
		oxygenMeter->uiMovie->Invoke("main.setMeterPercent", nullptr, &currentBreathAmount, 1);

		if (*fillPct == 0.0) {
			drowning = true;
		}

	} else {
		static std::chrono::time_point<std::chrono::steady_clock> start;
		static bool fading{ false };

		if (holding_breath || drowning) {
			holding_breath = false;
			drowning = false;
			const RE::GFxValue refill = 100;
			oxygenMeter->uiMovie->Invoke("main.updateMeterPercent", nullptr, &refill, 1);
			oxygenMeter->uiMovie->Invoke("main.doFadeOut", nullptr, nullptr, 0);
			start = std::chrono::steady_clock::now();
			fading = true;
		}

		if (fading && ((std::chrono::steady_clock::now() - start) > 1s || !want_visible))
		{
			SetMenuVisibilityMode(MenuVisibilityMode::kHidden);
			fading = false;
		}
	}

}

// apply location, rotations and scale settings to menu.
void oxygenMenu::ApplyLayout(RE::GPtr<RE::IMenu> oxygenMeter)
{
	if (!oxygenMeter || !oxygenMeter->uiMovie)
		return;

	auto def = oxygenMeter->uiMovie->GetMovieDef();

	const RE::GFxValue widget_xpos = (Settings::GetSingleton()->widget_xpos / 100.0) * def->GetWidth();
	const RE::GFxValue widget_ypos = (Settings::GetSingleton()->widget_ypos / 100.0) * def->GetHeight();
	const RE::GFxValue widget_rotation = Settings::GetSingleton()->widget_rotation;
	const RE::GFxValue widget_xscale = Settings::GetSingleton()->widget_xscale;
	const RE::GFxValue widget_yscale = Settings::GetSingleton()->widget_yscale;
	RE::GFxValue posArray[5]{ widget_xpos, widget_ypos, widget_rotation, widget_xscale, widget_yscale };
	oxygenMeter->uiMovie->Invoke("main.setLocation", nullptr, posArray, 5);
}

void oxygenMenu::ApplyColour(RE::GPtr<RE::IMenu> oxygenMeter)
{
	if (!oxygenMeter || !oxygenMeter->uiMovie)
		return;

	const RE::GFxValue bar_colour = Settings::GetSingleton()->widget_colour;
	const RE::GFxValue flash_colour = Settings::GetSingleton()->widget_flashcolour;
	RE::GFxValue colourArray[2]{ bar_colour, flash_colour };
	oxygenMeter->uiMovie->Invoke("main.setBarAndFlashColor", nullptr, colourArray, 2);
}

// Every time a new frame of the menu is rendered call the update function.
void oxygenMenu::AdvanceMovie(float a_interval, std::uint32_t a_currentTime)
{
	oxygenMenu::Update();
	RE::IMenu::AdvanceMovie(a_interval, a_currentTime);
}

RE::UI_MESSAGE_RESULTS oxygenMenu::ProcessMessage(RE::UIMessage& a_message)
{
	using Type = RE::UI_MESSAGE_TYPE;

	switch (*a_message.type) {
	case Type::kShow:
		OnOpen();
		return RE::IMenu::ProcessMessage(a_message);
	case Type::kHide:
		OnClose();
		return RE::IMenu::ProcessMessage(a_message);
	default:
		return RE::IMenu::ProcessMessage(a_message);
	}
}

bool oxygenMenu::IsOpen() const
{
	return _bIsOpen;
}

void oxygenMenu::OnOpen() 
{
	// The advance movie process will handle showing the meter when appropriate
	oxygenMenu::SetMenuVisibilityMode(oxygenMenu::MenuVisibilityMode::kHidden);
	_bIsOpen = true;
}

void oxygenMenu::OnClose() 
{
	want_visible = false;
	_bIsOpen = false;
}

void oxygenMenu::SetMenuVisibilityMode(MenuVisibilityMode a_mode)
{
	auto menu = GetOxygenMenu();
	if (menu) {
		auto _view = menu->uiMovie;

		if (_view) {
			switch (a_mode) {
			case MenuVisibilityMode::kHidden:
				if (menu->_menuVisibilityMode == MenuVisibilityMode::kVisible) {
					_view->SetVisible(false);
					menu->_menuVisibilityMode = a_mode;
				}
				break;

			case MenuVisibilityMode::kVisible:
				if (menu->_menuVisibilityMode == MenuVisibilityMode::kHidden && want_visible) {
					_view->SetVisible(true);
					menu->_menuVisibilityMode = a_mode;
				}
				break;
			}
		}
	}
}


#pragma once
#include "Events.h"
#include "oxyMeter.h"
#include "Settings.h"
#include "RE/U/UI.h"
#include "string.h"

MenuOpenCloseEventHandler* MenuOpenCloseEventHandler::GetSingleton()
{
	static MenuOpenCloseEventHandler singleton;
	return std::addressof(singleton);
}

void MenuOpenCloseEventHandler::Register()
{
	auto ui = RE::UI::GetSingleton();
	ui->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());
	REX::INFO("Registered {}"sv, typeid(RE::MenuOpenCloseEvent).name());
}

RE::BSEventNotifyControl MenuOpenCloseEventHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
{

	if (a_event)
		REX::DEBUG("Received RE::MenuOpenCloseEvent for {} with opening {}"sv, a_event->menuName.c_str(), a_event->opening);

	// On HUD menu open/close - open/close the plugin's HUD menu
	if (a_event) {
		if (a_event->menuName == RE::HUDMenu::MENU_NAME || a_event->menuName == "TrueHUD"sv) {
			if (a_event->opening) {
				oxygenMenu::Show();
			} else {
				oxygenMenu::Hide();
			}
		}
		if (a_event->menuName == RE::LoadingMenu::MENU_NAME && !a_event->opening) {
			oxygenMenu::Show();
		}
	}


	// Hide the widgets when a menu is open
	auto controlMap = RE::ControlMap::GetSingleton();
	if (controlMap) {
		auto& priorityStack = controlMap->contextPriorityStack;
		if (priorityStack.empty()) {
			oxygenMenu::want_visible = false;
		} else if (priorityStack.back() == RE::UserEvents::INPUT_CONTEXT_ID::kGameplay ||
				   priorityStack.back() == RE::UserEvents::INPUT_CONTEXT_ID::kFavorites ||
				   priorityStack.back() == RE::UserEvents::INPUT_CONTEXT_ID::kConsole) 
		{
			oxygenMenu::want_visible = true;
		} else {
			oxygenMenu::want_visible = false;
		}
		REX::DEBUG("OxygenMenu::want_Visible = {}", oxygenMenu::want_visible);
	}

	return RE::BSEventNotifyControl::kContinue;
}

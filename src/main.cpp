#include "Settings.h"
#include "SKSE/API.h"
#include "oxyMeter.h"
#include "Events.h"

const SKSE::MessagingInterface* g_messaging = nullptr;
const SKSE::LoadInterface* g_LoadInterface = nullptr;
const SKSE::QueryInterface* g_QueryInterface = nullptr;

// I've made this file very messy, I should really go through and clean it up.

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		MenuOpenCloseEventHandler::Register();
		logger::info("Registering oxygen meter menu.");
		oxygenMenu::Register();
		break;
	}
}

#	ifdef SKYRIM_SUPPORT_AE
extern "C" DLLEXPORT constinit auto SKSE_PLUGIN_VERSION = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(REL::Version{ Version::MAJOR});
	v.PluginName("Oxygen Meter 2");
	v.AuthorName("powerofthree and OsmosisWrench");
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

	if constexpr (SKSE::RUNTIME_LATEST < Runtime::MIN_ADDRESS_LIBRARY_V5) {
		v.MinimumRequiredXSEVersion(REL::Version{ 2, 2, 5 });
	} else {
		v.MinimumRequiredXSEVersion(REL::Version{ 2, 3, 0 });
	}

	return v;
}();
#	else
SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Oxygen Meter 2";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		REX::CRITICAL("Loaded in editor, marking as incompatible");
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_1_5_39) {
		REX::CRITICAL("Unsupported runtime version {}", ver.string());
		return false;
	}

	return true;
}
#	endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	SKSE::Init(a_skse);

	auto runtimeVersion = a_skse->RuntimeVersion();
	REX::INFO("Game version : {}", runtimeVersion);

	#ifdef SKYRIM_SUPPORT_AE
	if constexpr (SKSE::RUNTIME_LATEST < Runtime::MIN_ADDRESS_LIBRARY_V5) {
		if (runtimeVersion >= Runtime::MIN_ADDRESS_LIBRARY_V5) {
			REX::FAIL(
				"You are using a newer version of Skyrim than this version of {0} supports.\n"
				"Install the correct version of {0} for your game version.\n"
				"Runtime: {1}\n"
				"Supported: 1.6.1170 (Steam) / 1.6.1179 (GOG)",
				Version::PROJECT, runtimeVersion);
		}
	}
#endif

	Settings::GetSingleton()->Load();
	g_messaging = reinterpret_cast<SKSE::MessagingInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kMessaging));
	if (!g_messaging) {
		logger::critical("Failed to load messaging interface! This error is fatal, plugin will not load.");
		return false;
	}

	auto papyrus = reinterpret_cast<SKSE::PapyrusInterface*>(a_skse->QueryInterface(SKSE::LoadInterface::kPapyrus));
	if (!papyrus) {
		logger::critical("Failed to load scripting interface! This error is fatal, plugin will not load.");
		return false;
	}

	g_messaging->RegisterListener("SKSE", SKSEMessageHandler);

	return true;
}

#include "PCH.h"
#include "RE/G/GFxValue.h"
#include "RE/G/GPtr.h"

class oxygenMenu : public RE::IMenu
{
public:
    static constexpr const char* MENU_PATH = "oxygenMeter2";
	static constexpr const char* MENU_NAME = "oxygenMeter2";

    oxygenMenu();

    static void Register();
	static void Show();
	static void Hide();
	static void Update();
	static void ApplyLayout(RE::GPtr<RE::IMenu> oxygenMeter);
	static void ApplyColour(RE::GPtr<RE::IMenu> oxygenMeter);

	static RE::GPtr<oxygenMenu> GetOxygenMenu();
    static RE::IMenu* Creator() { return new oxygenMenu(); }

	void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override;
	RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;
	
	enum class MenuVisibilityMode : uint8_t
	{
		kHidden,
		kVisible
	};
	MenuVisibilityMode _menuVisibilityMode = MenuVisibilityMode::kVisible;
	static inline bool want_visible{ false };

	static void SetMenuVisibilityMode(MenuVisibilityMode a_mode);
	bool IsOpen() const;
	void OnOpen();
	void OnClose();

private:
	static inline bool holding_breath{ false };
	static inline bool drowning{ false };
	bool _bIsOpen = false;

private:
	class Logger : public RE::GFxLog
	{
	public:
		void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
		{
			std::string fmt(a_fmt ? a_fmt : "");
			while (!fmt.empty() && fmt.back() == '\n') {
				fmt.pop_back();
			}

			std::va_list args;
			va_copy(args, a_argList);
			std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
			std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
			va_end(args);

			REX::INFO("{}"sv, buf.data());
		}
	};
};

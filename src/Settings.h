#pragma once
#include <algorithm>

class Settings
{
public:
	[[nodiscard]] static Settings* GetSingleton()
	{
		static Settings singleton;
		return std::addressof(singleton);
	}

	static void get_hex_value(CSimpleIniA& a_ini, std::uint32_t& a_value, const char* a_section, const char* a_key, const char* a_comment)
	{
		a_value = static_cast<std::uint32_t>(
			a_ini.GetLongValue(a_section, a_key, static_cast<long>(a_value)));
		a_ini.SetLongValue(a_section, a_key, static_cast<long>(a_value), a_comment, true);
	}

	void Load()
	{
		constexpr auto path = L"Data/SKSE/Plugins/OxygenMeter2.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		ini::get_value(ini, fadeWhenDrowning, "Settings", "Fade out when drowning", ";Oxygen meter fades out when you run out of oxygen and your health starts decreasing");
		ini::get_value(ini, flashWhenBelow, "Settings", "Flash meter when air level below", ";Oxygen meter starts to flash when under this percentage of total air, default is 50");
		get_hex_value(ini, widget_colour, "Colour", "Widget Bar Colour",";The colour of the widget bar, default is 0x51CCCC");
		get_hex_value(ini, widget_flashcolour, "Colour", "Widget Flash Colour",";The colour of the flash effect around the widget, default is 0x51CCCC");
		ini::get_value(ini, widget_xpos, "Position", "Widget X Position", ";The X Position for the widget as a percentage, default is 50.0");
		ini::get_value(ini, widget_ypos, "Position", "Widget Y Position", ";The Y Position for the widget as a percentage, default is 96.3");
		ini::get_value(ini, widget_rotation, "Position", "Widget Rotation", ";The rotation for the widget, default is 0.000000");
		ini::get_value(ini, widget_xscale, "Position", "Widget X scale", ";The X scale of the widget, default is 75.000000");
		ini::get_value(ini, widget_yscale, "Position", "Widget Y scale", ";The Y scale of the widget, default is 75.000000");

		if (widget_xpos > 100.0f || widget_xpos < 0.0f)
			widget_xpos = 50.0f;

		if (widget_ypos > 100.0f || widget_ypos < 0.0f)
			widget_ypos = 96.3f;

		ini.SaveFile(path);
	}

	bool fadeWhenDrowning{ false };
	float flashWhenBelow{ 50.0f };
	std::uint32_t widget_colour{ 0x51CCCC };
	std::uint32_t widget_flashcolour{ 0x51CCCC };
	float widget_xpos{ 50.0f };
	float widget_ypos{ 96.3f };
	float widget_rotation{ 0.0f };
	float widget_xscale{ 75.0f };
	float widget_yscale{ 75.0f };

private:
};

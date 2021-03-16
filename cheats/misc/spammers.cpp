// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "spammers.h"

void spammers::clan_tag()
{
	auto apply = [](const char* tag) -> void
	{
		using Fn = int(__fastcall*)(const char*, const char*);
		static auto fn = reinterpret_cast<Fn>(util::FindSignature(crypt_str("engine.dll"), crypt_str("53 56 57 8B DA 8B F9 FF 15")));

		fn(tag, tag);
	};

	static auto removed = false;

	if (!g_cfg.misc.clantag_spammer && !removed)
	{
		removed = true;
		apply(crypt_str(""));
		return;
	}

	if (g_cfg.misc.clantag_spammer)
	{
		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return;

		static auto time = -1;

		auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)m_globals()->m_tickcount; //-V807
		auto intervals = 0.5f / m_globals()->m_intervalpertick;
		// clantag speed
		auto main_time = (int)(ticks / intervals) % 22;

		if (main_time != time && !m_clientstate()->m_choked_commands)
		{
			auto tag = crypt_str("");

			switch (main_time)
			{
			case 0:
				tag = crypt_str("PSense "); //-V1037
				break;
			case 1:
				tag = crypt_str("PSens ");
				break;
			case 2:
				tag = crypt_str("PSen ");
				break;
			case 3:
				tag = crypt_str("PSe ");
				break;
			case 4:
				tag = crypt_str("PS ");
				break;
			case 5:
				tag = crypt_str("P ");
				break;
			case 6:
				tag = crypt_str("P");
				break;
			case 7:
				tag = crypt_str(" ");
				break;
			case 8:
				tag = crypt_str("");
				break;
			case 9:
				tag = crypt_str("P");
				break;
			case 10:
				tag = crypt_str("P ");
				break;
			case 11:
				tag = crypt_str("PS ");
				break;
			case 12:
				tag = crypt_str("PSe ");
				break;
			case 13:
				tag = crypt_str("PSen ");
				break;
			case 14:
				tag = crypt_str("PSens ");
				break;
			case 15:
				tag = crypt_str("PSense ");
				break;
			case 16:
				tag = crypt_str("superior paste PSense");
				break;
			case 17:
				tag = crypt_str("superior paste PSense");
				break;
			case 18:
				tag = crypt_str("superior paste PSense");
				break;
			case 19:
				tag = crypt_str("superior paste PSense");
				break;
			case 20:
				tag = crypt_str("superior paste PSense");
				break;
			case 21:
				tag = crypt_str("superior paste PSense ");
				break;
			}

			apply(tag);
			time = main_time;
		}

		removed = false;
	}
}
#include <mutex>
#include <optional>
#include <iostream>
#include <tuple>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <unordered_set>
#include <thread>
#include <string>
#include <vector>
#include <random>
#include <memory>
#include <functional>


#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "clue.hpp"

int main()
{
	SERVER s;
	s.games.resize(1);
	auto& g = s.games[0];

	g.players.resize(6);
	g.DefaultCards();
	g.Deal();

	int iRound = 0;
	g.PrintGuilty();
	for (;;)
	{
		for (size_t ip = 0; ip < g.players.size(); ip++)
		{
			if (ip == 0)
			{
				std::cout << "Round " << iRound + 1 << "\r\n-----------------------------------------\r\n";
				iRound++;
			}

			auto& pl = g.players[ip];
			auto sugg = pl.Suggest();

			std::cout << "============================================================================\r\n";
			std::cout << "Player " << ip + 1 << " suggests ";
			for (auto& su : sugg)
			{
				std::wcout << g.cards[su.cat][su.idx] << L" ";
			}
			std::cout << std::endl;

			for (size_t ip2 = ip + 1 ; ip2 != ip ; )
			{
				if (ip2 >= g.players.size())
					ip2 = 0;
				if (ip2 == ip)
					break;

				auto& pln = g.players[ip2];
				auto resp = pln.Respond(ip,sugg);
				if (resp.has_value())
				{
					std::cout << "\tPlayer " << ip2 + 1 << " responds ";
					std::wcout << g.cards[resp.value().cat][resp.value().idx] << L"\r\n";

					pl.SendResponse(sugg, ip2, resp.value());

					for (size_t ipX = 0; ipX < g.players.size(); ipX++)
					{
						if (ipX != ip2 && ipX != ip)
							g.players[ipX].NotifyResponseForSuggestion(sugg, ip, ip2);
					}
					break;
				}

				// No response, next player
				std::cout << "\tPlayer " << ip2 + 1 << " skips\r\n";
				for (size_t ipX = 0; ipX < g.players.size(); ipX++)
				{
					if (ipX != ip2)
						g.players[ipX].NotifyNoResponseForSuggestion(sugg,ip,ip2);
				}
				ip2++;
				if (ip2 == ip)
					break;
			}

			pl.PrintSheet();
			auto guilty = pl.AskGuilty();
			if (guilty.has_value())
			{
				std::cout << "Round " << iRound << "\r\n";
				std::cout << "Pl " << ip + 1 << "   -> ";
				int icat = 0;
				for (auto& su : guilty.value())
				{
					std::wcout << g.cards[icat][su] << L" ";
					icat++;
				}
				std::cout << std::endl;
				g.PrintGuilty();
				std::cin.get();
				return 0;
			}
		}
	}

}


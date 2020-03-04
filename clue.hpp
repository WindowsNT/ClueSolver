// 



inline std::random_device rd; // obtain a random number from hardware
inline std::mt19937 random_engine(rd()); // seed the generator


struct PLAYERCARD
{
	size_t cat = 0;
	size_t idx = 0;
	PLAYERCARD(size_t c = 0,size_t i = 0)
	{
		cat = c;
		idx = i;
	}
};

inline bool operator < (const PLAYERCARD& pl1, const PLAYERCARD& pl2)
{
	if (pl1.cat < pl2.cat)
		return true;
	if (pl1.cat > pl2.cat)
		return false;
	if (pl1.idx < pl2.idx)
		return true;
	return 0;
}

class PLAYER
{
	std::vector<PLAYERCARD> mycards;
	std::map<PLAYERCARD, size_t> mycardsshown;
	std::map<size_t, std::vector<std::wstring>> TypeDeal;
	size_t me = 0;
	size_t num_players = 0;

public:

	std::map<PLAYERCARD, size_t> Positives;
	std::map<PLAYERCARD, std::unordered_set<size_t>> Negatives;
	std::vector<std::tuple<size_t, std::vector<PLAYERCARD>>> Possibilities;



	void PrintSheet()
	{
		std::cout << "\t\t";
		for (size_t i = 0; i < num_players; i++)
		{
			std::cout << (i + 1) << " ";
		}
		std::cout << "\r\n\t\t----------------------------------\r\n";

		// Cards
		for (size_t icat = 0; icat < TypeDeal.size(); icat++)
		{
			for (size_t idx = 0; idx < TypeDeal[icat].size(); idx++)
			{
				auto& cardname = TypeDeal[icat][idx];
				std::wcout << cardname;
				size_t j = cardname.length() / 4;
				if (j <= 1)
					std::wcout << L"\t\t";
				else
					std::wcout << L"\t";


				bool Someone = false;
				for (auto& p : Positives)
				{
					if (p.first.cat == icat && p.first.idx == idx)
					{
						Someone = true;
					}
				}

				for (size_t i = 0; i < num_players; i++)
				{
					bool F = false;
					for (auto& p : Positives)
					{
						if (p.first.cat == icat && p.first.idx == idx)
						{
							if (p.second == i)
							{
								std::cout << "X ";
								F = true;
							}
						}
					}
					if (!F)
					{
						if (Someone || i == me)
							std::cout << "- ";
						else
						{
							bool NN = false;
							for (auto& n : Negatives)
							{
								if (n.first.cat == icat && n.first.idx == idx)
								{
									auto& s = n.second;
									for (auto& ss : s)
									{
										if (ss == i)
											NN = true;
									}


								}
							}
							if (NN)
								std::cout << "O ";
							else
								std::cout << "  ";
						}
					}

				}
				std::wcout << L"" << std::endl;

			}
			std::cout << "\t\t----------------------------------\r\n";
		}
	}


	void Inform(size_t num, size_t you, std::map<size_t, std::vector<std::wstring>> cards, std::vector<PLAYERCARD> yours)
	{
		num_players = num;
		me = you;
		TypeDeal = cards;
		mycards = yours;

		// Create the positives
		for (auto& c : mycards)
			Positives[c] = me;
	}


	void NotifyNoResponseForSuggestion(std::vector<PLAYERCARD>& pcs, [[maybe_unused]] size_t reqpl, size_t resppl)
	{
		for (auto& e : pcs)
			Negatives[e].insert(resppl);
	}

	void SendResponse([[maybe_unused]] std::vector<PLAYERCARD>& req, size_t ipl, PLAYERCARD& pc)
	{
		Positives[pc] = ipl;
	}

	void NotifyResponseForSuggestion(std::vector<PLAYERCARD>& pcs, [[maybe_unused]] size_t reqpl, size_t resppl)
	{
		std::tuple<size_t, std::vector<PLAYERCARD>> tu = std::make_tuple<size_t, std::vector<PLAYERCARD>>(std::forward<size_t>(resppl), std::forward<std::vector<PLAYERCARD>>(pcs));
		Possibilities.push_back(tu);
	}




	std::vector<PLAYERCARD> ResolveNeg(size_t icat)
	{
		std::vector<PLAYERCARD> pp;
		for (auto& n : Negatives)
		{
			auto& c = n.first;
			auto& s = n.second;

			if (icat != (size_t)-1 && icat != c.cat)
				continue;

			if (s.size() == num_players)
				pp.push_back(c);
		}
		return pp;
	}

	void ResolvePb()
	{
		for (int j = (int)Possibilities.size() - 1; j >= 0; j--)
		{
			auto ipl = std::get<0>(Possibilities[j]);
			if (ipl == me)
				continue; // duh

			auto& v = std::get<1>(Possibilities[j]);

			for (int jj = (int)v.size() - 1; jj >= 0; jj--)
			{
				auto& ThisCard = v[jj];

				auto tpl = Positives.find(ThisCard);
				if (tpl != Positives.end())
				{
					if (tpl->second != ipl)
						v.erase(v.begin() + jj);
					else
					{
						// it has, remove all
						Possibilities.erase(Possibilities.begin() + j);
						break;
					}
				}
			}
		}

		// Single ones
		for (int j = (int)Possibilities.size() - 1; j >= 0; j--)
		{
			auto ipl = std::get<0>(Possibilities[j]);
			if (ipl == me)
				continue; // duh

			auto& v = std::get<1>(Possibilities[j]);
			if (v.size() == 1)
			{
				Positives[v[0]] = ipl;
				Possibilities.erase(Possibilities.begin() + j);
			}
		}
	}

	void NegativesOnMaxCards()
	{
		size_t NumAllCards = 0;
		for (size_t i = 0; i < TypeDeal.size(); i++)
		{
			NumAllCards += TypeDeal[i].size();
		}

		NumAllCards -= TypeDeal.size(); // Guilty Cards

		std::map<size_t, size_t> Ply;
		for (size_t i = 0; i < num_players; i++)
			Ply[i] = 0;
		for (;;)
		{
			for (size_t i = 0; i < num_players; i++)
			{
				Ply[i]++;
				NumAllCards--;
				if (NumAllCards == 0)
					break;
			}
			if (NumAllCards == 0)
				break;
		}

		std::map<size_t, size_t> PlyKnown;
		for (size_t i = 0; i < num_players; i++)
			PlyKnown[i] = 0;

		for (auto& p : Positives)
			PlyKnown[p.second]++;

		for (size_t i = 0; i < num_players; i++)
		{
			if (PlyKnown[i] == Ply[i])
			{
				// Negatives for all the rest of the cards for this player
				for (size_t icat = 0; icat < TypeDeal.size(); icat++)
				{
					for (size_t idx = 0; idx < TypeDeal[icat].size(); idx++)
					{
						PLAYERCARD tc(icat,idx);
						auto pf = Positives.find(tc);
						if (pf != Positives.end() && pf->second == i)
							continue;

						// This player cannot have this card
						Negatives[tc].insert(i);
					}
				}
			}
		}
	}

	std::optional<std::vector<size_t>> AskGuilty()
	{
		NegativesOnMaxCards();
		ResolvePb();
		std::vector<size_t> x;
		std::vector<float> perc;

		for (size_t i = 0; i < TypeDeal.size(); i++)
		{
			// Category i
			size_t NumUnknown = TypeDeal[i].size();

			std::vector<PLAYERCARD> rn = ResolveNeg(i);
			if (!rn.empty())
			{
				// We have the guilty part
				x.push_back(rn[0].idx);
				perc.push_back(1.0f);
				continue;
			}

			std::map<size_t, size_t> Unkns;
			for (size_t tt = 0; tt < NumUnknown; tt++)
				Unkns[tt] = 1;

			// Remove Pos
			for (auto& c : Positives)
			{
				if (c.first.cat == i)
					Unkns.erase(c.first.idx);
			}

			if (Unkns.size() == 1)
				x.push_back(Unkns.begin()->first);
			perc.push_back(1.0f / (float)Unkns.size());
		}

		float MyPerc = 1.0f;
		for (auto& p : perc)
			MyPerc *= p;
		std::cout << "My probability of solving: " << MyPerc << " ( ";
		for (auto& p : perc)
		{
			std::cout << p;
			std::cout << " ";
		}

		std::cout << ")" << std::endl;
		if (x.size() == TypeDeal.size())
			return x;
		return {};
	}

	std::vector<PLAYERCARD> Suggest()
	{
		std::vector<PLAYERCARD> o;
		size_t cats = TypeDeal.size();

		// Random, atm
		for (size_t i = 0; i < cats; i++)
		{
			struct CARDANDNEG
			{
				PLAYERCARD p;
				size_t nneg = 0;
				bool operator <(const CARDANDNEG& can)
				{
					if (nneg < can.nneg)
						return true;
					return false;
				}
			};
			std::vector<CARDANDNEG> picks;

			for (size_t idx = 0; idx < TypeDeal[i].size() ; idx++)
			{

				PLAYERCARD t(i,idx);
				if (Positives.find(t) == Positives.end())
				{
					CARDANDNEG can;
					can.p = t;
					picks.push_back(can);
				}
			}

			if (picks.empty())
			{
				PLAYERCARD t(i,0);
				CARDANDNEG can;
				can.p = t;
				picks.push_back(can); // duh
			}

			if (picks.size() == 1)
			{
				PLAYERCARD pc = picks[0].p;
				o.push_back(pc);
				continue;
			}

			// Choose the item that has the most negatives
			for (size_t ii = 0; ii < picks.size() ; ii++)
			{
				auto& p = picks[ii];
				if (Negatives.find(p.p) == Negatives.end())
					continue;
				p.nneg = Negatives[p.nneg].size();
			}

			// Sort
			std::sort(picks.begin(), picks.end());
			auto lastneg = picks[picks.size() - 1].nneg;
			while(picks.size() > 1)
			{
				if (picks[0].nneg == lastneg)
					break;
				picks.erase(picks.begin());
			}

			std::uniform_int_distribution<> distr(0, (int)(picks.size() - 1));
			int rt = distr(random_engine);
			PLAYERCARD pc = picks[rt].p;
			o.push_back(pc);
		}
		return o;
	}


	std::optional<PLAYERCARD> Respond([[maybe_unused]] size_t pl, std::vector<PLAYERCARD>& pcs)
	{
		size_t CanRespond = 0;
		for (auto& req : pcs)
		{
			for (auto& my : mycards)
			{
				if (req.cat == my.cat && req.idx == my.idx)
					CanRespond++;
			}
		}
		if (CanRespond == 0)
			return {};

		if (CanRespond == 1)
		{
			// Forced
			for (auto& req : pcs)
			{
				for (auto& my : mycards)
				{
					if (req.cat == my.cat && req.idx == my.idx)
					{
						mycardsshown[my]++;
						return my;
					}
				}
			}
		}

		PLAYERCARD Should;
		size_t sm = (size_t)-1;
		for (auto& req : pcs)
		{
			for (auto& my : mycards)
			{
				if (req.cat == my.cat && req.idx == my.idx)
				{
					size_t count = mycardsshown[my];
					if (count < sm || sm == (size_t)-1)
					{
						sm = count;
						Should = req;
					}
				}
			}
		}

		mycardsshown[Should]++;
		return Should;
	}




};

class GAME
{
public:

	std::map<size_t, std::vector<std::wstring>> cards;
	std::vector<PLAYER> players;
	std::vector<size_t> guilty;


	void DefaultCards()
	{
		cards.clear();
		cards[0].resize(6);
		cards[0][0] = L"Mustard";
		cards[0][1] = L"Orchid";
		cards[0][2] = L"Scarlett";
		cards[0][3] = L"Peacock";
		cards[0][4] = L"Plum";
		cards[0][5] = L"Green";

		cards[1].resize(6);
		cards[1][0] = L"Candlestick";
		cards[1][1] = L"Dagger";
		cards[1][2] = L"Lead Pipe";
		cards[1][3] = L"Revolver";
		cards[1][4] = L"Rope";
		cards[1][5] = L"Wrench";

		cards[2].resize(9);
		cards[2][0] = L"Ballroom";
		cards[2][1] = L"Billiard Room";
		cards[2][2] = L"Conservatory";
		cards[2][3] = L"Dining Room";
		cards[2][4] = L"Hall";
		cards[2][5] = L"Kitchen";
		cards[2][6] = L"Library";
		cards[2][7] = L"Lounge";
		cards[2][8] = L"Study";
		/*
		cards[3].resize(4);
		cards[3][0] = L"Morning";
		cards[3][1] = L"Noon";
		cards[3][2] = L"Afternoon";
		cards[3][3] = L"Night";

		cards[4].resize(6);
		cards[4][0] = L"00:00";
		cards[4][1] = L"00:10";
		cards[4][2] = L"00:20";
		cards[4][3] = L"00:30";
		cards[4][4] = L"00:40";
		cards[4][5] = L"00:50";
		*/
	}

	void PrintGuilty()
	{
		std::cout << "Guilty -> ";
		for (size_t i = 0; i < cards.size(); i++)
		{
			std::wcout << cards[i][guilty[i]] << L" ";
		}
		std::cout << "\r\n--------------------------\r\n";
	}

	int Deal()
	{
		if (players.empty())
			return -1;

		// How many
		guilty.clear();
		std::vector<PLAYERCARD> pcs;
		for (auto& c : cards)
		{
			PLAYERCARD pc;
			pc.cat = c.first;
			std::uniform_int_distribution<> distr(0, (int)(c.second.size() - 1));
			int gc = distr(random_engine);
			for (int i = 0; i < c.second.size(); i++)
			{
				if (gc == i)
				{
					guilty.push_back(i);
				}
				else
				{
					pc.idx = i;
					pcs.push_back(pc);
				}
			}
		}

		PrintGuilty();

		size_t AvCards = pcs.size();
		size_t iP = 0;
		std::map<size_t, std::vector<PLAYERCARD>> ply;
		while (AvCards > 0)
		{
			std::uniform_int_distribution<> distr(0, (int)(AvCards - 1)); // define the range
			int c = distr(random_engine);
			AvCards--;


			ply[iP].push_back(pcs[c]);
			pcs.erase(pcs.begin() + c);
			iP++;
			if (iP >= players.size())
				iP = 0;
		}

		for (size_t ip = 0; ip < players.size(); ip++)
			players[ip].Inform(players.size(), iP, cards, ply[ip]);

		return 1;
	}

};


class SERVER
{
public:

	std::vector<GAME> games;


};

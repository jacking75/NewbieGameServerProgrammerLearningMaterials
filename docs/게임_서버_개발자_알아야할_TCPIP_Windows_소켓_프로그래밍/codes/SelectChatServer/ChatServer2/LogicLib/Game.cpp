#include <chrono>

#include "Game.h"


namespace NLogicLib
{
	/*Game::Game() {}

	Game::~Game() {}*/

	void Game::Clear()
	{
		m_State = GameState::NONE;
	}

	bool Game::CheckSelectTime()
	{
		auto curTime = std::chrono::system_clock::now();
		auto curSecTime = std::chrono::system_clock::to_time_t(curTime);

		auto diff = curSecTime - m_SelectTime;
		if (diff >= 60)
		{
			//
			return true;
		}

		return false;
	}
}
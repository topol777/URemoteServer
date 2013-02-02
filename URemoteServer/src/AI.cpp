#include "AI.h"

#include <thread>
#include "modules\Speech.h"
#include "Translator.h"
#include "TextKey.h"
#include "Utils.h"

#define DELAY 60*5 // 5 min / 300 sec before repeate time

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////////
AI::AI(unique_ptr<AIConfig> config) : m_Config(move(config))
{
	m_Voice = unique_ptr<Speech>(new Speech(m_Config->Lang, m_Config->Gender));
	time(&m_LastWelcome);
	m_LastWelcome -= DELAY;

	Start();
}

AI::~AI()
{
	Shutdown();
}

bool AI::StartConnection(unique_ptr<ServerConfig> serverConfig)
{
	bool result = false;

	try {
		//m_ExchangeServer = unique_ptr<Server>(new Server(move(serverConfig), this));
	
		auto text = Translator::getInstance()->getString(TextKey::AI_SERVER_ONLINE, m_Config->Name);
		Say(text);

		Server* s = new Server(move(serverConfig), this);
		thread startListener(&Server::Start, s);
		startListener.join();

		//result = m_ExchangeServer->Start();
	} catch (const exception&) {
		Utils::getLogger()->error("AI::StartConnection(), " + (result) ? "OK" : "KO");
	}

	Utils::getLogger()->debug("AI::StartConnection(), " + (result) ? "OK" : "KO");
	return result;
}

bool AI::StopConnection()
{
	if (m_ExchangeServer) {
		m_ExchangeServer->Stop();
		return true;
	}

	return false;
}

void AI::Welcome()
{
	// Calculate the elapsed time since the last call to the method
	time_t now;
	time(&now);
	Utils::getLogger()->debug("AI::Welcome() now " + to_string(now));
	auto elapsedTime = difftime(now, m_LastWelcome);
	Utils::getLogger()->debug("AI::Welcome() elapsedTime " + to_string(elapsedTime));

	// Welcome if last welcome > DELAY
	if (elapsedTime > DELAY) {
		auto text = Translator::getInstance()->getString(TextKey::AI_WELCOME, m_Config->Name);
		Say(text);
		time(&m_LastWelcome);
	}
}

void AI::Say(string textToSpeak)
{
	// Test mute state
	if (!m_Config->IsMute) {
		m_Voice->SayInThread(textToSpeak);
	}
}

//! Change l'�tat de mute et renvoie le nouvel �tat
bool AI::ToggleMute()
{
	m_Config->IsMute = !m_Config->IsMute;
	return m_Config->IsMute;
}

//////////////////////////////////////////////////////////////////////////////
// Fonctions priv�es
//////////////////////////////////////////////////////////////////////////////

void AI::Start()
{
	auto text = Translator::getInstance()->getString(TextKey::AI_INITIATED);
	Say(text);
}

void AI::Shutdown()
{
	auto text = Translator::getInstance()->getString(TextKey::AI_SHUTDOWN, m_Config->Name);
	Say(text);
}
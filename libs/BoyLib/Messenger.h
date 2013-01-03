#pragma once

#include "CrtDbgInc.h"

#include <map>
#include <string>
#include <vector>

namespace BoyLib
{
	class MessageListener;
	class MessageSource;

	class Messenger
	{
	public:

		static Messenger *instance();

		static void init();
		static void destroy();

		void addListener(MessageListener *listener);
		void removeListener(MessageListener *listener);
		void sendMessage(std::string messageId, MessageSource *source=NULL, std::map<std::string,std::string> *params=NULL);

	private:

		Messenger();
		virtual ~Messenger();

	private:

		static Messenger *gInstance;

		std::vector<MessageListener*> mGlobalListeners;
	};
}

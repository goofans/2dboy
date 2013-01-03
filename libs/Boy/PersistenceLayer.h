#pragma once

#include "BoyLib/CrtDbgInc.h"

#include <map>
#include <string>

namespace Boy
{
	class PersistenceLayer
	{
	public:

		PersistenceLayer() {}
		virtual ~PersistenceLayer() {}

		virtual void putString(const std::string &name, const std::string &value, bool persist=false) = 0;
		virtual const std::string getString(const std::string &name) = 0;
		virtual bool remove(const std::string &name, bool persist=false) = 0;

		virtual int getKeyCount() = 0;
		virtual const std::string getKey(int i) = 0;

		virtual void persist() = 0;

	};
}
#pragma once

#include "BoyLib/CrtDbgInc.h"

#include "PersistenceLayer.h"
#include <map>
#include <string>
#include "BoyLib/UString.h"

namespace Boy
{
	class WinPersistenceLayer : public PersistenceLayer
	{
	public:

		WinPersistenceLayer(const Boy::UString &filename, unsigned char *key);
		virtual ~WinPersistenceLayer();

		virtual void putString(const std::string &name, const std::string &value, bool persist=false);
		virtual const std::string getString(const std::string &name);
		virtual bool remove(const std::string &name, bool persist=false);

		virtual int getKeyCount();
		virtual const std::string getKey(int i);

		virtual void persist();

	private:

		void load();
		void save();
		void parse(char *data, int size);
		int readInt(char *data, int *pos); // pos will be modified!

	private:

		unsigned char *mKey;
		std::map<std::string,std::string> mValues;
		UString mFileName;

	};
}

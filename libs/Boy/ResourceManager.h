#pragma once

#include "BoyLib/Rect.h"
#include "BoyLib/UString.h"
#include "BoyLib/Vector2.h"
#include <string>
#include "tinyxml/tinyxml.h"
#include <vector>

namespace Boy
{

	class Font;
	class Image;
	class Resource;
	class ResourceGroup;
	class ResourceLoader;
	class Sound;

	class ResourceManager
	{
	public:
		
		ResourceManager(ResourceLoader *loader, unsigned char *key, const std::string &language1, const std::string &language2);
		virtual ~ResourceManager();

		// resource file parsing:
		virtual bool parseResourceFile(const std::string &fileName, unsigned char *key=NULL);

		// resource loading/unloading:
		virtual bool loadResourceGroup(const std::string &groupName);
		virtual void unloadResourceGroup(const std::string &groupName);
		virtual void reloadResources();
		virtual void destroyResources(bool includeSounds);
		virtual void initResources(bool includeSounds);

		// id based resource access:
		virtual Image *getImage(const std::string &id);
		virtual bool hasString(const std::string &id);
		virtual Boy::UString getString(const std::string &id);
		virtual Sound *getSound(const std::string &id);
		virtual Font *getFont(const std::string &id);

		// direct resource access:
		virtual Image *getImageFromPath(const std::string &path);

		// misc:
		void getAllSounds(std::vector<Sound*> &sounds);
		std::string &getLanguage1() { return mLanguage1; }

		// debug:
		void dump();

	private:

		void parseResourceGroup(TiXmlElement *elem);
		void loadStrings(const char *filename, const char *filenamebin, unsigned char *key);
		void addResource(
			const std::string &id, 
			const std::string &path, 
			ResourceGroup *group,
			Resource *resource);
		void mapResource(const std::string &id, 
			const std::string &path, 
			ResourceGroup *group);
		Resource *createResource(const char *type, const std::string &path);

	private:

		// resource loader:
		ResourceLoader *mResourceLoader;

		// resource files:
		std::vector<std::string> mParsedResourceFiles;

		// resource groups:
		std::map<std::string,ResourceGroup*> mResourceGroups;

		// resources:
		std::map<std::string,Resource*> mResourcesByPath;
		std::map<std::string,Resource*> mResourcesById;

		// string resources:
		std::map<std::string,Boy::UString> mText;

		// language:
		std::string mLanguage1;
		std::string mLanguage2;
	};
}


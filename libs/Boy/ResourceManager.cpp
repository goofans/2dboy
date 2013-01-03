#include "ResourceManager.h"

#include <algorithm>
#include "Boy/Crypto.h"
#include "BoyLib/BoyUtil.h"
#include "Environment.h"
#include "Font.h"
#include <fstream>
#include "Image.h"
#include "Resource.h"
#include "ResourceGroup.h"
#include "ResourceLoader.h"
#include "Sound.h"
#include <string>

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

ResourceManager::ResourceManager(ResourceLoader *loader, unsigned char *key, const std::string &language1, const std::string &language2)
{
	mLanguage1 = language1;
	mLanguage2 = language2;

	// load string resources (currently in separate file):
	loadStrings("properties/text.xml","properties/text.xml.bin",key);
#if !defined(GOO_PLATFORM_WII)
	loadStrings("properties/profanity.xml","properties/profanity.xml.bin",key);
#endif
	if (loader->getLanguage1().size()>0)
	{
		std::string path = "properties/text.";
		path.append(loader->getLanguage1());
		path.append(".xml");
		std::string pathb = path;
		pathb.append(".bin");
		loadStrings(path.c_str(),pathb.c_str(),key);
	}

	assert(loader!=NULL);
	mResourceLoader = loader;
}

ResourceManager::~ResourceManager()
{
	// unload all resource groups:
	std::map<std::string,ResourceGroup*>::iterator groupIter;
	for (groupIter=mResourceGroups.begin() ; groupIter!=mResourceGroups.end() ; groupIter++)
	{
		unloadResourceGroup(groupIter->first);
		delete groupIter->second;
	}
	mResourceGroups.clear();

	// delete all resources:
	std::map<std::string,Resource*>::iterator resIter;
	for (resIter=mResourcesByPath.begin() ; resIter!=mResourcesByPath.end() ; resIter++)
	{
		assert(!resIter->second->isLoaded());

		// if the resource is loaded (this will happen 
		// when we call getImageFromPath directly):
		while (resIter->second->isLoaded())
		{
			// unload it:
			resIter->second->release();
		}

		delete resIter->second;
	}
	mResourcesByPath.clear();

	// clear the rest of the data
	mParsedResourceFiles.clear();
	mResourcesById.clear();
	mText.clear();

}

void ResourceManager::loadStrings(const char *filename, const char *filenamebin, unsigned char *key)
{
	TiXmlDocument doc;

	if( key == NULL )
	{
		// load the clear text strings directly
		if (doc.LoadFile(filename)==false)
		{
			return;
		}
	}
	else
	{
		// load and decrypt the strings when we have an encryption key
		char *data;
		int dataSize;
		if (Boy::loadDecrypt(key, filenamebin, &data, &dataSize)==false)
		{
			return;
		}

		doc.Parse( data );
		delete[] data;
	}

	TiXmlElement *root = doc.RootElement();

#ifdef _DEBUG
	int totalCount = 0;
	int missingCount = 0;
#endif
	for (TiXmlElement *e = root->FirstChildElement() ; e!=NULL ; e = e->NextSiblingElement())
	{
		assert(strcmp(e->Value(),"string")==0);
		const char *id = e->Attribute("id");
		const char *text = e->Attribute("text");
		assert(id!=NULL && text!=NULL);

#ifdef _DEBUG
		// only do this for actual strings, skip the mom state machine stuff:S
		if (std::string(text).find("MOM_")!=0 && strlen(text)!=0)
		{
			totalCount += 4;
			if (e->Attribute("es")==NULL)
			{
				printf("%s missing SPANISH\n",id);
				missingCount++;
			}
			if (e->Attribute("fr")==NULL)
			{
				printf("%s missing FRENCH\n",id);
				missingCount++;
			}
			if (e->Attribute("it")==NULL)
			{
				printf("%s missing ITALIAN\n",id);
				missingCount++;
			}
			if (e->Attribute("de")==NULL)
			{
				printf("%s missing GERMAN\n",id);
				missingCount++;
			}
		}
#endif

		// look for localized text:
		bool foundLocalized = false;
		for (TiXmlAttribute *a = e->FirstAttribute() ; a!=NULL ; a=a->Next())
		{
			// skip the 'id' and 'text' attributes:
			const char *name = a->Name();
			if (Boy::Environment::instance()->stricmp("id",name)==0 || Boy::Environment::instance()->stricmp("text",name)==0)
			{
				continue;
			}

			// if this attribute is the localized version for the first language:
			if (mLanguage1==a->Name())
			{
				// remebmer the localized text:
				text = a->Value();
				break;
			}
			else if (mLanguage2==a->Name())
			{
				// remebmer the localized text:
				text = a->Value();
				// (but keep searching for the first language)
			}
		}

		// store the text:
		mText[id] = text;
	}

#ifdef _DEBUG
	float progress = (float)(totalCount - missingCount) / totalCount;
	printf("LOCALIZATION IS %0.0f%% COMPLETE\n",progress*100);
#endif

	doc.Clear();
}

bool ResourceManager::parseResourceFile(const std::string &fileName, unsigned char *key)
{
	// if we've already loaded the fonts:
	if (find(mParsedResourceFiles.begin(),mParsedResourceFiles.end(),fileName) != mParsedResourceFiles.end())
	{
		// ignore this call:
		return true;
	}

	TiXmlDocument doc;

	if (key==NULL)
	{
		bool success = doc.LoadFile(fileName.c_str());
		if (!success)
		{
			return false;
		}
	}
	else
	{
		// adjust the filename to point to the encrypted version:
		std::string encFileName = fileName;
		if (key!=NULL)
		{
			encFileName.append(".bin");
		}

		// load and decrypt the resource file
		char *data;
		int dataSize;
		bool success = Boy::loadDecrypt(key, encFileName.c_str(), &data, &dataSize);
		if (!success)
		{
			return false;
		}

		// parse it:
		doc.Parse(data);
		mParsedResourceFiles.push_back(fileName);

		// deallocate the mem:
		delete[] data;
		data = NULL;
	}
	
	TiXmlElement *root = doc.RootElement();

	for (TiXmlElement *e = root->FirstChildElement() ; e!=NULL ; e = e->NextSiblingElement())
	{
		if (Boy::Environment::instance()->stricmp(e->Value(),"resources")==0)
		{
			parseResourceGroup(e);
		}
		else
		{
			assert(false);
		}
	}

	doc.Clear();

	return true;
}

void ResourceManager::parseResourceGroup(TiXmlElement *elem)
{
	std::string groupId = elem->Attribute("id");
	ResourceGroup *group = new ResourceGroup();
	mResourceGroups[groupId] = group;

	std::string basePath;
	std::string idPrefix;

	for (TiXmlElement *child = elem->FirstChildElement() ; child!=NULL ; child = child->NextSiblingElement())
	{
		const char *val = child->Value();
		if (Boy::Environment::instance()->stricmp(val,"SetDefaults")==0)
		{
			idPrefix = child->Attribute("idprefix");
			basePath = child->Attribute("path");
			// add trailing slash if necessary:
			if (basePath[basePath.size()-1]!='/' && 
				basePath[basePath.size()-1]!='\\')
			{
				basePath += '/';
			}
			// remove "./" from begining if it's there:
			if (basePath.substr(0,2)=="./" ||
				basePath.substr(0,2)==".\\")
			{
				basePath = basePath.substr(2);
			}
		}
		else
		{
			std::string id = idPrefix + child->Attribute("id");
			std::string fullPath = basePath;
			if (mLanguage1.size()>0 && child->Attribute(mLanguage1.c_str())!=NULL)
			{
				fullPath.append(child->Attribute(mLanguage1.c_str()));
			}
			else if (mLanguage2.size()>0 && child->Attribute(mLanguage2.c_str())!=NULL)
			{
				fullPath.append(child->Attribute(mLanguage2.c_str()));
			}
			else {
				fullPath.append(child->Attribute("path"));
			}

			// if this resource path already exists:
			if (mResourcesByPath.find(fullPath)!=mResourcesByPath.end())
			{
				// just create a mapping:
				mapResource(id, fullPath, group);
			}
			else
			{
				// create and add the resource:
				Boy::Resource *res = createResource(val,fullPath);
				addResource(id, fullPath, group, res);
			}
		}
	}
}

Resource *ResourceManager::createResource(const char *type, const std::string &path)
{
	Resource *res;
	if (Boy::Environment::instance()->stricmp(type,"image")==0)
	{
		res = mResourceLoader->createImage(path);
	}
	else if (Boy::Environment::instance()->stricmp(type,"font")==0)
	{
		res = new Font(mResourceLoader, path);
	}
	else if (Boy::Environment::instance()->stricmp(type,"sound")==0)
	{
		res = mResourceLoader->createSound(path);
	}
	else
	{
		assert(false);
	}
	return res;
}

void ResourceManager::addResource(const std::string &id, 
								  const std::string &path, 
								  ResourceGroup *group, 
								  Resource *resource)
{
	assert(resource!=NULL);

	// add it to the group:
	group->addResourcePath(path);

	// create a path->resource mapping:
	mResourcesByPath[path] = resource;
//	printf("[%p] %s\n",resource,path.c_str());

	// create an id->resource mapping:
	mResourcesById[id] = resource; 
}

void ResourceManager::mapResource(const std::string &id, 
								  const std::string &path, 
								  ResourceGroup *group)
{
	group->addResourcePath(path);

	// get the resource pointed to by this resource path:
	Resource *res = mResourcesByPath[path];

	// map new id to existing resource:
	mResourcesById[id] = res;
}

bool ResourceManager::loadResourceGroup(const std::string &groupName)
{
	// note: it's ok to load a group more than once as long as it's
	// released more than once too.  different parts of the code
	// can use the same resource group and each of them should load
	// it when they need it and unload it when they done with it
	if (mResourceGroups.find(groupName) != mResourceGroups.end())
	{
		// disable full screen toggle while resource load/unload:
		Environment::instance()->disableFullScreenToggle();

		ResourceGroup *g = mResourceGroups[groupName];
		bool success = true;
		if (!g->isEmpty())
		{
			for (const std::string *path = g->getFirstPath() ; path!=NULL ; path = g->getNextPath())
			{
				assert(mResourcesByPath.find(*path) != mResourcesByPath.end());
				Resource *res = mResourcesByPath[*path];
				assert(res!=NULL);
				success &= res->load();
			}
		}

		Environment::instance()->enableFullScreenToggle();

		return success;
	}
	else
	{
		assert(false);

		return false;
	}
}

void ResourceManager::unloadResourceGroup(const std::string &groupName)
{
	if (mResourceGroups.find(groupName) != mResourceGroups.end())
	{
		// disable full screen toggle while resource load/unload:
		Environment::instance()->disableFullScreenToggle();

		ResourceGroup *g = mResourceGroups[groupName];
		for (const std::string *path=g->getFirstPath() ; path!=NULL ; path=g->getNextPath())
		{
			Resource *res = mResourcesByPath[*path];
			assert(res!=NULL);
			res->release();
		}

		Environment::instance()->enableFullScreenToggle();
	}
	else
	{
		assert(false);
	}
}

void ResourceManager::reloadResources()
{
	// disable full screen toggle while resource load/unload:
	Environment::instance()->disableFullScreenToggle();

	// iterate over all reasources:
	for (std::map<std::string,Resource*>::iterator iter = mResourcesByPath.begin() ;
		iter != mResourcesByPath.end() ; 
		iter ++)
	{
		iter->second->reload();
	}

	Environment::instance()->enableFullScreenToggle();
}

void ResourceManager::destroyResources(bool includeSounds)
{
	// disable full screen toggle while resource load/unload:
	Environment::instance()->disableFullScreenToggle();

	// iterate over all reasources:
	for (std::map<std::string,Resource*>::iterator iter = mResourcesByPath.begin() ;
		iter != mResourcesByPath.end() ; 
		iter ++)
	{
		iter->second->destroy(includeSounds);
	}

	Environment::instance()->enableFullScreenToggle();
}

void ResourceManager::initResources(bool includeSounds)
{
	// disable full screen toggle while resource load/unload:
	Environment::instance()->disableFullScreenToggle();

	// iterate over all reasources:
	for (std::map<std::string,Resource*>::iterator iter = mResourcesByPath.begin() ;
		iter != mResourcesByPath.end() ; 
		iter ++)
	{
		iter->second->init(includeSounds);
	}

	Environment::instance()->enableFullScreenToggle();
}

Font *ResourceManager::getFont(const std::string &id)
{
	assert(mResourcesById.find(id)!=mResourcesById.end());
	Font *font = dynamic_cast<Font*>(mResourcesById[id]);
	assert(font->isLoaded());
	return font;
}

Image *ResourceManager::getImage(const std::string &id)
{
	std::map<std::string,Resource*>::iterator iter = mResourcesById.find(id);
	if (iter==mResourcesById.end())
	{
		envDebugLog("[WARNING: ResourceManager::getImage] image with id %s not found\n",id.c_str());
		return NULL;
	}
	Image *image = dynamic_cast<Image*>(iter->second);
	assert(image->isLoaded());
	return image;
}

Boy::UString ResourceManager::getString(const std::string &id)
{
	assert(mText.find(id)!=mText.end());
	return mText[id];
}

bool ResourceManager::hasString(const std::string &id)
{
	return mText.find(id) != mText.end();
}

Sound *ResourceManager::getSound(const std::string &id)
{
	std::map<std::string,Resource*>::iterator iter = mResourcesById.find(id);
	if (iter==mResourcesById.end())
	{
		return NULL;
	}

	Sound *sound = dynamic_cast<Sound*>(iter->second);
	assert(sound->isLoaded());
	return sound;
}

Image *ResourceManager::getImageFromPath(const std::string &path)
{
	std::map<std::string,Resource*>::iterator iter = mResourcesByPath.find(path);

	// if we have a reference to this image path:
	if (iter != mResourcesByPath.end())
	{
		// increment its ref count:
		Image *img = dynamic_cast<Image*>(iter->second);

		// increment ref count:
		img->load();

		// return the image:
		return img;
	}
	else
	{
		// create it:
		Image *img = mResourceLoader->createImage(path);

		// try to load it:
		if (img->load())
		{
			// keep track of it:
			mResourcesByPath[path] = img;

			// return it:
			return img;
		}
		else
		{
			// get rid of it:
			delete img;

			// return NULL;
			return NULL;
		}
	}
}

void ResourceManager::getAllSounds(std::vector<Sound*> &sounds)
{
	// TODO: this implementation is kinda hacky, we 
	// shouldn't be relying on the resource ID to 
	// identify whether it as a sound.

	std::map<std::string,Resource*>::iterator iter = mResourcesById.begin();
	std::map<std::string,Resource*>::iterator end = mResourcesById.end();

	while (iter!=end)
	{
		if (iter->first.substr(0,5)=="SOUND")
		{
			sounds.push_back(dynamic_cast<Sound*>(iter->second));
		}
		iter++;
	}
}

void ResourceManager::dump()
{
	std::map<std::string,int>::iterator refIter;

	printf("===============================================================\n");
	printf("------------------- ResourceManager: files --------------------\n");
	std::vector<std::string>::iterator iter1;
	for (iter1=mParsedResourceFiles.begin() ; iter1!=mParsedResourceFiles.end() ; iter1++)
	{
		printf("file: %s\n",iter1->c_str());
	}

	printf("------------------- ResourceManager: groups --------------------\n");
	std::map<std::string,ResourceGroup*>::iterator iter2;
	for (iter2=mResourceGroups.begin() ; iter2!=mResourceGroups.end() ; iter2++)
	{
		printf("group(%p): %s\n",iter2->second,iter2->first.c_str());
	}
	printf("------------------- ResourceManager: resources -----------------\n");
	std::map<std::string,Resource*>::iterator iter3;
	std::map<std::string,Resource*>::iterator iter4;
	for (iter3=mResourcesById.begin() ; iter3!=mResourcesById.end() ; iter3++)
	{
		std::map<std::string,Resource*>::iterator iter4;
		std::string path;
		for (iter4=mResourcesById.begin() ; iter4!=mResourcesById.end() ; iter4++)
		{
			if (iter4->second==iter3->second)
			{
				path = iter4->first;
				break;
			}
		}

		printf("resource(%p): %s=%s\n",iter3->second,iter3->first.c_str(),path.c_str());
	}
	printf("===============================================================\n");
}
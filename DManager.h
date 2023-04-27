#pragma once

#include <map>

class DResource;
class DManager
{
public:
	std::map<std::string, DResource *> m_mapResource;

public:
	DManager();
	virtual ~DManager();

	DResource *Create(std::string name);
	DResource *Find(std::string name);
	void Delete(std::string name);
};


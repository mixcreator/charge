#include "inireader.h"

#include <stdio.h>
#include <string.h>

//using namespace YAMC;

void trim_blanks(std::string& str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(isblank))).base(), str.end());
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(isblank))));
}

ini_file::ini_file()
{
}

ini_file::ini_file(const std::string& filename) : std::string(filename)
{
    load();
}

tINIFileSection* ini_file::get_section(const std::string& sect)
{
    tINIConfigIterator it = m_sections.find(sect);
    if(it != m_sections.end())
	return &(it->second);
    return 0;
}

tINIConfig* ini_file::sections()
{
    return &m_sections;
}

std::string ini_file::value(const std::string& sect, const std::string& key, const std::string& defvalue)
{
    tINIConfigIterator it = m_sections.find(sect);
    if(it != m_sections.end())
    {
	tINIFileSectionIterator it1 = it->second.find(key);
	if(it1 != it->second.end())
	    return it1->second;
    }
    return defvalue;
}

int ini_file::valueInt(const std::string& sect, const std::string& key, const int defvalue){
    std::string valstr = value(sect, key, "");
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    return end > value ? n : defvalue;
    
}
    
bool ini_file::valueBool(const std::string& sect, const std::string& key, const bool defvalue){
    std::string valstr = value(sect, key, "");
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
    if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        return true;
    else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        return false;
    else
        return defvalue;
}


bool ini_file::load()
{
    m_sections.clear();
    if(empty())
	return false;
    FILE *f = ::fopen(c_str(),"r");
    if(!f)
	return false;

    std::string sect;
    for (;;)
    {
	char buf[1024];
	if (!::fgets(buf,sizeof(buf),f))
	    break;

	char *pc = ::strchr(buf,'\r');
	if (pc)
	    *pc = 0;
	pc = ::strchr(buf,'\n');
	if (pc)
	    *pc = 0;
	pc = buf;
	while (*pc == ' ' || *pc == '\t')
	    pc++;
	switch (*pc)
	{
	    case 0:
	    case ';':
		continue;
	}
	std::string s(pc);
	if (s[0] == '[')
	{
	    size_t r = s.find(']');
	    if (r != std::string::npos)
	    {
		sect = s.substr(1,r-1);
	    }
	    continue;
	}
	size_t q = s.find('=');
	if ((q != std::string::npos) && (q > 0))
	{
	    std::string key = s.substr(0,q);
	    trim_blanks(key);
	    std::string val = s.substr(q+1);
	    trim_blanks(val);
	    m_sections[sect][key] = val;
	}
    }
    ::fclose(f);
    return true;
}


/* vi: set ts=8 sw=4 sts=4 noet: */

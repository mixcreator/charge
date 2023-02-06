#ifndef __INIREADER_H
#define __INIREADER_H    20190607

#ifndef __cplusplus
#error C++ is required
#endif

#include <string>
#include <map>
#include <algorithm>
//#include <mcclass.h>

/**
 * Holds all Telephony Engine related classes.
 */

//#include "mcformats.h"

extern std::string RecoveryCallPath;
extern std::string RecoveryCallPathArchiv;
extern std::string RecoveryScriptPath;
extern bool RecoveryCallEnabled;

typedef std::map<std::string,std::string> tINIFileSection;
typedef std::map<std::string,tINIFileSection> tINIConfig;

typedef std::map<std::string,std::string>::iterator tINIFileSectionIterator;
typedef std::map<std::string,tINIFileSection>::iterator tINIConfigIterator;



class ini_file : public std::string
{
public:
    /**
     * Create an empty INIFile
     */
    ini_file();

    /**
     * Create a INIFile from a file
     * @param filename Name of file to initialize from
     * @param warn True to warn if the INIFile could not be loaded
     */
    ini_file(const std::string& filename);

    /**
     * Assignment from string operator
     */
    inline ini_file& operator=(const std::string& value)
	{ std::string::operator=(value); return *this; }

    /**
     * Retrive an entire section
     * @param index Index of the section
     * @return The section's content or NULL if no such section
     */
    tINIConfig* sections();

    /**
     * Retrive an entire section
     * @param sect Name of the section
     * @return The section's content or NULL if no such section
     */
    tINIFileSection* get_section(const std::string& sect);


    static std::string sect_get_val(tINIFileSection& sect, const std::string& val, const std::string& def = "")
    {
	tINIFileSectionIterator it = sect.find(val);
	if(it != sect.end())
	{
	    return it->second;
	}
	return def;
    }


    /**
     * Retrive the value of a key in a section.
     * @param sect Name of the section
     * @param key Name of the key in section
     * @param defvalue Default value to return if not found
     * @return The string contained in the key or the default
     */
    std::string value(const std::string& sect, const std::string& key, const std::string& defvalue = "");

    int valueInt(const std::string& sect, const std::string& key, const int defvalue);
    
    bool valueBool(const std::string& sect, const std::string& key, const bool defvalue);

    /**
     * Load the INIFile from file
     * @param warn True to also warn if the INIFile could not be loaded
     * @return True if successfull, false for failure
     */
    bool load();

private:
    ini_file(const ini_file& value); // no copy constructor
    ini_file& operator=(const ini_file& value); // no assignment please
    tINIConfig m_sections;
};


#endif
/* vi: set ts=8 sw=4 sts=4 noet: */

#pragma once
#include "SectionInfo.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
class ConfigMgr
{
public:
    ~ConfigMgr(){
        _config_map.clear();
    }
    SectionInfo operator[](const std::string& section){
        if(_config_map.find(section)==_config_map.end()){
            return SectionInfo();
        }
        return _config_map[section];
    }

   ConfigMgr& operator=(const ConfigMgr& src) {
        if (&src == this) {
            return *this;
        }

        this->_config_map = src._config_map;
    };
     ConfigMgr(const ConfigMgr& src) {
        this->_config_map = src._config_map;
    }

    ConfigMgr();
    static ConfigMgr& Inst(){
        static ConfigMgr cfg_mgr;
        return cfg_mgr;
    }
private:

    // 存储section和key-value对的map  
    std::map<std::string, SectionInfo> _config_map;
};

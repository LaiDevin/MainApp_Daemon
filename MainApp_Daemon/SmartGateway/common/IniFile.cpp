#include "IniFile.h"
#include <fstream>
#include <sstream>
#include <algorithm>

IniFile::IniFile()
{
    m_vecCommentSign.push_back("//");
    m_vecCommentSign.push_back("#");
    m_vecCommentSign.push_back(";");
    m_modified = false;
}

IniFile::~IniFile()
{
    if (m_modified)
        SaveFile();
}

void IniFile::SetCommentSign(const std::vector<std::string> &vecCommentSign)
{
    for (auto it = vecCommentSign.begin(); it != vecCommentSign.end(); ++it)
    {
        if (std::find(m_vecCommentSign.begin(), m_vecCommentSign.end(), *it) == m_vecCommentSign.end())
        {
            m_vecCommentSign.push_back(*it);
        }
    }
}

void IniFile::LoadFile(const std::string &fileName)
{
    m_fileName = fileName;
    m_iniContent.clear();
    std::ifstream ifs(m_fileName);
    std::stringstream ss;
    ss << ifs.rdbuf();
    // Init a default section
    std::string sectionName = "default";
    T_Section section(sectionName);
    m_iniContent.push_back(section);
    // parse other sections
    std::vector<std::string> lineList = split(ss.str(), "\n", false);
    std::string strLine;
    for (auto it = lineList.begin(); it != lineList.end(); ++it) 
    {
        strLine = *it;
        T_LineConf lineConf;
        for (auto itComment = m_vecCommentSign.begin(); itComment != m_vecCommentSign.end(); ++itComment)
        {
            std::string::size_type commentPos = strLine.find_first_of(*itComment);            
            if (commentPos != std::string::npos) 
            {
                lineConf.Comment = trim(strLine.substr(commentPos)); // include comment sign
                strLine = strLine.substr(0, commentPos);                
                break;
            }
        }
        lineConf.Content = trim(strLine);
        std::string::size_type leftPos, rightPos;
        if ((strLine.npos != (leftPos = strLine.find('['))) && (strLine.npos != (rightPos = strLine.find(']'))))
        {
            sectionName = strLine.substr(leftPos + 1, rightPos - leftPos - 1);
            T_Section section(sectionName);
            if (!lineConf.Comment.empty()) // ����+ע�ͣ�ע�͸�ʽ��Ϊһ����
            {
                lineConf.Content = "";
                section.SectionLines.push_back(lineConf);
            }
            m_iniContent.push_back(section);
        }
        else
        {
            std::string::size_type equalPos = strLine.find('=');
            if (equalPos != std::string::npos)
            {
                lineConf.Key = trim(strLine.substr(0, equalPos));
                lineConf.Value = trim(strLine.substr(equalPos + 1));
            }
            auto itSection = std::find_if(m_iniContent.begin(), m_iniContent.end(), [sectionName](const T_Section &sec) { 
                return sec.SectionName == sectionName; 
            });
            if (itSection != m_iniContent.end())
                itSection->SectionLines.push_back(lineConf);
        }
    }
}

void IniFile::SaveFile()
{
    if (m_modified)
        SaveAsFile(m_fileName);
}

void IniFile::SaveAsFile(const std::string &fileName)
{
    std::ofstream ofs(fileName);
    if (!ofs)
        return;
    for (auto itSection = m_iniContent.begin(); itSection != m_iniContent.end(); ++itSection) 
    {
        if (itSection->SectionName != "default")
            ofs << "[" << itSection->SectionName << "]" << "\n";
        for (auto itLine = itSection->SectionLines.begin(); itLine != itSection->SectionLines.end(); ++itLine)
        {
            if (!itLine->Key.empty())
                ofs << itLine->Key << "=" << itLine->Value << " " << itLine->Comment << "\n";
            else if (!itLine->Content.empty())
                ofs << itLine->Content << itLine->Comment << "\n"; // std::endl;
            else
                ofs << itLine->Comment << "\n";
        }
        //if (itSection != m_iniContent.end() - 1)
        //    ofs << "\n";
    }
    ofs.flush();
    m_modified = false;
}

std::string IniFile::ReadSection(const std::string &sectionName)
{
    std::ostringstream oss;
    auto itSection = std::find_if(m_iniContent.begin(), m_iniContent.end(), [sectionName](const T_Section &sec) {
        return sec.SectionName == sectionName;
    });
    if (itSection != m_iniContent.end())
    {
        for (auto itLine = itSection->SectionLines.begin(); itLine != itSection->SectionLines.end(); ++itLine)
        {
            if (!itLine->Content.empty()) // remove all comments
                oss << itLine->Content << "\n";
        }
    }
    return oss.str();
}

void IniFile::ReadSection(const std::string &sectionName, std::vector<T_LineConf> &vecSection)
{
    vecSection.clear();
    auto itSection = std::find_if(m_iniContent.begin(), m_iniContent.end(), [sectionName](const T_Section &sec) {
        return sec.SectionName == sectionName;
    });
    if (itSection != m_iniContent.end())
        vecSection = itSection->SectionLines;
}

int IniFile::ReadInteger(const std::string &sectionName, const std::string &key)
{
    std::string val = ReadString(sectionName, key);
    if (!val.empty())
    {
        if ((val.substr(0, 2) == "0x") || (val.substr(0, 2) == "0X"))
            return std::stoi(val.substr(2), 0, 16);
        else
            return std::stoi(val);
    }
    return 0;
}

std::string IniFile::ReadString(const std::string &sectionName, const std::string &key)
{
    auto itSection = std::find_if(m_iniContent.begin(), m_iniContent.end(), [sectionName](const T_Section &sec) {
        return sec.SectionName == sectionName;
    });
    if (itSection != m_iniContent.end())
    {
        auto itLine = std::find_if(itSection->SectionLines.begin(), itSection->SectionLines.end(), [key](const T_LineConf &lineConf){
            return lineConf.Key == key;
        });
        if (itLine != itSection->SectionLines.end())
            return itLine->Value;
    }
    return "";
}

bool IniFile::WriteInteger(const std::string &sectionName, const std::string &key, int value)
{
    std::string strValue = std::to_string(value);
    return WriteString(sectionName, key, strValue);
}

bool IniFile::WriteString(const std::string &sectionName, const std::string &key, const std::string &value)
{
    auto itSection = std::find_if(m_iniContent.begin(), m_iniContent.end(), [sectionName](const T_Section &sec) {
        return sec.SectionName == sectionName;
    });
    if (itSection != m_iniContent.end())
    {
        auto itLine = std::find_if(itSection->SectionLines.begin(), itSection->SectionLines.end(), [key](const T_LineConf &lineConf){
            return lineConf.Key == key;
        });
        if (itLine != itSection->SectionLines.end())
            itLine->Value = value;
        else
        {
            // find the first blank line
            auto itBlankLine = std::find_if(itSection->SectionLines.begin(), itSection->SectionLines.end(), [](const T_LineConf &lineConf){
                return lineConf.Key.empty() && lineConf.Value.empty() && lineConf.Comment.empty() && lineConf.Content.empty();
            });
            if (itBlankLine != itSection->SectionLines.end())
            {
                itBlankLine->Key = key;  // Modify the blank line
                itBlankLine->Value = value;
                itSection->SectionLines.push_back(T_LineConf()); // And add a blank line
            }
            else
                itSection->SectionLines.push_back(T_LineConf(key, value, "", key + "=" + value));
        }
    }
    else
    {
        T_Section section(sectionName);
        section.SectionLines.push_back(T_LineConf(key, value, "", key + "=" + value));
        section.SectionLines.push_back(T_LineConf()); // Add a blank line
        m_iniContent.push_back(section);
    }
    m_modified = true;
    SaveFile();
    return true;
}

std::string IniFile::ltrim(std::string s)
{
    if (s.empty())
        return s;
    const char EMPTY_CHARS[] = { '\x20', '\x0d', '\x0a', '\x09' };
    int pos = 0;
    for (int i = 0; i < sizeof(EMPTY_CHARS) / sizeof(char); i++)
    {
        int t = s.find_first_not_of(EMPTY_CHARS[i]);
        if (t > pos)
            pos = t;
    }
    s.erase(0, pos);
    return s;
}

std::string IniFile::rtrim(std::string s)
{
    if (s.empty())
        return s;
    const char EMPTY_CHARS[] = { '\x20', '\x0d', '\x0a', '\x09' };
    int pos = s.size() - 1;
    for (int i = 0; i < sizeof(EMPTY_CHARS) / sizeof(char); i++)
    {
        int t = s.find_last_not_of(EMPTY_CHARS[i]);
        if (t < pos)
            pos = t;
    }
    s.erase(pos + 1);
    return s;
}

std::string IniFile::trim(std::string s)
{
    s = rtrim(ltrim(s));
    return s;
}

std::vector<std::string> IniFile::split(const std::string &str, const std::string &spliter, bool skipEmptyParts)
{
    std::vector<std::string> slTemp;
    std::string strTemp = str;
    std::string::size_type pos;
    do
    {
        pos = strTemp.find(spliter);
        std::string strSection = trim(strTemp.substr(0, pos));
        bool flag = !strSection.empty() || (strSection.empty() && !skipEmptyParts);
        if (flag)
            slTemp.push_back(strSection);
        if (pos != std::string::npos)
            strTemp = strTemp.substr(pos + 1);
    } while (pos != std::string::npos);
    return slTemp;
}

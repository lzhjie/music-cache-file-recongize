#include "file_head_define.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <io.h>
#include <stdexcept>
#include <fstream>
#include <winsock2.h>
#include <iosfwd>
#include "prefix_match_tree.h"

#pragma comment(lib, "ws2_32")

namespace file_recognise 
{

void Utf8ToUnicode(const std::string& source, std::string& dest) 
{
    std::wstring temp(source.size() * 2, 0);
    int temp_len = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), 
        const_cast<wchar_t *>(temp.c_str()), temp.size());
    if(temp_len <= 1) {
        return;
    }
    temp.resize(temp_len);
    dest.assign((const char*)temp.c_str(), temp_len * 2);
}
void UnicodeToGb2312(const std::string& source, std::string& dest)
{
    dest.assign(source.size(), 0);
    int result_len = WideCharToMultiByte(CP_ACP, 0, (const wchar_t *)(source.c_str()),source.size()/2, 
        const_cast<char *>(dest.c_str()), dest.size(), 0, 0);
    dest.resize(result_len);
    dest = dest.c_str(); // 去结束符
}

class FileFormatManager {
public:
    FileFormatManager()
    {
        files_ = new CFacadePrefixMactchTree<FileHeadAbstract*>(new CAsciiSymbolMap, 0);
    }
    ~FileFormatManager()
    {
        delete files_;
    }
    template<class FileHeadConcrete>
    void Regist() {
        try {
            FileHeadAbstract* file_head = new FileHeadConcrete;
            if(files_->InsertData(file_head->key().c_str(), &file_head) != 0) {
                fprintf(stderr, "%s Regist failed\r\n", file_head->key().c_str());
            }

        }
        catch(...) {
            fprintf(stderr, "FileFormatManager::Regist: memory error\r\n");
        }
    }
    FileHeadAbstract* FindFileHead(std::string key) 
    {
        FileHeadAbstract*const* file_head = files_->FindData(key.c_str());
        return file_head ? *file_head : 0;
    }
    void RecogniseFile(const char* name)
    {
        if(!name) {
            return ;
        }
        fprintf(stderr, "recognise file:%s \r\n", name);
        std::fstream file(name, std::ios::in|std::ios::binary);
        if (file.fail())
        {
            fprintf(stderr, "open file:%s fail\r\n", name);
            return ;
        }    
        std::string key;
        file >> key;
        FileHeadAbstract* file_recognise = FindFileHead(key);
        if(!file_recognise) {
            fprintf(stderr, "unknwon type:%s\r\n", key.c_str());
            return ;
        }
        file.seekp(0);
        std::string head(file_recognise->head_len(), 0);
        file.read((char*)head.c_str(), head.size());
        int read_len = file.tellp();
        file.close();
        if(read_len != head.size()) {
            fprintf(stderr, "unknown file\r\n");
            return ;
        }
        if(file_recognise->ProcessHead(head) == 0)
        {
            std::string music_name;
            UnicodeToGb2312(file_recognise->music_name(), music_name);
            std::string full_name = music_name + "." + file_recognise->suffix();
            std::cout << "result:" << full_name << std::endl;
            if(full_name != name) {
                rename(name, full_name.c_str());
            }
        }     
    }
private:
    CFacadePrefixMactchTree<FileHeadAbstract*>* files_;

public:
    static FileFormatManager* g_manager_;
}; 
FileFormatManager* FileFormatManager::g_manager_ = 0;

class FileHeadMp3 : public FileHeadAbstract {
public:
#pragma pack(1)
    struct FrameHead{
        char id[4];
        int size;
        char flag[2];
        char code_type;
    };
#pragma pack( )
    FileHeadMp3():FileHeadAbstract(256, "ID", "mp3")
    {
        
    }
    virtual int ProcessHead(const std::string& head_data)
    {
        music_name_.clear();
        const char* music_token = "TIT2";
        const char* data = head_data.c_str();
        int compare_times = head_data.size() - strlen(music_token);
        while(compare_times> 0) {
            if(memcmp(data, music_token, strlen(music_token)) == 0 ) {
                FrameHead frame_head = *(const FrameHead*)data;
                frame_head.size = ntohl(frame_head.size);
                -- frame_head.size; // 去编码标识
                if(frame_head.size < 128) {
                    const char* title = data + sizeof(FrameHead);
                    int title_len = frame_head.size;
                    if(frame_head.code_type == 1) { // UNICODE
                        music_name_.append(title + 2, title_len - 2); // 去BOM
                    }
                    if(frame_head.code_type == 3) {  // UTF8
                        std::string utf8_string;
                        utf8_string.append(title, title_len);
                        Utf8ToUnicode(utf8_string, music_name_);
                    }
                    else {
                        return -1;
                    }
                    /*
                    while(title_len > 0) {
                        if(*title)
                            music_name_.push_back(*title);
                        ++ title;
                        -- title_len;
                    }
                    */
                    return 0;
                }
                else {
                    music_name_ = "";
                }
                break;
            }
            ++ data;
            -- compare_times;
        }
        return -1;
    }
    
};

void Init()
{
    try{
        FileFormatManager::g_manager_ = new FileFormatManager;
        FileFormatManager::g_manager_->Regist<FileHeadMp3>();
    }
    catch(...) {
        fprintf(stderr, "Init falil \r\n");
    }  
}

void RecogniseFile(const char* name)
{
    if(!FileFormatManager::g_manager_) {
        return ;
    }
    FileFormatManager::g_manager_->RecogniseFile(name);
}
}
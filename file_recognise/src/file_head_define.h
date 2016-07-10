#ifndef SRC_FILE_HEAD_DEFINE_H_
#define SRC_FILE_HEAD_DEFINE_H_

#include <iostream>
#include <string>

namespace file_recognise
{
class FileHeadAbstract {
public:
    FileHeadAbstract(int head_len, const char* key, const char* suffix) {
        head_len_ = head_len;
        key_ = key;
        suffix_ = suffix;
    }
    virtual int ProcessHead(const std::string& head_data) = 0;
    int head_len() const{return head_len_;}
    std::string key() const{return key_;}
    std::string suffix() const{return suffix_;}
    std::string music_name() const {return music_name_;}
protected:
    std::string music_name_;
private:
    int head_len_;
    std::string key_;
    std::string suffix_;
    
}; 

void Init();
void RecogniseFile(const char* name);
}

#endif
#include "file_head_define.h"
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

void PaserDir(const char* dir)
{
    int lHandle = 0;
    _finddata_t tFindData = {0};
    if(dir && _chdir(dir)!=0)  return ;
    if(lHandle = _findfirst("*.*", &tFindData), lHandle != -1L)
    {
        do
        {
            if((tFindData.attrib & _A_SUBDIR) )
            {
                if(tFindData.name[0] != '.')
                {
                    PaserDir(tFindData.name);
                }
            }
            else
            {
                file_recognise::RecogniseFile(tFindData.name);
            }
        }
        while(_findnext(lHandle, &tFindData) == 0);
        _findclose(lHandle);
    }
    if(dir && _chdir("../")!=0)  return;
}
int main(int, char** args) 
{
    setlocale(LC_ALL, "");
    file_recognise::Init();
    PaserDir("./music");
    //PaserDir("./");
    system("pause");
    return 0;
}
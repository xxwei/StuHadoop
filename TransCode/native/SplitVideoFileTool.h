#ifndef SPLITVIDEOFILETOOL_H
#define SPLITVIDEOFILETOOL_H
#include"UtilTool.h"
#include"Allegion_Hadoop_SplitVideoFileTool.h"

class SplitVideoFileTool
{
    public:
        SplitVideoFileTool();
        virtual ~SplitVideoFileTool();
    protected:
    private:
};
void  *SplitThread(void *lp);
int    Split_ReadBuffer(void *opaque,uint8_t *buf,int buf_size);
int    Split_WriteBuffer(void *opaque,uint8_t *buf,int buf_size);
#endif // SPLITVIDEOFILETOOL_H

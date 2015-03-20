#ifndef MERGEVIDEOSPLITTOOL_H
#define MERGEVIDEOSPLITTOOL_H
#include"UtilTool.h"
#include"Allegion_Hadoop_MergeVideoSplitTool.h"

class MergeVideoSplitTool
{
    public:
        MergeVideoSplitTool();
        virtual ~MergeVideoSplitTool();
    protected:
    private:
};
void  *MergeThread(void *lp);
int    Merge_ReadBuffer(void *opaque,uint8_t *buf,int buf_size);
int    Merge_WriteBuffer(void *opaque,uint8_t *buf,int buf_size);
#endif // MERGEVIDEOSPLITTOOL_H

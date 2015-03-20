#ifndef TRANSVIDEOSPLITTOOL_H
#define TRANSVIDEOSPLITTOOL_H
#include"UtilTool.h"
#include"Allegion_Hadoop_TransVideoSplitTool.h"

class TransVideoSplitTool
{
    public:
        TransVideoSplitTool();
        virtual ~TransVideoSplitTool();
    protected:
    private:
};
void  *TransThread(void *lp);
int    Trans_ReadBuffer(void *opaque,uint8_t *buf,int buf_size);
int    Trans_WriteBuffer(void *opaque,uint8_t *buf,int buf_size);
#endif // TRANSVIDEOSPLITTOOL_H

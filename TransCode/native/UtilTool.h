#ifndef UTILCALL_H_INCLUDED
#define UTILCALL_H_INCLUDED
#include<iostream>
#include<map>
#include<pthread.h>
#include<jni.h>
#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif
extern "C"
{
	
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
}
using namespace std;


struct nav_Split
{
    AVFormatContext         *ifmt_ctx;
    AVFormatContext         *ofmt_ctx;
    jobject                               split_read_obj;
    jmethodID                       split_read_data_mid;
    jobject                               split_write_obj;
    jmethodID                       split_write_data_mid;
    jobject                               split_new_obj;
    jmethodID                       split_new_split_mid;
    AVIOContext                 *split_avio_in;
    AVIOContext                 *split_avio_out;
    long                                    id;
    uint8_t                           *avio_in_buffer;
    uint8_t                           *avio_out_buffer;
};

typedef map<long ,nav_Split>            SplitMap;

struct nav_Trans
{
     AVFormatContext         *ifmt_ctx;
    AVFormatContext         *ofmt_ctx;
    jobject                               trans_read_obj;
    jmethodID                       trans_read_data_mid;
     jobject                               trans_write_obj;
    jmethodID                       trans_write_data_mid;
    AVIOContext                 *trans_avio_in;
    AVIOContext                 *trans_avio_out;
    uint8_t                           *avio_in_buffer;
    uint8_t                           *avio_out_buffer;
    char                                 dest_format[1024];
};
typedef map<long ,nav_Trans>            TransMap;

struct nav_Merge
{
    AVFormatContext         *ifmt_ctx;
    AVFormatContext         *ofmt_ctx;
    jobject                               merge_read_obj;
    jmethodID                       merge_read_data_mid;
    jobject                               merge_write_obj;
    jmethodID                       merge_write_data_mid;
    AVIOContext                 *merge_avio_in;
    AVIOContext                 *merge_avio_out;
    uint8_t                           *avio_in_buffer;
    uint8_t                           *avio_out_buffer;
    long                                    nvid;
};
typedef map<long ,nav_Merge>            MergeMap;

#define AVIO_SIZE           32768

JNIEnv *GetEnv();

#endif // UTILCALL_H_INCLUDED

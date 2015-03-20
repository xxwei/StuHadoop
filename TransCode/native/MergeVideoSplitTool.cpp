#include "MergeVideoSplitTool.h"
extern MergeMap        g_nav_merge_map;
extern long                    g_nav_id;
extern JavaVM              *g_vm;
MergeVideoSplitTool::MergeVideoSplitTool()
{
    //ctor
}

MergeVideoSplitTool::~MergeVideoSplitTool()
{
    //dtor
}
/*
 * Class:     Allegion_Hadoop_MergeVideoSplitTool
 * Method:    InitFFmpeg
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_MergeVideoSplitTool_InitFFmpeg(JNIEnv *env, jobject obj)
{
    cout<<"初始化InitFFMpeg For Merge"<<endl;
    av_register_all();
    g_nav_id++;
    nav_Merge          nM;
    avformat_alloc_output_context2(&nM.ofmt_ctx,NULL,NULL,"0.ts");
    nM.merge_read_data_mid=NULL;
    nM.merge_write_data_mid=NULL;

    nM.avio_out_buffer= (uint8_t *)av_malloc(AVIO_SIZE);
    nM.merge_avio_out = avio_alloc_context(nM.avio_out_buffer,AVIO_SIZE,1,&g_nav_id,NULL,Merge_WriteBuffer,NULL);
     nM.nvid = g_nav_id;
     g_nav_merge_map.insert(map<long,nav_Merge>::value_type(g_nav_id,nM));

     return g_nav_id;
}

/*
 * Class:     Allegion_Hadoop_MergeVideoSplitTool
 * Method:    SetMergeReadCallBack
 * Signature: (JLjava/lang/Object;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_MergeVideoSplitTool_SetMergeReadCallBack(JNIEnv *env, jobject obj_self, jlong nvid, jobject obj, jstring funname)
{
    MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
        const char *signame="(J)J";//签名
        jclass recls = env->GetObjectClass(obj);
        const char *fun_name = env->GetStringUTFChars(funname,0);
        item->second.merge_read_obj = env->NewGlobalRef(obj);
        item->second.merge_read_data_mid = env->GetMethodID(recls,fun_name,signame);

        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_MergeVideoSplitTool
 * Method:    SetMergeWriteCallBack
 * Signature: (JLjava/lang/Object;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_MergeVideoSplitTool_SetMergeWriteCallBack(JNIEnv * env, jobject obj_self, jlong nvid, jobject obj, jstring funname)
{
    MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
        const char *signame="(J)J";//签名
        jclass recls = env->GetObjectClass(obj);
        const char *fun_name = env->GetStringUTFChars(funname,0);
        item->second.merge_write_obj = env->NewGlobalRef(obj);
        item->second.merge_write_data_mid = env->GetMethodID(recls,fun_name,signame);
        item->second.ofmt_ctx->pb =  item->second.merge_avio_out;
        item->second.ofmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_MergeVideoSplitTool
 * Method:    HaveNewSplit
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_MergeVideoSplitTool_HaveNewSplit(JNIEnv * env, jobject obj, jlong nvid)
{
    //对每个文件启动一个线程
   MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
         cout<<"打开输入流"<<nvid<<endl;
        item->second.ifmt_ctx = avformat_alloc_context();
        item->second.avio_in_buffer = (uint8_t *)av_malloc(AVIO_SIZE);
        item->second.merge_avio_in = avio_alloc_context(item->second.avio_in_buffer,AVIO_SIZE,0,&nvid,Merge_ReadBuffer,NULL,NULL);
        item->second.ifmt_ctx->pb =  item->second.merge_avio_in;
        item->second.ifmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
        AVInputFormat   *piFmt = NULL;
        int ret = av_probe_input_buffer(item->second.merge_avio_in,&piFmt,"",NULL,0,0);
        if(ret<0)
        {
            cout<<"probe failed"<<endl;
        }
        else
        {
            cout<<"检测到文件格式"<<piFmt->name<<"    "<<piFmt->long_name<<endl;
        }
        if(avformat_open_input(&item->second.ifmt_ctx,"",NULL,NULL)!=0)
        {
            cout<<"Couldn't open input stream(无法打开输入流)"<<endl;
            return -1;
        }
        if ((ret = avformat_find_stream_info(item->second.ifmt_ctx, NULL)) < 0)
        {
            cout<<"流类型查找失败"<<endl;
            return -1;
        }

        cout<<"准备创建视频文件合并线程"<<endl;
         pthread_t       merge_thread;
         pthread_create(&merge_thread,NULL,MergeThread,&( item->second));
         pthread_join(merge_thread,NULL);
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_MergeVideoSplitTool
 * Method:    StartMerge
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_MergeVideoSplitTool_StartMerge(JNIEnv *env, jobject obj, jlong nvid)
{
    //格式从第一个文件中复制，写入输出文件头
    cout<<"启动合并项目"<<endl;
    MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
        item->second.ifmt_ctx = avformat_alloc_context();
        item->second.avio_in_buffer = (uint8_t *)av_malloc(AVIO_SIZE);
        item->second.merge_avio_in = avio_alloc_context(item->second.avio_in_buffer,AVIO_SIZE,0,&nvid,Merge_ReadBuffer,NULL,NULL);
        item->second.ifmt_ctx->pb =  item->second.merge_avio_in;
        item->second.ifmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
         cout<<"打开输入流"<<nvid<<endl;
        AVInputFormat   *piFmt = NULL;
        int ret = av_probe_input_buffer(item->second.merge_avio_in,&piFmt,"",NULL,0,0);
        if(ret<0)
        {
            cout<<"probe failed"<<endl;
        }
        else
        {
            cout<<"检测到文件格式"<<piFmt->name<<"    "<<piFmt->long_name<<endl;
        }
        if(avformat_open_input(&item->second.ifmt_ctx,"",NULL,NULL)!=0)
        {
            cout<<"Couldn't open input stream(无法打开输入流)"<<endl;
            return -1;
        }
        if ((ret = avformat_find_stream_info(item->second.ifmt_ctx, NULL)) < 0)
        {
            cout<<"流类型查找失败"<<endl;
            return -1;
        }

         for(int j = 0;j<item->second.ifmt_ctx->nb_streams;j++)//给输出文件设置格式
        {
             int i = (int)j;
            AVStream *in_stream = item->second.ifmt_ctx->streams[i];
            AVStream *out_stream = avformat_new_stream(item->second.ofmt_ctx,in_stream->codec->codec);
            avcodec_copy_context(out_stream->codec,in_stream->codec);
            out_stream->codec->codec_tag = 0;
            if(item->second.ofmt_ctx->oformat->flags&AVFMT_GLOBALHEADER)
                out_stream->codec->flags|=CODEC_FLAG_GLOBAL_HEADER;
        }
        avio_open_dyn_buf(&item->second.merge_avio_out);
        avformat_write_header(item->second.ofmt_ctx,NULL);//写入文件头
         pthread_t       merge_thread;
         pthread_create(&merge_thread,NULL,MergeThread,&( item->second));
         pthread_join(merge_thread,NULL);
    }
    return 0;
}
/*
 * Class:     Allegion_Hadoop_MergeVideoSplitTool
 * Method:    StopMerge
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_MergeVideoSplitTool_StopMerge(JNIEnv *env, jobject obj, jlong nvid)
{
    MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
        av_write_trailer(item->second.ofmt_ctx);
        avio_close_dyn_buf(item->second.merge_avio_out,&item->second.avio_out_buffer);
        avformat_close_input(&item->second.ofmt_ctx);
        av_freep(item->second.avio_out_buffer);
        av_freep(item->second.merge_avio_out);
        avformat_free_context(item->second.ofmt_ctx);
        g_nav_merge_map.erase(item);
    }
    return 0;
}
void  *MergeThread(void *lp)
{
    JNIEnv      *env;
    g_vm->AttachCurrentThread((void **)&env,NULL);
    nav_Merge       * nM = (nav_Merge *)lp;
    AVPacket            packet;
    cout<<"合并线程启动   "<<endl;
    int ret = 0;
    unsigned int SeqNum = 0;
    //格式复制
     AVStream *in_stream_t = NULL;
     AVStream *out_stream_t = NULL;
     while(av_read_frame(nM->ifmt_ctx,&packet)>=0)
    {
        in_stream_t = nM->ifmt_ctx->streams[packet.stream_index];
        out_stream_t = nM->ofmt_ctx->streams[packet.stream_index];
        cout<<"  合并时间设置 "<<packet.pts<<"   "<<in_stream_t->time_base.den<<"   "<<in_stream_t->time_base.num<<endl;
        packet.pts = av_rescale_q_rnd(packet.pts,in_stream_t->time_base,out_stream_t->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts,in_stream_t->time_base,out_stream_t->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration,in_stream_t->time_base,out_stream_t->time_base);
        //packet.pos = -1;
         cout<<"  合并时间设置 "<<packet.pts<<"   "<<in_stream_t->time_base.den<<"   "<<in_stream_t->time_base.num<<endl;
        //ret = av_interleaved_write_frame(nM->ofmt_ctx,&packet);
        ret = av_write_frame(nM->ofmt_ctx,&packet);
        av_free_packet(&packet);
    }
    //结束输入文件
     avformat_close_input(&nM->ifmt_ctx);
     av_freep(nM->merge_avio_in);
     avformat_free_context(nM->ifmt_ctx);
    return NULL;
}
int    Merge_ReadBuffer(void *opaque,uint8_t *buf,int buf_size)
{
    JNIEnv      *env ;
    long       nvid = *((long *)opaque);
    g_vm->AttachCurrentThread((void **)&env,NULL);
    int  ret = -1;
    MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
        //cout<<"转码ffmpeg读取回调  准备读取"<<buf_size<<"大小的数据"<<endl;
        env->CallLongMethod( item->second.merge_read_obj,item->second.merge_read_data_mid,buf_size);
        jclass recls =env->GetObjectClass( item->second.merge_read_obj);
        jfieldID fid_len=   env->GetFieldID(recls,"readbuflen","J");
        jlong len  =env->GetLongField( item->second.merge_read_obj,fid_len);
        //cout<<"收到"<<len<<"byte数据"<<endl;
        jfieldID fid_buf = env->GetFieldID(recls,"readBuf","[B");
        jbyteArray  read_byteArray=  (jbyteArray) env->GetObjectField( item->second.merge_read_obj,fid_buf);
        jbyte * byte_arr =  env->GetByteArrayElements(read_byteArray,NULL);
        jlong arr_len = env->GetArrayLength(read_byteArray);
        if(len>0)
        {
            memcpy(buf,byte_arr,len);
        }
        env->ReleaseByteArrayElements(read_byteArray,byte_arr,0);
        ret = len;
    }
    g_vm->DetachCurrentThread();
    if(ret<0)
    {
        cout<<"没有读取到数据，可能流结束"<<endl;
        return -1;
    }
    return ret;
}
int    Merge_WriteBuffer(void *opaque,uint8_t *buf,int buf_size)
{
    JNIEnv      *env = GetEnv();
    int           ret = 0;
    long       nvid = *((long *)opaque);
    //cout<<"ffmpeg写入回调  准备写入"<<buf_size<<"大小的数据"<<endl;
    g_vm->AttachCurrentThread((void **)&env,NULL);
    MergeMap::iterator item = g_nav_merge_map.find(nvid);
    if(item!=g_nav_merge_map.end())
    {
         //cout<<"ffmpeg写入回调  准备写入"<<buf_size<<"大小的数据"<<endl;
         jclass recls =env->GetObjectClass( item->second.merge_write_obj);
         jfieldID fid_len=   env->GetFieldID(recls,"writebuflen","J");
         jlong len = buf_size;
         env->SetLongField(item->second.merge_write_obj,fid_len,len);
         jfieldID fid_buf=   env->GetFieldID(recls,"writeBuf","[B");
         jbyteArray  write_byteArray=  (jbyteArray) env->GetObjectField( item->second.merge_write_obj,fid_buf);
         jbyte * byte_arr =  env->GetByteArrayElements(write_byteArray,NULL);
         jlong arr_len = env->GetArrayLength(write_byteArray);
         if(arr_len>=buf_size)
         {
             memcpy(byte_arr,buf,buf_size);
             ret = buf_size;
         }
        env->ReleaseByteArrayElements(write_byteArray,byte_arr,0);
        env->CallLongMethod( item->second.merge_write_obj,item->second.merge_write_data_mid,ret);
        if(ret<AVIO_SIZE)
        {
             cout<<"结束写入文件"<<endl;
            env->CallLongMethod( item->second.merge_write_obj,item->second.merge_write_data_mid,0);
        }
    }
    g_vm->DetachCurrentThread();
    return ret;
}

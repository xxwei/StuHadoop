#include "SplitVideoFileTool.h"
extern SplitMap            g_nav_split_map;
extern long                    g_nav_id;
extern JavaVM              *g_vm;

jobject                                 g_obj;
SplitVideoFileTool::SplitVideoFileTool()
{
    //ctor
}

SplitVideoFileTool::~SplitVideoFileTool()
{
    //dtor
}


extern "C"
{
/*
 * Class:     Allegion_Hadoop_SplitVideoFileTool
 * Method:    InitFFmpeg
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_SplitVideoFileTool_InitFFmpeg(JNIEnv *env, jobject obj, jstring FormatName)
{
     cout<<"初始化InitFFMpeg For Split"<<endl;
    av_register_all();
    const char *format_name = env->GetStringUTFChars(FormatName,0);
    nav_Split       nav;
    g_nav_id++;
    nav.ifmt_ctx = avformat_alloc_context();
    avformat_alloc_output_context2(&nav.ofmt_ctx,NULL,NULL,"0.ts");
    //nav.ofmt_ctx = avformat_alloc_context();
    nav.split_read_data_mid = NULL;
    nav.split_write_data_mid = NULL;
    nav.split_new_split_mid = NULL;
    nav.id = 10000;
    nav.avio_in_buffer = (uint8_t *)av_malloc(AVIO_SIZE);
    nav.split_avio_in = avio_alloc_context(nav.avio_in_buffer,AVIO_SIZE,0,&g_nav_id,Split_ReadBuffer,NULL,NULL);
    nav.avio_out_buffer= (uint8_t *)av_malloc(AVIO_SIZE);
    nav.split_avio_out = avio_alloc_context(nav.avio_out_buffer,AVIO_SIZE,1,&g_nav_id,NULL,Split_WriteBuffer,NULL);

    g_nav_split_map.insert(map<long,nav_Split>::value_type(g_nav_id,nav));
    return g_nav_id;
}

/*
 * Class:     Allegion_Hadoop_SplitVideoFileTool
 * Method:    SetSplitReadCallBack
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_SplitVideoFileTool_SetSplitReadCallBack(JNIEnv * env, jobject obj_self, jlong nvid,jobject obj, jstring funName)
{
    SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
         cout<<"SetSplitReadCallBack"<<nvid<<endl;
        const char *signame="(J)J";//签名
        jclass recls = env->GetObjectClass(obj);
        const char *fun_name = env->GetStringUTFChars(funName,0);
        item->second.split_read_obj = env->NewGlobalRef(obj);
        item->second.split_read_data_mid = env->GetMethodID(recls,fun_name,signame);
        item->second.ifmt_ctx->pb =  item->second.split_avio_in;
        item->second.ifmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
        //env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,1111);
        //cout<<"SetSplitReadCallBack  over"<<endl;
        return 0;
    }
    return -1;
}
/*
 * Class:     Allegion_Hadoop_SplitVideoFileTool
 * Method:    SetSplitWriteCallBack
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_SplitVideoFileTool_SetSplitWriteCallBack(JNIEnv * env,  jobject obj_self, jlong nvid,jobject obj, jstring funName)
{
    SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
         cout<<"SetSplitWriteCallBack"<<nvid<<endl;
        const char *signame="(J)J";//签名
        jclass recls = env->GetObjectClass(obj);
        const char *fun_name = env->GetStringUTFChars(funName,0);
        item->second.split_write_obj = env->NewGlobalRef(obj);
        item->second.split_write_data_mid = env->GetMethodID(recls,fun_name,signame);
        item->second.ofmt_ctx->pb = item->second.split_avio_out;
        item->second.ofmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;

        //item->second.split_avio_in = avio_alloc_context( item->second.inbuffer,32768,0,&(item->second),SplitReadHDFS,NULL,NULL);
        //env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,2222);
        //cout<<"SetSplitWriteCallBack  over"<<endl;
        return 0;
    }
    return -1;
}
/*
 * Class:     Allegion_Hadoop_SplitVideoFileTool
 * Method:    SetSplitNewSplitCallBack
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_SplitVideoFileTool_SetSplitNewSplitCallBack(JNIEnv * env,  jobject obj_self, jlong nvid,jobject obj,jstring funName)
{

    SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
         //cout<<"SetSplitNewSplitCallBack"<<nvid<<endl;
        const char *signame="(Ljava/lang/String;)J";//签名
        jclass recls = env->GetObjectClass(obj);
         item->second.split_new_obj = env->NewGlobalRef(obj);;
        const char *fun_name = env->GetStringUTFChars(funName,0);
        item->second.split_new_split_mid = env->GetMethodID(recls,fun_name,signame);
        //env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,3333);
        //item->second.split_avio_in = avio_alloc_context( item->second.inbuffer,32768,0,&(item->second),SplitReadHDFS,NULL,NULL);
        //item->second.env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,3123);
        //cout<<"SetSplitNewSplitCallBack  over"<<endl;
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_SplitVideoFileTool
 * Method:    StartSplit
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_SplitVideoFileTool_StartSplit(JNIEnv * env, jobject obj, jlong nvid)
{
    SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
         cout<<"打开输入流"<<nvid<<endl;
        AVInputFormat   *piFmt = NULL;
        int ret = av_probe_input_buffer(item->second.split_avio_in,&piFmt,"",NULL,0,0);
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
          cout<<"准备创建视频文件分离线程"<<endl;
         pthread_t       split_thread;
         pthread_create(&split_thread,NULL,SplitThread,&( item->second));

         pthread_join(split_thread,NULL);
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_SplitVideoFileTool
 * Method:    CloseSplit
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_SplitVideoFileTool_CloseSplit(JNIEnv *env, jobject obj, jlong nvid)
{
     SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
        cout<<"切分结束"<<endl;
        av_free(item->second.avio_in_buffer);
        av_free(item->second.avio_out_buffer);
        av_freep(item->second.split_avio_in);
        av_freep(item->second.split_avio_out);
        avformat_close_input(&item->second.ifmt_ctx);
        avformat_free_context(item->second.ifmt_ctx);
        avformat_free_context(item->second.ofmt_ctx);
        g_nav_split_map.erase(item);
    }
}

}// end extern "C"
//分片读取线程
void  *SplitThread(void *lp)
{
    JNIEnv      *env;
    g_vm->AttachCurrentThread((void **)&env,NULL);
    nav_Split       * nS = (nav_Split *)lp;
    cout<<"分离线程启动   "<<endl;
    int ret = 0;
    unsigned int SeqNum = 0;

    //格式复制
    for(int i = 0;i<nS->ifmt_ctx->nb_streams;i++)
    {
        AVStream *in_stream = nS->ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(nS->ofmt_ctx,in_stream->codec->codec);
        ret = avcodec_copy_context(out_stream->codec,in_stream->codec);
        out_stream->codec->codec_tag = 0;
        if(nS->ofmt_ctx->oformat->flags&AVFMT_GLOBALHEADER)
            out_stream->codec->flags|=CODEC_FLAG_GLOBAL_HEADER;
    }
     avio_open_dyn_buf(&nS->split_avio_out);
    char  temp_name[1024]={0};
    //通知新建分片文件
    AVStream        *in_stream_t;
    AVStream        *out_stream_t;
    AVPacket    packet ;
    int     CurSplitFileLength = 0;
    sprintf(temp_name,"%d.ts",SeqNum);
    unsigned int SeqFrameNum = 0;

    jstring split_name = env->NewStringUTF(temp_name);//stoJstring(env,temp_name);
    env->CallLongMethod(nS->split_new_obj,nS->split_new_split_mid,split_name);
    env->DeleteLocalRef(split_name);
    avformat_write_header(nS->ofmt_ctx,NULL);
    while(av_read_frame(nS->ifmt_ctx,&packet)>=0)
    {
            //cout<<"循环读取数据包"<<packet.size<<endl;
            CurSplitFileLength=CurSplitFileLength+packet.size;
            in_stream_t = nS->ifmt_ctx->streams[packet.stream_index];
            out_stream_t = nS->ofmt_ctx->streams[packet.stream_index];
            //时间转换
            packet.pts = av_rescale_q_rnd(packet.pts,in_stream_t->time_base,out_stream_t->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            packet.dts = av_rescale_q_rnd(packet.dts,in_stream_t->time_base,out_stream_t->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            packet.duration = av_rescale_q(packet.duration,in_stream_t->time_base,out_stream_t->time_base);
            packet.pos = -1;
            //cout<<"packegt dts"<<packet.dts<<"索引"<<packet.stream_index<<"---"<<out_stream_t->time_base.den<<"---"<<in_stream_t->time_base.den<<endl;
            //ret = av_write_frame(nS->ofmt_ctx,&packet);
            //ret = av_interleaved_write_frame(nS->ofmt_ctx,&packet);
            //cout<<"已经读取长度"<<CurSplitFileLength<<endl;

            //stoJstring(env,temp_name);
            if(CurSplitFileLength>1024*1024*10)
            {
                //cout<<"切片文件已经超过限制，开始对视频进行解码"<<CurSplitFileLength<<"---"<<packet.stream_index<<endl;
                //在这里判断帧类型，如果当前为I帧，则切分　如果不是，继续写入文件
                 if(in_stream_t->codec->codec_type==AVMEDIA_TYPE_VIDEO)
                {
                      if(packet.flags&AV_PKT_FLAG_KEY)
                      {
                                    cout<<"遇到关键帧　开始切分　总共写入"<<SeqFrameNum<<"帧"<<endl;
                                    SeqFrameNum = 0;
                                    av_write_trailer(nS->ofmt_ctx);
                                    avio_close_dyn_buf(nS->split_avio_out,&nS->avio_out_buffer);
                                    CurSplitFileLength = 0;
                                    SeqNum++;
                                    sprintf(temp_name,"%d.ts",SeqNum);
                                    cout<<"开始切分新的文件 切片文件名"<<temp_name<<endl;
                                    int ret = g_vm->AttachCurrentThread((void **)&env,NULL);
                                    split_name = env->NewStringUTF(temp_name);//stoJstring(env,temp_name);
                                    env->CallLongMethod(nS->split_new_obj,nS->split_new_split_mid,split_name);
                                    env->DeleteLocalRef(split_name);
                                    avio_open_dyn_buf(&nS->split_avio_out);
                                    avformat_write_header(nS->ofmt_ctx,NULL);
                    } else{cout<<"不是关键帧"<<endl;}
                 }else{cout<<"不是视频"<<endl;}
            }
            if(in_stream_t->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                SeqFrameNum++;
            }
            ret = av_interleaved_write_frame(nS->ofmt_ctx,&packet);
            av_free_packet(&packet);
    }
    av_free_packet(&packet);
    av_write_trailer(nS->ofmt_ctx);
    avio_close_dyn_buf(nS->split_avio_out,&nS->avio_out_buffer);
    cout<<"流结束"<<endl;
    g_vm->DetachCurrentThread();
    return NULL;
}
int    Split_ReadBuffer(void *opaque,uint8_t *buf,int buf_size)
{
    JNIEnv      *env ;
    long       nvid = *((long *)opaque);
    g_vm->AttachCurrentThread((void **)&env,NULL);
    int  ret = -1;
    SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
        //cout<<"ffmpeg读取回调  准备读取"<<buf_size<<"大小的数据"<<endl;
        env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,buf_size);
        jclass recls =env->GetObjectClass( item->second.split_read_obj);
        jfieldID fid_len=   env->GetFieldID(recls,"readbuflen","J");
        jlong len  =env->GetLongField( item->second.split_read_obj,fid_len);
        //cout<<"收到"<<len<<"byte数据"<<endl;
        jfieldID fid_buf = env->GetFieldID(recls,"readBuf","[B");
        jbyteArray  read_byteArray=  (jbyteArray) env->GetObjectField( item->second.split_read_obj,fid_buf);
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

int    Split_WriteBuffer(void *opaque,uint8_t *buf,int buf_size)
{
    JNIEnv      *env = GetEnv();
    int           ret = 0;
    long       nvid = *((long *)opaque);
    //cout<<"ffmpeg写入回调  准备写入"<<buf_size<<"大小的数据"<<endl;
    g_vm->AttachCurrentThread((void **)&env,NULL);
    SplitMap::iterator item = g_nav_split_map.find(nvid);
    if(item!=g_nav_split_map.end())
    {
         //cout<<"ffmpeg写入回调  准备写入"<<buf_size<<"大小的数据"<<endl;
         jclass recls =env->GetObjectClass( item->second.split_write_obj);
         jfieldID fid_len=   env->GetFieldID(recls,"writebuflen","J");
         jlong len = buf_size;
         env->SetLongField(item->second.split_write_obj,fid_len,len);
         jfieldID fid_buf=   env->GetFieldID(recls,"writeBuf","[B");
         jbyteArray  write_byteArray=  (jbyteArray) env->GetObjectField( item->second.split_write_obj,fid_buf);
         jbyte * byte_arr =  env->GetByteArrayElements(write_byteArray,NULL);
         jlong arr_len = env->GetArrayLength(write_byteArray);
         if(arr_len>=buf_size)
         {
             memcpy(byte_arr,buf,buf_size);
             ret = buf_size;
         }
        env->ReleaseByteArrayElements(write_byteArray,byte_arr,0);
        env->CallLongMethod( item->second.split_write_obj,item->second.split_write_data_mid,ret);
        if(ret<AVIO_SIZE)
        {
             cout<<"结束写入文件"<<endl;
            env->CallLongMethod( item->second.split_write_obj,item->second.split_write_data_mid,0);
        }
    }
    g_vm->DetachCurrentThread();
    return ret;
}



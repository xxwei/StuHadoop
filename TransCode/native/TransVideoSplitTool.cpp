#include "TransVideoSplitTool.h"
extern TransMap          g_nav_trans_map;
extern long                    g_nav_id;
extern JavaVM              *g_vm;
TransVideoSplitTool::TransVideoSplitTool()
{
    //ctor
}

TransVideoSplitTool::~TransVideoSplitTool()
{
    //dtor
}
/*
 * Class:     Allegion_Hadoop_TransVideoSplitTool
 * Method:    InitFFmpeg
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_TransVideoSplitTool_InitFFmpeg(JNIEnv *env, jobject obj, jstring format)
{
    cout<<"初始化InitFFMpeg For Trans"<<endl;
    av_register_all();
    const char *format_name = env->GetStringUTFChars(format,0);
    g_nav_id++;
    nav_Trans       nT;
    nT.ifmt_ctx = avformat_alloc_context();
    avformat_alloc_output_context2(&nT.ofmt_ctx,NULL,NULL,"0.ts");
    nT.trans_read_data_mid=NULL;
    nT.trans_write_data_mid=NULL;
    nT.avio_in_buffer = (uint8_t *)av_malloc(AVIO_SIZE);
    nT.trans_avio_in = avio_alloc_context(nT.avio_in_buffer,AVIO_SIZE,0,&g_nav_id,Trans_ReadBuffer,NULL,NULL);
    nT.avio_out_buffer= (uint8_t *)av_malloc(AVIO_SIZE);
    nT.trans_avio_out = avio_alloc_context(nT.avio_out_buffer,AVIO_SIZE,1,&g_nav_id,NULL,Trans_WriteBuffer,NULL);
    strcpy(nT.dest_format,format_name);
    g_nav_trans_map.insert(map<long,nav_Trans>::value_type(g_nav_id,nT));
     cout<<"初始化InitFFMpeg For Trans　完成"<<endl;
    return g_nav_id;
}

/*
 * Class:     Allegion_Hadoop_TransVideoSplitTool
 * Method:    SetTransVideoSplitReadCallBack
 * Signature: (JLjava/lang/Object;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_TransVideoSplitTool_SetTransVideoSplitReadCallBack(JNIEnv *env, jobject obj_self, jlong nvid, jobject obj, jstring funcName)
{
    TransMap::iterator item = g_nav_trans_map.find(nvid);
    if(item!=g_nav_trans_map.end())
    {
         //cout<<"SetTransVideoSplitReadCallBack"<<nvid<<endl;
        const char *signame="(J)J";//签名
        jclass recls = env->GetObjectClass(obj);
        const char *fun_name = env->GetStringUTFChars(funcName,0);
        item->second.trans_read_obj = env->NewGlobalRef(obj);
        item->second.trans_read_data_mid = env->GetMethodID(recls,fun_name,signame);
        item->second.ifmt_ctx->pb =  item->second.trans_avio_in;
        item->second.ifmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
        //env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,1111);
        //cout<<"SetTransVideoSplitReadCallBack  over"<<endl;
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_TransVideoSplitTool
 * Method:    SetTransVideoSplitWriteCallBack
 * Signature: (JLjava/lang/Object;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_TransVideoSplitTool_SetTransVideoSplitWriteCallBack(JNIEnv *env, jobject obj_self, jlong nvid, jobject obj, jstring funcName)
{
    TransMap::iterator item = g_nav_trans_map.find(nvid);
    if(item!=g_nav_trans_map.end())
    {
         //cout<<"SetTransVideoSplitReadCallBack"<<nvid<<endl;
        const char *signame="(J)J";//签名
        jclass recls = env->GetObjectClass(obj);
        const char *fun_name = env->GetStringUTFChars(funcName,0);
        item->second.trans_write_obj = env->NewGlobalRef(obj);
        item->second.trans_write_data_mid = env->GetMethodID(recls,fun_name,signame);
        item->second.ofmt_ctx->pb =  item->second.trans_avio_out;
        item->second.ofmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
        //env->CallLongMethod( item->second.split_read_obj,item->second.split_read_data_mid,1111);
       // cout<<"SetTransVideoSplitReadCallBack  over"<<endl;
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_TransVideoSplitTool
 * Method:    StartTrans
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_TransVideoSplitTool_StartTrans(JNIEnv *env, jobject obj, jlong nvid)
{
    TransMap::iterator item = g_nav_trans_map.find(nvid);
    if(item!=g_nav_trans_map.end())
    {
         cout<<"打开输入流"<<nvid<<endl;
        AVInputFormat   *piFmt = NULL;
        int ret = av_probe_input_buffer(item->second.trans_avio_in,&piFmt,"",NULL,0,0);
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
          cout<<"准备创建视频文件转码线程"<<endl;
         pthread_t       trans_thread;
         pthread_create(&trans_thread,NULL,TransThread,&( item->second));

         pthread_join(trans_thread,NULL);
        return 0;
    }
    return -1;
}

/*
 * Class:     Allegion_Hadoop_TransVideoSplitTool
 * Method:    CloseTrans
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_Allegion_Hadoop_TransVideoSplitTool_CloseTrans(JNIEnv * env, jobject obj, jlong nvid)
{
    TransMap::iterator item = g_nav_trans_map.find(nvid);
    if(item!=g_nav_trans_map.end())
    {
        avformat_close_input(&item->second.ifmt_ctx);
        avformat_close_input(&item->second.ofmt_ctx);
         cout<<"关闭转码"<<nvid<<endl;
        //av_free(item->second.avio_in_buffer);
        av_freep(item->second.avio_out_buffer);
        av_freep(item->second.trans_avio_in);
        av_freep(item->second.trans_avio_out);
        avformat_free_context(item->second.ifmt_ctx);
        avformat_free_context(item->second.ofmt_ctx);
        g_nav_trans_map.erase(item);
        return 0;
    }
    return -1;
}
int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index)
{
    int ret;
    int got_frame;
	AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
                CODEC_CAP_DELAY))
        return 0;
    av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
    while (1) {

        //ret = encode_write_frame(NULL, stream_index, &got_frame);
        enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame)
		{ret=0;break;}
		/* prepare packet for muxing */
		enc_pkt.stream_index = stream_index;
		enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts,
				fmt_ctx->streams[stream_index]->codec->time_base,
				fmt_ctx->streams[stream_index]->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts,
				fmt_ctx->streams[stream_index]->codec->time_base,
				fmt_ctx->streams[stream_index]->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		enc_pkt.duration = av_rescale_q(enc_pkt.duration,
				fmt_ctx->streams[stream_index]->codec->time_base,
				fmt_ctx->streams[stream_index]->time_base);
		av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
		/* mux encoded frame */
		ret = av_interleaved_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
            break;
    }
    return ret;
}
void  *TransThread(void *lp)
{
    JNIEnv      *env;
    g_vm->AttachCurrentThread((void **)&env,NULL);
    nav_Trans       * nT = (nav_Trans *)lp;
    AVCodecContext          *dec_ctx,*enc_ctx;
    AVCodec                          *video_encoder;
    AVCodec                          *video_decoder;
    AVPacket                           packet ,enc_pkt;
    AVFrame                             *pVideoFrame;
    int ret = 0;

    //格式复制
    for(int i = 0;i<nT->ifmt_ctx->nb_streams;i++)
    {
        AVStream *in_stream = nT->ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(nT->ofmt_ctx,NULL);
        dec_ctx = in_stream->codec;
        enc_ctx = out_stream->codec;

        if(dec_ctx->codec_type==AVMEDIA_TYPE_VIDEO)
        {
                video_decoder = avcodec_find_decoder(dec_ctx->codec_id);
                ret = avcodec_open2(dec_ctx,video_decoder,NULL);
                if(ret<0)
                {
                    cout<<"解码器打开失败"<<endl;
                    return NULL;
                }
                //分析目标格式字符传  目前先固定格式
                video_encoder = avcodec_find_encoder(AV_CODEC_ID_H264);//当前先固定为此编码格式
                enc_ctx->height = dec_ctx->height==0?240: dec_ctx->height;
                enc_ctx->width =  dec_ctx->width==0?320: dec_ctx->width;
                cout<<"视频解码器 "<< dec_ctx->height<<"   "<< dec_ctx->width<<endl;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
                enc_ctx->pix_fmt =video_encoder->pix_fmts[0];//设置像素格式 dec_ctx->pix_fmt;//
                enc_ctx->time_base=dec_ctx->time_base;//timebase=num/den
                enc_ctx->gop_size=30;//每隔多少帧插入一个I帧
                //h264的必备选项　，没有会出错
                enc_ctx->me_range = 16;
                enc_ctx->max_qdiff = 4;
                enc_ctx->qmin = 10;
                enc_ctx->qmax = 31;
                enc_ctx->qcompress = 0.6;
                enc_ctx->refs=2;//参考帧数目
                enc_ctx->bit_rate = 500000; //比特率
                enc_ctx->max_b_frames = 3;//两个非B帧直接所允许插入的B帧的最大帧数
                ret = avcodec_open2(enc_ctx,video_encoder,NULL);
                if(ret<0)
                {
                    cout<<"视频编码器打开失败"<<endl;
                    return NULL;
                }
        }
        else if(dec_ctx->codec_type==AVMEDIA_TYPE_AUDIO)
        {
              ret = avcodec_copy_context(out_stream->codec,in_stream->codec);
            if(ret<0)
            {
                cout<<"复制音频流类型失败"<<endl;
            }
            out_stream->time_base = in_stream->time_base;
            cout<<"输出流类型"<<out_stream->codec->codec_type<<endl;
        }
        else
        {
            ret = avcodec_copy_context(out_stream->codec,in_stream->codec);
            if(ret<0)
            {
                cout<<"复制流类型失败"<<endl;
            }
             cout<<"输出流类型"<<out_stream->codec->codec_type<<endl;
        }
        if(nT->ofmt_ctx->oformat->flags&AVFMT_GLOBALHEADER)
            out_stream->codec->flags|=CODEC_FLAG_GLOBAL_HEADER;

    }
    avio_open_dyn_buf(&nT->trans_avio_out);
    ret = avformat_write_header(nT->ofmt_ctx,NULL);
    int     Skipped_frame=0;
    int      video_stream_idx=0;
     pVideoFrame = av_frame_alloc();
     while( av_read_frame(nT->ifmt_ctx,&packet)>=0)
    {
        AVStream  *in_stream_t = nT->ifmt_ctx->streams[packet.stream_index];
        if(in_stream_t->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {

                int  got_frame;
                video_stream_idx = packet.stream_index;
                packet.dts = av_rescale_q_rnd(packet.dts,in_stream_t->time_base,in_stream_t->codec->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                packet.pts = av_rescale_q_rnd(packet.pts,in_stream_t->time_base,in_stream_t->codec->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                //cout<<"解码前时间　"<<packet.pts<<endl;
                ret = avcodec_decode_video2(in_stream_t->codec,pVideoFrame,&got_frame,&packet);
                if(ret<0)
                {
                    break;
                }
                if(got_frame)
                {
                    pVideoFrame->pts = packet.pts;//av_frame_get_best_effort_timestamp(pVideoFrame);
                    pVideoFrame->pict_type = AV_PICTURE_TYPE_NONE;
                    enc_pkt.data = NULL;
                    enc_pkt.size = 0;
                    av_init_packet(&enc_pkt);
                    int enc_got_frame;
                    //如果分辨率变化，这里要提前对图像进行缩放
                    //cout<<"编码前　"<<packet.pts<<"   "<<enc_ctx->time_base.den<<"  "<<enc_ctx->time_base.num<<endl;
                    ret = avcodec_encode_video2(nT->ofmt_ctx->streams[packet.stream_index]->codec,&enc_pkt,pVideoFrame,&enc_got_frame);
                    //cout<<"编码后  "<<enc_pkt.pts<<endl;
                    if(ret<0)
                    {
                        goto end;
                    }
                    if(!enc_got_frame)
                        continue;
                    enc_pkt.stream_index = packet.stream_index;
                    //cout<<"时间设置前"<<enc_pkt.pts<<" "<<enc_ctx->time_base.den<<"  "<<enc_ctx->time_base.num<<endl;
                    enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                    enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                    enc_pkt.duration = av_rescale_q(enc_pkt.duration,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base);
                    //cout<<"编码后 "<<enc_pkt.pts<<" "<<nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base.den<<"  "<<nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base.num<<endl;
                    av_interleaved_write_frame(nT->ofmt_ctx,&enc_pkt);
                    Skipped_frame=0;
                }
                else
                {
                    Skipped_frame++; //防止文件尾的数据丢失
                    //av_frame_free(&pVideoFrame);
                }
                for(int i=Skipped_frame;i>0;i--)
                {
                    ret = avcodec_decode_video2(in_stream_t->codec,pVideoFrame,&got_frame,&packet);
                    if(got_frame)
                    {
                           pVideoFrame->pts = packet.pts;
                            pVideoFrame->pict_type = AV_PICTURE_TYPE_NONE;
                            enc_pkt.data = NULL;
                            enc_pkt.size = 0;
                            av_init_packet(&enc_pkt);
                            int enc_got_frame;
                            ret = avcodec_encode_video2(nT->ofmt_ctx->streams[packet.stream_index]->codec,&enc_pkt,pVideoFrame,&enc_got_frame);
                            if(ret<0)
                            {
                                goto end;
                            }
                            if(!enc_got_frame)
                                continue;
                            enc_pkt.stream_index = video_stream_idx;
                            enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                            enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                            enc_pkt.duration = av_rescale_q(enc_pkt.duration,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base);
                            av_interleaved_write_frame(nT->ofmt_ctx,&enc_pkt);
                            Skipped_frame=0;
                        }
                    }
        }
        else
        {
            //cout<<"音频时间"<<packet.pts<<endl;
            //packet.dts = av_rescale_q_rnd(packet.dts,nT->ofmt_ctx->streams[packet.stream_index]->time_base,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            //packet.pts = av_rescale_q_rnd(packet.pts,nT->ofmt_ctx->streams[packet.stream_index]->time_base,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            //packet.duration = av_rescale_q(packet.duration,nT->ofmt_ctx->streams[packet.stream_index]->codec->time_base,nT->ofmt_ctx->streams[packet.stream_index]->time_base);
            //其他流数据　直接复制
            //cout<<"音频时间后"<<packet.pts<<endl;
            av_interleaved_write_frame(nT->ofmt_ctx,&packet);
        }
        av_free_packet(&packet);

    }
    av_frame_free(&pVideoFrame);
    avio_flush(nT->trans_avio_out);
    //flush encoder
      if ((nT->ofmt_ctx->streams[video_stream_idx]->codec->codec->capabilities &CODEC_CAP_DELAY))
      {
             while (1)
            {
                    enc_pkt.data = NULL;
                    enc_pkt.size = 0;
                    av_init_packet(&enc_pkt);
                    int got_frame;
                    ret = avcodec_encode_video2 (nT->ofmt_ctx->streams[video_stream_idx]->codec, &enc_pkt,NULL, &got_frame);
                    av_frame_free(NULL);
                    if (ret>=0)
                    {
                        if (got_frame)
                        {
                            enc_pkt.stream_index = video_stream_idx;
                            enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts,
                                    nT->ofmt_ctx->streams[video_stream_idx]->codec->time_base,
                                    nT->ofmt_ctx->streams[video_stream_idx]->time_base,
                                    (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                            enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts,
                                    nT->ofmt_ctx->streams[video_stream_idx]->codec->time_base,
                                    nT->ofmt_ctx->streams[video_stream_idx]->time_base,
                                    (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                            enc_pkt.duration = av_rescale_q(enc_pkt.duration,
                                    nT->ofmt_ctx->streams[video_stream_idx]->codec->time_base,
                                    nT->ofmt_ctx->streams[video_stream_idx]->time_base);
                            //cout<<"enc pts"<<enc_pkt.pts<<" enc dts "<<enc_pkt.dts <<endl;
                            ret = av_interleaved_write_frame(nT->ofmt_ctx, &enc_pkt);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else{break;}
            }
      }
    //flush_encoder(nT->ofmt_ctx,video_stream_idx);
    ret = av_write_trailer(nT->ofmt_ctx);
    avio_close_dyn_buf(nT->trans_avio_out,&nT->avio_out_buffer);
    return NULL;
end:
    av_free_packet(&packet);
    av_frame_free(&pVideoFrame);
    fcloseall();
    return NULL;
}
int    Trans_ReadBuffer(void *opaque,uint8_t *buf,int buf_size)
{
    JNIEnv      *env ;
    long       nvid = *((long *)opaque);
    g_vm->AttachCurrentThread((void **)&env,NULL);
    int  ret = -1;
    TransMap::iterator item = g_nav_trans_map.find(nvid);
    if(item!=g_nav_trans_map.end())
    {
        //cout<<"转码ffmpeg读取回调  准备读取"<<buf_size<<"大小的数据"<<endl;
        env->CallLongMethod( item->second.trans_read_obj,item->second.trans_read_data_mid,buf_size);
        jclass recls =env->GetObjectClass( item->second.trans_read_obj);
        jfieldID fid_len=   env->GetFieldID(recls,"readbuflen","J");
        jlong len  =env->GetLongField( item->second.trans_read_obj,fid_len);
        //cout<<"收到"<<len<<"byte数据"<<endl;
        jfieldID fid_buf = env->GetFieldID(recls,"readBuf","[B");
        jbyteArray  read_byteArray=  (jbyteArray) env->GetObjectField( item->second.trans_read_obj,fid_buf);
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
int    Trans_WriteBuffer(void *opaque,uint8_t *buf,int buf_size)
{
  JNIEnv      *env = GetEnv();
    int           ret = 0;
    long       nvid = *((long *)opaque);
    //cout<<"ffmpeg写入回调  准备写入"<<buf_size<<"大小的数据"<<endl;
    g_vm->AttachCurrentThread((void **)&env,NULL);
    TransMap::iterator item = g_nav_trans_map.find(nvid);
    if(item!=g_nav_trans_map.end())
    {
         //cout<<"ffmpeg写入回调  准备写入"<<buf_size<<"大小的数据"<<endl;
         jclass recls =env->GetObjectClass( item->second.trans_write_obj);
         jfieldID fid_len=   env->GetFieldID(recls,"writebuflen","J");
         jlong len = buf_size;
         env->SetLongField(item->second.trans_write_obj,fid_len,len);
         jfieldID fid_buf=   env->GetFieldID(recls,"writeBuf","[B");
         jbyteArray  write_byteArray=  (jbyteArray) env->GetObjectField( item->second.trans_write_obj,fid_buf);
         jbyte * byte_arr =  env->GetByteArrayElements(write_byteArray,NULL);
         jlong arr_len = env->GetArrayLength(write_byteArray);
         if(arr_len>=buf_size)
         {
             memcpy(byte_arr,buf,buf_size);
             ret = buf_size;
         }
        env->ReleaseByteArrayElements(write_byteArray,byte_arr,0);
        env->CallLongMethod( item->second.trans_write_obj,item->second.trans_write_data_mid,ret);
        if(ret<AVIO_SIZE)
        {
             cout<<"结束写入文件"<<endl;
            env->CallLongMethod( item->second.trans_write_obj,item->second.trans_write_data_mid,0);
        }
    }
    g_vm->DetachCurrentThread();
    return ret;
}

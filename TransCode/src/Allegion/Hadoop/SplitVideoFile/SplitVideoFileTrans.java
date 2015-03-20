package Allegion.Hadoop.SplitVideoFile;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.InputFormat;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.LineRecordReader;
import org.apache.hadoop.mapred.RecordReader;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.fs.BlockLocation;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.PathFilter;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapreduce.security.TokenCache;
import org.apache.hadoop.util.ReflectionUtils;
import org.apache.hadoop.util.StringUtils;

import Allegion.Hadoop.TransVideoSplitTool;;
public class SplitVideoFileTrans {
	public long nvid;
	public ArrayList<String>			DestList;
	public TransVideoSplitTool					tvst;
	private FSDataInputStream			curInput;
	private FSDataOutputStream		curOutput;
	private byte[]  readBuf = new byte[32*1024];
	private byte[]  writeBuf = new byte[32*1024];
	private long   readbuflen = 0;
	private long 	writebuflen = 0;

	public void Trans(String destformat,String FilePath) throws IOException 
	{
		tvst = new TransVideoSplitTool();
		nvid = tvst.InitFFmpeg(destformat);
		tvst.SetTransVideoSplitReadCallBack(nvid,this, "PreReadCallBack");
		tvst.SetTransVideoSplitWriteCallBack(nvid, this,"PreWriteCallBack");
		
		Configuration hdfsconf = new Configuration();
		hdfsconf.set("fs.defaultFS", "hdfs://10.192.165.170:9000");
		Path  path = new Path(FilePath);
		FileSystem fs = path.getFileSystem(hdfsconf);
		curInput = fs.open(path);
		Path  Outputpath = new Path(FilePath+".dst");
		curOutput = fs.create(Outputpath,true,32768);
		tvst.StartTrans(nvid);
		tvst.CloseTrans(nvid);
		curInput.close();
		//删除原始文件
		fs.delete(path, true);
	}
	public long PreReadCallBack(long len) throws IOException
	{
		readbuflen = 0;
		long ret = curInput.read(readBuf, 0, (int)len);
		readbuflen = ret;
		if(len>ret||ret<0)
		{
			if(ret<0)
			{
				System.out.println("文件结束");
				
			}
			else
			{
				System.out.println("遇到文件尾,文件结束"+ret);
			}				
			//文件结束通知分片等待线程
		}
		return 0;
	}
	public long PreWriteCallBack(long len)throws IOException
	{
		if(len>0)
		{
			curOutput.write(writeBuf,0,(int)writebuflen);
		}
		else
		{
			System.out.println("java-----------触发写入回调函数,文件结束");
			curOutput.close();
		}			
		return 0;
	}
		
}

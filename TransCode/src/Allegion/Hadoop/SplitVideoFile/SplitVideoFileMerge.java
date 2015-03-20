package Allegion.Hadoop.SplitVideoFile;
import java.io.DataOutputStream;
import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

import Allegion.Hadoop.MergeVideoSplitTool;
public class SplitVideoFileMerge {
	
	private MergeVideoSplitTool merge_tool;
	private byte[]  readBuf = new byte[32*1024];
	private byte[]  writeBuf = new byte[32*1024];
	private long   readbuflen = 0;
	private long 	writebuflen = 0;
	private DataOutputStream video_out;
	private FSDataInputStream			curInput;
	private long nvid;
	private boolean MergeStart = false;
	
	public void StartMerge(DataOutputStream out)
	{
		merge_tool = new MergeVideoSplitTool();
		nvid = merge_tool.InitFFmpeg();
		merge_tool.SetMergeWriteCallBack(nvid, this,"PreWriteCallBack");
		merge_tool.SetMergeReadCallBack(nvid,this, "PreReadCallBack");
		video_out = out;
		
	}
	public void StopMerge()throws IOException 
	{
		merge_tool.StopMerge(nvid);
		video_out.close();
	}
	public void Merge(String HdfsFile)throws IOException 
	{
		
		Configuration hdfsconf = new Configuration();
		hdfsconf.set("fs.defaultFS", "hdfs://10.192.165.170:9000");
		Path  path = new Path(HdfsFile);
		FileSystem fs = path.getFileSystem(hdfsconf);
		curInput = fs.open(path);
		if(MergeStart==false)
		{
			MergeStart = true;
			merge_tool.StartMerge(nvid);
		}
		else
		{
			merge_tool.HaveNewSplit(nvid);	
		}
		curInput.close();
		fs.delete(path,true);
	}
	public long PreReadCallBack(long len) throws IOException
	{
		readbuflen = 0;
		long ret  = -1;
		if(curInput!=null)
		{
			ret = curInput.read(readBuf, 0, (int)len);
			readbuflen = ret;
			if(len>ret||ret<0)
			{
				if(ret<0)
				{
					System.out.println("文件结束");
				}
				else
				{
					//不一定是文件尾
					System.out.println("可能遇到文件尾"+len);
				}				
				//文件结束通知分片等待线程
			}
		}
		return ret;
	}
	public long PreWriteCallBack(long len)throws IOException
	{
		if(len>0)
		{
			video_out.write(writeBuf,0,(int)writebuflen);
		}		
		return 0;
	}
	

}

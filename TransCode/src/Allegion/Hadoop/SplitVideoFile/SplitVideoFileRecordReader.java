package Allegion.Hadoop.SplitVideoFile;
import java.io.IOException;

import org.apache.hadoop.mapred.RecordReader;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.mapred.InputSplit;
import org.apache.hadoop.mapred.FileSplit;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.io.BytesWritable;

//将读入到map的数据拆分成<key,value>对
public class SplitVideoFileRecordReader  implements RecordReader<LongWritable,BytesWritable>{

	private long start;
	private long pos;
	private long end;
	private byte[] buffer;
	private FSDataInputStream fileIn;
	private LongWritable key = null;
	private BytesWritable value= null;
	private long split_length;
	private int  PerReadLen = 102400;
	//构造函数
	SplitVideoFileRecordReader(InputSplit Insplit,JobConf job) throws IOException
	{
		//初始化，文件定位，内存分配
		FileSplit	split = (FileSplit)Insplit;
		start = split.getStart();
		end = split.getLength()+start;
		
		final Path path = split.getPath();
		System.out.print("split path:"+path.toString());
		final FileSystem fs = path.getFileSystem(job);
		fileIn = fs.open(path);
		fileIn.seek(start);
		System.out.println("len="+split.getLength()+"start="+start+"end="+end);
		buffer = new byte[PerReadLen];
		this.pos = start;
	
	}
	 public  boolean next(LongWritable key, BytesWritable  value) throws IOException
	 {
		 //System.out.println("Next key/value for Map func");
		 
		 //是在这里进行解码 还是在map里面
		 if(key==null)
		 {
			 key  = new LongWritable();
		 }
		 if(value==null)
		 {
			 value= new BytesWritable();
		 }
		 key.set(pos);
		//System.out.println("当前读取位置"+pos+" 数据总长度"+end);
		 while(pos+PerReadLen<=end)
		 {
			 //如果是音视频数据 就要在这里分析 每次读取多少数据了
			fileIn.seek(pos);
			 fileIn.readFully(pos, buffer, 0, PerReadLen);
			 value.set(buffer, 0, PerReadLen);
			 pos+=PerReadLen;
			 return true;
		 }
		 if(pos<end)
		 {
			 fileIn.seek(pos);
			 fileIn.readFully(pos, buffer, 0, (int)(end-pos));
			 value.set(buffer,0,(int)(end-pos));
			 pos+=PerReadLen;
			 return true;
		 }
		 return false;
	 }
	 public  LongWritable  createKey()
	 {
		 System.out.println("调用 CreateKey");
		 if(key==null)
		 {
			 key = new LongWritable();
			 //key.set(start/(end-start));//使用偏移量作为key
		 }		 
		 return key;
	 }
	  public BytesWritable  createValue()
	  {
		  System.out.println("调用 createValue");
		  if(value==null)
		  {
			  value = new BytesWritable();
		  }
		  return value;
	  }
	  public   long getPos() throws IOException
	  {
		  System.out.println("调用 getPos");
		  return this.pos;
	  }
	  public void close() throws IOException
	  {
		  System.out.println("调用 close");
		  if(fileIn!=null)
		  {
			  fileIn.close();
		  }
	  }
	  //  @return progress from <code>0.0</code> to <code>1.0</code>.
	  public float getProgress() throws IOException
	  {
		  //System.out.println("调用 getProgress");
		  if(start==end)
		  {
			  return 0.0f;
		  }
		  else
		  {
			  float process =  Math.min(1.0f,(pos-start)/(float)(end-start));
			  System.out.println("调用 getProgress 当前MapTask的进度"+process*100+"%");
			  return process;
		  }
	  }
}

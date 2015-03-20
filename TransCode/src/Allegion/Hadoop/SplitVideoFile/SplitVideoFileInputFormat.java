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
import org.apache.hadoop.net.NetworkTopology;

import com.google.common.base.Charsets;

import Allegion.Hadoop.SplitVideoFileTool;



//视频已经提前切分完成，现在要做的就是每个文件对应一个split
public class SplitVideoFileInputFormat  implements  InputFormat<LongWritable, Text>{
	
	// private  long splitSize  =102400; //Split块大小  每次读取的大小
	 //private final float SPLIT_SLOP =(float)1.1;
	 private FSDataInputStream		curInput;
	 private FSDataOutputStream		curOutput;
	 private long	curNvid;
	 private byte[] readBuf = new byte[64*1024];
	 private long   readbuflen = 0;
	 private byte[] writeBuf = new byte[64*1024];
	 private long 	writebuflen = 0;
	 private ArrayList<String>		SplitFileList;
	 private JobConf						conf;
	 private String							MidPathStr;
	  //隐藏文件过滤器
		private static final PathFilter hiddenFilter = new PathFilter()
		{
			public boolean accept(Path p){
				String name = p.getName();
				return !name.startsWith("_")&&!name.startsWith(".");
			}
		};
		//多条件过滤器
		private static class MultiPathFilter implements PathFilter
		{
			private List<PathFilter> filters;
			public MultiPathFilter(List<PathFilter> filters){
				this.filters = filters;
			}
			public boolean accept(Path path)
			{
				for(PathFilter filter:filters)
				{
					if(!filter.accept(path))
					{
						return false;
					}
				}
				return true;
			}
		}
		
		//标记所有的输入数据，然后将他们切分为小的输入数据块(逻辑上切分，标记处理文件的偏移量，实际上不切分),每个Map任务处理一个数据块
		public InputSplit[] getSplits(JobConf job,int numSplits) throws IOException
		{	
			FileStatus[] files = listStatus(job); 
			conf = job;
			MidPathStr=job.get("mapreduce.splitvideofile.midPath","");
			ArrayList<FileSplit> splits = new ArrayList<FileSplit>();
			SplitFileList = new ArrayList<String>();
			for(FileStatus file:files)
			{
				if(file.isDirectory())
				{
					System.out.println("is Dir");
					continue;
				}
				Path  path = file.getPath();
				FileSystem fs = path.getFileSystem(job);
				long length = file.getLen(); //文件长度
				System.out.println("文件:"+path.toString()+" 长度为"+length);
				curInput = fs.open(path);
				SplitVideoFileTool		Svft = new SplitVideoFileTool();
				long nvid = Svft.InitFFmpeg("h264");
				//音视频文件不能只对文件进行逻辑上切分　　需要对文件进行物理切分　　大文件要生成小文件　使用结束后再进行删除				
				Svft.SetSplitReadCallBack(nvid, this,"PreReadCallBack");
				Svft.SetSplitWriteCallBack(nvid, this,"PreWriteCallBack");
				Svft.SetSplitNewSplitCallBack(nvid, this,"NewSplitCallBack");
				Svft.StartSplit(nvid);//会阻塞　直到切分完成
				Svft.CloseSplit(nvid);
				//wait
				curInput.close();
				//fs.close();//这个不能随便关闭，所有线程都用的是一个FileSystem 中cache
				
			}	
			Path task = new Path(MidPathStr+"SplitTransTask.m3u8");
			FileSystem fs = task.getFileSystem(job);
			FSDataOutputStream taskStream = fs.create(task, true,1024);
			
			for(String item:SplitFileList )
			{
				//创建任务列表
				taskStream.writeBytes(item+"\n");	
			}
			taskStream.close();
			FSDataInputStream			jobfile;
			jobfile =fs.open(task);
			FileStatus[]   taskStatus = fs.listStatus(task);
			long taskfilelen = taskStatus[0].getLen();
			BlockLocation[] blklocaltion =fs.getFileBlockLocations(taskStatus[0], 0, taskfilelen);
			int offset = 0;
			while(true)
			{
				String maptask = jobfile.readLine();
				if(maptask==null)
				{
					break;
				}
				//System.out.println("maptask"+maptask);
				//System.out.println("splits 偏移量"+offset+"长度"+maptask.length());
				splits.add(new FileSplit(task,offset,maptask.length(),blklocaltion[0].getHosts()));
				offset+=maptask.length();
			}
			jobfile.close();
			return splits.toArray(new FileSplit[splits.size()]);
		}
		 
		public long PreReadCallBack(long len) throws IOException
		{
			//System.out.println("java-----------触发读取回调函数"+len);
			readbuflen = 0;
			long ret  = 0;
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
						//System.out.println("可能遇到文件尾"+len);
					}				
					//文件结束通知分片等待线程
				}
			}
			return ret;
		}
		public long PreWriteCallBack(long len)throws IOException
		{
			
			//System.out.println("java-----------触发写入回调函数"+len+"  "+writebuflen);
			if(len>0)
			{
				curOutput.write(writeBuf,0,(int)writebuflen);
			}
			else
			{
				//System.out.println("java-----------触发写入回调函数,文件结束");
				curOutput.close();
			}			
			return 0;
		}
		
		public long NewSplitCallBack(String SplitName)throws IOException
		{			
			Path file = new Path(MidPathStr+SplitName);
			if(curOutput!=null)
			{
				curOutput.close();
				curOutput = null;
			}
			//System.out.println("java-----------触发新建分片回调函数"+SplitName);
			FileSystem fs = file.getFileSystem(conf);
			curOutput = fs.create(file, true,32768);
			SplitFileList.add(file.toString());
			
			return 0;
		}
		
	    //提供一个RecordReader来从给定的数据块中迭代处理数据，然后将数据处理为<key, value>格式
		public RecordReader<LongWritable,Text> getRecordReader(InputSplit split,JobConf job,Reporter reporter)throws IOException
		{
			   reporter.setStatus(split.toString());
			    String delimiter = job.get("textinputformat.record.delimiter");
			    byte[] recordDelimiterBytes = null;
			    if (null != delimiter) {
			      recordDelimiterBytes = delimiter.getBytes(Charsets.UTF_8);
			    }
			    return new LineRecordReader(job, (FileSplit) split,recordDelimiterBytes);
		}
		protected long computeSplitSize(long goalSize, long minSize, long blockSize)
		{
			return Math.max(minSize, Math.min(goalSize, blockSize));
		}
		private int  getBlockIndex(BlockLocation[] blkLocation,long offset)
		{
			int i = 0;
			for( i=0;i<blkLocation.length;i++)
			{
				if((blkLocation[i].getOffset()<=offset)&&(offset<blkLocation[i].getOffset()+blkLocation[i].getLength()))
				{
					return i;
				}
			}
			BlockLocation last = blkLocation[blkLocation.length-1];
			long fileLength = last.getOffset()+last.getLength()-1;
			throw new IllegalArgumentException("offfset"+offset+"is outsize fiile(0..."+fileLength+")");
		}
		//获取输入路径下的所有文件状态
		private FileStatus[] listStatus(JobConf  job) throws IOException
		{
			Path[] dirs = getInputPath(job);
			if(dirs.length==0)
				throw new IOException("No Input File");
			//这是权限验证？
			TokenCache.obtainTokensForNamenodes(job.getCredentials(),dirs, job);
			List<FileStatus> result = new ArrayList<FileStatus>();
			List<IOException> errors = new ArrayList<IOException>();
			
			List<PathFilter> filters = new ArrayList<PathFilter>();
			filters.add(hiddenFilter);
			PathFilter jobFilter = getInputPathFilter(job);
			if(jobFilter!=null)
			{
				filters.add(jobFilter);
			}
			PathFilter inputFilter = new MultiPathFilter(filters);
			for(Path p:dirs)
			{
				FileSystem fs = p.getFileSystem(job);
				FileStatus[] matches = fs.globStatus(p,inputFilter);
				if(matches==null)
				{
					errors.add(new IOException("match=null error"));					
				}
				else if(matches.length==0)
				{
					errors.add(new IOException("matches.length==0 error"));
				}
				else
				{
					for(FileStatus globStat:matches)
					{
						if(globStat.isDirectory())
						{
							//只能访问一级目录
							for(FileStatus stat:fs.listStatus(globStat.getPath(),inputFilter))
							{
								result.add(stat);
							}
						}
						else
						{
							result.add(globStat);
						}
					}
				}
			}
			return result.toArray(new FileStatus[result.size()]);
		}
		
		private PathFilter getInputPathFilter(JobConf job)
		{
			Configuration conf = job;
			Class<?> filterClass = conf.getClass("mapred.input"+".pathFilter.class",null,PathFilter.class);
			return (filterClass!=null)?(PathFilter)ReflectionUtils.newInstance(filterClass, conf):null;
			
		}
		//获取出入路径的   如果参数是文件，这样写就不对了
		private Path[] getInputPath(JobConf job)
		{
			String dirs = job.get("mapreduce.splitvideofile.input","");
			String[] list = StringUtils.split(dirs);
			Path[] result = new Path[list.length];
			for(int i=0;i<list.length;i++)
				result[i]= new Path(StringUtils.unEscapeString(list[i]));
			return result;
			
		}
}


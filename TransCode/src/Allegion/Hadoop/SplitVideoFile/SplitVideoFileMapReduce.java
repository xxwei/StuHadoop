package Allegion.Hadoop.SplitVideoFile;

import java.io.IOException;
import java.util.Iterator;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapreduce.filecache.DistributedCache;
import org.apache.hadoop.util.GenericOptionsParser;


public class SplitVideoFileMapReduce {

	public static Configuration hdfsconf ;
	public static class SplitTransMap extends MapReduceBase implements Mapper<LongWritable,Text,LongWritable, Text>
	{

		public  void map(LongWritable key,Text  value,OutputCollector<LongWritable,Text> output,Reporter reporter) throws IOException
		{		
				SplitVideoFileTrans svft = new SplitVideoFileTrans();
				System.out.println("开始转码"+value.toString());
				svft.Trans("codec=h264&size=320*240&fps=15", value.toString());
				
				Text  newTxt = new Text(value.toString()+".dst");
				System.out.println("转码完成"+value.toString());
				output.collect(key, newTxt);
		}
	}
	public static class SplitTransReduce extends MapReduceBase implements Reducer<LongWritable,Text,LongWritable,Text>
	{
		public void reduce(LongWritable key,Iterator<Text> values,OutputCollector<LongWritable,Text> output,Reporter reporter) throws IOException
		{
			//同一个key进行合并，这里应该啥也不用做
			Text pa=new Text("Nothing");
			while(values.hasNext())
			{
				 pa = values.next();
			}	
			System.out.println("准备合并"+pa.toString());
			output.collect(key,pa);
		}
	}
	/**
	 * @param args
	 */
	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub
		
		hdfsconf = new Configuration();
		hdfsconf.set("fs.defaultFS", "hdfs://10.192.165.170:9000");
		long timestart = System.currentTimeMillis();
		FileSystem hdfs = FileSystem.get(hdfsconf);
		long timeover = System.currentTimeMillis();
		System.out.println("连接远程HDFS耗时"+(timeover-timestart));
		Path outputPath = new Path("/SplitVideo");
		if(hdfs.exists(outputPath))
		{
			System.out.println("删除输出目录");
		}
		//删除文件
		hdfs.delete(outputPath,true);
		//hdfs.close();
		
		
		JobConf config = new JobConf(SplitVideoFileMapReduce.class);
		GenericOptionsParser goparser = new GenericOptionsParser(config,args);
		String otherargs[] = goparser.getRemainingArgs();
		
		config.setJarByClass(SplitVideoFileMapReduce.class);
		config.setJobName("AVTransCode");
		config.setInputFormat(SplitVideoFileInputFormat.class);
		config.setOutputFormat(SplitVideoFileOutputFormat.class);
		config.setOutputKeyClass(LongWritable.class);   
		config.setOutputValueClass(Text.class); 
		config.setMapperClass(SplitTransMap.class);
		
		 //config.setCombinerClass(LocalTaskReduce.class); // 本质是一个本地reducer 设计初衷是本地将需要reduce操作的数据进行合并
		config.setReducerClass(SplitTransReduce.class);  
		config.set("mapreduce.splitvideofile.input", args[0]);
		config.set("mapreduce.splitvideofile.midPath", "hdfs://10.192.165.170:9000/SplitVideo/");
		//必须得设置一个输出目录
		FileOutputFormat.setOutputPath(config, new Path("hdfs://10.192.165.170:9000/SplitVideo"));
		//DistributedCache.addCacheFile(("hdfs://10.192.165.170:9000/lib/TransCode.jar", config);
		JobClient.runJob(config); 
	}

}

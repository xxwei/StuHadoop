package Allegion.Hadoop.SplitVideoFile;

import java.io.IOException;

import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.OutputFormat;
import org.apache.hadoop.mapred.RecordWriter;
import org.apache.hadoop.util.Progressable;


public class SplitVideoFileOutputFormat  implements OutputFormat<LongWritable,Text> { 

	
	public RecordWriter<LongWritable, Text> getRecordWriter(FileSystem ignored, JobConf job,   String name, Progressable progress)throws IOException
	{
		 System.out.println("getRecordWriter ...........");
		 Path file = FileOutputFormat.getTaskOutputPath(job, name);
	     FileSystem fs = file.getFileSystem(job);
	     FSDataOutputStream fileOut = fs.create(file, progress);
	     return new SplitVideoFileRecordWriter(fileOut);
	}
	public void checkOutputSpecs(FileSystem ignored, JobConf job) throws IOException
	{
		
		System.out.println("检查输出空间checkOutputSpecs ...........");
	}
}
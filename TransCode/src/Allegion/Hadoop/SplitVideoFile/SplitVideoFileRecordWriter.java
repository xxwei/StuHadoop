package Allegion.Hadoop.SplitVideoFile;

import java.io.DataOutputStream;
import java.io.IOException;

import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.RecordWriter;
import org.apache.hadoop.mapred.Reporter;


public class SplitVideoFileRecordWriter  implements RecordWriter<LongWritable,Text> {


	private SplitVideoFileMerge		Svfm;
	
	public SplitVideoFileRecordWriter (DataOutputStream out, String keyValueSeparator)
	{
		
	}
	public SplitVideoFileRecordWriter (DataOutputStream out)
	{
		Svfm = new SplitVideoFileMerge();
		Svfm.StartMerge(out);
	}
	public void write(LongWritable  key, Text value) throws IOException
	{
		System.out.println("开始合并文件"+value);
		Svfm.Merge(value.toString());
	}
	 public void close(Reporter reporter) throws IOException
	 {
		 Svfm.StopMerge();
	 }
}

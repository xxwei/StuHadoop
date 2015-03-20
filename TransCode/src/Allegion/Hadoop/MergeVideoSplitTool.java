package Allegion.Hadoop;
//bin目录下 执行
	//javah -classpath ./  -d ../jni   Allegion.Hadoop.MergeVideoSplitTool
public class MergeVideoSplitTool {
	public native long InitFFmpeg();
	public native long SetMergeReadCallBack(long nvid,Object obj,String FuncName);
	public native long SetMergeWriteCallBack(long nvid,Object obj,String FuncName);
	public native long HaveNewSplit(long nvid);
	public native long StartMerge(long nvid);
	public native long StopMerge(long nvid);
	static
	{
		//System.load("/home/hadoop/workspace/HelloNavite/bin/Release/libHelloNavite.so");
		System.load("/usr/local/lib/libTransCode.so");
	}
}

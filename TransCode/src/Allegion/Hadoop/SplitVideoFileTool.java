package Allegion.Hadoop;
//bin目录下 执行
	//javah -classpath ./  -d ../jni   Allegion.Hadoop.SplitVideoFileTool
public class SplitVideoFileTool {
	public native long InitFFmpeg(String FormatName);
	public native long SetSplitReadCallBack(long nvid,Object obj,String FuncName);
	public native long SetSplitWriteCallBack(long nvid,Object obj,String FuncName);
	public native long SetSplitNewSplitCallBack(long nvid,Object obj,String FuncName);
	public native long StartSplit(long nvid);
	public native long CloseSplit(long nvid);
	static
	{
		//System.load("/home/hadoop/workspace/HelloNavite/bin/Release/libHelloNavite.so");
		System.load("/usr/local/lib/libTransCode.so");
	}
}

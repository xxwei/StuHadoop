package Allegion.Hadoop;
//bin目录下 执行
	//javah -classpath ./  -d ../jni   Allegion.Hadoop.TransVideoSplitTool
public class TransVideoSplitTool {
	public native long InitFFmpeg(String Format);
	public native long SetTransVideoSplitReadCallBack(long nvid,Object obj,String FuncName);
	public native long SetTransVideoSplitWriteCallBack(long nvid,Object obj,String FuncName);
	public native long StartTrans(long nvid);
	public native long CloseTrans(long nvid);
	static
	{
		//System.load("/home/hadoop/workspace/HelloNavite/bin/Release/libHelloNavite.so");
		System.load("/usr/local/lib/libTransCode.so");
	}
}

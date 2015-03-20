// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.

#include"UtilTool.h"



long                    g_nav_id = 0;
JavaVM              *g_vm;
SplitMap            g_nav_split_map;
TransMap          g_nav_trans_map;
MergeMap        g_nav_merge_map;

/*
JNI签名规则
boolean    Z
byte            B
char            C
long            J
float           F
double      D
short           S
int                 I
类               L全限定类名
数组          [元素类型签名
全限定类名以"/"分隔　括号外面为返回值类型签名
例如
long fun(int n,String str,int[] arr)
(ILjava/lang/String;[I)J
*/


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    cout<<"加载虚拟机"<<endl;
    g_vm = vm;
    return JNI_VERSION_1_6;
}
JNIEnv *GetEnv()
{
    JNIEnv * env;
    g_vm->GetEnv((void **)&env,JNI_VERSION_1_6);
    return env;
}



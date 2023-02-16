#include "PuppyMemory.hpp"


struct MemoryHack{

PuppyMemory Dog1;

}PatchTest;

int main()
{
	PatchTest.Dog1.Init("tv.danmaku.bili");
	//修改	
    PatchTest.Dog1.Patch("libbiliid.so",0x0,"01 00 A0 E3 1E FF 2F E1");    
    sleep(3);  
    
    //还原
    PatchTest.Dog1.Restore();    
	return 0;
}

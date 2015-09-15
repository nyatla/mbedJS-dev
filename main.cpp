#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "mimic.h"
#include "fsdata.h"
#include "rpctbl.h"
/**
 * local filesystem support.
 * MiMic::LocalFileSystem2 do not freeze on LPCXpresso.
 */
LocalFileSystem2 lf("local");

/**
 * initialization
 */
#if defined(TARGET_LPC1768) || defined(TARGET_LPC4088)
void pf_init()
{
}
SDFileSystem sd(p5, p6, p7, p8,"sd");
#elif defined(TARGET_K64F)
void pf_init()
{
    DigitalOut led_r(LED1);
    DigitalOut led_g(LED2);
    DigitalOut led_b(LED3);
    led_r=1;
    led_b=1;
    led_g=1;
}
#endif




/**
 * MiMic RemoteMCU httpd.<br/>
 * <p>Service list</p>
 * <pre>
 * /rom/ - romfs
 * /setup/ - MiMic configulation REST API.
 * /local/ - mbed LocalFileSystem
 * /mvm/   - MiMicVM REST API
 * </pre>
 */
class MiMicRemoteMcu:public MiMic::Httpd
{
private:
    ModRomFiles modromfs; //ROM file module
    ModMiMicSetting mimicsetting; //mimic setting API
    ModRemoteMcu remotemcu; // remotemcu API
    ModLocalFileSystem modlocal; //FileSystem mounter
    ModLocalFileSystem modsd; //FileSystem mounter
    ModFileIo modfio;   //fileupload API
    ModUPnPDevice modupnp;
    ModJsonRpc modrpc;
public:
    MiMicRemoteMcu(Net& i_net,NetConfig& i_cfg):Httpd(i_cfg.getHttpPort())
    {
        this->modromfs.setParam("rom",RMCU_FSDATA,18);
        this->mimicsetting.setParam("setup");
        this->remotemcu.setParam("mvm");
        this->modlocal.setParam("local");
        this->modsd.setParam("sd",ModLocalFileSystem::FST_SDFATFS);
        this->modfio.setParam("fio");
        this->modupnp.setParam(i_net);
        this->modrpc.setParam("rpc",RPCTBL);
    }
    /**
     * Http handler
     */
    virtual void onRequest(HttpdConnection& i_connection)
    {
        //pause persistent mode if websocket ready.
        if(this->modrpc.isStarted()){
            i_connection.breakPersistentConnection();
        }
        //try to ModRomFS module.
        if(this->modromfs.execute(i_connection)){
            return;
        }
        //try to ModMiMicSetting module.
        if(this->mimicsetting.execute(i_connection)){
            return;
        }
        //try to ModRemoteMcu module.
        if(this->remotemcu.execute(i_connection)){
            return;
        }
        //try to ModLocalFileSystem
        if(this->modlocal.execute(i_connection)){
            return;
        }
        //try to ModLocalFileSystem(SD)
        if(this->modsd.execute(i_connection)){
            return;
        }
        //try to FileUpload
        if(this->modfio.execute(i_connection)){
            return;
        }
        //try to UPnP
        if(this->modupnp.execute(i_connection)){
            return;
        }
        if(this->modrpc.execute(i_connection)){
            this->modrpc.dispatchRpc();
            return;
        }
        
        //Otherwise, Send the redirect response to /rom/index.html
        i_connection.sendHeader(302,
            "text/html",
            "Status: 302:Moved Temporarily\r\n"
            "Location: /rom/index.html\r\n");        
        return;
    }
};

/*
extern "C" void setLed(int i){
    switch(i){
    case 0:{DigitalOut led_r(p23);led_r=0;}break;
    case 1:{DigitalOut led_g(p24);led_g=0;}break;
    case 2:{DigitalOut led_b(p25);led_b=0;}break;
    }
}*/


MiMicRemoteMcu* httpd; //create a httpd instance.
char friendly_name[48];//32(HostNameMAX)+16で十分(mbedJSの場合)
int main()
{   
    MiMicNetIf netif;
    Net net(netif);//Net constructor must be created after started RTOS
    NetConfig cfg; //create network configulation  with onchip-setting.
    pf_init();
    //Prepare configulation.
    cfg.setUPnPIcon(64,64,8,"image/png","/rom/icon.png");
    cfg.setUPnPUdn(0xe29f7101,0x4ba2,0x01e0,0);
    cfg.setUPnPPresentationURL("/rom/index.html");
    //try to override setting by local file.
    if(!cfg.loadFromFile("/local/mimic.cfg")){
        Thread::wait(2000);//wait for SD card initialization.
        cfg.loadFromFile("/sd/mimic.cfg");
    }
    //FriendlyNameの上書き
    strcpy(friendly_name,"mbedJS-");
    strcpy(friendly_name+7,cfg.getHostName());
    cfg.setFriendlyName(friendly_name);

    
    httpd=new MiMicRemoteMcu(net,cfg); //create a httpd instance.
    net.start(cfg);
    httpd->loop();  //start httpd loop.
    return 0;
}

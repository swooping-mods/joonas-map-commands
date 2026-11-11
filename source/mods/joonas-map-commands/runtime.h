/* Each feature owns a copy: there are no runtime feature-mod dependencies. */
#include "../../runtime/native/gameplay.h"
#include "api.h"
#include "../../runtime/native/input.h"
#include "store.h"
#include "../../runtime/sdk/wallet.h"
#include "shop_signal.h"
int _fltused=0;
typedef struct {float x,y;} V2;
static struct {
 PTR world_field,transform,position,component,alive,find,resources,focused,typing,blocked,menu,restoring,first,frame;
 PTR joined,host,local,player,wallet,charge,decimal_ctor,decimal_int,save_root;
 PTR go_ctor,go_add,go_tr,parent,destroy,active,anchor_min,anchor_max,pivot,anchored,size;
 PTR canvas_mode,canvas_order,text,font,font_size,color,raycast,alignment,image_color;
 PTR map_open,map_begin,map_close,landmark_count,landmark_dir,raised,settings_field,radius_field;
 DRoot world,walker,ui,label,map;DWORD thread;int last_frame,down[256],pressed[256];
 int owned,shop,active_tool,invalid_store,dirty,ready,authority_lost;char root[192];unsigned char known[128];
 const char *status;float next_scan,time;PTR log;
 int (*map_context_active)(void *walker);
} m;
static inline int malive(PTR obj){if(!obj)return 0;int ok=1;void *a[]={obj};PTR v=call(m.alive,0,a,&ok);return ok&&v&&*(unsigned char*)unbox(v);}

static inline void mevent(const char *s){if(!m.log)return;char b[256],*p=cat(b,"{\"event\":\"");p=cat(p,s);p=cat(p,"\"}\n");DWORD n=0;WriteFile(m.log,b,(DWORD)(p-b),&n,0);}
static inline PTR mm(const char *ns,const char *type,const char *name,const char *sig){return dmethod(ns,type,name,sig);}
static inline DVec vadd(DVec a,DVec b){return (DVec){a.x+b.x,a.y+b.y,a.z+b.z};}
static inline DVec vmul(DVec a,float f){return (DVec){a.x*f,a.y*f,a.z*f};}
static inline float vdot(DVec a,DVec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
static inline float msqrt(float x){if(x<=0)return 0;float n=x>1?x:1;for(int i=0;i<18;i++)n=.5f*(n+x/n);return n;}
static inline DVec normal(DVec a){float n=msqrt(vdot(a,a));return n>.001f?vmul(a,1/n):(DVec){0,1,0};}
static inline float dist2(DVec a,DVec b){return vdot(vadd(a,vmul(b,-1)),vadd(a,vmul(b,-1)));}
static inline PTR mc(PTR obj,const char *ns,const char *name,int *ok){void *a[]={dtype(ns,name)};PTR get=object_class(obj)==find_class("UnityEngine","GameObject")?mm("UnityEngine","GameObject","GetComponent","System.Type"):m.component;return a[0]?call(get,obj,a,ok):0;}
static inline DVec mpos(PTR obj,int *ok){return dvector(m.position,call(m.transform,obj,0,ok),ok);}
static inline PTR objects(const char *name,int *ok){int inactive=0,sort=0;void *a[]={dtype("MoonCleaners",name),&inactive,&sort};return call(m.find,0,a,ok);}
static inline int host(void){int ok=1;if(!boolean(m.joined,0,&ok))return ok;PTR h=call(m.host,0,0,&ok),l=call(m.local,0,0,&ok);return ok&&h&&l&&*(U64*)unbox(h)&&*(U64*)unbox(h)==*(U64*)unbox(l);}
static inline void destroy_root(DRoot *root){PTR obj=dget(*root);if(malive(obj)){int ok=1;void *a[]={obj};call(m.destroy,0,a,&ok);}ddrop(root);}
static inline void mset(PTR method_id,PTR obj,void *value){int ok=1;void *a[]={value};call(method_id,obj,a,&ok);}
static inline PTR go(const char *name,PTR parent,int *ok){PTR o=da.new_object(find_class("UnityEngine","GameObject"));void *a[]={da.string(name)};call(m.go_ctor,o,a,ok);if(parent){PTR tr=call(m.go_tr,o,0,ok);unsigned char keep=0;void *b[]={parent,&keep};call(m.parent,tr,b,ok);}return o;}
static inline PTR add(PTR obj,const char *ns,const char *type,int *ok){void *a[]={dtype(ns,type)};return call(m.go_add,obj,a,ok);}
static inline void rect(PTR obj,float x,float y,float w,float h,int *ok){PTR r=mc(obj,"UnityEngine","RectTransform",ok);V2 one={1,1},pos={x,y},extent={w,h};mset(m.anchor_min,r,&one);mset(m.anchor_max,r,&one);mset(m.pivot,r,&one);mset(m.anchored,r,&pos);mset(m.size,r,&extent);}
static inline int ui(void){
 if(malive(dget(m.ui)))return 1;ddrop(&m.ui);ddrop(&m.label);int ok=1;
 PTR root=go(MOD_ID " HUD",0,&ok);m.ui=dhold(root);PTR canvas=add(root,"UnityEngine","Canvas",&ok);int mode=0,order=230+FEATURE; mset(m.canvas_mode,canvas,&mode);mset(m.canvas_order,canvas,&order);
 PTR tr=call(m.go_tr,root,0,&ok),panel=go(MOD_ID " panel",tr,&ok);add(panel,"UnityEngine","RectTransform",&ok);PTR image=add(panel,"UnityEngine.UI","Image",&ok);DColor bg={.055f,.06f,.065f,.94f};mset(m.image_color,image,&bg);unsigned char no=0;mset(m.raycast,image,&no);rect(panel,-16,-(16+HUD_Y),408,126,&ok);
 PTR label=go(MOD_ID " text",call(m.go_tr,panel,0,&ok),&ok);add(label,"UnityEngine","RectTransform",&ok);PTR text=add(label,"UnityEngine.UI","Text",&ok);m.label=dhold(text);rect(label,-10,-8,388,110,&ok);
 void *a[]={dtype("UnityEngine","Font")};DRoot fonts=dhold(call(m.resources,0,a,&ok));PTR f=fonts&&da.array_length(dget(fonts))?*(PTR*)darray(dget(fonts),0):0;if(!f)ok=0;else {void *b[]={f};call(m.font,text,b,&ok);}ddrop(&fonts);
 int size=16,align=0;mset(m.font_size,text,&size);mset(m.alignment,text,&align);DColor fg={.92f,.93f,.94f,1};mset(m.color,text,&fg);mset(m.raycast,text,&no);
 return ok;
}
static inline void show(const char *text,int visible){(void)text;(void)visible;}
static inline unsigned balance(PTR world,PTR player,int *ok){void *a[]={player};DRoot value=dhold(call(m.wallet,world,a,ok));void *b[]={value?unbox(dget(value)):0};PTR number=b[0]?call(m.decimal_int,0,b,ok):0;int n=number?*(int*)unbox(number):-1;if(n<0)*ok=0;ddrop(&value);return n<0?0:(unsigned)n;}
typedef struct {PTR world,player;} MappingWalletContext;
static inline int mapping_balance(void *context,long long *amount){MappingWalletContext *c=context;int ok=1;*amount=balance(c->world,c->player,&ok);return ok;}
static inline int mapping_adjust(void *context,int delta){MappingWalletContext *c=context;int ok=1;unsigned char decimal[16]={0};void *a[]={&delta};call(m.decimal_ctor,decimal,a,&ok);void *b[]={c->player,decimal};if(ok)call(m.charge,c->world,b,&ok);return ok;}
static inline int debit(int price){int ok=1;PTR id=call(m.player,0,0,&ok);DRoot player=dhold(id);if(!ok||!player){ddrop(&player);return 0;}MappingWalletContext context={dget(m.world),dget(player)};const MoonWallet wallet={mapping_balance,mapping_adjust};int result=mw_debit(&wallet,&context,price,0);ddrop(&player);return result==MW_PAID;}
static inline void save(void){if(!m.dirty||!m.root[0]||!host()||m.authority_lost)return;unsigned char b[136]={0x4d,0x4d,0x01,FEATURE};b[4]=(unsigned char)m.owned;dcopy(b+8,m.known,128);if(ss_save(m.root,b,sizeof(b))){m.dirty=0;}else m.status="Save pending; do not buy again";}
static inline void load(void){int ok=1;PTR value=call(m.save_root,dget(m.world),0,&ok);if(!ok||!value)return;int n=((int(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_string_length"))(value);const unsigned short *s=((const unsigned short *(*)(PTR))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_string_chars"))(value);if(n<1||n>180)return;for(int i=0;i<n;i++){if(s[i]>127)return;m.root[i]=(char)s[i];}m.root[n]=0;
 unsigned char b[136];unsigned length=0;if(ss_load(m.root,b,sizeof(b),&length)){if(length!=sizeof(b)||b[0]!=0x4d||b[1]!=0x4d||b[2]!=1||b[3]!=FEATURE||b[4]>1){m.invalid_store=1;return;}m.owned=b[4];dcopy(m.known,b+8,128);}else if(ss.last_error!=SS_NOT_FOUND)m.invalid_store=1;
}
static inline void purchase(int price){if(m.owned||m.invalid_store||!m.root[0]||!host()||m.authority_lost)return;if(debit(price)){m.owned=1;m.dirty=1;m.status="Purchased";save();mevent("purchase");}else m.status="Insufficient funds or unavailable";}
static inline PTR map(void){int ok=1;PTR o=mc(dget(m.walker),"MoonCleaners","MoonMapScreen",&ok);return ok&&malive(o)?o:0;}
static inline int mapopen(PTR obj){int ok=1;return obj&&boolean(m.map_open,obj,&ok)&&ok;}
static inline int known(int i){return i>=0&&i<1024&&(m.known[i>>3]&(1u<<(i&7)));}
static inline void discover(int i){if(i<0||i>=1024||known(i))return;m.known[i>>3]|=(unsigned char)(1u<<(i&7));m.dirty=1;}
static inline DVec direction(PTR world,int i,int *ok){void *a[]={&i};PTR v=call(m.landmark_dir,world,a,ok);return v?*(DVec*)unbox(v):(DVec){0};}
static inline int raised(PTR world,int i,int *ok){void *a[]={&i};PTR v=call(m.raised,world,a,ok);return v&&*(unsigned char*)unbox(v);}
static inline int isflag(PTR world,int i){int ok=1;void *a[]={&i};PTR item=call(method("MoonCleaners","MoonWorld","Landmark",1),world,a,&ok);if(!ok||!item)return 0;int kind=-1,flag=-2;PTR f=get_field(object_class(item),"kind"),k=get_field(find_class("MoonCleaners","MoonLandmarkKind"),"Flag");void (*stat)(PTR,void*)=(void(*)(PTR,void*))GetProcAddress(GetModuleHandleA("GameAssembly.dll"),"il2cpp_field_static_get_value");if(!f||!k||!stat)return 0;field_value(item,f,&kind);stat(k,&flag);return kind==flag;}
static inline float radius(PTR world){PTR settings=0;float r=0;field_value(world,m.settings_field,&settings);if(settings)field_value(settings,m.radius_field,&r);return r;}
static int feature_start(PTR domain,PTR game);
static void feature_end(void);
static void feature_tick(PTR walker,PTR world,int input);
static int update(int id,Context *context){(void)id;if(!m.thread)m.thread=GetCurrentThreadId();if(m.thread!=GetCurrentThreadId())return 0;int ok=1;int frame=integer(m.frame,0,&ok);if(!ok||frame==m.last_frame)return 0;m.last_frame=frame;
 PTR walker=(PTR)context->rcx,world=0;field_value(walker,m.world_field,&world);
 if(!malive(world)||world!=dget(m.world)||walker!=dget(m.walker)){
  save();feature_end();destroy_root(&m.ui);ddrop(&m.label);ddrop(&m.map);ddrop(&m.world);ddrop(&m.walker);m.root[0]=0;m.owned=m.shop=m.active_tool=m.invalid_store=m.dirty=m.authority_lost=0;dzero(m.known,sizeof(m.known));m.status="";m.next_scan=0;
  if(!malive(world))return 0;m.world=dhold(world);m.walker=dhold(walker);if(host())load();
 }
 if(boolean(m.restoring,world,&ok)||!ok){show("",0);return 0;}
 if(!host())m.authority_lost=1;
 moon_input_snapshot(m.down,m.pressed);
#ifdef MOD_INPUT_ACTIONS
 static const MoonInputAlias input_actions[]=MOD_INPUT_ACTIONS;
 moon_input_alias_edges(input_actions,sizeof(input_actions)/sizeof(*input_actions),m.pressed,1);
#endif
 int menu_blocked=boolean(m.menu,walker,&ok);
 int input=boolean(m.focused,0,&ok)&&!boolean(m.typing,0,&ok)&&(!menu_blocked||(m.map_context_active&&m.map_context_active(walker)))&&ok;
 m.time=dnumber(mm("UnityEngine","Time","get_unscaledTime",""),0,&ok);
 if(moon_shop_poll()&&input){m.shop=1;m.status="";}
 if(!input){m.shop=0;show("",0);return 0;}
 feature_tick(walker,world,input);if(m.dirty&&m.time>=m.next_scan+2)save();return 0;
}
static int bind(PTR game){
 (void)game;
#define B(F,N,T,M,S) m.F=mm(N,T,M,S);if(!m.F)return 0
 B(transform,"UnityEngine","Component","get_transform","");B(position,"UnityEngine","Transform","get_position","");B(component,"UnityEngine","Component","GetComponent","System.Type");B(alive,"UnityEngine","Object","op_Implicit","UnityEngine.Object");B(find,"UnityEngine","Object","FindObjectsByType","System.Type;UnityEngine.FindObjectsInactive;UnityEngine.FindObjectsSortMode");B(resources,"UnityEngine","Resources","FindObjectsOfTypeAll","System.Type");
 B(focused,"UnityEngine","Application","get_isFocused","");B(typing,"MoonCleaners","MoonInput","get_Typing","");B(menu,"MoonCleaners","MoonWalker","get_MenuBlocked","");B(blocked,"MoonCleaners","MoonWalker","get_UIBlocked","");B(first,"MoonCleaners","MoonWalker","get_IsFirstPerson","");B(restoring,"MoonCleaners","MoonWorld","get_IsRestoringRendering","");B(frame,"UnityEngine","Time","get_frameCount","");
 B(joined,"MoonCleaners","MoonCrew","get_Joined","");B(host,"MoonCleaners","MoonCrew","get_HostId","");B(local,"Steamworks","SteamUser","GetSteamID","");B(player,"MoonCleaners","MoonCrew","get_LocalPlayerId","");B(wallet,"MoonCleaners","MoonWorld","WalletDollars","System.String");B(charge,"MoonCleaners","MoonWorld","ChargeBuild","System.String;System.Decimal");B(decimal_ctor,"System","Decimal",".ctor","System.Int32");B(decimal_int,"System","Decimal","ToInt32","System.Decimal");B(save_root,"MoonCleaners","MoonWorld","get_SaveDirectory","");
 B(go_ctor,"UnityEngine","GameObject",".ctor","System.String");B(go_add,"UnityEngine","GameObject","AddComponent","System.Type");B(go_tr,"UnityEngine","GameObject","get_transform","");B(parent,"UnityEngine","Transform","SetParent","UnityEngine.Transform;System.Boolean");B(destroy,"UnityEngine","Object","Destroy","UnityEngine.Object");B(active,"UnityEngine","GameObject","SetActive","System.Boolean");
 B(anchor_min,"UnityEngine","RectTransform","set_anchorMin","UnityEngine.Vector2");B(anchor_max,"UnityEngine","RectTransform","set_anchorMax","UnityEngine.Vector2");B(pivot,"UnityEngine","RectTransform","set_pivot","UnityEngine.Vector2");B(anchored,"UnityEngine","RectTransform","set_anchoredPosition","UnityEngine.Vector2");B(size,"UnityEngine","RectTransform","set_sizeDelta","UnityEngine.Vector2");B(canvas_mode,"UnityEngine","Canvas","set_renderMode","UnityEngine.RenderMode");B(canvas_order,"UnityEngine","Canvas","set_sortingOrder","System.Int32");
 B(text,"UnityEngine.UI","Text","set_text","System.String");B(font,"UnityEngine.UI","Text","set_font","UnityEngine.Font");B(font_size,"UnityEngine.UI","Text","set_fontSize","System.Int32");B(color,"UnityEngine.UI","Graphic","set_color","UnityEngine.Color");B(raycast,"UnityEngine.UI","Graphic","set_raycastTarget","System.Boolean");B(alignment,"UnityEngine.UI","Text","set_alignment","UnityEngine.TextAnchor");B(image_color,"UnityEngine.UI","Graphic","set_color","UnityEngine.Color");
 B(map_open,"MoonCleaners","MoonMapScreen","get_IsOpen","");B(map_begin,"MoonCleaners","MoonMapScreen","Open","");B(map_close,"MoonCleaners","MoonMapScreen","Close","");B(landmark_count,"MoonCleaners","MoonWorld","get_LandmarkCount","");B(landmark_dir,"MoonCleaners","MoonWorld","LandmarkDirection","System.Int32");B(raised,"MoonCleaners","MoonWorld","IsLandmarkRaised","System.Int32");
#undef B
 m.world_field=get_field(find_class("MoonCleaners","MoonWalker"),"world");m.settings_field=get_field(find_class("MoonCleaners","MoonWorld"),"settings");m.radius_field=get_field(find_class("MoonCleaners","MoonSettings"),"radius");return m.world_field&&m.settings_field&&m.radius_field&&ss_bind();
}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){if(!bind_runtime(domain,game)||!dapi(game)||!bind(game)||!feature_start(domain,game))return 0;char path[1024];DWORD n=GetEnvironmentVariableA("MOONLAB_EVIDENCE",path,900);if(n&&n<900){cat(path+n,"\\" MOD_ID ".jsonl");m.log=CreateFileA(path,0x40000000,1,0,1,0x80,0);}if(FEATURE==8||FEATURE==15)moon_shop_listen(MOD_ID);m.last_frame=-1;before_hook=update;if(!prepare_hook(method("MoonCleaners","MoonWalker","Update",0),100671733,0,game)||!install_hooks()){moon_shop_unlisten();return 0;}mevent("ready-testing-deferred");return 1;}

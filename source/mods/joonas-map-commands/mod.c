#define MOD_INPUT_ACTIONS {{"joonas-map-commands.select","LeftMouse",1}, {"joonas-map-commands.stop","S",'S'}, {"joonas-map-commands.resume","C",'C'}, {"joonas-map-commands.roam","R",'R'}, {"joonas-map-commands.point","P",'P'}, {"joonas-map-commands.area","G",'G'}}
#define FEATURE 5
#define MOD_ID "joonas-map-commands"
#define HUD_Y 0
#include "runtime.h"
#include "../map-control-framework/sdk/client.h"
#include "../map-control-framework/ground_pick.h"
static const MoonMapControls *controls;
static MMCGroundBindings ground_bindings;
static DRoot docks[MMC_UNIT_LIMIT];static int dock_ids[MMC_UNIT_LIMIT],dock_count,selected=-1,selection_changed;static float next_docks;
static int has_ground,last_sector=-1,ground_attempted;static float ground_attempt_at;static MoonMapVector last_point,last_direction;
enum {JC_MOVE=1,JC_SECTOR,JC_STOP,JC_RESUME,JC_ROAM};
static void clear_ground(void){has_ground=ground_attempted=0;last_sector=-1;last_point=(MoonMapVector){0};last_direction=(MoonMapVector){0};}
static void feature_end(void){for(int i=0;i<dock_count;i++)ddrop(docks+i);dock_count=0;selected=-1;selection_changed=0;clear_ground();next_docks=0;}
static void feature_idle(void){
 PTR screen=map();if(screen!=dget(m.map)){feature_end();ddrop(&m.map);m.map=dhold(screen);}
 if(!mapopen(screen))clear_ground();
}
static void scan_docks(PTR world){
 if(m.time<next_docks)return;next_docks=m.time+.5f;for(int i=0;i<dock_count;i++)ddrop(docks+i);dock_count=0;
 int ok=1;DRoot found=dhold(objects("MoonRobotDock",&ok));U64 count=found?da.array_length(dget(found)):0;if(count>256)count=256;
 for(U64 i=0;i<count&&dock_count<MMC_UNIT_LIMIT;i++){
  ok=1;PTR dock=*(PTR*)darray(dget(found),i);if(!malive(dock))continue;
  PTR w=call(method("MoonCleaners","MoonRobotDock","get_World",0),dock,0,&ok);if(!ok||w!=world)continue;
  int id=integer(method("MoonCleaners","MoonRobotDock","get_DockId",0),dock,&ok);if(!ok||id<0)continue;
  PTR robot=call(method("MoonCleaners","MoonRobotDock","get_Robot",0),dock,0,&ok);PTR launcher=robot?call(method("MoonCleaners","MoonRobotVacuum","get_Launcher",0),robot,0,&ok):0;if(!ok||!malive(robot)||!launcher)continue;
  int duplicate=0;for(int j=0;j<dock_count;j++)if(dock_ids[j]==id)duplicate=1;if(duplicate)continue;
  DRoot root=dhold(dock);if(root){docks[dock_count]=root;dock_ids[dock_count++]=id;}
 }
 ddrop(&found);
}
static PTR selected_dock(int id){for(int i=0;i<dock_count;i++)if(dock_ids[i]==id&&malive(dget(docks[i])))return dget(docks[i]);return 0;}
static int move_order(PTR world,PTR dock,int id,MoonMapVector destination){
 int ok=1;PTR robot=call(method("MoonCleaners","MoonRobotDock","get_Robot",0),dock,0,&ok);if(!ok||!malive(robot))return 0;
 PTR game=GetModuleHandleA("GameAssembly.dll");PTR (*nested)(PTR,PTR*)=(PTR(*)(PTR,PTR*))GetProcAddress(game,"il2cpp_class_get_nested_types");const char *(*name)(PTR)=(const char *(*)(PTR))GetProcAddress(game,"il2cpp_class_get_name");void (*set)(PTR,PTR,void*)=(void(*)(PTR,PTR,void*))GetProcAddress(game,"il2cpp_field_set_value");void (*stat)(PTR,void*)=(void(*)(PTR,void*))GetProcAddress(game,"il2cpp_field_static_get_value");
 PTR route=mm("MoonCleaners","MoonRobotVacuum","PlanRoute","UnityEngine.Vector3"),rc=find_class("MoonCleaners","MoonRobotVacuum"),target=get_field(rc,"target"),state=get_field(rc,"state"),clock=get_field(rc,"stateTime"),iterator=0,child,travel=0;
 if(!nested||!name||!set||!stat||!route||method_token(route)!=100670010||!target||!state||!clock){m.status="Current native point-route bindings unavailable";return 0;}
 while((child=nested(rc,&iterator)))if(deq(name(child),"State")){travel=get_field(child,"Travel");break;}if(!travel)return 0;
 PTR power_method=mm("MoonCleaners","MoonWorld","SetRobotOn","System.Int32;System.Boolean");
 DVec point={destination.x,destination.y,destination.z};float square=vdot(point,point);
 if(!power_method||!mmc_ground_vector(point)||!(square>1&&square<1e16f))return 0;
 void *args[]={&point};PTR planned=call(route,robot,args,&ok);
 PTR result=planned&&object_class(planned)==find_class("System","Boolean")?unbox(planned):0;
 if(!ok||!result||!*(unsigned char*)result){m.status="Native route unavailable";return 0;}
 /* A rejected route must not resume a stopped robot. Power only after the
  * native route succeeds, and never write state into a replaced robot. */
 unsigned char powered=1;void *power[]={&id,&powered};call(power_method,world,power,&ok);
 PTR current=ok?call(method("MoonCleaners","MoonRobotDock","get_Robot",0),dock,0,&ok):0;
 if(!ok||current!=robot||!malive(robot))return 0;
 if(ok){int travelling=0;float zero=0;stat(travel,&travelling);set(robot,target,&point);set(robot,state,&travelling);set(robot,clock,&zero);m.status="Moving to destination";}else m.status="Native route unavailable";
 return ok;
}
static int command(const MoonMapContext *ctx,int id,unsigned action){
 if(!ctx||ctx->abi!=MMC_ABI||ctx->size<sizeof(*ctx)||ctx->world!=dget(m.world)||ctx->walker!=dget(m.walker)||!moon_local_actor_admit(ctx->walker)||ctx->map!=map()||!mapopen(ctx->map)||!host()||m.authority_lost)return 0;
 int ok=1;if(!controls||!controls->context_active(ctx->walker)||!boolean(m.focused,0,&ok)||boolean(m.typing,0,&ok)||boolean(m.restoring,ctx->world,&ok)||!ok)return 0;
 scan_docks(ctx->world);PTR dock=selected_dock(id);PTR dock_world=dock?call(method("MoonCleaners","MoonRobotDock","get_World",0),dock,0,&ok):0;
 PTR robot=dock?call(method("MoonCleaners","MoonRobotDock","get_Robot",0),dock,0,&ok):0;
 PTR launcher=robot?call(method("MoonCleaners","MoonRobotVacuum","get_Launcher",0),robot,0,&ok):0;
 if(!ok||dock_world!=ctx->world||!malive(robot)||!launcher)dock=0;if(!dock){m.status="Select a loaded dock-backed J.O.O.N.A.S.";return 0;}
 PTR power=mm("MoonCleaners","MoonWorld","SetRobotOn","System.Int32;System.Boolean"),roams=mm("MoonCleaners","MoonRobotDock","set_Roams","System.Boolean");
 if(action==JC_STOP||action==JC_RESUME){unsigned char on=(unsigned char)(action==JC_RESUME);void *a[]={&id,&on};call(power,ctx->world,a,&ok);m.status=ok?(on?"Robot resumed":"Robot stopped"):"Power unavailable";}
 else if(action==JC_ROAM){unsigned char yes=1;void *a[]={&yes};call(roams,dock,a,&ok);m.status=ok?"Robot roaming":"Roaming unavailable";}
 else if(action==JC_MOVE){if(!ctx->has_ground){m.status="Right-click ground to choose a destination";return 0;}ok=move_order(ctx->world,dock,id,ctx->ground_point);}
 else if(action==JC_SECTOR){
  int sector=ctx->hovered_sector;PTR area=call(method("MoonCleaners","MoonRobotDock","get_Area",0),dock,0,&ok),add_method=area?get_method(object_class(area),"Add",1):0,clear=area?get_method(object_class(area),"Clear",0):0;
  PTR version=mm("MoonCleaners","MoonRobotDock","set_AreaVersion","System.Int32");
  /* Preflight before mutating the native work area; vanilla retains route
   * planning, simulation and save ownership. */
  if(ok&&sector>=0&&sector<100000&&area&&add_method&&clear&&roams&&version){
   int revision=integer(method("MoonCleaners","MoonRobotDock","get_AreaVersion",0),dock,&ok);if(ok&&revision<2147483646){unsigned char no=0;void *a[]={&no},*b[]={&sector};call(clear,area,0,&ok);if(ok)call(add_method,area,b,&ok);if(ok)call(roams,dock,a,&ok);revision++;void *c[]={&revision};if(ok)call(version,dock,c,&ok);m.status=ok?"Work sector assigned":"Order unavailable";}else ok=0;
  }else{m.status="Hover ground before choosing a work sector";return 0;}
 }else return 0;
 if(ok){call(method("MoonCleaners","MoonWorld","RequestSave",0),ctx->world,0,&ok);if(action==JC_MOVE)call(method("MoonCleaners","MoonMapScreen","CloseMenu",0),ctx->map,0,&ok);}mevent(ok?"map-command":"map-command-unavailable");return ok;
}
static int describe(const MoonMapContext *ctx,int requested,MoonMapRow *row){
 if(!ctx||ctx->abi!=MMC_ABI||ctx->size<sizeof(*ctx)||ctx->world!=dget(m.world)||ctx->walker!=dget(m.walker)||!moon_local_actor_admit(ctx->walker)||ctx->map!=map())return 0;scan_docks(ctx->world);
 if(ctx->hovered_sector>=0)last_sector=ctx->hovered_sector;
 if(!selection_changed&&selected_dock(requested))selected=requested;selection_changed=0;if(!selected_dock(selected))selected=dock_count?dock_ids[0]:-1;
 row->unit_count=(unsigned)dock_count;row->selected_id=selected;for(int i=0;i<dock_count;i++){row->units[i].id=dock_ids[i];num(cat(row->units[i].label,"Dock "),(U64)dock_ids[i]);}
 const char *labels[]={"P Move","G Sector","S Stop","C Resume","R Roam"};int authority=host()&&!m.authority_lost&&selected>=0;row->action_count=5;
 for(unsigned i=0;i<5;i++){row->actions[i].command=i+1;cat(row->actions[i].label,labels[i]);row->actions[i].enabled=authority&&(i!=0||ctx->has_ground)&&(i!=1||ctx->hovered_sector>=0);}
 cat(row->status,!host()||m.authority_lost?"Host controls only":selected<0?"No loaded dock-backed robots":m.status&&m.status[0]?m.status:"Right-click ground, then Move; select robots with < >");return 1;
}
static void keyboard_ground(PTR screen,MoonMapContext *ctx){
 unsigned char open=0,ground=0;PTR cls=object_class(screen),f=get_field(cls,"menuOpen"),g=get_field(cls,"menuOnGround"),spot=get_field(cls,"menuSpot"),at=get_field(cls,"menuAt");
 if(!f||!g||!spot||!at){has_ground=0;ctx->has_ground=0;return;}
 field_value(screen,f,&open);field_value(screen,g,&ground);
 if(open){
  float menu_at=0;field_value(screen,at,&menu_at);
  if(!ground){has_ground=0;ground_attempted=0;}
  else if(!ground_attempted||menu_at!=ground_attempt_at){
   ground_attempted=1;ground_attempt_at=menu_at;has_ground=0;
   MMCGroundV2 local={0};field_value(screen,spot,&local);DVec direction={0};
   int result=mmc_ground_pick_local(&ground_bindings,screen,local,&direction,0);
   if(result==MMC_GROUND_OK){
    float r=radius(ctx->world);if(r>1&&r<1e7f){direction=normal(direction);DVec point=vmul(direction,r);has_ground=1;last_direction=(MoonMapVector){direction.x,direction.y,direction.z};last_point=(MoonMapVector){point.x,point.y,point.z};}
   }
  }
 }
 ctx->has_ground=has_ground;ctx->ground_direction=last_direction;ctx->ground_point=last_point;
}
static void feature_tick(PTR walker,PTR world,int input){
 (void)input;feature_idle();PTR screen=map();if(!mapopen(screen))return;scan_docks(world);
 if(controls&&controls->pointer_captured(walker))return;
 /* Use the native mark's stable dock ID, as the framework does. The robot
  * list can contain boxed value records and is not a component array. */
 PTR cls=object_class(screen),mark=0,mf=get_field(cls,"hoveredMark");if(mf)field_value(screen,mf,&mark);
 if(m.pressed[1]&&mark){int kind=-1,id=-1;PTR type=object_class(mark),kf=get_field(type,"kind"),idf=get_field(type,"id");if(kf&&idf){field_value(mark,kf,&kind);field_value(mark,idf,&id);if(kind==5&&selected_dock(id)){selected=id;selection_changed=1;}}}
 MoonMapContext ctx={MMC_ABI,sizeof(ctx),0,world,walker,screen,has_ground,last_sector,-1,-1,last_direction,last_point};PTR sector=get_field(cls,"hoveredSector");if(sector)field_value(screen,sector,&ctx.hovered_sector);
 if(m.pressed['P']){keyboard_ground(screen,&ctx);command(&ctx,selected,JC_MOVE);}if(m.pressed['G'])command(&ctx,selected,JC_SECTOR);if(m.pressed['S'])command(&ctx,selected,JC_STOP);if(m.pressed['C'])command(&ctx,selected,JC_RESUME);if(m.pressed['R'])command(&ctx,selected,JC_ROAM);
}
static int feature_start(PTR domain,PTR game){if(!mmc_ground_bind(&ground_bindings,game))return 0;controls=mmc_connect(domain,game);if(!controls)return 0;m.map_context_active=controls->context_active;MoonMapProvider provider={MMC_ABI,sizeof(provider),MOD_ID,"J.O.O.N.A.S.",describe,command};return controls->register_provider(&provider);}

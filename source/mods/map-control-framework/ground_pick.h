#ifndef MOON_MAP_GROUND_PICK_H
#define MOON_MAP_GROUND_PICK_H
/* Native OpenMenu stores menuSpot in overlayLayer's local coordinates, after
 * ScreenPointToLocalPointInRectangle. Pick instead accepts screen pixels. Keep
 * this inverse in one production boundary for panel and retained keyboard use. */
typedef struct {float x,y;} MMCGroundV2;
typedef struct {PTR overlay_field,transform_point,world_screen,pick,vector2,vector3;} MMCGroundBindings;
enum {MMC_GROUND_OK=1,MMC_GROUND_LOCAL=-1,MMC_GROUND_OVERLAY=-2,MMC_GROUND_TRANSFORM=-3,MMC_GROUND_SCREEN=-4,MMC_GROUND_PICK=-5};
static inline int mmc_ground_finite(float value){return value==value&&value>-1e12f&&value<1e12f;}
static inline int mmc_ground_vector(DVec value){return mmc_ground_finite(value.x)&&mmc_ground_finite(value.y)&&mmc_ground_finite(value.z);}
static inline int mmc_ground_bind(MMCGroundBindings *binding,PTR game){
 binding->vector2=find_class("UnityEngine","Vector2");binding->vector3=find_class("UnityEngine","Vector3");
#define GB(F,N,T,M,S,K) binding->F=dmethod(N,T,M,S);if(!binding->F||method_token(binding->F)!=K)return 0
 GB(transform_point,"UnityEngine","Transform","TransformPoint","UnityEngine.Vector3",100667384);
 GB(world_screen,"UnityEngine","RectTransformUtility","WorldToScreenPoint","UnityEngine.Camera;UnityEngine.Vector3",100663387);
 GB(pick,"MoonCleaners","MoonMapScreen","Pick","UnityEngine.Vector2",100668218);
#undef GB
 binding->overlay_field=get_field(find_class("MoonCleaners","MoonMapScreen"),"overlayLayer");
 PTR (*field_type)(PTR)=(PTR(*)(PTR))GetProcAddress(game,"il2cpp_field_get_type");int (*field_offset)(PTR)=(int(*)(PTR))GetProcAddress(game,"il2cpp_field_get_offset");
 if(!binding->vector2||!binding->vector3||!binding->overlay_field||!field_type||!field_offset||field_offset(binding->overlay_field)!=640)return 0;
 char *name=da.type_name(field_type(binding->overlay_field));int typed=name&&deq(name,"UnityEngine.RectTransform");if(name)da.free(name);return typed;
}
static inline int mmc_ground_pick_local(const MMCGroundBindings *binding,PTR map,MMCGroundV2 local,DVec *direction,MMCGroundV2 *screen_out){
 if(!mmc_ground_finite(local.x)||!mmc_ground_finite(local.y))return MMC_GROUND_LOCAL;
 PTR overlay=0;field_value(map,binding->overlay_field,&overlay);if(!overlay)return MMC_GROUND_OVERLAY;
 int ok=1;DVec position={local.x,local.y,0};void *transform_args[]={&position};PTR boxed=call(binding->transform_point,overlay,transform_args,&ok);
 if(!ok||!boxed||object_class(boxed)!=binding->vector3)return MMC_GROUND_TRANSFORM;
 DVec world=*(DVec*)unbox(boxed);if(!mmc_ground_vector(world))return MMC_GROUND_TRANSFORM;
 void *screen_args[]={0,&world};boxed=call(binding->world_screen,0,screen_args,&ok);
 if(!ok||!boxed||object_class(boxed)!=binding->vector2)return MMC_GROUND_SCREEN;
 MMCGroundV2 screen=*(MMCGroundV2*)unbox(boxed);
 if(!mmc_ground_finite(screen.x)||!mmc_ground_finite(screen.y)||screen.x<0||screen.y<0)return MMC_GROUND_SCREEN;
 void *pick_args[]={&screen};boxed=call(binding->pick,map,pick_args,&ok);
 /* Preserve the current runtime-invoke Nullable behavior: a valid result is
  * boxed as Vector3, and an empty pick returns null. */
 if(!ok||!boxed||object_class(boxed)!=binding->vector3)return MMC_GROUND_PICK;
 DVec picked=*(DVec*)unbox(boxed);float square=picked.x*picked.x+picked.y*picked.y+picked.z*picked.z;
 if(!mmc_ground_vector(picked)||!(square>.01f&&square<1e16f))return MMC_GROUND_PICK;
 *direction=picked;if(screen_out)*screen_out=screen;return MMC_GROUND_OK;
}
static inline const char *mmc_ground_failure_event(int result){
 switch(result){case MMC_GROUND_LOCAL:return "ground-pick-invalid-local";case MMC_GROUND_OVERLAY:return "ground-pick-missing-overlay";case MMC_GROUND_TRANSFORM:return "ground-pick-transform-failed";case MMC_GROUND_SCREEN:return "ground-pick-screen-failed";default:return "ground-pick-empty";}
}
#endif
